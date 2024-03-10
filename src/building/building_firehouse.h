#pragma once

#include "building/building.h"

class building_firehouse : public building_impl {
public:
    BUILDING_METAINFO(BUILDING_FIREHOUSE, building_firehouse)

    building_firehouse(building &b) : building_impl(b) {}
    virtual building_firehouse *dcast_firehouse() { return this; }

    virtual void spawn_figure() override;
    virtual void window_info_background(object_info &c) override;
    virtual e_overlay get_overlay() const override { return OVERLAY_FIRE; }
    virtual int animation_speed(int speed) const override { return 4; }
    virtual bool draw_ornaments_and_animations_height(painter &ctx, vec2i point, tile2i tile, color color_mask) override;
};