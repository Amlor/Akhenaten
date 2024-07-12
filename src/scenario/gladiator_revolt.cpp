#include "gladiator_revolt.h"

#include "building/count.h"
#include "city/message.h"
#include "core/random.h"
#include "game/time.h"
#include "scenario/scenario.h"

static struct {
    int game_year;
    int month;
    int end_month;
    int state;
} data;

void scenario_gladiator_revolt_init(void) {
    data.game_year = g_scenario_data.start_year + g_scenario_data.gladiator_revolt.year;
    data.month = 3 + (random_byte() & 3);
    data.end_month = 3 + data.month;
    data.state = e_event_state_initial;
}

void scenario_gladiator_revolt_process(void) {
    if (!g_scenario_data.gladiator_revolt.enabled)
        return;
    if (data.state == e_event_state_initial) {
        if (game_time_year() == data.game_year && game_time_month() == data.month) {
            if (building_count_active(BUILDING_CONSERVATORY) > 0) {
                data.state = e_event_state_in_progress;
                city_message_post(true, MESSAGE_GLADIATOR_REVOLT, 0, 0);
            } else {
                data.state = e_event_state_finished;
            }
        }
    } else if (data.state == e_event_state_in_progress) {
        if (data.end_month == game_time_month()) {
            data.state = e_event_state_finished;
            city_message_post(true, MESSAGE_GLADIATOR_REVOLT_FINISHED, 0, 0);
        }
    }
}

int scenario_gladiator_revolt_is_in_progress(void) {
    return data.state == e_event_state_in_progress;
}

int scenario_gladiator_revolt_is_finished(void) {
    return data.state == e_event_state_finished;
}

void scenario_gladiator_revolt_save_state(buffer* buf) {
    buf->write_i32(data.game_year);
    buf->write_i32(data.month);
    buf->write_i32(data.end_month);
    buf->write_i32(data.state);
}

void scenario_gladiator_revolt_load_state(buffer* buf) {
    data.game_year = buf->read_i32();
    data.month = buf->read_i32();
    data.end_month = buf->read_i32();
    data.state = buf->read_i32();
}
