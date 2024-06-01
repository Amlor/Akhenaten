#pragma once

#include "building/building.h"
#include "graphics/elements/tooltip.h"
#include "core/vec2i.h"
#include "city_overlay_fwd.h"
#include "core/bstring.h"
#include "core/svector.h"

constexpr int NO_COLUMN = -1;

class figure;
class building;
struct painter;

inline bool show_figure_none(const figure *f) { return false; }
inline int get_column_height_none(const building* b) { return NO_COLUMN; }

struct city_overlay {
    svector<int, 10> tooltips;
    svector<e_figure_type, 10> walkers;
    svector<e_building_type, 10> buildings;
    e_overlay type = OVERLAY_NONE;
    int column_type = -1;

    int tooltip_base;
    bstring64 caption;

    virtual bool show_figure(const figure *f) const;
    virtual int get_column_height(const building *b) const { return NO_COLUMN; }
    virtual int get_tooltip_for_grid_offset(tooltip_context* c, int grid_offset) const { return 0; }
    virtual int get_tooltip_for_building(tooltip_context* c, const building* b) const { return 0; }
    virtual bool draw_custom_footprint(vec2i pixel, tile2i point, painter &ctx) const { return false; }
    virtual void draw_custom_top(vec2i pixel, tile2i point, painter &ctx) const;
    virtual bool show_building(const building *b) const;

    void draw_building_top(vec2i pixel, tile2i tile, painter &ctx) const;
    void draw_overlay_column(vec2i pixel, int height, int column_style, painter &ctx) const;
    void draw_building_footprint(painter &ctx, vec2i pos, tile2i tile, int image_offset) const;
    bool is_drawable_farm_corner(tile2i tile) const;
    bool is_drawable_building_corner(tile2i tile, tile2i main, int size) const;
    bool is_drawable_farmhouse(tile2i tile, int map_orientation) const;
    void draw_flattened_footprint_anysize(vec2i pos, int size_x, int size_y, int image_offset, color color_mask, painter &ctx) const;
    void draw_flattened_footprint_building(const building *b, vec2i pos, int image_offset, color color_mask, painter &ctx) const;
};

const city_overlay* get_city_overlay();
city_overlay *get_city_overlay(e_overlay e);
bool select_city_overlay();
int widget_city_overlay_get_tooltip_text(tooltip_context* c, int grid_offset);