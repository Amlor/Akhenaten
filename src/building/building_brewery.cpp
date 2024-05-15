#include "building/building_brewery.h"

#include "building/building_workshop.h"
#include "widget/city/ornaments.h"
#include "city/labor.h"
#include "city/resource.h"
#include "city/warnings.h"
#include "building/count.h"
#include "empire/empire_city.h"

#include "js/js_game.h"
#include "graphics/window.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "dev/debug.h"

#include <iostream>

declare_console_command(addbeer, game_cheat_add_resource<RESOURCE_BEER>);

buildings::model_t<building_brewery> brewery_m;

ANK_REGISTER_CONFIG_ITERATOR(config_load_building_brewery);
void config_load_building_brewery() {
    brewery_m.load();
}

void building_brewery::on_create(int orientation) {
    data.industry.first_material_id = RESOURCE_BARLEY;
    base.output_resource_first_id = RESOURCE_BEER;
}

void building_brewery::window_info_background(object_info& c) {
    e_resource input_resource = RESOURCE_BARLEY;
    e_resource output_resource = RESOURCE_BEER;

    building_workshop_draw_background(c, 96, "brewery", 122, output_resource, input_resource);
}

void building_brewery::window_info_foreground(object_info &c) {
    building_workshop_draw_foreground(c);
}

bool building_brewery::draw_ornaments_and_animations_height(painter &ctx, vec2i point, tile2i tile, color color_mask) {
    const auto &anim = brewery_m.anim["work"];
    building_draw_normal_anim(ctx, point, &base, tile, anim, color_mask);

    int amount = std::min<int>(2, ceil((float)base.stored_amount() / 100.0) - 1);
    if (amount >= 0) {
        const auto &anim = brewery_m.anim["barley"];
        ImageDraw::img_generic(ctx, anim.first_img() + amount, point + anim.pos, color_mask);
    }

    return true;
}

void building_brewery::update_count() const {
    building_increase_industry_count(RESOURCE_BEER, num_workers() > 0);
}

void building_brewery::on_place_checks() {
    if (building_count_industry_active(RESOURCE_BARLEY) > 0) {
        return;
    }

    if (city_resource_count(RESOURCE_BEER) > 0 || city_resource_count(RESOURCE_BARLEY) > 0) {
        return;
    }

    building_construction_warning_show(WARNING_VINES_NEEDED);
    if (empire_can_produce_resource(RESOURCE_BARLEY, true)) {
        building_construction_warning_show(WARNING_BUILD_VINES_FARM);
    } else if (!empire_can_import_resource(RESOURCE_BARLEY, true)) {
        building_construction_warning_show(WARNING_OPEN_TRADE_TO_IMPORT);
    } else if (city_resource_trade_status(RESOURCE_BARLEY) != TRADE_STATUS_IMPORT) {
        building_construction_warning_show(WARNING_TRADE_IMPORT_RESOURCE);
    }
}
