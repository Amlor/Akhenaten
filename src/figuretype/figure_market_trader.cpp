#include "figure_market_trader.h"

#include "building/building_bazaar.h"
#include "figuretype/figure_market_buyer.h"
#include "figure/service.h"

#include "js/js_game.h"

figures::model_t<figure_market_trader> market_trader_m;

ANK_REGISTER_CONFIG_ITERATOR(config_load_figure_market_trader);
void config_load_figure_market_trader() {
    market_trader_m.load();
}

void figure_market_trader::figure_action() {
    if (action_state() != FIGURE_ACTION_125_ROAMING) {
        return;
    }

    building_bazaar* bazaar = home()->dcast_bazaar();
    if (!bazaar) {
        return;
    }
    // force return on out of stock
    int stock = bazaar->max_food_stock() + bazaar->max_goods_stock();
    if (base.roam_length >= 96 && stock <= 0) {
        base.roam_length = base.max_roam_length;
    }
}

figure_sound_t figure_market_trader::get_sound_reaction(xstring key) const {
    return market_trader_m.sounds[key];
}

sound_key figure_market_trader::phrase_key() const {
    if (base.action_state == FIGURE_ACTION_126_ROAMER_RETURNING) {
        return "goods_are_finished";
    } else {
        return "we_are_selling_goods";
    }
}

void bazaar_coverage(building* b, figure *f, int &) {
    b->data.house.bazaar_access = MAX_COVERAGE;
}

int figure_market_trader::provide_service() {
    int none_service;
    int houses_serviced = provide_market_goods(home(), tile());
    figure_provide_service(tile(), &base, none_service, bazaar_coverage);
    return houses_serviced;
}

const animations_t &figure_market_trader::anim() const {
    return market_trader_m.anim;
}
