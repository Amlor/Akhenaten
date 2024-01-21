#include "resource.h"

#include "building/building.h"
#include "building/industry.h"
#include "building/model.h"
#include "building/storage_yard.h"
#include "city/data_private.h"
#include "core/calc.h"
#include "core/profiler.h"
#include "empire/empire_city.h"
#include "game/tutorial.h"
#include "grid/road_access.h"
#include "scenario/building.h"
#include "scenario/property.h"

struct available_data_t {
    resources_list resource_list;
    resources_list food_list;
    resources_list market_goods_list;
};

available_data_t g_available_data;

int city_resource_count(e_resource resource) {
    return city_data.resource.stored_in_warehouses[resource];
}

const resources_list* city_resource_get_available() {
    return &g_available_data.resource_list;
}
const resources_list* city_resource_get_available_foods() {
    return &g_available_data.food_list;
}
const resources_list* city_resource_get_available_market_goods() {
    return &g_available_data.market_goods_list;
}

int city_resource_multiple_wine_available() {
    return city_data.resource.wine_types_available >= 2;
}

int city_resource_food_types_available() {
    return city_data.resource.food_types_available_num;
}
int city_resource_food_stored() {
    return city_data.resource.granary_total_stored;
}
int city_resource_food_needed() {
    return city_data.resource.food_needed_per_month;
}
int city_resource_food_supply_months() {
    return city_data.resource.food_supply_months;
}
int city_resource_food_percentage_produced() {
    return calc_percentage(city_data.resource.food_produced_last_month, city_data.resource.food_consumed_last_month);
}

int city_resource_operating_granaries() {
    return city_data.resource.granaries.operating;
}
int city_resource_last_used_storageyard() {
    return city_data.resource.last_used_warehouse;
}
void city_resource_set_last_used_storageyard(int warehouse_id) {
    city_data.resource.last_used_warehouse = warehouse_id;
}

int city_resource_trade_status(e_resource resource) {
    return city_data.resource.trade_status[resource];
}

void city_resource_cycle_trade_status(e_resource resource) {
    ++city_data.resource.trade_status[resource];
    if (city_data.resource.trade_status[resource] > TRADE_STATUS_EXPORT)
        city_data.resource.trade_status[resource] = TRADE_STATUS_NONE;

    if (city_data.resource.trade_status[resource] == TRADE_STATUS_IMPORT
        && !empire_can_import_resource(resource, true)) {
        city_data.resource.trade_status[resource] = TRADE_STATUS_EXPORT;
    }

    if (city_data.resource.trade_status[resource] == TRADE_STATUS_EXPORT
        && !empire_can_export_resource(resource, true)) {
        city_data.resource.trade_status[resource] = TRADE_STATUS_NONE;
    }

    if (city_data.resource.trade_status[resource] == TRADE_STATUS_EXPORT) {
        city_data.resource.stockpiled[resource] = 0;
    }
}

void city_resource_cycle_trade_import(e_resource resource) {
    // no sellers?
    if (!empire_can_import_resource(resource, true))
        return;

    switch (city_data.resource.trade_status[resource]) {
    default:
        city_data.resource.trade_status[resource] = TRADE_STATUS_IMPORT_AS_NEEDED;
        break;
    case TRADE_STATUS_IMPORT_AS_NEEDED:
        city_data.resource.trade_status[resource] = TRADE_STATUS_IMPORT;
        break;
    case TRADE_STATUS_IMPORT:
        city_data.resource.trade_status[resource] = TRADE_STATUS_NONE;
        break;
    }
}

void city_resource_cycle_trade_export(e_resource resource) {
    // no buyers?
    if (!empire_can_export_resource(resource, true))
        return;

    switch (city_data.resource.trade_status[resource]) {
    default:
        city_data.resource.trade_status[resource] = TRADE_STATUS_EXPORT_SURPLUS;
        city_data.resource.stockpiled[resource] = false;
        break;
    case TRADE_STATUS_EXPORT_SURPLUS:
        city_data.resource.trade_status[resource] = TRADE_STATUS_EXPORT;
        city_data.resource.stockpiled[resource] = false;
        break;
    case TRADE_STATUS_EXPORT:
        city_data.resource.trade_status[resource] = TRADE_STATUS_NONE;
        break;
    }
}
int city_resource_trading_amount(e_resource resource) {
    return city_data.resource.trading_amount[resource];
}
void city_resource_change_trading_amount(e_resource resource, int delta) {
    city_data.resource.trading_amount[resource] = calc_bound(city_data.resource.trading_amount[resource] + delta, 0, 100);
}

int city_resource_is_stockpiled(e_resource resource) {
    return city_data.resource.stockpiled[resource];
}

int city_resource_ready_for_using(e_resource resource) {
    int amount = 0;
    buildings_valid_do([&] (building &b) {
        if (city_resource_is_stockpiled(resource)) {
            return;
        }

        amount += building_storageyard_get_amount(&b, resource);
    }, BUILDING_STORAGE_YARD);

    return amount;
}

void city_resource_toggle_stockpiled(e_resource resource) {
    if (city_data.resource.stockpiled[resource])
        city_data.resource.stockpiled[resource] = 0;
    else {
        city_data.resource.stockpiled[resource] = 1;
        if (city_data.resource.trade_status[resource] == TRADE_STATUS_EXPORT
            || city_data.resource.trade_status[resource] == TRADE_STATUS_EXPORT_SURPLUS)
            city_data.resource.trade_status[resource] = TRADE_STATUS_NONE;
    }
}

int city_resource_is_mothballed(e_resource resource) {
    return city_data.resource.mothballed[resource];
}

void city_resource_toggle_mothballed(e_resource resource) {
    city_data.resource.mothballed[resource] = city_data.resource.mothballed[resource] ? 0 : 1;
}

void city_resource_add_produced_to_granary(int amount) {
    city_data.resource.food_produced_this_month += amount;
}
void city_resource_remove_from_granary(int food, int amount) {
    city_data.resource.granary_food_stored[food] -= amount;
}
void city_resource_add_to_storageyard(e_resource resource, int amount) {
    city_data.resource.space_in_warehouses[resource] -= amount;
    city_data.resource.stored_in_warehouses[resource] += amount;
}
void city_resource_remove_from_storageyard(e_resource resource, int amount) {
    city_data.resource.space_in_warehouses[resource] += amount;
    city_data.resource.stored_in_warehouses[resource] -= amount;
}

void city_resource_calculate_storageyard_stocks() {
    OZZY_PROFILER_SECTION("Game/Run/Tick/Warehouse Stocks Update");
    for (int i = 0; i < RESOURCES_MAX; i++) {
        city_data.resource.space_in_warehouses[i] = 0;
        city_data.resource.stored_in_warehouses[i] = 0;
    }

    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building* b = building_get(i);
        if (b->state == BUILDING_STATE_VALID && b->type == BUILDING_STORAGE_YARD) {
            b->has_road_access = 0;
            if (map_has_road_access_rotation(b->subtype.orientation, b->tile, b->size, 0)) {
                b->has_road_access = 1;
            } else if (map_has_road_access_rotation(b->subtype.orientation, b->tile, 3, 0)) {
                b->has_road_access = 2;
            }
        }
    }

    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building* b = building_get(i);
        if (b->state != BUILDING_STATE_VALID || b->type != BUILDING_STORAGE_YARD_SPACE)
            continue;

        building* warehouse = b->main();
        if (warehouse->has_road_access) {
            b->has_road_access = warehouse->has_road_access;
            if (b->subtype.warehouse_resource_id) {
                int amounts = b->stored_full_amount;
                int resource = b->subtype.warehouse_resource_id;
                city_data.resource.stored_in_warehouses[resource] += amounts;
                city_data.resource.space_in_warehouses[resource] += 400 - amounts;
            } else {
                city_data.resource.space_in_warehouses[RESOURCE_NONE] += 4;
            }
        }
    }
}

void city_resource_determine_available() {
    for (int i = 0; i < RESOURCES_MAX; i++) {
        g_available_data.resource_list.items[i] = RESOURCE_NONE;
        g_available_data.food_list.items[i] = RESOURCE_NONE;
    }
    g_available_data.resource_list.size = 0;
    g_available_data.food_list.size = 0;
    g_available_data.market_goods_list.size = 0;

    for (e_resource i = RESOURCE_FOOD_MIN; i < RESOURCES_FOODS_MAX; i = (e_resource)(i + 1)) {
        if (empire_can_produce_resource(i, true) || empire_can_import_resource(i, false)) {
            const int food_index = g_available_data.food_list.size;
            g_available_data.food_list.items[food_index] = i;
            g_available_data.food_list.size++;

            const int good_index = g_available_data.market_goods_list.size;
            g_available_data.market_goods_list.items[good_index] = i;
            g_available_data.market_goods_list.size++;
        }
    }
    for (e_resource i = RESOURCE_MIN; i < RESOURCES_MAX; i = (e_resource)(i + 1)) {
        if (empire_can_produce_resource(i, true) || empire_can_import_resource(i, false)) {
            g_available_data.resource_list.items[g_available_data.resource_list.size++] = i;
            switch (i) {
            case RESOURCE_POTTERY:
            case RESOURCE_BEER:
            case RESOURCE_LINEN:
            case RESOURCE_LUXURY_GOODS:
                g_available_data.market_goods_list.items[g_available_data.market_goods_list.size++] = i;
                break;
            }
        }
    }
}
static void calculate_available_food() {
    for (int i = 0; i < RESOURCES_FOODS_MAX; i++) {
        city_data.resource.granary_food_stored[i] = 0;
    }
    city_data.resource.granary_total_stored = 0;
    city_data.resource.food_types_available_num = 0;
    city_data.resource.food_supply_months = 0;
    city_data.resource.granaries.operating = 0;
    city_data.resource.granaries.understaffed = 0;
    city_data.resource.granaries.not_operating = 0;
    city_data.resource.granaries.not_operating_with_food = 0;
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building* b = building_get(i);
        if (b->state != BUILDING_STATE_VALID || b->type != BUILDING_GRANARY)
            continue;

        b->has_road_access = 0;
        if (map_has_road_access(b->tile, b->size)) { // map_has_road_access_granary(b->tile.x(), b->tile.y(), 0)
            b->has_road_access = 1;
            int pct_workers = calc_percentage<int>(b->num_workers, model_get_building(b->type)->laborers);
            if (pct_workers < 100) {
                city_data.resource.granaries.understaffed++;
            }

            int amount_stored = 0;
            for (int r = RESOURCE_FOOD_MIN; r < RESOURCES_FOODS_MAX; r++) {
                amount_stored += b->data.granary.resource_stored[r];
            }

            if (pct_workers < 50) {
                city_data.resource.granaries.not_operating++;
                if (amount_stored > 0)
                    city_data.resource.granaries.not_operating_with_food++;

            } else {
                city_data.resource.granaries.operating++;
                for (int r = 0; r < RESOURCES_FOODS_MAX; r++)
                    city_data.resource.granary_food_stored[r] += b->data.granary.resource_stored[r];

                if (amount_stored >= 100)
                    tutorial_on_filled_granary(amount_stored);
            }
        }
    }
    for (int i = RESOURCE_FOOD_MIN; i < RESOURCES_FOODS_MAX; i++) {
        if (city_data.resource.granary_food_stored[i]) {
            city_data.resource.granary_total_stored += city_data.resource.granary_food_stored[i];
            city_data.resource.food_types_available_num++;
        }
    }
    city_data.resource.food_needed_per_month = calc_adjust_with_percentage(city_data.population.population, 50);
    if (city_data.resource.food_needed_per_month > 0) {
        city_data.resource.food_supply_months
          = city_data.resource.granary_total_stored / city_data.resource.food_needed_per_month;
    } else {
        city_data.resource.food_supply_months = city_data.resource.granary_total_stored > 0 ? 1 : 0;
    }
    if (scenario_property_kingdom_supplies_grain()) {
        //        city_data.resource.food_types_available = 1;
        city_data.resource.food_supply_months = 12;
    }
}

void city_resource_calculate_food_stocks_and_supply_wheat() {
    OZZY_PROFILER_SECTION("Game/Run/Tick/Food Stocks Update");
    calculate_available_food();
    if (scenario_property_kingdom_supplies_grain()) {
        for (int i = 1; i < MAX_BUILDINGS; i++) {
            building* b = building_get(i);
            if (b->state == BUILDING_STATE_VALID && b->type == BUILDING_BAZAAR)
                b->data.market.inventory[0] = 200;
        }
    }
}

void city_resource_consume_food() {
    calculate_available_food();
    city_data.resource.food_types_eaten_num = 0;
    city_data.unused.unknown_00c0 = 0;
    int total_consumed = 0;
    buildings_house_do([&total_consumed] (building &b) {
        if (!b.house_size) {
            return;
        }

        int num_types = model_get_house(b.subtype.house_level)->food_types;
        short amount_per_type = calc_adjust_with_percentage<short>(b.house_population, 35);
        if (num_types > 1) {
            amount_per_type /= num_types;
        }

        b.data.house.num_foods = 0;
        if (scenario_property_kingdom_supplies_grain()) {
            city_data.resource.food_types_eaten_num = 1;
            b.data.house.inventory[0] = amount_per_type;
            b.data.house.num_foods = 1;
            return;
        } 
        
        if (num_types <= 0) {
            return;
        }

        for (int t = INVENTORY_MIN_FOOD; t < INVENTORY_MAX_FOOD && b.data.house.num_foods < num_types; t++) {
            if (b.data.house.inventory[t] >= amount_per_type) {
                b.data.house.inventory[t] -= amount_per_type;
                b.data.house.num_foods++;
                total_consumed += amount_per_type;
            } else if (b.data.house.inventory[t]) {
                // has food but not enough
                b.data.house.inventory[t] = 0;
                b.data.house.num_foods++;
                total_consumed += amount_per_type;
            }

            if (b.data.house.num_foods > city_data.resource.food_types_eaten_num) {
                city_data.resource.food_types_eaten_num = b.data.house.num_foods;
            }
        }
    });

    city_data.resource.food_consumed_last_month = total_consumed;
    city_data.resource.food_produced_last_month = city_data.resource.food_produced_this_month;
    city_data.resource.food_produced_this_month = 0;
}

void city_resource_add_items(e_resource res, int amount) {
    building* chosen_yard = nullptr;
    int lowest_stock_found = 10000;
    buildings_valid_do([&] (building &b) {
        int total_stored = building_storageyard_get_amount(&b, res);
        int free_space = building_storageyard_get_freespace(&b, res);
        
        if (free_space >= amount && total_stored < lowest_stock_found) {
            lowest_stock_found = total_stored;
            chosen_yard = &b;
        }
    }, BUILDING_STORAGE_YARD);

    bool storage_found = false;
    int lowest_resource_found = 10000;
    if (chosen_yard == nullptr) {
        return;
    }

    for (int i = 0; i < 6; ++i) {
        int stored = building_storageyard_get_amount(chosen_yard, res);
        if (stored >= 0 && stored < lowest_resource_found) {
            lowest_resource_found = stored;
            storage_found = true;
        }
    }

    if (storage_found) {
        building_storageyard_add_resource(chosen_yard, res, amount); // because I'm lazy.
    }
}
