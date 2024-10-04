#pragma once

#include "building/building.h"

class building_courthouse : public building_impl {
public:
    BUILDING_METAINFO(BUILDING_COURTHOUSE, building_courthouse)

    building_courthouse(building &b) : building_impl(b) {}
    virtual building_courthouse *dcast_courthouse() override { return this; }

    //virtual void on_create() override;
    virtual e_overlay get_overlay() const override { return OVERLAY_COUTHOUSE; }
    virtual bool is_administration() const override { return true; }
    virtual void spawn_figure() override;
    virtual void update_graphic() override;
    virtual bool draw_ornaments_and_animations_height(painter &ctx, vec2i point, tile2i tile, color color_mask) override;
};