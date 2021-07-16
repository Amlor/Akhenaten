#include <cmath>
#include "cartpusher.h"

#include "building/barracks.h"
#include "building/granary.h"
#include "building/industry.h"
#include "building/storage.h"
#include "building/warehouse.h"
#include "city/resource.h"
#include "core/calc.h"
#include "core/config.h"
#include "core/image.h"
#include "figure/combat.h"
#include "figure/image.h"
#include "figure/movement.h"
#include "figure/route.h"
#include "game/resource.h"
#include "map/road_network.h"
#include "map/routing_terrain.h"

static const int CART_OFFSET_MULTIPLE_LOADS_FOOD[] = {0, 0, 8, 16, 0, 0, 24, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static const int CART_OFFSET_MULTIPLE_LOADS_NON_FOOD[] = {0, 0, 0, 0, 0, 8, 0, 16, 24, 32, 40, 48, 56, 64, 72, 80};
static const int CART_OFFSET_8_LOADS_FOOD[] = {0, 40, 48, 56, 0, 0, 64, 0, 0, 0, 0, 0, 0, 0, 0, 0};

#include "city/buildings.h"

void figure::set_resource(int resource) {
    resource_id = resource;
}
int figure::get_resource() const {
    return resource_id;
}
void figure::load_resource(int amount, int resource) {
    resource_id = resource;
    resource_amount_full = amount;
    resource_amount_loads = amount / 100;
}
int figure::unload_resource(int amount) {
    amount = fmin(amount, resource_amount_full);
    resource_amount_full -= amount;
    resource_amount_loads -= amount / 100;
    return resource_amount_full;
}
int figure::get_carrying_amount() {
    return resource_amount_full;
}

void figure::determine_deliveryman_destination(building *b, int road_network_id) {
    map_point dst;
    int understaffed_storages = 0;

    destination_x = 0;
    destination_y = 0;

    // first: gold deliverers
    if (resource_id == RESOURCE_GOLD) {
        int senate_id = city_buildings_get_senate_id();
        building *b = building_get(senate_id);
        if (senate_id && b->state == BUILDING_STATE_VALID && b->num_workers >= 5) {
            destination_building_id = senate_id;
            return advance_action(11);
        }
    }

    // priority 1: warehouse if resource is on stockpile
    destination_building_id = building_warehouse_for_storing(0, tile_x, tile_y, resource_id, b->distance_from_entry, road_network_id, &understaffed_storages, &dst);
    if (!city_resource_is_stockpiled(resource_id))
        destination_building_id = 0;
    if (destination_building_id)
        return advance_action(FIGURE_ACTION_21_CARTPUSHER_DELIVERING_TO_WAREHOUSE);

    // priority 2: accepting granary for food
    destination_building_id = building_granary_for_storing(tile_x, tile_y, resource_id, b->distance_from_entry, road_network_id, 0, &understaffed_storages, &dst);
    if (config_get(CONFIG_GP_CH_FARMS_DELIVER_CLOSE)) {
        int dist = 0;
        building *src_building = building_get(building_id);
        building *dst_building = building_get(destination_building_id);
        int src_int = src_building->type;
        if ((src_int >= BUILDING_WHEAT_FARM && src_int <= BUILDING_PIG_FARM) || src_int == BUILDING_WHARF)
            dist = calc_distance_with_penalty(src_building->x, src_building->y, dst_building->x, dst_building->y, src_building->distance_from_entry, dst_building->distance_from_entry);
        if (dist >= 64)
            destination_building_id = 0;
    }
    if (destination_building_id)
        return advance_action(FIGURE_ACTION_22_CARTPUSHER_DELIVERING_TO_GRANARY);

    // priority 3: workshop for raw material
    destination_building_id = building_get_workshop_for_raw_material_with_room(tile_x, tile_y, resource_id, b->distance_from_entry, road_network_id, &dst);
    if (destination_building_id)
        return advance_action(FIGURE_ACTION_23_CARTPUSHER_DELIVERING_TO_WORKSHOP);

    // priority 4: warehouse
    destination_building_id = building_warehouse_for_storing(0, tile_x, tile_y, resource_id, b->distance_from_entry, road_network_id, &understaffed_storages, &dst);
    if (destination_building_id)
        return advance_action(FIGURE_ACTION_21_CARTPUSHER_DELIVERING_TO_WAREHOUSE);

    // priority 5: granary forced when on stockpile
    destination_building_id = building_granary_for_storing(tile_x, tile_y, resource_id, b->distance_from_entry, road_network_id, 1, &understaffed_storages, &dst);
    if (config_get(CONFIG_GP_CH_FARMS_DELIVER_CLOSE)) {
        int dist = 0;
        building *src_building = building_get(building_id);
        building *dst_building = building_get(destination_building_id);
        int src_int = src_building->type;
        if ((src_int >= BUILDING_WHEAT_FARM && src_int <= BUILDING_PIG_FARM) || src_int == BUILDING_WHARF)
            dist = calc_distance_with_penalty(src_building->x, src_building->y, dst_building->x, dst_building->y, src_building->distance_from_entry, dst_building->distance_from_entry);
        if (dist >= 64)
            destination_building_id = 0;
    }
    if (destination_building_id)
        return advance_action(FIGURE_ACTION_22_CARTPUSHER_DELIVERING_TO_GRANARY);

    // no one will accept
    min_max_seen = understaffed_storages ? 2 : 1;
    advance_action(ACTION_8_RECALCULATE);
}
void figure::determine_deliveryman_destination_food(int road_network_id) {
    building *b = building_get(building_id);
    map_point dst;
    // priority 1: accepting granary for food
    int dst_building_id = building_granary_for_storing(tile_x, tile_y, resource_id, b->distance_from_entry, road_network_id, 0, 0, &dst);
    if (dst_building_id && config_get(CONFIG_GP_CH_FARMS_DELIVER_CLOSE)) {
        int dist = 0;
        building *dst_building = building_get(dst_building_id);
        if ((b->type >= BUILDING_WHEAT_FARM && b->type <= BUILDING_PIG_FARM) || b->type == BUILDING_WHARF)
            dist = calc_distance_with_penalty(b->x, b->y, dst_building->x, dst_building->y, b->distance_from_entry, dst_building->distance_from_entry);
        if (dist >= 64)
            dst_building_id = 0;
    }
    if (dst_building_id) {
        destination_building_id = dst_building_id;
        return advance_action(FIGURE_ACTION_22_CARTPUSHER_DELIVERING_TO_GRANARY);
    }
    // priority 2: warehouse
    dst_building_id = building_warehouse_for_storing(0, tile_x, tile_y, resource_id, b->distance_from_entry, road_network_id, 0, &dst);
    if (dst_building_id) {
        destination_building_id = dst_building_id;
        return advance_action(FIGURE_ACTION_21_CARTPUSHER_DELIVERING_TO_WAREHOUSE);
    }
    // priority 3: granary
    dst_building_id = building_granary_for_storing(tile_x, tile_y, resource_id, b->distance_from_entry, road_network_id, 1, 0, &dst);
    if (dst_building_id) {
        destination_building_id = dst_building_id;
        return advance_action(FIGURE_ACTION_22_CARTPUSHER_DELIVERING_TO_GRANARY);
    }
    // no one will accept, stand idle
    advance_action(ACTION_8_RECALCULATE);
}
void figure::determine_granaryman_destination( int road_network_id) {
    map_point dst;
//    int dst_building_id;
    building *granary = building_get(building_id);
    if (!resource_id) {
        // getting granaryman
        destination_building_id = building_granary_for_getting(granary, &dst);
        if (destination_building_id) {
            advance_action(FIGURE_ACTION_54_WAREHOUSEMAN_GETTING_FOOD);
//            set_destination(FIGURE_ACTION_54_WAREHOUSEMAN_GETTING_FOOD, dst_building_id, dst.x, dst.y);
            if (config_get(CONFIG_GP_CH_GETTING_GRANARIES_GO_OFFROAD))
                terrain_usage = TERRAIN_USAGE_PREFER_ROADS;
        } else
            poof();
        return;
    }
    // delivering resource
    // priority 1: another granary
    destination_building_id = building_granary_for_storing(tile_x, tile_y, resource_id, granary->distance_from_entry, road_network_id, 0, 0, &dst);
    if (destination_building_id) {
        building_granary_remove_resource(granary, resource_id, 100);
        return advance_action(FIGURE_ACTION_51_WAREHOUSEMAN_DELIVERING_RESOURCE);
    }
    // priority 2: warehouse
    destination_building_id = building_warehouse_for_storing(0, tile_x, tile_y, resource_id, granary->distance_from_entry, road_network_id, 0, &dst);
    if (destination_building_id) {
        building_granary_remove_resource(granary, resource_id, 100);
        return advance_action(FIGURE_ACTION_51_WAREHOUSEMAN_DELIVERING_RESOURCE);
    }
    // priority 3: granary even though resource is on stockpile
    destination_building_id = building_granary_for_storing(tile_x, tile_y, resource_id, granary->distance_from_entry, road_network_id, 1, 0, &dst);
    if (destination_building_id) {
        building_granary_remove_resource(granary, resource_id, 100);
        return advance_action(FIGURE_ACTION_51_WAREHOUSEMAN_DELIVERING_RESOURCE);
    }
    // nowhere to go to: recalculate
    advance_action(FIGURE_ACTION_56_WAREHOUSEMAN_RETURNING_WITH_FOOD);
}
void figure::determine_warehouseman_destination(int road_network_id) {
    map_point dst;

    ////// getting resources!
    if (!resource_id) {
        destination_building_id = building_warehouse_for_getting(building_get(building_id), collecting_item_id, &dst);
        if (destination_building_id) {
            advance_action(FIGURE_ACTION_57_WAREHOUSEMAN_GETTING_RESOURCE);
            terrain_usage = TERRAIN_USAGE_PREFER_ROADS;
        } else
            poof();
        return;
    }
    building *warehouse = building_get(building_id);

    ////// delivering resource!
    // priority 1: weapons to barracks
    destination_building_id = building_get_barracks_for_weapon(tile_x, tile_y, resource_id, road_network_id, warehouse->distance_from_entry, &dst);
    if (destination_building_id)
        return advance_action(FIGURE_ACTION_51_WAREHOUSEMAN_DELIVERING_RESOURCE);

    // priority 2: raw materials to workshop
    destination_building_id = building_get_workshop_for_raw_material_with_room(tile_x, tile_y, resource_id, warehouse->distance_from_entry, road_network_id, &dst);
    if (destination_building_id)
        return advance_action(FIGURE_ACTION_51_WAREHOUSEMAN_DELIVERING_RESOURCE);

    // priority 3: food to granary
    destination_building_id = building_granary_for_storing(tile_x, tile_y, resource_id, warehouse->distance_from_entry, road_network_id, 0, 0, &dst);
    if (destination_building_id)
        return advance_action(FIGURE_ACTION_51_WAREHOUSEMAN_DELIVERING_RESOURCE);

    // priority 4: food to getting granary
    destination_building_id = building_getting_granary_for_storing(tile_x, tile_y, resource_id, warehouse->distance_from_entry, road_network_id, &dst);
    if (destination_building_id)
        advance_action(FIGURE_ACTION_51_WAREHOUSEMAN_DELIVERING_RESOURCE);

    // priority 5: resource to other warehouse
    destination_building_id = building_warehouse_for_storing(building_id, tile_x, tile_y, resource_id, warehouse->distance_from_entry, road_network_id, 0, &dst);
    int empty_warehouse = building_storage_get(building_get(building_id)->storage_id)->empty_all; // deliver to another warehouse because this one is being emptied
    if (destination_building_id && empty_warehouse) {
        if (destination_building_id == building_id)
            poof();
        else
            advance_action(FIGURE_ACTION_51_WAREHOUSEMAN_DELIVERING_RESOURCE);
        return;
    }

//    // priority 6: raw material to well-stocked workshop
//    destination_building_id = building_get_workshop_for_raw_material(tile_x, tile_y, resource_id, warehouse->distance_from_entry, road_network_id, &dst);
//    if (destination_building_id) {
//        remove_resource_from_warehouse();
//        return advance_action(FIGURE_ACTION_51_WAREHOUSEMAN_DELIVERING_RESOURCE);
//    }

    // no destination: recalculate
    advance_action(FIGURE_ACTION_59_WAREHOUSEMAN_RETURNING_WITH_RESOURCE);
}

#include "city/finance.h"

void figure::cartpusher_action() {
    building *b = building_get(building_id);
    int road_network_id = map_road_network_get(grid_offset_figure);
    switch (action_state) {
        case ACTION_8_RECALCULATE:
        case FIGURE_ACTION_20_CARTPUSHER_INITIAL:
            destination_building_id = 0;
            anim_frame = 0;
            wait_ticks++;
            if (wait_ticks > 30) {
                wait_ticks = 0;
                determine_deliveryman_destination(b, road_network_id);
            }
            break;
        case ACTION_9_DELIVERING_GOODS:
            do_gotobuilding(destination_building_id, true, TERRAIN_USAGE_ROADS, ACTION_12_UNLOADING1, ACTION_8_RECALCULATE);
            break;
        case ACTION_10_DELIVERING_FOOD:
            do_gotobuilding(destination_building_id, true, TERRAIN_USAGE_ROADS, ACTION_13_UNLOADING2, ACTION_8_RECALCULATE);
            break;
        case 11: // delivering gold
            do_gotobuilding(destination_building_id, true, TERRAIN_USAGE_ROADS, 14, ACTION_8_RECALCULATE);
            break;
        case FIGURE_ACTION_21_CARTPUSHER_DELIVERING_TO_WAREHOUSE:
            do_gotobuilding(destination_building_id, true, TERRAIN_USAGE_ROADS, FIGURE_ACTION_24_CARTPUSHER_AT_WAREHOUSE, ACTION_8_RECALCULATE);
            break;
        case FIGURE_ACTION_22_CARTPUSHER_DELIVERING_TO_GRANARY:
            do_gotobuilding(destination_building_id, true, TERRAIN_USAGE_ROADS, FIGURE_ACTION_25_CARTPUSHER_AT_GRANARY, ACTION_8_RECALCULATE);
            break;
        case FIGURE_ACTION_23_CARTPUSHER_DELIVERING_TO_WORKSHOP:
            do_gotobuilding(destination_building_id, true, TERRAIN_USAGE_ROADS, FIGURE_ACTION_26_CARTPUSHER_AT_WORKSHOP, ACTION_8_RECALCULATE);
            break;
        case 12: // storage yard
        case 13: // granary
        case 14: // palace
        case FIGURE_ACTION_24_CARTPUSHER_AT_WAREHOUSE:
        case FIGURE_ACTION_25_CARTPUSHER_AT_GRANARY:
        case FIGURE_ACTION_26_CARTPUSHER_AT_WORKSHOP:
            wait_ticks++;
            if (wait_ticks >= 10) {
                wait_ticks = 0;
                building *dest = building_get(destination_building_id);
                if (dest->state != BUILDING_STATE_VALID)
                    advance_action(ACTION_8_RECALCULATE);
                bool delivery_check = false;
                int amount_left = fmin(get_carrying_amount(), 100);
                switch (dest->type) {
                    case BUILDING_WAREHOUSE:
                    case BUILDING_WAREHOUSE_SPACE:
                        delivery_check = (building_warehouse_add_resource(dest, resource_id) != -1);
                        break;
                    case BUILDING_GRANARY:
                        delivery_check = (building_granary_add_resource(dest, resource_id, true, amount_left) != -1);
                        break;
                    case BUILDING_BARRACKS:
                        building_barracks_add_weapon(dest);
                        delivery_check = true;
                        break;
                    case BUILDING_VILLAGE_PALACE:
                    case BUILDING_TOWN_PALACE:
                    case BUILDING_CITY_PALACE:
                        city_finance_process_gold_extraction(100);
                        delivery_check = true;
                        break;
                    default:
                        building_workshop_add_raw_material(dest);
                        delivery_check = true;
                        break;
                }
                if (delivery_check)
                    amount_left = unload_resource(100);
                if (!amount_left) {
                    advance_action(ACTION_15_RETURNING2);
                } else if (!delivery_check)
                    advance_action(ACTION_8_RECALCULATE);
            }
            anim_frame = 0;
            break;
//        case ACTION_11_RETURNING_EMPTY:
        case ACTION_15_RETURNING2:
        case FIGURE_ACTION_27_CARTPUSHER_RETURNING:
            if (building_is_floodplain_farm(b)) // do not return to floodplain farms
                poof();
            else
                do_returnhome();
            break;
    }
    cart_update_image();
}
void figure::warehouseman_action() {
    int road_network_id = map_road_network_get(grid_offset_figure);
    switch (action_state) {
        case ACTION_8_RECALCULATE:
        case FIGURE_ACTION_50_WAREHOUSEMAN_CREATED: {
            destination_building_id = 0;
            building *b = building_get(building_id);
            wait_ticks++;
            if (wait_ticks > 2) {
                wait_ticks = 0;
                if (building_get(building_id)->type == BUILDING_GRANARY)
                    determine_granaryman_destination(road_network_id);
                else
                    determine_warehouseman_destination(road_network_id);
            }
            anim_frame = 0;
            break;
        }
        case ACTION_9_DELIVERING_GOODS:
        case FIGURE_ACTION_51_WAREHOUSEMAN_DELIVERING_RESOURCE:
            do_gotobuilding(destination_building_id, true, TERRAIN_USAGE_ROADS, FIGURE_ACTION_52_WAREHOUSEMAN_AT_DELIVERY_BUILDING, ACTION_8_RECALCULATE);
            break;
        case ACTION_12_UNLOADING1:
        case FIGURE_ACTION_52_WAREHOUSEMAN_AT_DELIVERY_BUILDING:
            wait_ticks++;
            if (wait_ticks > 4) {
                bool delivery_check = false;
                building *dest = building_get(destination_building_id);
                if (dest->state != BUILDING_STATE_VALID || dest->figure_id != id)
                    advance_action(ACTION_8_RECALCULATE);
                int amount_left = fmin(get_carrying_amount(), 100);
                switch (dest->type) {
                    case BUILDING_GRANARY:
                        delivery_check = (building_granary_add_resource(dest, resource_id, 0, amount_left) != -1);
                        break;
                    case BUILDING_BARRACKS:
                        building_barracks_add_weapon(dest);
                        delivery_check = true;
                        break;
                    case BUILDING_WAREHOUSE:
                    case BUILDING_WAREHOUSE_SPACE:
                        delivery_check = (building_warehouse_add_resource(dest, resource_id) != -1);
                        break;
                    default: // workshop
                        if (dest->loads_stored < 2) {
                            building_workshop_add_raw_material(dest);
                            delivery_check = true; // temp
                        } else
                            advance_action(ACTION_8_RECALCULATE);
                        break;
                }
                if (delivery_check)
                    amount_left = unload_resource(100);
                if (!amount_left) {
                    advance_action(ACTION_11_RETURNING_EMPTY);
                    wait_ticks = 0;
                } else if (!delivery_check)
                    advance_action(ACTION_8_RECALCULATE);
            }
            anim_frame = 0;
            break;
        case ACTION_11_RETURNING_EMPTY:
        case FIGURE_ACTION_53_WAREHOUSEMAN_RETURNING_EMPTY:
            do_returnhome();
            break;
        case FIGURE_ACTION_54_WAREHOUSEMAN_GETTING_FOOD:
            do_gotobuilding(destination_building_id, true, TERRAIN_USAGE_ROADS, FIGURE_ACTION_55_WAREHOUSEMAN_AT_GRANARY, ACTION_8_RECALCULATE);
            break;
        case FIGURE_ACTION_55_WAREHOUSEMAN_AT_GRANARY:
            wait_ticks++;
            if (wait_ticks > 4) {
                int resource;
                int loads = building_granary_remove_for_getting_deliveryman(building_get(destination_building_id), building_get(building_id), &resource);
                load_resource(loads * 100, resource);
                advance_action(FIGURE_ACTION_56_WAREHOUSEMAN_RETURNING_WITH_FOOD);
            }
            anim_frame = 0;
            break;
        case ACTION_14_RETURNING_WITH_FOOD:
        case FIGURE_ACTION_56_WAREHOUSEMAN_RETURNING_WITH_FOOD:
        case FIGURE_ACTION_59_WAREHOUSEMAN_RETURNING_WITH_RESOURCE:
            // update graphic
            if (do_returnhome()) {
                building *b = building_get(building_id);
                bool delivery_check = false;
                int amount_left = fmin(get_carrying_amount(), 100);
                for (int i = 0; i < get_carrying_amount() / 100; i++) {
                    switch (b->type) {
                        case BUILDING_GRANARY:
                            delivery_check = (building_granary_add_resource(building_get(building_id), resource_id, 0, amount_left) != -1);
                            break;
                        case BUILDING_WAREHOUSE:
                        case BUILDING_WAREHOUSE_SPACE:
                            delivery_check = (building_warehouse_add_resource(building_get(building_id), resource_id) != -1);
                            break;
                    }
                }
                if (!delivery_check)
                    advance_action(ACTION_8_RECALCULATE);
            }
            break;
        case ACTION_15_GETTING_GOODS:
        case FIGURE_ACTION_57_WAREHOUSEMAN_GETTING_RESOURCE:
            do_gotobuilding(destination_building_id, true, TERRAIN_USAGE_ROADS, FIGURE_ACTION_58_WAREHOUSEMAN_AT_WAREHOUSE, ACTION_8_RECALCULATE);
            break;
        case FIGURE_ACTION_58_WAREHOUSEMAN_AT_WAREHOUSE:
            wait_ticks++;
            if (wait_ticks > 4) {
//                loads_counter = 0;
                if (get_carrying_amount() < 400 && building_warehouse_remove_resource(building_get(destination_building_id), collecting_item_id, 1) == 0)
                    load_resource(100, collecting_item_id);
                else { // made it so it goes through the loads bit by bit??
                    set_resource(collecting_item_id);
                    advance_action(FIGURE_ACTION_59_WAREHOUSEMAN_RETURNING_WITH_RESOURCE);
                }
            }
            anim_frame = 0;
            break;
    }
    cart_update_image();
}
