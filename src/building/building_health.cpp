#include "building_health.h"

#include "building/building.h"
#include "building/count.h"
#include "city/object_info.h"
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
#include "city/city.h"
#include "js/js_game.h"

#include "dev/debug.h"

#include <iostream>

building_apothecary::static_params apothercary_m;
building_mortuary::static_params mortuary_m;

declare_console_command_p(plague_start, game_cheat_start_plague);
declare_console_command_p(plague_no, game_cheat_noplague);

void game_cheat_noplague(std::istream &is, std::ostream &os) {
    buildings_valid_do([&] (building &b) {
        if (!b.house_size || !b.house_population) {
            return;
        }
        building *main = b.main();
        main->disease_days = 0;
        main->has_plague = false;
    });
}

void game_cheat_start_plague(std::istream &is, std::ostream &os) {
    std::string args; is >> args;
    int plague_people = atoi(args.empty() ? "100" : args.c_str());

    int total_population = 0;
    buildings_valid_do([&] (building &b) {
        if (!b.house_size || !b.house_population) {
            return;
        }
        total_population += b.house_population;
    });
    g_city.health.start_disease(total_population, true, plague_people);
}

void building_apothecary::spawn_figure() {
    common_spawn_roamer(FIGURE_HERBALIST, 50, FIGURE_ACTION_62_HERBALIST_ROAMING);
    //    check_labor_problem();
    //    if (has_figure_of_type(FIGURE_DOCTOR))
    //        return;
    //    map_point road;
    //    if (map_has_road_access(x, y, size, &road)) {
    //        spawn_labor_seeker(50);
    //        int spawn_delay = figure_spawn_timer();
    //        if (spawn_delay == -1)
    //            return;
    //        figure_spawn_delay++;
    //        if (figure_spawn_delay > spawn_delay) {
    //            figure_spawn_delay = 0;
    //            create_roaming_figure(road.x, road.y, FIGURE_DOCTOR);
    //        }
    //    }
}

void building_apothecary::static_params::load(archive arch) {
    max_serve_clients = arch.r_int("max_serve_clients");
}

void building_apothecary::update_graphic() {
    const xstring &animkey = can_play_animation() ? animkeys().work : animkeys().none;
    set_animation(animkey);

    building_impl::update_graphic();
}

bool building_apothecary::draw_ornaments_and_animations_height(painter &ctx, vec2i point, tile2i tile, color color_mask) {
    draw_normal_anim(ctx, point, tile, color_mask);

    return true;
}

void building_mortuary::spawn_figure() {
    common_spawn_roamer(FIGURE_EMBALMER, 50, FIGURE_ACTION_125_ROAMING);
    //    check_labor_problem();
    //    if (has_figure_of_type(FIGURE_BARBER))
    //        return;
    //    map_point road;
    //    if (map_has_road_access(x, y, size, &road)) {
    //        spawn_labor_seeker(50);
    //        int spawn_delay = figure_spawn_timer();
    //        if (spawn_delay == -1)
    //            return;
    //        figure_spawn_delay++;
    //        if (figure_spawn_delay > spawn_delay) {
    //            figure_spawn_delay = 0;
    //            create_roaming_figure(road.x, road.y, FIGURE_BARBER);
    //        }
    //    }
}

void building_mortuary::update_graphic() {
    const xstring &animkey = can_play_animation() ? animkeys().work : animkeys().none;
    set_animation(animkey);

    building_impl::update_graphic();
}

bool building_mortuary::draw_ornaments_and_animations_height(painter &ctx, vec2i point, tile2i tile, color color_mask) {
    draw_normal_anim(ctx, point, tile, color_mask);

    return false;
}

void building_mortuary::update_count() const {
    g_city.health.add_mortuary_workers(num_workers());
}

void building_mortuary::static_params::load(archive arch) {
    max_serve_clients = arch.r_int("max_serve_clients");
}