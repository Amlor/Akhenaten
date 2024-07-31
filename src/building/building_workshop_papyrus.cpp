#include "building_workshop_papyrus.h"

#include "building_workshop.h"
#include "building/count.h"
#include "widget/city/ornaments.h"
#include "city/labor.h"
#include "city/city_resource.h"
#include "city/warning.h"
#include "city/city.h"
#include "city/warnings.h"
#include "graphics/window.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "empire/empire.h"

#include "js/js_game.h"
#include "dev/debug.h"

#include <iostream>

buildings::model_t<building_papyrus_maker> papyrus_maker_m;

ANK_REGISTER_CONFIG_ITERATOR(config_load_building_papyrus_maker);
void config_load_building_papyrus_maker() {
    papyrus_maker_m.load();
}

declare_console_command(addpapyrus, game_cheat_add_resource<RESOURCE_PAPYRUS>);

void building_papyrus_maker::on_create(int orientation) {
    data.industry.first_material_id = RESOURCE_REEDS;
    base.output_resource_first_id = RESOURCE_PAPYRUS;
}

void building_papyrus_maker::window_info_background(object_info &c) {
    building_workshop_draw_background(c);
}

void building_papyrus_maker::window_info_foreground(object_info &c) {
    building_workshop_draw_foreground(c);
}

void building_papyrus_maker::update_count() const {
    building_increase_industry_count(RESOURCE_PAPYRUS, num_workers() > 0);
}

bool building_papyrus_maker::draw_ornaments_and_animations_height(painter &ctx, vec2i point, tile2i tile, color color_mask) {
    building_draw_normal_anim(ctx, point, &base, tile, papyrus_maker_m.anim["work"], color_mask);

    int amount = std::min<int>(2, ceil((float)base.stored_amount() / 100.0) - 1);
    if (amount >= 0) {
        const auto &anim = papyrus_maker_m.anim["reed"];
        ImageDraw::img_generic(ctx, anim.first_img() + amount, point + anim.pos, color_mask);
    }
    return true;
}

void building_papyrus_maker::on_place_checks() {
    if (building_count_industry_active(RESOURCE_REEDS) > 0) {
        return;
    }

    if (city_resource_count(RESOURCE_REEDS) > 0) {
        return;
    }

    building_construction_warning_show(WARNING_NEED_REEDS);
    if (g_city.can_produce_resource(RESOURCE_REEDS)) {
        building_construction_warning_show(WARNING_BUILD_REEDS_GATHERER);
    } else if (!g_empire.can_import_resource(RESOURCE_REEDS, true)) {
        building_construction_warning_show(WARNING_INSTRUCT_OVERSEER_TO_IMPORT_REED);
    } else if (city_resource_trade_status(RESOURCE_REEDS) != TRADE_STATUS_IMPORT) {
        building_construction_warning_show(WARNING_OPEN_TRADE_TO_IMPORT_REED);
    }
}
