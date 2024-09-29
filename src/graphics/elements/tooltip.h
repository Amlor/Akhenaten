#pragma once

#include "core/vec2i.h"
#include "core/string.h"
#include "core/xstring.h"

#include <functional>

#define TOOLTIP_MAX_EXTRA_VALUES 5
struct mouse;

struct tooltip_context {
    vec2i mpos;
    int high_priority = 0;
    xstring text;
    int has_numeric_prefix = 0;
    int numeric_prefix = 0;
    int num_extra_values = 0;
    int extra_value_text_groups[TOOLTIP_MAX_EXTRA_VALUES] = {0};
    int extra_value_text_ids[TOOLTIP_MAX_EXTRA_VALUES] = {0};

    std::function<void()> _drawtooltip;

    void set(int t, textid tx);
    void set(int t, pcstr tx);
};

void tooltip_invalidate();
void draw_tooltip_box(int x, int y, int width, int height);
void tooltip_handle(const mouse* m, void (*func)(tooltip_context*));
