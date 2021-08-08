#include "city_overlay_other.h"

#include "building/model.h"
#include "city/constants.h"
#include "city/finance.h"
#include "core/calc.h"
#include "core/config.h"
#include "game/resource.h"
#include "game/state.h"
#include "graphics/image.h"
#include "map/building.h"
#include "map/desirability.h"
#include "map/image.h"
#include "map/property.h"
#include "map/random.h"
#include "map/terrain.h"

static int show_building_religion(const building *b) {
    return
            b->type == BUILDING_ORACLE || b->type == BUILDING_SMALL_TEMPLE_CERES ||
            b->type == BUILDING_SMALL_TEMPLE_NEPTUNE || b->type == BUILDING_SMALL_TEMPLE_MERCURY ||
            b->type == BUILDING_SMALL_TEMPLE_MARS || b->type == BUILDING_SMALL_TEMPLE_VENUS ||
            b->type == BUILDING_LARGE_TEMPLE_CERES || b->type == BUILDING_LARGE_TEMPLE_NEPTUNE ||
            b->type == BUILDING_LARGE_TEMPLE_MERCURY || b->type == BUILDING_LARGE_TEMPLE_MARS ||
            b->type == BUILDING_LARGE_TEMPLE_VENUS;
}
static int show_building_food_stocks(const building *b) {
    return b->type == BUILDING_MARKET || b->type == BUILDING_WHARF || b->type == BUILDING_GRANARY;
}
static int show_building_tax_income(const building *b) {
    return b->type == BUILDING_FORUM || b->type == BUILDING_SENATE_UPGRADED;
}
static int show_building_water(const building *b) {
    return b->type == BUILDING_WELL || b->type == BUILDING_FOUNTAIN || b->type == BUILDING_RESERVOIR || b->type == BUILDING_WATER_SUPPLY;
}
static int show_building_desirability(const building *b) {
    return 0;
}

static int show_figure_religion(const figure *f) {
    return f->type == FIGURE_PRIEST;
}
static int show_figure_food_stocks(const figure *f) {
    if (f->type == FIGURE_MARKET_BUYER || f->type == FIGURE_MARKET_TRADER ||
        f->type == FIGURE_DELIVERY_BOY || f->type == FIGURE_FISHING_BOAT) {
        return 1;
    } else if (f->type == FIGURE_CART_PUSHER)
        return resource_is_food(f->get_resource());

    return 0;
}
static int show_figure_tax_income(const figure *f) {
    return f->type == FIGURE_TAX_COLLECTOR;
}
static int show_figure_water(const figure *f) {
    return f->type == FIGURE_WATER_CARRIER;
}
static int show_figure_none(const figure *f) {
    return 0;
}

static int get_column_height_religion(const building *b) {
    return b->house_size && b->data.house.num_gods ? b->data.house.num_gods * 17 / 10 : NO_COLUMN;
}
static int get_column_height_food_stocks(const building *b) {
    if (b->house_size && model_get_house(b->subtype.house_level)->food_types) {
        int pop = b->house_population;
        int stocks = 0;
        for (int i = INVENTORY_MIN_FOOD; i < INVENTORY_MAX_FOOD; i++)
            stocks += b->data.house.inventory[i];
        int pct_stocks = calc_percentage(stocks, pop);
        if (pct_stocks <= 0)
            return 10;
        else if (pct_stocks < 100)
            return 5;
        else if (pct_stocks <= 200)
            return 1;
    }
    return NO_COLUMN;
}
static int get_column_height_tax_income(const building *b) {
    if (b->house_size) {
        int pct = calc_adjust_with_percentage(b->tax_income_or_storage / 2, city_finance_tax_percentage());
        if (pct > 0)
            return pct / 25;
    }
    return NO_COLUMN;
}
static int get_column_height_water(const building *b) {
    return b->house_size ? b->data.house.bathhouse * 17 / 10 : NO_COLUMN;
}
static int get_column_height_none(const building *b) {
    return NO_COLUMN;
}

static void add_god(tooltip_context *c, int god_id) {
    int index = c->num_extra_values;
    c->extra_value_text_groups[index] = 59;
    c->extra_value_text_ids[index] = 11 + god_id;
    c->num_extra_values++;
}

static int get_tooltip_religion(tooltip_context *c, const building *b) {
    if (b->data.house.num_gods < 5) {
        if (b->data.house.temple_ceres)
            add_god(c, GOD_CERES);

        if (b->data.house.temple_neptune)
            add_god(c, GOD_NEPTUNE);

        if (b->data.house.temple_mercury)
            add_god(c, GOD_MERCURY);

        if (b->data.house.temple_mars)
            add_god(c, GOD_MARS);

        if (b->data.house.temple_venus)
            add_god(c, GOD_VENUS);

    }
    if (b->data.house.num_gods <= 0)
        return 12;
    else if (b->data.house.num_gods == 1)
        return 13;
    else if (b->data.house.num_gods == 2)
        return 14;
    else if (b->data.house.num_gods == 3)
        return 15;
    else if (b->data.house.num_gods == 4)
        return 16;
    else if (b->data.house.num_gods == 5)
        return 17;
    else {
        return 18; // >5 gods, shouldn't happen...
    }
}
static int get_tooltip_food_stocks(tooltip_context *c, const building *b) {
    if (b->house_population <= 0)
        return 0;

    if (!model_get_house(b->subtype.house_level)->food_types)
        return 104;
    else {
        int stocks_present = 0;
        for (int i = INVENTORY_MIN_FOOD; i < INVENTORY_MAX_FOOD; i++) {
            stocks_present += b->data.house.inventory[i];
        }
        int stocks_per_pop = calc_percentage(stocks_present, b->house_population);
        if (stocks_per_pop <= 0)
            return 4;
        else if (stocks_per_pop < 100)
            return 5;
        else if (stocks_per_pop <= 200)
            return 6;
        else {
            return 7;
        }
    }
}
static int get_tooltip_tax_income(tooltip_context *c, const building *b) {
    int denarii = calc_adjust_with_percentage(b->tax_income_or_storage / 2, city_finance_tax_percentage());
    if (denarii > 0) {
        c->has_numeric_prefix = 1;
        c->numeric_prefix = denarii;
        return 45;
    } else if (b->house_tax_coverage > 0)
        return 44;
    else {
        return 43;
    }
}
static int get_tooltip_water(tooltip_context *c, int grid_offset) {
    if (map_terrain_is(grid_offset, TERRAIN_GROUNDWATER)) {
        if (map_terrain_is(grid_offset, TERRAIN_FOUNTAIN_RANGE))
            return 2;
        else {
            return 1;
        }
    } else if (map_terrain_is(grid_offset, TERRAIN_FOUNTAIN_RANGE))
        return 3;

    return 0;
}
static int get_tooltip_desirability(tooltip_context *c, int grid_offset) {
    int desirability = map_desirability_get(grid_offset);
    if (desirability < 0)
        return 91;
    else if (desirability == 0)
        return 92;
    else {
        return 93;
    }
}

const city_overlay *city_overlay_for_religion(void) {
    static city_overlay overlay = {
            OVERLAY_RELIGION,
            COLUMN_TYPE_WATER_ACCESS,
            show_building_religion,
            show_figure_religion,
            get_column_height_religion,
            0,
            get_tooltip_religion,
            0,
            0
    };
    return &overlay;
}
const city_overlay *city_overlay_for_food_stocks(void) {
    static city_overlay overlay = {
            OVERLAY_FOOD_STOCKS,
            COLUMN_TYPE_RISK,
            show_building_food_stocks,
            show_figure_food_stocks,
            get_column_height_food_stocks,
            0,
            get_tooltip_food_stocks,
            0,
            0
    };
    return &overlay;
}
const city_overlay *city_overlay_for_tax_income(void) {
    static city_overlay overlay = {
            OVERLAY_TAX_INCOME,
            COLUMN_TYPE_WATER_ACCESS,
            show_building_tax_income,
            show_figure_tax_income,
            get_column_height_tax_income,
            0,
            get_tooltip_tax_income,
            0,
            0
    };
    return &overlay;
}

static int has_deleted_building(int grid_offset) {
    if (!config_get(CONFIG_UI_VISUAL_FEEDBACK_ON_DELETE))
        return 0;

    building *b = building_at(grid_offset);
    b = b->main();
    return b->id && (b->is_deleted || map_property_is_deleted(b->grid_offset));
}

static int terrain_on_water_overlay(void) {
    return
            TERRAIN_TREE | TERRAIN_ROCK | TERRAIN_WATER | TERRAIN_SHRUB |
            TERRAIN_GARDEN | TERRAIN_ROAD | TERRAIN_AQUEDUCT | TERRAIN_ELEVATION |
            TERRAIN_ACCESS_RAMP | TERRAIN_RUBBLE | TERRAIN_DUNE | TERRAIN_REEDS;
}
static void draw_footprint_water(int x, int y, int grid_offset) {
    // roads, bushes, dunes, etc. are drawn normally
    if (map_terrain_is(grid_offset, terrain_on_water_overlay())) {
        // (except for roadblocks on roads, draw these as flattened tiles)
        if (building_at(grid_offset)->type == BUILDING_ROADBLOCK)
            city_with_overlay_draw_building_footprint(x, y, grid_offset, 0);
        else if (map_property_is_draw_tile(grid_offset))
            ImageDraw::isometric_footprint_from_drawtile(map_image_at(grid_offset), x, y, 0);
    }
    else {
        int terrain = map_terrain_get(grid_offset);
        building *b = building_at(grid_offset);
        // draw houses, wells and water supplies either fully or flattened
        if (terrain & TERRAIN_BUILDING &&
            (building_is_house(b->type))
            || b->type == BUILDING_WELL
            || b->type == BUILDING_WATER_SUPPLY) {
            if (map_property_is_draw_tile(grid_offset))
                city_with_overlay_draw_building_footprint(x, y, grid_offset, 0);
        } else {
            // draw groundwater levels
            int image_id = image_id_from_group(GROUP_TERRAIN_OVERLAY_WATER);
            switch (map_terrain_get(grid_offset) & (TERRAIN_GROUNDWATER | TERRAIN_FOUNTAIN_RANGE)) {
                case TERRAIN_GROUNDWATER | TERRAIN_FOUNTAIN_RANGE:
                case TERRAIN_FOUNTAIN_RANGE:
                    image_id += 2;
                    break;
                case TERRAIN_GROUNDWATER:
                    image_id += 1;
                    break;
            }
            ImageDraw::isometric_footprint(image_id, x, y, 0);
        }
    }
}
static void draw_top_water(int x, int y, int grid_offset) {
    if (!map_property_is_draw_tile(grid_offset))
        return;
    if (map_terrain_is(grid_offset, terrain_on_water_overlay())) {
        if (!map_terrain_is(grid_offset, TERRAIN_BUILDING)) {
            color_t color_mask = 0;
            if (map_property_is_deleted(grid_offset) && map_property_multi_tile_size(grid_offset) == 1)
                color_mask = COLOR_MASK_RED;
            ImageDraw::isometric_top_from_drawtile(map_image_at(grid_offset), x, y, color_mask);
        }
    } else if (map_building_at(grid_offset))
        city_with_overlay_draw_building_top(x, y, grid_offset);
}
const city_overlay *city_overlay_for_water(void) {
    static city_overlay overlay = {
            OVERLAY_WATER,
            COLUMN_TYPE_WATER_ACCESS,
            show_building_water,
            show_figure_water,
            get_column_height_water,
            get_tooltip_water,
            0,
            draw_footprint_water,
            draw_top_water
    };
    return &overlay;
}

static int terrain_on_desirability_overlay(void) {
    return
            TERRAIN_TREE | TERRAIN_ROCK | TERRAIN_WATER |
            TERRAIN_SHRUB | TERRAIN_GARDEN | TERRAIN_ROAD |
            TERRAIN_ELEVATION | TERRAIN_ACCESS_RAMP | TERRAIN_RUBBLE;
}
static int get_desirability_image_offset(int desirability) {
    if (desirability < -10)
        return 0;
    else if (desirability < -5)
        return 1;
    else if (desirability < 0)
        return 2;
    else if (desirability == 1)
        return 3;
    else if (desirability < 5)
        return 4;
    else if (desirability < 10)
        return 5;
    else if (desirability < 15)
        return 6;
    else if (desirability < 20)
        return 7;
    else if (desirability < 25)
        return 8;
    else
        return 9;
}
static void draw_footprint_desirability(int x, int y, int grid_offset) {
    color_t color_mask = map_property_is_deleted(grid_offset) ? COLOR_MASK_RED : 0;
    if (map_terrain_is(grid_offset, terrain_on_desirability_overlay()) &&
        !map_terrain_is(grid_offset, TERRAIN_BUILDING)) {
        // display normal tile
        if (map_property_is_draw_tile(grid_offset))
            ImageDraw::isometric_footprint_from_drawtile(map_image_at(grid_offset), x, y, color_mask);

    } else if (map_terrain_is(grid_offset, TERRAIN_AQUEDUCT | TERRAIN_WALL)) {
        // display empty land/groundwater
        int image_id = image_id_from_group(GROUP_TERRAIN_EMPTY_LAND) + (map_random_get(grid_offset) & 7);
        ImageDraw::isometric_footprint_from_drawtile(image_id, x, y, color_mask);
    } else if (map_terrain_is(grid_offset, TERRAIN_BUILDING) || map_desirability_get(grid_offset)) {
        if (has_deleted_building(grid_offset))
            color_mask = COLOR_MASK_RED;

        int offset = get_desirability_image_offset(map_desirability_get(grid_offset));
        ImageDraw::isometric_footprint_from_drawtile(image_id_from_group(GROUP_TERRAIN_DESIRABILITY) + offset, x, y,
                                                     color_mask);
    } else
        ImageDraw::isometric_footprint_from_drawtile(map_image_at(grid_offset), x, y, color_mask);
}
static void draw_top_desirability(int x, int y, int grid_offset) {
    color_t color_mask = map_property_is_deleted(grid_offset) ? COLOR_MASK_RED : 0;
    if (map_terrain_is(grid_offset, terrain_on_desirability_overlay()) &&
        !map_terrain_is(grid_offset, TERRAIN_BUILDING)) {
        // display normal tile
        if (map_property_is_draw_tile(grid_offset))
            ImageDraw::isometric_top_from_drawtile(map_image_at(grid_offset), x, y, color_mask);

    } else if (map_terrain_is(grid_offset, TERRAIN_AQUEDUCT | TERRAIN_WALL)) {
        // groundwater, no top needed
    } else if (map_terrain_is(grid_offset, TERRAIN_BUILDING) || map_desirability_get(grid_offset)) {
        if (has_deleted_building(grid_offset))
            color_mask = COLOR_MASK_RED;

        int offset = get_desirability_image_offset(map_desirability_get(grid_offset));
        ImageDraw::isometric_top_from_drawtile(image_id_from_group(GROUP_TERRAIN_DESIRABILITY) + offset, x, y,
                                               color_mask);
    } else
        ImageDraw::isometric_top_from_drawtile(map_image_at(grid_offset), x, y, color_mask);
}
const city_overlay *city_overlay_for_desirability(void) {
    static city_overlay overlay = {
            OVERLAY_DESIRABILITY,
            COLUMN_TYPE_WATER_ACCESS,
            show_building_desirability,
            show_figure_none,
            get_column_height_none,
            get_tooltip_desirability,
            0,
            draw_footprint_desirability,
            draw_top_desirability
    };
    return &overlay;
}
