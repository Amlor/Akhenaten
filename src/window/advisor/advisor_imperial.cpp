#include "advisor_imperial.h"
#include "city/city_data.h"

#include "city/emperor.h"
#include "city/finance.h"
#include "city/military.h"
#include "city/ratings.h"
#include "city/resource.h"
#include "empire/empire_city.h"
#include "figure/formation_legion.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/elements/generic_button.h"
#include "graphics/elements/lang_text.h"
#include "graphics/elements/panel.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "scenario/property.h"
#include "scenario/request.h"
#include "window/donate_to_city.h"
#include "window/empire.h"
#include "window/gift_to_emperor.h"
#include "window/popup_dialog.h"
#include "window/set_salary.h"
#include "game/game.h"

#define ADVISOR_HEIGHT 27

enum E_STATUS {
    STATUS_NOT_ENOUGH_RESOURCES = -1,
    STATUS_CONFIRM_SEND_LEGIONS = -2,
    STATUS_NO_LEGIONS_SELECTED = -3,
    STATUS_NO_LEGIONS_AVAILABLE = -4,
};

ui::advisor_imperial_window g_advisor_imperial_window;

static void button_donate_to_city(int param1, int param2);
static void button_set_salary(int param1, int param2);
static void button_gift_to_emperor(int param1, int param2);
static void button_request(int index, int param2);

static generic_button imperial_buttons[] = {
  {320, 367, 250, 20, button_donate_to_city, button_none, 0, 0},
  {70, 393, 500, 20, button_set_salary, button_none, 0, 0},
  {320, 341, 250, 20, button_gift_to_emperor, button_none, 0, 0},
  {38, 96, 560, 40, button_request, button_none, 0, 0},
  {38, 138, 560, 40, button_request, button_none, 1, 0},
  {38, 180, 560, 40, button_request, button_none, 2, 0},
  {38, 222, 560, 40, button_request, button_none, 3, 0},
  {38, 264, 560, 40, button_request, button_none, 4, 0},
};

static int focus_button_id;

static int get_request_status(int index) {
    const scenario_request* request = scenario_request_get_visible(index);
    if (request) {
        if (request->resource == RESOURCE_DEBEN) {
            if (city_finance_treasury() <= request->amount) {
                return STATUS_NOT_ENOUGH_RESOURCES;
            }
        } else if (request->resource == RESOURCE_TROOPS) {
            if (city_military_months_until_distant_battle() > 0 && !city_military_distant_battle_kingdome_army_is_traveling_forth()) {
                if (city_military_total_legions() <= 0) {
                    return STATUS_NO_LEGIONS_AVAILABLE;
                } else if (city_military_empire_service_legions() <= 0) {
                    return STATUS_NO_LEGIONS_SELECTED;
                } else {
                    return STATUS_CONFIRM_SEND_LEGIONS;
                }
            }
        } else {
            if (city_resource_count((e_resource)request->resource) < request->get_resource_amount()) {
                return STATUS_NOT_ENOUGH_RESOURCES;
            }
        }
        return request->event_id;
    }
    return 0;
}

static void draw_request(int index, const scenario_request* request) {
    if (index >= 5) {
        return;
    }

    painter ctx = game.painter();
    button_border_draw(38, 96 + 42 * index, 560, 42, 0);
    int resource_offset = request->resource + resource_image_offset(request->resource, RESOURCE_IMAGE_ICON);
    ImageDraw::img_generic(ctx, image_id_resource_icon(resource_offset), 45, 103 + 42 * index);

    int request_amount = 0;
    if (request->resource == RESOURCE_DEBEN) {
        request_amount = request->amount;
    } else {
        request_amount = request->get_resource_amount();
    }

    int width = text_draw_number(stack_proper_quantity(request_amount, request->resource), '@', " ", 65, 102 + 42 * index, FONT_NORMAL_WHITE_ON_DARK);
    lang_text_draw(23, request->resource, 65 + width, 102 + 42 * index, FONT_NORMAL_WHITE_ON_DARK);

    width = lang_text_draw_amount(8, 4, request->months_to_comply, 310, 102 + 42 * index, FONT_NORMAL_WHITE_ON_DARK);
    lang_text_draw(12, 2, 310 + width, 102 + 42 * index, FONT_NORMAL_WHITE_ON_DARK);

    if (request->resource == RESOURCE_DEBEN) {
        // request for money
        int treasury = city_finance_treasury();
        width = text_draw_number(treasury, '@', " ", 40, 120 + 42 * index, FONT_NORMAL_WHITE_ON_DARK);
        width += lang_text_draw(52, 44, 40 + width, 120 + 42 * index, FONT_NORMAL_WHITE_ON_DARK);
        if (treasury < request->amount) {
            lang_text_draw(52, 48, 80 + width, 120 + 42 * index, FONT_NORMAL_WHITE_ON_DARK);
        } else {
            lang_text_draw(52, 47, 80 + width, 120 + 42 * index, FONT_NORMAL_WHITE_ON_DARK);
        }
    } else {
        // normal goods request
        int amount_stored = city_resource_count(request->resource);
        int request_amount = request->get_resource_amount();
        width = text_draw_number(amount_stored, '@', " ", 40, 120 + 42 * index, FONT_NORMAL_WHITE_ON_DARK);
        width += lang_text_draw(52, 43, 40 + width, 120 + 42 * index, FONT_NORMAL_WHITE_ON_DARK);
        if (amount_stored < request_amount) {
            lang_text_draw(52, 48, 80 + width, 120 + 42 * index, FONT_NORMAL_WHITE_ON_DARK);
        } else {
            lang_text_draw(52, 47, 80 + width, 120 + 42 * index, FONT_NORMAL_WHITE_ON_DARK);
        }
    }
}

int ui::advisor_imperial_window::draw_background() {
    int military_resource = RESOURCE_WEAPONS;

    painter ctx = game.painter();
    city_emperor_calculate_gift_costs();

    outer_panel_draw(vec2i{0, 0}, 40, ADVISOR_HEIGHT);
    ImageDraw::img_generic(ctx, image_id_from_group(GROUP_ADVISOR_ICONS) + 2, 10, 10);

    text_draw(city_player_name(), 60, 12, FONT_LARGE_BLACK_ON_LIGHT, 0);

    int width = lang_text_draw(52, 0, 60, 44, FONT_NORMAL_BLACK_ON_LIGHT);
    text_draw_number(city_rating_kingdom(), '@', " ", 60 + width, 44, FONT_NORMAL_BLACK_ON_LIGHT);

    lang_text_draw_multiline(52, city_rating_kingdom() / 5 + 22, vec2i{60, 60}, 544, FONT_NORMAL_BLACK_ON_LIGHT);

    inner_panel_draw(32, 90, 36, 14);

    int num_requests = 0;
    if (city_military_months_until_distant_battle() > 0
        && !city_military_distant_battle_kingdome_army_is_traveling_forth()) {
        // can send to distant battle
        button_border_draw(38, 96, 560, 40, 0);
        ImageDraw::img_generic(ctx, image_id_resource_icon(military_resource), 50, 106);
        width = lang_text_draw(52, 72, 80, 102, FONT_NORMAL_WHITE_ON_DARK);
        lang_text_draw(21, empire_city_get(city_military_distant_battle_city())->name_id, 80 + width, 102, FONT_NORMAL_WHITE_ON_DARK);
        int strength_text_id;
        int enemy_strength = city_military_distant_battle_enemy_strength();
        if (enemy_strength < 46)
            strength_text_id = 73;
        else if (enemy_strength < 89)
            strength_text_id = 74;
        else {
            strength_text_id = 75;
        }
        width = lang_text_draw(52, strength_text_id, 80, 120, FONT_NORMAL_WHITE_ON_DARK);
        lang_text_draw_amount(8, 4, city_military_months_until_distant_battle(), 80 + width, 120, FONT_NORMAL_WHITE_ON_DARK);
        num_requests = 1;
    }
    num_requests = scenario_request_foreach_visible(num_requests, draw_request);
    if (!num_requests) {
        lang_text_draw_multiline(52, 21, vec2i{64, 160}, 512, FONT_NORMAL_WHITE_ON_DARK);
    }

    return ADVISOR_HEIGHT;
}

void ui::advisor_imperial_window::draw_foreground() {
    inner_panel_draw(64, 324, 32, 6);

    lang_text_draw(32, city_emperor_rank(), 72, 338, FONT_LARGE_BLACK_ON_DARK);

    int width = lang_text_draw(52, 1, 72, 372, FONT_NORMAL_WHITE_ON_DARK);
    text_draw_money(city_emperor_personal_savings(), 80 + width, 372, FONT_NORMAL_WHITE_ON_DARK);

    //
    button_border_draw(320, 367, 250, 20, focus_button_id == 1);
    lang_text_draw_centered(52, 2, 320, 372, 250, FONT_NORMAL_WHITE_ON_DARK);

    // button set salary
    button_border_draw(70, 393, 500, 20, focus_button_id == 2);
    width = lang_text_draw(52, city_emperor_salary_rank() + 4, 120, 398, FONT_NORMAL_WHITE_ON_DARK);
    width += text_draw_number(city_emperor_salary_amount(), '@', " ", 120 + width, 398, FONT_NORMAL_WHITE_ON_DARK);
    lang_text_draw(52, 3, 120 + width, 398, FONT_NORMAL_WHITE_ON_DARK);

    button_border_draw(320, 341, 250, 20, focus_button_id == 3);
    lang_text_draw_centered(52, 49, 320, 346, 250, FONT_NORMAL_WHITE_ON_DARK);

    // Request buttons
    for (int i = 0; i < 5; i++) {
        if (get_request_status(i)) {
            button_border_draw(38, 96 + i * 42, 560, 42, focus_button_id == (4 + i));
        }
    }
}

int ui::advisor_imperial_window::handle_mouse(const mouse* m) {
    return generic_buttons_handle_mouse(m, 0, 0, imperial_buttons, 8, &focus_button_id);
}

static void button_donate_to_city(int param1, int param2) {
    window_donate_to_city_show();
}
static void button_set_salary(int param1, int param2) {
    window_set_salary_show();
}
static void button_gift_to_emperor(int param1, int param2) {
    window_gift_to_emperor_show();
}

static void confirm_nothing(bool accepted) {}

static void confirm_send_troops(bool accepted) {
    if (accepted) {
        formation_legions_dispatch_to_distant_battle();
        window_empire_show();
    }
}

static void button_request(int index, int param2) {
    int status = get_request_status(index);
    // in C3, the enums are offset by two! (I have not fixed this)
    if (status) {
        city_military_clear_empire_service_legions();
        switch (status) {
        case STATUS_NO_LEGIONS_AVAILABLE:
            window_ok_dialog_show("#popup_dialog_no_legions_available");
            break;
        case STATUS_NO_LEGIONS_SELECTED:
            window_ok_dialog_show("#popup_dialog_no_legions_selected");
            break;
        case STATUS_CONFIRM_SEND_LEGIONS:
            window_ok_dialog_show("#popup_dialog_send_troops");
            break;
        case STATUS_NOT_ENOUGH_RESOURCES:
            window_ok_dialog_show("#popup_dialog_not_enough_goods");
            break;
        default:
            window_yes_dialog_show("#popup_dialog_send_goods", [selected_request_id = index] {
                scenario_request_dispatch(selected_request_id);
            });
            break;
        }
    }
}

int ui::advisor_imperial_window::get_tooltip_text(void) {
    if (focus_button_id && focus_button_id <= 2)
        return 93 + focus_button_id;
    else if (focus_button_id == 3)
        return 131;
    else {
        return 0;
    }
}

advisor_window* ui::advisor_imperial_window::instance() {
    return &g_advisor_imperial_window;
}
