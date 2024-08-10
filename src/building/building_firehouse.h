#pragma once

#include "building/building.h"
#include "window/window_building_info.h"

class building_firehouse : public building_impl {
public:
    BUILDING_METAINFO(BUILDING_FIREHOUSE, building_firehouse)

    building_firehouse(building &b) : building_impl(b) {}
    virtual building_firehouse *dcast_firehouse() { return this; }

    virtual void spawn_figure() override;
    virtual e_overlay get_overlay() const override { return OVERLAY_FIRE; }
    virtual void update_graphic() override;
    virtual int animation_speed(int speed) const override { return 4; }
    virtual bool draw_ornaments_and_animations_height(painter &ctx, vec2i point, tile2i tile, color color_mask) override;
};

struct info_window_firehouse_t : public building_info_window {
    virtual void window_info_background(object_info& c) override;
    virtual bool check(object_info& c) override;
};