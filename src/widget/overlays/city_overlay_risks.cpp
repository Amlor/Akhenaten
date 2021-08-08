#include <building/model.h>
#include "city_overlay_risks.h"

#include "building/industry.h"
#include "game/state.h"
#include "graphics/image.h"
#include "map/building.h"
#include "map/image.h"
#include "map/property.h"
#include "map/random.h"
#include "map/terrain.h"

static int is_problem_cartpusher(figure *fig) {
    if (fig->id > 0)
        return fig->action_state == FIGURE_ACTION_20_CARTPUSHER_INITIAL && fig->min_max_seen;
    else
        return 0;
}

void overlay_problems_prepare_building(building *b) {
    if (b->house_size)
        return;
    if (b->type == BUILDING_FOUNTAIN || b->type == BUILDING_BATHHOUSE) {
        if (!b->has_water_access)
            b->show_on_problem_overlay = 1;
    } else if (b->type >= BUILDING_WHEAT_FARM && b->type <= BUILDING_CLAY_PIT) {
        if (is_problem_cartpusher(b->get_figure(0)))
            b->show_on_problem_overlay = 1;
    } else if (building_is_workshop(b->type)) {
        if (is_problem_cartpusher(b->get_figure(0)))
            b->show_on_problem_overlay = 1;
        else if (b->loads_stored <= 0)
            b->show_on_problem_overlay = 1;
    } else if (b->state == BUILDING_STATE_MOTHBALLED)
        b->show_on_problem_overlay = 1;
}

static int show_building_crime(const building *b) {
    return b->type == BUILDING_POLICE_STATION || b->type == BUILDING_FESTIVAL_SQUARE;
}
static int show_building_fire(const building *b) {
    return b->type == BUILDING_FIREHOUSE || b->type == BUILDING_BURNING_RUIN || b->type == BUILDING_FESTIVAL_SQUARE;
}
static int show_building_damage(const building *b) {
    return b->type == BUILDING_ENGINEERS_POST || b->type == BUILDING_FESTIVAL_SQUARE;
}
static int show_building_problems(const building *b) {
    return b->show_on_problem_overlay;
}
static int show_building_native(const building *b) {
    return b->type == BUILDING_NATIVE_HUT || b->type == BUILDING_NATIVE_MEETING || b->type == BUILDING_MISSION_POST;
}

static int show_figure_fire(const figure *f) {
    return f->type == FIGURE_PREFECT;
}
static int show_figure_damage(const figure *f) {
    return f->type == FIGURE_ENGINEER;
}
static int show_figure_crime(const figure *f) {
    return f->type == FIGURE_POLICEMAN || f->type == FIGURE_PROTESTER ||
           f->type == FIGURE_CRIMINAL || f->type == FIGURE_RIOTER;
}
static int show_figure_problems(const figure *f) {
    if (f->type == FIGURE_LABOR_SEEKER)
        return ((figure*)f)->home()->show_on_problem_overlay;
    else if (f->type == FIGURE_CART_PUSHER)
        return f->action_state == FIGURE_ACTION_20_CARTPUSHER_INITIAL || f->min_max_seen;
    else {
        return 0;
    }
}
static int show_figure_native(const figure *f) {
    return f->type == FIGURE_INDIGENOUS_NATIVE || f->type == FIGURE_MISSIONARY;
}

static int get_column_height_fire(const building *b) {
    auto model = model_get_building(b->type);
    if (b->prev_part_building_id || !model->fire_risk)
        return NO_COLUMN;
    return b->fire_risk / 100;
}
static int get_column_height_damage(const building *b) {
    auto model = model_get_building(b->type);
    if (b->prev_part_building_id || !model->damage_risk)
        return NO_COLUMN;
    return b->damage_risk / 100;
}
static int get_column_height_crime(const building *b) {
    if (b->house_size) {
        int happiness = b->sentiment.house_happiness;
//        return (50 - happiness) / 5;
        if (happiness <= 0)
            return 10;
        else if (happiness <= 10)
            return 8;
        else if (happiness <= 20)
            return 6;
        else if (happiness <= 30)
            return 4;
        else if (happiness <= 40)
            return 2;
        else if (happiness < 50)
            return 1;
    }
    return NO_COLUMN;
}
static int get_column_height_none(const building *b) {
    return NO_COLUMN;
}

static int get_tooltip_fire(tooltip_context *c, const building *b) {
    if (b->fire_risk <= 0)
        return 46;
    else if (b->fire_risk <= 200)
        return 47;
    else if (b->fire_risk <= 400)
        return 48;
    else if (b->fire_risk <= 600)
        return 49;
    else if (b->fire_risk <= 800)
        return 50;
    else
        return 51;
}
static int get_tooltip_damage(tooltip_context *c, const building *b) {
    if (b->damage_risk <= 0)
        return 52;
    else if (b->damage_risk <= 40)
        return 53;
    else if (b->damage_risk <= 80)
        return 54;
    else if (b->damage_risk <= 120)
        return 55;
    else if (b->damage_risk <= 160)
        return 56;
    else {
        return 57;
    }
}
static int get_tooltip_crime(tooltip_context *c, const building *b) {
    if (b->sentiment.house_happiness <= 0)
        return 63;
    else if (b->sentiment.house_happiness <= 10)
        return 62;
    else if (b->sentiment.house_happiness <= 20)
        return 61;
    else if (b->sentiment.house_happiness <= 30)
        return 60;
    else if (b->sentiment.house_happiness < 50)
        return 59;
    else {
        return 58;
    }
}

const city_overlay *city_overlay_for_fire(void) {
    static city_overlay overlay = {
            OVERLAY_FIRE,
            COLUMN_TYPE_RISK,
            show_building_fire, //show_building_fire_crime
            show_figure_fire,
            get_column_height_fire,
            0,
            get_tooltip_fire,
            0,
            0
    };
    return &overlay;
}
const city_overlay *city_overlay_for_damage(void) {
    static city_overlay overlay = {
            OVERLAY_DAMAGE,
            COLUMN_TYPE_RISK,
            show_building_damage,
            show_figure_damage,
            get_column_height_damage,
            0,
            get_tooltip_damage,
            0,
            0
    };
    return &overlay;
}
const city_overlay *city_overlay_for_crime(void) {
    static city_overlay overlay = {
            OVERLAY_CRIME,
            COLUMN_TYPE_RISK,
            show_building_crime, //show_building_fire_crime
            show_figure_crime,
            get_column_height_crime,
            0,
            get_tooltip_crime,
            0,
            0
    };
    return &overlay;
}
const city_overlay *city_overlay_for_problems(void) {
    static city_overlay overlay = {
            OVERLAY_PROBLEMS,
            COLUMN_TYPE_RISK,
            show_building_problems,
            show_figure_problems,
            get_column_height_none,
            0,
            0,
            0,
            0
    };
    return &overlay;
}

static int terrain_on_native_overlay(void) {
    return
            TERRAIN_TREE | TERRAIN_ROCK | TERRAIN_WATER | TERRAIN_SHRUB |
            TERRAIN_GARDEN | TERRAIN_ELEVATION | TERRAIN_ACCESS_RAMP | TERRAIN_RUBBLE;
}
static void draw_footprint_native(int x, int y, int grid_offset) {
    if (!map_property_is_draw_tile(grid_offset))
        return;
    if (map_terrain_is(grid_offset, terrain_on_native_overlay())) {
        if (map_terrain_is(grid_offset, TERRAIN_BUILDING))
            city_with_overlay_draw_building_footprint(x, y, grid_offset, 0);
        else {
            ImageDraw::isometric_footprint_from_drawtile(map_image_at(grid_offset), x, y, 0);
        }
    } else if (map_terrain_is(grid_offset, TERRAIN_AQUEDUCT | TERRAIN_WALL)) {
        // display groundwater
        int image_id = image_id_from_group(GROUP_TERRAIN_EMPTY_LAND) + (map_random_get(grid_offset) & 7);
        ImageDraw::isometric_footprint_from_drawtile(image_id, x, y, 0);
    } else if (map_terrain_is(grid_offset, TERRAIN_BUILDING))
        city_with_overlay_draw_building_footprint(x, y, grid_offset, 0);
    else {
        if (map_property_is_native_land(grid_offset))
            ImageDraw::isometric_footprint_from_drawtile(image_id_from_group(GROUP_TERRAIN_DESIRABILITY) + 1, x, y, 0);
        else {
            ImageDraw::isometric_footprint_from_drawtile(map_image_at(grid_offset), x, y, 0);
        }
    }
}
static void draw_top_native(int x, int y, int grid_offset) {
    if (!map_property_is_draw_tile(grid_offset))
        return;
    if (map_terrain_is(grid_offset, terrain_on_native_overlay())) {
        if (!map_terrain_is(grid_offset, TERRAIN_BUILDING)) {
            color_t color_mask = 0;
            if (map_property_is_deleted(grid_offset) && map_property_multi_tile_size(grid_offset) == 1)
                color_mask = COLOR_MASK_RED;

            ImageDraw::isometric_top_from_drawtile(map_image_at(grid_offset), x, y, color_mask);
        }
    } else if (map_building_at(grid_offset))
        city_with_overlay_draw_building_top(x, y, grid_offset);

}

const city_overlay *city_overlay_for_native(void) {
    static city_overlay overlay = {
            OVERLAY_NATIVE,
            COLUMN_TYPE_RISK,
            show_building_native,
            show_figure_native,
            get_column_height_none,
            0,
            0,
            draw_footprint_native,
            draw_top_native
    };
    return &overlay;
}
