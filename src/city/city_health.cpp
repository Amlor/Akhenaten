#include "city_health.h"

#include "core/profiler.h"
#include "core/calc.h"
#include "building/building_health.h"
#include "building/building_house.h"
#include "building/destruction.h"
#include "building/count.h"
#include "city/city.h"
#include "city/coverage.h"
#include "city/message.h"
#include "core/calc.h"
#include "core/random.h"
#include "game/tutorial.h"
#include "scenario/scenario.h"

void city_health_t::change(int amount) {
    value = calc_bound(value + amount, 0, 100);
}

void city_health_t::start_disease(int total_people, bool force, int plague_people) {
    if (!force && value >= 40) {
        return;
    }

    int chance_value = random_byte() & 0x3f;
    if (g_city.religion.bast_curse_active) {
        // force plague
        chance_value = 0;
        g_city.religion.bast_curse_active = false;
    }

    if (!force && (chance_value > 40 - value)) {
        return;
    }

    int sick_people = calc_adjust_with_percentage(total_people, 7 + (random_byte() & 3));
    if (plague_people > 0) {
        sick_people = plague_people;
    }

    if (sick_people <= 0) {
        return;
    }

    change(10);
    int people_to_plague = sick_people - num_mortuary_workers;
    if (people_to_plague <= 0) {
        city_message_post_with_popup_delay(MESSAGE_CAT_HEALTH_PROBLEM, false, MESSAGE_HEALTH_MALARIA_PROBLEM, 0, 0);
        return;
    }

    tutorial_on_disease();

    // kill people where has little common_health
    building *warn_building = nullptr;
    buildings_valid_do([&] (building &b) {
        if (people_to_plague <= 0 || !b.house_size || !b.house_population) {
            return;
        }

        if (b.common_health < 10) {
            warn_building = &b;
            people_to_plague -= b.house_population;
            building_mark_plague(&b);
        }
    });

    // kill people who don't have access to apothecary/physician
    buildings_valid_do([&] (building &b) {
        if (people_to_plague <= 0 || !b.house_size || !b.house_population) {
            return;
        }

        if (!(b.data.house.apothecary || b.data.house.physician)) {
            warn_building = &b;
            people_to_plague -= b.house_population;
            building_mark_plague(&b);
        }
    });

    // kill people in tents
    buildings_valid_do([&] (building &b) {
        if (people_to_plague <= 0 || !b.house_size || !b.house_population) {
            return;
        }

        if (b.subtype.house_level <= HOUSE_STURDY_HUT) {
            warn_building = &b;
            people_to_plague -= b.house_population;
            building_mark_plague(&b);
        }
    });

    // kill anyone
    buildings_valid_do([&] (building &b) {
        if (people_to_plague <= 0 || !b.house_size || !b.house_population) {
            return;
        }

        warn_building = &b;
        people_to_plague -= b.house_population;
        building_mark_plague(&b);
    });

    e_building_type btype = (warn_building ? warn_building->type : BUILDING_NONE);
    int grid_offset = (warn_building ? warn_building->tile.grid_offset() : 0);
    if (num_mortuary_workers > 0) {
        city_message_post_with_popup_delay(MESSAGE_CAT_HEALTH_PROBLEM, false, MESSAGE_HEALTH_DISEASE, btype, grid_offset);
    } else {
        city_message_post_with_popup_delay(MESSAGE_CAT_HEALTH_PROBLEM, false, MESSAGE_HEALTH_PLAGUE, btype, grid_offset);
    }
}

void city_health_t::update_coverage() {
    OZZY_PROFILER_SECTION("Game/Update/Health Coverage");
    int population = g_city.population.population;

    const auto &mortuary_params = building_mortuary::current_params();
    g_coverage.mortuary = std::min<int>(calc_percentage(mortuary_params.max_serve_clients * building_count_active(BUILDING_MORTUARY), population), 100);

    g_coverage.physician = std::min<int>(calc_percentage(1000 * building_count_active(BUILDING_PHYSICIAN), population), 100);
    g_coverage.dentist = std::min<int>(calc_percentage(1000 * building_count_active(BUILDING_DENTIST), population), 100);

    const auto &apothecary_params = building_apothecary::current_params();
    g_coverage.apothecary = std::min<int>(calc_percentage(apothecary_params.max_serve_clients * building_count_active(BUILDING_APOTHECARY), population), 100);
}


void city_health_t::update() {
    if (g_city.population.population < 200 || scenario_is_mission_rank(1) || scenario_is_mission_rank(2)) {
        value = 50;
        target_value = 50;
        return;
    }

    int total_population = 0;
    int healthy_population = 0;
    buildings_valid_do([&] (building &b) {
        if (!b.house_size || !b.house_population) {
            return;
        }

        total_population += b.house_population;
        if (b.subtype.house_level <= HOUSE_STURDY_HUT) {
            if (b.data.house.apothecary) {
                healthy_population += b.house_population;
            } else {
                healthy_population += b.house_population / 4;
            }
        } else if (b.data.house.physician) {
            if (b.house_days_without_food == 0) {
                healthy_population += b.house_population;
            } else {
                healthy_population += b.house_population / 4;
            }
        } else if (b.house_days_without_food == 0) {
            healthy_population += b.house_population / 4;
        }
    });

    target_value = calc_percentage(healthy_population, total_population);
    if (value < target_value) {
        value += 2;
        if (value > target_value) {
            value = target_value;
        }

    } else if (value > target_value) {
        value -= 2;
        if (value < target_value) {
            value = target_value;
        }
    }
    value = calc_bound(value, 0, 100);

    start_disease(total_population, false, 0);
}

void city_health_t::reset_mortuary_workers() {
    num_mortuary_workers = 0;
}

void city_health_t::add_mortuary_workers(int amount) {
    num_mortuary_workers += amount;
}
