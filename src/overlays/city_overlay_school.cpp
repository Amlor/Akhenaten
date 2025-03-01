#include "city_overlay_school.h"

#include "building/building_type.h"
#include "figure/figure.h"
#include "game/state.h"
#include "grid/property.h"
#include "grid/building.h"

city_overlay_schools g_city_overlay_schools;

city_overlay* city_overlay_for_scribal_school() {
    return &g_city_overlay_schools;
}

int city_overlay_schools::get_column_height(const building *b) const {
    if (b->house_size == 0) {
        return COLUMN_TYPE_NONE;
    }

    return b->data.house.school / 10;
}

xstring city_overlay_schools::get_tooltip_for_building(tooltip_context *c, const building *b) const {
    if (b->data.house.school <= 0)
        return ui::str(66, 19);
    
    if (b->data.house.school >= 80)
        return ui::str(66, 20);
    
    if (b->data.house.school >= 20)
        return ui::str(66, 21);
    
    return ui::str(66, 22);
}
