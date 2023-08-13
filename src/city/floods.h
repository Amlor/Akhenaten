#pragma once

#include "core/buffer.h"

enum e_flood_state {
    FLOOD_STATE_IMMINENT = 1,
    FLOOD_STATE_FLOODING = 2,
    FLOOD_STATE_INUNDATED = 3,
    FLOOD_STATE_CONTRACTING = 4,
    FLOOD_STATE_RESTING = 5,
    FLOOD_STATE_FARMABLE = 6,
};

struct floods_data_t {
    int season_initial;
    int duration_initial;
    int quality_initial;
    int season;
    int duration;
    int quality;
    int unk00; // 00
    int quality_next;
    int quality_last;
    //
    // Pharaoh had a struct containing tile data for the updates.
    // I reimplemented it via other means.
    //
    int flood_progress;   // 0 - 30
    int unk01;            // season/floodplain_width -- probably a leftover from a previous flood system
    int state;            // 1 - 6
    int floodplain_width; // 0 - 30
    bool has_floodplains; // 0 - 1
};

floods_data_t& floodplain_data();

void floodplains_init();

int floods_debug_period();
int floods_fticks();

extern const double CYCLES_IN_A_YEAR;
double floods_current_cycle();
int floods_current_subcycle();
bool tick_is_flood_cycle();
int floods_start_cycle();
int floods_end_cycle();
double floods_period_length(bool upcoming = false);

bool floodplains_is(int state);

void floodplains_adjust_next_quality(int quality);
int floodplains_expected_quality();
int floodplains_expected_month();

void floodplains_tick_update(bool calc_only);
