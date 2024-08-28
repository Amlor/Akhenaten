#include "window_building_info.h"

#include "building/building.h"
#include "city/object_info.h"
#include "window/building/common.h"
#include "js/js_game.h"

struct health_info_window : public building_info_window {
    virtual void window_info_background(object_info &c) override;
    virtual bool check(object_info &c) override {
        building *b = c.building_get();
        return building_type_any_of(*b, BUILDING_APOTHECARY, BUILDING_PHYSICIAN, BUILDING_DENTIST, BUILDING_MORTUARY);
    }
};

health_info_window health_infow;

ANK_REGISTER_CONFIG_ITERATOR(config_load_health_info_window);
void config_load_health_info_window() {
    health_infow.load("info_window_health");
}

void health_info_window::window_info_background(object_info &c) {
    c.go_to_advisor.first = ADVISOR_HEALTH;

    building_info_window::window_info_background(c);
    building *b = c.building_get();

    e_figure_type ftype = FIGURE_NONE;
    switch (b->type) {
    case BUILDING_APOTHECARY: ftype = FIGURE_HERBALIST; break;
    case BUILDING_PHYSICIAN: ftype = FIGURE_PHYSICIAN; break;
    case BUILDING_DENTIST: ftype = FIGURE_DENTIST; break;
    case BUILDING_MORTUARY: ftype = FIGURE_EMBALMER; break;
    }

    std::pair<int, int> reason = {c.group_id, 6};

    if (!c.has_road_access) { reason = {69, 25}; }
    else if (ftype != FIGURE_NONE && b->has_figure_of_type(BUILDING_SLOT_SERVICE, ftype)) { reason.second = 1; } 
    else if (c.building_get()->num_workers <= 0) { reason.second = 2; }
    else { reason.second = approximate_value(c.worker_percentage / 100.f, make_array(5, 4, 3, 2)); }
 
    ui["warning_text"] = ui::str(reason.first, reason.second);
    draw_employment_details(c);
}
