#include "building_roadblock.h"

#include "js/js_game.h"
#include "city/labor.h"
#include "graphics/elements/ui.h"
#include "graphics/window.h"
#include "window/building/common.h"
#include "window/window_building_info.h"

buildings::model_t<building_roadblock> roadblock_m;

ANK_REGISTER_CONFIG_ITERATOR(config_load_building_roadblock);
void config_load_building_roadblock() {
    roadblock_m.load();
}

void building_roadblock::on_place_checks() {
    /*nothing*/
}

bool building_roadblock::force_draw_flat_tile(painter &ctx, tile2i tile, vec2i pixel, color mask) {
    /*nothing*/
    return true;
}

void building_roadblock::set_permission(e_permission p) {
    int permission_bit = 1 << p;
    base.subtype.roadblock_exceptions ^= permission_bit;
}

int building_roadblock::get_permission(e_permission p) {
    int permission_bit = 1 << p;
    return (base.subtype.roadblock_exceptions & permission_bit);
}
