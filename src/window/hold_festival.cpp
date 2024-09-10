#include "hold_festival.h"

#include "city/constants.h"
#include "city/finance.h"
#include "city/city.h"
#include "core/game_environment.h"
#include "game/resource.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/elements/generic_button.h"
#include "graphics/elements/image_button.h"
#include "graphics/elements/lang_text.h"
#include "graphics/elements/panel.h"
#include "graphics/image_groups.h"
#include "graphics/window.h"
#include "input/input.h"
#include "window/advisors.h"
#include "window/message_dialog.h"
#include "window/window_city.h"
#include "game/game.h"
#include "js/js_game.h"

ui::hold_festival_window g_hold_festival_window;

void ui::hold_festival_window::close() {
    if (callback) {
        callback();
    } else {
        window_go_back();
        window_invalidate();
    }
}

void window_hold_festival_select_size(e_festival_type size) {
    if (city_finance_out_of_money()) {
        return;
    }

    if (g_city.festival.select_size(size)) {
        window_invalidate();
    }
}

int ui::hold_festival_window::draw_background() {     
    if (!background) {
        game.animation = false;
        window_city_draw_panels();
        window_city_draw();
    }

    ui["background_image"].enabled = background;
    ui["title"] = ui::str(58, 25 + g_city.festival.selected_god());

    int resource_image_deben = image_id_from_group(PACK_GENERAL, 103) + 18;
    ui["small_festival"].text_var("%s %u @I%u", ui::str(58, 31), g_city.festival.small_cost, resource_image_deben);
    ui["small_festival"].grayed = city_finance_out_of_money();
    ui["small_festival"].onclick([] {
        window_hold_festival_select_size(FESTIVAL_SMALL);
    });

    ui["middle_festival"].text_var("%s %u @I%u", ui::str(58, 32), g_city.festival.large_cost, resource_image_deben);
    ui["middle_festival"].grayed = city_finance_out_of_money();
    ui["middle_festival"].onclick([] { 
        window_hold_festival_select_size(FESTIVAL_LARGE); 
    });

    int resource_image_beer = image_id_resource_icon(RESOURCE_BEER);
    ui["large_festival"].grayed = city_finance_out_of_money() || g_city.festival.not_enough_alcohol;
    ui["large_festival"].text_var("%s %u @I%u %u  @I%u", ui::str(58, 32), g_city.festival.grand_cost, resource_image_deben, g_city.festival.grand_alcohol, resource_image_beer);
    ui["large_festival"].onclick([] {
        window_hold_festival_select_size(FESTIVAL_GRAND);
    });

    ui["button_ok"].onclick([] { 
        if (!city_finance_out_of_money()) {
            g_city.festival.schedule();
        }
        g_hold_festival_window.close();
    });

    ui["button_cancel"].onclick([] { 
        g_hold_festival_window.close();
    });

    ui["button_help"].onclick([] {
        window_message_dialog_show(MESSAGE_DIALOG_ADVISOR_ENTERTAINMENT, -1, 0);
    });

    for (e_god god = GOD_OSIRIS; god < MAX_GODS; ++god) {
        bstring32 god_id; god_id.printf("god%d", god);
        if (g_city.religion.is_god_known(god) == GOD_STATUS_UNKNOWN) {
            ui[god_id].select(false);
            ui[god_id].grayed = true;
            continue;
        }

        ui[god_id].select(god == g_city.festival.selected_god());
        ui[god_id].onclick([god] {
            g_city.festival.select_god(god);
            window_invalidate();
        });
    }

    ui["festival_type"] = ui::str(58, 30 + g_city.festival.selected_size());

    return 0;
}

int ui::hold_festival_window::ui_handle_mouse(const mouse *m) {
    autoconfig_window::ui_handle_mouse(m);

    const hotkeys *h = hotkey_state();

    if (input_go_back_requested(m, h)) {
        if (callback) {
            callback();
        } else {
            window_go_back();
            window_invalidate();
        }
    }

    return 0;
}

void ui::hold_festival_window::get_tooltip(tooltip_context* c) {
    //if (!focus_image_button_id && (!focus_button_id || focus_button_id > 5))
    //    return;
    return;

    //c->type = TOOLTIP_BUTTON;
    //// image buttons
    //switch (focus_image_button_id) {
    //case 1:
    //    c->text_id = 1;
    //    break;
    //case 2:
    //    c->text_id = 2;
    //    break;
    //case 3:
    //    c->text_id = 113;
    //    break;
    //case 4:
    //    c->text_id = 114;
    //    break;
    //}
    //// gods
    //switch (focus_button_id) {
    //case 1:
    //    c->text_id = 115;
    //    break;
    //case 2:
    //    c->text_id = 116;
    //    break;
    //case 3:
    //    c->text_id = 117;
    //    break;
    //case 4:
    //    c->text_id = 118;
    //    break;
    //case 5:
    //    c->text_id = 119;
    //    break;
    //}
}

void window_hold_festival_show(bool bg, std::function<void()> cb) {
    static window_type window = {
        WINDOW_HOLD_FESTIVAL,
        [] { g_hold_festival_window.draw_background(); },
        [] { g_hold_festival_window.ui_draw_foreground(); },
        [] (const mouse *m, const hotkeys *h) { g_hold_festival_window.ui_handle_mouse(m); },
        [] (tooltip_context *c) { g_hold_festival_window.get_tooltip(c); } 
    };

    g_hold_festival_window.callback = cb;
    g_hold_festival_window.background = bg;
    window_show(&window);
}
