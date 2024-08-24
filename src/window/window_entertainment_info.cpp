#include "window_building_info.h"

#include "building/building.h"
#include "city/object_info.h"
#include "window/building/common.h"

struct entertainment_info_window : public building_info_window {
    virtual void window_info_background(object_info &c) override;
    virtual bool check(object_info &c) override {
        building *b = c.building_get();
        return building_type_any_of(b->type, BUILDING_PAVILLION);
    }
};

struct info_window_bandstand : public building_info_window {
    virtual void window_info_background(object_info &c) override;
    virtual bool check(object_info &c) override {
        return c.building_get()->dcast_bandstand();
    }
};

struct info_window_booth : public building_info_window {
    virtual void window_info_background(object_info &c) override;
    virtual bool check(object_info &c) override {
        building *b = c.building_get();
        return c.building_get()->dcast_booth();
    }
};

entertainment_info_window entertainment_infow;
info_window_bandstand bandstand_infow;
info_window_booth booth_infow;

ANK_REGISTER_CONFIG_ITERATOR(config_load_entertainment_info_window);
void config_load_entertainment_info_window() {
    entertainment_infow.load("info_window_entertainment");
    bandstand_infow.load("info_window_bandstand");
    booth_infow.load("info_window_bandstand");
}

void entertainment_info_window::window_info_background(object_info &c) {
    c.go_to_advisor.first = ADVISOR_ENTERTAINMENT;

    building_info_window::window_info_background(c);

    building *b = c.building_get();

    std::pair<int, int> reason = {c.group_id, 6};
    if (!c.has_road_access) { reason = {69, 25}; }
    else if (b->num_workers <= 0) { reason.second = 7; }
    else { reason.second = approximate_value(c.worker_percentage / 100.f, make_array(5, 4, 3, 2)); }
 
    ui["warning_text"] = ui::str(reason.first, reason.second);
    draw_employment_details(c);
}

void info_window_bandstand::window_info_background(object_info &c) {
    building_info_window::window_info_background(c);

    building *b = c.building_get();

    textid reason{ c.group_id, 0 };
    if (!c.has_road_access) { reason = { 69, 25 }; } 
    else if (b->num_workers <= 0) { reason.id = 6; } 
    else if (!b->data.entertainment.num_shows) { reason.id = 2; }
    else if (b->data.entertainment.num_shows == 2) { reason.id = 3; } 
    else if (b->data.entertainment.days1) { reason.id = 4; } 
    else if (b->data.entertainment.days2) { reason.id = 5; }

    draw_employment_details(c);

    if (b->data.entertainment.days1 > 0) {
        ui["play_text"].text_var("%s %s %d", ui::str(c.group_id, 8), ui::str(8, 44), 2 * b->data.entertainment.days1);
    } else {
        ui["play_text"] = ui::str(c.group_id, 7);
    }
  
    if (b->data.entertainment.days2 > 0) {
        ui["play2_text"].text_var("%s %s %d %s", ui::str(c.group_id, 10), ui::str(8, 44), 2 * b->data.entertainment.days2, ui::str(c.group_id, 7 + b->data.entertainment.days3_or_play));
    } else {
        ui["play2_text"] = ui::str(c.group_id, 9);
    }
}

void info_window_booth::window_info_background(object_info &c) {
    building_info_window::window_info_background(c);

    building *b = c.building_get();

    textid reason{ c.group_id, 0 };
    if (!c.has_road_access) { reason = {69, 25}; } 
    else if (b->num_workers <= 0) { reason.id = 4; }
    else if (!b->data.entertainment.num_shows) { reason.id = 2; }
    else if (b->data.entertainment.days1) { reason.id = 3; }

    draw_employment_details(c);

    if (b->data.entertainment.days1 > 0) {
        ui["play_text"].text_var("%s %s %d %s", ui::str(c.group_id, 6), ui::str(8, 44), 2 * b->data.entertainment.days1, ui::str(c.group_id, 7 + b->data.entertainment.days3_or_play));
    } else {
        ui["play_text"] = ui::str(c.group_id, 5);
    }

    ui["play2_text"] = "";
}
