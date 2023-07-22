#include <tgmath.h>
#include "dev/debug.h"
#include "city.h"

#include "building/menu.h"
#include "city/message.h"
#include "core/game_environment.h"
#include "game/orientation.h"
#include "game/state.h"
#include "game/undo.h"
#include "graphics/boilerplate.h"
#include "graphics/elements/image_button.h"
#include "graphics/elements/lang_text.h"
#include "graphics/screen.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "scenario/property.h"
#include "widget/minimap.h"
#include "widget/sidebar/common.h"
#include "widget/sidebar/extra.h"
#include "widget/sidebar/slide.h"
#include "widget/city.h"
#include "window/advisors.h"
#include "window/build_menu.h"
#include "window/city.h"
#include "window/empire.h"
#include "window/message_dialog.h"
#include "window/message_list.h"
#include "window/mission_briefing.h"
#include "window/overlay_menu.h"

#define MINIMAP_Y_OFFSET 59

static void button_overlay(int param1, int param2);
static void button_collapse_expand(int param1, int param2);
static void button_build(int submenu, int param2);
static void button_undo(int param1, int param2);
static void button_messages(int param1, int param2);
static void button_help(int param1, int param2);
static void button_go_to_problem(int param1, int param2);
static void button_advisors(int param1, int param2);
static void button_empire(int param1, int param2);
static void button_mission_briefing(int param1, int param2);
static void button_rotate_north(int param1, int param2);
static void button_rotate(int clockwise, int param2);

static image_button buttons_overlays_collapse_sidebar[2] = {
        {128, 0, 31, 20, IB_NORMAL, GROUP_SIDEBAR_UPPER_BUTTONS, 7, button_collapse_expand, button_none, 0, 0, 1},
        {4,   3, 117, 31, IB_NORMAL, GROUP_SIDEBAR_UPPER_BUTTONS, 0, button_overlay,        button_help, 0, MESSAGE_DIALOG_OVERLAYS, 1}
};

static image_button button_expand_sidebar[1] = {
        {8, 0, 31, 20, IB_NORMAL, GROUP_SIDEBAR_UPPER_BUTTONS, 10, button_collapse_expand, button_none, 0, 0, 1}
};

#define CL_ROW0 21 //22

static image_button buttons_build_collapsed[12] = {
        {9, CL_ROW0, 36, 48, IB_BUILD, GROUP_SIDEBAR_BUTTONS, 90, button_build, button_none, BUILD_MENU_VACANT_HOUSE, 0, 1},
        {9, CL_ROW0 + 36, 36, 48, IB_BUILD, GROUP_SIDEBAR_BUTTONS, 94, button_build, button_none, BUILD_MENU_ROAD, 0, 1},
        {9, CL_ROW0 + 71, 36, 48, IB_BUILD, GROUP_SIDEBAR_BUTTONS, 98, button_build, button_none, BUILD_MENU_CLEAR_LAND, 0, 1},
        {9, CL_ROW0 + 108, 36, 48, IB_BUILD, GROUP_SIDEBAR_BUTTONS, 102, button_build, button_none, BUILD_MENU_FOOD, 0, 1},
        {9, CL_ROW0 + 142, 36, 48, IB_BUILD, GROUP_SIDEBAR_BUTTONS, 106, button_build, button_none, BUILD_MENU_INDUSTRY, 0, 1},
        {9, CL_ROW0 + 177, 36, 48, IB_BUILD, GROUP_SIDEBAR_BUTTONS, 110, button_build, button_none, BUILD_MENU_DISTRIBUTION, 0, 1},
        {9, CL_ROW0 + 212, 36, 48, IB_BUILD, GROUP_SIDEBAR_BUTTONS, 114, button_build, button_none, BUILD_MENU_ENTERTAINMENT, 0, 1},
        {9, CL_ROW0 + 245, 36, 48, IB_BUILD, GROUP_SIDEBAR_BUTTONS, 118, button_build, button_none, BUILD_MENU_RELIGION, 0, 1},
        {9, CL_ROW0 + 281, 36, 48, IB_BUILD, GROUP_SIDEBAR_BUTTONS, 122, button_build, button_none, BUILD_MENU_EDUCATION, 0, 1},
        {9, CL_ROW0 + 317, 36, 48, IB_BUILD, GROUP_SIDEBAR_BUTTONS, 126, button_build, button_none, BUILD_MENU_HEALTH, 0, 1},
        {9, CL_ROW0 + 353, 36, 48, IB_BUILD, GROUP_SIDEBAR_BUTTONS, 130, button_build, button_none, BUILD_MENU_ADMINISTRATION, 0, 1},
        {9, CL_ROW0 + 385, 36, 48, IB_BUILD, GROUP_SIDEBAR_BUTTONS, 134, button_build, button_none, BUILD_MENU_SECURITY, 0, 1},
};

#define COL1 9
#define COL2 COL1 + 34 + 3
#define COL3 COL2 + 36 + 4
#define COL4 COL3 + 34 + 5

#define ROW1 251
#define ROW2 ROW1 + 48 + 1
#define ROW3 ROW2 + 50 + 1
#define ROW4 ROW3 + 49 + 4

static image_button buttons_build_expanded[15] = {
        {COL1, ROW1, 34, 48, IB_BUILD, GROUP_SIDEBAR_BUTTONS, 0, button_build, button_none, BUILD_MENU_VACANT_HOUSE, 0, 1},
        {COL1, ROW2, 34, 50, IB_BUILD, GROUP_SIDEBAR_BUTTONS, 4, button_build, button_none, BUILD_MENU_ROAD,       0, 1},
        {COL1, ROW3, 34, 49, IB_BUILD, GROUP_SIDEBAR_BUTTONS, 8,  button_build, button_none, BUILD_MENU_CLEAR_LAND, 0, 1},

        {COL2, ROW1, 36, 48, IB_BUILD, GROUP_SIDEBAR_BUTTONS, 12, button_build, button_none, BUILD_MENU_FOOD,  0, 1},
        {COL2, ROW2, 36, 50, IB_BUILD, GROUP_SIDEBAR_BUTTONS, 16, button_build, button_none, BUILD_MENU_INDUSTRY, 0, 1},
        {COL2, ROW3, 36, 49, IB_BUILD, GROUP_SIDEBAR_BUTTONS, 20, button_build, button_none, BUILD_MENU_DISTRIBUTION, 0, 1},

        {COL3, ROW1, 34, 48, IB_BUILD, GROUP_SIDEBAR_BUTTONS, 24, button_build, button_none, BUILD_MENU_ENTERTAINMENT, 0, 1},
        {COL3, ROW2, 34, 50, IB_BUILD, GROUP_SIDEBAR_BUTTONS, 28, button_build, button_none, BUILD_MENU_RELIGION,      0, 1},
        {COL3, ROW3, 34, 49, IB_BUILD, GROUP_SIDEBAR_BUTTONS, 32, button_build, button_none, BUILD_MENU_EDUCATION,      0, 1},

        {COL4, ROW1, 34, 48, IB_BUILD, GROUP_SIDEBAR_BUTTONS, 36, button_build, button_none, BUILD_MENU_HEALTH,      0, 1},
        {COL4, ROW2, 34, 50, IB_BUILD, GROUP_SIDEBAR_BUTTONS, 40, button_build, button_none, BUILD_MENU_ADMINISTRATION, 0, 1},
        {COL4, ROW3, 34, 49, IB_BUILD, GROUP_SIDEBAR_BUTTONS, 44, button_build, button_none, BUILD_MENU_SECURITY, 0, 1},

        {COL1, ROW4, 35, 45, IB_NORMAL, GROUP_SIDEBAR_BUTTONS, 48, button_undo, button_none, 0, 0, 1},
        {COL2, ROW4, 38, 45, IB_NORMAL, GROUP_SIDEBAR_BUTTONS, 52, button_messages, button_help, 0, MESSAGE_DIALOG_MESSAGES, 1},
        {COL3, ROW4, 28, 45, IB_NORMAL, GROUP_SIDEBAR_BUTTONS, 56, button_go_to_problem, button_none, 0, 0, 1},
};

static image_button buttons_top_expanded[3] = {
        {COL1 + 7, 143, 60, 36, IB_NORMAL, GROUP_SIDEBAR_BUTTONS, 64, button_advisors, button_none, 0, 0, 1},
        {COL3 + 4, 143, 62, 36, IB_NORMAL, GROUP_SIDEBAR_BUTTONS, 68, button_empire, button_help, 0, MESSAGE_DIALOG_EMPIRE_MAP, 1},
        {COL4 - 9, ROW4, 43, 45, IB_NORMAL, GROUP_SIDEBAR_BUTTONS, 60, button_mission_briefing, button_none, 0, 0, 1},
};

struct sidebar_data_t {
    int focus_tooltip_text_id;
};

sidebar_data_t g_sidebar_data;

static void draw_overlay_text(int x_offset) {
    if (game_state_overlay()) {
        lang_text_draw_centered(14, game_state_overlay(), x_offset - 15, 30, 117, FONT_NORMAL_BLACK_ON_LIGHT);
    } else {
        const bool is_button_focused = buttons_overlays_collapse_sidebar[1].focused;
        lang_text_draw_centered(6, 4, x_offset - 15, 30, 117, is_button_focused ? FONT_NORMAL_WHITE_ON_DARK : FONT_NORMAL_BLACK_ON_LIGHT);
    }
}
static void draw_sidebar_remainder(int x_offset, bool is_collapsed) {
    int width = SIDEBAR_EXPANDED_WIDTH;
    if (is_collapsed)
        width = SIDEBAR_COLLAPSED_WIDTH;
    int available_height = sidebar_common_get_height() - SIDEBAR_MAIN_SECTION_HEIGHT;
    int extra_height = sidebar_extra_draw_background(x_offset, SIDEBAR_MAIN_SECTION_HEIGHT + TOP_MENU_HEIGHT, 162, available_height, is_collapsed, SIDEBAR_EXTRA_DISPLAY_ALL);
    sidebar_extra_draw_foreground();
    int relief_y_offset = SIDEBAR_MAIN_SECTION_HEIGHT + TOP_MENU_HEIGHT + extra_height; // + (GAME_ENV == ENGINE_ENV_PHARAOH) * 6;
    sidebar_common_draw_relief(x_offset, relief_y_offset, GROUP_SIDE_PANEL, is_collapsed);
}
static void draw_number_of_messages(int x_offset) {
    int messages = city_message_count();
    buttons_build_expanded[13].enabled = messages > 0;
    if (messages) {
        text_draw_number_centered_colored(messages, x_offset + 74, 452, 32, FONT_SMALL_PLAIN, COLOR_BLACK);
        text_draw_number_centered_colored(messages, x_offset + 73, 453, 32, FONT_SMALL_PLAIN, COLOR_WHITE);
    }
}

static void draw_buttons_collapsed(int x_offset) {
    image_buttons_draw(x_offset, TOP_MENU_HEIGHT, button_expand_sidebar, 1);
    image_buttons_draw(x_offset, TOP_MENU_HEIGHT, buttons_build_collapsed, 12);
}
static void draw_buttons_expanded(int x_offset) {
    buttons_build_expanded[12].enabled = game_can_undo();
    buttons_build_expanded[14].enabled = city_message_problem_area_count();
    if (GAME_ENV == ENGINE_ENV_C3) {
        image_buttons_draw(x_offset, TOP_MENU_HEIGHT, buttons_overlays_collapse_sidebar, 2);
        image_buttons_draw(x_offset, TOP_MENU_HEIGHT, buttons_build_expanded, 15);
        image_buttons_draw(x_offset, TOP_MENU_HEIGHT, buttons_top_expanded, 6);
    } else if (GAME_ENV == ENGINE_ENV_PHARAOH) {
        image_buttons_draw(x_offset, TOP_MENU_HEIGHT, buttons_overlays_collapse_sidebar, 1);
        image_buttons_draw(x_offset, TOP_MENU_HEIGHT, buttons_build_expanded, 15);
        image_buttons_draw(x_offset, TOP_MENU_HEIGHT, buttons_top_expanded, 3);
    }
}

static void refresh_build_menu_buttons(void) {
    int num_buttons = 12;
    for (int i = 0; i < num_buttons; i++) {
        buttons_build_expanded[i].enabled = 1;
        if (building_menu_count_items(buttons_build_expanded[i].parameter1) <= 0)
            buttons_build_expanded[i].enabled = 0;

        buttons_build_collapsed[i].enabled = 1;
        if (building_menu_count_items(buttons_build_collapsed[i].parameter1) <= 0)
            buttons_build_collapsed[i].enabled = 0;
    }
}
static void draw_collapsed_background(void) {
    int x_offset = sidebar_common_get_x_offset_collapsed();
    if (GAME_ENV == ENGINE_ENV_C3)
        ImageDraw::img_generic(image_id_from_group(GROUP_SIDE_PANEL), x_offset, TOP_MENU_HEIGHT);
    else if (GAME_ENV == ENGINE_ENV_PHARAOH)
        ImageDraw::img_generic(image_id_from_group(GROUP_SIDE_PANEL) + 1, x_offset, TOP_MENU_HEIGHT);
    draw_buttons_collapsed(x_offset);
    draw_sidebar_remainder(x_offset, true);
}
static void draw_expanded_background(int x_offset) {
    ImageDraw::img_generic(image_id_from_group(GROUP_SIDE_PANEL), x_offset, TOP_MENU_HEIGHT);
    ImageDraw::img_generic(window_build_menu_image(), x_offset + 11, 181 + TOP_MENU_HEIGHT);
    widget_minimap_draw(x_offset + 12, MINIMAP_Y_OFFSET, MINIMAP_WIDTH, MINIMAP_HEIGHT, 1);

    // extra bar spacing on the right
    int block_height = 702;
    int s_end = 768;
    int s_num = ceil((float) (screen_height() - s_end) / (float) block_height);
    int s_start = s_num * block_height;
    for (int i = 0; i < s_num; i++) {
        ImageDraw::img_generic(image_id_from_group(GROUP_SIDE_PANEL) + 2, x_offset + 162, s_start + i * block_height);
    }
    
    ImageDraw::img_generic(image_id_from_group(GROUP_SIDE_PANEL) + 2, x_offset + 162, 0);
    draw_number_of_messages(x_offset - 26);

    draw_buttons_expanded(x_offset);
    draw_overlay_text(x_offset + 4);

    draw_sidebar_remainder(x_offset, false);
}
void widget_sidebar_city_draw_background(void) {
    if (city_view_is_sidebar_collapsed())
        draw_collapsed_background();
    else
        draw_expanded_background(sidebar_common_get_x_offset_expanded());
}
void widget_sidebar_city_draw_foreground(void) {
    if (building_menu_has_changed())
        refresh_build_menu_buttons();

    if (city_view_is_sidebar_collapsed()) {
        int x_offset = sidebar_common_get_x_offset_collapsed();
        draw_buttons_collapsed(x_offset);
    } else {
        int x_offset = sidebar_common_get_x_offset_expanded();
        draw_buttons_expanded(x_offset);
        draw_overlay_text(x_offset + 4);

        if (GAME_ENV == ENGINE_ENV_C3) {
            widget_minimap_draw(x_offset + 8, MINIMAP_Y_OFFSET, MINIMAP_WIDTH, MINIMAP_HEIGHT, 0);
            draw_number_of_messages(x_offset);
        } else if (GAME_ENV == ENGINE_ENV_PHARAOH) {
            widget_minimap_draw(x_offset + 12, MINIMAP_Y_OFFSET, MINIMAP_WIDTH, MINIMAP_HEIGHT, 0);
            draw_number_of_messages(x_offset - 26);
        }

    }
    sidebar_extra_draw_foreground();

    window_request_refresh();
    draw_debug_ui(10, 30);
}
void widget_sidebar_city_draw_foreground_military(void) {
    widget_minimap_draw(sidebar_common_get_x_offset_expanded() + 8, MINIMAP_Y_OFFSET, MINIMAP_WIDTH, MINIMAP_HEIGHT, 1);
}
int widget_sidebar_city_handle_mouse(const mouse *m) {
    if (widget_city_has_input())
        return false;

    bool handled = false;
    int button_id;
    auto &data = g_sidebar_data;
    data.focus_tooltip_text_id = 0;
    if (city_view_is_sidebar_collapsed()) {
        int x_offset = sidebar_common_get_x_offset_collapsed();
        handled |= image_buttons_handle_mouse(m, x_offset, 24, button_expand_sidebar, 1, &button_id);
        if (button_id)
            data.focus_tooltip_text_id = 12;

        handled |= image_buttons_handle_mouse(m, x_offset, 24, buttons_build_collapsed, 12, &button_id);
        if (button_id)
            data.focus_tooltip_text_id = button_id + 19;

    } else {
        if (widget_minimap_handle_mouse(m))
            return true;

        int x_offset = sidebar_common_get_x_offset_expanded();
        handled |= image_buttons_handle_mouse(m, x_offset, 24, buttons_overlays_collapse_sidebar, 2, &button_id);
        if (button_id)
            data.focus_tooltip_text_id = button_id + 9;

        handled |= image_buttons_handle_mouse(m, x_offset, 24, buttons_build_expanded, 15, &button_id);
        if (button_id)
            data.focus_tooltip_text_id = button_id + 19;

        handled |= image_buttons_handle_mouse(m, x_offset, 24, buttons_top_expanded, 6, &button_id);
        if (button_id)
            data.focus_tooltip_text_id = button_id + 40;

        handled |= (sidebar_extra_handle_mouse(m) != 0);
    }
    return handled;
}
int widget_sidebar_city_handle_mouse_build_menu(const mouse *m) {
    if (city_view_is_sidebar_collapsed()) {
        return image_buttons_handle_mouse(m, sidebar_common_get_x_offset_collapsed(), 24, buttons_build_collapsed, 12, 0);
    } else {
        return image_buttons_handle_mouse(m, sidebar_common_get_x_offset_expanded(), 24, buttons_build_expanded, 15, 0);
    }
}
int widget_sidebar_city_get_tooltip_text(void) {
    return g_sidebar_data.focus_tooltip_text_id;
}

void widget_sidebar_city_release_build_buttons() {
    image_buttons_release_press(buttons_build_expanded, 20);
    image_buttons_release_press(buttons_build_collapsed, 20);
}

static void slide_finished(void) {
    city_view_toggle_sidebar();
    window_city_show();
    window_draw(1);
}

static void button_overlay(int param1, int param2) {
    window_overlay_menu_show();
}
static void button_collapse_expand(int param1, int param2) {
    city_view_start_sidebar_toggle();
    sidebar_slide(!city_view_is_sidebar_collapsed(), draw_collapsed_background, draw_expanded_background,
                  slide_finished);
}
static void button_build(int submenu, int param2) {
    window_build_menu_show(submenu);
}
static void button_undo(int param1, int param2) {
    game_undo_perform();
    window_invalidate();
}
static void button_messages(int param1, int param2) {
    window_message_list_show();
}
static void button_help(int param1, int param2) {
    window_message_dialog_show(param2, -1, window_city_draw_all);
}
static void button_go_to_problem(int param1, int param2) {
    int grid_offset = city_message_next_problem_area_grid_offset();
    if (grid_offset) {
        camera_go_to_mappoint(map_point(grid_offset));
        window_city_show();
    } else {
        window_invalidate();
    }
}
static void button_advisors(int param1, int param2) {
    window_advisors_show_checked();
}
static void button_empire(int param1, int param2) {
    window_empire_show_checked();
}
static void button_mission_briefing(int param1, int param2) {
    if (!scenario_is_custom())
        window_mission_briefing_show_review();

}
static void button_rotate_north(int param1, int param2) {
    game_orientation_rotate_north();
    window_invalidate();
}
static void button_rotate(int clockwise, int param2) {
    if (clockwise) {
        game_orientation_rotate_right();
    } else {
        game_orientation_rotate_left();
    }
    window_invalidate();
}
