#pragma once

#include "core/vec2i.h"

#define TOOLTIP_MAX_EXTRA_VALUES 5
struct mouse;

enum e_tooltip_mode { 
    TOOLTIP_NONE = 0,
    TOOLTIP_BUTTON = 1,
    TOOLTIP_OVERLAY = 2,
    TOOLTIP_SENATE = 3,
    TOOLTIP_TILES = 4
};

struct tooltip_context {
    vec2i mpos;
    int type = 0;
    int high_priority = 0;
    int text_group = 0;
    int text_id = 0;
    int has_numeric_prefix = 0;
    int numeric_prefix = 0;
    int num_extra_values = 0;
    int extra_value_text_groups[TOOLTIP_MAX_EXTRA_VALUES] = {0};
    int extra_value_text_ids[TOOLTIP_MAX_EXTRA_VALUES] = {0};
};

void tooltip_invalidate();
void tooltip_handle(const mouse* m, void (*func)(tooltip_context*));
