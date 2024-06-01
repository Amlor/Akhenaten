#include "city_overlay_juggler.h"

#include "grid/property.h"
#include "grid/building.h"
#include "graphics/elements/tooltip.h"
#include "figure/figure.h"

city_overlay_booth g_city_overlay_booth;

city_overlay* city_overlay_for_booth() {
    return &g_city_overlay_booth;
}

city_overlay_booth::city_overlay_booth() {
    type = OVERLAY_BOOTH;
    column_type = COLUMN_TYPE_POSITIVE;
}

int city_overlay_booth::get_column_height(const building *b) const {
    if (b->house_size) {
        if (b->data.house.juggler || b->data.house.bandstand_juggler) {
            return std::max<int>(b->data.house.juggler, b->data.house.bandstand_juggler) / 10;
        }
    }

    return NO_COLUMN;
}

int city_overlay_booth::get_tooltip_for_building(tooltip_context *c, const building *b) const {
    if (b->data.house.juggler <= 0)
        return 75;
    else if (b->data.house.juggler >= 80)
        return 76;
    else if (b->data.house.juggler >= 20)
        return 77;
    else {
        return 78;
    }
}