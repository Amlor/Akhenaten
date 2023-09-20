#pragma once

#include "building/building.h"
#include "grid/point.h"

void map_building_tiles_add(int building_id, map_point tile, int size, int image_id, int terrain);
void map_building_tiles_add_farm(int building_id, int x, int y, int crop_image_offset, int progress);

void map_add_bandstand_tiles(building* b);
int map_bandstand_main_img_offset(int orientation);
int map_bandstand_add_img_offset(int orientation);
void map_add_venue_plaza_tiles(int building_id, int size, int x, int y, int image_id, bool update_only);

void map_add_temple_complex_base_tiles(int type, int x, int y, int orientation);
void map_building_tiles_add_temple_complex_parts(building* b);

void map_building_tiles_remove(int building_id, int x, int y);
void map_building_tiles_set_rubble(int building_id, int x, int y, int size);
void map_building_tiles_mark_deleting(int grid_offset);
bool map_building_tiles_mark_construction(int x, int y, int size_x, int size_y, int terrain, bool absolute_xy);
int map_building_tiles_are_clear(int x, int y, int size, int terrain);
