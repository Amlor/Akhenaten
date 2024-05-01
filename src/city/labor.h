#pragma once

#include "building/building.h"
#include "labor_category.h"

struct labor_category_data {
    int workers_needed;
    int workers_allocated;
    int buildings;
    int priority;
    int total_houses_covered;
} ;

struct city_labor_t {
    int32_t wages;
    int32_t wages_kingdome;
    int32_t workers_available;
    int32_t workers_employed;
    int32_t workers_unemployed;
    int32_t workers_needed;
    int32_t unemployment_percentage;
    int32_t unemployment_percentage_for_goverment;
    labor_category_data categories[10];

    int raise_wages_kingdome();
    int lower_wages_kingdome();
    void change_wages(int amount);
    void calculate_workers(int num_plebs, int num_patricians);
    void calculate_workers_needed_per_category();
    void set_building_worker_weight();
    void allocate_workers_to_categories();
    void allocate_workers_to_water();
    void allocate_workers_to_non_water_buildings();
    void allocate_workers_to_buildings();
    void check_employment();
    void allocate_workers();
    void update();
    void set_priority(int category, int new_priority);
    int max_selectable_priority(int category);
};

e_labor_category category_for_building(building* b);

const labor_category_data* city_labor_category(int category);
void city_labor_set_category(e_building_type type, int category);

template<class T>
void city_labor_set_category(const T &model) {
    city_labor_set_category(model.TYPE, model.labor_category);
}