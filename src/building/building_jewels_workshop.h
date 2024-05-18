#pragma once

#include "building/building.h"

class building_jewels_workshop : public building_impl {
public:
    BUILDING_METAINFO(BUILDING_JEWELS_WORKSHOP, building_jewels_workshop)

    building_jewels_workshop(building &b) : building_impl(b) {}

    virtual void on_create(int orientation) override;
    virtual void on_place_checks() override;
    virtual bool is_administration() const override { return true; }
    virtual void spawn_figure() override;
    virtual void window_info_background(object_info &ctx) override;
    virtual bool draw_ornaments_and_animations_height(painter &ctx, vec2i point, tile2i tile, color color_mask) override;
    virtual void update_count() const override;
    virtual e_sound_channel_city sound_channel() const override { return SOUND_CHANNEL_CITY_NONE; }
};