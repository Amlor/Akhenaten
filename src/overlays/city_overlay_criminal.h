#pragma once

#include "city_overlay.h"

struct city_overlay_crime : public city_overlay {
    city_overlay_crime();

    virtual bool show_figure(const figure *f) const override;
    virtual int get_column_height(const building *b) const override;
    virtual int get_tooltip_for_building(tooltip_context *c, const building *b) const override;
    virtual void draw_custom_top(vec2i pixel, tile2i point, painter &ctx) const override;
    virtual bool show_building(const building *b) const override;
};

city_overlay* city_overlay_for_crime();