#include "advisor_education.h"

#include "building/count.h"
#include "city/coverage.h"
#include "city/city.h"
#include "city/population.h"
#include "graphics/image.h"
#include "graphics/graphics.h"
#include "graphics/elements/lang_text.h"
#include "graphics/elements/panel.h"
#include "graphics/text.h"
#include "game/game.h"

ui::advisor_education_window g_advisor_education_window;

static int get_education_advice() {
    const house_demands &demands = g_city.houses;
    if (demands.education == 1)
        return demands.requiring.school ? 1 : 0;
    else if (demands.education == 2)
        return demands.requiring.library ? 3 : 2;
    else if (demands.education == 3)
        return 4;

    int advice_id;
    int coverage_school = g_coverage.school;
    int coverage_academy = g_coverage.academy;
    int coverage_library = g_coverage.library;
    if (!demands.requiring.school)
        advice_id = 5; // no demands yet
    else if (!demands.requiring.library) {
        if (coverage_school >= 100 && coverage_academy >= 100)
            advice_id = 6; // education is perfect
        else if (coverage_school <= coverage_academy)
            advice_id = 7; // build more schools
        else {
            advice_id = 8; // build more academies
        }
    } else {
        // all education needed
        if (coverage_school >= 100 && coverage_academy >= 100 && coverage_library >= 100)
            advice_id = 6;
        else if (coverage_school <= coverage_academy && coverage_school <= coverage_library)
            advice_id = 7; // build more schools
        else if (coverage_academy <= coverage_school && coverage_academy <= coverage_library)
            advice_id = 8; // build more academies
        else if (coverage_library <= coverage_school && coverage_library <= coverage_academy)
            advice_id = 9; // build more libraries
        else {
            advice_id = 6; // unlikely event that all coverages are equal
        }
    }
    return advice_id;
}

int ui::advisor_education_window::draw_background() {
    ui["population"].text_var("%u %s", city_population(), ui::str(57, 1));
    ui["school_age"].text_var("%u %s", city_population_school_age(), ui::str(57, 2));
    ui["academy_age"].text_var("%u %s", city_population_academy_age(), ui::str(57, 3));

    ui["school_total"].text_var("%u %s", building_count_total(BUILDING_SCRIBAL_SCHOOL), ui::str(8, 18));
    ui["school_active"].text_var("%u", building_count_active(BUILDING_SCRIBAL_SCHOOL));
    ui["school_care"].text_var("%u %s", 75 * building_count_active(BUILDING_SCRIBAL_SCHOOL), ui::str(57, 7));
    textid covg_school{ 57, 21 };
    if (g_coverage.school == 0) { covg_school = { 57, 10 }; }
    else if (g_coverage.school < 100) { covg_school = {57, g_coverage.school / 10 + 11}; }
    ui["school_covg"].text_var("%u %s", 75 * building_count_active(BUILDING_SCRIBAL_SCHOOL), ui::str(57, 7));

    ui["academies_total"].text_var("%u %s", building_count_total(BUILDING_ACADEMY), ui::str(8, 20));
    ui["academies_active"].text_var("%u", building_count_active(BUILDING_ACADEMY));
    ui["academies_care"].text_var("%u %s", 75 * building_count_active(BUILDING_ACADEMY), ui::str(57, 8));
    textid covg_academies{ 57, 21 };
    if (g_coverage.academy == 0) { covg_academies = { 57, 10 }; } 
    else if (g_coverage.academy < 100) { covg_academies = { 57, g_coverage.academy / 10 + 11 }; }
    ui["academies_covg"].text_var("%u %s", 100 * building_count_active(BUILDING_ACADEMY), ui::str(57, 7));

    ui["libraries_total"].text_var("%u %s", building_count_total(BUILDING_LIBRARY), ui::str(8, 22));
    ui["libraries_active"].text_var("%u", building_count_active(BUILDING_LIBRARY));
    ui["libraries_care"].text_var("%u %s", 75 * building_count_active(BUILDING_LIBRARY), ui::str(57, 9));
    textid covg_libraries{ 57, 21 };
    if (g_coverage.library == 0) { covg_libraries = { 57, 10 }; }
    else if (g_coverage.library < 100) { covg_libraries = { 57, g_coverage.library / 10 + 11 }; }
    ui["libraries_covg"].text_var("%u %s", 100 * building_count_active(BUILDING_LIBRARY), ui::str(57, 7));

    ui["education_advice"] = ui::str(57, 22 + get_education_advice());

    return 0;
}

void ui::advisor_education_window::ui_draw_foreground() {
    ui.begin_widget(screen_dialog_offset());
    ui.draw();
    ui.end_widget();
}

advisor_window* ui::advisor_education_window::instance() {
    return &g_advisor_education_window;
}
