#pragma once

#include "building.h"
#include "game/resource.h"
#include "grid/road_access.h"

#define MONUMENT_FINISHED -1
#define MONUMENT_START 1
#define MARS_OFFERING_FREQUENCY 16

enum module_type {
    
} ;

int building_monument_access_point(building *b, map_point *dst);
int building_monument_add_module(building *b, int module_type);
int building_monument_deliver_resource(building *b, e_resource resource);
int building_monument_get_monument(tile2i tile, e_resource resource, int road_network_id, map_point *dst);
int building_monument_has_unfinished_monuments();
void building_monument_set_phase(building *b, int phase);
bool building_monument_is_monument(const building *b);
bool building_monument_type_is_monument(e_building_type type);
bool building_monument_type_is_mini_monument(e_building_type type);
bool building_monument_is_temple_complex(e_building_type type);
int building_monument_needs_resource(building *b, e_resource resource);
bool building_monument_needs_resources(building *b);
int building_monument_progress(building *b);
bool building_monument_has_labour_problems(building *b);
int building_monument_working(e_building_type type);
bool building_monument_requires_resource(e_building_type type, e_resource resource);
int building_monument_has_required_resources_to_build(e_building_type type);
bool building_monument_resources_needed_for_monument_type(e_building_type type, e_resource resource, int phase);
int building_monument_resource_in_delivery(building *b, int resource_id);
void building_monument_remove_delivery(int figure_id);
void building_monument_add_delivery(int monument_id, int figure_id, int resource_id, int num_loads);
bool building_monument_has_delivery_for_worker(int figure_id);
void building_monument_remove_all_deliveries(int monument_id);
int building_monument_get_id(e_building_type type);
int building_monument_upgraded(e_building_type type);
int building_monument_module_type(e_building_type type);
int building_monument_phases(e_building_type building_type);
void building_monument_finish_monuments();
void building_monuments_set_construction_phase(int phase);
void building_monument_initialize_deliveries();

bool building_monument_need_workers(building *b);
int building_monument_is_construction_halted(building *b);
int building_monument_toggle_construction_halted(building *b);
bool building_monument_is_unfinished(const building *b);
bool building_monument_is_finished(const building *b);

int get_monument_part_image(int part, int orientation, int level);
int get_temple_complex_part_image(int type, int part, int orientation, int level);

uint32_t map_monuments_get_progress(int grid_offset);
inline uint32_t map_monuments_get_progress(tile2i tile) { return map_monuments_get_progress(tile.grid_offset()); }
void map_monuments_set_progress(int grid_offset, uint32_t progress);
inline void map_monuments_set_progress(tile2i tile, uint32_t progress) { return map_monuments_set_progress(tile.grid_offset(), progress); }
void map_monuments_clear();

building *city_has_unfinished_monuments();