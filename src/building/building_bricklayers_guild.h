#pragma once

#include "building/building.h"

class building_bricklayers_guild : public building_impl {
public:
    BUILDING_METAINFO(BUILDING_BRICKLAYERS_GUILD, building_bricklayers_guild)

    building_bricklayers_guild(building &b) : building_impl(b) {}

    virtual void on_create(int orientation) override;
    virtual void spawn_figure() override;
    virtual void window_info_background(object_info &c) override;
    virtual bool draw_ornaments_and_animations_height(painter &ctx, vec2i point, tile2i tile, color color_mask) override;

    bool can_spawn_bricklayer_man(int max_gatherers_per_building);
};