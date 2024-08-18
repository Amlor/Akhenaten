#include "building_storage_yard.h"

#include "building/building_barracks.h"
#include "building/building_storage_room.h"
#include "building/count.h"
#include "building/building_granary.h"
#include "building/industry.h"
#include "building/rotation.h"
#include "building/model.h"
#include "building/figure.h"
#include "building/monuments.h"
#include "city/buildings.h"
#include "city/finance.h"
#include "city/city.h"
#include "city/warnings.h"
#include "city/city_resource.h"
#include "city/labor.h"
#include "core/calc.h"
#include "core/vec2i.h"
#include "empire/trade_prices.h"
#include "game/tutorial.h"
#include "game/game.h"
#include "game/undo.h"
#include "graphics/image.h"
#include "graphics/text.h"
#include "graphics/image_groups.h"
#include "graphics/elements/panel.h"
#include "graphics/elements/button.h"
#include "graphics/elements/image_button.h"
#include "graphics/elements/lang_text.h"
#include "grid/image.h"
#include "grid/building_tiles.h"
#include "grid/terrain.h"
#include "figure/figure.h"
#include "grid/road_access.h"
#include "scenario/scenario.h"
#include "config/config.h"
#include "widget/city/ornaments.h"
#include "widget/city/building_ghost.h"

#include "figuretype/figure_storageyard_cart.h"
#include "figuretype/figure_sled.h"

#include <cmath>

buildings::model_t<building_storage_yard> storage_yard_m;

ANK_REGISTER_CONFIG_ITERATOR(config_load_building_storage_yard);
void config_load_building_storage_yard() {
    storage_yard_m.load();
}

int building_storage_yard::get_space_info() const {
    int total_amounts = 0;
    int empty_spaces = 0;
    building* space = &base;
    for (int i = 0; i < 8; i++) {
        space = space->next();
        if (space->id <= 0) {
            return 0;
        }

        if (space->subtype.warehouse_resource_id) {
            total_amounts += space->stored_full_amount;
        } else {
            empty_spaces++;
        }
    }

    if (empty_spaces > 0)
        return STORAGEYARD_ROOM;
    else if (total_amounts < 3200)
        return STORAGEYARD_SOME_ROOM;
    else
        return STORAGEYARD_FULL;
}

int building_storage_yard::amount(e_resource resource) const {
    int total = 0;
    const building* space = &base;
    for (int i = 0; i < 8; i++) {
        space = (const building*)space->next();
        if (space->id <= 0)
            return 0;

        if (space->subtype.warehouse_resource_id && space->subtype.warehouse_resource_id == resource) {
            total += space->stored_full_amount;
        }
    }
    return total;
}

int building_storage_yard::total_stored() const {
    int total_stored = 0;
    for (e_resource r = RESOURCE_MIN; r < RESOURCES_MAX; ++r) {
        total_stored += amount(r);
    }

    return total_stored;
}

int building_storage_yard::freespace(e_resource resource) {
    int freespace = 0;
    building_storage_room* space = room();
    while (space) {
        if (!space->base.subtype.warehouse_resource_id) {
            freespace += 400;
        } else if(space->base.subtype.warehouse_resource_id == resource) {
            freespace += (400 - space->stored_full_amount);
        }
        space = space->next_room();
    }
    return freespace;
}

int building_storage_yard::freespace() {
    int freespace = 0;
    building_storage_room* space = room();
    while (space) {
        if (!space->base.subtype.warehouse_resource_id) {
            freespace += 400;
        }
        space = space->next_room();
    }

    return freespace;
}

int building_storage_yard::add_resource(e_resource resource, bool is_produced, int amount, bool force) {
    assert(id() > 0);

    if (!force && is_not_accepting(resource)) {
        return -1;
    }

    // check the initial provided space itself, first
    bool look_for_space = false;
    if (base.subtype.warehouse_resource_id && base.subtype.warehouse_resource_id != resource) {
        look_for_space = true;
    } else if (base.stored_full_amount >= 400) {
        look_for_space = true;
    } else if (type() == BUILDING_STORAGE_YARD) {
        look_for_space = true;
    }

    // repeat until no space left... easier than calculating all amounts by hand.
    int amount_left = amount;
    int amount_last_turn = amount;
    building_storage_room* space = room();
    while (amount_left > 0) {
        if (look_for_space) {
            bool space_found = false;
            while (space) {
                if (!space->base.subtype.warehouse_resource_id || space->base.subtype.warehouse_resource_id == resource) {
                    if (space->stored_full_amount < 400) {
                        space_found = true;
                        break;
                    }
                }
                space = space->next_room();
            }

            if (!space_found) {
                return -1; // no space found at all!
            }
        }

        city_resource_add_to_storageyard(resource, 1);
        space->base.subtype.warehouse_resource_id = resource;
        int space_on_tile = 400 - space->stored_full_amount;
        int unloading_amount = std::min<int>(space_on_tile, amount_left);
        space->stored_full_amount += unloading_amount;
        space_on_tile -= unloading_amount;
        if (space_on_tile == 0) {
            look_for_space = true;
        }
        
        space->set_image(resource);
        amount_last_turn = amount_left;
        amount_left -= unloading_amount;

        if (amount_left == amount_last_turn)
            return amount_left;
    }
    return amount_left;
}

int building_storage_yard::remove_resource(e_resource resource, int amount) {
    building_storage_room* space = room();
    while (space) {
        if (amount <= 0)
            return 0;

        if (space->base.subtype.warehouse_resource_id != resource || space->base.stored_full_amount <= 0) {
            space = space->next_room();
            continue;
        }

        if (space->base.stored_full_amount > amount) {
            city_resource_remove_from_storageyard(resource, amount);
            space->base.stored_full_amount -= amount;
            amount = 0;

        } else {
            city_resource_remove_from_storageyard(resource, space->base.stored_full_amount);
            amount -= space->base.stored_full_amount;
            space->base.stored_full_amount = 0;
            space->base.subtype.warehouse_resource_id = RESOURCE_NONE;
        }
        space->set_image(resource);
        space = space->next_room();
    }

    return amount;
}
void building_storageyard_remove_resource_curse(building* b, int amount) {
    building_storage_yard *warehouse = b->dcast_storage_yard();
    if (!warehouse) {
        return;
    }
    
    building_storage_room* space = warehouse->room();
    while(space && amount > 0) {
        if (space->stored_full_amount <= 0) {
            continue;
        }

        e_resource resource = space->base.subtype.warehouse_resource_id;
        if (space->stored_full_amount > amount) {
            city_resource_remove_from_storageyard(resource, amount);
            space->stored_full_amount -= amount;
            amount = 0;
        } else {
            city_resource_remove_from_storageyard(resource, space->stored_full_amount);
            amount -= space->stored_full_amount;
            space->stored_full_amount = 0;
            space->base.subtype.warehouse_resource_id = RESOURCE_NONE;
        }
        space->set_image(resource);
        space = space->next_room();
    }
}

void building_storageyards_add_resource(e_resource resource, int amount) {
    int building_id = city_resource_last_used_storageyard();

    for (int i = 1; i < MAX_BUILDINGS && amount > 0; i++) {
        building_id++;
        if (building_id >= MAX_BUILDINGS) {
            building_id = 1;
        }

        auto warehouse = building_get(building_id)->dcast_storage_yard();
        if (warehouse && warehouse->is_valid()) {
            city_resource_set_last_used_storageyard(building_id);
            while (amount && warehouse->add_resource(resource, false, UNITS_PER_LOAD, /*force*/false)) {
                amount--;
            }
        }
    }
}

constexpr int FULL_WAREHOUSE = 3200;
constexpr int THREEQ_WAREHOUSE = 2400;
constexpr int HALF_WAREHOUSE = 1600;
constexpr int QUARTER_WAREHOUSE = 800;

bool building_storage_yard::is_accepting(e_resource resource) {
    const storage_t* s = storage();

    int amount = this->amount(resource);
    bool accepting = (s->resource_state[resource] == STORAGE_STATE_PHARAOH_ACCEPT);
    bool fool_storage = (s->resource_max_accept[resource] == FULL_WAREHOUSE);
    bool threeq_storage = (s->resource_max_accept[resource] >= THREEQ_WAREHOUSE && amount < THREEQ_WAREHOUSE);
    bool half_storage = (s->resource_max_accept[resource] >= HALF_WAREHOUSE && amount < HALF_WAREHOUSE);
    bool quart_storage = (s->resource_max_accept[resource] >= QUARTER_WAREHOUSE && amount < QUARTER_WAREHOUSE);

    return (accepting && (fool_storage || threeq_storage || half_storage || quart_storage));
}

bool building_storage_yard::is_getting(e_resource resource) {
    const storage_t* s = storage();
    int amount = this->amount(resource);
    if ((s->resource_state[resource] == STORAGE_STATE_PHARAOH_GET && s->resource_max_get[resource] == FULL_WAREHOUSE) 
        || (s->resource_state[resource] == STORAGE_STATE_PHARAOH_GET && s->resource_max_get[resource] >= THREEQ_WAREHOUSE && amount < THREEQ_WAREHOUSE / 100)
        || (s->resource_state[resource] == STORAGE_STATE_PHARAOH_GET && s->resource_max_get[resource] >= HALF_WAREHOUSE && amount < HALF_WAREHOUSE / 100)
        || (s->resource_state[resource] == STORAGE_STATE_PHARAOH_GET && s->resource_max_get[resource] >= QUARTER_WAREHOUSE && amount < QUARTER_WAREHOUSE / 100)) {
        return true;
    } else {
        return false;
    }
}

bool building_storage_yard::is_not_accepting(e_resource resource) {
    return !((is_accepting(resource) || is_getting(resource)));
}

bool building_storage_yard::draw_ornaments_and_animations_height(painter &ctx, vec2i point, tile2i tile, color color_mask) {
    building_draw_normal_anim(ctx, point, &base, tile, storage_yard_m.anim["work"], color_mask);
    ImageDraw::img_generic(ctx, storage_yard_m.anim["base"].first_img() + 17, point.x - 5, point.y - 42, color_mask);

    return true;
}

int building_storageyards_remove_resource(e_resource resource, int amount) {
    int amount_left = amount;
    int building_id = city_resource_last_used_storageyard();
    // first go for non-getting warehouses
    for (int i = 1; i < MAX_BUILDINGS && amount_left > 0; i++) {
        building_id++;
        if (building_id >= MAX_BUILDINGS) {
            building_id = 1;
        }

        building_storage_yard *warehouse = building_get(building_id)->dcast_storage_yard();
        if (warehouse && warehouse->is_valid()) {
            if (!warehouse->is_getting(resource)) {
                city_resource_set_last_used_storageyard(building_id);
                amount_left = warehouse->remove_resource(resource, amount_left);
            }
        }
    }
    // if that doesn't work, take it anyway
    for (int i = 1; i < MAX_BUILDINGS && amount_left > 0; i++) {
        building_id++;
        if (building_id >= MAX_BUILDINGS) {
            building_id = 1;
        }

        building_storage_yard* warehouse = building_get(building_id)->dcast_storage_yard();
        if (warehouse && warehouse->is_valid()) {
            city_resource_set_last_used_storageyard(building_id);
            amount_left = warehouse->remove_resource(resource, amount_left);
        }
    }
    return amount - amount_left;
}

int building_storage_yard_for_storing(tile2i tile, e_resource resource, int distance_from_entry, int road_network_id, int* understaffed, tile2i &dst) {
    int min_dist = 10000;
    int min_building_id = 0;
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building_storage_room* room = building_get(i)->dcast_storage_room();
        if (!room || !room->is_valid()) {
            continue;
        }

        if (!room->has_road_access() || room->base.distance_from_entry <= 0 || room->road_network() != road_network_id)
            continue;

        building_storage_yard* warehouse = room->yard();
        if (warehouse->is_not_accepting(resource) || warehouse->is_empty_all()) {
            continue;
        }

        if (!config_get(CONFIG_GP_CH_UNDERSTAFFED_ACCEPT_GOODS)) {
            int pct_workers = warehouse->pct_workers();
            if (pct_workers < 100) {
                if (understaffed)
                    *understaffed += 1;
                continue;
            }
        }

        int dist = room->distance_with_penalty(tile, resource, distance_from_entry);
        if (dist > 0 && dist < min_dist) {
            min_dist = dist;
            min_building_id = i;
        }
    }

    // abuse null building space
    building* b = building_get(min_building_id)->main();
    if (b->has_road_access == 1) {
        map_point_store_result(b->tile, dst);
    } else {
        dst = map_has_road_access_rotation(b->subtype.orientation, b->tile, 3);
        if (!dst.valid()) {
            return 0;
        }
    }

    return min_building_id;
}

int building_storage_yard::for_getting(e_resource resource, tile2i* dst) {
    int min_dist = 10000;
    building* min_building = 0;
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building_storage_yard *other_warehouse = building_get(i)->dcast_storage_yard();
        if (!other_warehouse || !other_warehouse->is_valid()) {
            continue;
        }

        if (i == id()) {
            continue;
        }

        int amounts_stored = 0;
        building_storage_room* space = other_warehouse->room();
        const storage_t* s = space->storage();
        while (space) {
            if (space->stored_full_amount > 0) {
                if (space->base.subtype.warehouse_resource_id == resource)
                    amounts_stored += space->stored_full_amount;
            }
            space = space->next_room();
        }

        if (amounts_stored > 0 && !other_warehouse->is_gettable(resource)) {
            int dist = calc_distance_with_penalty(other_warehouse->tile(), tile(), base.distance_from_entry, other_warehouse->base.distance_from_entry);
            dist -= 4 * (amounts_stored / 100);
            if (dist < min_dist) {
                min_dist = dist;
                min_building = &other_warehouse->base;
            }
        }
    }
    if (min_building) {
        if (dst) {
            map_point_store_result(min_building->road_access, *dst);
        }
        return min_building->id;
    } else
        return 0;
}

static bool determine_granary_accept_foods(resource_list &foods, int road_network) {
    if (scenario_property_kingdom_supplies_grain()) {
        return false;
    }

    foods.clear();
    buildings_valid_do([&] (building &b) {
        building_granary *granary = b.dcast_granary();
        assert(granary);
        if (!granary->has_road_access()) {
            return;
        }

        if (road_network != granary->road_network()) {
            return;
        }

        int pct_workers = granary->pct_workers();
        if (pct_workers < 100 || granary->amount(RESOURCE_NONE) < 1200) {
            return;
        }

        if (granary->is_empty_all()) {
            return;
        }

        for (const auto &r : resource_list::foods) {
            if (granary->is_accepting(r.type)) {
                foods[r.type] = 1;
            }
        }

    }, BUILDING_GRANARY);

    return foods.any();
}


static bool contains_non_stockpiled_food(building* space, const resource_list &foods) {
    if (space->id <= 0) {
        return false;
    }

    if (space->stored_full_amount <= 0) {
        return false;
    }

    e_resource resource = space->subtype.warehouse_resource_id;
    if (city_resource_is_stockpiled(resource)) {
        return false;
    }

    if (resource_type_any_of(resource, RESOURCE_GRAIN, RESOURCE_MEAT, RESOURCE_LETTUCE, RESOURCE_FIGS)) {
        return (foods[resource] > 0);
    }

    return false;
}

storage_worker_task building_storage_yard_determine_getting_up_resources(building* b) {
    building_storage_yard* warehouse = b->dcast_storage_yard();
    for (e_resource check_resource = RESOURCE_MIN; check_resource < RESOURCES_MAX; ++check_resource) {
        if (!warehouse->is_getting(check_resource) || city_resource_is_stockpiled(check_resource)) {
            continue;
        }

        int total_stored = 0; // total amounts of resource in warehouse!
        int room = 0;         // total potential room for resource!
        auto space = warehouse->room();
        while (space) {
            if (space->stored_full_amount <= 0) { // this space (tile) is empty! FREE REAL ESTATE
                room += 4;
            }

            if (space->base.subtype.warehouse_resource_id == check_resource) { // found a space (tile) with resource on it!
                total_stored += space->stored_full_amount;   // add loads to total, if any!
                room += 400 - space->stored_full_amount;     // add room to total, if any!
            }
            space = space->next_room();
        }

        int requesting = warehouse->accepting_amount(check_resource);
        int lacking = requesting - total_stored;

        // determine if there's enough room for more to accept, depending on "get up to..." settings!
        if (room >= 0 && lacking > 0 && city_resource_count(check_resource) - total_stored > 0) {
            if (!warehouse->for_getting(check_resource, 0)) { // any other place contain this resource..?
                continue;
            }

            // bug in original Pharaoh: warehouses send out two cartpushers even if there is no room!
            e_storageyard_task status = lacking > requesting / 2
                                            ? STORAGEYARD_TASK_GETTING_MOAR
                                            : STORAGEYARD_TASK_GETTING;
            return {status, &warehouse->base, lacking, check_resource};
        }
    }

    return {STORAGEYARD_TASK_NONE};
}

storage_worker_task building_storageyard_deliver_weapons(building *warehouse) {
    building *space = warehouse;

    if (building_count_active(BUILDING_RECRUITER) > 0 
        && (g_city.military.has_infantry_batalions() || config_get(CONFIG_GP_CH_RECRUITER_NOT_NEED_FORTS))
        && !city_resource_is_stockpiled(RESOURCE_WEAPONS)) {
        auto result = building_get_asker_for_resource(warehouse->tile, BUILDING_RECRUITER, RESOURCE_WEAPONS, warehouse->road_network_id, warehouse->distance_from_entry);
        building* barracks = building_get(result.building_id);
        int barracks_want = barracks->need_resource_amount(RESOURCE_WEAPONS);

        if (barracks_want > 0 && warehouse->road_network_id == barracks->road_network_id) {
            int available = 0;
            space = warehouse;
            for (int i = 0; i < 8; i++) {
                space = space->next();
                if (space->id > 0 && space->stored_full_amount > 0
                    && space->subtype.warehouse_resource_id == RESOURCE_WEAPONS) {
                    available += space->stored_full_amount;
                }
            }

            if (available > 0) {
                int amount = std::min(available, barracks_want);
                return {STORAGEYARD_TASK_DELIVERING, space, amount, RESOURCE_WEAPONS};
            }
        }
    }

    return {STORAGEYARD_TASK_NONE};
}

storage_worker_task building_storageyard_deliver_papyrus_to_scribal_school(building *warehouse) {
    building *space = warehouse;

    if (building_count_active(BUILDING_SCRIBAL_SCHOOL) > 0 
        //&& (config_get(CONFIG_GP_CH_SCRIBAL_SCHOOL_NEED_PAPYRUS))
        && !city_resource_is_stockpiled(RESOURCE_PAPYRUS)) {
        auto result = building_get_asker_for_resource(warehouse->tile, BUILDING_SCRIBAL_SCHOOL, RESOURCE_PAPYRUS, warehouse->road_network_id, warehouse->distance_from_entry);
        building* scribal_school = building_get(result.building_id);
        int school_want = scribal_school->need_resource_amount(RESOURCE_PAPYRUS);

        if (school_want > 0 && warehouse->road_network_id == scribal_school->road_network_id) {
            int available = 0;
            space = warehouse;
            for (int i = 0; i < 8; i++) {
                space = space->next();
                if (space->id > 0 && space->stored_full_amount > 0
                    && space->subtype.warehouse_resource_id == RESOURCE_PAPYRUS) {
                    available += space->stored_full_amount;
                }
            }

            if (available > 0) {
                int amount = std::min(available, school_want);
                return {STORAGEYARD_TASK_DELIVERING, space, amount, RESOURCE_PAPYRUS};
            }
        }
    }

    return {STORAGEYARD_TASK_NONE};
}

storage_worker_task building_storageyard_deliver_resource_to_workshop(building *warehouse) {
    building *space = warehouse;
    for (int i = 0; i < 8; i++) {
        space = space->next();
        if (space->id <= 0 || space->stored_full_amount <= 0) {
            continue;
        }

        e_resource check_resource = space->subtype.warehouse_resource_id;
        if (city_resource_is_stockpiled(check_resource)) {
            continue;
        }

        storage_worker_task task = {STORAGEYARD_TASK_NONE};
        buildings_workshop_do([&] (building &b) {
            if (!resource_required_by_workshop(&b, space->subtype.warehouse_resource_id) || b.need_resource_amount(check_resource) < 100) {
                return;
            }
            task = {STORAGEYARD_TASK_DELIVERING, space, 100, space->subtype.warehouse_resource_id};
        });

        if (task.result == STORAGEYARD_TASK_DELIVERING) {
            return task;
        }
    }

    return {STORAGEYARD_TASK_NONE};
}

storage_worker_task building_storage_yard::deliver_food_to_gettingup_granary(building *warehouse) {
    resource_list granary_resources;
    if (!g_city.determine_granary_get_foods(granary_resources, warehouse->road_network_id)) {
        return {STORAGEYARD_TASK_NONE};
    }

    building *space = warehouse;
    for (int i = 0; i < 8; i++) {
        space = space->next();
        if (contains_non_stockpiled_food(space, granary_resources)) {
            // always one load only for granaries?
            return {STORAGEYARD_TASK_DELIVERING, space, 100, space->subtype.warehouse_resource_id};
        }
    }

    return {STORAGEYARD_TASK_NONE};
}

storage_worker_task building_storageyard_deliver_food_to_accepting_granary(building *warehouse) {
    building *space = warehouse;

    resource_list granary_resources;
    if (determine_granary_accept_foods(granary_resources, warehouse->road_network_id)
        && !scenario_property_kingdom_supplies_grain()) {
        space = warehouse;
        for (int i = 0; i < 8; i++) {
            space = space->next();
            if (contains_non_stockpiled_food(space, granary_resources)) {
                // always one load only for granaries?
                return {STORAGEYARD_TASK_DELIVERING, space, 100, space->subtype.warehouse_resource_id};
            }
        }
    }

    return {STORAGEYARD_TASK_NONE};
}

storage_worker_task building_storageyard_deliver_to_monuments(building *warehouse) {
    building *space = warehouse;

    svector<building *, 10> monuments;
    buildings_valid_do([&] (building &b) { 
        if (!monuments.full() && building_is_monument(b.type) && b.data.monuments.phase != MONUMENT_FINISHED) {
            monuments.push_back(&b);
        }
    });

    if (monuments.empty()) {
        return {STORAGEYARD_TASK_NONE};
    }

    for (int i = 0; i < 8; i++) {
        space = space->next();
        int available = space->stored_full_amount;
        if (space->id <= 0 || !space->subtype.warehouse_resource_id || available <= 0) {
            continue;
        }

        e_resource resource = space->subtype.warehouse_resource_id;
        if (city_resource_is_stockpiled(resource)) {
            continue;
        }

        for (auto monument : monuments) {
            int monuments_want = building_monument_needs_resource(monument, resource);
            if (monuments_want > 0) {
                int amount = std::min(available, monuments_want);
                return {STORAGEYARD_TASK_MONUMENT, space, amount, resource, monument};
            }
        }
    }

    return {STORAGEYARD_TASK_NONE};
}

storage_worker_task building_storage_yard_deliver_emptying_resources(building *b) {
    building_storage_yard *warehouse = b->dcast_storage_yard();
    if (!warehouse) {
        return {STORAGEYARD_TASK_NONE};
    }

    for (e_resource r = RESOURCE_MIN; r < RESOURCES_MAX; ++r) {
        if (!warehouse->is_emptying(r)) {
            continue;
        }

        int in_storage = warehouse->amount(r);
        if (in_storage > 0) {
            return {STORAGEYARD_TASK_EMPTYING, nullptr, in_storage, r};
        }
    }

    return {STORAGEYARD_TASK_NONE};
}

storage_worker_task building_storage_yard::determine_worker_task() {
    // check workers - if less than enough, no task will be done today.
    if (pct_workers() < 50) {
        return {STORAGEYARD_TASK_NONE};
    }

    using action_type = decltype(building_storageyard_deliver_weapons);
    action_type *actions[] = {
        &building_storage_yard_determine_getting_up_resources,       // getting up resources
        &building_storageyard_deliver_weapons,                      // deliver weapons to barracks
        &building_storageyard_deliver_resource_to_workshop,         // deliver raw materials to workshops
        &building_storageyard_deliver_papyrus_to_scribal_school,    // deliver raw materials to workshops
        &building_storage_yard::deliver_food_to_gettingup_granary,    // deliver food to getting granary
        &building_storageyard_deliver_food_to_accepting_granary,    // deliver food to accepting granary
        &building_storage_yard_deliver_emptying_resources,           // emptying resource
        &building_storageyard_deliver_to_monuments,                 // monuments resource
    };

    for (const auto &action : actions) {
        storage_worker_task task = (*action)(&base);
        if (task.result != STORAGEYARD_TASK_NONE) {
            return task;
        }
    }
    
    // move goods to other warehouses
    if (is_empty_all()) {
        building *space = &base;
        for (int i = 0; i < 8; i++) {
            space = space->next();
            if (space->id > 0 && space->stored_full_amount > 0) {
                return {STORAGEYARD_TASK_DELIVERING, space, space->stored_full_amount, space->subtype.warehouse_resource_id};
            }
        }
    }

    return {STORAGEYARD_TASK_NONE};
}

void building_storage_yard::on_create(int orientation) {
    base.subtype.orientation = building_rotation_global_rotation();
}

building* building_storage_yard::add_storageyard_space(int x, int y, building* prev) {
    building* b = building_create(BUILDING_STORAGE_ROOM, tile2i(x, y), 0);
    game_undo_add_building(b);
    b->prev_part_building_id = prev->id;
    prev->next_part_building_id = b->id;
    map_building_tiles_add(b->id, tile2i(x, y), 1, image_id_from_group(GROUP_BUILDING_STORAGE_YARD_SPACE_EMPTY), TERRAIN_BUILDING);
    return b;
}

void building_storage_yard::on_place_update_tiles(int orientation, int variant) {
    tile2i offset[9] = {{0, 0}, {0, 1}, {1, 0}, {1, 1}, {0, 2}, {2, 0}, {1, 2}, {2, 1}, {2, 2}};
    int global_rotation = building_rotation_global_rotation();
    int corner = building_rotation_get_corner(2 * global_rotation);

    base.storage_id = building_storage_create(BUILDING_STORAGE_YARD);
    if (config_get(CONFIG_GP_CH_WAREHOUSES_DONT_ACCEPT)) {
        building_storage_accept_none(base.storage_id);
    }

    base.prev_part_building_id = 0;
    tile2i shifted_tile = tile().shifted(offset[corner]);

    int base_image = storage_yard_m.anim["base"].first_img();
    map_building_tiles_add(id(), shifted_tile, 1, base_image, TERRAIN_BUILDING);

    building* prev = &base;
    for (int i = 0; i < 9; i++) {
        if (i == corner) {
            continue;
        }
        prev = add_storageyard_space(tilex() + offset[i].x(), tiley() + offset[i].y(), prev);
    }

    base.tile = tile().shifted(offset[corner]);
    game_undo_adjust_building(&base);

    prev->next_part_building_id = 0;
}

void building_storage_yard::on_place_checks() {
    if (!map_has_road_access(tile(), 3)) {
        building_construction_warning_show(WARNING_ROAD_ACCESS_NEEDED);
    }
}

void building_storage_yard::spawn_figure() {
    base.check_labor_problem();
    if (!base.has_road_access) {
        return;
    }

    building *space = &base;
    for (int i = 0; i < 8; i++) {
        space = space->next();
        if (space->id) {
            space->show_on_problem_overlay = base.show_on_problem_overlay;
        }
    }

    base.common_spawn_labor_seeker(100);
    auto task = determine_worker_task();
    if (task.result == STORAGEYARD_TASK_NONE || task.amount <= 0) {
        return;
    }

    if (!base.has_figure(BUILDING_SLOT_SERVICE) && task.result == STORAGEYARD_TASK_MONUMENT) {
        figure *leader = base.create_figure_with_destination(FIGURE_SLED_PULLER, task.dest, FIGURE_ACTION_50_SLED_PULLER_CREATED);
        leader->set_direction_to(task.dest);
        for (int i = 0; i < 5; ++i) {
            figure *follower = base.create_figure_with_destination(FIGURE_SLED_PULLER, task.dest, FIGURE_ACTION_50_SLED_PULLER_CREATED);
            follower->set_direction_to(task.dest);
            follower->wait_ticks = i * 4;
        }

        figure *f = figure_create(FIGURE_SLED, base.tile, 0);
        figure_sled *sled = f->dcast_sled();
        sled->set_destination(task.dest);
        sled->set_direction_to(task.dest);
        sled->load_resource(task.resource, task.amount);
        sled->base.leading_figure_id = leader->id;
        remove_resource(task.resource, task.amount);

    } else if (!base.has_figure(BUILDING_SLOT_SERVICE)) {
        figure* f = figure_create(FIGURE_STORAGEYARD_CART, base.road_access, DIR_4_BOTTOM_LEFT);
        auto cart = f->dcast_storageyard_cart();
        assert(cart);
        if (!cart) {
            f->poof();
            return;
        }

        cart->advance_action(FIGURE_ACTION_50_WAREHOUSEMAN_CREATED);

        switch (task.result) {
        case STORAGEYARD_TASK_GETTING:
        case STORAGEYARD_TASK_GETTING_MOAR:
            cart->load_resource(RESOURCE_NONE, 0);
            cart->base.collecting_item_id = task.resource;
            break;

        case STORAGEYARD_TASK_DELIVERING:
        case STORAGEYARD_TASK_EMPTYING:
            task.amount = std::min<int>(task.amount, 400);
            cart->load_resource(task.resource, task.amount);
            remove_resource(task.resource, task.amount);
            break;
        }
        base.set_figure(0, f->id);
        f->set_home(base.id);

    } else if (task.result == STORAGEYARD_TASK_GETTING_MOAR && !base.has_figure_of_type(1, FIGURE_STORAGEYARD_CART)) {
        figure* f = figure_create(FIGURE_STORAGEYARD_CART, base.road_access, DIR_4_BOTTOM_LEFT);
        auto cart = f->dcast_storageyard_cart();
        cart->base.action_state = FIGURE_ACTION_50_WAREHOUSEMAN_CREATED;
        cart->load_resource(RESOURCE_NONE, 0);
        cart->base.collecting_item_id = task.resource;

        base.set_figure(1, cart->id());
        cart->set_home(base.id);
    }
}

building_storage_yard *storage_yard_cast(building *b) {
    auto yard = b->dcast_storage_yard();
    if (yard) {
        return yard;
    }

    auto room = b->dcast_storage_room();
    if (room) {
        return room->yard();
    }

    return nullptr;
}

void building_storage_yard::ghost_preview(vec2i tile, painter &ctx) {
    int global_rotation = building_rotation_global_rotation();
    int index_rotation = building_rotation_get_storage_fort_orientation(global_rotation);
    int corner = building_rotation_get_corner(index_rotation);
    vec2i corner_offset{-5, -45};
    vec2i place_offset{0, 0};

    int image_id_hut = storage_yard_m.anim["base"].first_img();
    int image_id_space = image_id_from_group(GROUP_BUILDING_STORAGE_YARD_SPACE_EMPTY);
    for (int i = 0; i < 9; i++) {
        if (i == corner) {
            draw_building_ghost(ctx, image_id_hut, tile + VIEW_OFFSETS[i]);
            ImageDraw::img_generic(ctx, image_id_hut + 17, tile.x + VIEW_OFFSETS[i].x + corner_offset.x, tile.y + VIEW_OFFSETS[i].y + corner_offset.y, COLOR_MASK_GREEN);
        } else {
            draw_building_ghost(ctx, image_id_space, tile + VIEW_OFFSETS[i] + place_offset);
        }
    }
}