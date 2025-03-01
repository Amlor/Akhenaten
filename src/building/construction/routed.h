#pragma once

#include "grid/routing/routing.h"

struct routed_building_result {
    bool ok;
    int items;
};

routed_building_result place_routed_building(tile2i start, tile2i end, e_routed_mode type);
routed_building_result building_construction_place_canal_for_reservoir(bool measure_only, tile2i start, tile2i end);

int building_construction_place_canal(bool measure_only, tile2i start, tile2i end);
