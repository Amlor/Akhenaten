#ifndef GAME_TUTORIAL_H
#define GAME_TUTORIAL_H

#include "core/buffer.h"

typedef enum {
    AVAILABLE,
    NOT_AVAILABLE,
    NOT_AVAILABLE_YET,
} tutorial_availability;

typedef struct tutorial_flags {
    struct {
        int fire;
        int crime;
        int collapse;
        int senate_built;
    } tutorial1;
    struct {
        int granary_built;
        int population_250_reached;
        int population_450_reached;
        int pottery_made;
        int pottery_made_year;
    } tutorial2;
    struct {
        int disease;
    } tutorial3;
    struct {
        bool flags[41];
        //
        bool fire;
        bool population_150_reached;
        bool gamemeat_400_stored;
        bool collapse;
        bool disease;
        bool gold_mined_500;
        bool temples_built;
        bool figs_800_stored;
        bool pottery_made;
        bool beer_made;
        bool spacious_apartment;

        bool papyrus_made;
        bool bricks_bought;



        bool tut5_can_trade_finally;

        //
        bool tut1_start;
        bool tut2_start;
        bool tut3_start;
        bool tut4_start;
        bool tut5_start;
        bool tut6_start;
        bool tut7_start;
        bool tut8_start;

    } pharaoh;
} tutorial_flags_t;

const tutorial_flags_t * tutorial_flags_struct();

void tutorial_init(void);

tutorial_availability tutorial_advisor_availability(void);
tutorial_availability tutorial_empire_availability(void);

void tutorial_menu_update(int tut);

int tutorial_get_population_cap(int current_cap);
int tutorial_get_immediate_goal_text(void);

int tutorial_adjust_request_year(int *year);

int tutorial_extra_fire_risk(void);
int tutorial_extra_damage_risk(void);

int tutorial_handle_fire(void);
int tutorial_handle_collapse(void);

void tutorial_on_crime(void);
void tutorial_on_disease(void);
void tutorial_on_filled_granary(int quantity);
void tutorial_on_add_to_warehouse(void);
void tutorial_on_gold_extracted();
void tutorial_on_religion();
void tutorial_on_house_evolve(int level);

void tutorial_starting_message();

void tutorial_on_day_tick(void);
void tutorial_on_month_tick(void);

#endif // GAME_TUTORIAL_H
