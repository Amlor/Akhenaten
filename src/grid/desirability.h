#pragma once

#include "grid/point.h"

struct desirability_t {
    struct influence_t {
        int size = 0;
        int value = 0;
        int step = 0;
        int step_size = 0;
        int range = 0;
    };

    struct {
        influence_t plaza;
        influence_t earthquake;
        influence_t garden;
        influence_t rubble;
    } influence;

    void update_terrain();
    void clear();
    void update();
    void update_buildings();
    void load();

    void add_to_terrain_at_distance(tile2i tile, int size, int distance, int desirability);
    void add_to_terrain(tile2i tile, int size, int desirability, int step, int step_size, int range);
    int get(int grid_offset);
    inline int get(tile2i tile) { return get(tile.grid_offset()); }
    int get_max(tile2i tile, int size);
    int get_avg(tile2i tile, int size);
};

extern desirability_t g_desirability;