#include "figure_labor_seeker.h"

#include "core/profiler.h"
#include "city/sentiment.h"
#include "city/labor.h"
#include "city/gods.h"
#include "city/city.h"
#include "figure/service.h"

#include "js/js_game.h"

struct labor_seeker_model : public figures::model_t<FIGURE_LABOR_SEEKER, figure_labor_seeker> {};
labor_seeker_model labor_seeker_m;

ANK_REGISTER_CONFIG_ITERATOR(config_load_figure_labor_seeker);
void config_load_figure_labor_seeker() {
    g_config_arch.r_section("figure_labor_seeker", [] (archive arch) {
        labor_seeker_m.anim.load(arch);
        labor_seeker_m.sounds.load(arch);
    });
}

void figure_labor_seeker::figure_action() {
    switch (action_state()) {
    case FIGURE_ACTION_150_ATTACK:
        base.figure_combat_handle_attack();
        break;

    case FIGURE_ACTION_149_CORPSE:
        base.figure_combat_handle_corpse();
        break;

    case FIGURE_ACTION_125_ROAMING:
        do_roam(TERRAIN_USAGE_ROADS, FIGURE_ACTION_126_ROAMER_RETURNING);
        break;

    case FIGURE_ACTION_126_ROAMER_RETURNING:
        do_returnhome(TERRAIN_USAGE_ROADS);
        break;
    }
}

void figure_labor_seeker::figure_before_action() {
    building *b = home();
    if (!b->is_valid()) {
        poof();
    }
}

sound_key figure_labor_seeker::phrase_key() const {
    svector<sound_key, 10> keys;
    if (city_sentiment_low_mood_cause() == LOW_MOOD_NO_JOBS) {
        keys.push_back("no_jobs");
    }

    if (g_city.labor.workers_needed >= 0) {
        keys.push_back("no_some_workers");
    }

    if (g_city.labor.workers_needed >= 10) {
        keys.push_back("need_workers");
    }

    if (g_city.labor.workers_needed >= 20) {
        keys.push_back("need_more_workers");
    }

    int houses_in_disease = 0;
    buildings_valid_do([&] (building &b) {
        if (!b.house_size || !b.house_population) {
            return;
        }
        houses_in_disease = (b.disease_days > 0) ? 1 : 0;
    });

    if (houses_in_disease > 0) {
        keys.push_back("disease_in_city");
    }

    if (formation_get_num_forts() < 1) {
        keys.push_back("city_not_safety_workers_leaving");
    }

    if (city_gods_least_mood() <= GOD_MOOD_INDIFIRENT) { // any gods in wrath
        keys.push_back("gods_are_angry");
    } else { // gods are good
        keys.push_back("city_is_amazing");
    }

    if (city_sentiment_low_mood_cause() == LOW_MOOD_NO_FOOD) {
        keys.push_back("no_food_in_city");
    }

    if (city_sentiment_low_mood_cause() == LOW_MOOD_LOW_WAGES) {
        keys.push_back("i_want_to_leave_city");
    }

    if (g_city.labor.unemployment_percentage >= 15) {
        keys.push_back("much_unemployments");
    }

    keys.push_back("i_looking_for_the_workers");
    int index = rand() % keys.size();
    return keys[index];
}

void labor_seeker_coverage(building* b, figure *f, int&) {
    // nothing here, the labor seeker works simply via the `houses_covered` variable
}

int figure_labor_seeker::provide_service() {
    int none_service;
    int houses_serviced = figure_provide_service(tile(), &base, none_service, labor_seeker_coverage);
    return houses_serviced;
}

figure_sound_t figure_labor_seeker::get_sound_reaction(pcstr key) const {
    return labor_seeker_m.sounds[key];
}

const animations_t &figure_labor_seeker::anim() const {
    return labor_seeker_m.anim;
}
