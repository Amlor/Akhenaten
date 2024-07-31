#include "building_weaver.h"

#include "building/count.h"
#include "building/building_workshop.h"
#include "graphics/graphics.h"
#include "city/city_resource.h"
#include "city/warnings.h"
#include "city/labor.h"
#include "city/city.h"
#include "empire/empire.h"
#include "js/js_game.h"

buildings::model_t<building_weaver> bweaver_m;

ANK_REGISTER_CONFIG_ITERATOR(config_load_building_weaver);
void config_load_building_weaver() {
    bweaver_m.load();
}

void building_weaver::on_create(int orientation) {
    data.industry.first_material_id = RESOURCE_FLAX;
    base.output_resource_first_id = RESOURCE_LINEN;
}

void building_weaver::on_place_checks() {
    if (building_count_industry_active(RESOURCE_FLAX) <= 0) {
        return;
    }

    if (city_resource_count(RESOURCE_FLAX) > 0) {
        return;
    }

    building_construction_warning_show(WARNING_OLIVES_NEEDED);
    if (g_city.can_produce_resource(RESOURCE_STRAW)) {
        building_construction_warning_show(WARNING_BUILD_OLIVE_FARM);
        return;
    } 
    
    if (!g_empire.can_import_resource(RESOURCE_STRAW, true)) {
        building_construction_warning_show(WARNING_OPEN_TRADE_TO_IMPORT);
        return;
    } 
    
    if (city_resource_trade_status(RESOURCE_STRAW) != TRADE_STATUS_IMPORT) {
        building_construction_warning_show(WARNING_TRADE_IMPORT_RESOURCE);
        return;
    }
}

void building_weaver::update_graphic() {
    const xstring &animkey = can_play_animation() ? animkeys().work : animkeys().none;
    set_animation(animkey);
}

void building_weaver::window_info_background(object_info &ctx) {
    e_resource input_resource = RESOURCE_FLAX;
    e_resource output_resource = RESOURCE_LINEN;
    building_workshop_draw_background(ctx, "flax_workshop", output_resource, input_resource);
}

void building_weaver::window_info_foreground(object_info &ctx) {
    building_workshop_draw_foreground(ctx);
}

bool building_weaver::draw_ornaments_and_animations_height(painter &ctx, vec2i point, tile2i tile, color color_mask) {
    draw_normal_anim(ctx, point, tile, color_mask);

    int amount = std::min<int>(2, ceil((float)base.stored_amount() / 100.0) - 1);
    if (amount >= 0) {
        const auto &anim = bweaver_m.anim["flax"];
        ImageDraw::img_generic(ctx, anim.first_img() + amount, point + anim.pos, color_mask);
    }

    return true;
}

void building_weaver::update_count() const {
    building_increase_industry_count(RESOURCE_LINEN, num_workers() > 0);
}
