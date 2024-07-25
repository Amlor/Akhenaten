#include "time.h"

#include "io/io_buffer.h"
#include "scenario/scenario.h"
#include "core/game_environment.h"

void game_time_t::init(int year) {
    tick = 0;
    day = 0;
    month = 0;
    total_days = 0;
    year = year;
}

int game_time_t::years_since_start() const {
    return year - scenario_property_start_year();
}

int game_time_t::absolute_day(bool since_start) const {
    int days = month * days_in_month + day;
    if (since_start)
        days += (days_in_month * months_in_year)/*192*/ * years_since_start();
    return days;
}

int game_time_t::absolute_tick(bool since_start) const {
    int ticks = absolute_day() * ticks_in_day + tick;

    if (since_start) {
        ticks += (ticks_in_day * days_in_month * months_in_year) /*9792*/ * years_since_start();
    }

    return ticks;
}

bool game_time_t::advance_tick() {
    if (++tick >= game_time_t::ticks_in_day) {
        tick = 0;
        return true;
    }
    return false;
}

bool game_time_t::advance_day() {
    total_days++;
    if (++day >= 16) {
        day = 0;
        return true;
    }
    return false;
}

bool game_time_t::advance_month() {
    if (++month >= 12) {
        month = 0;
        return true;
    }
    return false;
}

void game_time_t::advance_year() {
    ++year;
}

io_buffer* iob_game_time = new io_buffer([](io_buffer* iob, size_t version) {
    auto &data = (game_time_t&)gametime();
    iob->bind(BIND_SIGNATURE_INT32, &data.tick);
    iob->bind(BIND_SIGNATURE_INT32, &data.day);
    iob->bind(BIND_SIGNATURE_INT32, &data.month);
    iob->bind(BIND_SIGNATURE_INT32, &data.year);
    iob->bind(BIND_SIGNATURE_INT32, &data.total_days);
});
