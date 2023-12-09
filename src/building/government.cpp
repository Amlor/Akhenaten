#include "government.h"

#include "building/building.h"
#include "building/count.h"
#include "city/finance.h"
#include "core/game_environment.h"
#include "core/profiler.h"
#include "config/config.h"

void building_government_distribute_treasury() {
    OZZY_PROFILER_SECTION("Game/Run/Tick/Distribute Treasury");

    constexpr int greate_palace_units = 8;
    constexpr int palace_units = 5;
    constexpr int tax_collectpr_up_units = 2;
    constexpr int tax_collector_units = 1;

    const uint32_t vil_palace_count = building_count_active(BUILDING_VILLAGE_PALACE);
    const uint32_t town_palace_count = building_count_active(BUILDING_TOWN_PALACE);
    const uint32_t city_palace_count = building_count_active(BUILDING_CITY_PALACE);
    const uint32_t tax_col_count = building_count_active(BUILDING_TAX_COLLECTOR);
    const uint32_t tax_col_up_count = building_count_active(BUILDING_TAX_COLLECTOR_UPGRADED);

    int units = palace_units * (vil_palace_count + town_palace_count + city_palace_count);
                    //+ greate_palace_units * palace_up_count;

    if (!config_get(CONFIG_GP_CH_NEW_TAX_COLLECTION_SYSTEM)) {
        units += tax_collector_units * tax_col_count 
                    + tax_collectpr_up_units * tax_col_up_count;
    }

    int amount_per_unit = 0;
    int remainder = 0;
    int treasury = city_finance_treasury();

    if (treasury > 0 && units > 0) {
        amount_per_unit = treasury / units;
        remainder = treasury - units * amount_per_unit;
    } 

    buildings_valid_do([&] (building &b) {
        if (b.house_size) {
            return;
        }

        b.tax_income_or_storage = 0;
        if (b.num_workers <= 0) {
            return;
        }

        switch (b.type) {
        // ordered based on importance: most important gets the remainder
        //case BUILDING_GREATE_PALACE_UPGRADED:
        //    b.tax_income_or_storage = greate_palace_units * amount_per_unit + remainder;
        //    remainder = 0;
        //    break;

        case BUILDING_VILLAGE_PALACE:
        case BUILDING_TOWN_PALACE:
        case BUILDING_CITY_PALACE:
            b.tax_income_or_storage = palace_units * amount_per_unit + remainder;
            remainder = 0;
            break;

        case BUILDING_TAX_COLLECTOR_UPGRADED:
            if (!config_get(CONFIG_GP_CH_NEW_TAX_COLLECTION_SYSTEM)) {
                b.tax_income_or_storage = tax_collectpr_up_units * amount_per_unit + remainder;
                remainder = 0;
            }
            break;

        case BUILDING_TAX_COLLECTOR:
            if (!config_get(CONFIG_GP_CH_NEW_TAX_COLLECTION_SYSTEM)) {
                b.tax_income_or_storage = amount_per_unit + remainder;
                remainder = 0;
            }
            break;
        }
    });
}
