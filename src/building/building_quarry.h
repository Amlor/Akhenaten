#pragma once

#include "building/building.h"

class building_quarry : public building_impl {
public:
    building_quarry(building &b) : building_impl(b) {}
    virtual building_quarry *dcast_quarry() override { return this; }
};

class building_sandstone_quarry : public building_quarry {
public:
    BUILDING_METAINFO(BUILDING_SANDSTONE_QUARRY, building_sandstone_quarry)
    building_sandstone_quarry(building &b) : building_quarry(b) {}

    virtual int animation_speed(int speed) const override { return 3; }
    virtual bool draw_ornaments_and_animations_height(painter &ctx, vec2i point, tile2i tile, color color_mask) override;
    virtual void update_count() const override;
};

class building_stone_quarry : public building_quarry {
public:
    BUILDING_METAINFO(BUILDING_STONE_QUARRY, building_stone_quarry)
    building_stone_quarry(building &b) : building_quarry(b) {}

    virtual int animation_speed(int speed) const override { return 3; }
    virtual bool draw_ornaments_and_animations_height(painter &ctx, vec2i point, tile2i tile, color color_mask) override;
    virtual int get_produce_uptick_per_day() const override { return base.num_workers > 0 ? std::max<int>(1, base.num_workers / 2) : 0; }
    virtual void update_count() const override;
};

class building_limestone_quarry : public building_quarry {
public:
    BUILDING_METAINFO(BUILDING_LIMESTONE_QUARRY, building_limestone_quarry)
    building_limestone_quarry(building &b) : building_quarry(b) {}

    virtual int animation_speed(int speed) const override { return 3; }
    virtual bool draw_ornaments_and_animations_height(painter &ctx, vec2i point, tile2i tile, color color_mask) override;
    virtual void update_count() const override;
};

class building_granite_quarry : public building_quarry {
public:
    BUILDING_METAINFO(BUILDING_GRANITE_QUARRY, building_granite_quarry)
    building_granite_quarry(building &b) : building_quarry(b) {}

    virtual int animation_speed(int speed) const override { return 3; }
    virtual bool draw_ornaments_and_animations_height(painter &ctx, vec2i point, tile2i tile, color color_mask) override;
    virtual void update_count() const override;
};