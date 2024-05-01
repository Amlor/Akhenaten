#pragma once

#include <cstdint>

struct city_military_t {
    uint8_t total_batalions;
    uint8_t total_soldiers;
    uint8_t kingdome_service_batalions;
    int32_t infantry_batalions;
    int32_t native_attack_duration;

    void clear_infantry_batalions();
    void add_infantry_batalion();
    bool has_infantry_batalions();
    void clear_kingdome_service_batalions();
    void update_totals();
    bool is_native_attack_active();
    void start_native_attack();
    void decrease_native_attack_duration();
};


void city_military_determine_distant_battle_city();
int city_military_distant_battle_city();
int city_military_distant_battle_city_is_roman();

int city_military_distant_battle_enemy_strength();

void city_military_dispatch_to_distant_battle(int roman_strength);
int city_military_distant_battle_kingdome_army_is_traveling();
int city_military_distant_battle_kingdome_army_is_traveling_forth();
int city_military_distant_battle_kingdome_army_is_traveling_back();

int city_military_distant_battle_enemy_months_traveled();
int city_military_distant_battle_kingdome_months_traveled();

void city_military_init_distant_battle(int enemy_strength);
int city_military_has_distant_battle();
int city_military_months_until_distant_battle();

void city_military_process_distant_battle();
