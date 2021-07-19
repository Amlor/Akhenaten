#include "config.h"

#include "core/hotkey_config.h"
#include "core/image_group.h"
#include "core/string.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/panel.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "translation/translation.h"

#define NUM_BOTTOM_BUTTONS 2

static void button_close(int save, int param2);

static generic_button bottom_buttons[] = {
        {192, 228, 120, 24, button_close, button_none, 0},
        {328, 228, 120, 24, button_close, button_none, 1},
};

static int bottom_button_texts[] = {
        TR_BUTTON_CANCEL,
        TR_BUTTON_OK
};

static struct {
    int action;
    int index;
    int key;
    int modifiers;
    void (*callback)(int, int, int, int);
    int focus_button;
} data;

static void init(int action, int index,
                 void (*callback)(int, int, int, int)) {
    data.action = action;
    data.index = index;
    data.callback = callback;
    data.key = KEY_NONE;
    data.modifiers = KEY_MOD_NONE;
    data.focus_button = 0;
}

static void draw_background(void) {
    graphics_in_dialog();
    outer_panel_draw(168, 128, 19, 9);

    text_draw_centered(translation_for(TR_HOTKEY_EDIT_TITLE), 176, 144, 296, FONT_LARGE_BLACK, 0);

    for (int i = 0; i < NUM_BOTTOM_BUTTONS; i++) {
        generic_button *btn = &bottom_buttons[i];
        text_draw_centered(translation_for(bottom_button_texts[i]),
                           btn->x, btn->y + 6, btn->width, FONT_NORMAL_BLACK, 0);
    }

    graphics_reset_dialog();
}

static void draw_foreground(void) {
    graphics_in_dialog();

    inner_panel_draw(192, 184, 16, 2);

    text_draw_centered(key_combination_display_name(data.key, data.modifiers),
                       192, 193, 256, FONT_NORMAL_WHITE, 0);

    for (int i = 0; i < NUM_BOTTOM_BUTTONS; i++) {
        generic_button *btn = &bottom_buttons[i];
        button_border_draw(btn->x, btn->y, btn->width, btn->height, data.focus_button == i + 1);
    }
    graphics_reset_dialog();
}

static void handle_input(const mouse *m, const hotkeys *h) {
    const mouse *m_dialog = mouse_in_dialog(m);

    int handled = 0;
    handled |= generic_buttons_handle_mouse(m_dialog, 0, 0, bottom_buttons, NUM_BOTTOM_BUTTONS, &data.focus_button);
    if (!handled && m->right.went_up)
        button_close(0, 0);

}

static void button_close(int ok, int param2) {
    if (ok)
        data.callback(data.action, data.index, data.key, data.modifiers);

    window_go_back();
}

void window_hotkey_editor_key_pressed(int key, int modifiers) {
    if (key == KEY_ENTER && modifiers == KEY_MOD_NONE)
        button_close(1, 0);
    else if (key == KEY_ESCAPE && modifiers == KEY_MOD_NONE)
        button_close(0, 0);
    else {
        if (key != KEY_NONE)
            data.key = key;

        data.modifiers = modifiers;
    }
}

void window_hotkey_editor_key_released(int key, int modifiers) {
    // update modifiers as long as we don't have a proper keypress
    if (data.key == KEY_NONE && key == KEY_NONE)
        data.modifiers = modifiers;

}

void window_hotkey_editor_show(int action, int index,
                               void (*callback)(int, int, int, int)) {
    window_type window = {
            WINDOW_HOTKEY_EDITOR,
            draw_background,
            draw_foreground,
            handle_input
    };
    init(action, index, callback);
    window_show(&window);
}
