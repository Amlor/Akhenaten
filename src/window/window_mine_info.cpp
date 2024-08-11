#include "window/window_building_info.h"

#include "building/building_raw_material.h"
#include "city/object_info.h"
#include "city/city_resource.h"
#include "window/building/common.h"

struct info_window_raw_material : building_info_window {
    using building_info_window::load;
    virtual void load(archive arch, pcstr section) override {
        common_info_window::load(arch, section);
    }

    virtual void window_info_background(object_info &c) override;
    virtual bool check(object_info &c) override {
        building *b = building_get(c.building_id);
        return b->dcast_mine() 
                   || b->dcast_quarry()
                   || b->dcast_clay_pit()
                   || b->dcast_wood_cutter();
    }
};

info_window_raw_material raw_material_infow;

ANK_REGISTER_CONFIG_ITERATOR(config_load_raw_material_info_window);
void config_load_raw_material_info_window() {
    raw_material_infow.load("raw_material_info_window");
}

void info_window_raw_material::window_info_background(object_info &c) {
    building_info_window::window_info_background(c);

    building *b = building_get(c.building_id);
    window_building_play_sound(&c, b->get_sound());

    int pct_done = calc_percentage<int>(b->data.industry.progress, 200);
    ui["resource_img"].image(b->output_resource_first_id);
    ui["progress_desc"].text_var("%s %d %s", ui::str(c.group_id, 2), pct_done, ui::str(c.group_id, 3));

    std::pair<int, int> reason = { c.group_id, 10 };
    if (!c.has_road_access) { reason = { 69, 25 }; } 
    else if (city_resource_is_mothballed(b->output_resource_first_id)) reason.second = 4;
    else if (b->data.industry.curse_days_left > 4) reason.second = 11;
    else if (b->num_workers <= 0) reason.second = 5;
    else if (c.worker_percentage >= 100) reason.second = 6;
    else if (c.worker_percentage >= 75) reason.second = 7;
    else if (c.worker_percentage >= 50) reason.second = 8;
    else if (c.worker_percentage >= 25) reason.second = 9;

    draw_employment_details_ui(ui, c, b, -1);
    ui["workers_desc"] = ui::str(reason.first, reason.second);
    ui["mine_desc"] = ui::str(c.group_id, 1);
}
