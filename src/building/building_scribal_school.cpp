#include "building_scribal_school.h"

#include "js/js_game.h"
#include "core/calc.h"
#include "building/count.h"
#include "grid/road_access.h"
#include "figure/figure.h"
#include "building/building_education.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "city/labor.h"
#include "city/resource.h"
#include "city/warnings.h"
#include "city/city.h"
#include "empire/empire.h"

#include "widget/city/ornaments.h"

struct scribal_school_model : public buildings::model_t<building_scribal_school> {
    using inherited = buildings::model_t<building_scribal_school>;
    vec2i papyrus;
    vec2i icon_res;
    vec2i text_res;

    using inherited::load;
    virtual void load(archive arch) override {
        papyrus = arch.r_vec2i("papyrus_icon");
        icon_res = arch.r_vec2i("info_icon_res");
        text_res = arch.r_vec2i("info_text_res");
    }
};

scribal_school_model scribal_school_m;

ANK_REGISTER_CONFIG_ITERATOR(config_load_scribal_school);
void config_load_scribal_school() {
    scribal_school_m.load();
}

void building_scribal_school::update_month() {
    if (base.stored_full_amount <= 0) {
        return;
    }

    short want_spent = calc_adjust_with_percentage<short>(base.num_workers, 50);
    short spent = std::min(base.stored_full_amount, want_spent);
    base.stored_full_amount -= spent;
}

void building_scribal_school::on_place_checks() {
    if (building_count_industry_active(RESOURCE_PAPYRUS) > 0) {
        return;
    }
        
    if (city_resource_count(RESOURCE_PAPYRUS) > 0) {
        return;
    }

    building_construction_warning_show(WARNING_PAPYRUS_NEEDED);
    if (g_city.can_produce_resource(RESOURCE_PAPYRUS)) {
        building_construction_warning_show(WARNING_BUILD_PAPYRUS_MAKER);
    } else if (!g_empire.can_import_resource(RESOURCE_PAPYRUS, true)) {
        building_construction_warning_show(WARNING_INSTRUCT_OVERSEER_TO_IMPORT_PAPYRUS);
    } else if (city_resource_trade_status(RESOURCE_PAPYRUS) != TRADE_STATUS_IMPORT) {
        building_construction_warning_show(WARNING_OPEN_TRADE_TO_IMPORT_PAPYRUS);
    }
}

void building_scribal_school::on_create(int orientation) {

}

void building_scribal_school::window_info_background(object_info &c) {
    building_education_draw_info(c, "school_scribe", FIGURE_TEACHER, RESOURCE_PAPYRUS, scribal_school_m.icon_res, scribal_school_m.text_res);
}

void building_scribal_school::spawn_figure() {
    check_labor_problem();
    if (has_figure_of_type(BUILDING_SLOT_SERVICE, FIGURE_TEACHER)) {
        return;
    }

    tile2i road = map_get_road_access_tile(tile(), size());
    if (!road.valid()) {
        return;
    }

    common_spawn_labor_seeker(50);
    int spawn_delay = figure_spawn_timer();
    if (spawn_delay == -1) {
        return;
    }

    base.figure_spawn_delay++;
    if (base.figure_spawn_delay > spawn_delay) {
        base.figure_spawn_delay = 0;

        figure* f = figure_create(FIGURE_TEACHER, road, DIR_0_TOP_RIGHT);
        f->action_state = FIGURE_ACTION_125_ROAMING;
        f->set_home(id());
        base.set_figure(BUILDING_SLOT_SERVICE, f->id);
        f->init_roaming_from_building(0);
    }
}

void building_scribal_school::update_day() {
    set_animation(can_play_animation() ? animkeys().work : animkeys().none);
}

bool building_scribal_school::draw_ornaments_and_animations_height(painter &ctx, vec2i point, tile2i tile, color mask) {
    building_impl::draw_ornaments_and_animations_height(ctx, point, tile, mask);

    int amount = ceil((float)base.stored_amount() / 100.0) - 1;
    if (amount >= 0) {
        ImageDraw::img_generic(ctx, image_group(IMG_RESOURCE_PAPYRUS) + amount, point + scribal_school_m.papyrus, mask);
    }

    return true;
}
