#include "window_building_info.h"

#include "city/object_info.h"
#include "graphics/elements/lang_text.h"
#include "graphics/window.h"
#include "grid/building.h"
#include "building/distribution.h"
#include "building/building.h"
#include "building/culture.h"
#include "building/house_evolution.h"
#include "building/government.h"
#include "window/building/common.h"
#include "game/game.h"
#include "game/state.h"
#include "dev/debug.h"

void window_building_draw_burning_ruin(object_info* c) {
    c->help_id = 0;
    window_building_play_sound(c, "Wavs/ruin.wav");
    outer_panel_draw(c->offset, c->bgsize.x, c->bgsize.y);
    lang_text_draw_centered(111, 0, c->offset.x, c->offset.y + 10, 16 * c->bgsize.x, FONT_LARGE_BLACK_ON_LIGHT);

    lang_text_draw(41, c->rubble_building_type, c->offset.x + 32, c->offset.y + 16 * c->bgsize.y - 173, FONT_NORMAL_BLACK_ON_LIGHT);
    lang_text_draw_multiline(111, 1, c->offset + vec2i{32, 16 * c->bgsize.y - 143}, 16 * (c->bgsize.x - 4), FONT_NORMAL_BLACK_ON_LIGHT);
}

building_info_window::building_info_window() {
    window_info_register_handler(this);
}

int building_info_window::window_info_handle_mouse(const mouse *m, object_info &c) {
    building *b = building_get(c.building_id);
    switch (building_get(c.building_id)->type) {
    case BUILDING_STORAGE_YARD:
        if (c.storage_show_special_orders)
            return window_building_handle_mouse_warehouse_orders(m, &c);
        else
            return window_building_handle_mouse_warehouse(m, &c);
        break;

    default:
        return b->dcast()->window_info_handle_mouse(m, c);
    }
}

static void draw_native(object_info* c, int group_id) {
    c->help_id = 0;
    window_building_play_sound(c, "Wavs/empty_land.wav");
    outer_panel_draw(c->offset, c->bgsize.x, c->bgsize.y);
    lang_text_draw_centered(group_id, 0, c->offset.x, c->offset.y + 10, 16 * c->bgsize.x, FONT_LARGE_BLACK_ON_LIGHT);
    window_building_draw_description_at(c, 106, group_id, 1);
}

void window_building_draw_native_hut(object_info* c) {
    draw_native(c, 131);
}

void window_building_draw_native_meeting(object_info* c) {
    draw_native(c, 132);
}

void window_building_draw_native_crops(object_info* c) {
    draw_native(c, 133);
}


void building_info_window::window_info_foreground(object_info &c) {
    draw();

    building *b = building_get(c.building_id);
    b->dcast()->window_info_foreground(c);
}

void window_building_draw_mission_post(object_info* c) {
    c->help_id = 8;
    window_building_play_sound(c, "Wavs/mission.wav");
    outer_panel_draw(c->offset, c->bgsize.x, c->bgsize.y);
    lang_text_draw_centered(134, 0, c->offset.x, c->offset.y + 10, 16 * c->bgsize.x, FONT_LARGE_BLACK_ON_LIGHT);
    window_building_draw_description(c, 134, 1);
    inner_panel_draw(c->offset.x + 16, c->offset.y + 136, c->bgsize.x - 2, 4);
    window_building_draw_employment_without_house_cover(c, 142);
}

void building_info_window::common_info_background(object_info& c) {
    building_info_window::window_info_background(c);

    building* b = building_get(c.building_id);
    auto params = b->dcast()->params();

    window_building_play_sound(&c, b->get_sound()); // TODO: change to firehouse

    std::pair<int, int> reason = { c.group_id, 0 };
    std::pair<int, int> workers = { c.group_id, 8 };
    if (!c.has_road_access) {
        reason = { 69, 25 };
    } else if (!b->num_workers) {
        reason.second = 9;
    } else {
        reason.second = b->has_figure(0) ? 2 : 3;
        workers.second = approximate_value(c.worker_percentage / 100.f, make_array(4, 5, 6, 7));
    }

    ui["warning_text"] = ui::str(reason.first, reason.second);
    ui["workers_desc"] = ui::str(workers.first, workers.second);

    draw_employment_details_ui(ui, c, b, -1);
}

void building_info_window::window_info_background(object_info &c) {
    g_debug_building_id = c.building_id;
    building *b = building_get(c.building_id);

    switch (b->type) {
    case BUILDING_ORACLE: window_building_draw_oracle(&c); break;
    case BUILDING_RESERVED_TRIUMPHAL_ARCH_56: window_building_draw_triumphal_arch(&c); break;

    case BUILDING_BURNING_RUIN: window_building_draw_burning_ruin(&c); break;
    case BUILDING_UNUSED_NATIVE_HUT_88: window_building_draw_native_hut(&c); break;
    case BUILDING_UNUSED_NATIVE_MEETING_89: window_building_draw_native_meeting(&c); break;
    case BUILDING_UNUSED_NATIVE_CROPS_93: window_building_draw_native_crops(&c); break;
    case BUILDING_RESERVER_MISSION_POST_80: window_building_draw_mission_post(&c); break;

    default:
        b->dcast()->window_info_background(c);
        break;
    }

    c.worker_percentage = calc_percentage<int>(b->num_workers, model_get_building(b->type)->laborers);

    b->dcast()->highlight_waypoints();
    window_invalidate();

    switch (b->type) {
    case BUILDING_FORT_GROUND:
        c.building_id = b->main()->id;
        // fallthrough

    case BUILDING_FORT_ARCHERS:
    case BUILDING_FORT_CHARIOTEERS:
    case BUILDING_FORT_INFANTRY:
        c.formation_id = b->formation_id;
        break;

    case BUILDING_STORAGE_ROOM:
    case BUILDING_SENET_HOUSE:
    case BUILDING_TEMPLE_COMPLEX_OSIRIS:
    case BUILDING_TEMPLE_COMPLEX_RA:
    case BUILDING_TEMPLE_COMPLEX_PTAH:
    case BUILDING_TEMPLE_COMPLEX_SETH:
    case BUILDING_TEMPLE_COMPLEX_BAST:
        b = b->main();
        c.building_id = b->id;
        break;

    default:
        if (b->house_size) {
            c.worst_desirability_building_id = building_house_determine_worst_desirability_building(b);
            building_house_determine_evolve_text(b, c.worst_desirability_building_id);
        }
        break;
    }

    c.show_overlay = b->get_overlay();
    c.has_road_access = b->has_road_access;
    const auto &params = b->dcast()->params();
    c.help_id = params.meta.help_id;
    c.group_id = params.meta.text_id;

    ui["title"] = ui::str(c.group_id, 0);

    common_info_window::window_info_background(c);
}

std::pair<int, int> building_info_window::get_tooltip(object_info &c) {
    if (!c.storage_show_special_orders) {
        return {0, 0};
    }

    building *b = building_get(c.building_id);
    if (b->type == BUILDING_STORAGE_YARD) {
        return window_building_get_tooltip_warehouse_orders();
    }

    return b->dcast()->get_tooltip();
}

void building_info_window::update_buttons(object_info &c) {
    building *b = building_get(c.building_id);

    int workers_needed = model_get_building(b->type)->laborers;
    vec2i bgsize = ui["background"].pxsize();
    ui["mothball"].pos.y = bgsize.y - 40;
    ui["mothball"].enabled = workers_needed > 0;
    if (workers_needed) {
        ui["mothball"] = (b->state == BUILDING_STATE_VALID ? "x" : "");
        ui["mothball"].onclick([&c, b, workers_needed] {
            if (workers_needed) {
                building_mothball_toggle(b);
                window_invalidate();
            }
        });
        auto tooltip = (b->state == BUILDING_STATE_VALID) ? std::pair{54, 16} : std::pair{54, 17};
        ui["mothball"].tooltip(tooltip);
    }

    //ui::img_button(GROUP_CONTEXT_ICONS, vec2i(16 * c.bgsize.x - 40, y_offset + 16 * height_blocks - 40), {28, 28}, {4})
    //    .onclick([&c] (int, int) {
    //    if (c.storage_show_special_orders) {
    //        c.storage_show_special_orders = 0;
    //        storage_settings_backup_reset();
    //        window_invalidate();
    //    } else {
    //        window_city_show();
    //    }
    //});

    ui["show_overlay"].enabled = (c.show_overlay != OVERLAY_NONE);
    ui["show_overlay"] = (game.current_overlay != c.show_overlay ? "v" : "V");
    ui["show_overlay"].pos.y = bgsize.y - 40;
    ui["show_overlay"].onclick([&c] {
        if (game.current_overlay != c.show_overlay) {
            game_state_set_overlay((e_overlay)c.show_overlay);
        } else {
            game_state_reset_overlay();
        }
        window_invalidate();
    });

    common_info_window::update_buttons(c);
}
