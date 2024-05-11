#include "scenario.h"
#include "io/io_buffer.h"
#include "city/city.h"
#include "city/gods.h"

#include "city/resource.h"
#include "empire/trade_route.h"
#include "events.h"
#include "game/difficulty.h"
#include "game/settings.h"
#include "scenario/scenario.h"

#include "js/js_game.h"

scenario_data_t g_scenario_data;

ANK_REGISTER_CONFIG_ITERATOR(config_load_scenario_load_meta_data);
void config_load_scenario_load_meta_data() {
    scenario_load_meta_data(g_scenario_data.settings.campaign_scenario_id);
}

bool scenario_is_saved() {
    return g_scenario_data.is_saved;
}

void scenario_settings_init() {
    g_scenario_data.settings.campaign_scenario_id = 0;
    g_scenario_data.settings.campaign_mission_rank = 0;
    g_scenario_data.settings.is_custom = 0;
    g_scenario_data.settings.starting_kingdom = difficulty_starting_kingdom();
    g_scenario_data.settings.starting_personal_savings = 0;
}

void scenario_settings_init_mission() {
    g_scenario_data.settings.starting_kingdom = difficulty_starting_kingdom();
    g_scenario_data.settings.starting_personal_savings = g_settings.personal_savings_for_mission(g_scenario_data.settings.campaign_mission_rank);
}

void scenario_load_meta_data(int scenario_id) {
    bstring128 missionid;
    missionid.printf("mission%d", scenario_id);
    g_config_arch.r_section(missionid.c_str(), [] (archive arch) {
        g_scenario_data.meta.start_message = arch.r_int("start_message");
        g_scenario_data.env.has_animals = arch.r_bool("city_has_animals");

        memset(g_scenario_data.allowed_buildings, 0, sizeof(g_scenario_data.allowed_buildings));
        auto buildings = arch.r_array_num<e_building_type>("buildings");
        for (const auto &b : buildings) {
            g_scenario_data.allowed_buildings[b] = true;
        }

        g_scenario_data.building_stages.clear();
        arch.r_objects("stages", [](pcstr key, archive stage_arch) {
            auto buildings = archive::r_array_num<e_building_type>(stage_arch);
            g_scenario_data.building_stages.push_back({key, buildings});
        });
    });
}

bool scenario_building_allowed(int building_type) {
    return g_scenario_data.allowed_buildings[building_type];
}

int scenario_building_image_native_hut() {
    return g_scenario_data.native_images.hut;
}

int scenario_building_image_native_meeting() {
    return g_scenario_data.native_images.meeting;
}

int scenario_building_image_native_crops() {
    return g_scenario_data.native_images.crops;
}


// fancy lambdas! probably gonna create many problems down the road. :3
io_buffer* iob_scenario_mission_id = new io_buffer([](io_buffer* iob, size_t version) {
    iob->bind(BIND_SIGNATURE_INT8, &g_scenario_data.settings.campaign_scenario_id);
});

int scenario_is_custom() {
    return g_scenario_data.settings.is_custom;
}
void scenario_set_custom(int custom) {
    g_scenario_data.settings.is_custom = custom;
}
int scenario_campaign_rank(void) {
    return g_scenario_data.settings.campaign_mission_rank;
}
void scenario_set_campaign_rank(int rank) {
    g_scenario_data.settings.campaign_mission_rank = rank;
}
int scenario_campaign_scenario_id() {
    return g_scenario_data.settings.campaign_scenario_id;
}
void scenario_set_campaign_scenario(int scenario_id) {
    g_scenario_data.settings.campaign_scenario_id = scenario_id;
}

bool scenario_is_mission_rank(int rank) {
    return !g_scenario_data.settings.is_custom && g_scenario_data.settings.campaign_mission_rank == rank - 1;
}
int scenario_is_tutorial_before_mission_5() {
    return !g_scenario_data.settings.is_custom && g_scenario_data.settings.campaign_mission_rank < 5;
}

int scenario_starting_kingdom(void) {
    return g_scenario_data.settings.starting_kingdom;
}
int scenario_starting_personal_savings(void) {
    return g_scenario_data.settings.starting_personal_savings;
}

const uint8_t* scenario_name(void) {
    return g_scenario_data.scenario_name;
}
void scenario_set_name(const uint8_t* name) {
    string_copy(name, g_scenario_data.scenario_name, MAX_SCENARIO_NAME);
}

int scenario_is_open_play(void) {
    return g_scenario_data.is_open_play;
}
int scenario_open_play_id(void) {
    return g_scenario_data.open_play_scenario_id;
}

int scenario_property_climate(void) {
    return g_scenario_data.climate;
}

int scenario_property_start_year(void) {
    return g_scenario_data.start_year;
}
int scenario_property_kingdom_supplies_grain(void) {
    return g_scenario_data.kingdom_supplies_grain;
}
int scenario_property_enemy(void) {
    return g_scenario_data.enemy_id;
}
int scenario_property_player_rank(void) {
    return g_scenario_data.player_rank;
}
int scenario_image_id(void) {
    return g_scenario_data.image_id;
}

const uint8_t* scenario_subtitle() {
    return g_scenario_data.subtitle;
}

int scenario_initial_funds() {
    return g_scenario_data.initial_funds;
}
int scenario_rescue_loan() {
    return g_scenario_data.rescue_loan;
}

int scenario_property_monuments_is_enabled(void) {
    return (g_scenario_data.monuments.first > 0 || g_scenario_data.monuments.second > 0
            || g_scenario_data.monuments.third > 0);
}
int scenario_property_monument(int field) {
    switch (field) {
    case 0:
        return g_scenario_data.monuments.first;
    case 1:
        return g_scenario_data.monuments.second;
    case 2:
        return g_scenario_data.monuments.third;
    }
    return -1;
}

void scenario_set_monument(int field, int m) {
    switch (field) {
    case 0:
        g_scenario_data.monuments.first = m;
        break;
    case 1:
        g_scenario_data.monuments.second = m;
        break;
    case 2:
        g_scenario_data.monuments.third = m;
        break;
    }
}

io_buffer* iob_scenario_info = new io_buffer([](io_buffer* iob, size_t version) {
    iob->bind(BIND_SIGNATURE_INT16, &g_scenario_data.start_year);
    iob->bind____skip(2);
    iob->bind(BIND_SIGNATURE_INT16, &g_scenario_data.empire.id);
    iob->bind(BIND_SIGNATURE_UINT8, &g_scenario_data.meta.start_message_shown);
    iob->bind____skip(3);
    for (int i = 0; i < MAX_GODS; i++) {
        iob->bind(BIND_SIGNATURE_INT16, &g_city.religion.gods[i].is_known);
    }
    iob->bind____skip(10);
    iob->bind____skip(2); // 2 bytes ???        03 00

    iob->bind(BIND_SIGNATURE_INT32, &g_scenario_data.initial_funds);
    iob->bind(BIND_SIGNATURE_INT16, &g_scenario_data.enemy_id);
    iob->bind____skip(6);
    iob->bind(BIND_SIGNATURE_INT32, &g_scenario_data.map.width);
    iob->bind(BIND_SIGNATURE_INT32, &g_scenario_data.map.height);
    iob->bind(BIND_SIGNATURE_INT32, &g_scenario_data.map.start_offset);
    iob->bind(BIND_SIGNATURE_INT32, &g_scenario_data.map.border_size);
    iob->bind(BIND_SIGNATURE_RAW, &g_scenario_data.subtitle, MAX_SUBTITLE);
    iob->bind(BIND_SIGNATURE_RAW, &g_scenario_data.brief_description, MAX_BRIEF_DESCRIPTION);

    iob->bind(BIND_SIGNATURE_INT16, &g_scenario_data.image_id);
    iob->bind(BIND_SIGNATURE_INT16, &g_scenario_data.is_open_play);
    iob->bind(BIND_SIGNATURE_INT16, &g_scenario_data.player_rank);

    for (int i = 0; i < MAX_PREDATOR_HERD_POINTS; i++)
        iob->bind(BIND_SIGNATURE_UINT16, g_scenario_data.herd_points_predator[i].private_access(_X));
    for (int i = 0; i < MAX_PREDATOR_HERD_POINTS; i++)
        iob->bind(BIND_SIGNATURE_UINT16, g_scenario_data.herd_points_predator[i].private_access(_Y));

    for (int i = 0; i < MAX_FISH_POINTS; i++)
        iob->bind(BIND_SIGNATURE_UINT16, g_scenario_data.fishing_points[i].private_access(_X));
    for (int i = 0; i < MAX_FISH_POINTS; i++)
        iob->bind(BIND_SIGNATURE_UINT16, g_scenario_data.fishing_points[i].private_access(_Y));

    iob->bind(BIND_SIGNATURE_UINT16, &g_scenario_data.alt_predator_type);

    iob->bind____skip(42);

    for (int i = 0; i < MAX_INVASION_POINTS_LAND; i++)
        iob->bind(BIND_SIGNATURE_UINT16, g_scenario_data.invasion_points_land[i].private_access(_X));
    for (int i = 0; i < MAX_INVASION_POINTS_SEA; i++)
        iob->bind(BIND_SIGNATURE_UINT16, g_scenario_data.invasion_points_land[i].private_access(_X));
    for (int i = 0; i < MAX_INVASION_POINTS_LAND; i++)
        iob->bind(BIND_SIGNATURE_UINT16, g_scenario_data.invasion_points_land[i].private_access(_Y));
    for (int i = 0; i < MAX_INVASION_POINTS_SEA; i++)
        iob->bind(BIND_SIGNATURE_UINT16, g_scenario_data.invasion_points_land[i].private_access(_Y));

    iob->bind____skip(36); // 18 * 2

    iob->bind(BIND_SIGNATURE_INT32, &g_scenario_data.win_criteria.culture.goal);
    iob->bind(BIND_SIGNATURE_INT32, &g_scenario_data.win_criteria.prosperity.goal);
    iob->bind(BIND_SIGNATURE_INT32, &g_scenario_data.win_criteria.monuments.goal);
    iob->bind(BIND_SIGNATURE_INT32, &g_scenario_data.win_criteria.kingdom.goal);
    iob->bind(BIND_SIGNATURE_INT32, &g_scenario_data.win_criteria.housing_count.goal);
    iob->bind(BIND_SIGNATURE_INT32, &g_scenario_data.win_criteria.housing_level.goal);

    iob->bind(BIND_SIGNATURE_UINT8, &g_scenario_data.win_criteria.culture.enabled);
    iob->bind(BIND_SIGNATURE_UINT8, &g_scenario_data.win_criteria.prosperity.enabled);
    iob->bind(BIND_SIGNATURE_UINT8, &g_scenario_data.win_criteria.monuments.enabled);
    iob->bind(BIND_SIGNATURE_UINT8, &g_scenario_data.win_criteria.kingdom.enabled);
    iob->bind(BIND_SIGNATURE_UINT8, &g_scenario_data.win_criteria.housing_count.enabled);
    iob->bind(BIND_SIGNATURE_UINT8, &g_scenario_data.win_criteria.housing_level.enabled);

    iob->bind____skip(6); // ???
                          //    iob->bind(BIND_SIGNATURE_INT32, &g_scenario_data.earthquake.severity);
                          //    iob->bind(BIND_SIGNATURE_INT16, g_scenario_data.earthquake.private_access(_Y)ear); // ??

    iob->bind(BIND_SIGNATURE_INT32, &g_scenario_data.win_criteria.time_limit.enabled);
    iob->bind(BIND_SIGNATURE_INT32, &g_scenario_data.win_criteria.time_limit.years);
    iob->bind(BIND_SIGNATURE_INT32, &g_scenario_data.win_criteria.survival_time.enabled);
    iob->bind(BIND_SIGNATURE_INT32, &g_scenario_data.win_criteria.survival_time.years);
    iob->bind(BIND_SIGNATURE_INT32, &g_scenario_data.win_criteria.population.enabled);
    iob->bind(BIND_SIGNATURE_INT32, &g_scenario_data.win_criteria.population.goal);

    iob->bind(BIND_SIGNATURE_INT16, g_scenario_data.earthquake_point.private_access(_X));
    iob->bind(BIND_SIGNATURE_INT16, g_scenario_data.earthquake_point.private_access(_Y));

    *g_scenario_data.entry_point.private_access(_GRID_OFFSET) = -1;
    iob->bind(BIND_SIGNATURE_INT16, g_scenario_data.entry_point.private_access(_X));
    iob->bind(BIND_SIGNATURE_INT16, g_scenario_data.entry_point.private_access(_Y));

    *g_scenario_data.exit_point.private_access(_GRID_OFFSET) = -1;
    iob->bind(BIND_SIGNATURE_INT16, g_scenario_data.exit_point.private_access(_X));
    iob->bind(BIND_SIGNATURE_INT16, g_scenario_data.exit_point.private_access(_Y));

    // junk 4a
    iob->bind____skip(28); // 14 * 2
    iob->bind____skip(4);  // 2 * 2 (58, 64)

    *g_scenario_data.river_entry_point.private_access(_GRID_OFFSET) = -1;
    iob->bind(BIND_SIGNATURE_INT16, g_scenario_data.river_entry_point.private_access(_X));
    iob->bind(BIND_SIGNATURE_INT16, g_scenario_data.river_entry_point.private_access(_Y));

    *g_scenario_data.river_exit_point.private_access(_GRID_OFFSET) = -1;
    iob->bind(BIND_SIGNATURE_INT16, g_scenario_data.river_exit_point.private_access(_X));
    iob->bind(BIND_SIGNATURE_INT16, g_scenario_data.river_exit_point.private_access(_Y));

    iob->bind(BIND_SIGNATURE_INT32, &g_scenario_data.rescue_loan);
    iob->bind(BIND_SIGNATURE_INT32, &g_scenario_data.win_criteria.milestone25_year);
    iob->bind(BIND_SIGNATURE_INT32, &g_scenario_data.win_criteria.milestone50_year);
    iob->bind(BIND_SIGNATURE_INT32, &g_scenario_data.win_criteria.milestone75_year);

    // junk 4b
    iob->bind____skip(10); // 3 * 4 (usually go n, n+2, n+1497)
    iob->bind(BIND_SIGNATURE_UINT8, &g_scenario_data.env.has_animals);
    iob->bind(BIND_SIGNATURE_UINT8, &g_scenario_data.env.flotsam_enabled);
    iob->bind(BIND_SIGNATURE_UINT8, &g_scenario_data.climate);

    // junk 4e
    iob->bind____skip(1);
    iob->bind____skip(1); // used?
    iob->bind____skip(1); // used?
    iob->bind____skip(8);

    iob->bind(BIND_SIGNATURE_UINT8, &g_scenario_data.monuments.monuments_set);
    iob->bind(BIND_SIGNATURE_UINT8, &g_scenario_data.player_faction);

    // junk 4f
    iob->bind____skip(1); // -1 or -31
    iob->bind____skip(1); // -1

    for (int i = 0; i < MAX_PREY_HERD_POINTS; i++)
        iob->bind(BIND_SIGNATURE_INT32, g_scenario_data.herd_points_prey[i].private_access(_X));
    for (int i = 0; i < MAX_PREY_HERD_POINTS; i++)
        iob->bind(BIND_SIGNATURE_INT32, g_scenario_data.herd_points_prey[i].private_access(_Y));

    // 114
    int16_t reserved_data = 0;
    for (int i = 0; i < 114; i++) {
        iob->bind(BIND_SIGNATURE_INT16, &reserved_data);
    }

    for (int i = 0; i < MAX_DISEMBARK_POINTS; ++i)
        iob->bind(BIND_SIGNATURE_INT32, g_scenario_data.disembark_points[i].private_access(_X));

    for (int i = 0; i < MAX_DISEMBARK_POINTS; ++i)
        iob->bind(BIND_SIGNATURE_INT32, g_scenario_data.disembark_points[i].private_access(_Y));

    iob->bind(BIND_SIGNATURE_UINT32, &g_scenario_data.debt_interest_rate);

    iob->bind(BIND_SIGNATURE_UINT16, &g_scenario_data.monuments.first);
    iob->bind(BIND_SIGNATURE_UINT16, &g_scenario_data.monuments.second);
    iob->bind(BIND_SIGNATURE_UINT16, &g_scenario_data.monuments.third);

    // junk 6a
    iob->bind____skip(2);

    for (int i = 0; i < RESOURCES_MAX; ++i)
        iob->bind(BIND_SIGNATURE_UINT32, &g_scenario_data.monuments.burial_provisions[i].required);
    for (int i = 0; i < RESOURCES_MAX; ++i)
        iob->bind(BIND_SIGNATURE_UINT32, &g_scenario_data.monuments.burial_provisions[i].dispatched);

    iob->bind(BIND_SIGNATURE_UINT32, &g_scenario_data.current_pharaoh);
    iob->bind(BIND_SIGNATURE_UINT32, &g_scenario_data.player_incarnation);

    // junk 8b
    iob->bind____skip(4);

    ///

    g_scenario_data.is_saved = true;
});

io_buffer* iob_scenario_carry_settings = new io_buffer([](io_buffer* iob, size_t version) {
    iob->bind(BIND_SIGNATURE_INT32, &g_scenario_data.settings.starting_kingdom);
    iob->bind(BIND_SIGNATURE_INT32, &g_scenario_data.settings.starting_personal_savings);
    iob->bind(BIND_SIGNATURE_INT32, &g_scenario_data.settings.campaign_mission_rank);
});

io_buffer* iob_scenario_is_custom = new io_buffer(
  [](io_buffer* iob, size_t version) { iob->bind(BIND_SIGNATURE_INT32, &g_scenario_data.settings.is_custom); });

io_buffer* iob_scenario_map_name = new io_buffer([](io_buffer* iob, size_t version) {
    iob->bind(BIND_SIGNATURE_RAW, &g_scenario_data.scenario_name, MAX_SCENARIO_NAME);
});

