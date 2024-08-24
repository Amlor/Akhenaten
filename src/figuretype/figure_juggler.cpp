#include "figure_juggler.h"

#include "city/labor.h"
#include "figure/service.h"
#include "city/city.h"
#include "city/sentiment.h"
#include "figure/service.h"

#include "js/js_game.h"

figures::model_t<figure_juggler> juggler_m;

ANK_REGISTER_CONFIG_ITERATOR(config_load_figure_juggler);
void config_load_figure_juggler() {
    juggler_m.load();
}

void figure_juggler::update_shows() {
    building *b = destination();
    if (b->type == BUILDING_BOOTH) { // update show index for booths
        b->data.entertainment.days3_or_play++;
        if (b->data.entertainment.days3_or_play >= 5) {
            b->data.entertainment.days3_or_play = 0;
        }
    }

    b->data.entertainment.days1 = 32;
}

svector<e_building_type, 4> figure_juggler::allow_venue_types() const {
    return {BUILDING_BOOTH, BUILDING_BANDSTAND, BUILDING_PAVILLION};
}

sound_key figure_juggler::phrase_key() const {
    int enemies = g_city.figures.enemies;
    if (enemies > 0) {
        return "city_not_safety_workers_leaving";
    }

    svector<sound_key, 10> keys;
    uint32_t months_since_last_festival = g_city.religion.months_since_last_festival();
    if (months_since_last_festival < 6) {
        keys.push_back("i_like_festivals");
    }

    int houses_in_disease = 0;
    buildings_valid_do([&] (building &b) {
        if (!b.house_size || !b.house_population) {
            return;
        }
        houses_in_disease = (b.disease_days > 0) ? 1 : 0;
    });

    if (houses_in_disease > 0) {
        return "disease_in_city";
    }

    if (city_sentiment() < 30) {
        if (city_sentiment() < 20) {
            keys.push_back("city_verylow_sentiment");
        }
        keys.push_back("city_low_sentiment");
    }

    if (city_sentiment_low_mood_cause() == LOW_MOOD_NO_JOBS) {
        keys.push_back("much_unemployments");
    }

    if (city_sentiment_low_mood_cause() == LOW_MOOD_LOW_WAGES) {
        keys.push_back("salary_too_low");
    }

    if (g_city.religion.least_mood() <= GOD_MOOD_INDIFIRENT) { // any gods in wrath
        keys.push_back("gods_are_angry");
    } else {
        keys.push_back("gods_are_pleasures");
    }

    if (g_city.festival.months_since_festival > 6) {  // low entertainment
        keys.push_back("low_entertainment");
    }

    if (city_sentiment() > 40) {
        keys.push_back("city_is_good");
    }

    if (city_sentiment() > 90) {
        keys.push_back("city_is_amazing");
    }

    int index = rand() % keys.size();
    return keys[index];
}

void juggler_coverage(building* b, figure *f, int&) {
    b->data.house.juggler = MAX_COVERAGE;
}

int figure_juggler::provide_service() {
    int houses_serviced = 0;
    building* b = home();
    if (b->type == BUILDING_BOOTH) {
        houses_serviced = figure_provide_culture(tile(), &base, juggler_coverage);
    } else if (b->type == BUILDING_BANDSTAND) {
        houses_serviced = provide_entertainment(b->data.entertainment.days1 ? 2 : 1, bandstand_coverage);
    }
    return houses_serviced;
}

figure_sound_t figure_juggler::get_sound_reaction(xstring key) const {
    return juggler_m.sounds[key];
}

const animations_t &figure_juggler::anim() const {
    return juggler_m.anim;
}
