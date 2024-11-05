#include "building/building_bazaar.h"

#include "graphics/elements/ui.h"
#include "window/building/common.h"
#include "graphics/image.h"
#include "graphics/graphics.h"
#include "graphics/window.h"
#include "city/city.h"
#include "window/window_building_info.h"
#include "window/building/distribution.h"
#include "sound/sound_building.h"
#include "figure/figure.h"
#include "game/game.h"
#include "js/js_game.h"

void window_bazaar_orders_show(object_info &c);

struct bazaar_info_window : public building_info_window_t<bazaar_info_window> {
    virtual void init(object_info &c) override;
    virtual bool check(object_info &c) override {
        return c.building_get()->type == BUILDING_BAZAAR;
    }
};

bazaar_info_window g_bazaar_infow;

void bazaar_info_window::init(object_info &c) {
    building_info_window::init(c);

    auto bazaar = c.building_get()->dcast_bazaar();
    if (!bazaar) {
        return;
    }

    window_building_play_sound(&c, bazaar->get_sound());

    const auto &meta = bazaar->get_info();
    textid reason = { 0, 0 };
    if (!c.has_road_access) {
        reason = { 69, 25 };
    } else if (bazaar->num_workers() <= 0) {
        reason = { meta.text_id, 2 };
    }

    if (reason.group) {
        ui["workers_desc"] = "";
        ui["workers_text"] = ui::str(reason);
        return;
    }

    fill_employment_details(c);

    int image_id = image_id_resource_icon(0);
    auto &data = bazaar->data;
    if (data.market.inventory[0] || data.market.inventory[1] || data.market.inventory[2] || data.market.inventory[3]) {
        figure *buyer = bazaar->get_figure(BUILDING_SLOT_MARKET_BUYER);
        figure *trader = bazaar->get_figure(BUILDING_SLOT_SERVICE);
        if (buyer->is_valid() && trader->is_valid()) {
            ui["warning_text"] = ui::str(meta.text_id, 1);
        } else if (buyer->is_valid()) {
            ui["warning_text"] = ui::str(meta.text_id, 10);
        } else if (trader->is_valid()) {
            int state = (trader->action_state == FIGURE_ACTION_126_ROAMER_RETURNING) ? 12 : 11;
            ui["warning_text"] = ui::str(meta.text_id, state);
        }
    } else {
        ui["warning_text"] = ui::str(meta.text_id, 4);
    }

    // food stocks
    for (int i = 0; i < bazaar->allow_food_types(); ++i) {
        bstring32 id_icon; id_icon.printf("food%u_icon", i);
        bstring32 id_text; id_text.printf("food%u_text", i);

        e_resource food_res = g_city.allowed_foods(i);
        ui[id_icon].image(food_res);
        ui[id_text].font(bazaar->is_good_accepted(i) ? FONT_NORMAL_BLACK_ON_LIGHT : FONT_NORMAL_YELLOW);
        ui[id_text].text_var(food_res ? "%u" : "", data.market.inventory[INVENTORY_FOOD1 + i]);
    }

    // good stocks
    for (int i = 0; i < bazaar->allow_good_types(); ++i) {
        bstring32 id_icon; id_icon.printf("good%u_icon", i);
        bstring32 id_text; id_text.printf("good%u_text", i);

        e_resource good_res = INV_RESOURCES[i];
        ui[id_icon].image(good_res);
        ui[id_text].font(bazaar->is_good_accepted(INVENTORY_GOOD1 + i) ? FONT_NORMAL_BLACK_ON_LIGHT : FONT_NORMAL_YELLOW);
        ui[id_text].text_var(good_res ? "%u" : "", data.market.inventory[INVENTORY_GOOD1 + i]);
    }

    ui["orders"].onclick([&c] {
        window_bazaar_orders_show(c);
    });
}