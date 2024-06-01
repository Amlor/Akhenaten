#include "city_overlay_physician.h"

#include "grid/property.h"
#include "grid/building.h"
#include "graphics/elements/tooltip.h"
#include "figure/figure.h"

city_overlay_physician g_city_overlay_physician;

city_overlay* city_overlay_for_physician() {
    return &g_city_overlay_physician;
}

city_overlay_physician::city_overlay_physician() {
    type = OVERLAY_PHYSICIAN;
    column_type = COLUMN_TYPE_WATER_ACCESS;
}

int city_overlay_physician::get_column_height(const building *b) const {
    return b->house_size && b->subtype.house_level
             ? b->data.house.physician
                ? b->data.house.physician / 10
                : 0
             : NO_COLUMN;
}

int city_overlay_physician::get_tooltip_for_building(tooltip_context *c, const building *b) const {
    if (b->data.house.physician <= 0) {
        return 132;
    } else if (b->data.house.physician <= 33) {
        return 133;
    } else if (b->data.house.physician <= 66) {
        return 134;
    } else {
        return 135;
    }
}
