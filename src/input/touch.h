#ifndef INPUT_TOUCH_H
#define INPUT_TOUCH_H

#include "core/vec2i.h"
#include "core/time.h"

#define MAX_ACTIVE_TOUCHES 2

enum {
    TOUCH_MODE_ORIGINAL = 0, // original akhenaten touch mode
    TOUCH_MODE_TOUCHPAD = 1, // drag the pointer and tap-click like on a touchpad (default)
    TOUCH_MODE_DIRECT = 2,   // pointer jumps to finger but doesn't click on tap
    TOUCH_MODE_MAX = 3
};

typedef struct {
    int in_use;
    int has_started;
    int has_moved;
    int has_ended;
    vec2i start_point;
    vec2i current_point;
    vec2i previous_frame_point;
    vec2i frame_movement;
    vec2i last_movement;
    time_millis start_time;
    time_millis last_change_time;
} touch;

const touch* get_earliest_touch(void);
const touch* get_latest_touch(void);
int get_total_active_touches(void);

int touch_not_click(const touch* t);
int touch_was_click(const touch* t);
int touch_was_double_click(const touch* t);

int touch_is_scroll(void);
int touch_get_scroll(void);

void reset_touches(int reset_old_touch);

int touch_create(vec2i start_coords, time_millis start_time);
void touch_move(int index, vec2i current_coords, time_millis current_time);
void touch_end(int index, time_millis current_time);
int touch_in_use(int index);

int touch_to_mouse(void);

void touch_set_mode(int mode);
void touch_cycle_mode(void);

#endif // INPUT_TOUCH_H
