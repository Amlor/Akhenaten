#include "tile_draw.h"

#include "dev/debug.h"
#include "core/svector.h"
#include "building/construction/build_planner.h"
#include "graphics/view/view.h"
#include "building/monuments.h"
#include "game/state.h"
#include "graphics/graphics.h"
#include "grid/building.h"
#include "grid/figure.h"
#include "grid/image.h"
#include "grid/property.h"
#include "grid/random.h"
#include "grid/terrain.h"
#include "config/config.h"
#include "ornaments.h"
#include "sound/sound_city.h"
#include "game/game.h"
#include "graphics/view/lookup.h"

#include "overlays/city_overlay.h"
#include "overlays/city_overlay_risks.h"

static const int ADJACENT_OFFSETS_PH[2][4][7]
  = {{{GRID_OFFSET(-1, 0), GRID_OFFSET(-1, -1), GRID_OFFSET(-1, -2), GRID_OFFSET(0, -2), GRID_OFFSET(1, -2)},
      {GRID_OFFSET(0, -1), GRID_OFFSET(1, -1), GRID_OFFSET(2, -1), GRID_OFFSET(2, 0), GRID_OFFSET(2, 1)},
      {GRID_OFFSET(1, 0), GRID_OFFSET(1, 1), GRID_OFFSET(1, 2), GRID_OFFSET(0, 2), GRID_OFFSET(-1, 2)},
      {GRID_OFFSET(0, 1), GRID_OFFSET(-1, 1), GRID_OFFSET(-2, 1), GRID_OFFSET(-2, 0), GRID_OFFSET(-2, -1)}},
     {{GRID_OFFSET(-1, 0),
       GRID_OFFSET(-1, -1),
       GRID_OFFSET(-1, -2),
       GRID_OFFSET(-1, -3),
       GRID_OFFSET(0, -3),
       GRID_OFFSET(1, -3),
       GRID_OFFSET(2, -3)},
      {GRID_OFFSET(0, -1),
       GRID_OFFSET(1, -1),
       GRID_OFFSET(2, -1),
       GRID_OFFSET(3, -1),
       GRID_OFFSET(3, 0),
       GRID_OFFSET(3, 1),
       GRID_OFFSET(3, 2)},
      {GRID_OFFSET(1, 0),
       GRID_OFFSET(1, 1),
       GRID_OFFSET(1, 2),
       GRID_OFFSET(1, 3),
       GRID_OFFSET(0, 3),
       GRID_OFFSET(-1, 3),
       GRID_OFFSET(-2, 3)},
      {GRID_OFFSET(0, 1),
       GRID_OFFSET(-1, 1),
       GRID_OFFSET(-2, 1),
       GRID_OFFSET(-3, 1),
       GRID_OFFSET(-3, 0),
       GRID_OFFSET(-3, -1),
       GRID_OFFSET(-3, -2)}}};


grid_xx g_render_grid = {0,
                          {
                             FS_UINT32, // c3
                             FS_UINT32  // ph
                          }
};

void map_render_clear() {
    map_grid_clear(&g_render_grid);
}

bool map_render_is(int grid_offset, int render_mask) {
    return map_grid_is_valid_offset(grid_offset) && !!(map_grid_get(&g_render_grid, grid_offset) & render_mask);
}

bool map_render_is(tile2i tile, int render_mask) {
    return map_render_is(tile.grid_offset(), render_mask);
}

void map_render_set(tile2i tile, int flag) {
    map_grid_set(&g_render_grid, tile.grid_offset(), flag);
}

void map_render_set(int grid_offset, int flag) {
    map_grid_set(&g_render_grid, grid_offset, flag);
}

enum e_figure_draw_mode { e_figure_draw_common = 0, e_figure_draw_overlay = 1 };

struct draw_context_t {
    time_millis last_water_animation_time;
    int advance_water_animation;

    int image_id_water_first;
    int image_id_water_last;

    int image_id_deepwater_first;
    int image_id_deepwater_last;

    int selected_figure_id;
    int highlighted_formation;
    vec2i* selected_figure_coord;
};

draw_context_t g_draw_context;

draw_context_t& get_draw_context() {
    return g_draw_context;
}

void init_draw_context(int selected_figure_id, vec2i* figure_coord, int highlighted_formation) {
    g_draw_context.advance_water_animation = 0;
    if (!selected_figure_id) {
        time_millis now = time_get_millis();
        if (now - g_draw_context.last_water_animation_time > 60) {
            g_draw_context.last_water_animation_time = now;
            g_draw_context.advance_water_animation = 1;
        }
    }
    g_draw_context.image_id_water_first = image_id_from_group(GROUP_TERRAIN_WATER);
    g_draw_context.image_id_water_last = 5 + g_draw_context.image_id_water_first;
    g_draw_context.image_id_deepwater_first = image_id_from_group(GROUP_TERRAIN_DEEPWATER);
    g_draw_context.image_id_deepwater_last = 89 + g_draw_context.image_id_deepwater_first;
    g_draw_context.selected_figure_id = selected_figure_id;
    g_draw_context.selected_figure_coord = figure_coord;
    g_draw_context.highlighted_formation = highlighted_formation;
}

bool drawing_building_as_deleted(building* b) {
    if (!config_get(CONFIG_UI_VISUAL_FEEDBACK_ON_DELETE)) {
        return false;
    }

    b = b->main();
    if (b->id && (b->is_deleted || map_property_is_deleted(b->tile))) {
        return true;
    }

    return false;
}

static bool is_multi_tile_terrain(int grid_offset) {
    return (!map_building_at(grid_offset) && map_property_multi_tile_size(grid_offset) > 1);
}

static bool has_adjacent_deletion(int grid_offset) {
    int size = map_property_multi_tile_size(grid_offset);
    int total_adjacent_offsets = size * 2 + 1;
    const int* adjacent_offset; // = ADJACENT_OFFSETS[size - 2][city_view_orientation() / 2];

    adjacent_offset = ADJACENT_OFFSETS_PH[size - 2][city_view_orientation() / 2];
    for (int i = 0; i < total_adjacent_offsets; ++i) {
        if (map_property_is_deleted(grid_offset + adjacent_offset[i])
            || drawing_building_as_deleted(building_at(grid_offset + adjacent_offset[i]))) {
            return true;
        }
    }
    return false;
}

static bool is_drawable_farmhouse(int grid_offset, int map_orientation) {
    if (!map_property_is_draw_tile(grid_offset))
        return false;

    int xy = map_property_multi_tile_xy(grid_offset);
    if (map_orientation == DIR_0_TOP_RIGHT && xy == EDGE_X0Y1)
        return true;

    if (map_orientation == DIR_2_BOTTOM_RIGHT && xy == EDGE_X0Y0)
        return true;

    if (map_orientation == DIR_4_BOTTOM_LEFT && xy == EDGE_X1Y0)
        return true;

    if (map_orientation == DIR_2_BOTTOM_RIGHT && xy == EDGE_X1Y1)
        return true;

    return false;
}

static bool is_drawable_farm_corner(int grid_offset) {
    if (!map_property_is_draw_tile(grid_offset))
        return false;

    int map_orientation = city_view_orientation();
    int xy = map_property_multi_tile_xy(grid_offset);
    if (map_orientation == DIR_0_TOP_RIGHT && xy == EDGE_X0Y2)
        return true;
    else if (map_orientation == DIR_2_BOTTOM_RIGHT && xy == EDGE_X0Y0)
        return true;
    else if (map_orientation == DIR_4_BOTTOM_LEFT && xy == EDGE_X2Y0)
        return true;
    else if (map_orientation == DIR_6_TOP_LEFT && xy == EDGE_X2Y2)
        return true;

    return false;
}

void draw_flattened_footprint_anysize(vec2i pos, int size_x, int size_y, int image_offset, color color_mask, painter &ctx) {
    int image_base = image_id_from_group(GROUP_TERRAIN_OVERLAY_FLAT) + image_offset;

    for (int xx = 0; xx < size_x; xx++) {
        for (int yy = 0; yy < size_y; yy++) {
            vec2i tp = pos + vec2i{(30 * xx) + (30 * yy), (15 * xx) - (15 * yy)};

            // tile shape -- image offset
            // (0 = top corner, 1 = left edge, 2 = right edge, 3 = any other case)
            int shape_offset = 3;
            if (xx == 0) {
                shape_offset = 1;
                if (yy == size_y - 1)
                    shape_offset = 0;
            } else if (yy == size_y - 1) {
                shape_offset = 2;
            }

            ImageDraw::isometric_from_drawtile(ctx, image_base + shape_offset, tp, color_mask);
        }
    }
}

void draw_flattened_footprint_building(const building* b, vec2i pos, int image_offset, color color_mask, painter &ctx) {
    return draw_flattened_footprint_anysize(pos, b->size, b->size, image_offset, color_mask, ctx);
}

static void clip_between_rectangles(int* xOut, int* yOut, int* wOut, int* hOut, int xA, int yA, int wA, int hA, int xB, int yB, int wB, int hB) {
    *xOut = (xA > xB) ? xA : xB;
    *yOut = (yA > yB) ? yA : yB;
    int x_end_A = (xA + wA);
    int x_end_B = (xB + wB);
    int x_end_Out = (x_end_A < x_end_B) ? x_end_A : x_end_B;
    int y_end_A = (yA + hA);
    int y_end_B = (yB + hB);
    int y_end_Out = (y_end_A < y_end_B) ? y_end_A : y_end_B;
    *wOut = x_end_Out - *xOut;
    *hOut = y_end_Out - *yOut;
    if (*wOut < 0)
        *wOut = 0;
    if (*hOut < 0)
        *hOut = 0;
}

void draw_isometric_mark_sound(int building_id, int grid_offset, color &color_mask, int direction) {
    if (building_id) {
        building* b = building_get(building_id);
        if (config_get(CONFIG_UI_VISUAL_FEEDBACK_ON_DELETE) && drawing_building_as_deleted(b)) {
            color_mask = COLOR_MASK_RED;
        }

        sound_city_mark_building_view(b, direction);
    } else {
        int terrain = map_terrain_get(grid_offset);
        sound_city_mark_terrain_view(terrain, grid_offset, direction);
    }
}

template<bool flat>
bool draw_isometric_flat_building(building *b, tile2i point, painter &ctx) {
    int img_id = 0;
    int tile_id = 0;
    switch (b->type) {
    case BUILDING_SMALL_MASTABA_SIDE:
    case BUILDING_SMALL_MASTABA_WALL: 
    case BUILDING_SMALL_MASTABA_ENTRANCE: 
    case BUILDING_SMALL_MASTABA: {
            if (building_monument_is_finished(b)) {
                return false;
            }

            return ( b->data.monuments.phase < 2);
        }
        break;

    case BUILDING_ROADBLOCK:
    case BUILDING_CHICKPEAS_FARM:
    case BUILDING_BURNING_RUIN:
    case BUILDING_FLAX_FARM:
    case BUILDING_GRAIN_FARM:
    case BUILDING_LETTUCE_FARM:
    case BUILDING_POMEGRANATES_FARM:
        return true;
    }

    return b->dcast()->draw_isometric_flat_building(point, ctx);
}

void draw_isometric_flat(vec2i pixel, tile2i tile, painter &ctx) {
    auto& draw_context = get_draw_context();

    int grid_offset = tile.grid_offset();
    // black tile outside of map
    if (grid_offset < 0) {
        ImageDraw::isometric_from_drawtile(ctx, image_id_from_group(GROUP_TERRAIN_BLACK), pixel, COLOR_BLACK);
        return;
    }

    Planner.construction_record_view_position(pixel, tile);
    if (!map_property_is_draw_tile(grid_offset)) {
        return;
    }
   
    // Valid grid_offset_figure and leftmost tile -> draw
    int building_id = map_building_at(grid_offset);
    if (building_id > 0) {
        building *b = building_get(building_id);
        draw_isometric_flat_building<true>(b, tile, ctx);
    }

    color color_mask = COLOR_MASK_NONE;
    bool deletion_tool = (Planner.build_type == BUILDING_CLEAR_LAND && Planner.end == tile);
    if (deletion_tool || map_property_is_deleted(tile)) {
        color_mask = COLOR_MASK_RED;
    }

    vec2i view_pos, view_size;
    city_view_get_viewport(*ctx.view, view_pos, view_size);
    int direction = SOUND_DIRECTION_CENTER;
    if (pixel.x < view_pos.x + 100) {
        direction = SOUND_DIRECTION_LEFT;
    } else if (pixel.x > view_pos.x + view_size.x - 100) {
        direction = SOUND_DIRECTION_RIGHT;
    }

    draw_isometric_mark_sound(building_id, grid_offset, color_mask, direction);

    int image_id = map_image_at(grid_offset);
    if (draw_context.advance_water_animation) {
        if (image_id >= draw_context.image_id_water_first && image_id <= draw_context.image_id_water_last) {
            image_id++; // wrong, but eh
            if (image_id > draw_context.image_id_water_last) {
                image_id = draw_context.image_id_water_first;
            }
        }

        if (image_id >= draw_context.image_id_deepwater_first && image_id <= draw_context.image_id_deepwater_last) {
            image_id += 15;

            if (image_id > draw_context.image_id_deepwater_last) {
                image_id -= 90;
            }
        }
        map_image_set(grid_offset, image_id);
    }

    if (map_property_is_constructing(grid_offset)) {
        image_id = image_id_from_group(GROUP_TERRAIN_OVERLAY_FLAT);
    }

    const image_t *img = ImageDraw::isometric_from_drawtile(ctx, image_id, pixel, color_mask);
    if (!img) {
        return;
    }

    int image_alt_value = map_image_alt_at(grid_offset);
    int image_alt_id = (image_alt_value & 0x00ffffff);
    uint8_t image_alt_alpha = ((image_alt_value & 0xff000000) >> 24);
    if (image_alt_id > 0 && image_alt_alpha > 0) {
        ImageDraw::isometric_from_drawtile(ctx, image_alt_id, pixel, (0x00ffffff | (image_alt_alpha << 24)), /*alpha*/true);
    }

    int top_height = img->isometric_top_height();
    map_render_set(grid_offset, top_height > 0 ? RENDER_TALL_TILE : 0);
}

void draw_isometric_terrain_height(vec2i pixel, tile2i tile, painter &ctx) {
    auto& draw_context = get_draw_context();

    int grid_offset = tile.grid_offset();
    // black tile outside of map
    if (grid_offset < 0) {
        ImageDraw::isometric_from_drawtile(ctx, image_id_from_group(GROUP_TERRAIN_BLACK), pixel, COLOR_BLACK);
        return;
    }

    Planner.construction_record_view_position(pixel, tile);
    if (!map_property_is_draw_tile(grid_offset)) {
        return;
    }

    bool tall_flat_tile = map_render_is(grid_offset, RENDER_TALL_TILE);
    if (!tall_flat_tile) {
        return;
    }

    const bool non_terrain = map_terrain_is(grid_offset, TERRAIN_TREE|TERRAIN_ROCK|TERRAIN_BUILDING|TERRAIN_ELEVATION|TERRAIN_WALL|TERRAIN_GATEHOUSE);
    if (non_terrain) {
        return;
    }

    map_render_set(grid_offset, 0);
    color color_mask = COLOR_MASK_NONE;
    bool deletion_tool = (Planner.build_type == BUILDING_CLEAR_LAND && Planner.end == tile);
    if (deletion_tool || map_property_is_deleted(tile)) {
        color_mask = COLOR_MASK_RED;
    }

    int image_id = map_image_at(grid_offset);
    ImageDraw::isometric_from_drawtile_top(ctx, image_id, pixel, color_mask);

    int image_alt_value = map_image_alt_at(grid_offset);
    int image_alt_id = (image_alt_value & 0x00ffffff);
    uint8_t image_alt_alpha = ((image_alt_value & 0xff000000) >> 24);
    if (image_alt_id > 0 && image_alt_alpha > 0) {
        ImageDraw::isometric_from_drawtile_top(ctx, image_alt_id, pixel, (0x00ffffff | (image_alt_alpha << 24)), /*alpha*/true);
    }
}

void draw_isometric_nonterrain_height(vec2i pixel, tile2i tile, painter &ctx) {
    auto& draw_context = get_draw_context();

    int grid_offset = tile.grid_offset();
    // black tile outside of map
    if (grid_offset < 0) {
        ImageDraw::isometric_from_drawtile(ctx, image_id_from_group(GROUP_TERRAIN_BLACK), pixel, COLOR_BLACK);
        return;
    }

    Planner.construction_record_view_position(pixel, tile);
    if (!map_property_is_draw_tile(grid_offset)) {
        return;
    }
    // Valid grid_offset_figure and leftmost tile -> draw
    int building_id = map_building_at(grid_offset);
    building* b = building_get(building_id);
    if (building_id > 0) {
        if (draw_isometric_flat_building<false>(b, tile, ctx)) {
            return;
        }
    }

    bool tall_flat_tile = map_render_is(grid_offset, RENDER_TALL_TILE);
    bool should_draw = building_id > 0 || tall_flat_tile;

    if (!should_draw) {
        return;
    }

    color color_mask = COLOR_MASK_NONE;
    bool deletion_tool = (Planner.build_type == BUILDING_CLEAR_LAND && Planner.end == tile);
    if (deletion_tool || map_property_is_deleted(tile) || drawing_building_as_deleted(b)) {
        color_mask = COLOR_MASK_RED;
    }

    if (tall_flat_tile && building_id > 0) {
        building *b = building_get(building_id);
        map_grid_area_foreach(tile, tile.shifted(b->size, b->size), [] (tile2i tile) {
            const bool is_building = map_terrain_is(tile, TERRAIN_BUILDING);
            if (is_building) {
                return;
            }
            int image_id = map_image_at(tile);
            const image_t* img = image_get(image_id);
            int top_height = img->isometric_top_height();
            map_render_set(tile, top_height > 0);
        });
    }

    int image_id = map_image_at(grid_offset);
    if (tall_flat_tile) {
        ImageDraw::isometric_from_drawtile_top(ctx, image_id, pixel, color_mask);

        int image_alt_value = map_image_alt_at(grid_offset);
        int image_alt_id = (image_alt_value & 0x00ffffff);
        uint8_t image_alt_alpha = ((image_alt_value & 0xff000000) >> 24);
        if (image_alt_id > 0 && image_alt_alpha > 0) {
            ImageDraw::isometric_from_drawtile_top(ctx, image_alt_id, pixel, (0x00ffffff | (image_alt_alpha << 24)), /*alpha*/true);
        }
        return;
    }

    vec2i view_pos, view_size;
    city_view_get_viewport(*ctx.view, view_pos, view_size);
    int direction = SOUND_DIRECTION_CENTER;
    if (pixel.x < view_pos.x + 100) {
        direction = SOUND_DIRECTION_LEFT;
    } else if (pixel.x > view_pos.x + view_size.x - 100) {
        direction = SOUND_DIRECTION_RIGHT;
    }

    if (config_get(CONFIG_UI_VISUAL_FEEDBACK_ON_DELETE) && drawing_building_as_deleted(b)) {
        color_mask = COLOR_MASK_RED;
    }

    sound_city_mark_building_view(b, direction);
    if (map_property_is_constructing(grid_offset)) {
        image_id = image_id_from_group(GROUP_TERRAIN_OVERLAY_FLAT);
    }

    //ImageDraw::isometric_from_drawtile(ctx, image_id, pixel, color_mask);
}

void draw_figures(vec2i pixel, tile2i tile, painter &ctx) {
    draw_figures(pixel, tile, ctx, false);
}

void draw_figures(vec2i pixel, tile2i tile, painter &ctx, bool force) {
    auto& draw_context = get_draw_context();

    int grid_offset = tile.grid_offset();
    auto figures = map_figures_in_row(tile);

    for (auto *f : figures) {
        if (f->is_ghost) {
            continue;
        }

        if (f->is_drawn && !force) {
            continue;
        }

        if (f->cached_pos.x < pixel.x || f->cached_pos.x > pixel.x + TILE_WIDTH_PIXELS) {
            continue;
        }

        if (!draw_context.selected_figure_id) {
            int highlight = f->formation_id > 0 && f->formation_id == draw_context.highlighted_formation;
            f->city_draw_figure(ctx, highlight);
        } else if (f->id == draw_context.selected_figure_id) {
            f->city_draw_figure(ctx, 0, draw_context.selected_figure_coord);
        }
    }
}

void draw_isometrics_overlay_flat(vec2i pixel, tile2i point, painter &ctx) {
    int grid_offset = point.grid_offset();
    Planner.construction_record_view_position(pixel, point);
    constexpr uint32_t mode_highlighted[] = {0, COLOR_BLUE, COLOR_RED};
    if (grid_offset < 0) {
        // Outside map: draw black tile
        ImageDraw::isometric_from_drawtile(ctx, image_id_from_group(GROUP_TERRAIN_BLACK), pixel, 0);
        return;
    }

    if (get_city_overlay()->draw_custom_footprint) {
        get_city_overlay()->draw_custom_footprint(pixel, point, ctx);

    } else if (map_property_is_draw_tile(grid_offset)) {
        int terrain = map_terrain_get(grid_offset);
        if (terrain & (TERRAIN_CANAL | TERRAIN_WALL)) {
        //    // display groundwater
        //    int image_id = image_id_from_group(GROUP_TERRAIN_EMPTY_LAND) + (map_random_get(grid_offset) & 7);
        //    ImageDraw::isometric_from_drawtile(ctx, image_id, pixel, mode_highlighted[map_is_highlighted(grid_offset)]);
        //
        } else if ((terrain & TERRAIN_ROAD) && !(terrain & TERRAIN_BUILDING)) {
            ImageDraw::isometric_from_drawtile(ctx, map_image_at(grid_offset), pixel, mode_highlighted[map_is_highlighted(grid_offset)]);
        
        } else if (terrain & TERRAIN_BUILDING) {
            city_with_overlay_draw_building_footprint(ctx, pixel, grid_offset, 0);
        
        } else {
            const image_t *img = ImageDraw::isometric_from_drawtile(ctx, map_image_at(grid_offset), pixel, 0);
            int top_height = img->isometric_top_height();
            map_render_set(grid_offset, top_height > 0 ? RENDER_TALL_TILE : 0);
        }
    }

    //get_city_overlay()->draw_custom_top(pixel, point, ctx);
}

void draw_isometrics_overlay_height(vec2i pixel, tile2i point, painter &ctx) {
    int grid_offset = point.grid_offset();
    //Planner.construction_record_view_position(pixel, point);
    constexpr uint32_t mode_highlighted[] = {0, COLOR_BLUE, COLOR_RED};
    if (grid_offset < 0) {
        // Outside map: draw black tile
        //ImageDraw::isometric_from_drawtile(ctx, image_id_from_group(GROUP_TERRAIN_BLACK), pixel, 0);

    } else if (get_city_overlay()->draw_custom_footprint) {
        //get_city_overlay()->draw_custom_footprint(pixel, point, ctx);

    } else if (map_property_is_draw_tile(grid_offset)) {
        bool tall_flat_tile = map_render_is(grid_offset, RENDER_TALL_TILE);
        if (tall_flat_tile) {
            ImageDraw::isometric_from_drawtile_top(ctx, map_image_at(grid_offset), pixel, 0);
        }

        //int terrain = map_terrain_get(grid_offset);
        //if (terrain & (TERRAIN_CANAL | TERRAIN_WALL)) {
        //    // display groundwater
        //    int image_id = image_id_from_group(GROUP_TERRAIN_EMPTY_LAND) + (map_random_get(grid_offset) & 7);
        //    ImageDraw::isometric_from_drawtile_top(ctx, image_id, pixel, mode_highlighted[map_is_highlighted(grid_offset)]);
        //
        //} else if ((terrain & TERRAIN_ROAD) && !(terrain & TERRAIN_BUILDING)) {
        //    ImageDraw::isometric_from_drawtile_top(ctx, map_image_at(grid_offset), pixel, mode_highlighted[map_is_highlighted(grid_offset)]);
        //
        //} else if (terrain & TERRAIN_BUILDING) {
        //    //city_with_overlay_draw_building_footprint(ctx, pixel.x, pixel.y, grid_offset, 0);
        //
        //} else {
        //    ImageDraw::isometric_from_drawtile_top(ctx, map_image_at(grid_offset), pixel, 0);
        //}
    }

    get_city_overlay()->draw_custom_top(pixel, point, ctx);
}

void draw_ornaments_overlay(vec2i pixel, tile2i point, painter &ctx) {
    int grid_offset = point.grid_offset();
    int x = pixel.x;
    int y = pixel.y;
    int b_id = map_building_at(grid_offset);

    if (b_id) {
        const building* b = building_at(grid_offset);
        if (get_city_overlay()->show_building(b)) {
            draw_ornaments_and_animations_height(pixel, point, ctx);
        }
    } else {
        draw_ornaments_and_animations_height(pixel, point, ctx);
    }
}

void draw_figures_overlay(vec2i pixel, tile2i tile, painter &ctx) {
    auto& draw_context = get_draw_context();

    int grid_offset = tile.grid_offset();
    auto figures = map_figures_in_row(tile);

    for (auto *f : figures) {
        if (!get_city_overlay()->show_figure(f)) {
            continue;
        }

        if (f->is_ghost) {
            continue;
        }

        if (f->is_drawn) {
            continue;
        }

        if (f->cached_pos.x < pixel.x || f->cached_pos.x > pixel.x + TILE_WIDTH_PIXELS) {
            continue;
        }

        if (!draw_context.selected_figure_id) {
            int highlight = f->formation_id > 0 && f->formation_id == draw_context.highlighted_formation;
            f->city_draw_figure(ctx, highlight);
        } else if (f->id == draw_context.selected_figure_id) {
            f->city_draw_figure(ctx, 0, draw_context.selected_figure_coord);
        }
    }
}

static void draw_overlay_column(vec2i pixel, int height, int column_style, painter &ctx) {
    int image_id = image_id_from_group(GROUP_OVERLAY_COLUMN);
    switch (column_style) {
    case COLUMN_TYPE_RISK:
        if (height <= 5)
            image_id += COLUMN_COLOR_PLAIN;
        else if (height < 7)
            image_id += COLUMN_COLOR_YELLOW;
        else if (height < 9)
            image_id += COLUMN_COLOR_ORANGE;
        else
            image_id += COLUMN_COLOR_RED;
        break;

    case COLUMN_TYPE_POSITIVE:
        image_id += COLUMN_COLOR_BLUE;
        break;

    case COLUMN_TYPE_WATER_ACCESS:
        image_id += COLUMN_COLOR_BLUE;
        break;
    }

    if (height > 10) {
        height = 10;
    }

    int capital_height = image_get(image_id)->height;
    // base
    ImageDraw::img_generic(ctx, image_id + 2, pixel.x + 9, pixel.y - 8);
    if (height) {
        // column
        for (int i = 1; i < height; i++) {
            ImageDraw::img_generic(ctx, image_id + 1, pixel.x + 17, pixel.y - 8 - 10 * i + 13);
        }
        // capital
        ImageDraw::img_generic(ctx, image_id, pixel.x + 5, pixel.y - 8 - capital_height - 10 * (height - 1) + 13);
    }
}

void city_with_overlay_draw_building_footprint(painter &ctx, vec2i pos, int grid_offset, int image_offset) {
    int building_id = map_building_at(grid_offset);
    if (!building_id) {
        return;
    }

    building* b = building_get(building_id);
    if (get_city_overlay()->show_building(b)) {
        if (building_is_farm(b->type)) {
            if (is_drawable_farmhouse(grid_offset, city_view_orientation())) {
                ImageDraw::isometric_from_drawtile(ctx, map_image_at(grid_offset), pos, 0);
            } else if (map_property_is_draw_tile(grid_offset)) {
                ImageDraw::isometric_from_drawtile(ctx, map_image_at(grid_offset), pos, 0);
            }
        } else {
            ImageDraw::isometric_from_drawtile(ctx, map_image_at(grid_offset), pos, 0);
        }
    } else {
        if (b->type == BUILDING_FESTIVAL_SQUARE) {
            return;
        }
        int draw = 1;
        if (b->size == 3 && building_is_farm(b->type)) {
            draw = is_drawable_farm_corner(grid_offset);
        }

        if (draw) {
            draw_flattened_footprint_building(b, pos, image_offset, 0, ctx);
        }
    }
}

void city_with_overlay_draw_building_top(vec2i pixel, tile2i tile, painter &ctx) {
    int grid_offset = tile.grid_offset();
    building* b = building_at(grid_offset);
    if (get_city_overlay()->type == OVERLAY_PROBLEMS) {
        overlay_problems_prepare_building(b);
    }

    if (get_city_overlay()->show_building(b)) {
        map_render_set(grid_offset, RENDER_TALL_TILE);
        draw_isometric_nonterrain_height(pixel, tile, ctx);
        return;
    }

    int column_height = get_city_overlay()->get_column_height(b);
    if (column_height == NO_COLUMN) {
        return;
    }

    int draw = 1;
    if (building_is_farm(b->type)) {
        draw = is_drawable_farm_corner(grid_offset);
    }

    if (draw) {
        draw_overlay_column(pixel, column_height, get_city_overlay()->column_type, ctx);
    }
}
