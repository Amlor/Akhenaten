#include "city_overlay_tax_income.h"

#include "building/building.h"
#include "graphics/elements/tooltip.h"
#include "core/calc.h"
#include "city/finance.h"
#include "grid/property.h"
#include "grid/building.h"
#include "figure/figure.h"

city_overlay_tax_income g_city_overlay_tax_income;

city_overlay* city_overlay_for_tax_income() {
    return &g_city_overlay_tax_income;
}

city_overlay_tax_income::city_overlay_tax_income() {
    type = OVERLAY_TAX_INCOME;
    column_type = COLUMN_TYPE_WATER_ACCESS;
}

int city_overlay_tax_income::get_column_height(const building *b) const {
    if (b->house_size) {
        int pct = calc_adjust_with_percentage(b->tax_income_or_storage / 2, city_finance_tax_percentage());
        if (pct > 0) {
            return pct / 25;
        }
    }

    return NO_COLUMN;
}

int city_overlay_tax_income::get_tooltip_for_building(tooltip_context *c, const building *b) const {
    int denarii = calc_adjust_with_percentage(b->tax_income_or_storage / 2, city_finance_tax_percentage());
    if (denarii > 0) {
        c->has_numeric_prefix = 1;
        c->numeric_prefix = denarii;
        return 45;
    } else if (b->house_tax_coverage > 0) {
        return 44;
    } else {
        return 43;
    }
}