#include "building/building_ferry.h"

#include "city/labor.h"
#include "grid/water.h"
#include "grid/image.h"
#include "grid/building.h"
#include "grid/building_tiles.h"
#include "grid/routing/routing.h"
#include "graphics/elements/ui.h"

#include "io/gamefiles/lang.h"

buildings::model_t<building_ferry> ferry_m;
info_window_ferry ferry_infow;

void building_ferry::on_create(int orientation) {
    data.dock.orientation = orientation;
}

void building_ferry::update_month() {
    building_impl::update_month();

    map_water_update_docking_points(base, get_orientation(), 2);
}

void building_ferry::on_place_update_tiles(int orientation, int variant) {
    int img_id = anim(animkeys().base).first_img() + orientation;
    map_water_add_building(id(), tile(), size(), img_id);
    map_building_tiles_add(id(), tile(), size(), img_id, TERRAIN_BUILDING|TERRAIN_ROAD|TERRAIN_FERRY_ROUTE);
}

void building_ferry::update_map_orientation(int orientation) {
    int image_offset = city_view_relative_orientation(data.dock.orientation);
    int image_id = anim(animkeys().base).first_img() + image_offset;
    map_water_add_building(id(), tile(), size(), image_id);
}

bool building_ferry::force_draw_height_tile(painter &ctx, tile2i t, vec2i pixel, color mask) {
    if (this->main()->tile() == t) {
        int image_id = map_image_at(t);
        ImageDraw::isometric_from_drawtile_top(ctx, image_id, pixel + vec2i{-30, 15}, mask);

        return true;
    }

    return false;
}

bool building_ferry::force_draw_top_tile(painter &ctx, tile2i t, vec2i pixel, color mask) {
    if (this->main()->tile() == t) {
        return true;
    }

    return false;
}


void building_ferry::highlight_waypoints() {
    building_impl::highlight_waypoints();

    docking_tiles fpoints = map_water_get_docking_points(base, get_orientation(), 1);
    map_highlight_set(fpoints.point_a, ehighligth_green);
    map_highlight_set(fpoints.point_b, ehighligth_green);
}

void building_ferry::bind_dynamic(io_buffer *iob, size_t verrsion) {
    iob->bind____skip(88);
    iob->bind(BIND_SIGNATURE_UINT8, &data.dock.orientation);
    iob->bind(BIND_SIGNATURE_INT32, &data.dock.dock_tiles[0]);
    iob->bind(BIND_SIGNATURE_INT32, &data.dock.dock_tiles[1]);
}

bool info_window_ferry::check(object_info &c) {
    return c.building_get()->dcast_ferry();
}

void info_window_ferry::init(object_info &c) {
    building_info_window::init(c);

    building *ferry = c.building_get();

    textid reason{ 0, 0 };
    if (!map_routing_ferry_has_routes(ferry)) {
        reason = { c.group_id, 1 };
    } else if (!ferry->has_road_access) {
        reason = { c.group_id, 2 };
    } else if (ferry->num_workers <= 0) {
        reason = { c.group_id, 3 };
    }

    if (reason.id) {
        ui["warning_text"] = reason;
    }
}
