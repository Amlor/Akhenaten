#include "building_bazaar.h"

#include "figure/figure.h"
#include "building/storage.h"
#include "building/building_type.h"
#include "building/building_storage_yard.h"
#include "graphics/elements/ui.h"
#include "city/city.h"
#include "city/labor.h"
#include "core/calc.h"
#include "game/resource.h"
#include "scenario/scenario.h"
#include "grid/desirability.h"
#include "grid/building_tiles.h"
#include "grid/terrain.h"
#include "grid/routing/routing.h"
#include "graphics/image.h"
#include "window/building/distribution.h"
#include "graphics/graphics.h"
#include "game/game.h"
#include "widget/city/ornaments.h"
#include "building/count.h"

#include <numeric>

buildings::model_t<building_bazaar> bazaar_m;

ANK_REGISTER_CONFIG_ITERATOR(config_load_building_bazaar);
void config_load_building_bazaar() {
    bazaar_m.load();
}

struct resource_data {
    int building_id;
    int min_distance;
    int num_buildings;

    void update_food(int resource, building &b, int distance) {
        if (!resource) {
            return;
        }

        building_granary *granary = b.dcast_granary();
        if (!granary) {
            return;
        }

        if (b.data.granary.resource_stored[resource] < 100) {
            return;
        }

        num_buildings++;
        if (distance < min_distance) {
            min_distance = distance;
            building_id = b.id;
        }
    }

    void update_good(e_resource resource, building &b, int distance) {
        if (city_resource_is_stockpiled(resource)) {
            return;
        }

        building_storage_yard *warehouse = b.dcast_storage_yard();
        if (!warehouse) {
            return;
        }

        if (warehouse->amount(resource) <= 0) {
            return;
        }

        num_buildings++;
        if (distance < min_distance) {
            min_distance = distance;
            building_id = b.id;
        }
    }
};

int building_bazaar::max_food_stock() {
    auto it = std::max_element(data.market.inventory + INVENTORY_MIN_FOOD, data.market.inventory + INVENTORY_MAX_FOOD);
    return *it;
}

int building_bazaar::max_goods_stock() {
    auto it = std::max_element(data.market.inventory + INVENTORY_MIN_GOOD, data.market.inventory + INVENTORY_MAX_GOOD);
    return *it;
}

bool building_bazaar::is_good_accepted(int index) {
    int goods_bit = 1 << index;
    return !(base.subtype.market_goods & goods_bit);
}

void building_bazaar::toggle_good_accepted(int index) {
    int goods_bit = (1 << index);
    base.subtype.market_goods ^= goods_bit;
}

void building_bazaar::unaccept_all_goods() {
    base.subtype.market_goods = (short)0xFFFF;
}

building *building_bazaar::get_storage_destination() {
    resource_data resources[INVENTORY_MAX];

    std::fill(std::begin(resources), std::end(resources), resource_data{0, 40, 0});
    buildings_valid_do([&] (building &b) {
        if (!b.has_road_access || b.distance_from_entry <= 0 || b.road_network_id != base.road_network_id) {
            return;
        }

        if (!building_storage_get_permission(BUILDING_STORAGE_PERMISSION_MARKET, &b)) {
            return;
        }

        int distance = calc_maximum_distance(base.tile, b.tile);
        if (distance >= 40) {
            return;
        }

        if (b.type == BUILDING_GRANARY) {
            if (scenario_property_kingdom_supplies_grain()) {
                return;
            }

            // todo: fetch map available foods?
            resources[0].update_food(g_city.allowed_foods(0), b, distance);
            resources[1].update_food(g_city.allowed_foods(1), b, distance);
            resources[2].update_food(g_city.allowed_foods(2), b, distance);
            resources[3].update_food(g_city.allowed_foods(3), b, distance);

        } else if (b.type == BUILDING_STORAGE_YARD) {
            // goods
            resources[INVENTORY_GOOD1].update_good(RESOURCE_POTTERY, b, distance);
            resources[INVENTORY_GOOD2].update_good(RESOURCE_LUXURY_GOODS, b, distance);
            resources[INVENTORY_GOOD3].update_good(RESOURCE_LINEN, b, distance);
            resources[INVENTORY_GOOD4].update_good(RESOURCE_BEER, b, distance);
        }
    }, BUILDING_GRANARY, BUILDING_STORAGE_YARD);

    // update demands
    if (data.market.pottery_demand)
        data.market.pottery_demand--;
    else
        resources[INVENTORY_GOOD1].num_buildings = 0;

    if (data.market.furniture_demand)
        data.market.furniture_demand--;
    else
        resources[INVENTORY_GOOD2].num_buildings = 0;

    if (data.market.oil_demand)
        data.market.oil_demand--;
    else
        resources[INVENTORY_GOOD3].num_buildings = 0;

    if (data.market.wine_demand)
        data.market.wine_demand--;
    else
        resources[INVENTORY_GOOD4].num_buildings = 0;

    int can_go = 0;
    for (int i = 0; i < INVENTORY_MAX; i++) {
        if (resources[i].num_buildings) {
            can_go = 1;
            break;
        }
    }

    if (!can_go) {
        return building_get(0);
    }

    // prefer food if we don't have it
    if (!data.market.inventory[0] && resources[0].num_buildings && is_good_accepted(0)) {
        data.market.fetch_inventory_id = 0;
        return building_get(resources[0].building_id);

    } else if (!data.market.inventory[1] && resources[1].num_buildings && is_good_accepted(1)) {
        data.market.fetch_inventory_id = 1;
        return building_get(resources[1].building_id);

    } else if (!data.market.inventory[2] && resources[2].num_buildings && is_good_accepted(2)) {
        data.market.fetch_inventory_id = 2;
        return building_get(resources[2].building_id);

    } else if (!data.market.inventory[3] && resources[3].num_buildings && is_good_accepted(3)) {
        data.market.fetch_inventory_id = 3;
        return building_get(resources[3].building_id);
    }

    // then prefer resource if we don't have it
    if (!data.market.inventory[INVENTORY_GOOD1] && resources[INVENTORY_GOOD1].num_buildings && is_good_accepted(INVENTORY_GOOD1)) {
        data.market.fetch_inventory_id = INVENTORY_GOOD1;
        return building_get(resources[INVENTORY_GOOD1].building_id);

    } else if (!data.market.inventory[INVENTORY_GOOD2] && resources[INVENTORY_GOOD2].num_buildings && is_good_accepted(INVENTORY_GOOD2)) {
        data.market.fetch_inventory_id = INVENTORY_GOOD2;
        return building_get(resources[INVENTORY_GOOD2].building_id);

    } else if (!data.market.inventory[INVENTORY_GOOD3] && resources[INVENTORY_GOOD3].num_buildings && is_good_accepted(INVENTORY_GOOD3)) {
        data.market.fetch_inventory_id = INVENTORY_GOOD3;
        return building_get(resources[INVENTORY_GOOD3].building_id);

    } else if (!data.market.inventory[INVENTORY_GOOD4] && resources[INVENTORY_GOOD4].num_buildings && is_good_accepted(INVENTORY_GOOD4)) {
        data.market.fetch_inventory_id = INVENTORY_GOOD4;
        return building_get(resources[INVENTORY_GOOD4].building_id);
    }

    // then prefer smallest stock below 50
    int min_stock = 50;
    int fetch_inventory = -1;
    if (resources[0].num_buildings && data.market.inventory[0] < min_stock && is_good_accepted(0)) {
        min_stock = data.market.inventory[0];
        fetch_inventory = 0;
    }

    if (resources[1].num_buildings && data.market.inventory[1] < min_stock && is_good_accepted(1)) {
        min_stock = data.market.inventory[1];
        fetch_inventory = 1;
    }

    if (resources[2].num_buildings && data.market.inventory[2] < min_stock && is_good_accepted(2)) {
        min_stock = data.market.inventory[2];
        fetch_inventory = 2;
    }

    if (resources[3].num_buildings && data.market.inventory[3] < min_stock && is_good_accepted(3)) {
        min_stock = data.market.inventory[3];
        fetch_inventory = 3;
    }

    if (resources[INVENTORY_GOOD1].num_buildings && data.market.inventory[INVENTORY_GOOD1] < min_stock && is_good_accepted(INVENTORY_GOOD1)) {
        min_stock = data.market.inventory[INVENTORY_GOOD1];
        fetch_inventory = INVENTORY_GOOD1;
    }

    if (resources[INVENTORY_GOOD2].num_buildings && data.market.inventory[INVENTORY_GOOD2] < min_stock && is_good_accepted(INVENTORY_GOOD2)) {
        min_stock = data.market.inventory[INVENTORY_GOOD2];
        fetch_inventory = INVENTORY_GOOD2;
    }

    if (resources[INVENTORY_GOOD3].num_buildings && data.market.inventory[INVENTORY_GOOD3] < min_stock && is_good_accepted(INVENTORY_GOOD3)) {
        min_stock = data.market.inventory[INVENTORY_GOOD3];
        fetch_inventory = INVENTORY_GOOD3;
    }

    if (resources[INVENTORY_GOOD4].num_buildings && data.market.inventory[INVENTORY_GOOD4] < min_stock && is_good_accepted(INVENTORY_GOOD4)) {
        fetch_inventory = INVENTORY_GOOD4;
    }

    if (fetch_inventory == -1) {
        // all items well stocked: pick food below threshold
        if (resources[0].num_buildings && data.market.inventory[0] < 600 && is_good_accepted(0)) {
            fetch_inventory = 0;
        }
        if (resources[1].num_buildings && data.market.inventory[1] < 400 && is_good_accepted(1)) {
            fetch_inventory = 1;
        }
        if (resources[2].num_buildings && data.market.inventory[2] < 400 && is_good_accepted(2)) {
            fetch_inventory = 2;
        }
        if (resources[3].num_buildings && data.market.inventory[3] < 400 && is_good_accepted(3)) {
            fetch_inventory = 3;
        }
    }

    if (fetch_inventory < 0) {
        return building_get(0);
    }

    data.market.fetch_inventory_id = fetch_inventory;
    return building_get(resources[fetch_inventory].building_id);
}

void building_bazaar::update_graphic() {
    if (base.state != BUILDING_STATE_VALID) {
        return;
    }

    base.fancy_state = (g_desirability.get(base.tile) <= 30) ? efancy_normal : efancy_good;
    pcstr animkey = (base.fancy_state == efancy_normal) ? "base" : "fancy";
    const animation_t &anim = bazaar_m.anim[animkey];
    map_building_tiles_add(base.id, base.tile, base.size, anim.first_img(), TERRAIN_BUILDING);
}

void building_bazaar::on_create(int orientation) {
    base.subtype.market_goods = 0;
    base.fancy_state = efancy_normal;
}

void building_bazaar::spawn_figure() {
    base.check_labor_problem();

    if (common_spawn_figure_trigger(50)) {
        // market buyer
        int spawn_delay = base.figure_spawn_timer();
        if (!base.has_figure_of_type(1, FIGURE_MARKET_BUYER)) {
            base.figure_spawn_delay++;
            if (base.figure_spawn_delay > spawn_delay) {
                building *dest = get_storage_destination();
                if (dest->id) {
                    base.figure_spawn_delay = 0;
                    figure *f = base.create_figure_with_destination(FIGURE_MARKET_BUYER, dest, FIGURE_ACTION_145_MARKET_BUYER_GOING_TO_STORAGE, BUILDING_SLOT_MARKET_BUYER);
                    f->collecting_item_id = data.market.fetch_inventory_id;
                }
            }
        }

        // market trader
        if (!base.has_figure_of_type(0, FIGURE_MARKET_TRADER)) {
            int bazar_inventory = std::accumulate(data.market.inventory, data.market.inventory + 7, 0);
            if (bazar_inventory > 0) { // do not spawn trader if bazaar is 100% empty!
                base.figure_spawn_delay++;
                if (base.figure_spawn_delay > spawn_delay) {
                    base.figure_spawn_delay = 0;
                    base.create_roaming_figure(FIGURE_MARKET_TRADER);
                    return;
                }
            }
        }
    }
}

int building_bazaar::handle_mouse_simple(const mouse* m, object_info &c) {
    auto &data = g_window_building_distribution;
    return generic_buttons_handle_mouse(m, {c.offset.x + 80, c.offset.y + 16 * c.bgsize.x - 34}, data.go_to_orders_button.data(), 1, &data.focus_button_id);
}

int building_bazaar::handle_mouse_orders(const mouse* m, object_info &c) {
    auto &data = g_window_building_distribution;
    int y_offset = window_building_get_vertical_offset(&c, 28 - 11);
    data.resource_focus_button_id = 0;

    // resources
    const auto &resources = city_resource_get_available_market_goods();
    data.building_id = c.building_id;
    return generic_buttons_handle_mouse(m, {c.offset.x + 205, y_offset + 46}, data.orders_resource_buttons.data(), resources.size(), &data.resource_focus_button_id);
}

int building_bazaar::window_info_handle_mouse(const mouse *m, object_info &c) {
    if (c.storage_show_special_orders) {
        return handle_mouse_orders(m, c);
    }

    return handle_mouse_simple(m, c);
}

bool building_bazaar::draw_ornaments_and_animations_height(painter &ctx, vec2i point, tile2i tile, color color_mask) {
    pcstr animkey = (base.fancy_state == efancy_normal) ? "base_work" : "fancy_work";
    const animation_t &anim = bazaar_m.anim[animkey];
    building_draw_normal_anim(ctx, point, &base, tile, anim, color_mask);

    return true;
}
