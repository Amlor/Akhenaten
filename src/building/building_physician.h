#pragma once

#include "building/building.h"

class building_physician : public building_impl {
public:
    BUILDING_METAINFO(BUILDING_PHYSICIAN, building_physician)

    building_physician(building &b) : building_impl(b) {}

    virtual void window_info_background(object_info &c) override;
    virtual void spawn_figure() override;
    virtual e_overlay get_overlay() const override { return OVERLAY_PHYSICIAN; }
    virtual bool draw_ornaments_and_animations_height(painter &ctx, vec2i point, tile2i tile, color color_mask) override;
};