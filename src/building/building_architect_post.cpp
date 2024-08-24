#include "building_architect_post.h"

#include "building/building.h"
#include "core/svector.h"
#include "building/destruction.h"
#include "city/object_info.h"
#include "city/labor.h"
#include "game/resource.h"
#include "graphics/elements/panel.h"
#include "graphics/elements/lang_text.h"
#include "graphics/graphics.h"
#include "io/gamefiles/lang.h"
#include "config/config.h"
#include "window/building/common.h"
#include "window/building/figures.h"
#include "sound/sound_building.h"
#include "widget/city/ornaments.h"
#include "dev/debug.h"

#include <iostream>

declare_console_command_p(nodamage, console_command_nodamage);
declare_console_command_p(collapse, console_command_collapse);

buildings::model_t<building_architect_post> architect_post_m;
info_window_architect_post architect_post_infow;

ANK_REGISTER_CONFIG_ITERATOR(config_load_building_architect_post);
void config_load_building_architect_post() {
    architect_post_m.load();
    architect_post_infow.load("building_info_window");
}

void console_command_nodamage(std::istream &, std::ostream &) {
    buildings_valid_do([&] (building &b) {
        b.damage_risk = 0;
    });
}

void console_command_collapse(std::istream &is, std::ostream &) {
    std::string args;
    is >> args;
    int count = atoi(!args.empty() ? args.c_str() : "10");

    svector<building *, 1000> buildings;
    buildings_valid_do([&] (building &b) {
        if (!b.dcast_farm()) {
            buildings.push_back(&b);
        }
    });

    int step = std::max<int>(1, (int)buildings.size() / count);
    for (int i = 0; i < buildings.size(); i += step) {
        building_destroy_by_collapse(buildings[i]);
    }
}

void building_architect_post::spawn_figure() {
    common_spawn_roamer(FIGURE_ARCHITECT, 50, FIGURE_ACTION_60_ENGINEER_CREATED);
}

void building_architect_post::update_graphic() {
    const xstring& animkey = can_play_animation()
        ? animkeys().work
        : animkeys().none;
    set_animation(animkey);
    building_impl::update_graphic();
}

bool building_architect_post::draw_ornaments_and_animations_height(painter &ctx, vec2i point, tile2i tile, color color_mask) {
    draw_normal_anim(ctx, point, tile, color_mask);

    return true;
}

void info_window_architect_post::window_info_background(object_info &c) {
    building_info_window::common_info_background(c);
}

inline bool info_window_architect_post::check(object_info& c) {
    return c.building_get()->dcast_architect_post();
}