#pragma once

#include "building/building.h"

class building_weaponsmith : public building_impl {
public:
    BUILDING_METAINFO(BUILDING_WEAPONSMITH, building_weaponsmith)

    building_weaponsmith(building &b) : building_impl(b) {}
    virtual void on_create(int orientation) override;
    virtual void on_place_checks() override;
    virtual bool need_road_access() const { return true; }
    virtual void update_graphic() override;
    virtual bool draw_ornaments_and_animations_height(painter &ctx, vec2i point, tile2i tile, color color_mask) override;
    virtual void update_count() const;
};