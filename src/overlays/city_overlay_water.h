#pragma once

#include "overlays/city_overlay.h"

struct city_overlay_water : public city_overlay {
    city_overlay_water();

    void draw_custom_top(vec2i pixel, tile2i tile, painter &ctx) const override;
    bool draw_custom_footprint(vec2i pixel, tile2i point, painter &ctx) const override;
    int get_tooltip_for_grid_offset(tooltip_context* c, int grid_offset) const override;
    int get_column_height(const building *b) const override;
};

city_overlay* city_overlay_for_water();