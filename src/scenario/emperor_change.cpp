#include "emperor_change.h"

#include "city/message.h"
#include "city/ratings.h"
#include "core/config.h"
#include "core/random.h"
#include "game/time.h"
#include "scenario/data.h"

static struct {
    int game_year;
    int month;
    int state;
} data;

void scenario_emperor_change_init(void) {
    data.game_year = scenario.start_year + scenario.emperor_change.year;
    data.month = 1 + (random_byte() & 7);
    data.state = 0;
}

void scenario_emperor_change_process(void) {
    if (!scenario.emperor_change.enabled)
        return;
    if (data.state == 0) {
        if (game_time_year() == data.game_year && game_time_month() == data.month) {
            data.state = 1; // done
            if (config_get(CONFIG_GP_FIX_EDITOR_EVENTS))
                city_ratings_reset_favor_emperor_change();

            city_message_post(1, MESSAGE_EMPEROR_CHANGE, 0, 0);
        }
    }
}

void scenario_emperor_change_save_state(buffer *time, buffer *state) {
    time->write_i32(data.game_year);
    time->write_i32(data.month);
    state->write_i32(data.state);
}

void scenario_emperor_change_load_state(buffer *time, buffer *state) {
    data.game_year = time->read_i32();
    data.month = time->read_i32();
    data.state = state->read_i32();
}
