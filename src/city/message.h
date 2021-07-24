#ifndef CITY_MESSAGE_H
#define CITY_MESSAGE_H

#include "core/buffer.h"

enum {
    MESSAGE_CAT_RIOT = 0,
    MESSAGE_CAT_FIRE = 1,
    MESSAGE_CAT_COLLAPSE = 2,
    MESSAGE_CAT_RIOT_COLLAPSE = 3,
    MESSAGE_CAT_BLOCKED_DOCK = 4,
    MESSAGE_CAT_WORKERS_NEEDED = 8,
    MESSAGE_CAT_TUTORIAL3 = 9,
    MESSAGE_CAT_NO_WORKING_DOCK = 10,
    MESSAGE_CAT_FISHING_BLOCKED = 11,
};

enum {
    MESSAGE_ADVISOR_NONE = 0,
    MESSAGE_ADVISOR_LABOR = 1,
    MESSAGE_ADVISOR_TRADE = 2,
    MESSAGE_ADVISOR_POPULATION = 3,
    MESSAGE_ADVISOR_IMPERIAL = 4,
    MESSAGE_ADVISOR_MILITARY = 5,
    MESSAGE_ADVISOR_HEALTH = 6,
    MESSAGE_ADVISOR_RELIGION = 7,
};

enum {
    MESSAGE_POPULATION_500 = 2,
    MESSAGE_POPULATION_1000 = 3,
    MESSAGE_POPULATION_2000 = 4,
    MESSAGE_POPULATION_3000 = 5,
    MESSAGE_POPULATION_5000 = 6,
    MESSAGE_POPULATION_10000 = 7,
    MESSAGE_POPULATION_15000 = 8,
    MESSAGE_POPULATION_20000 = 9,
    MESSAGE_POPULATION_25000 = 10,
    MESSAGE_RIOT = 11,
    MESSAGE_FIRE = 12,
    MESSAGE_COLLAPSED_BUILDING = 13,
    MESSAGE_DESTROYED_BUILDING = 14,
    MESSAGE_NAVIGATION_IMPOSSIBLE = 15,
    MESSAGE_CITY_IN_DEBT = 16,
    MESSAGE_CITY_IN_DEBT_AGAIN = 17,
    MESSAGE_CITY_STILL_IN_DEBT = 18,
    MESSAGE_CAESAR_WRATH = 19,
    MESSAGE_CAESAR_ARMY_CONTINUE = 20,
    MESSAGE_CAESAR_ARMY_RETREAT = 21,
    MESSAGE_LOCAL_UPRISING = 22,
    MESSAGE_BARBARIAN_ATTACK = 23,
    MESSAGE_CAESAR_ARMY_ATTACK = 24,
    MESSAGE_DISTANT_BATTLE = 25,
    MESSAGE_ENEMIES_CLOSING = 26,
    MESSAGE_ENEMIES_AT_THE_DOOR = 27,
    MESSAGE_CAESAR_REQUESTS_GOODS = 28,
    MESSAGE_CAESAR_REQUESTS_MONEY = 29,
    MESSAGE_CAESAR_REQUESTS_ARMY = 30,
    MESSAGE_REQUEST_REMINDER = 31,
    MESSAGE_REQUEST_RECEIVED = 32,
    MESSAGE_REQUEST_REFUSED = 33,
    MESSAGE_REQUEST_REFUSED_OVERDUE = 34,
    MESSAGE_REQUEST_RECEIVED_LATE = 35,
    MESSAGE_UNEMPLOYMENT = 36,
    MESSAGE_WORKERS_NEEDED = 37,
    MESSAGE_SMALL_FESTIVAL = 38,
    MESSAGE_LARGE_FESTIVAL = 39,
    MESSAGE_GRAND_FESTIVAL = 40,
    MESSAGE_WRATH_OF_CERES = 41,
    MESSAGE_WRATH_OF_NEPTUNE_NO_SEA_TRADE = 42,
    MESSAGE_WRATH_OF_MERCURY = 43,
    MESSAGE_WRATH_OF_MARS_NO_MILITARY = 44,
    MESSAGE_WRATH_OF_VENUS = 45,
    MESSAGE_PEOPLE_DISGRUNTLED = 46,
    MESSAGE_PEOPLE_UNHAPPY = 47,
    MESSAGE_PEOPLE_ANGRY = 48,
    MESSAGE_NOT_ENOUGH_FOOD = 49,
    MESSAGE_FOOD_NOT_DELIVERED = 50,
    MESSAGE_THEFT = 52,
    MESSAGE_TUTORIAL_FIRE = 53,
    MESSAGE_TUTORIAL_COLLAPSE = 54,
    MESSAGE_GODS_UNHAPPY = 55,
    MESSAGE_TUTORIAL_WATER = 56,
    MESSAGE_TUTORIAL_GROWING_YOUR_CITY = 57,
    MESSAGE_TUTORIAL_HUNGER_HALTS_IMMIGRANTS = 58,
    MESSAGE_TUTORIAL_RELIGION = 59,
    MESSAGE_TUTORIAL_TAXES_INDUSTRY = 60,
    MESSAGE_TUTORIAL_TRADE = 61,
    MESSAGE_EARTHQUAKE = 62,
    MESSAGE_GLADIATOR_REVOLT = 63,
    MESSAGE_EMPEROR_CHANGE = 64,
    MESSAGE_LAND_TRADE_DISRUPTED_SANDSTORMS = 65,
    MESSAGE_SEA_TRADE_DISRUPTED = 66,
    MESSAGE_LAND_TRADE_DISRUPTED_LANDSLIDES = 67,
    MESSAGE_ROME_RAISES_WAGES = 68,
    MESSAGE_ROME_LOWERS_WAGES = 69,
    MESSAGE_CONTAMINATED_WATER = 70,
    MESSAGE_IRON_MINE_COLLAPED = 71,
    MESSAGE_CLAY_PIT_FLOODED = 72,
    MESSAGE_GLADIATOR_REVOLT_FINISHED = 73,
    MESSAGE_INCREASED_TRADING = 74,
    MESSAGE_DECREASED_TRADING = 75,
    MESSAGE_TRADE_STOPPED = 76,
    MESSAGE_PRICE_INCREASED = 78,
    MESSAGE_PRICE_DECREASED = 79,
    MESSAGE_EMPIRE_HAS_EXPANDED = 77,
    MESSAGE_ROAD_TO_ROME_BLOCKED = 80,
    MESSAGE_WRATH_OF_NEPTUNE = 81,
    MESSAGE_WRATH_OF_MARS = 82,
    MESSAGE_DISTANT_BATTLE_LOST_NO_TROOPS = 84,
    MESSAGE_DISTANT_BATTLE_LOST_TOO_LATE = 85,
    MESSAGE_DISTANT_BATTLE_LOST_TOO_WEAK = 86,
    MESSAGE_DISTANT_BATTLE_WON = 87,
    MESSAGE_TROOPS_RETURN_FAILED = 88,
    MESSAGE_TROOPS_RETURN_VICTORIOUS = 89,
    MESSAGE_DISTANT_BATTLE_CITY_RETAKEN = 90,
    MESSAGE_CERES_IS_UPSET = 91,
    MESSAGE_NEPTUNE_IS_UPSET = 92,
    MESSAGE_MERCURY_IS_UPSET = 93,
    MESSAGE_MARS_IS_UPSET = 94,
    MESSAGE_VENUS_IS_UPSET = 95,
    MESSAGE_BLESSING_FROM_CERES = 96,
    MESSAGE_BLESSING_FROM_NEPTUNE = 97,
    MESSAGE_BLESSING_FROM_MERCURY = 98,
    MESSAGE_BLESSING_FROM_MARS = 99,
    MESSAGE_BLESSING_FROM_VENUS = 100,
    MESSAGE_GODS_WRATHFUL = 101,
    MESSAGE_HEALTH_ILLNESS = 102,
    MESSAGE_HEALTH_DISEASE = 103,
    MESSAGE_HEALTH_PESTILENCE = 104,
    MESSAGE_SPIRIT_OF_MARS = 105,
    MESSAGE_CAESAR_RESPECT_1 = 106,
    MESSAGE_CAESAR_RESPECT_2 = 107,
    MESSAGE_CAESAR_RESPECT_3 = 108,
    MESSAGE_WORKING_HIPPODROME = 109,
    MESSAGE_WORKING_COLOSSEUM = 110,
    MESSAGE_EMIGRATION = 111,
    MESSAGE_FIRED = 112,
    MESSAGE_ENEMY_ARMY_ATTACK = 114,
    MESSAGE_REQUEST_CAN_COMPLY = 115,
    MESSAGE_ROAD_TO_ROME_OBSTRUCTED = 116,
    MESSAGE_NO_WORKING_DOCK = 117,
    MESSAGE_FISHING_BOAT_BLOCKED = 118,
    MESSAGE_TUTORIAL_HEALTH = 119,
    MESSAGE_LOCAL_UPRISING_MARS = 121,

    //

    MESSAGE_NO_WORKING_DOCK_PH = 316 - 99,

    MESSAGE_TUTORIAL_FOOD_OR_FAMINE = 239 - 99,
    MESSAGE_TUTORIAL_ENTERTAINMENT = 240 - 99,
    MESSAGE_TUTORIAL_GOLD_AND_CRIME = 241 - 99,
    MESSAGE_TUTORIAL_FARMING_ALONG_THE_NILE = 242 - 99,
    MESSAGE_TUTORIAL_DEVELOPING_CULTURE = 243 - 99,
    MESSAGE_TUTORIAL_GETTING_STARTED = 244 - 99,
    MESSAGE_TUTORIAL_SOLDIERS_AND_FORT = 245 - 99,
    MESSAGE_TUTORIAL_AT_WATERS_EDGE = 246 - 99,
    MESSAGE_TUTORIAL_AT_WATERS_EDGE_2 = 247 - 99,
    MESSAGE_TUTORIAL_THE_FINER_THINGS = 248 - 99,
    MESSAGE_TUTORIAL_HOUSING_AND_ROADS = 249 - 99,
    MESSAGE_TUTORIAL_BASIC_HEALTHCARE = 250 - 99,
    MESSAGE_TUTORIAL_REQUESTS_FROM_OTHER_CITIES = 251 - 99,
    MESSAGE_TUTORIAL_FIRE_IN_THE_VILLAGE = 252 - 99,
    MESSAGE_TUTORIAL_COLLAPSED_BUILDING = 253 - 99,
    MESSAGE_TUTORIAL_EDUCATION = 254 - 99,
    MESSAGE_TUTORIAL_CLEAN_WATER = 255 - 99,
    MESSAGE_TUTORIAL_MUNICIPAL_STRUCTURES = 256 - 99,
    MESSAGE_TUTORIAL_MONUMENTS_AND_MORE = 257 - 99,
    MESSAGE_TUTORIAL_GODS_OF_EGYPT = 258 - 99,
    MESSAGE_TUTORIAL_INDUSTRY = 259 - 99,
    MESSAGE_TUTORIAL_TRADE_WITH_OTHER_CITIES = 260 - 99,

    MESSAGE_TUTORIAL_MONUMENTS = 283 - 99,
    MESSAGE_TUTORIAL_FINANCES = 310 - 99,

//    MESSAGE_BLESSING_OSIRIS = 295 - 99,
//    MESSAGE_BLESSING_RA = 296 - 99,
//    MESSAGE_BLESSING_PTAH = 297 - 99,
//    MESSAGE_BLESSING_SETH = 298 - 99,
//    MESSAGE_BLESSING_BAST = 299 - 99,

    MESSAGE_BLESSING_OSIRIS = 295 - 99,
    MESSAGE_SMALL_BLESSING_OSIRIS = 321 - 99,
    MESSAGE_CURSE_OSIRIS_NOEFFECT = 337 - 99,
    MESSAGE_CURSE_OSIRIS_1 = 140 - 99,
    MESSAGE_CURSE_OSIRIS_2 = 147 - 99,
    MESSAGE_SMALL_CURSE_OSIRIS = 290 - 99,

    MESSAGE_BLESSING_RA = 296 - 99,
    MESSAGE_SMALL_BLESSING_RA_1 = 322 - 99,
    MESSAGE_SMALL_BLESSING_RA_2 = 331 - 99,
    MESSAGE_CURSE_RA_1 = 280 - 99,
    MESSAGE_CURSE_RA_2 = 332 - 99,
    MESSAGE_CURSE_RA_3 = 333 - 99,
    MESSAGE_CURSE_RA_NOEFFECT = 336 - 99,
    MESSAGE_SMALL_CURSE_RA_1 = 291 - 99,
    MESSAGE_SMALL_CURSE_RA_2 = 334 - 99,

    MESSAGE_BLESSING_PTAH = 297 - 99,
    MESSAGE_SMALL_BLESSING_PTAH = 323 - 99,
    MESSAGE_CURSE_PTAH_1 = 142 - 99,
    MESSAGE_CURSE_PTAH_2 = 148 - 99,
    MESSAGE_CURSE_PTAH_NOEFFECT = 141 - 99,
    MESSAGE_SMALL_CURSE_PTAH = 292 - 99,

    MESSAGE_BLESSING_SETH = 298 - 99,
    MESSAGE_SMALL_BLESSING_SETH = 324 - 99,
    MESSAGE_CURSE_SETH_1 = 281 - 99,
    MESSAGE_CURSE_SETH_2 = 149 - 99,
    MESSAGE_CURSE_SETH_NOEFFECT = 143 - 99,
    MESSAGE_SMALL_CURSE_SETH = 293 - 99,

    MESSAGE_BLESSING_BAST = 299 - 99,
    MESSAGE_SMALL_BLESSING_BAST = 325 - 99,
    MESSAGE_CURSE_BAST_1 = 144 - 99,
    MESSAGE_CURSE_BAST_2 = 134 - 99,
    MESSAGE_CURSE_BAST_NOEFFECT = 335 - 99,
    MESSAGE_SMALL_CURSE_BAST = 294 - 99,

};

typedef struct {
    int sequence;
//    int message_type;
    int message_id;
    int year;
    int month;
    int param1;
    int param2;
    int is_read;
    //
    int unk_00;
    int unk_01;
    int unk_02;
    int unk_03;
    int unk_04;
    int unk_05;
    int unk_06;
    int unk_07;
} city_message;

void city_message_init_scenario(void);

void city_message_init_problem_areas(void);

void city_message_disable_sound_for_next_message(void);

void city_message_apply_sound_interval(int category);

void city_message_post(bool use_popup, int message_id, int param1, int param2);

void city_message_post_with_popup_delay(int category, int message_type, int param1, short param2);

void city_message_post_with_message_delay(int category, int use_popup, int message_type, int delay);

void city_message_process_queue(void);

void city_message_sort_and_compact(void);

int city_message_get_text_id(int message_id);

int city_message_get_advisor(int message_type);

void city_message_reset_category_count(int category);

void city_message_increase_category_count(int category);

int city_message_get_category_count(int category);

void city_message_decrease_delays(void);

int city_message_mark_population_shown(int population);

const city_message *city_message_get(int message_id);

int city_message_set_current(int message_id);

void city_message_mark_read(int message_id);

void city_message_delete(int message_id);

int city_message_count(void);

int city_message_problem_area_count(void);

int city_message_next_problem_area_grid_offset(void);

void city_message_clear_scroll(void);

int city_message_scroll_position(void);

void city_message_set_scroll_position(int scroll_position);

void city_message_save_state(buffer *messages, buffer *extra, buffer *counts, buffer *delays, buffer *population);

void city_message_load_state(buffer *messages, buffer *extra, buffer *counts, buffer *delays, buffer *population);

#endif // CITY_MESSAGE_H
