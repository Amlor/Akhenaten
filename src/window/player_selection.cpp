#include "player_selection.h"

#include "core/calc.h"
#include "core/encoding.h"
#include "core/string.h"
#include "core/time.h"
#include "game/file_editor.h"
#include "graphics/boilerplate.h"
#include "graphics/graphics.h"
#include "graphics/elements/generic_button.h"
#include "graphics/elements/image_button.h"
#include "graphics/view/view.h"
#include "graphics/elements/lang_text.h"
#include "graphics/elements/panel.h"
#include "graphics/elements/scrollbar.h"
#include "graphics/image_groups.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "input/input.h"
#include "content/dir.h"
#include "content/vfs.h"
#include "io/gamefiles/lang.h"
#include "io/gamestate/boilerplate.h"
#include "game/player_data.h"
#include "main_menu.h"
#include "popup_dialog.h"
#include "window/city.h"
#include "window/editor/map.h"
#include "window/game_menu.h"
#include "window/new_career.h"
#include "game/game.h"
#include "game/settings.h"

#include "graphics/elements/scroll_list_panel.h"
#include "core/game_environment.h"

#include <string.h>

static const time_millis NOT_EXIST_MESSAGE_TIMEOUT = 500;

static void button_click(int param1, int param2);
static void button_select_file(int index, int param2);
static void button_double_click(int param1, int param2);

static generic_button buttons[] = {
  {144, 306, 126, 25, button_click, button_none, 0, 0},
  {274, 306, 126, 25, button_click, button_none, 1, 0},
  {406, 306, 86, 25, button_click, button_none, 2, 0},
  {192, 336, 256, 25, button_click, button_none, 3, 0},
};

#define NUM_FILES_IN_VIEW 12
#define MAX_FILE_WINDOW_TEXT_WIDTH (18 * INPUT_BOX_BLOCK_SIZE)

static scrollable_list_ui_params ui_params = [] {
    scrollable_list_ui_params ret;
    ret.x = 144;
    ret.y = 92;
    ret.blocks_x = 20;
    ret.blocks_y = NUM_FILES_IN_VIEW + 1;
    ret.draw_scrollbar_always = true;
    return ret;
}();

struct window_player_selection_t {
    scroll_list_panel* panel = nullptr;
    int focus_button_id;
    bstring32 selected_player;
    bstring32 selected_player_utf8;

    window_player_selection_t() {
        panel = new scroll_list_panel(NUM_FILES_IN_VIEW, button_select_file, button_none, button_double_click, button_none, ui_params, true, "Save/", "folders");
    }
};

window_player_selection_t *g_window_player_selection = nullptr;

void window_player_set_name(const char* name) {
    auto& data = *g_window_player_selection;
    data.selected_player_utf8 = name;
    encoding_from_utf8(data.selected_player_utf8, data.selected_player, MAX_PLAYER_NAME);
}

static void clear_selectd_name() {
    window_player_set_name("");
}

static bool is_selected_name(int index) {
    auto& data = *g_window_player_selection;
    return data.selected_player_utf8 == data.panel->get_selected_entry_text(FILE_NO_EXT);
}
static bool is_valid_selected_player() {
    auto& data = *g_window_player_selection;
    if (data.selected_player_utf8.empty()) {
        return false;
    }

    if (data.panel->get_entry_idx(data.selected_player_utf8) > -1) {
        return true;
    }

    return false;
}

void window_player_selection_init() {
    auto& data = *g_window_player_selection;
    data.panel->refresh_file_finder();
    data.selected_player = g_settings.player_name;
    encoding_to_utf8(data.selected_player, data.selected_player_utf8, MAX_PLAYER_NAME, 0);
    
    data.panel->select(data.selected_player_utf8);
    if (data.panel->get_total_entries() == 1) {
        data.panel->select_entry(0);
        window_player_set_name(data.panel->get_selected_entry_text(FILE_NO_EXT));
        g_settings.set_player_name(data.selected_player);
    }
}

static void draw_background() {
    painter ctx = game.painter();
    graphics_clear_screen();
    ImageDraw::img_background(ctx, image_id_from_group(GROUP_PLAYER_SELECTION));
}
static void draw_foreground() {
    auto& data = *g_window_player_selection;
    graphics_set_to_dialog();

    outer_panel_draw(128, 40, 24, 21);

    // title
    lang_text_draw_centered(292, 3, 160, 60, 304, FONT_LARGE_BLACK_ON_LIGHT);

    // family names
    data.panel->draw();

    // buttons
    for (int i = 0; i < 4; i++) {
        button_border_draw(buttons[i].x, buttons[i].y, buttons[i].width, buttons[i].height, data.focus_button_id == i + 1 ? 1 : 0);
        if (i < 3) {
            lang_text_draw_centered(292, i, buttons[i].x, buttons[i].y + 6, buttons[i].width, FONT_NORMAL_BLACK_ON_LIGHT);
        } else {
            lang_text_draw_centered(292, 4, buttons[i].x, buttons[i].y + 6, buttons[i].width, FONT_NORMAL_BLACK_ON_LIGHT);
        }
    }

    graphics_reset_dialog();
}

static void confirm_nothing(bool accepted) {
}

static void confirm_delete_player(bool accepted) {
    auto &data = *g_window_player_selection;
    if (accepted) {
        player_data_delete(data.selected_player);
        data.panel->refresh_file_finder();
    }
}

static void button_select_file(int index, int param2) {
    auto& data = *g_window_player_selection;

    if (index >= data.panel->get_total_entries()) {
        return clear_selectd_name();
    }

    window_player_set_name(data.panel->get_selected_entry_text(FILE_NO_EXT));
    g_settings.set_player_name(data.selected_player);
}

static void button_double_click(int index, int param2) {
    button_click(2, 0);
}

static void button_click(int param1, int param2) {
    switch (param1) {
    case 0: // new player
        window_new_career_show();
        break;

    case 1: // delete player
        if (is_valid_selected_player()) {
            window_popup_dialog_show(POPUP_DIALOG_DELETE_DYNASTY, confirm_delete_player, e_popup_btns_yesno);
        } else {
            window_popup_dialog_show(POPUP_DIALOG_NO_DYNASTY, confirm_nothing, e_popup_btns_yes);
        }
        break;

    case 2: // proceed with selected player
        if (!is_valid_selected_player())
            window_popup_dialog_show(POPUP_DIALOG_NO_DYNASTY, confirm_nothing, e_popup_btns_yes);
        else
            window_game_menu_show();
        break;

    case 3: // back to main menu
        window_main_menu_show(false);
        break;
    }
}

static void on_scroll(void) {
    //    data.message_not_exist_start_time = 0;
}

static void handle_input(const mouse* m, const hotkeys* h) {
    auto& data = *g_window_player_selection;

    if (input_go_back_requested(m, h)) {
        window_main_menu_show(false);
    }

    const mouse* m_dialog = mouse_in_dialog(m);
    if (data.panel->input_handle(m_dialog)) {
        return;
    }

    if (generic_buttons_handle_mouse(m_dialog, 0, 0, buttons, 4, &data.focus_button_id)) {
        return;
    }
}

void window_player_selection_show(void) {
    if (!g_window_player_selection) {
        g_window_player_selection = new window_player_selection_t();
    }

    window_type window = {
        WINDOW_PLAYER_SELECTION,
        draw_background,
        draw_foreground,
        handle_input
    };
    window_player_selection_init();
    window_show(&window);
}