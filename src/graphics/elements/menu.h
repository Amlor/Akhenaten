#pragma once

#include "input/mouse.h"
#include "core/bstring.h"
#include "core/svector.h"
#include <functional>

#define TOP_MENU_HEIGHT 30
#define SIDEBAR_EXPANDED_WIDTH 186

struct menu_item {
    short text_group;
    short text_number;
    void (*left_click_handler)(int) = nullptr;
    int parameter;
    int hidden = false;
    bstring64 text;
    bstring64 id;
};

struct menu_header {
    svector<menu_item, 32> items;
    bstring64 text;
    short x_start;
    short x_end;
    int calculated_width_blocks;
    int calculated_height_blocks;
    std::function<void(menu_item&)> _onclick;
};
