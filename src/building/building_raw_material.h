#pragma once

#include "building/building.h"
#include "graphics/animation.h"

void building_marble_quarry_draw_info(object_info& c);
void building_raw_material_draw_info(object_info &c, const char *type, e_resource resource);

class building_clay_pit : public building_impl {
public:
    BUILDING_METAINFO(BUILDING_CLAY_PIT, building_clay_pit)

    building_clay_pit(building &b) : building_impl(b) {}
    virtual building_clay_pit *dcast_clay_pit() override { return this; }

    virtual void on_create(int orientation) override;
    virtual void window_info_background(object_info &c) override;
    virtual int get_fire_risk(int value) const override;
    virtual e_sound_channel_city sound_channel() const override { return SOUND_CHANNEL_CITY_CLAY_PIT; }
    virtual bool draw_ornaments_and_animations_height(painter &ctx, vec2i point, tile2i tile, color color_mask) override;
    virtual void update_count() const override;
};

class building_reed_gatherer : public building_impl {
public:
    BUILDING_METAINFO(BUILDING_REED_GATHERER, building_reed_gatherer)

    building_reed_gatherer(building &b) : building_impl(b) {}
    virtual building_reed_gatherer *dcast_reed_gatherer() override { return this; }

    virtual void on_create(int orientation) override;
    virtual void window_info_background(object_info &c) override;
    virtual void spawn_figure() override;
    virtual e_sound_channel_city sound_channel() const override { return SOUND_CHANNEL_CITY_CLAY_PIT; }
    virtual bool draw_ornaments_and_animations_height(painter &ctx, vec2i point, tile2i tile, color color_mask) override;
    virtual void update_count() const override;

    bool can_spawn_gatherer(int max_gatherers_per_building, int carry_per_person);
};

class building_mine : public building_impl {
public:
    building_mine(building &b) : building_impl(b) {}
    virtual void on_create(int orientation) override;
    virtual const animation_t &anim(pcstr key) const = 0;
    virtual void window_info_background(object_info &c) override;
    virtual bool draw_ornaments_and_animations_height(painter &ctx, vec2i point, tile2i tile, color color_mask) override;
};

class building_mine_gold : public building_mine {
public:
    BUILDING_METAINFO(BUILDING_GOLD_MINE, building_mine_gold)

    building_mine_gold(building &b) : building_mine(b) {}
    virtual const static_params &params() const override;
    virtual const animation_t &anim(pcstr key) const override;
    virtual int get_produce_uptick_per_day() const override;
    virtual void update_count() const override;
};

class building_mine_gems : public building_mine {
public:
    BUILDING_METAINFO(BUILDING_GEMSTONE_MINE, building_mine_gems)

    building_mine_gems(building &b) : building_mine(b) {}
    virtual const static_params &params() const override;
    virtual const animation_t &anim(pcstr key) const override;
    virtual void update_count() const override;
    virtual int get_produce_uptick_per_day() const override { return base.num_workers > 0 ? std::max<int>(1, base.num_workers / 3) : 0; }
};

class building_quarry_stone : public building_impl {
public:
    building_quarry_stone(building &b) : building_impl(b) {}
    virtual void on_create(int orientation) override;
    virtual void window_info_background(object_info &c) override;
    virtual int get_produce_uptick_per_day() const override { return base.num_workers > 0 ? std::max<int>(1, base.num_workers / 2) : 0; }
};

class building_mine_copper : public building_mine {
public:
    BUILDING_METAINFO(BUILDING_COPPER_MINE, building_mine_copper)

    building_mine_copper(building &b) : building_mine(b) {}
    virtual const static_params &params() const override;
    virtual const animation_t &anim(pcstr key) const override;
    virtual int get_produce_uptick_per_day() const override { return base.num_workers > 0 ? std::max<int>(1, base.num_workers / 2) : 0; }
};
