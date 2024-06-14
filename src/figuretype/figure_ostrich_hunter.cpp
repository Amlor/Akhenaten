#include "figure_ostrich_hunter.h"

#include "core/calc.h"
#include "city/sentiment.h"
#include "figure/properties.h"
#include "grid/figure.h"
#include "graphics/image_groups.h"
#include "figuretype/maintenance.h"
#include "core/random.h"

#include "js/js_game.h"

struct ostrich_hunter_model : public figures::model_t<figure_ostrich_hunter> {
    using inherited = figures::model_t<figure_ostrich_hunter>;

    using inherited::load;
    virtual void load(archive arch) {
        max_hunting_distance = arch.r_int("max_hunting_distance");
    }

    int max_hunting_distance;
};
ostrich_hunter_model ostrich_hunter_m;

ANK_REGISTER_CONFIG_ITERATOR(config_load_figure_ostrich_hunter);
void config_load_figure_ostrich_hunter() {
    ostrich_hunter_m.load();
}

static void scared_animals_in_area(tile2i center, int size) {
    map_grid_area_foreach(center, size, [] (tile2i tile) {
        figure *f = map_figure_get(tile);
        if (f && f->is_alive() && f->type == FIGURE_OSTRICH) {
            f->advance_action(ACTION_8_RECALCULATE);
        }
    });
}

void figure_ostrich_hunter::figure_action() {
    figure* prey = figure_get(base.target_figure_id);
    int dist = 0;
    if (base.target_figure_id) {
        dist = calc_maximum_distance(tile(), prey->tile);
    }

    if (tile() == base.previous_tile) {
        base.movement_ticks_watchdog++;
    } else {
        base.movement_ticks_watchdog = 0;
    }

    if (base.movement_ticks_watchdog > 60) {
        base.movement_ticks_watchdog = 0;
        route_remove();
        advance_action(ACTION_8_RECALCULATE);
    }

    switch (action_state()) {
    case ACTION_8_RECALCULATE:
        base.target_figure_id = base.is_nearby(NEARBY_ANIMAL, &dist, ostrich_hunter_m.max_hunting_distance, false);
        if (base.target_figure_id) {
            figure_get(base.target_figure_id)->targeted_by_figure_id = id();
            advance_action(ACTION_9_CHASE_PREY);
        } else {
            advance_action(ACTION_16_HUNTER_INVESTIGATE);
            tile2i base_tile;
            int figure_id = base.is_nearby(NEARBY_ANIMAL, &dist, 10000, /*gang*/true);
            if (figure_id) {
                base_tile = figure_get(figure_id)->tile;
            } else {
                base_tile = home()->tile;
            }
            destination_tile = random_around_point(base_tile, tile(), /*step*/4, /*bias*/8, /*max_dist*/32);
        }
        break;

    case ACTION_16_HUNTER_INVESTIGATE:
        do_goto(destination_tile, TERRAIN_USAGE_ANIMAL, ACTION_8_RECALCULATE, ACTION_8_RECALCULATE);
        if (direction() == DIR_FIGURE_CAN_NOT_REACH) {
            base.direction = DIR_0_TOP_RIGHT;
            advance_action(ACTION_8_RECALCULATE);
        }
        break;

    case ACTION_13_WAIT_FOR_ACTION: // pitpat
        if (!base.target_figure_id) {
            return advance_action(ACTION_8_RECALCULATE);
        }

        wait_ticks--;
        if (wait_ticks <= 0) {
            advance_action(ACTION_9_CHASE_PREY);
        }
        break;

    case ACTION_9_CHASE_PREY: // following prey
        if (!base.target_figure_id) {
            return advance_action(ACTION_8_RECALCULATE);
        }

        if (dist >= 2) {
            do_goto(prey->tile, TERRAIN_USAGE_ANIMAL, ACTION_15_HUNTER_HUNT, ACTION_8_RECALCULATE);
        } else {
            wait_ticks = figure_properties_for_type(FIGURE_HUNTER_ARROW)->missile_delay;
            advance_action(ACTION_15_HUNTER_HUNT);
        }
        break;

    case ACTION_15_HUNTER_HUNT: // firing at prey
        wait_ticks--;
        if (wait_ticks <= 0) {
            if (!base.target_figure_id) {
                return advance_action(ACTION_8_RECALCULATE);
            }
            wait_ticks = figure_properties_for_type(FIGURE_HUNTER_ARROW)->missile_delay;
            if (prey->state == FIGURE_STATE_DYING) {
                advance_action(ACTION_11_HUNTER_WALK);
                scared_animals_in_area(prey->tile, /*dist*/16);
            } else if (dist >= 2) {
                wait_ticks = 12;
                advance_action(ACTION_13_WAIT_FOR_ACTION);
            } else {
                base.direction = calc_missile_shooter_direction(tile(), prey->tile);
                base.missile_fire_at(base.target_figure_id, FIGURE_HUNTER_ARROW);
            }
        }
        break;

    case ACTION_11_GOING_TO_PICKUP_POINT: // going to pick up prey
        if (!base.target_figure_id) {
            return advance_action(ACTION_8_RECALCULATE);
        }

        if (do_goto(prey->tile, TERRAIN_USAGE_ANIMAL, ACTION_10_PICKUP_ANIMAL, ACTION_11_GOING_TO_PICKUP_POINT)) {
            base.anim.offset = 0;
        }
        break;

    case ACTION_10_PICKUP_ANIMAL: // picking up prey
        if (base.target_figure_id) {
            prey->poof();
        }

        base.target_figure_id = 0;
        if (base.anim.frame >= 17) {
            advance_action(ACTION_12_GOING_HOME_AND_UNLOAD);
        }
        break;

    case ACTION_12_GOING_HOME_AND_UNLOAD:                                     // returning with prey
        if (do_returnhome(TERRAIN_USAGE_ANIMAL)) { // add game meat to hunting lodge!
            home()->stored_full_amount += 100;
        }

        break;
    }
}

sound_key figure_ostrich_hunter::phrase_key() const {
    if (action_state() == ACTION_16_HUNTER_INVESTIGATE || action_state() == ACTION_9_CHASE_PREY || action_state() == ACTION_15_HUNTER_HUNT) {
        return "hunting";
    } else if (action_state() == ACTION_8_RECALCULATE ) {
        if (city_sentiment() > 40) {
            return "city_is_good";
        }
    } 
    
    return "back";
}

figure_sound_t figure_ostrich_hunter::get_sound_reaction(pcstr key) const {
    return ostrich_hunter_m.sounds[key];
}

const animations_t &figure_ostrich_hunter::anim() const {
    return ostrich_hunter_m.anim;
}

void figure_ostrich_hunter::update_animation() {
    switch (action_state()) {
    case ACTION_14_RETURNING_WITH_FOOD:
    case ACTION_9_CHASE_PREY:
    case ACTION_11_HUNTER_WALK: // normal walk
        image_set_animation("walk");
        break;

    case ACTION_8_RECALCULATE:
    case ACTION_13_WAIT_FOR_ACTION:
        image_set_animation("fight");
        break;

    case ACTION_15_HUNTER_HUNT: // hunting
        image_set_animation("hunt");
        break;
        //        case ??: // attacking
        //            image_set_animation(GROUP_FIGURE_HUNTER, 200, 12);
        //        case ??: // attacking w/ prey on his back
        //            image_set_animation(GROUP_FIGURE_HUNTER, 296, 12);
    case ACTION_15_HUNTER_PACK:
        image_set_animation("pack");
        break;

    case ACTION_15_HUNTER_MOVE_PACKED:
        image_set_animation("move_pack");
        break;
    }
}

void figure_ostrich_hunter::figure_before_action() {
    building* b = home();
    if (b->state != BUILDING_STATE_VALID) {
        poof();
    }
}
