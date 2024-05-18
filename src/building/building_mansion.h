#pragma once

#include "building/building.h"

class building_mansion : public building_impl {
public:
    building_mansion(building &b) : building_impl(b) {}

    virtual void window_info_background(object_info &ctx) override;
    virtual void on_place(int orientation, int variant) override;
    virtual int window_info_handle_mouse(const mouse *m, object_info &c) override;
    virtual e_sound_channel_city sound_channel() const override { return SOUND_CHANNEL_CITY_MANSION; }
    virtual bool draw_ornaments_and_animations_height(painter &ctx, vec2i point, tile2i tile, color mask) override;
};

class building_personal_mansion : public building_mansion {
public:
    BUILDING_METAINFO(BUILDING_PERSONAL_MANSION, building_personal_mansion)
    building_personal_mansion(building &b) : building_mansion(b) {}
};

class building_family_mansion : public building_mansion {
public:
    BUILDING_METAINFO(BUILDING_PERSONAL_MANSION, building_personal_mansion)
    building_family_mansion(building &b) : building_mansion(b) {}
};

class building_dynasty_mansion : public building_mansion {
public:
    BUILDING_METAINFO(BUILDING_PERSONAL_MANSION, building_personal_mansion)
    building_dynasty_mansion(building &b) : building_mansion(b) {}
};