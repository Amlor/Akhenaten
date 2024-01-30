#include "city_overlay_fertility.h"

#include "city_overlay.h"
#include "grid/floodplain.h"
#include "building/model.h"
#include "building/building.h"
#include "figure/figure.h"
#include "grid/property.h"
#include "grid/building.h"
#include "grid/terrain.h"
#include "grid/image.h"
#include "graphics/color.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "grid/point.h"
#include "game/state.h"

static int get_tooltip_fertility(tooltip_context* c, const building* b) {
    if (building_is_farm(b->type)) {
        map_point tile = b->tile;
        int fertility = map_get_fertility_for_farm(tile.grid_offset());
        if (fertility > 80)
            return 63;
        else if (fertility > 60)
            return 62;
        else if (fertility > 40)
            return 61;
        else if (fertility > 20)
            return 60;
        else if (fertility > 10)
            return 59;
        else {
            return 58;
        }
    }
    return 58;
}

static int get_column_height_fertility_none(const building* b) {
    return NO_COLUMN;
}

static int terrain_on_fertility_overlay(void) {
    return TERRAIN_TREE | TERRAIN_ROCK | TERRAIN_WATER | TERRAIN_ROAD
            | TERRAIN_ELEVATION | TERRAIN_ACCESS_RAMP | TERRAIN_RUBBLE | TERRAIN_CANAL | TERRAIN_WALL;
}

static int get_fertility_image_offset(int fertilty) {
    return (fertilty / 10);
}

struct city_overlay_fertility : public city_overlay {
    city_overlay_fertility() {
        type = OVERLAY_FERTILITY;
        column_type = COLUMN_TYPE_POSITIVE;

        get_column_height = get_column_height_fertility_none;
        get_tooltip_for_building = get_tooltip_fertility;   
    }

    bool show_figure(const figure* f) const override {
        if (f->type != FIGURE_CART_PUSHER) {
            return false;
        }

        if (f->sender_building_id == 0) {
            return false;
        }

        building *b = building_get(f->sender_building_id);
        return b && building_is_farm(b->type);
    }

    void draw_custom_top(vec2i pixel, tile2i point, painter &ctx) const override {
        int grid_offset = point.grid_offset();

        color color_mask = 0;
        if (map_terrain_is(grid_offset, terrain_on_fertility_overlay()) && !map_terrain_is(grid_offset, TERRAIN_BUILDING)) {
            // display normal tile
            //if (map_property_is_draw_tile(grid_offset)) {
                ImageDraw::isometric_from_drawtile(ctx, map_image_at(grid_offset), pixel, color_mask);
            //}
        } else if (map_terrain_is(grid_offset, TERRAIN_BUILDING) || map_terrain_is(grid_offset, TERRAIN_FLOODPLAIN)) {
            int fertility = map_get_fertility(grid_offset, FERT_WITH_MALUS);
            int offset = get_fertility_image_offset(fertility);
            ImageDraw::isometric_from_drawtile(ctx, image_id_from_group(GROUP_TERRAIN_DESIRABILITY) + offset, pixel, color_mask);
        } else {
            ImageDraw::isometric_from_drawtile(ctx, map_image_at(grid_offset), pixel, color_mask);
        }
    }

    bool show_building(const building* b) const override {
        return false;// building_is_farm(b->type);
    }
};

city_overlay_fertility g_city_overlay_fertility;

city_overlay* city_overlay_for_fertility() {
    return &g_city_overlay_fertility;
}
