#pragma once

#include <cstdint>
#include "core/bstring.h"

// file versions found so far:
//  144 (Bridges.map only)
//  146 (NAFTA.map and Warfare.map only)
//  147 (most of the older campaign scenarios)
//  149 (later campaign scenarios and maps)
//  150 (later campaign scenarios and maps)
//  160 (post-Cleopatra campaign scenarios, patched Bubastis scenario)
//  161 akhenaten: save foods in houses
//  162 akhenaten: save g_terrain_floodplain_growth
//  163 akhenaten: save bazaar_days in house
//  164 akhenaten: save water_supply in house
//  165 akhenaten: save house health option
constexpr uint32_t latest_save_version = 165;

bstring256 fullpath_saves(const char* filename);
void fullpath_maps(char* full, const char* filename);

namespace GamestateIO {
const int get_campaign_scenario_offset(int scenario_id);
const int read_file_version(const char* filename, int offset);

bool write_mission(const int scenario_id);
bool write_savegame(const char* filename_short);

bool write_map(const char* filename_short);

bool load_mission(const int scenario_id, bool start_immediately);
bool load_savegame(pcstr filename_short, bool start_immediately = true);
bool load_map(pcstr filename_short, bool start_immediately = true);

void start_loaded_file();

bool delete_mission(const int scenario_id);
bool delete_savegame(const char* filename_short);
bool delete_map(const char* filename_short);

} // namespace GamestateIO
