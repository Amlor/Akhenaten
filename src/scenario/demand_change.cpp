#include "demand_change.h"

#include "city/message.h"
#include "core/random.h"
#include "empire/empire.h"
#include "empire/empire_city.h"
#include "empire/trade_route.h"
#include "game/time.h"
#include "scenario/scenario.h"

void scenario_demand_change_init(void) {
    for (int i = 0; i < MAX_DEMAND_CHANGES; i++) {
        random_generate_next();
        if (g_scenario_data.demand_changes[i].year)
            g_scenario_data.demand_changes[i].month = (random_byte() & 7) + 2;
    }
}

void scenario_demand_change_process() {
    for (int i = 0; i < MAX_DEMAND_CHANGES; i++) {
        if (!g_scenario_data.demand_changes[i].year)
            continue;

        if (game_time_year() != g_scenario_data.demand_changes[i].year + g_scenario_data.start_year
            || game_time_month() != g_scenario_data.demand_changes[i].month) {
            continue;
        }

        int route = g_scenario_data.demand_changes[i].route_id;
        e_resource resource = g_scenario_data.demand_changes[i].resource;
        int city_id = g_empire.get_city_for_trade_route(route);
        city_id = std::max(0, city_id);

        auto &trade_route = g_empire.get_route(route);
        if (g_scenario_data.demand_changes[i].is_rise) {
            if (trade_route.increase_limit(resource) && g_empire.is_trade_route_open(route))
                city_message_post(true, MESSAGE_INCREASED_TRADING, city_id, resource);

        } else {
            if (trade_route.decrease_limit(resource) && g_empire.is_trade_route_open(route)) {
                if (trade_route.limit(resource) > 0)
                    city_message_post(true, MESSAGE_DECREASED_TRADING, city_id, resource);
                else {
                    city_message_post(true, MESSAGE_TRADE_STOPPED, city_id, resource);
                }
            }
        }
    }
}
