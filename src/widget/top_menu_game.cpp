#include "top_menu_game.h"

#include "game/game.h"

#include "graphics/elements/menu.h"
#include "graphics/elements/ui.h"
#include "graphics/screenshot.h"
#include "graphics/window.h"
#include "graphics/graphics.h"
#include "city/constants.h"
#include "city/finance.h"
#include "city/population.h"
#include "core/profiler.h"
#include "core/core_utility.h"
#include "core/span.hpp"
#include "config/config.h"
#include "game/settings.h"
#include "game/cheats.h"
#include "game/orientation.h"
#include "game/state.h"
#include "game/time.h"
#include "game/undo.h"
#include "window/file_dialog.h"
#include "window/message_dialog.h"
#include "io/gamestate/boilerplate.h"
#include "building/construction/build_planner.h"
#include "scenario/scenario.h"
#include "game/system.h"
#include "graphics/screen.h"
#include "widget/widget_city.h"
#include "window/display_options.h"
#include "window/advisors.h"
#include "window/window_city.h"
#include "window/difficulty_options.h"
#include "window/config.h"
#include "window/game_menu.h"
#include "window/hotkey_config.h"
#include "window/main_menu.h"
#include "window/popup_dialog.h"
#include "window/speed_options.h"
#include "window/sound_options.h"
#include "dev/debug.h"

#include "js/js_game.h"

enum e_info {
    INFO_NONE = 0,
    INFO_FUNDS = 1,
    INFO_POPULATION = 2,
    INFO_DATE = 3
};

static void button_rotate_left(int param1, int param2);
static void button_rotate_reset(int param1, int param2);
static void button_rotate_right(int param1, int param2);

struct top_menu_data_t {
    int offset_funds;
    int offset_funds_basic;
    int offset_population;
    int offset_population_basic;
    int offset_date;
    int offset_date_basic;
    int offset_rotate;
    int offset_rotate_basic;

    bstring64 open_sub_menu;
    bstring64 focus_menu_id;
    bstring64 focus_sub_menu_id;

    int population;
    int treasury;
    int month;

    vec2i offset;
    int item_height;
    int spacing;
    e_image_id background;

    ui::widget headers;
};

top_menu_data_t g_top_menu_data;

static generic_button orientation_buttons_ph[] = {
    {12, 0, 36 - 24, 21, button_rotate_reset, button_none, 0, 0},
    {0, 0, 12, 21, button_rotate_left, button_none, 0, 0},
    {36 - 12, 0, 12, 21, button_rotate_right, button_none, 0, 0},
};

void widget_top_menu_item_update(pcstr header, int item, pcstr text) {
    auto &data = g_top_menu_data;
    auto menu = data.headers[header].dcast_menu_header();
    if (!menu) {
        return;
    }

    menu->item(item).text = text;
}

static void menu_debug_render_text(int opt, bool v) {
    auto& data = g_top_menu_data;
    static const char *debug_text_rend[][2] = {
        {"Buildings ON", "Buildings OFF"},
        {"Tile Size ON", "Tile Size OFF"},
        {"Roads ON", "Roads OFF"},
        {"Routing Dist ON", "Routing Dist OFF"},
        {"Routing Grid ON", "Routing Grid OFF"},
        {"Moisture ON", "Moisture OFF"},
        {"Grass Level ON", "Grass Level OFF"},
        {"Soil Depl ON", "Soil Depl OFF"},
        {"Flood Order ON", "Flood Order OFF"},
        {"Flood Flags ON", "Flood Flags OFF"},
        {"Labor ON", "Labor OFF"},
        {"Sprite Frames ON", "Sprite Frames OFF"},
        {"Terrain Bits ON", "Terrain Bits OFF"},
        {"Image ON", "Image OFF"},
        {"Image Alt ON", "Image Alt OFF"},
        {"Marshland ON", "Marshland OFF"},
        {"Terrain ON", "Terrain OFF"},
        {"Tile Coord ON", "Tile Coord OFF"},
        {"Flood Shore ON", "Flood Shore OFF"},
        {"Tile TopH ON", "Tile TopH OFF"},
        {"Monuments ON", "Monuments OFF"},
        {"Figures ON", "Figures OFF"},
        {"Height ON", "Height OFF"},
        {"Marshland Depl ON", "Marshland Depl OFF"},
        {"Dmg Fire ON", "Dmg Fire OFF"},
        {"Desirability ON", "Desirability OFF"},
    };
    widget_top_menu_item_update("debug_render", opt, debug_text_rend[opt][v ? 0 : 1]);
}

ANK_REGISTER_CONFIG_ITERATOR(config_load_top_menu_bar);
void config_load_top_menu_bar() {
    g_config_arch.r_section("top_menu_bar", [] (archive arch) {
        auto& data = g_top_menu_data;
        data.offset = arch.r_vec2i("offset");
        data.item_height = arch.r_int("item_height");
        data.background = (e_image_id)arch.r_int("background");
        data.spacing = arch.r_int("spacing");
        data.offset_funds_basic = arch.r_int("offset_funds_basic");
        data.offset_population_basic = arch.r_int("offset_population_basic");
        data.offset_date_basic = arch.r_int("offset_date_basic");
        data.offset_rotate_basic = arch.r_int("offset_rotate_basic");

        data.headers.load(arch, "headers");
        for (auto &header : data.headers.elements) {
            auto impl = header->dcast_menu_header();
            if (impl) {
                impl->load_items(arch, header->id);
            }
        }
    });
}

static void menu_debug_opt_text(int opt, bool v) {
    static const char* debug_text_opt[][2] = {
        {"Pages ON", "Pages OFF"},
        {"Game Time ON", "Game Time OFF"},
        {"Build Planner ON", "Build Planner OFF"},
        {"Religion ON", "Religion OFF"},
        {"Tutorial ON", "Tutorial OFF"},
        {"Floods ON", "Floods OFF"},
        {"Camera ON", "Camera OFF"},
        {"Tile Cache ON", "Tile Cache OFF"},
        {"Migration ON", "Migration OFF"},
        {"Sentiment ON", "Sentiment OFF"},
        {"Sound Channels ON", "Sound Channels OFF"},
        {"Properties ON", "Properties OFF"},
        {"Show console", "Show console"},
        {"Screenshot", "Screenshot"},
        {"Full Screenshot", "Full Screenshot"},
        {"Write Video ON", "Write Video OFF"},
    };
    widget_top_menu_item_update("debug", opt, debug_text_opt[opt][v ? 0 : 1]);
}

static void menu_debug_screenshot(int opt) {
    widget_top_menu_clear_state();
    window_go_back();
    window_invalidate();
    graphics_save_screenshot(SCREENSHOT_DISPLAY);
}

static void menu_debug_full_screenshot(int opt) {
    widget_top_menu_clear_state();
    window_go_back();
    window_invalidate();
    graphics_save_screenshot(SCREENSHOT_FULL_CITY);
}

static void menu_debug_change_opt(menu_item &item) {
    int opt = item.parameter;
    switch (opt) {
    case e_debug_show_console: game_cheat_console(true); break;
    case e_debug_make_screenshot: menu_debug_screenshot(0); break;
    case e_debug_make_full_screenshot: menu_debug_full_screenshot(0); break;
    case e_debug_show_properties: 
        game.debug_properties = !game.debug_properties;
        g_debug_show_opts[opt] = game.debug_properties;
        widget_top_menu_clear_state();
        window_go_back();
        window_invalidate();
        menu_debug_opt_text(e_debug_show_properties, game.debug_properties );
        break;

    case e_debug_write_video: 
        game.set_write_video(!game.get_write_video());
        menu_debug_opt_text(e_debug_write_video, game.get_write_video());
        g_debug_show_opts[opt] = game.get_write_video();
        break;

    default:
        g_debug_show_opts[opt] = !g_debug_show_opts[opt];
        menu_debug_opt_text(opt, g_debug_show_opts[opt]);
    }
}

static void menu_debug_render_change_opt(menu_item &item) {
    int opt = item.parameter;
    g_debug_render = (opt == g_debug_render) ? 0 : opt;
    auto& data = g_top_menu_data;
    auto *render = data.headers["debug_render"].dcast_menu_header();
    for (int i = 0; i < render->impl.items.size(); ++i) {
        menu_debug_render_text(i, g_debug_render == render->impl.items[i].parameter);
    }
}

static void button_rotate_reset(int param1, int param2) {
    game_orientation_rotate_north();
    window_invalidate();
}
static void button_rotate_left(int param1, int param2) {
    game_orientation_rotate_left();
    window_invalidate();
}
static void button_rotate_right(int param1, int param2) {
    game_orientation_rotate_right();
    window_invalidate();
}

void top_menu_bar_draw() {
    auto& data = g_top_menu_data;
    auto &headers = data.headers;
    vec2i offset = data.offset;
    e_font hightlight_font = config_get(CONFIG_UI_HIGHLIGHT_TOP_MENU_HOVER) ? FONT_NORMAL_YELLOW : FONT_NORMAL_BLACK_ON_LIGHT;
    for (auto &it : headers.elements) {
        ui::emenu_header *header = it->dcast_menu_header();

        if (!header) {
            continue;
        }

        header->impl.x_start = offset.x;
        header->font((it->id == data.focus_menu_id) ? hightlight_font : FONT_NORMAL_BLACK_ON_LIGHT);
        header->pos = vec2i{offset.x, data.offset.y};
        header->draw();

        offset.x += header->text_width();
        header->impl.x_end = offset.x;
        offset.x += data.spacing;
    }
}

static bstring64 top_menu_bar_get_selected_header(const mouse* m) {
    auto& data = g_top_menu_data;
    auto &headers = g_top_menu_data.headers;
    for (auto &it : headers.elements) {
        ui::emenu_header *header = it->dcast_menu_header();

        if (!header) {
            continue;
        }

        if (header->impl.x_start <= m->x && header->impl.x_end > m->x && data.offset.y <= m->y && data.offset.y + 12 > m->y) {
            return header->id;
        }
    }
    return {};
}

static bstring64 top_menu_bar_handle_mouse(const mouse* m) {
    g_top_menu_data.focus_menu_id = top_menu_bar_get_selected_header(m);
    return top_menu_bar_get_selected_header(m);
}

static void top_menu_calculate_menu_dimensions(menu_header& menu) {
    auto& data = g_top_menu_data;
    int max_width = 0;
    int height_pixels = data.item_height;
    for (const auto &item: menu.items) {
        if (item.hidden) {
            continue;
        }

        int width_pixels = lang_text_get_width(item.text, FONT_NORMAL_BLACK_ON_LIGHT);
        max_width = std::max(max_width, width_pixels);

        height_pixels += data.item_height;
    }
    int blocks = (max_width + 8) / 16 + 1; // 1 block padding
    menu.calculated_width_blocks = blocks < 10 ? 10 : blocks;
    menu.calculated_height_blocks = height_pixels / 16;
}

void top_menu_menu_draw(const bstring64 &header, const bstring64 &focus_item_id) {
    auto& menu = g_top_menu_data;
    auto &impl = ((ui::emenu_header *)&menu.headers[header])->impl;

    if (impl.calculated_width_blocks == 0 || impl.calculated_height_blocks == 0) {
        top_menu_calculate_menu_dimensions(impl);
    }

    unbordered_panel_draw(impl.x_start, TOP_MENU_HEIGHT, impl.calculated_width_blocks, impl.calculated_height_blocks);
    int y_offset = TOP_MENU_HEIGHT + menu.offset.y * 2;
    for (const auto &item: impl.items) {
        if (item.hidden) {
            continue;
        }
        // Set color/font on the menu item mouse hover
        lang_text_draw(item.text, vec2i{impl.x_start + 8, y_offset}, item.id == focus_item_id ? FONT_NORMAL_YELLOW : FONT_NORMAL_BLACK_ON_LIGHT);
        y_offset += menu.item_height;
    }
}

static bstring64 top_menu_get_subitem(const mouse* m, menu_header &menu) {
    auto& data = g_top_menu_data;
    int y_offset = TOP_MENU_HEIGHT + data.offset.y * 2;

    for (const auto &item: menu.items) {
        if (item.hidden) {
            continue;
        }

        if (menu.x_start <= m->x && menu.x_start + 16 * menu.calculated_width_blocks > m->x && y_offset - 2 <= m->y && y_offset + 19 > m->y) {
            return item.id;
        }

        y_offset += data.item_height;
    }

    return {};
}

bstring64 top_menu_menu_handle_mouse(const mouse* m, menu_header* menu, bstring64& focus_item_id) {
    if (!menu) {
        return "";
    }

    bstring64 item_id = top_menu_get_subitem(m, *menu);
    focus_item_id = item_id;

    if (!item_id) {
        return "";
    }

    if (m->left.went_up) {
        auto it = std::find_if(menu->items.begin(), menu->items.end(), [&item_id] (auto &it) { return it.id == item_id; });
        if (it != menu->items.end()) {
            if (it->left_click_handler) {
                it->left_click_handler(it->parameter);
            } else if (menu->_onclick) {
                menu->_onclick(*it);
            }
        }
    }

    return item_id;
}

void top_menu_header_update_text(pcstr header, pcstr text) {
    auto& menu = g_top_menu_data;
    auto &impl = ((ui::emenu_header *)&menu.headers[header])->impl;

    menu.headers[header].text(text);
    if (impl.calculated_width_blocks == 0) {
        return;
    }

    int item_width = lang_text_get_width(impl.text, FONT_NORMAL_BLACK_ON_LIGHT);
    int blocks = (item_width + 8) / 16 + 1;
    if (blocks > impl.calculated_width_blocks) {
        impl.calculated_width_blocks = blocks;
    }
}

std::pair<bstring64, bstring64> split_string(pcstr input) {
    std::pair<bstring64, bstring64> result;
    pcstr pos = strstr(input, "/");
    if (pos) {
        result.first.ncat(input, (pos - input));
        result.second.cat(pos + 1);
    }

    return result;
}

void top_menu_item_update_text(pcstr path, pcstr text) {
    auto &menu = g_top_menu_data;

    auto pair = split_string(path);
    auto header = menu.headers[pair.first].dcast_menu_header();
    auto &item = header->item(pair.second);
    item.text = text;
}

void widget_top_menu_clear_state() {
    auto& data = g_top_menu_data;

    data.open_sub_menu = "";
    data.focus_menu_id = "";
    data.focus_sub_menu_id = "";
}

static void set_text_for_autosave() {
    top_menu_item_update_text("options/autosave_options", ui::str(19, g_settings.monthly_autosave ? 51 : 52));
}

static void set_text_for_tooltips() {
    top_menu_item_update_text("help/mouse", ui::str(3, g_settings.tooltips + 2));
}

static void set_text_for_warnings(void) {
    top_menu_item_update_text("help/warnings", ui::str(3, g_settings.warnings ? 6 : 5));
}

static void set_text_for_debug_city() {
    auto& data = g_top_menu_data;
    auto *debug = data.headers["debug"].dcast_menu_header();
    for (int i = 0; i < debug->impl.items.size(); ++i) {
        menu_debug_opt_text(i, g_debug_show_opts[i]);
    }
}

static void set_text_for_debug_render() {
    auto& data = g_top_menu_data;
    auto *render = data.headers["debug_render"].dcast_menu_header();
    for (int i = 0; i < render->impl.items.size(); ++i) {
        menu_debug_render_text(i, g_debug_render == render->impl.items[i].parameter);
    }
}

static void menu_file_delete_game(int param) {
    widget_top_menu_clear_state();
    window_city_show();
    window_file_dialog_show(FILE_TYPE_SAVED_GAME, FILE_DIALOG_DELETE);
}

static void menu_file_exit_city(int param) {
    widget_top_menu_clear_state();
    window_yesno_dialog_show("#popup_dialog_quit", [] (bool accepted) {
        if (accepted) {
            widget_top_menu_clear_state();
            window_main_menu_show(true);
        } else {
            window_city_show();
        }
    });
}

static void menu_file_load_game(int param) {
    widget_top_menu_clear_state();
    Planner.reset();
    window_city_show();
    window_file_dialog_show(FILE_TYPE_SAVED_GAME, FILE_DIALOG_LOAD);
}

static void menu_file_save_game(int param) {
    widget_top_menu_clear_state();
    window_city_show();
    window_file_dialog_show(FILE_TYPE_SAVED_GAME, FILE_DIALOG_SAVE);
}

static void menu_file_new_game(int param) {
    widget_top_menu_clear_state();
    window_yesno_dialog_show("#popup_dialog_quit", [] (bool confirmed) {
        if (!confirmed) {
            window_city_show();
            return;
        }

        Planner.reset();
        game_undo_disable();
        game_state_reset_overlay();
        window_game_menu_show();
    });
}

static void menu_file_replay_map(int param) {
    widget_top_menu_clear_state();
    window_popup_dialog_show_confirmation("#replay_mission", [] (bool confirmed) {
        if (!confirmed) {
            window_city_show();
            return;
        }

        Planner.reset();
        if (scenario_is_custom()) {
            GamestateIO::load_savegame("autosave_replay.sav");
            window_city_show();
        } else {
            int scenario_id = scenario_campaign_scenario_id();
            widget_top_menu_clear_state();
            GamestateIO::load_mission(scenario_id, true);
        }
    });
}

static void top_menu_file_handle(menu_item &item) {
    if (item.id == "new_game") { menu_file_new_game(0); }
    else if (item.id == "replay_map") { menu_file_replay_map(0); }
    else if (item.id == "load_game") { menu_file_load_game(0); }
    else if (item.id == "save_game") { menu_file_save_game(0); }
    else if (item.id == "delete_game") { menu_file_delete_game(0); }
    else if (item.id == "exit_game") { menu_file_exit_city(0); }
}

static void menu_options_display(int param) {
    widget_top_menu_clear_state();
    ui::window_display_options::window.show(window_city_show);
}

static void menu_options_sound(int param) {
    widget_top_menu_clear_state();
    window_sound_options_show(window_city_show);
}

static void menu_options_speed(int param) {
    widget_top_menu_clear_state();
    window_speed_options_show(window_city_show);
}

static void menu_options_difficulty(int param) {
    widget_top_menu_clear_state();
    window_difficulty_options_show(window_city_show);
}

static void menu_options_autosave(int param) {
    g_settings.toggle_monthly_autosave();
    set_text_for_autosave();
}

static void menu_options_change_enh(int param) {
    window_config_show([] {});
}

static void menu_options_hotkeys(int param) {
    window_hotkey_config_show([] {});
}

static void top_menu_options_handle(menu_item &item) {
    if (item.id == "display_options") { menu_options_display(0); }
    else if (item.id == "sound_options") { menu_options_sound(0); }
    else if (item.id == "speed_options") { menu_options_speed(0); }
    else if (item.id == "difficulty_options") { menu_options_difficulty(0); }
    else if (item.id == "autosave_options") { menu_options_autosave(0); }
    else if (item.id == "hotkeys_options") { menu_options_hotkeys(0); }
    else if (item.id == "enhanced_options") { menu_options_change_enh(0); }
}

static void menu_help_help(int param) {
    widget_top_menu_clear_state();
    window_go_back();
    window_message_dialog_show(MESSAGE_DIALOG_HELP, -1, window_city_draw_all);
}

static void menu_help_mouse_help(int param) {
    g_settings.toggle_tooltips();
    set_text_for_tooltips();
}
static void menu_help_warnings(int param) {
    g_settings.toggle_warnings();
    set_text_for_warnings();
}

static void menu_help_about(int param) {
    widget_top_menu_clear_state();
    window_go_back();
    window_message_dialog_show(MESSAGE_DIALOG_ABOUT, -1, window_city_draw_all);
}

static void top_menu_help_handle(menu_item &item) {
    if (item.id == "help") { menu_help_help(0); }
    else if (item.id == "mouse") { menu_help_mouse_help(0); }
    else if (item.id == "warnings") { menu_help_warnings(0); }
    else if (item.id == "about") { menu_help_about(0); }
}

static void top_menu_advisors_handle(menu_item &item) {
    widget_top_menu_clear_state();
    window_go_back();
    window_advisors_show_advisor((e_advisor)item.parameter);
}

static void widget_top_menu_init() {
    auto& data = g_top_menu_data;
    auto *options = data.headers["options"].dcast_menu_header();
    if (options) {
        options->item(0).hidden = system_is_fullscreen_only();
        options->onclick(top_menu_options_handle);
    }

    auto *file = data.headers["file"].dcast_menu_header();
    if (file) {
        file->item("new_game").hidden = config_get(CONFIG_UI_HIDE_NEW_GAME_TOP_MENU);
        file->onclick(top_menu_file_handle);
    }

    auto *help = data.headers["help"].dcast_menu_header();
    if (help) {
        help->onclick(top_menu_help_handle);
    }

    auto *advisors = data.headers["advisors"].dcast_menu_header();
    if (advisors) {
        advisors->onclick(top_menu_advisors_handle);
    }

    auto *debug = data.headers["debug"].dcast_menu_header();
    if (debug) {
        debug->onclick(menu_debug_change_opt);
    }

    auto *render = data.headers["debug_render"].dcast_menu_header();
    if (render) {
        render->onclick(menu_debug_render_change_opt);
    }

    g_debug_show_opts[e_debug_show_properties] = game.debug_properties;

    set_text_for_autosave();
    set_text_for_tooltips();
    set_text_for_warnings();
    set_text_for_debug_city();
    set_text_for_debug_render();
}

static void top_menu_draw_background() {
    window_city_draw_panels();
    window_city_draw();
}

static void top_menu_draw_foreground() {
    auto& data = g_top_menu_data;
    if (!data.open_sub_menu) {
        return;
    }

    top_menu_menu_draw(data.open_sub_menu, data.focus_sub_menu_id);
}

void widget_top_menu_show() {
    static window_type window = {
        WINDOW_TOP_MENU,
        top_menu_draw_background,
        top_menu_draw_foreground,
        widget_top_menu_handle_input
    };
    widget_top_menu_init();
    window_show(&window);
}

int orientation_button_state = 0;
int orientation_button_pressed = 0;

static void refresh_background() {
    painter ctx = game.painter();
    int block_width = 96;
    int s_width = screen_width();

    int s_end = s_width - 1000 - 24 + city_view_is_sidebar_collapsed() * (162 - 18);
    int s_start = s_end - ceil((float)s_end / (float)block_width) * block_width;

    int img_id = image_group(g_top_menu_data.background);
    for (int i = 0; s_start + i * block_width < s_end; i++) {
        ImageDraw::img_generic(ctx, img_id, s_start + (i * block_width), COLOR_MASK_NONE);
    }

    ImageDraw::img_generic(ctx, img_id, s_end, 0);
}

void widget_top_menu_draw(int force) {
    OZZY_PROFILER_SECTION("Render/Frame/Window/City/Topmenu");
    auto& data = g_top_menu_data;
    if (!force && data.treasury == city_finance_treasury()
        && data.population == city_population() && data.month == gametime().month) {
        return;
    }

    refresh_background();
    top_menu_bar_draw();

    color treasure_color = COLOR_WHITE;
    int treasury = city_finance_treasury();

    if (treasury < 0) {
        treasure_color = COLOR_FONT_RED;
    }

    e_font treasure_font = (treasury >= 0 ? FONT_NORMAL_BLACK_ON_LIGHT : FONT_NORMAL_YELLOW);
    int s_width = screen_width();

    data.offset_funds = s_width - data.offset_funds_basic;
    data.offset_population = s_width - data.offset_population_basic;
    data.offset_date = s_width - data.offset_date_basic;
    data.offset_rotate = s_width - data.offset_rotate_basic;

    lang_text_draw_month_year_max_width(gametime().month, gametime().year, data.offset_date - 2, 5, 110, FONT_NORMAL_BLACK_ON_LIGHT, 0);
    // Orientation icon
    painter ctx = game.painter();
    if (orientation_button_pressed) {
        ImageDraw::img_generic(ctx, image_id_from_group(GROUP_SIDEBAR_BUTTONS) + 72 + orientation_button_state + 3, data.offset_rotate, 0);
        orientation_button_pressed--;
    } else {
        ImageDraw::img_generic(ctx, image_id_from_group(GROUP_SIDEBAR_BUTTONS) + 72 + orientation_button_state, data.offset_rotate, 0);
    }

    if (s_width < 800) {
        data.offset_funds = 338;      // +2
        data.offset_population = 453; // +5
        data.offset_date = 547;
        data.offset_rotate = data.offset_date - 50;

        int width = lang_text_draw_colored(6, 0, 341, 5, FONT_SMALL_PLAIN, treasure_color);
        text_draw_number_colored(treasury, '@', " ", 346 + width, 5, FONT_SMALL_PLAIN, treasure_color);

        width = lang_text_draw(6, 1, 458, 5, FONT_NORMAL_BLACK_ON_LIGHT);
        text_draw_number(city_population(), '@', " ", 450 + width, 5, FONT_NORMAL_BLACK_ON_LIGHT);

        lang_text_draw_month_year_max_width(gametime().month, gametime().year, 540, 5, 100, FONT_NORMAL_BLACK_ON_LIGHT, 0);
    } else if (s_width < 1024) {
        int width = lang_text_draw_colored(6, 0, data.offset_funds + 2 + 100, 5, treasure_font, 0);
        text_draw_number_colored(treasury, '@', " ", data.offset_funds + 7 + width + 100, 5, treasure_font, 0);

        width = lang_text_draw(6, 1, data.offset_population + 2 + 100, 5, FONT_NORMAL_BLACK_ON_LIGHT);
        text_draw_number(city_population(), '@', " ", data.offset_population + 7 + width + 100, 5, FONT_NORMAL_BLACK_ON_LIGHT);
    } else {
        int width = lang_text_draw_colored(6, 0, data.offset_funds + 2, 5, treasure_font, 0);
        text_draw_number_colored(treasury, '@', " ", data.offset_funds + 7 + width, 5, treasure_font, 0);

        width = lang_text_draw(6, 1, data.offset_population + 2, 5, FONT_NORMAL_BLACK_ON_LIGHT);
        text_draw_number(city_population(), '@', " ", data.offset_population + 7 + width, 5, FONT_NORMAL_BLACK_ON_LIGHT);
    }

    data.treasury = treasury;
    data.population = city_population();
    data.month = gametime().month;
}

static int get_info_id(vec2i m) {
    auto& data = g_top_menu_data;
    if (m.y < 4 || m.y >= 18)
        return INFO_NONE;

    if (m.x > data.offset_funds && m.x < data.offset_funds + 128)
        return INFO_FUNDS;

    if (m.x > data.offset_population && m.x < data.offset_population + 128)
        return INFO_POPULATION;

    if (m.x > data.offset_date && m.x < data.offset_date + 128)
        return INFO_DATE;

    if (m.x > data.offset_rotate && m.x < data.offset_rotate + 36) {
        if (m.x <= data.offset_rotate + 12)
            return -15;
        else if (m.x <= data.offset_rotate + 24)
            return -16;
        else
            return -14;
    }
    return INFO_NONE;
}

static bool widget_top_menu_handle_input_submenu(const mouse* m, const hotkeys* h) {
    auto& data = g_top_menu_data;
    if (m->right.went_up || h->escape_pressed) {
        widget_top_menu_clear_state();
        window_go_back();
        return true;
    }

    bstring64 menu_id = top_menu_bar_handle_mouse(m);
    if (!!menu_id && menu_id != data.open_sub_menu) {
        window_invalidate();
        data.open_sub_menu = menu_id;
    }

    auto *header = data.headers[data.open_sub_menu].dcast_menu_header();
    if (!top_menu_menu_handle_mouse(m, header ? &header->impl : nullptr, data.focus_sub_menu_id)) {
        if (m->left.went_up) {
            widget_top_menu_clear_state();
            window_go_back();
            return true;
        }
    }
    return false;
}

static bool handle_right_click(int type) {
    if (type == INFO_NONE)
        return false;

    if (type == INFO_FUNDS) {
        window_message_dialog_show(MESSAGE_DIALOG_TOP_FUNDS, -1, window_city_draw_all);
    } else if (type == INFO_POPULATION) {
        window_message_dialog_show(MESSAGE_DIALOG_TOP_POPULATION, -1, window_city_draw_all);
    } else if (type == INFO_DATE) {
        window_message_dialog_show(MESSAGE_DIALOG_TOP_DATE, -1, window_city_draw_all);
    }

    return true;
}

static bool widget_top_menu_handle_mouse_menu(const mouse* m) {
    auto& data = g_top_menu_data;
    bstring64 menu_id = top_menu_bar_handle_mouse(m);
    if (!!menu_id && m->left.went_up) {
        data.open_sub_menu = menu_id;
        widget_top_menu_show();
        return true;
    }

    if (m->right.went_up) {
        return handle_right_click(get_info_id(*m));
    }

    return false;
}

void widget_top_menu_handle_input(const mouse* m, const hotkeys* h) {
    auto& data = g_top_menu_data;
    int result = 0;
    if (!widget_city_has_input()) {
        int button_id = 0;
        int handled = false;

        handled = generic_buttons_handle_mouse(m, {data.offset_rotate, 0}, orientation_buttons_ph, 3, &button_id);
        if (button_id) {
            orientation_button_state = button_id;
            if (handled)
                orientation_button_pressed = 5;
        } else {
            orientation_button_state = 0;
        }

        if (button_id)
            result = handled;
        else if (!!data.open_sub_menu)
            widget_top_menu_handle_input_submenu(m, h);
        else
            widget_top_menu_handle_mouse_menu(m);
    }
}

int widget_top_menu_get_tooltip_text(tooltip_context* c) {
    auto& data = g_top_menu_data;
    //if (data.focus_menu_id)
    //    return 50 + data.focus_menu_id;
    //
    //int button_id = get_info_id(c->mpos);
    //if (button_id) {
    //    button_id += 1;
    //}
    //
    //if (button_id) {
    //    return 59 + button_id;
    //}

    return 0;
}