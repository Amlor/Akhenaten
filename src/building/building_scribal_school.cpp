#include "building_scribal_school.h"

#include "js/js_game.h"
#include "building/count.h"
#include "grid/road_access.h"
#include "figure/figure.h"
#include "building/building_education.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "city/labor.h"

#include "widget/city/ornaments.h"

struct scribal_school_model : public buildings::model_t<building_scribal_school> {
    vec2i papyrus;
    vec2i icon_res;
    vec2i text_res;
} scribal_school_m;

ANK_REGISTER_CONFIG_ITERATOR(config_load_scribal_school);
void config_load_scribal_school() {
    scribal_school_m.load([] (archive arch) {
        scribal_school_m.papyrus = arch.r_vec2i("papyrus_icon");
        scribal_school_m.icon_res = arch.r_vec2i("info_icon_res");
        scribal_school_m.text_res = arch.r_vec2i("info_text_res");
    });
}

void building_scribal_school::update_count() const {
    building_increase_type_count(type(), num_workers() > 0);
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

    tile2i road;
    if (map_get_road_access_tile(tile(), size(), road)) {
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
}

bool building_scribal_school::draw_ornaments_and_animations_height(painter &ctx, vec2i point, tile2i tile, color mask) {
    building_draw_normal_anim(ctx, point, &base, tile, scribal_school_m.anim["work"], mask);

    int amount = 0;
    amount = ceil((float)base.stored_amount() / 100.0) - 1;

    if (amount >= 0) {
        ImageDraw::img_generic(ctx, image_group(IMG_RESOURCE_PAPYRUS) + amount, point + scribal_school_m.papyrus, mask);
    }

    return true;
}
