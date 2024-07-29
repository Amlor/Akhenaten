#include "figure_governor.h"
#include "figure_governor.h"
#include "figure_governor.h"

#include "city/buildings.h"
#include "grid/road_access.h"
#include "graphics/image.h"

#include "city/city.h"

#include "js/js_game.h"

figures::model_t<figure_governor> governor_m;

ANK_REGISTER_CONFIG_ITERATOR(config_load_figure_governor);
void config_load_figure_governor() {
    governor_m.load();
}

void figure_governor::figure_action() {
    switch (action_state()) {
    case FIGURE_ACTION_120_GOVERNOR_CREATED:
    {
        // if city has palace, all mugger will go there
        wait_ticks = 0;
        int senate_id = g_city.buildings.get_palace_id();
        building* b_dst = building_get(senate_id);
        tile2i road_tile = map_closest_road_within_radius(b_dst->tile, b_dst->size, 2);
        if (road_tile.valid()) {
            destination_tile = road_tile;
            set_destination(senate_id);
            advance_action(FIGURE_ACTION_121_GOVERNOR_MOVING);
            route_remove();
        } else {
            poof();
        }
    }
    break;

    case FIGURE_ACTION_121_GOVERNOR_MOVING:
        base.move_ticks(1);
        base.wait_ticks = 0;
        if (direction() == DIR_FIGURE_NONE) {
            poof();
        } else if (direction() == DIR_FIGURE_REROUTE || direction() == DIR_FIGURE_CAN_NOT_REACH) {
            advance_action(FIGURE_ACTION_120_GOVERNOR_CREATED);
            route_remove();
        }
    break;

    }

    base.wait_ticks++;
    if (wait_ticks > 200) {
        poof();
        base.anim.frame = 0;
    }
}

sound_key figure_governor::phrase_key() const {
    int nobles_in_city = 0;
    buildings_valid_do([&] (building &b) {
        if (!b.house_size || !b.house_population || b.subtype.house_level < BUILDING_HOUSE_COMMON_MANOR) {
            return;
        }
        nobles_in_city += b.house_population;
    });

    int nolbes_leave_city_pct = calc_percentage<int>(g_city.migration.nobles_leave_city_this_year, nobles_in_city);
    if (nolbes_leave_city_pct > 10) {
        return "governor_city_left_much_nobles";
    }

    if (g_city.festival.months_since_festival < 6) {
        return "governor_festival_was_near";
    }

    return {};
}

const animations_t &figure_governor::anim() const {
    return governor_m.anim;
}
