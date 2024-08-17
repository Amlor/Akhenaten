#pragma once

#include "building/building.h"

class building_weaver : public building_impl {
public:
    BUILDING_METAINFO(BUILDING_WEAVER_WORKSHOP, building_weaver)

    building_weaver(building &b) : building_impl(b) {}

    virtual void on_create(int orientation) override;
    virtual void on_place_checks() override;
    virtual void update_graphic() override;

    virtual bool draw_ornaments_and_animations_height(painter &ctx, vec2i point, tile2i tile, color color_mask) override;
    virtual void update_count() const override;
    virtual e_sound_channel_city sound_channel() const override { return SOUND_CHANNEL_CITY_NONE; }
};