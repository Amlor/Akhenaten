#pragma once

#include <cstdint>

enum e_image_id : uint16_t {
    IMG_NONE = 0,
    IMG_IMMIGRANT = 1,
    IMG_IMMIGRANT_DEATH = 2,
    IMG_EMIGRANT = 3,
    IMG_EMIGRANT_DEATH = 4,
    IMG_HOMELESS = 5,
    IMG_HOMELESS_DEATH = 6,
    IMG_FISHIHG_WHARF_ANIM = 7,
    IMG_CARTPUSHER = 8,
    IMG_CARTPUSHER_DEATH = 9,
    IMG_FISHING_BOAT = 10,
    IMG_FISHING_BOAT_WORK = 11,
    IMG_FISHING_BOAT_DEATH = 13,
    IMG_FISHING_BOAT_IDLE = 14,
    IMG_FLOTSAM = 15,
    IMG_LABOR_SEEKER = 16,
    IMG_LABOR_SEEKER_DEATH = 17,
    IMG_FISHING_POINT = 18,
    IMG_FISHING_POINT_BUBLES = 19,
    IMG_ENGINEER = 20,
    IMG_ENGINEER_DEATH = 21,
    IMG_WATER_SUPPLY = 22,
    IMG_WATER_SUPPLY_FANCY = 23,
    IMG_WELL = 24,
    IMG_WELL_FANCY = 25,
    IMG_EXPLOSION = 26,

    IMG_SIZE
};

struct image_desc {
    int pack = 0;
    int id = 0;
    int offset = 0;
};

void set_image_desc(e_image_id t, int pack, int id, int offset);
image_desc get_image_desc(e_image_id t);