#include "chief.h"

#include "city/floods.h"
#include "scenario/request.h"

#include "city/figures.h"
#include "city/finance.h"
#include "city/health.h"
#include "city/houses.h"
#include "city/labor.h"
#include "city/migration.h"
#include "city/military.h"
#include "city/population.h"
#include "city/resource.h"
#include "city/sentiment.h"
#include "core/calc.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/elements/lang_text.h"
#include "graphics/elements/panel.h"
#include "graphics/view/view.h"
#include "graphics/text.h"
#include "scenario/invasion.h"
#include "scenario/property.h"
#include "translation/translation.h"
#include "window/advisors.h"
#include "game/game.h"
#include "io/gamefiles/lang.h"

#include "js/js_game.h"

#define ADVISOR_HEIGHT 24
#define X_OFFSET 185

ANK_REGISTER_CONFIG_ITERATOR(config_load_advisor_chief);

struct advisor_chief_window : public ui::widget {
};

advisor_chief_window g_advisor_chief_window;

void config_load_advisor_chief() {
    g_config_arch.r_section("advisor_chief_window", [] (archive arch) {
        g_advisor_chief_window.load(arch);
    });
}

static void draw_title(int y, int text_id) {
    painter ctx = game.painter();
    ImageDraw::img_generic(ctx, image_id_from_group(PACK_GENERAL, 158), 26, y + 1);
    lang_text_draw(61, text_id, 44, y, FONT_NORMAL_WHITE_ON_DARK);
}

static int draw_advisor_chief_background() {
    auto &ui = g_advisor_chief_window;

    // sentiment
    {
        int sentiment = city_sentiment();
        std::pair<int, int> sentiment_status;
        if (sentiment <= 0) { sentiment_status = {20, FONT_NORMAL_YELLOW}; } 
        else if (sentiment >= 100) { sentiment_status = {31, FONT_NORMAL_BLACK_ON_DARK}; }
        else { sentiment_status = {32 + sentiment / 10, FONT_NORMAL_BLACK_ON_DARK}; }
        ui["sentiment_info"].text((pcstr)lang_get_string(61, sentiment_status.first));
        ui["sentiment_info"].font(sentiment_status.second);
    }

    // migration
    {
        std::pair<int, int> migration_status;
        if (city_figures_total_invading_enemies() > 3) { migration_status = {43, FONT_NORMAL_BLACK_ON_DARK}; } 
        else if (city_migration_newcomers() >= 5) { migration_status = {44, FONT_NORMAL_BLACK_ON_DARK}; }
        else if (city_migration_no_room_for_immigrants()) { migration_status = {45, FONT_NORMAL_YELLOW}; }
        else if (city_migration_percentage() >= 80) { migration_status = {44, FONT_NORMAL_BLACK_ON_DARK}; } 
        else {
            migration_status = {43, FONT_NORMAL_BLACK_ON_DARK};
            switch (city_migration_problems_cause()) {
            case NO_IMMIGRATION_LOW_WAGES: migration_status.first = 46; break;
            case NO_IMMIGRATION_NO_JOBS: migration_status.first = 47; break;
            case NO_IMMIGRATION_NO_FOOD: migration_status.first = 48; break;
            case NO_IMMIGRATION_HIGH_TAXES: migration_status.first = 49; break;
            case NO_IMMIGRATION_MANY_TENTS: migration_status.first = 50; break;
            case NO_IMMIGRATION_LOW_MOOD: migration_status.first = 51; break;
            default: migration_status.first = 59; break;
            }
        }
        ui["migration_info"].text((pcstr)lang_get_string(61, migration_status.first));
        ui["migration_info"].font(migration_status.second);
    }

    // workers
    {
        int pct_unemployment = city_labor_unemployment_percentage();
        int needed_workers = city_labor_workers_needed();
        std::pair<int, int> workers_status;
        if (pct_unemployment > 0) {
            if (pct_unemployment > 10) { workers_status = {76, FONT_NORMAL_YELLOW}; }
            else if (pct_unemployment > 5) { workers_status = {77, FONT_NORMAL_YELLOW}; }
            else if (pct_unemployment > 2) { workers_status = {78, FONT_NORMAL_YELLOW}; }
            else { workers_status = {79, FONT_NORMAL_BLACK_ON_DARK}; }
            int unemployed_num = city_labor_workers_unemployed() - needed_workers;
            ui["workers_info"].text_var("%s %d(%d)", (pcstr)lang_get_string(61, workers_status.first), pct_unemployment, unemployed_num);
        } else if (needed_workers > 0) {
            if (needed_workers > 75) { workers_status = {80, FONT_NORMAL_YELLOW}; }
            else if (needed_workers > 50) { workers_status = {81, FONT_NORMAL_YELLOW }; }
            else if (needed_workers > 25) { workers_status = {82, FONT_NORMAL_YELLOW}; }
            else { workers_status = {83, FONT_NORMAL_BLACK_ON_DARK}; }
            ui["workers_info"].text_var("%s %d", (pcstr)lang_get_string(61, workers_status.first), needed_workers);
        } else {
            workers_status = {84, FONT_NORMAL_BLACK_ON_DARK};
            ui["workers_info"].text((pcstr)lang_get_string(61, workers_status.first));
        }
       
        ui["workers_info"].font(workers_status.second);
    }

    {
        std::pair<int, int> foodstocks_status;
        if (scenario_property_kingdom_supplies_grain()) {
            foodstocks_status = {26, FONT_NORMAL_BLACK_ON_DARK};
            ui["foodstocks_info"].text((pcstr)lang_get_string(61, foodstocks_status.first));
        } else if (city_resource_food_supply_months() > 0) {
            foodstocks_status = {98, FONT_NORMAL_BLACK_ON_DARK};
            ui["foodstocks_info"].text_var("%s %d", (pcstr)lang_get_string(61, foodstocks_status.first));
        } else {
            foodstocks_status = {95, FONT_NORMAL_YELLOW};
            ui["foodstocks_info"].text((pcstr)lang_get_string(61, foodstocks_status.first));
        }

        ui["foodstocks_info"].font(foodstocks_status.second);
    }

    {
        std::pair<int, int> foodcomsuption_status;
        if (scenario_property_kingdom_supplies_grain()) {
            foodcomsuption_status = {26, FONT_NORMAL_BLACK_ON_DARK};
        } else {
            int pct = city_resource_food_percentage_produced();
            if (pct > 150) { foodcomsuption_status = {13, FONT_NORMAL_BLACK_ON_DARK}; }
            else if (pct > 105) { foodcomsuption_status = {14, FONT_NORMAL_BLACK_ON_DARK}; }
            else if (pct > 95) { foodcomsuption_status = {15, FONT_NORMAL_BLACK_ON_DARK}; }
            else if (pct > 75) { foodcomsuption_status = {16, FONT_NORMAL_YELLOW}; }
            else if (pct > 30) { foodcomsuption_status = {17, FONT_NORMAL_YELLOW}; }
            else if (pct > 0) { foodcomsuption_status = {18, FONT_NORMAL_YELLOW}; }
            else { foodcomsuption_status = {18, FONT_NORMAL_YELLOW}; }
        }

        ui["foodconsumption_info"].text((pcstr)lang_get_string(61, foodcomsuption_status.first));
        ui["foodconsumption_info"].font(foodcomsuption_status.second);
    }

    {
        int health_rate = city_health();
        ui["health_info"].text((pcstr)lang_get_string(61, 103 + health_rate / 10));
        ui["health_info"].font(health_rate >= 40 ? FONT_NORMAL_BLACK_ON_DARK : FONT_NORMAL_YELLOW);
    }

    {
        // todo
        //    house_demands *demands = city_houses_demands();
        //    if (demands->religion == 1)
        //        lang_text_draw(61, 46, X_OFFSET, y_line, FONT_NORMAL_RED);
        //    else if (demands->religion == 2)
        //        lang_text_draw(61, 47, X_OFFSET, y_line, FONT_NORMAL_RED);
        //    else if (demands->religion == 3)
        //        lang_text_draw(61, 48, X_OFFSET, y_line, FONT_NORMAL_RED);
        //    else
        //        lang_text_draw(61, 49, X_OFFSET, y_line, FONT_NORMAL_GREEN);
        ui["religion_info"].text((pcstr)lang_get_string(61, 125));
        ui["religion_info"].font(FONT_NORMAL_BLACK_ON_DARK);
    }

    {
        int treasury = city_finance_treasury();
        int balance_last_year = city_finance_overview_last_year()->balance;
        if (treasury > balance_last_year) { // assets have rison by ...
            ui["finance_info"].text_var("%s %d", (pcstr)lang_get_string(61, 152), treasury - balance_last_year);
            ui["finance_info"].font(FONT_NORMAL_BLACK_ON_DARK);
        } else if (treasury < balance_last_year) { // assets have fallen by ...
            ui["finance_info"].text_var("%s %d", (pcstr)lang_get_string(61, 154), balance_last_year - treasury);
            ui["finance_info"].font(FONT_NORMAL_YELLOW);
        } else if (city_finance_percentage_taxed_people() < 75) { // not collecting many taxes!
            ui["finance_info"].text((pcstr)lang_get_string(61, 151));
            ui["finance_info"].font(FONT_NORMAL_BLACK_ON_DARK);
        } else { // doing about as well as last year
            ui["finance_info"].text((pcstr)lang_get_string(61, 153));
            ui["finance_info"].font(FONT_NORMAL_BLACK_ON_DARK);
        }
    }

    {
        std::pair<bstring256, int> crime_status;
        if (city_sentiment_criminals() > 10) {
            crime_status = {bstring256().printf("%s %d %s", (pcstr)lang_get_string(61, 159),
                                                         city_finance_overview_this_year()->expenses.stolen,
                                                         (pcstr)lang_get_string(61, 164)), FONT_NORMAL_YELLOW};
        } else if (city_sentiment_criminals() > 7) {
            crime_status = {bstring256().printf("%s %d %s", (pcstr)lang_get_string(61, 160),
                                                         city_finance_overview_this_year()->expenses.stolen,
                                                         (pcstr)lang_get_string(61, 164)), FONT_NORMAL_YELLOW};
        } else if (city_sentiment_criminals() > 5) {
            crime_status = {bstring256().printf("%s %d %s", (pcstr)lang_get_string(61, 161),
                                                         city_finance_overview_this_year()->expenses.stolen,
                                                         (pcstr)lang_get_string(61, 164)), FONT_NORMAL_YELLOW};
        } else if (city_sentiment_criminals() > 2) {
            crime_status = {bstring256().printf("%s %d %s", (pcstr)lang_get_string(61, 162),
                                                         city_finance_overview_this_year()->expenses.stolen,
                                                         (pcstr)lang_get_string(61, 164)), FONT_NORMAL_BLACK_ON_DARK};
        } else {
            crime_status = {bstring256().printf((pcstr)lang_get_string(61, 163)), FONT_NORMAL_BLACK_ON_DARK};
        }

        ui["crime_info"].text_var("%s", crime_status.first);
        ui["crime_info"].font(FONT_NORMAL_BLACK_ON_DARK);
    }

    return ADVISOR_HEIGHT;
}

static void draw_advisor_chief_foreground() {
    g_advisor_chief_window.draw();

    painter ctx = game.painter();
    int width;

    int y_line = 246;
    int text_b = 20;

    //    // housing capacity
    //    imagedrawnamespace::image_draw_namespace::image_draw(image_id_from_group(GROUP_BULLET), 32, y_line + 1);
    //    text_draw(translation_for(TR_HEADER_HOUSING), 52, y_line, FONT_NORMAL_WHITE, 0);
    //
    //    if (!city_population_open_housing_capacity())
    //        width = text_draw(translation_for(TR_ADVISOR_HOUSING_NO_ROOM), X_OFFSET, y_line, FONT_NORMAL_GREEN, 0);
    //    else {
    //        width = text_draw(translation_for(TR_ADVISOR_HOUSING_ROOM), X_OFFSET, y_line, FONT_NORMAL_GREEN, 0);
    //        text_draw_number(city_population_open_housing_capacity(), '@', " ", X_OFFSET + width, y_line,
    //        FONT_NORMAL_GREEN);
    //    }
    //    y_line += 20;

    //    // education
    //    house_demands *demands = city_houses_demands();
    //    draw_title(y_line, 8);
    //    if (demands->education == 1)
    //        lang_text_draw(61, 39, X_OFFSET, y_line, FONT_NORMAL_RED);
    //    else if (demands->education == 2)
    //        lang_text_draw(61, 40, X_OFFSET, y_line, FONT_NORMAL_RED);
    //    else if (demands->education == 3)
    //        lang_text_draw(61, 41, X_OFFSET, y_line, FONT_NORMAL_RED);
    //    else
    //        lang_text_draw(61, 42, X_OFFSET, y_line, FONT_NORMAL_GREEN);
    //    y_line += 20;

    // military
    text_b = 170;
    // todo
    draw_title(y_line, 10);
    if (city_figures_imperial_soldiers())
        lang_text_draw(61, text_b, X_OFFSET, y_line, FONT_NORMAL_YELLOW);
    else if (city_figures_enemies())
        lang_text_draw(61, text_b, X_OFFSET, y_line, FONT_NORMAL_YELLOW);
    else if (scenario_invasion_exists_upcoming())
        lang_text_draw(61, text_b, X_OFFSET, y_line, FONT_NORMAL_YELLOW);
    else if (city_military_distant_battle_kingdome_army_is_traveling())
        lang_text_draw(61, text_b, X_OFFSET, y_line, FONT_NORMAL_BLACK_ON_DARK);
    else if (city_military_months_until_distant_battle() > 0)
        lang_text_draw(61, text_b, X_OFFSET, y_line, FONT_NORMAL_YELLOW);
    else if (city_figures_soldiers() > 0) // FIXED was ">=0" (always true)
        lang_text_draw(61, text_b + 7, X_OFFSET, y_line, FONT_NORMAL_BLACK_ON_DARK);
    else
        lang_text_draw(61, text_b + 1, X_OFFSET, y_line, FONT_NORMAL_BLACK_ON_DARK);
    y_line += 20;

    //    // entertainment
    //    draw_title(y_line, 10);
    //    if (demands->entertainment == 1)
    //        lang_text_draw(61, 43, X_OFFSET, y_line, FONT_NORMAL_RED);
    //    else if (demands->entertainment == 2)
    //        lang_text_draw(61, 44, X_OFFSET, y_line, FONT_NORMAL_RED);
    //    else
    //        lang_text_draw(61, 45, X_OFFSET, y_line, FONT_NORMAL_GREEN);
    //    y_line += 20;

    // kingdom
    text_b = 184;
    draw_title(y_line, 11);
    int requests = scenario_requests_active_count();
    if (requests == 0)
        lang_text_draw(61, text_b + 3, X_OFFSET, y_line, FONT_NORMAL_BLACK_ON_DARK);
    else if (requests == 1)
        lang_text_draw(61, text_b + 2, X_OFFSET, y_line, FONT_NORMAL_WHITE_ON_DARK);
    else if (requests == 2)
        lang_text_draw(61, text_b + 1, X_OFFSET, y_line, FONT_NORMAL_YELLOW);
    else
        lang_text_draw(61, text_b + 0, X_OFFSET, y_line, FONT_NORMAL_YELLOW);
    y_line += 20;

    // nilometer
    text_b = 192;
    draw_title(y_line, 12);
    int flood_quality = floodplains_expected_quality();
    if (flood_quality == 100)
        lang_text_draw(61, text_b + 5, X_OFFSET, y_line, FONT_NORMAL_BLACK_ON_DARK);
    else if (flood_quality > 75)
        lang_text_draw(61, text_b + 4, X_OFFSET, y_line, FONT_NORMAL_BLACK_ON_DARK);
    else if (flood_quality > 50)
        lang_text_draw(61, text_b + 3, X_OFFSET, y_line, FONT_NORMAL_BLACK_ON_DARK);
    else if (flood_quality > 25)
        lang_text_draw(61, text_b + 2, X_OFFSET, y_line, FONT_NORMAL_BLACK_ON_DARK);
    else if (flood_quality > 0)
        lang_text_draw(61, text_b + 1, X_OFFSET, y_line, FONT_NORMAL_YELLOW);
    else
        lang_text_draw(61, text_b + flood_quality, X_OFFSET, y_line, FONT_NORMAL_YELLOW);
    y_line += 20;

    // nilometer #2
    text_b = 204;
    if (flood_quality > 0) {
        int flood_month = floodplains_expected_month();
        lang_text_draw(61, text_b + flood_month, X_OFFSET, y_line, FONT_NORMAL_BLACK_ON_DARK);
    }
    y_line += 20;
}

const advisor_window* window_advisor_chief() {
    static const advisor_window window = {
        draw_advisor_chief_background,
        draw_advisor_chief_foreground,
        nullptr,
        nullptr
    };
    return &window;
}
