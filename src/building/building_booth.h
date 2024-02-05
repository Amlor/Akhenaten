#pragma once

#include "building/building.h"

class building_booth : public building_impl {
public:
    building_booth(building &b) : building_impl(b) {}
    virtual void window_info_background(object_info &c) override;
    virtual void spawn_figure() override;
    virtual bool draw_ornaments_and_animations_height(painter &ctx, vec2i point, tile2i tile, color mask) override;
    virtual e_overlay get_overlay() const override { return OVERLAY_BOOTH; }
    virtual int get_fire_risk(int value) const override { return value / 10; }
};
