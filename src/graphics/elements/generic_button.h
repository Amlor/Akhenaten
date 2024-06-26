#pragma once

#include "core/string.h"
#include "input/mouse.h"
#include "graphics/text.h"
#include "graphics/elements/button.h"

#include <vector>
#include <cstdint>
#include <functional>

struct generic_button {
    int x;
    int y;
    int width;
    int height;
    void (*left_click_handler)(int param1, int param2) = nullptr;
    void (*right_click_handler)(int param1, int param2) = nullptr;
    int parameter1 = 0;
    int parameter2 = 0;

    std::function<void(int, int)> _onclick = nullptr;
    std::pair<int, int> _tooltip;

    inline vec2i pos() const { return {x, y}; }
    inline vec2i size() const { return {width, height}; }

    template<class Func> generic_button &onclick(Func f) { _onclick = f; return *this; }
    generic_button &tooltip(std::pair<int, int> t) { _tooltip = t; return *this; }
    generic_button &tooltip(const std::initializer_list<int> &t) { _tooltip.first = *t.begin(); _tooltip.second = *(t.begin() + 1); return *this; }
};

int generic_buttons_handle_mouse(const mouse* m, int x, int y, const generic_button* buttons, int num_buttons, int* focus_button_id);
int generic_buttons_min_handle_mouse(const mouse* m, int x, int y, const generic_button* buttons, int num_buttons, int* focus_button_id, int minimum_button);

template<uint32_t N>
inline int generic_buttons_handle_mouse(const mouse *m, vec2i pos, const generic_button (&buttons)[N], int &focus_button_id) {
    return generic_buttons_handle_mouse(m, pos.x, pos.y, buttons, N, &focus_button_id);
}

template<class T>
inline int generic_buttons_handle_mouse(const mouse *m, vec2i pos, const T &buttons, int &focus_button_id) {
    return buttons.size() > 0
                ? generic_buttons_handle_mouse(m, pos.x, pos.y, &buttons.front(), (int)buttons.size(), &focus_button_id)
                : 0; 
}