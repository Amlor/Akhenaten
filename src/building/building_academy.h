#pragma once

#include "building/building.h"

class building_academy : public building_impl {
public:
    BUILDING_METAINFO(BUILDING_ACADEMY, building_academy)

    building_academy(building &b) : building_impl(b) {}

    virtual void on_create(int orientation) override {}
    virtual void window_info_background(object_info &c) override;
    //virtual bool draw_ornaments_and_animations_height(painter &ctx, vec2i point, tile2i tile, color mask) override;
    virtual e_overlay get_overlay() const override { return OVERLAY_EDUCATION; }
    virtual e_sound_channel_city sound_channel() const override { return SOUND_CHANNEL_CITY_NONE; }
};