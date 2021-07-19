#include <graphics/image.h>
#include <tgmath.h>
#include "slide.h"

#include "core/speed.h"
#include "graphics/graphics.h"
#include "graphics/menu.h"
#include "graphics/window.h"
#include "sound/effect.h"
#include "widget/sidebar/common.h"
#include "window/city.h"

#define SLIDE_SPEED 7
#define SLIDE_ACCELERATION_MILLIS 65
#define SIDEBAR_DECELERATION_OFFSET 125

static struct {
    int position;
    speed_type slide_speed;
    int direction;
    back_sidebar_draw_function back_sidebar_draw;
    front_sidebar_draw_function front_sidebar_draw;
    slide_finished_function finished_callback;
} data;

#include "core/game_environment.h"
#include "graphics/screen.h"

static void draw_sliding_foreground(void) {
    window_request_refresh();
    data.position += speed_get_delta(&data.slide_speed);
    if (data.position >= SIDEBAR_EXPANDED_WIDTH[GAME_ENV]) {
        data.finished_callback();
        return;
    }

    int x_offset = sidebar_common_get_x_offset_expanded();
//    graphics_set_clip_rectangle(x_offset, 0, SIDEBAR_EXPANDED_WIDTH[GAME_ENV], sidebar_common_get_height()); //TOP_MENU_HEIGHT[GAME_ENV]

    int rel_offset = 0;
    if (data.direction == SLIDE_DIRECTION_IN) {
        if (data.position > SIDEBAR_DECELERATION_OFFSET)
            speed_set_target(&data.slide_speed, 1, SLIDE_ACCELERATION_MILLIS, 1);
        rel_offset = SIDEBAR_EXPANDED_WIDTH[GAME_ENV] - data.position;
    } else
        rel_offset = data.position;
    x_offset += rel_offset;

    if (GAME_ENV == ENGINE_ENV_PHARAOH) {
        int block_width = 96;
        int s_end = screen_width() - 1000 - 24 + (rel_offset <= 162 - 18 ? rel_offset : 162 - 18);

        int s_start = s_end - ceil((float) s_end / (float) block_width) * block_width;
        for (int i = 0; s_start + i * block_width < s_end; i++)
            image_draw(image_id_from_group(GROUP_SIDE_PANEL) + 8, s_start + (i * block_width), 0);
        image_draw(image_id_from_group(GROUP_SIDE_PANEL) + 8, s_end, 0);
    }

    data.back_sidebar_draw();
    data.front_sidebar_draw(x_offset);

//    graphics_reset_clip_rectangle();
}

void sidebar_slide(int direction, back_sidebar_draw_function back_sidebar_callback,
                   front_sidebar_draw_function front_sidebar_callback, slide_finished_function finished_callback) {
    data.direction = direction;
    data.position = 0;
    speed_clear(&data.slide_speed);
    speed_set_target(&data.slide_speed, SLIDE_SPEED,
                     direction == SLIDE_DIRECTION_OUT ? SLIDE_ACCELERATION_MILLIS : SPEED_CHANGE_IMMEDIATE, 1);
    data.back_sidebar_draw = back_sidebar_callback;
    data.front_sidebar_draw = front_sidebar_callback;
    data.finished_callback = finished_callback;
    sound_effect_play(SOUND_EFFECT_SIDEBAR);

    window_type window = {
            WINDOW_SLIDING_SIDEBAR,
            window_city_draw,
            draw_sliding_foreground,
            0,
            0
    };
    window_show(&window);
}
