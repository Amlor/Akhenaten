#pragma once

#include "building/building.h"

class building_bandstand : public building_impl {
public:
    BUILDING_METAINFO(BUILDING_BANDSTAND, building_bandstand)
    building_bandstand(building &b) : building_impl(b) {}

    virtual void on_create(int orientation) override;
    virtual void on_place(int orientation, int variant) override;
    virtual void spawn_figure() override;
    virtual void window_info_background(object_info &c) override;
    virtual int animation_speed(int speed) const override { return 4; }
    virtual int get_fire_risk(int value) const override { return value / 10; }
    virtual void update_count() const;
    virtual e_sound_channel_city sound_channel() const override { return SOUND_CHANNEL_CITY_BANDSTAND; }
    virtual bool draw_isometric_flat_building(tile2i point, painter &ctx) override;

    virtual bool draw_ornaments_and_animations_height(painter &ctx, vec2i point, tile2i tile, color mask) override;
    void draw_shows_musicians(painter &ctx, vec2i pixel, int direction, color color_mask);
    static void ghost_preview(painter &ctx, tile2i tile, vec2i pixel, int orientation);
};