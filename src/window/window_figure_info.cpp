#include "window_figure_info.h"

#include "figure/figure.h"
#include "graphics/window.h"
#include "grid/figure.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "game/game.h"
#include "dev/debug.h"
#include "window/message_dialog.h"
#include "window/building/figures.h"
#include "window/window_city.h"
#include "game/game.h"
#include "game/state.h"
#include "sound/sound.h"

declare_console_var_int(figure_small_image_x, -32)
declare_console_var_int(figure_small_image_y, -48)

struct figure_small_image {
    int image_id = 0;

    figure_small_image(figure_small_image &o) {
        this->image_id = o.image_id;
        o.image_id = 0;
    }

    figure_small_image(int figure_id) {
        painter ctx = game.painter();
        vec2i coord = { 0, 0 };
        tile2i camera_tile = city_view_get_camera_mappoint();

        int grid_offset = figure_get(figure_id)->tile.grid_offset();
        widget_city_draw_for_figure(ctx, figure_id, &coord);

        image_id = graphics_save_to_texture(-1, coord + vec2i{ figure_small_image_x(), figure_small_image_y()}, {48, 48});
    }

    ~figure_small_image() {
        if (image_id) {
            graphics_delete_saved_texture(image_id);
        }
    }
};

struct figures_data_t {
    svector<figure_small_image, 7> figure_images;
    int focus_button_id;
    object_info* context_for_callback;
};

figures_data_t g_figures_data;

void figure_info_window::prepare_figures(object_info &c) {
    if (c.nfigure.ids.size() <= 0) {
        return;
    }

    auto &data = g_figures_data;

    data.figure_images.clear();

    for (const auto &id: c.nfigure.ids) {
        data.figure_images.emplace_back(id);
    }
    //        if (config_get(CONFIG_UI_ZOOM))
    //            graphics_set_active_canvas(CANVAS_CITY);
    //
    //        for (int i = 0; i < c->figure.count; i++) {
    //            draw_figure_in_city(c->figure.figure_ids[i], &coord);
    //            graphics_save_to_buffer(coord.x - 25, coord.y - 45, 48, 48, data.figure_images[i]);
    //        }
    //        graphics_set_active_canvas(CANVAS_UI);
    painter ctx = game.painter();
    widget_city_draw(ctx);
}

void figure_info_window::play_figure_phrase(object_info &c) {
    if (!c.can_play_sound) {
        return;
    }

    figure* f = c.figure_get();
    f->figure_phrase_play();
    c.show_overlay = f->dcast()->get_overlay();
    c.nfigure.phrase = f->phrase;
    c.nfigure.phrase_key = f->phrase_key;
    c.can_play_sound = false;
}

figure_info_window::figure_info_window() {
    window_figure_register_handler(this);
}

inline void figure_info_window::window_info_foreground(object_info &c) {
    draw();

    figure *f = c.figure_get();
    g_debug_figure_id = c.figure_get_id();

    bool custom_window = f->dcast()->window_info_background(c);
    if (custom_window) {
        return;
    }

    c.nfigure.drawn = 1;
}

void figure_info_window::window_info_background(object_info &c) {
    common_info_window::window_info_background(c);

    int figure_id = c.nfigure.ids[c.nfigure.selected_index];
    figure *f = ::figure_get(figure_id);

    for (auto &w : ui.elements) {
        bstring1024 formated_text;
        formated_text = common_info_window::format(f->dcast(), w->format().c_str());
        if (!formated_text.empty()) {
            w->text(formated_text);
        }
    }

    ui["show_path"] = (f->draw_debug_mode ? "P" : "p");

    for (int i = 0; i < c.nfigure.ids.size(); i++) {
        bstring64 btn_id; btn_id.printf("button_figure%d", i);
        ui[btn_id].select(i == c.nfigure.selected_index);
    }

    if (c.can_play_sound) {
        play_figure_phrase(c);
    }

    e_overlay foverlay = f->dcast()->get_overlay();
    ui["show_overlay"].enabled = (foverlay != OVERLAY_NONE);
    ui["show_overlay"] = (game.current_overlay != foverlay ? "v" : "V");    
}

int figure_info_window::window_info_handle_mouse(const mouse *m, object_info &c) {
    return 0;
}

void figure_info_window::init(object_info &c) {
    common_info_window::init(c);

    int figure_id = c.nfigure.ids[c.nfigure.selected_index];
    figure *f = ::figure_get(figure_id);

    prepare_figures(c);
    c.nfigure.draw_debug_path = 1;

    int image_id = f->type;
    if (f->action_state == FIGURE_ACTION_74_FIREMAN_GOING_TO_FIRE || f->action_state == FIGURE_ACTION_75_FIREMAN_AT_FIRE) {
        image_id = 18;
    }
    ui["bigimage"].image(image_id);

    for (int i = 0; i < c.nfigure.ids.size(); i++) {
        bstring64 btn_id; btn_id.printf("button_figure%d", i);
        ui[btn_id].select(i == c.nfigure.selected_index);
        ui[btn_id].onclick([index = i, &c] {
            auto &data = g_figures_data;
            data.context_for_callback = &c;
            data.context_for_callback->nfigure.selected_index = index;
            data.context_for_callback->can_play_sound = true;
        });

        auto screen_opt = ui[btn_id].dcast_image_button();
        if (screen_opt) {
            screen_opt->texture_id = g_figures_data.figure_images[i].image_id;
        }
    }

    const auto &meta = figure_impl::params(f->type).meta;
    c.help_id = meta.help_id;
    c.group_id = meta.text_id;

    ui["phrase"] = c.nfigure.phrase.valid()
        ? c.nfigure.phrase.c_str_safe("")
        : bstring256().printf("#undefined_phrase ( %s )", c.nfigure.phrase_key.c_str()).c_str();

    e_overlay foverlay = f->dcast()->get_overlay();
    ui["show_overlay"].onclick([foverlay] {
        if (game.current_overlay != foverlay) {
            game_state_set_overlay((e_overlay)foverlay);
        } else {
            game_state_reset_overlay();
        }
    });

    ui["show_path"].onclick([f] {
        f->draw_debug_mode = f->draw_debug_mode ? 0 : FIGURE_DRAW_DEBUG_ROUTING;
    });
}

bool figure_info_window::check(object_info &c) {
    return false;
}

figure *figure_info_window::figure_get(object_info &c) {
    int figure_id = c.grid_offset ? map_figure_id_get(c.grid_offset) : 0;
    return ::figure_get(figure_id);
}