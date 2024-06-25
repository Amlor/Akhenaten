#include "advisor_trade.h"

#include "graphics/image.h"
#include "graphics/graphics.h"
#include "graphics/elements/rich_text.h"
#include "graphics/elements/scrollbar.h"

#include "city/resource.h"
#include "game/resource.h"
#include "graphics/elements/ui.h"
#include "graphics/screen.h"
#include "graphics/window.h"
#include "city/city.h"
#include "empire/empire.h"
#include "window/window_empire.h"
#include "window/resource_settings.h"
#include "window/trade_prices.h"
#include "game/game.h"

ui::advisor_trade_window g_advisor_trade_window;

ANK_REGISTER_CONFIG_ITERATOR(config_load_advisor_trade);
void config_load_advisor_trade() {
    g_config_arch.r_section("advisor_trade_window", [] (archive arch) {
        g_advisor_trade_window.load(arch);
    });
}

static void button_prices(int param1, int param2);
static void button_empire(int param1, int param2);

static generic_button resource_buttons[] = {
    {400, 398, 200, 23, button_prices, button_none, 1, 0},
    {100, 398, 200, 23, button_empire, button_none, 1, 0},
};

int ui::advisor_trade_window::draw_background() {
    city_resource_determine_available();

    auto &ui = g_advisor_trade_window;
    ui["scrollbar"].onevent([] {
        window_invalidate();
    });
    ui["scrollbar"].max_value(city_resource_get_available()->size - 15);

    return 0;
}

void ui::advisor_trade_window::ui_draw_foreground() {
    auto &ui = g_advisor_trade_window;
    ui.begin_frame();
    ui.begin_widget(screen_dialog_offset());
    ui.draw();

    int scroll_position = ui["scrollbar"].value();

    const resources_list* list = city_resource_get_available();
    for (int i = scroll_position; i < list->size; i++) {
        int y_offset = ui["inner_panel"].pos.y + 22 * (i - scroll_position);
        e_resource resource = list->items[i];
        ui.icon({24, y_offset}, resource);

        e_font font_color = FONT_NORMAL_WHITE_ON_DARK;
        if (city_resource_is_mothballed(resource)) {
            font_color = FONT_NORMAL_YELLOW;
        }

        // resource name and amount in warehouses
        int res_count = city_resource_count(resource);
        int proper_quality = stack_proper_quantity(res_count, resource);
        const bool is_stockpiled = city_resource_is_stockpiled(resource);
        const bool is_mothballed = city_resource_is_mothballed(resource);

        ui.label(ui::str(23, resource), vec2i{46, y_offset}, font_color, UiFlags_LabelYCentered);
        ui.label(bstring32().printf("%u", proper_quality).c_str(), vec2i{152, y_offset}, font_color, UiFlags_LabelCentered, 60);

        // mothballed / stockpiled
        {
            bstring128 text;
            if (is_stockpiled) {
                text = ui::str(54, 3);
            } else if (is_mothballed) {
                text = ui::str(18, 5);
                font_color = FONT_NORMAL_YELLOW;
            }

            if (!!text) {
                ui.label(text.c_str(), vec2i{210, y_offset}, font_color, UiFlags_LabelCentered, 100);
            }
        }

        int trade_status = city_resource_trade_status(resource);
        int trade_amount = stack_proper_quantity(city_resource_trading_amount(resource), resource);
        std::pair<bstring64, e_font> text;
        switch (trade_status) {
        case TRADE_STATUS_NONE: {
            bool can_import = g_empire.can_import_resource(resource, true);
            bool can_export = g_empire.can_export_resource(resource, true);
            bool could_import = g_empire.can_import_resource(resource, false);
            bool could_export = g_empire.can_export_resource(resource, false);
            if (can_import && !can_export) text = {ui::str(54, 31), font_color};
            else if (!can_import && can_export) text = {ui::str(54, 32), font_color};
            else if (can_import && can_export) text = {ui::str(54, 33), font_color};
            else if (could_import && !could_export) text = {ui::str(54, 34), FONT_NORMAL_BLACK_ON_DARK};
            else if (!could_import && could_export) text = {ui::str(54, 35), FONT_NORMAL_BLACK_ON_DARK};
            else if (could_import && could_export) text = {ui::str(54, 36), FONT_NORMAL_BLACK_ON_DARK};
        }
                              break;

        case TRADE_STATUS_IMPORT:
        text = {bstring64().printf("%s %u", ui::str(54, 5), trade_amount),  font_color};
        break;

        case TRADE_STATUS_EXPORT:
        text = {bstring64().printf("%s %u", ui::str(54, 6), trade_amount), font_color};
        break;

        case TRADE_STATUS_IMPORT_AS_NEEDED:
        text = {ui::str(54, 37), font_color};
        break;

        case TRADE_STATUS_EXPORT_SURPLUS:
        text = {ui::str(54, 38), font_color};
        break;
        }

        ui.label(text.first.c_str(), vec2i{310, y_offset}, font_color);

        // update/draw buttons accordingly
        ui.button("", vec2i{20, y_offset}, vec2i{570, 20}, FONT_NORMAL_BLACK_ON_LIGHT, UiFlags_NoBody)
            .onclick([resource_index = i] (int, int) {
            auto resource = city_resource_get_available()->items[resource_index];
            window_resource_settings_show(resource);
        });
    }
}

void ui::advisor_trade_window::draw_foreground() {
    painter ctx = game.painter();

    // prices
    button_border_draw(98, 396, 200, 24, false);
    lang_text_draw_centered(54, 30, 100, 402, 200, FONT_NORMAL_BLACK_ON_LIGHT);

    // map
    button_border_draw(398, 396, 200, 24, false);
    lang_text_draw_centered(54, 2, 400, 402, 200, FONT_NORMAL_BLACK_ON_LIGHT);
}

int ui::advisor_trade_window::handle_mouse(const mouse* m) {
    int num_resources = std::min(city_resource_get_available()->size, 15);

    return generic_buttons_handle_mouse(m, 0, 0, resource_buttons, num_resources + 2, nullptr);
}

static void button_prices(int param1, int param2) {
    window_trade_prices_show();
}
static void button_empire(int param1, int param2) {
    window_empire_show();
}

int ui::advisor_trade_window::get_tooltip_text(void) {
   //if (focus_button_id == 1)
   //    return 108;
   //else if (focus_button_id == 2)
   //    return 42;
   //else if (focus_button_id)
   //    return 109;
   //else
   //    return 0;
    return 0;
}

advisor_window* ui::advisor_trade_window::instance() {
    return &g_advisor_trade_window;
}

void window_advisor_trade_draw_dialog_background(void) {
    //draw_background();
    //draw_foreground();
}
