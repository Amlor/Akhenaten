#include "difficulty.h"

#include "core/calc.h"
#include "game/settings.h"

struct difficulty_data_t {
    int money;
    int enemies;
    int starting_kingdom;
    int sentiment;
};

static const difficulty_data_t g_difficulty_data[] = {
  {300, 40, 70, 80},  // very easy
  {200, 60, 60, 70},  // easy
  {150, 80, 50, 60},  // normal
  {100, 100, 50, 50}, // hard
  {75, 120, 40, 40}   // very hard
};

int difficulty_starting_kingdom(void) {
    return g_difficulty_data[g_settings.difficulty].starting_kingdom;
}

int difficulty_sentiment(void) {
    return g_difficulty_data[g_settings.difficulty].sentiment;
}

int difficulty_adjust_money(int money) {
    return calc_adjust_with_percentage(money, g_difficulty_data[g_settings.difficulty].money);
}

int difficulty_adjust_enemies(int enemies) {
    return calc_adjust_with_percentage(enemies, g_difficulty_data[g_settings.difficulty].enemies);
}

int difficulty_adjust_wolf_attack(int attack) {
    switch (g_settings.difficulty) {
    case DIFFICULTY_VERY_EASY:
        return 2;
    case DIFFICULTY_EASY:
        return 4;
    case DIFFICULTY_NORMAL:
        return 6;
    default:
        return attack;
    }
}
