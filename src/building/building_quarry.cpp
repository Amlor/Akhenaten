#include "building_quarry.h"

#include "building/building_raw_material.h"
#include "building/count.h"
#include "widget/city/ornaments.h"
#include "js/js_game.h"
#include "city/labor.h"

buildings::model_t<building_sandstone_quarry> sandstone_quarry_m;
buildings::model_t<building_stone_quarry> stone_quarry_m;
buildings::model_t<building_limestone_quarry> limestone_quarry_m;
buildings::model_t<building_granite_quarry> granite_quarry_m;

ANK_REGISTER_CONFIG_ITERATOR(config_load_building_quarry);
void config_load_building_quarry() {
    sandstone_quarry_m.load();
    stone_quarry_m.load();
    limestone_quarry_m.load();
    granite_quarry_m.load();
}

void building_sandstone_quarry::window_info_background(object_info &c) {
    building_raw_material_draw_info(c, RESOURCE_SANDSTONE);
}

void building_sandstone_quarry::on_create(int orientration) {
    base.output_resource_first_id = RESOURCE_SANDSTONE;
}

bool building_sandstone_quarry::draw_ornaments_and_animations_height(painter &ctx, vec2i point, tile2i tile, color color_mask) {
    building_draw_normal_anim(ctx, point + vec2i{54, 15}, &base, tile, sandstone_quarry_m.anim["work"], color_mask);
    return true;
}

void building_sandstone_quarry::update_count() const {
    building_increase_industry_count(RESOURCE_SANDSTONE, num_workers() > 0);
}

// ======================  building_stone_quarry =============================
void building_stone_quarry::on_create(int orientration) {
    base.output_resource_first_id = RESOURCE_STONE;
}

void building_stone_quarry::window_info_background(object_info &c) {
    building_raw_material_draw_info(c, RESOURCE_STONE);
}

bool building_stone_quarry::draw_ornaments_and_animations_height(painter &ctx, vec2i point, tile2i tile, color color_mask) {
    building_draw_normal_anim(ctx, point + vec2i{54, 15}, &base, tile, stone_quarry_m.anim["work"], color_mask);
    return true;
}

void building_stone_quarry::update_count() const {
    building_increase_industry_count(RESOURCE_STONE, num_workers() > 0);
}

void building_limestone_quarry::window_info_background(object_info &c) {
    building_raw_material_draw_info(c, RESOURCE_LIMESTONE);
}

void building_limestone_quarry::on_create(int orientration) {
    base.output_resource_first_id = RESOURCE_LIMESTONE;
}

bool building_limestone_quarry::draw_ornaments_and_animations_height(painter &ctx, vec2i point, tile2i tile, color color_mask) {
    building_draw_normal_anim(ctx, point + vec2i{54, 15}, &base, tile, limestone_quarry_m.anim["work"], color_mask);
    return true;
}

void building_limestone_quarry::update_count() const {
    building_increase_industry_count(RESOURCE_LIMESTONE, num_workers() > 0);
}

void building_granite_quarry::window_info_background(object_info &c) {
    building_raw_material_draw_info(c, RESOURCE_GRANITE);
}

void building_granite_quarry::on_create(int orientration) {
    base.output_resource_first_id = RESOURCE_GRANITE;
}

bool building_granite_quarry::draw_ornaments_and_animations_height(painter &ctx, vec2i point, tile2i tile, color color_mask) {
    const auto &anim = granite_quarry_m.anim["work"];
    building_draw_normal_anim(ctx, point + vec2i{54, 15}, &base, tile, anim, color_mask);
    return true;
}

void building_granite_quarry::update_count() const {
    building_increase_industry_count(RESOURCE_GRANITE, num_workers() > 0);
}
