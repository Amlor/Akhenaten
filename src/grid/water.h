#pragma once

#include "figure/figure.h"
#include "grid/point.h"
#include "terrain.h"
#include "tile_cache.h"

tile_cache &river_tiles();
void foreach_river_tile(void (*callback)(int grid_offset));

template<typename T>
void foreach_river_tile(T func) {
    for (auto &tile : river_tiles()) {
        func(tile);
    }
}

struct ferry_points {
    map_point point_a = {-1, -1};
    map_point point_b = {-1, -1};
};

ferry_points map_water_docking_points(building *b);
bool map_water_is_point_inside(tile2i tile);

void map_water_add_building(int building_id, map_point tile, int size, int image_id, int ext_terrain_flags = 0);

void map_water_cache_river_tiles();

struct shore_orientation {
    bool match;
    int orientation_absolute;
};

shore_orientation map_shore_determine_orientation(map_point tile, int size, bool adjust_xy, bool adjacent = false, int shore_terrain = TERRAIN_WATER);

int map_water_get_wharf_for_new_fishing_boat(figure* boat, map_point* tile);
int map_water_find_alternative_fishing_boat_tile(figure* boat, map_point* tile);
int map_water_find_shipwreck_tile(figure* wreck, map_point* tile);
int map_water_can_spawn_fishing_boat(int x, int y, int size, map_point* tile);
