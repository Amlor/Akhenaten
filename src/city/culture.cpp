#include "culture.h"

#include "building/building.h"
#include "building/count.h"
#include "city/constants.h"
#include "city/data_private.h"
#include "city/entertainment.h"
#include "city/festival.h"
#include "city/population.h"
#include "core/calc.h"

static struct {
    int theater;
    int amphitheater;
    int colosseum;
    int hippodrome;
    int hospital;
    int school;
    int academy;
    int library;
    int religion[5];
    int oracle;
} coverage;

int city_culture_coverage_theater(void) {
    return coverage.theater;
}

int city_culture_coverage_amphitheater(void) {
    return coverage.amphitheater;
}

int city_culture_coverage_colosseum(void) {
    return coverage.colosseum;
}

int city_culture_coverage_hippodrome(void) {
    return coverage.hippodrome;
}

int city_culture_coverage_average_entertainment(void) {
    return (coverage.hippodrome + coverage.colosseum + coverage.amphitheater + coverage.theater) / 4;
}

int city_culture_coverage_religion(int god) {
    return coverage.religion[god];
}

int city_culture_coverage_school(void) {
    return coverage.school;
}

int city_culture_coverage_library(void) {
    return coverage.library;
}

int city_culture_coverage_academy(void) {
    return coverage.academy;
}

int city_culture_coverage_hospital(void) {
    return coverage.hospital;
}

int city_culture_average_education(void) {
    return city_data.culture.average_education;
}

int city_culture_average_entertainment(void) {
    return city_data.culture.average_entertainment;
}

int city_culture_average_health(void) {
    return city_data.culture.average_health;
}

static int top(int input) {
    return input > 100 ? 100 : input;
}

void city_culture_update_coverage(void) {
    int population = city_data.population.population;

    // entertainment
    coverage.theater = top(calc_percentage(400 * building_count_active(BUILDING_BOOTH), population));
    coverage.amphitheater = top(calc_percentage(700 * building_count_active(BUILDING_BANDSTAND), population));
    coverage.colosseum = top(calc_percentage(1200 * building_count_active(BUILDING_PAVILLION), population));
    if (building_count_active(BUILDING_SENET_HOUSE) <= 0)
        coverage.hippodrome = 0;
    else
        coverage.hippodrome = 100;

    // religion
    int oracles = building_count_total(BUILDING_ORACLE);
    coverage.religion[GOD_CERES] = top(calc_percentage(
            500 * oracles +
            750 * building_count_active(BUILDING_TEMPLE_OSIRIS) +
            1500 * building_count_active(BUILDING_TEMPLE_COMPLEX_OSIRIS),
            population));
    coverage.religion[GOD_NEPTUNE] = top(calc_percentage(
            500 * oracles +
            750 * building_count_active(BUILDING_TEMPLE_RA) +
            1500 * building_count_active(BUILDING_TEMPLE_COMPLEX_RA),
            population));
    coverage.religion[GOD_MERCURY] = top(calc_percentage(
            500 * oracles +
            750 * building_count_active(BUILDING_TEMPLE_PTAH) +
            1500 * building_count_active(BUILDING_TEMPLE_COMPLEX_PTAH),
            population));
    coverage.religion[GOD_MARS] = top(calc_percentage(
            500 * oracles +
            750 * building_count_active(BUILDING_TEMPLE_SETH) +
            1500 * building_count_active(BUILDING_TEMPLE_COMPLEX_SETH),
            population));
    coverage.religion[GOD_VENUS] = top(calc_percentage(
            500 * oracles +
            750 * building_count_active(BUILDING_TEMPLE_BAST) +
            1500 * building_count_active(BUILDING_TEMPLE_COMPLEX_BAST),
            population));
    coverage.oracle = top(calc_percentage(500 * oracles, population));

    city_data.culture.religion_coverage =
            coverage.religion[GOD_CERES] +
            coverage.religion[GOD_NEPTUNE] +
            coverage.religion[GOD_MERCURY] +
            coverage.religion[GOD_MARS] +
            coverage.religion[GOD_VENUS];
    city_data.culture.religion_coverage /= 5;

    // education
    city_population_calculate_educational_age();

    coverage.school = top(calc_percentage(
            75 * building_count_active(BUILDING_SCHOOL), city_population_school_age()));
    coverage.library = top(calc_percentage(
            800 * building_count_active(BUILDING_LIBRARY), population));
    coverage.academy = top(calc_percentage(
            100 * building_count_active(BUILDING_MENU_WATER_CROSSINGS), city_population_academy_age()));

    // health
    coverage.hospital = top(calc_percentage(
            1000 * building_count_active(BUILDING_MORTUARY), population));
}

void city_culture_calculate(void) {
    city_data.culture.average_entertainment = 0;
    city_data.culture.average_religion = 0;
    city_data.culture.average_education = 0;
    city_data.culture.average_health = 0;

    int num_houses = 0;
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (b->state == BUILDING_STATE_VALID && b->house_size) {
            num_houses++;
            city_data.culture.average_entertainment += b->data.house.entertainment;
            city_data.culture.average_religion += b->data.house.num_gods;
            city_data.culture.average_education += b->data.house.education;
            city_data.culture.average_health += b->data.house.health;
        }
    }
    if (num_houses) {
        city_data.culture.average_entertainment /= num_houses;
        city_data.culture.average_religion /= num_houses;
        city_data.culture.average_education /= num_houses;
        city_data.culture.average_health /= num_houses;
    }

    city_entertainment_calculate_shows();
    city_festival_calculate_costs();
}

void city_culture_save_state(buffer *buf) {
    // Yes, hospital is saved twice
    buf->write_i32(coverage.theater);
    buf->write_i32(coverage.amphitheater);
    buf->write_i32(coverage.colosseum);
    buf->write_i32(coverage.hospital);
    buf->write_i32(coverage.hippodrome);
    for (int i = GOD_CERES; i <= GOD_VENUS; i++) {
        buf->write_i32(coverage.religion[i]);
    }
    buf->write_i32(coverage.oracle);
    buf->write_i32(coverage.school);
    buf->write_i32(coverage.library);
    buf->write_i32(coverage.academy);
    buf->write_i32(coverage.hospital);
}

void city_culture_load_state(buffer *buf) {
    // Yes, hospital is saved twice
    coverage.theater = buf->read_i32();
    coverage.amphitheater = buf->read_i32();
    coverage.colosseum = buf->read_i32();
    coverage.hospital = buf->read_i32();
    coverage.hippodrome = buf->read_i32();
    for (int i = GOD_CERES; i <= GOD_VENUS; i++) {
        coverage.religion[i] = buf->read_i32();
    }
    coverage.oracle = buf->read_i32();
    coverage.school = buf->read_i32();
    coverage.library = buf->read_i32();
    coverage.academy = buf->read_i32();
    coverage.hospital = buf->read_i32();
}

