#include "editor_map.h"

#include "figuretype/water.h"
#include "grid/routing/routing_terrain.h"
#include "scenario/scenario_data.h"

// TODO !!!!!!

void scenario_editor_set_entry_point(int x, int y) {
    //    g_scenario_data.entry_point.x() = x;
    //    g_scenario_data.entry_point.y() = y;
    g_scenario_data.is_saved = 0;
}

void scenario_editor_set_exit_point(int x, int y) {
    //    g_scenario_data.exit_point.x() = x;
    //    g_scenario_data.exit_point.y() = y;
    g_scenario_data.is_saved = 0;
}

static void update_river(void) {
    figure_create_flotsam();
    map_routing_update_water();
}

void scenario_editor_set_river_entry_point(int x, int y) {
    //    g_scenario_data.river_entry_point.x() = x;
    //    g_scenario_data.river_entry_point.y() = y;
    g_scenario_data.is_saved = 0;
    update_river();
}

void scenario_editor_set_river_exit_point(int x, int y) {
    //    g_scenario_data.river_exit_point.x() = x;
    //    g_scenario_data.river_exit_point.y() = y;
    g_scenario_data.is_saved = 0;
    update_river();
}

void scenario_editor_clear_predator_herd_points(void) {
    for (int i = 0; i < MAX_PREDATOR_HERD_POINTS; i++) {
        //        g_scenario_data.herd_points_predator[i].x = -1;
        //        g_scenario_data.herd_points_predator[i].y = -1;
    }
    g_scenario_data.is_saved = 0;
}

map_point scenario_editor_predator_herd_point(int id) {
    return g_scenario_data.herd_points_predator[id];
}

void scenario_editor_set_predator_herd_point(int id, int x, int y) {
    //    g_scenario_data.herd_points_predator[id].x = x;
    //    g_scenario_data.herd_points_predator[id].y = y;
    g_scenario_data.is_saved = 0;
}

void scenario_editor_clear_fishing_points(void) {
    for (int i = 0; i < MAX_FISH_POINTS; i++) {
        //        g_scenario_data.fishing_points[i].x = -1;
        //        g_scenario_data.fishing_points[i].y = -1;
    }
    g_scenario_data.is_saved = 0;
}

map_point scenario_editor_fishing_point(int id) {
    return g_scenario_data.fishing_points[id];
}

void scenario_editor_set_fishing_point(int id, int x, int y) {
    //    g_scenario_data.fishing_points[id].x = x;
    //    g_scenario_data.fishing_points[id].y = y;
    g_scenario_data.is_saved = 0;
}

int scenario_editor_count_invasion_points(void) {
    int points = 0;
    //    for (int i = 0; i < MAX_INVASION_POINTS_LAND; i++) {
    //        if (g_scenario_data.invasion_points_land[i].x != -1)
    //            points++;
    //    }
    //    for (int i = 0; i < MAX_INVASION_POINTS_SEA; i++) {
    //        if (g_scenario_data.invasion_points_sea[i].x != -1)
    //            points++;
    //    }
    return points;
}

void scenario_editor_clear_invasion_points(void) {
    //    for (int i = 0; i < MAX_INVASION_POINTS_LAND; i++) {
    //        g_scenario_data.invasion_points_land[i].x = -1;
    //        g_scenario_data.invasion_points_land[i].y = -1;
    //    }
    //    for (int i = 0; i < MAX_INVASION_POINTS_SEA; i++) {
    //        g_scenario_data.invasion_points_sea[i].x = -1;
    //        g_scenario_data.invasion_points_sea[i].y = -1;
    //    }
    g_scenario_data.is_saved = 0;
}

map_point scenario_editor_land_invasion_point(int id) {
    return g_scenario_data.invasion_points_land[id];
}

void scenario_editor_set_land_invasion_point(int id, int x, int y) {
    //    g_scenario_data.invasion_points_land[id].x = x;
    //    g_scenario_data.invasion_points_land[id].y = y;
    g_scenario_data.is_saved = 0;
}

map_point scenario_editor_earthquake_point(void) {
    return g_scenario_data.earthquake_point;
}

void scenario_editor_set_earthquake_point(int x, int y) {
    //    g_scenario_data.earthquake_point.x = x;
    //    g_scenario_data.earthquake_point.y = y;
    g_scenario_data.is_saved = 0;
}

void scenario_editor_updated_terrain(void) {
    g_scenario_data.is_saved = 0;
}
