#pragma once

#include "building/building.h"

class building_papyrus_maker : public building_impl {
public:
    BUILDING_METAINFO(BUILDING_PAPYRUS_WORKSHOP, building_papyrus_maker)

    building_papyrus_maker(building &b) : building_impl(b) {}
    virtual building_papyrus_maker *dcast_papyrus_maker() override { return this; }

    virtual void on_create() override;
    virtual void window_info_background(object_info &c) override;
    virtual bool draw_ornaments_and_animations_height(painter &ctx, vec2i point, tile2i tile, color color_mask) override;
};
