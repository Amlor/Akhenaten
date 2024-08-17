#include "building/building_festival_square.h"

#include "sound/sound_building.h"
#include "window/building/common.h"
#include "window/window_building_info.h"
#include "window/hold_festival.h"
#include "city/city.h"
#include "game/time.h"

struct festival_square_info_window : building_info_window {
    virtual void window_info_background(object_info &c) override;
    virtual bool check(object_info &c) override {
        return !!building_get(c.building_id)->dcast_festival_square();
    }
};

festival_square_info_window festival_square_infow;

ANK_REGISTER_CONFIG_ITERATOR(config_load_festival_square_info_window);
void config_load_festival_square_info_window() {
    festival_square_infow.load("festival_square_info_window");
}

void festival_square_info_window::window_info_background(object_info &c) {
    building_info_window::window_info_background(c);

    building *b = building_get(c.building_id);

    window_building_play_sound(&c, b->get_sound());

    ui["warning"] = ui::str(c.group_id, 1);
    ui["fest_months_last"].text_var("%d %s %s", g_city.festival.months_since_festival, ui::str(8, 5), ui::str(58, 15));

    if (g_city.festival.is_planned()) {
        int size = g_city.festival.selected_size();
        int months_left = g_city.festival.months_till_next();
        int planned_month = (gametime().month + months_left) % game_time_t::months_in_year;
        int festival_text_iffs[] = { 0, 10, 20, 31 };

        ui["hold_festival"].enabled = false;
        ui["planed_festival"].text_var("%s %s", ui::str(58, 34), ui::str(160, planned_month));
        ui["festival_advice"] = ui::str(295, festival_text_iffs[size] + months_left - 1);
    } else {
        ui["planed_festival"] = "";
        ui["hold_festival"].enabled = true;
        ui["hold_festival"] = ui::str(58, 16);
        ui["hold_festival"].onclick([] {
            if (!g_city.festival.is_planned()) {
                window_hold_festival_show(false);
            }
        });
        ui["festival_advice"] = ui::str(58, 18 + g_city.festival.get_advice());
    }
}