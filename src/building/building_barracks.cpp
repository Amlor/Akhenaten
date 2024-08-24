#include "building_barracks.h"

#include "building/count.h"
#include "building/model.h"
#include "city/buildings.h"
#include "city/military.h"
#include "city/city_resource.h"
#include "city/warnings.h"
#include "city/labor.h"
#include "core/calc.h"
#include "figure/action.h"
#include "figure/formation_legion.h"
#include "figure/figure.h"
#include "figure/formation.h"
#include "grid/grid.h"
#include "grid/road_access.h"
#include "config/config.h"

#include "game/game.h"
#include "graphics/elements/ui.h"
#include "graphics/graphics.h"
#include "graphics/window.h"
#include "graphics/image.h"

#define INFINITE 10000

int g_tower_sentry_request = 0;

buildings::model_t<building_recruiter> brecruiter_m;
info_window_recruiter brecruiter_infow;

ANK_REGISTER_CONFIG_ITERATOR(config_load_building_recruiter);
void config_load_building_recruiter() {
    brecruiter_m.load();
    brecruiter_infow.load("info_window_recruiter");
}

static int get_closest_legion_needing_soldiers(building* barracks) {
    int recruit_type = LEGION_RECRUIT_NONE;
    int min_formation_id = 0;
    int min_distance = INFINITE;
    for (int i = 1; i < MAX_FORMATIONS; i++) {
        formation* m = formation_get(i);
        if (!m->in_use || !m->is_legion)
            continue;

        if (m->in_distant_battle || m->legion_recruit_type == LEGION_RECRUIT_NONE)
            continue;

        if (m->legion_recruit_type == LEGION_RECRUIT_INFANTRY && barracks->stored_full_amount <= 0)
            continue;

        building* fort = building_get(m->building_id);
        int dist = calc_maximum_distance(barracks->tile, fort->tile);
        if (m->legion_recruit_type > recruit_type || (m->legion_recruit_type == recruit_type && dist < min_distance)) {
            recruit_type = m->legion_recruit_type;
            min_formation_id = m->id;
            min_distance = dist;
        }
    }
    return min_formation_id;
}

static int get_closest_military_academy(building* fort) {
    int min_building_id = 0;
    int min_distance = INFINITE;
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building* b = building_get(i);
        if (b->state == BUILDING_STATE_VALID && b->type == BUILDING_MILITARY_ACADEMY
            && b->num_workers >= model_get_building(BUILDING_MILITARY_ACADEMY)->laborers) {
            int dist = calc_maximum_distance(fort->tile, b->tile);
            if (dist < min_distance) {
                min_distance = dist;
                min_building_id = i;
            }
        }
    }
    return min_building_id;
}

bool building_recruiter::create_soldier() {
    if (base.stored_full_amount < 100) {
        return false;
    }

    int formation_id = get_closest_legion_needing_soldiers(&base);
    if (formation_id > 0) {
        const formation* m = formation_get(formation_id);
        figure* f = figure_create(m->figure_type, base.road_access, DIR_0_TOP_RIGHT);
        f->formation_id = formation_id;
        f->formation_at_rest = 1;
        if (base.stored_full_amount > 0) {
            base.stored_full_amount -= 100;
        }
        int academy_id = get_closest_military_academy(building_get(m->building_id));
        if (academy_id) {
            building* academy = building_get(academy_id);
            tile2i road = map_get_road_access_tile(academy->tile, academy->size);
            if (road.valid()) {
                f->advance_action(FIGURE_ACTION_85_SOLDIER_GOING_TO_MILITARY_ACADEMY);
                f->destination_tile = road;
            } else {
                f->advance_action(FIGURE_ACTION_81_SOLDIER_GOING_TO_FORT);
            }
        } else {
            f->advance_action(FIGURE_ACTION_81_SOLDIER_GOING_TO_FORT);
        }
    }
    formation_calculate_figures();
    return formation_id ? 1 : 0;
}

void building_barracks_request_tower_sentry() {
    g_tower_sentry_request = 2;
}

void building_barracks_decay_tower_sentry_request() {
    if (g_tower_sentry_request > 0)
        g_tower_sentry_request--;
}

int building_barracks_has_tower_sentry_request() {
    return g_tower_sentry_request;
}

void building_barracks_save_state(buffer* buf) {
    buf->write_i32(g_tower_sentry_request);
}
void building_barracks_load_state(buffer* buf) {
    g_tower_sentry_request = buf->read_i32();
}

int building_recruiter::get_priority() {
    return base.subtype.barracks_priority;
}

bool building_recruiter::create_tower_sentry() {
    if (g_tower_sentry_request <= 0)
        return false;

    building* tower = 0;
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building* b = building_get(i);
        if (b->state == BUILDING_STATE_VALID && b->type == BUILDING_MUD_TOWER && b->num_workers > 0 && !b->has_figure(0)
            && (b->road_network_id == base.road_network_id || config_get(CONFIG_GP_CH_TOWER_SENTRIES_GO_OFFROAD))) {
            tower = b;
            break;
        }
    }

    if (!tower) {
        return false;
    }

    figure* f = figure_create(FIGURE_TOWER_SENTRY, base.road_access, DIR_0_TOP_RIGHT);
    f->action_state = FIGURE_ACTION_174_TOWER_SENTRY_GOING_TO_TOWER;
    tile2i road = map_get_road_access_tile(tower->tile, tower->size);
    if (road.valid()) {
        f->destination_tile = road;
    } else {
        f->poof();
    }

    tower->set_figure(0, f->id);
    f->set_home(tower->id);
    return true;
}

void building_recruiter::on_create(int orientation) {
    city_buildings_add_recruiter(&base);
}

void building_recruiter::on_place_checks() {
    if (building_construction_has_warning()) {
        return;
    }

    if (city_resource_count(RESOURCE_WEAPONS) <= 0) {
        building_construction_warning_show(WARNING_WEAPONS_NEEDED);
    }
}

void building_recruiter::add_weapon(int amount) {
    assert(id() > 0);
    base.stored_full_amount += amount;
}

void building_recruiter::spawn_figure() {
    check_labor_problem();
    //    map_point road;
    if (base.has_road_access) {
        common_spawn_labor_seeker(100);
        int pct_workers = worker_percentage();
        int spawn_delay = figure_spawn_timer();
        //        if (pct_workers >= 100)
        //            spawn_delay = 8;
        //        else if (pct_workers >= 75)
        //            spawn_delay = 12;
        //        else if (pct_workers >= 50)
        //            spawn_delay = 16;
        //        else if (pct_workers >= 25)
        //            spawn_delay = 32;
        //        else if (pct_workers >= 1)
        //            spawn_delay = 48;
        //        else
        //            return;
        base.figure_spawn_delay++;
        if (base.figure_spawn_delay > spawn_delay) {
            base.figure_spawn_delay = 0;
            switch (base.subtype.barracks_priority) {
            case PRIORITY_FORT:
            if (!create_soldier())
                create_tower_sentry();
            break;

            default:
            if (!create_tower_sentry())
                create_soldier();
            }
        }
    }
}

void info_window_recruiter::window_info_background(object_info &c) {
    building_info_window::window_info_background(c);

    c.barracks_soldiers_requested = formation_legion_recruits_needed();
    c.barracks_soldiers_requested += building_barracks_has_tower_sentry_request();

    building_recruiter *b = c.building_get()->dcast_recruiter();

    int amount = b->base.stored_full_amount < 100 ? 0 : b->base.stored_full_amount;
    ui["storage_state"].text_var("%s ( %d )", ui::str(8, 10), amount);

    textid reason{ c.group_id, 0 };
    if (!c.has_road_access) { reason = { 69, 25 }; } else if (b->num_workers() <= 0) { reason.id = 3; } else if (!c.barracks_soldiers_requested) { reason.id = 4; } else {
        int offset = 0;
        if (b->base.stored_full_amount > 0) {
            offset = 4;
        }

        int workers_state = approximate_value(c.worker_percentage / 100.f, make_array(8, 7, 6, 5));
        ui["workers_text"] = ui::str(c.group_id, workers_state);
    }

    draw_employment_details(c);

    const int priority = b->get_priority();
    ui["tower_button"] = (priority == 0) ? "x" : "";
    ui["tower_button"].onclick([bid = c.building_id] {
        building *barracks = ::building_get(bid);
        barracks->subtype.barracks_priority = 0;
    });

    ui["fort_button"] = (priority == 1) ? "x" : "";
    ui["fort_button"].onclick([bid = c.building_id] {
        building *barracks = ::building_get(bid);
        barracks->subtype.barracks_priority = 1;
    });
}

inline bool info_window_recruiter::check(object_info &c) {
    return c.building_get()->dcast_recruiter();
}
