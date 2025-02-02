#include "building_statue.h"

#include "building/building.h"
#include "building/rotation.h"
#include "city/object_info.h"
#include "city/labor.h"
#include "game/resource.h"
#include "graphics/elements/panel.h"
#include "graphics/elements/lang_text.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "io/gamefiles/lang.h"
#include "config/config.h"
#include "window/building/common.h"
#include "sound/sound_building.h"
#include "core/svector.h"
#include "grid/terrain.h"
#include "grid/building_tiles.h"
#include "io/io_buffer.h"
#include "window/window_building_info.h"
#include "construction/build_planner.h"

int building_statue::statue_params_t::get_image(e_building_type type, int orientation, int variant) const {
    int image_id = 0;

    int size = this->var.size();

    if (!size) {
        return 0;
    }
    //
    while (orientation < 0) { orientation += 4; }
    //
    while (orientation > 3) { orientation -= 4; }

    while (variant < 0) { variant += 4; }

    while (variant > (size - 1)) { variant -= size; }

    variant %= size;
    return image_group(this->var[variant]);
}

template<typename T>
void building_statue::static_params_t<T>::load(archive arch) {
    inherited::load(arch);
    arch.r_array("variants", var, [] (archive arch, auto &item) {
        item.pack = arch.r_int("pack");
        item.id = arch.r_int("id");
        item.offset = arch.r_int("offset");
    });
}

template<typename T>
void building_statue::static_params_t<T>::planer_setup_preview_graphics(build_planner &planer) const {
    int statue_img = get_image(T::TYPE, planer.relative_orientation, planer.building_variant);
    planer.set_tiles_building(statue_img, this->building_size);
}

template<typename T>
int building_statue::static_params_t<T>::planer_setup_building_variant(e_building_type type, tile2i tile, int variant) const {
    assert(building_is_statue(type));
    int size = this->var.size();

    return rand() % size;
}

template<typename T>
int building_statue::static_params_t<T>::planer_next_building_variant(e_building_type type, tile2i tile, int variant) const {
    if (variant < 0) {
        return 0;
    }

    int size = this->var.size();
    if (!size) {
        return variant;
    }

    variant = (variant + 1) % size;
    return variant;
}

building_small_statue::static_params small_statue_m;
building_medium_statue::static_params medium_statue_m;
building_large_statue::static_params large_statue_m;

void building_statue::on_create(int o) {
    int orientation = (4 + building_rotation_global_rotation() + city_view_orientation() / 2) % 4;
    data.monuments.variant = g_city_planner.building_variant;
    data.monuments.statue_offset = rand() % 4;
}

void building_statue::on_place_update_tiles(int orientation, int variant) {
    int orientation_rel = city_view_relative_orientation(orientation);
    int image_id = statue_params().get_image(type(), orientation_rel, variant);
    map_building_tiles_add(id(), tile(), size(), image_id, TERRAIN_BUILDING);
}

void building_statue::on_place_checks() {
    /*nothing*/
}

void building_statue::update_map_orientation(int map_orientation) {
    int variant = data.monuments.variant;
    int combined = 0;

    int orientation = combined % 4 - (map_orientation / 2);
    int image_id = statue_params().get_image(type(), orientation - 1, variant);
    map_building_tiles_add(id(), tile(), base.size, image_id, TERRAIN_BUILDING);
}

void building_statue::bind_dynamic(io_buffer *iob, size_t version) {
    iob->bind____skip(38);
    iob->bind(BIND_SIGNATURE_UINT8, &data.monuments.orientation);
    for (int i = 0; i < 5; i++) {
        iob->bind(BIND_SIGNATURE_UINT16, &data.monuments.workers[i]);
    }
    iob->bind(BIND_SIGNATURE_UINT8, &data.monuments.phase);
    iob->bind(BIND_SIGNATURE_UINT8, &data.monuments.statue_offset);
    iob->bind____skip(1);
    iob->bind(BIND_SIGNATURE_UINT8, &data.monuments.variant);

    for (int i = 0; i < RESOURCES_MAX; i++) {
        iob->bind(BIND_SIGNATURE_UINT8, &data.monuments.resources_pct[i]);
    }
}