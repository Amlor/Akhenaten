#include <widget/sidebar/city.h>
#include <figure/formation_herd.h>
#include <SDL_log.h>
#include "build_planner.h"

#include "clear.h"
#include "routed.h"
#include "warnings.h"
#include "building/count.h"
#include "building/model.h"
#include "building/properties.h"
#include "building/storage.h"
#include "building/rotation.h"
#include "building/warehouse.h"
#include "building/dock.h"
#include "building/menu.h"
#include "city/buildings.h"
#include "city/finance.h"
#include "city/resource.h"
#include "city/view.h"
#include "city/warning.h"
#include "core/calc.h"
#include "core/config.h"
#include "core/image.h"
#include "core/random.h"
#include "figure/formation.h"
#include "figure/formation_legion.h"
#include "game/undo.h"
#include "graphics/window.h"
#include "map/aqueduct.h"
#include "map/bridge.h"
#include "map/building.h"
#include "map/building_tiles.h"
#include "map/grid.h"
#include "map/image.h"
#include "map/orientation.h"
#include "map/point.h"
#include "map/property.h"
#include "map/routing.h"
#include "map/routing_terrain.h"
#include "map/terrain.h"
#include "map/tiles.h"
#include "map/water.h"
#include "building/monuments.h"

struct reservoir_info {
    int cost;
    int place_reservoir_at_start;
    int place_reservoir_at_end;
};

enum {
    PLACE_RESERVOIR_BLOCKED = -1,
    PLACE_RESERVOIR_NO = 0,
    PLACE_RESERVOIR_YES = 1,
    PLACE_RESERVOIR_EXISTS = 2
};

BuildPlanner Planner;

static int last_items_cleared;

static const int FORT_X_OFFSET[4][4] = {{3,  4,  4,  3},
                                        {-1, 0,  0,  -1},
                                        {-4, -3, -3, 4},
                                        {0,  1,  1,  0}};
static const int FORT_Y_OFFSET[4][4] = {{-1, -1, 0,  0},
                                        {-4, -4, -3, -3},
                                        {0,  0,  1,  1},
                                        {3,  3,  4,  4}};

const int CROPS_OFFSETS[2] = {5, 6};

static void add_fort(int type, building *fort) {
    fort->prev_part_building_id = 0;
    map_building_tiles_add(fort->id, fort->x, fort->y, fort->size, image_id_from_group(GROUP_BUILDING_FORT),
                           TERRAIN_BUILDING);
    if (type == BUILDING_FORT_CHARIOTEERS)
        fort->subtype.fort_figure_type = FIGURE_FORT_LEGIONARY;
    else if (type == BUILDING_FORT_ARCHERS)
        fort->subtype.fort_figure_type = FIGURE_FORT_JAVELIN;
    else if (type == BUILDING_FORT_INFANTRY)
        fort->subtype.fort_figure_type = FIGURE_FORT_MOUNTED;


    // create parade ground
    const int offsets_x[] = {3, -1, -4, 0};
    const int offsets_y[] = {-1, -4, 0, 3};
    building *ground = building_create(BUILDING_FORT_GROUND, fort->x + offsets_x[building_rotation_get_rotation()],
                                       fort->y + offsets_y[building_rotation_get_rotation()], 0);
    game_undo_add_building(ground);
    ground->prev_part_building_id = fort->id;
    fort->next_part_building_id = ground->id;
    ground->next_part_building_id = 0;
    map_building_tiles_add(ground->id, fort->x + offsets_x[building_rotation_get_rotation()],
                           fort->y + offsets_y[building_rotation_get_rotation()], 4,
                           image_id_from_group(GROUP_BUILDING_FORT) + 1, TERRAIN_BUILDING);

    fort->formation_id = formation_legion_create_for_fort(fort);
    ground->formation_id = fort->formation_id;
}

void BuildPlanner::add_building_tiles_from_list(int building_id, bool graphics_only) {
    for (int row = 0; row < size.y; ++row) {
        for (int column = 0; column < size.x; ++column) {
            int image_id = tile_graphics_array[row][column];
            int size = tile_sizes_array[row][column];
            map_tile tile = tile_coord_cache[row][column];

            // correct for city orientation
            switch (city_view_orientation() / 2) {
                case 0:
                    tile.y = tile.y - size + 1;
                    break;
                case 1:
                    break;
                case 2:
                    tile.x = tile.x - size + 1;
                    break;
                case 3:
                    tile.x = tile.x - size + 1;
                    tile.y = tile.y - size + 1;
                    break;
            }
            if (image_id > 0 && size > 0) {
                if (!graphics_only)
                    map_building_tiles_add(building_id, tile.x, tile.y, size, image_id, TERRAIN_BUILDING);
                else
                    map_image_set(tile.grid_offset, image_id);
            }
        }
    }
}

static building *add_temple_complex_element(int x, int y, int orientation, building *prev) {
    building *b = building_create(prev->type, x, y, orientation);
    game_undo_add_building(b);

    b->size = 3;
    b->grid_offset = map_grid_offset(b->x, b->y);
    b->prev_part_building_id = prev->id;
    prev->next_part_building_id = b->id;
    int image_id = map_image_at(map_grid_offset(x, y));
    map_building_tiles_add(b->id, b->x, b->y, 3, image_id, TERRAIN_BUILDING);

    return b;
}
static void add_temple_complex(building *b, int orientation) {
    Planner.add_building_tiles_from_list(b->id, false);
    map_point offset = {0, 0};
    switch (orientation) {
        case 0:
            offset = {0, -3};
            break;
        case 1:
            offset = {3, 0};
            break;
        case 2:
            offset = {0, 3};
            break;
        case 3:
            offset = {-3, 0};
            break;
    }
    building *altar = add_temple_complex_element(b->x + offset.x, b->y + offset.y, orientation, b);
    building *oracle = add_temple_complex_element(b->x + 2 * offset.x, b->y + 2 * offset.y, orientation, altar);
}

static void latch_on_venue(int type, building *main, int dx, int dy, int orientation, bool main_venue = false) {
    int x = main->x + dx;
    int y = main->y + dy;
    int grid_offset = map_grid_offset(x, y);
    building *this_venue = main;
    if (main_venue) { // this is the main venue building!!
        if (type != main->type)
            return; // hmmm, this shouldn't happen
        main->grid_offset = grid_offset;
//        main->x += dx;
//        main->y += dy;
    } else if (type == BUILDING_GARDENS) { // add gardens
        map_terrain_add(grid_offset, TERRAIN_GARDEN);
    } else { // extra venue
        this_venue = building_create(type, x, y, 0);
        building *parent = main; // link venue to last one in the chain
        while (parent->next_part_building_id)
            parent = building_get(parent->next_part_building_id);
        parent->next_part_building_id = this_venue->id;
        this_venue->prev_part_building_id = parent->id;

        map_building_set(grid_offset, this_venue->id);
        if (type == BUILDING_PAVILLION) {
            map_building_set(grid_offset + map_grid_delta(1, 0), this_venue->id);
            map_building_set(grid_offset + map_grid_delta(1, 1), this_venue->id);
            map_building_set(grid_offset + map_grid_delta(0, 1), this_venue->id);
        }
    }

    // set map graphics accordingly
    switch (type) {
        case BUILDING_GARDENS:
            map_tiles_update_all_gardens();
            break;
        case BUILDING_BOOTH:
            map_image_set(grid_offset, image_id_from_group(GROUP_BUILDING_BOOTH));
            break;
        case BUILDING_BANDSTAND:
            map_image_set(grid_offset, image_id_from_group(GROUP_BUILDING_BANDSTAND) + orientation);
            if (orientation == 1)
                latch_on_venue(BUILDING_BANDSTAND, main, dx, dy + 1, 0);
            else if (orientation == 2)
                latch_on_venue(BUILDING_BANDSTAND, main, dx + 1, dy, 3);
            map_add_bandstand_tiles(this_venue);
            break;
        case BUILDING_PAVILLION:
            map_building_tiles_add(this_venue->id, x, y, 2, image_id_from_group(GROUP_BUILDING_PAVILLION), TERRAIN_BUILDING);
            break;
    }
}
static void add_entertainment_venue(building *b, int orientation) {
    b->data.entertainment.booth_corner_grid_offset = b->grid_offset;
    int size = 0;
    switch (b->type) {
        case BUILDING_BOOTH:
            size = 2; break;
        case BUILDING_BANDSTAND:
            size = 3; break;
        case BUILDING_PAVILLION:
            size = 4; break;
        case BUILDING_FESTIVAL_SQUARE:
            size = 5; break;
    }
    if (!map_grid_is_inside(b->x, b->y, size))
        return;
    int image_id = 0;
    switch (b->type) {
        case BUILDING_BOOTH:
            image_id = image_id_from_group(GROUP_BOOTH_SQUARE);
            break;
        case BUILDING_BANDSTAND:
            image_id = image_id_from_group(GROUP_BANDSTAND_SQUARE);
            break;
        case BUILDING_PAVILLION:
            image_id = image_id_from_group(GROUP_PAVILLION_SQUARE);
            break;
        case BUILDING_FESTIVAL_SQUARE:
            image_id = image_id_from_group(GROUP_FESTIVAL_SQUARE);
            break;
    }

    // add underlying plaza first
    map_add_venue_plaza_tiles(b->id, size, b->x, b->y, image_id, false);

    // add additional building parts, update graphics accordingly
    switch (b->type) {
        case BUILDING_BOOTH:
            switch (orientation / 2) {
                case 0:
                    latch_on_venue(BUILDING_BOOTH, b, 0, 0, 0, true);
                    break;
                case 1:
                    latch_on_venue(BUILDING_BOOTH, b, 1, 0, 0, true);
                    break;
                case 2:
                    latch_on_venue(BUILDING_BOOTH, b, 1, 1, 0, true);
                    break;
                case 3:
                    latch_on_venue(BUILDING_BOOTH, b, 0, 1, 0, true);
                    break;
            }
            break;
        case BUILDING_BANDSTAND:
            switch (orientation / 2) {
                case 0:
                    latch_on_venue(BUILDING_GARDENS, b, 2, 1, 0);
                    latch_on_venue(BUILDING_BANDSTAND, b, 0, 0, 1, true);
                    latch_on_venue(BUILDING_BOOTH, b, 2, 0, 0);
                    break;
                case 1:
                    latch_on_venue(BUILDING_GARDENS, b, 1, 2, 0);
                    latch_on_venue(BUILDING_BANDSTAND, b, 1, 0, 2, true);
                    latch_on_venue(BUILDING_BOOTH, b, 2, 2, 0);
                    break;
                case 2:
                    latch_on_venue(BUILDING_GARDENS, b, 2, 1, 0);
                    latch_on_venue(BUILDING_BANDSTAND, b, 0, 1, 1, true);
                    latch_on_venue(BUILDING_BOOTH, b, 2, 2, 0);
                    break;
                case 3:
                    latch_on_venue(BUILDING_GARDENS, b, 1, 2, 0);
                    latch_on_venue(BUILDING_BANDSTAND, b, 0, 0, 2, true);
                    latch_on_venue(BUILDING_BOOTH, b, 0, 2, 0);
                    break;
            }
            break;
        case BUILDING_PAVILLION:
            // todo
            switch (orientation) {
                case 0:
                    latch_on_venue(BUILDING_GARDENS, b, 1, 2, 0);
                    latch_on_venue(BUILDING_GARDENS, b, 3, 2, 0);
                    latch_on_venue(BUILDING_PAVILLION, b, 0, 0, 0, true);
                    latch_on_venue(BUILDING_BANDSTAND, b, 3, 0, 1);
                    latch_on_venue(BUILDING_BOOTH, b, 0, 2, 0);
                    break;
                case 1:
                    latch_on_venue(BUILDING_GARDENS, b, 2, 2, 0);
                    latch_on_venue(BUILDING_GARDENS, b, 3, 2, 0);
                    latch_on_venue(BUILDING_PAVILLION, b, 2, 0, 0, true);
                    latch_on_venue(BUILDING_BANDSTAND, b, 0, 0, 1);
                    latch_on_venue(BUILDING_BOOTH, b, 0, 2, 0);
                    break;
                case 2:
                    latch_on_venue(BUILDING_GARDENS, b, 1, 3, 0);
                    latch_on_venue(BUILDING_GARDENS, b, 2, 3, 0);
                    latch_on_venue(BUILDING_PAVILLION, b, 1, 0, 0, true);
                    latch_on_venue(BUILDING_BANDSTAND, b, 3, 0, 1);
                    latch_on_venue(BUILDING_BOOTH, b, 3, 3, 0);
                    break;
                case 3:
                    latch_on_venue(BUILDING_GARDENS, b, 1, 0, 0);
                    latch_on_venue(BUILDING_GARDENS, b, 2, 0, 0);
                    latch_on_venue(BUILDING_PAVILLION, b, 1, 2, 0, true);
                    latch_on_venue(BUILDING_BANDSTAND, b, 3, 2, 1);
                    latch_on_venue(BUILDING_BOOTH, b, 3, 0, 0);
                    break;
                case 4:
                    latch_on_venue(BUILDING_GARDENS, b, 2, 3, 0);
                    latch_on_venue(BUILDING_GARDENS, b, 3, 3, 0);
                    latch_on_venue(BUILDING_PAVILLION, b, 2, 1, 0, true);
                    latch_on_venue(BUILDING_BANDSTAND, b, 0, 1, 1);
                    latch_on_venue(BUILDING_BOOTH, b, 0, 3, 0);
                    break;
                case 5:
                    latch_on_venue(BUILDING_GARDENS, b, 1, 3, 0);
                    latch_on_venue(BUILDING_GARDENS, b, 3, 3, 0);
                    latch_on_venue(BUILDING_PAVILLION, b, 0, 1, 0, true);
                    latch_on_venue(BUILDING_BANDSTAND, b, 3, 1, 1);
                    latch_on_venue(BUILDING_BOOTH, b, 0, 3, 0);
                    break;
                case 6:
                    // in the original game, this orientation is not allowed for some reason?
                    latch_on_venue(BUILDING_GARDENS, b, 1, 0, 0);
                    latch_on_venue(BUILDING_GARDENS, b, 2, 0, 0);
                    latch_on_venue(BUILDING_PAVILLION, b, 0, 2, 0, true);
                    latch_on_venue(BUILDING_BANDSTAND, b, 2, 2, 1);
                    latch_on_venue(BUILDING_BOOTH, b, 0, 0, 0);
                    break;
                case 7:
                    latch_on_venue(BUILDING_GARDENS, b, 1, 3, 0);
                    latch_on_venue(BUILDING_GARDENS, b, 2, 3, 0);
                    latch_on_venue(BUILDING_PAVILLION, b, 0, 0, 0, true);
                    latch_on_venue(BUILDING_BANDSTAND, b, 2, 0, 1);
                    latch_on_venue(BUILDING_BOOTH, b, 0, 3, 0);
                    break;
            }
            break;
    }
}

static building *add_warehouse_space(int x, int y, building *prev) {
    building *b = building_create(BUILDING_WAREHOUSE_SPACE, x, y, 0);
    game_undo_add_building(b);
    b->prev_part_building_id = prev->id;
    prev->next_part_building_id = b->id;
    map_building_tiles_add(b->id, x, y, 1,
                           image_id_from_group(GROUP_BUILDING_WAREHOUSE_STORAGE_EMPTY), TERRAIN_BUILDING);
    return b;
}
static void add_warehouse(building *b) {
    int x_offset[9] = {0, 0, 1, 1, 0, 2, 1, 2, 2};
    int y_offset[9] = {0, 1, 0, 1, 2, 0, 2, 1, 2};
    int corner = building_rotation_get_corner(2 * building_rotation_get_rotation());

    b->storage_id = building_storage_create(BUILDING_WAREHOUSE);
    if (config_get(CONFIG_GP_CH_WAREHOUSES_DONT_ACCEPT))
        building_storage_accept_none(b->storage_id);

    b->prev_part_building_id = 0;
    map_building_tiles_add(b->id, b->x + x_offset[corner], b->y + y_offset[corner], 1,
                           image_id_from_group(GROUP_BUILDING_WAREHOUSE), TERRAIN_BUILDING);

    building *prev = b;
    for (int i = 0; i < 9; i++) {
        if (i == corner) {
            continue;
        }
        prev = add_warehouse_space(b->x + x_offset[i], b->y + y_offset[i], prev);
    }
    // adjust BUILDING_WAREHOUSE
    b->x = b->x + x_offset[corner];
    b->y = b->y + y_offset[corner];
    b->grid_offset = map_grid_offset(b->x, b->y);
    game_undo_adjust_building(b);

    prev->next_part_building_id = 0;
}

static void add_building_tiles_image(building *b, int image_id) {
    map_building_tiles_add(b->id, b->x, b->y, b->size, image_id, TERRAIN_BUILDING);
}
static void add_building(building *b, int orientation, int variant) {
    int orientation_rel = (4 + orientation - city_view_orientation() / 2) % 4;
    switch (b->type) {
        // houses
        case BUILDING_HOUSE_LARGE_TENT:
            add_building_tiles_image(b, image_id_from_group(GROUP_BUILDING_HOUSE_TENT) + 2);
            break;
        case BUILDING_HOUSE_SMALL_SHACK:
            add_building_tiles_image(b, image_id_from_group(GROUP_BUILDING_HOUSE_SHACK));
            break;
        case BUILDING_HOUSE_LARGE_SHACK:
            add_building_tiles_image(b, image_id_from_group(GROUP_BUILDING_HOUSE_SHACK) + 2);
            break;
        case BUILDING_HOUSE_SMALL_HOVEL:
            add_building_tiles_image(b, image_id_from_group(GROUP_BUILDING_HOUSE_HOVEL));
            break;
        case BUILDING_HOUSE_LARGE_HOVEL:
            add_building_tiles_image(b, image_id_from_group(GROUP_BUILDING_HOUSE_HOVEL) + 2);
            break;
        case BUILDING_HOUSE_SMALL_CASA:
            add_building_tiles_image(b, image_id_from_group(GROUP_BUILDING_HOUSE_CASA));
            break;
        case BUILDING_HOUSE_LARGE_CASA:
            add_building_tiles_image(b, image_id_from_group(GROUP_BUILDING_HOUSE_CASA) + 2);
            break;
        case BUILDING_HOUSE_SMALL_INSULA:
            add_building_tiles_image(b, image_id_from_group(GROUP_BUILDING_HOUSE_INSULA_1));
            break;
        case BUILDING_HOUSE_MEDIUM_INSULA:
            add_building_tiles_image(b, image_id_from_group(GROUP_BUILDING_HOUSE_INSULA_1) + 2);
            break;
        case BUILDING_HOUSE_LARGE_INSULA:
            add_building_tiles_image(b, image_id_from_group(GROUP_BUILDING_HOUSE_INSULA_2));
            break;
        case BUILDING_HOUSE_GRAND_INSULA:
            add_building_tiles_image(b, image_id_from_group(GROUP_BUILDING_HOUSE_INSULA_2) + 2);
            break;
        case BUILDING_HOUSE_SMALL_VILLA:
            add_building_tiles_image(b, image_id_from_group(GROUP_BUILDING_HOUSE_VILLA_1));
            break;
        case BUILDING_HOUSE_MEDIUM_VILLA:
            add_building_tiles_image(b, image_id_from_group(GROUP_BUILDING_HOUSE_VILLA_1) + 2);
            break;
        case BUILDING_HOUSE_LARGE_VILLA:
            add_building_tiles_image(b, image_id_from_group(GROUP_BUILDING_HOUSE_VILLA_2));
            break;
        case BUILDING_HOUSE_GRAND_VILLA:
            add_building_tiles_image(b, image_id_from_group(GROUP_BUILDING_HOUSE_VILLA_2) + 1);
            break;
        case BUILDING_HOUSE_SMALL_PALACE:
            add_building_tiles_image(b, image_id_from_group(GROUP_BUILDING_HOUSE_PALACE_1));
            break;
        case BUILDING_HOUSE_MEDIUM_PALACE:
            add_building_tiles_image(b, image_id_from_group(GROUP_BUILDING_HOUSE_PALACE_1) + 1);
            break;
        case BUILDING_HOUSE_LARGE_PALACE:
            add_building_tiles_image(b, image_id_from_group(GROUP_BUILDING_HOUSE_PALACE_2));
            break;
        case BUILDING_HOUSE_LUXURY_PALACE:
            add_building_tiles_image(b, image_id_from_group(GROUP_BUILDING_HOUSE_PALACE_2) + 1);
            break;
            // entertainment
        case BUILDING_BOOTH:
        case BUILDING_BANDSTAND:
        case BUILDING_PAVILLION:
        case BUILDING_FESTIVAL_SQUARE:
            add_entertainment_venue(b, orientation);
            break;
            // statues
        case BUILDING_SMALL_STATUE:
        case BUILDING_MEDIUM_STATUE:
        case BUILDING_LARGE_STATUE:
            add_building_tiles_image(b, get_statue_image(b->type, orientation_rel, variant));
            break;
            // farms
        case BUILDING_BARLEY_FARM:
            map_building_tiles_add_farm(b->id, b->x, b->y, 0, 0);
            break;
        case BUILDING_FLAX_FARM:
            map_building_tiles_add_farm(b->id, b->x, b->y, CROPS_OFFSETS[GAME_ENV], 0);
            break;
        case BUILDING_GRAIN_FARM:
            map_building_tiles_add_farm(b->id, b->x, b->y, CROPS_OFFSETS[GAME_ENV] * 2, 0);
            break;
        case BUILDING_LETTUCE_FARM:
            map_building_tiles_add_farm(b->id, b->x, b->y, CROPS_OFFSETS[GAME_ENV] * 3, 0);
            break;
        case BUILDING_POMEGRANATES_FARM:
            map_building_tiles_add_farm(b->id, b->x, b->y, CROPS_OFFSETS[GAME_ENV] * 4, 0);
            break;
        case BUILDING_CHICKPEAS_FARM:
            map_building_tiles_add_farm(b->id, b->x, b->y, CROPS_OFFSETS[GAME_ENV] * 5, 0);
            break;
        case BUILDING_FIGS_FARM:
            map_building_tiles_add_farm(b->id, b->x, b->y, CROPS_OFFSETS[GAME_ENV] * 6, 0);
            break;
        case BUILDING_HENNA_FARM:
            map_building_tiles_add_farm(b->id, b->x, b->y, CROPS_OFFSETS[GAME_ENV] * 7, 0);
            break;
            // government
        case BUILDING_TEMPLE_COMPLEX_OSIRIS:
        case BUILDING_TEMPLE_COMPLEX_RA:
        case BUILDING_TEMPLE_COMPLEX_PTAH:
        case BUILDING_TEMPLE_COMPLEX_SETH:
        case BUILDING_TEMPLE_COMPLEX_BAST:
            add_temple_complex(b, orientation);
            break;
//        case BUILDING_ROADBLOCK:
//            add_building_tiles_image(b, image_id_from_group(GROUP_BUILDING_ROADBLOCK));
//            map_terrain_add_roadblock_road(b->x, b->y, orientation);
//            break;
            // ships

        case BUILDING_WATER_LIFT: {
            auto props = building_properties_for_type(b->type);
            map_water_add_building(b->id, b->x, b->y, props->size,
                                   image_id_from_group(props->image_collection, props->image_group) + orientation_rel + 4 * variant);
            break;
        }
        case BUILDING_FISHING_WHARF:
        case BUILDING_TRANSPORT_WHARF:
        case BUILDING_SHIPYARD:
        case BUILDING_WARSHIP_WHARF:
        case BUILDING_FERRY:
        case BUILDING_DOCK: {
            auto props = building_properties_for_type(b->type);
            map_water_add_building(b->id, b->x, b->y, props->size,
                                   image_id_from_group(props->image_collection, props->image_group) + orientation_rel);
            break;
        }
            // defense
        case BUILDING_TOWER:
            map_terrain_remove_with_radius(b->x, b->y, 2, 0, TERRAIN_WALL);
            map_building_tiles_add(b->id, b->x, b->y, b->size, image_id_from_group(GROUP_BUILDING_TOWER),
                                   TERRAIN_BUILDING | TERRAIN_GATEHOUSE);
            map_tiles_update_area_walls(b->x, b->y, 5);
            break;
        case BUILDING_GATEHOUSE_PH:
        case BUILDING_GATEHOUSE:
            map_building_tiles_add(b->id, b->x, b->y, b->size,
                                   image_id_from_group(GROUP_BUILDING_TOWER) + orientation,
                                   TERRAIN_BUILDING | TERRAIN_GATEHOUSE);
//            map_orientation_update_buildings();
            map_terrain_add_gatehouse_roads(b->x, b->y, orientation);
            break;
        case BUILDING_TRIUMPHAL_ARCH:
            add_building_tiles_image(b, image_id_from_group(GROUP_BUILDING_TRIUMPHAL_ARCH) + orientation - 1);
//            map_orientation_update_buildings();
            map_terrain_add_triumphal_arch_roads(b->x, b->y, orientation);
            city_buildings_build_triumphal_arch();
            building_menu_update(BUILDSET_NORMAL);
            Planner.reset();
            break;
        case BUILDING_WAREHOUSE:
            add_warehouse(b);
            break;
        case BUILDING_FORT_CHARIOTEERS:
        case BUILDING_FORT_ARCHERS:
        case BUILDING_FORT_INFANTRY:
            add_fort(b->type, b);
            break;
        default:
            auto p = building_properties_for_type(b->type);
            add_building_tiles_image(b, image_id_from_group(p->image_collection, p->image_group) + p->image_offset);
            break;
    }
}

static void mark_construction(int x, int y, int size_x, int size_y, int terrain, bool absolute_xy) {
    if (Planner.can_be_placed() == CAN_PLACE && map_building_tiles_mark_construction(x, y, size_x, size_y, terrain, absolute_xy))
        Planner.draw_as_constructing = true;
}
static int has_nearby_enemy(int x_start, int y_start, int x_end, int y_end) {
    for (int i = 1; i < MAX_FIGURES[GAME_ENV]; i++) {
        figure *f = figure_get(i);
        if (config_get(CONFIG_GP_CH_WOLVES_BLOCK)) {
            if (f->state != FIGURE_STATE_ALIVE || (!f->is_enemy() && f->type != FIGURE_WOLF))
                continue;

        } else if (f->state != FIGURE_STATE_ALIVE || !f->is_enemy())
            continue;

        int dx = (f->tile_x > x_start) ? (f->tile_x - x_start) : (x_start - f->tile_x);
        int dy = (f->tile_y > y_start) ? (f->tile_y - y_start) : (y_start - f->tile_y);
        if (dx <= 12 && dy <= 12)
            return 1;

        dx = (f->tile_x > x_end) ? (f->tile_x - x_end) : (x_end - f->tile_x);
        dy = (f->tile_y > y_end) ? (f->tile_y - y_end) : (y_end - f->tile_y);
        if (dx <= 12 && dy <= 12)
            return 1;

    }
    return 0;
}

static int place_houses(bool measure_only, int x_start, int y_start, int x_end, int y_end) {
    int x_min, x_max, y_min, y_max;
    map_grid_start_end_to_area(x_start, y_start, x_end, y_end, &x_min, &y_min, &x_max, &y_max);

    int needs_road_warning = 0;
    int items_placed = 0;
    game_undo_restore_building_state();
    for (int y = y_min; y <= y_max; y++) {
        for (int x = x_min; x <= x_max; x++) {
            int grid_offset = map_grid_offset(x, y);
            if (map_terrain_is(grid_offset, TERRAIN_NOT_CLEAR) || map_terrain_exists_tile_in_radius_with_type(x, y, 1, 1, TERRAIN_FLOODPLAIN))
                continue;

            if (measure_only) {
                map_property_mark_constructing(grid_offset);
                items_placed++;
            } else {
                if (formation_herd_breeding_ground_at(x, y, 1)) {
                    map_property_clear_constructing_and_deleted();
                    city_warning_show(WARNING_HERD_BREEDING_GROUNDS);
                } else {
                    building *b = building_create(BUILDING_HOUSE_VACANT_LOT, x, y, 0);
                    game_undo_add_building(b);
                    if (b->id > 0) {
                        items_placed++;
                        map_building_tiles_add(b->id, x, y, 1,
                                               image_id_from_group(GROUP_BUILDING_HOUSE_VACANT_LOT), TERRAIN_BUILDING);
                        if (!map_terrain_exists_tile_in_radius_with_type(x, y, 1, 2, TERRAIN_ROAD))
                            needs_road_warning = 1;
                    }
                }
            }
        }
    }
    if (!measure_only) {
        building_construction_warning_check_food_stocks(BUILDING_HOUSE_VACANT_LOT);
        if (needs_road_warning)
            city_warning_show(WARNING_HOUSE_TOO_FAR_FROM_ROAD);

        map_routing_update_land();
        window_invalidate();
    }
    return items_placed;
}
static int place_plaza(int x_start, int y_start, int x_end, int y_end) {
    int x_min, y_min, x_max, y_max;
    map_grid_start_end_to_area(x_start, y_start, x_end, y_end, &x_min, &y_min, &x_max, &y_max);
    game_undo_restore_map(1);

    int items_placed = 0;
    for (int y = y_min; y <= y_max; y++) {
        for (int x = x_min; x <= x_max; x++) {
            int grid_offset = map_grid_offset(x, y);
            if (map_terrain_is(grid_offset, TERRAIN_ROAD) &&
                !map_terrain_is(grid_offset, TERRAIN_WATER | TERRAIN_BUILDING | TERRAIN_AQUEDUCT)
                && map_tiles_is_paved_road(grid_offset)) {
                if (!map_property_is_plaza_or_earthquake(grid_offset))
                    items_placed++;

                map_image_set(grid_offset, 0);
                map_property_mark_plaza_or_earthquake(grid_offset);
                map_property_set_multi_tile_size(grid_offset, 1);
                map_property_mark_draw_tile(grid_offset);
            }
        }
    }
    map_tiles_update_all_plazas();
    return items_placed;
}
static int place_garden(int x_start, int y_start, int x_end, int y_end) {
    game_undo_restore_map(1);

    int x_min, y_min, x_max, y_max;
    map_grid_start_end_to_area(x_start, y_start, x_end, y_end, &x_min, &y_min, &x_max, &y_max);

    int items_placed = 0;
    for (int y = y_min; y <= y_max; y++) {
        for (int x = x_min; x <= x_max; x++) {
            int grid_offset = map_grid_offset(x, y);
            if (!map_terrain_is(grid_offset, TERRAIN_NOT_CLEAR) &&  !map_terrain_exists_tile_in_radius_with_type(x, y, 1, 1, TERRAIN_FLOODPLAIN)) {
                if (formation_herd_breeding_ground_at(x, y, 1)) {
                    map_property_clear_constructing_and_deleted();
                    city_warning_show(WARNING_HERD_BREEDING_GROUNDS);
                } else {
                    items_placed++;
                    map_terrain_add(grid_offset, TERRAIN_GARDEN);
                }
            }
        }
    }
    map_tiles_update_all_gardens();
    return items_placed;
}

building *last_created_building = nullptr;
static bool place_building(int type, int x, int y, int orientation, int variant) {

    // by default, get size from building's properties
    int size = building_properties_for_type(type)->size;
    int check_figures = 2;
    switch (type) { // special cases
        case BUILDING_WAREHOUSE:
            size = 3; break;
        case BUILDING_BOOTH:
            check_figures = 1;
            size = 2; break;
        case BUILDING_BANDSTAND:
            check_figures = 1;
            size = 3; break;
        case BUILDING_PAVILLION:
            check_figures = 1;
            size = 4; break;
        case BUILDING_FESTIVAL_SQUARE:
            check_figures = 1;
            size = 5; break;
    }

    // correct building placement for city orientations
    switch (city_view_orientation()) {
        case DIR_2_BOTTOM_RIGHT:
            x = x - size + 1;
            break;
        case DIR_4_BOTTOM_LEFT:
            x = x - size + 1;
            y = y - size + 1;
            break;
        case DIR_6_TOP_LEFT:
            y = y - size + 1;
            break;
    }

    // create building
    last_created_building = nullptr;
    building *b;
//    if (building_is_fort(type)) // TODO
//        b = building_create(BUILDING_MENU_FORTS, x, y);
    b = building_create(type, x, y, orientation);
    game_undo_add_building(b);
    if (b->id <= 0) // building creation failed????
        return false;
    add_building(b, orientation, variant);
    last_created_building = b;
    return true;
}

static bool attach_temple_upgrade(int upgrade_param, int grid_offset) {
    building *target = building_at(grid_offset)->main();
    if (!building_at(grid_offset) || !building_is_large_temple(target->type))
        return false;
    if (target->data.monuments.temple_complex_attachments & upgrade_param)
        return false;
    target->data.monuments.temple_complex_attachments |= upgrade_param;
    map_building_tiles_add_temple_complex_parts(target);
    building_menu_update_temple_complexes();
    return true;
}
static map_tile temple_complex_part_target(building *main, int part) {
    building *b = main;
    if (part == 1)
        b = b->next();
    else if (part == 2)
        b = get_temple_complex_front_facing_part(main);

    int x = b->x;
    int y = b->y;
    switch (city_view_orientation() / 2) {
        case 1: // east
            x += 2;
            break;
        case 2: // south
            x += 2;
            y += 2;
            break;
        case 3: // west
            y += 2;
            break;
    }
    return {x, y, map_grid_offset(x, y)};
}

//////////////////////

int BuildPlanner::can_be_placed() {
    return can_place;
}

void BuildPlanner::reset() {

    // reset build info
    total_cost = 0;
    build_type = BUILDING_NONE;
    in_progress = false;
    draw_as_constructing = false;

    // set boundary size and reset pivot
    size = {0, 0};
    pivot = {0, 0};
    tiles_blocked_total = 0;

    // position and orientation
    start.x = 0;
    start.y = 0;
    end.x = 0;
    end.y = 0;
    orientation = 0;
    variant = 0;

    // reset special requirements flags/params
    special_flags = 0;
    additional_req_param1 = -1;
    additional_req_param2 = -1;
    additional_req_param3 = -1;
    can_place = CAN_PLACE;
    immediate_warning_id = -1;
    extra_warning_id = -1;
}
void BuildPlanner::init_tiles(int size_x, int size_y) {
    size.x = size_x;
    size.y = size_y;
    for (int row = 0; row < size.y; ++row) {
        for (int column = 0; column < size.x; ++column) {
            if (column > 29 || row > 29)
                return;
            tile_graphics_array[row][column] = 0;
            tile_sizes_array[row][column] = 1; // reset tile size to 1 by default
            tile_blocked_array[row][column] = false;

            // reset caches
            tile_coord_cache[row][column] = {0, 0};
            pixel_coords_cache[row][column] = {0, 0};
        }
    }
}

void BuildPlanner::set_tile_size(int row, int column, int size) {
    if (row > 29 || column > 29)
        return;
    tile_sizes_array[row][column] = size;
}
void BuildPlanner::set_flag(long long flags, int param1, int param2, int param3) {
    special_flags |= flags;
    if (param1 != -1)
        additional_req_param1 = param1;
    if (param2 != -1)
        additional_req_param2 = param2;
    if (param3 != -1)
        additional_req_param3 = param3;
}
bool BuildPlanner::has_flag_set(int flag, int param1, int param2, int param3) {
    if (param1 != -1 && additional_req_param1 != param1)
        return false;
    if (param2 != -1 && additional_req_param2 != param2)
        return false;
    if (param3 != -1 && additional_req_param3 != param3)
        return false;
    if (special_flags & flag)
        return true;
    return false;
}

void BuildPlanner::set_graphics_row(int row, int *image_ids, int total) {
    for (int i = 0; i < total; ++i) {
        if (row > 29 || i > 29)
            return;
        tile_graphics_array[row][i] = image_ids[i];

        // set sizes automatically as default
        int tile_size = 0;
        if (image_ids[i] != 0) {
            auto img = image_get(image_ids[i]);
            int tile_size = (img->width + 2) / 60;
            set_tile_size(row, i, tile_size);
        }
    }
}
void BuildPlanner::set_tiles_building(int image_id, int size_xx) {
    init_tiles(size_xx, size_xx);
    int empty_row[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int draw_row[] = {image_id, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    for (int row = 0; row < size.y; ++row) {
        if (row == size.y - 1)
            set_graphics_row(row, draw_row, size.x);
        else
            set_graphics_row(row, empty_row, size.x);
    }
}
void BuildPlanner::set_graphics_array(int *image_set, int size_x, int size_y) {
    init_tiles(size_x, size_y);
    int (*image_array)[size_y][size_x] = (int(*)[size_y][size_x])image_set;

    // do it row by row...
    for (int row = 0; row < size_y; ++row)
        set_graphics_row(row, (*image_array)[row], size_x);
}

void BuildPlanner::setup_build(int type) { // select building for construction, set up main terrain restrictions/requirements

    // initial data
    reset();
    build_type = type;

    // ignore empty building types
    switch (type) {
        case BUILDING_NONE:
        case BUILDING_MENU_FARMS:
        case BUILDING_MENU_RAW_MATERIALS:
        case BUILDING_MENU_SHRINES:
        case BUILDING_MENU_TEMPLES:
        case BUILDING_MENU_TEMPLE_COMPLEX:
        case BUILDING_MENU_GUILDS:
        case BUILDING_MENU_DEFENSES:
        case BUILDING_MENU_FORTS:
        case BUILDING_MENU_WATER_CROSSINGS:
        case BUILDING_MENU_BEAUTIFICATION:
        case BUILDING_MENU_MONUMENTS:
            return;
    }

    // leftover special cases....
    switch (build_type) {
        case BUILDING_SMALL_STATUE:
        case BUILDING_MEDIUM_STATUE:
        case BUILDING_LARGE_STATUE:
            //
        case BUILDING_TEMPLE_COMPLEX_OSIRIS:
        case BUILDING_TEMPLE_COMPLEX_RA:
        case BUILDING_TEMPLE_COMPLEX_PTAH:
        case BUILDING_TEMPLE_COMPLEX_SETH:
        case BUILDING_TEMPLE_COMPLEX_BAST:
            orientation = 1;
            break;
    }

    // load building data
    setup_build_flags();
    setup_build_graphics();
}
void BuildPlanner::setup_build_flags() {
    switch (build_type) {
        case BUILDING_BARLEY_FARM:
        case BUILDING_FLAX_FARM:
        case BUILDING_GRAIN_FARM:
        case BUILDING_LETTUCE_FARM:
        case BUILDING_POMEGRANATES_FARM:
        case BUILDING_CHICKPEAS_FARM:
        case BUILDING_FIGS_FARM:
        case BUILDING_HENNA_FARM:
            set_flag(PlannerFlags::Meadow);
            break;
        case BUILDING_STONE_QUARRY:
        case BUILDING_LIMESTONE_QUARRY:
        case BUILDING_GRANITE_QUARRY:
        case BUILDING_SANDSTONE_QUARRY:
            set_flag(PlannerFlags::Rock);
            break;
        case BUILDING_GOLD_MINE:
        case BUILDING_GEMSTONE_MINE:
        case BUILDING_COPPER_MINE:
            set_flag(PlannerFlags::Rock);
            set_flag(PlannerFlags::Ore);
            break;
        case BUILDING_CLAY_PIT:
            set_flag(PlannerFlags::NearbyWater);
            break;
        case BUILDING_TOWER:
            set_flag(PlannerFlags::Walls);
            break;
        case BUILDING_WELL:
        case BUILDING_WATER_SUPPLY:
        case BUILDING_VILLAGE_PALACE:
        case BUILDING_TOWN_PALACE:
        case BUILDING_CITY_PALACE:
            set_flag(PlannerFlags::Groundwater);
            break;
//        case BUILDING_LIBRARY: // TODO
//            set_requirements(PlannerReqs::Resources, RESOURCE_PAPYRUS, 300);
//            break;
//        case BUILDING_OBELYSK: // TODO
//            set_requirements(PlannerReqs::Resources, RESOURCE_GRANITE, 200);
//            break;
        case BUILDING_WATER_LIFT:
            set_flag(PlannerFlags::ShoreLine, 2);
            set_flag(PlannerFlags::FloodplainShore, 2);
            break;
        case BUILDING_FISHING_WHARF:
        case BUILDING_TRANSPORT_WHARF:
            set_flag(PlannerFlags::ShoreLine, 2);
            break;
        case BUILDING_SHIPYARD:
        case BUILDING_DOCK:
        case BUILDING_WARSHIP_WHARF:
            set_flag(PlannerFlags::ShoreLine, 3);
            break;
        case BUILDING_FERRY:
            set_flag(PlannerFlags::ShoreLine, 2);
            set_flag(PlannerFlags::Ferry);
            break;
        case BUILDING_LOW_BRIDGE:
        case BUILDING_SHIP_BRIDGE:
            set_flag(PlannerFlags::ShoreLine, 1);
            set_flag(PlannerFlags::Bridge);
            break;
        case BUILDING_ROAD:
            set_flag(PlannerFlags::Road, false);
            break;
        case BUILDING_ROADBLOCK:
            set_flag(PlannerFlags::Road, true, WARNING_ROADBLOCKS_ROAD_NEEDED);
            break;
        case BUILDING_PLAZA:
            set_flag(PlannerFlags::Road, true, -1);
            set_flag(PlannerFlags::FancyRoad);
            break;
        case BUILDING_BOOTH:
            set_flag(PlannerFlags::Intersection, 0, WARNING_BOOTH_ROAD_INTERSECTION_NEEDED);
            break;
        case BUILDING_BANDSTAND:
            set_flag(PlannerFlags::Intersection, 1, WARNING_BOOTH_ROAD_INTERSECTION_NEEDED);
            break;
        case BUILDING_PAVILLION:
            set_flag(PlannerFlags::Intersection, 2, WARNING_BOOTH_ROAD_INTERSECTION_NEEDED);
            break;
        case BUILDING_FESTIVAL_SQUARE:
            set_flag(PlannerFlags::Intersection, 3, WARNING_FESTIVAL_ROAD_INTERSECTION_NEEDED);
            break;
        case BUILDING_CLEAR_LAND:
            set_flag(PlannerFlags::IgnoreNearbyEnemy);
            break;
        case BUILDING_TEMPLE_COMPLEX_ALTAR:
            set_flag(PlannerFlags::TempleUpgrade, 2);
            break;
        case BUILDING_TEMPLE_COMPLEX_ORACLE:
            set_flag(PlannerFlags::TempleUpgrade, 1);
            break;
    }
    if (building_is_draggable(build_type))
        set_flag(PlannerFlags::Draggable);
}
void BuildPlanner::setup_build_graphics() {
    const building_properties *props = building_properties_for_type(build_type);
    switch (build_type) {
        case BUILDING_TEMPLE_COMPLEX_OSIRIS:
        case BUILDING_TEMPLE_COMPLEX_RA:
        case BUILDING_TEMPLE_COMPLEX_PTAH:
        case BUILDING_TEMPLE_COMPLEX_SETH:
        case BUILDING_TEMPLE_COMPLEX_BAST: { // CHANGE: in the original game, the cursor does not always follow the main building's top tile
            // size of every big item 3x3, in general 7x13
            // 25 max tiles at the moment to check blocked tiles
            int main_image_id = image_id_from_group(GROUP_BUILDING_TEMPLE_COMPLEX_MAIN, build_type);
            int oracle_image_id = image_id_from_group(GROUP_BUILDING_TEMPLE_COMPLEX_ORACLE, build_type);
            int altar_image_id = image_id_from_group(GROUP_BUILDING_TEMPLE_COMPLEX_ALTAR, build_type);
            int flooring_image_id = image_id_from_group(GROUP_BUILDING_TEMPLE_COMPLEX_FLOORING, build_type);
            int statue1_image_id = image_id_from_group(GROUP_BUILDING_TEMPLE_COMPLEX_STATUE_1, build_type);
            int statue2_image_id = image_id_from_group(GROUP_BUILDING_TEMPLE_COMPLEX_STATUE_2, build_type);

            int EMPTY = 0;
            int mn_1A = main_image_id;
            int mn_1B = main_image_id + 3;
            int mn_2A = oracle_image_id;
            int mn_2B = oracle_image_id + 3;
            int mn_3A = altar_image_id;
            int mn_3B = altar_image_id + 3;

            int til_0 = flooring_image_id + 0;
            int til_1 = flooring_image_id + 1;
            int til_2 = flooring_image_id + 2;
            int til_3 = flooring_image_id + 3;

            int smst0 = statue1_image_id + 0; // north
            int smst1 = statue1_image_id + 1; // east
            int smst2 = statue1_image_id + 2; // south
            int smst3 = statue1_image_id + 3; // west

            int lst0A = statue2_image_id + 0; // north
            int lst0B = statue2_image_id + 1;
            int lst1A = statue2_image_id + 2; // east
            int lst1B = statue2_image_id + 3;
            int lst2A = statue2_image_id + 4; // south
            int lst2B = statue2_image_id + 5;
            int lst3A = statue2_image_id + 6; // west
            int lst3B = statue2_image_id + 7;

            int orientation_rel = (4 + orientation - city_view_orientation() / 2) % 4;
            switch (orientation_rel) {
                case 0: { // NE
                    int TEMPLE_COMPLEX_SCHEME[13][7] = {
                            {til_3, lst1A, lst1B, til_1, lst3A, lst3B, til_3},
                            {til_2, lst1A, lst1B, til_1, lst3A, lst3B, til_2},
                            {til_3, lst1A, lst1B, til_1, lst3A, lst3B, til_3},
                            {til_2, til_0, til_0, til_1, til_0, til_0, til_2},
                            {til_0, til_0, EMPTY, EMPTY, EMPTY, til_0, til_0},
                            {smst3, til_0, EMPTY, EMPTY, EMPTY, til_0, smst1},
                            {smst3, til_0, mn_1B, EMPTY, EMPTY, til_0, smst1},
                            {til_1, til_1, EMPTY, EMPTY, EMPTY, til_1, til_1},
                            {smst3, til_0, EMPTY, EMPTY, EMPTY, til_0, smst1},
                            {smst3, til_0, mn_2B, EMPTY, EMPTY, til_0, smst1},
                            {til_1, til_1, EMPTY, EMPTY, EMPTY, til_1, til_1},
                            {smst3, til_0, EMPTY, EMPTY, EMPTY, til_0, smst1},
                            {smst3, til_0, mn_3B, EMPTY, EMPTY, til_0, smst1},
                    };
                    set_graphics_array((int *)TEMPLE_COMPLEX_SCHEME, 7, 13);
                    pivot = {2, 10};
                    break;
                }
                case 1: { // SE
                    int TEMPLE_COMPLEX_SCHEME[7][13] = {
                            {smst0, smst0, til_1, smst0, smst0, til_1, smst0, smst0, til_0, til_2, til_3, til_2, til_3},
                            {til_0, til_0, til_1, til_0, til_0, til_1, til_0, til_0, til_0, til_0, lst2B, lst2B, lst2B},
                            {EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, til_0, lst2A, lst2A, lst2A},
                            {EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, til_1, til_1, til_1, til_1},
                            {mn_1A, EMPTY, EMPTY, mn_2A, EMPTY, EMPTY, mn_3A, EMPTY, EMPTY, til_0, lst0B, lst0B, lst0B},
                            {til_0, til_0, til_1, til_0, til_0, til_1, til_0, til_0, til_0, til_0, lst0A, lst0A, lst0A},
                            {smst2, smst2, til_1, smst2, smst2, til_1, smst2, smst2, til_0, til_2, til_3, til_2, til_3},
                    };
                    set_graphics_array((int *)TEMPLE_COMPLEX_SCHEME, 13, 7);
                    pivot = {0, 2};
                    break;
                }
                case 2: { // SW
                    int TEMPLE_COMPLEX_SCHEME[13][7] = {
                            {smst3, til_0, EMPTY, EMPTY, EMPTY, til_0, smst1},
                            {smst3, til_0, EMPTY, EMPTY, EMPTY, til_0, smst1},
                            {til_1, til_1, mn_1B, EMPTY, EMPTY, til_1, til_1},
                            {smst3, til_0, EMPTY, EMPTY, EMPTY, til_0, smst1},
                            {smst3, til_0, EMPTY, EMPTY, EMPTY, til_0, smst1},
                            {til_1, til_1, mn_2B, EMPTY, EMPTY, til_1, til_1},
                            {smst3, til_0, EMPTY, EMPTY, EMPTY, til_0, smst1},
                            {smst3, til_0, EMPTY, EMPTY, EMPTY, til_0, smst1},
                            {til_0, til_0, mn_3B, EMPTY, EMPTY, til_0, til_0},
                            {til_2, til_0, til_0, til_1, til_0, til_0, til_2},
                            {til_3, lst1A, lst1B, til_1, lst3A, lst3B, til_3},
                            {til_2, lst1A, lst1B, til_1, lst3A, lst3B, til_2},
                            {til_3, lst1A, lst1B, til_1, lst3A, lst3B, til_3},
                    };
                    set_graphics_array((int *)TEMPLE_COMPLEX_SCHEME, 7, 13);
                    pivot = {2, 0};
                    break;
                }
                case 3: { // NW
                    int TEMPLE_COMPLEX_SCHEME[7][13] = {
                            {til_3, til_2, til_3, til_2, til_0, smst0, smst0, til_1, smst0, smst0, til_1, smst0, smst0},
                            {lst2B, lst2B, lst2B, til_0, til_0, til_0, til_0, til_1, til_0, til_0, til_1, til_0, til_0},
                            {lst2A, lst2A, lst2A, til_0, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY},
                            {til_1, til_1, til_1, til_1, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY},
                            {lst0B, lst0B, lst0B, til_0, mn_1A, EMPTY, EMPTY, mn_2A, EMPTY, EMPTY, mn_3A, EMPTY, EMPTY},
                            {lst0A, lst0A, lst0A, til_0, til_0, til_0, til_0, til_1, til_0, til_0, til_1, til_0, til_0},
                            {til_3, til_2, til_3, til_2, til_0, smst2, smst2, til_1, smst2, smst2, til_1, smst2, smst2},
                    };
                    set_graphics_array((int *)TEMPLE_COMPLEX_SCHEME, 13, 7);
                    pivot = {10, 2};
                    break;
                }
            }
            break;
        }
        case BUILDING_TEMPLE_COMPLEX_ALTAR:
        case BUILDING_TEMPLE_COMPLEX_ORACLE:
            init_tiles(3, 3);
            set_tiles_building(get_temple_complex_part_image(building_at(end.grid_offset)->main()->type, additional_req_param1, orientation, 1), 3);
            break;
        case BUILDING_WATER_LIFT:
            set_tiles_building(image_id_from_group(props->image_collection, props->image_group) + orientation + variant * 4, props->size);
            break;
        case BUILDING_FISHING_WHARF:
        case BUILDING_DOCK:
        case BUILDING_SHIPYARD:
        case BUILDING_WARSHIP_WHARF:
        case BUILDING_TRANSPORT_WHARF:
        case BUILDING_FERRY:
            set_tiles_building(image_id_from_group(props->image_collection, props->image_group) + orientation, props->size);
            break;
        case BUILDING_LOW_BRIDGE:
        case BUILDING_SHIP_BRIDGE:
            // TODO
            break;
        case BUILDING_SMALL_STATUE:
        case BUILDING_MEDIUM_STATUE:
        case BUILDING_LARGE_STATUE:
            set_tiles_building(get_statue_image(build_type, orientation, variant), props->size);
            break;
        case BUILDING_WAREHOUSE:
            set_tiles_building(image_id_from_group(props->image_collection, props->image_group), 3);
            break;
        case BUILDING_BOOTH:
            init_tiles(2, 2); // TODO
            break;
        case BUILDING_BANDSTAND:
            init_tiles(3, 3); // TODO
            break;
        case BUILDING_PAVILLION:
            init_tiles(4, 4); // TODO
            break;
        case BUILDING_FESTIVAL_SQUARE:
            init_tiles(5, 5); // TODO
            break;
        default: // regular buildings
            set_tiles_building(image_id_from_group(props->image_collection, props->image_group), props->size);
            break;
    }
}

void BuildPlanner::update_obstructions_check() {
    tiles_blocked_total = 0;
    for (int row = 0; row < size.y; row++) {
        for (int column = 0; column < size.x; column++) {

            // check terrain at coords
            map_tile current_tile = tile_coord_cache[row][column];
            unsigned int restricted_terrain = TERRAIN_ALL;

            // special cases
            if (special_flags & PlannerFlags::Meadow || special_flags & PlannerFlags::FloodplainShore)
                restricted_terrain -= TERRAIN_FLOODPLAIN;
            if (special_flags & PlannerFlags::Road || special_flags & PlannerFlags::Intersection)
                restricted_terrain -= TERRAIN_ROAD;
            if (special_flags & PlannerFlags::Water || special_flags & PlannerFlags::ShoreLine)
                restricted_terrain -= TERRAIN_WATER;
            if (special_flags & PlannerFlags::TempleUpgrade) // special case
                return;

            tile_blocked_array[row][column] = false;
            int grid_offset = map_grid_offset(current_tile.x, current_tile.y);
            if (!map_grid_is_inside(current_tile.x, current_tile.y, 1) || map_terrain_is(grid_offset, restricted_terrain & TERRAIN_NOT_CLEAR)) {
                tile_blocked_array[row][column] = true;
                tiles_blocked_total++;
            }
        }
    }
    if (tiles_blocked_total > 0) {
        immediate_warning_id = WARNING_CLEAR_LAND_NEEDED;
        can_place = CAN_NOT_BUT_GREEN;
    }
}
void BuildPlanner::update_requirements_check() {
    // invalid build
    if (build_type == BUILDING_NONE) {
        can_place = CAN_NOT_PLACE;
        return;
    }

    // out of money!
    if (city_finance_out_of_money()) {
        // TODO: no money needed if building has zero cost?
        immediate_warning_id = WARNING_OUT_OF_MONEY;
        can_place = CAN_NOT_PLACE;
        return;
    }

    /////// special requirements
    //
    if (special_flags & PlannerFlags::Resources) {
        if (city_resource_count(additional_req_param1) < additional_req_param2) {
            immediate_warning_id = additional_req_param3;
            can_place = CAN_NOT_BUT_GREEN;
        }
    }
    if (special_flags & PlannerFlags::Groundwater) {
        if (!map_terrain_exists_tile_in_radius_with_type(end.x, end.y, size.x, 0, TERRAIN_GROUNDWATER)) {
            immediate_warning_id = WARNING_GROUNDWATER_NEEDED;
            can_place = CAN_NOT_PLACE;
        }
    }
    if (special_flags & PlannerFlags::NearbyWater) {
        if (!map_terrain_exists_tile_in_radius_with_type(end.x, end.y, size.x, 3, TERRAIN_WATER)
            && !map_terrain_exists_tile_in_radius_with_type(end.x, end.y, size.x, 3, TERRAIN_FLOODPLAIN)) {
            immediate_warning_id = WARNING_WATER_NEEDED;
            can_place = CAN_NOT_PLACE;
        }
    }
    if (special_flags & PlannerFlags::Meadow) {
        if (!map_terrain_exists_tile_in_radius_with_type(end.x, end.y, size.x, 0, TERRAIN_MEADOW)
            && !map_terrain_all_tiles_in_radius_are(end.x, end.y, size.x, 0, TERRAIN_FLOODPLAIN)) {
            immediate_warning_id = WARNING_MEADOW_NEEDED;
            can_place = CAN_NOT_PLACE;
        }
    }
    if (special_flags & PlannerFlags::Rock) {
        if (!map_terrain_exists_tile_in_radius_with_type(end.x, end.y, size.x, 1, TERRAIN_ROCK)) {
            immediate_warning_id = WARNING_ROCK_NEEDED;
            can_place = CAN_NOT_PLACE;
        }
    }
    if (special_flags & PlannerFlags::Ore) {
        if (!map_terrain_exists_tile_in_radius_with_type(end.x, end.y, size.x, 1, TERRAIN_ORE)) {
            immediate_warning_id = WARNING_ROCK_NEEDED;
            can_place = CAN_NOT_PLACE;
        }
    }
    if (special_flags & PlannerFlags::Trees) {
        if (!map_terrain_exists_tile_in_radius_with_type(end.x, end.y, size.x, 1, TERRAIN_SHRUB | TERRAIN_TREE)) {
            immediate_warning_id = WARNING_TREE_NEEDED;
            can_place = CAN_NOT_PLACE;
        }
    }
    if (special_flags & PlannerFlags::Walls) {
        if (!map_terrain_all_tiles_in_radius_are(end.x, end.y, size.x, 0, TERRAIN_WALL)) {
            immediate_warning_id = WARNING_WALL_NEEDED;
            can_place = CAN_NOT_PLACE;
        }
    }
    if (special_flags & PlannerFlags::IgnoreNearbyEnemy == false) {
        if (has_nearby_enemy(start.x, start.y, end.x, end.y)) {
            immediate_warning_id = WARNING_ENEMY_NEARBY;
            can_place = CAN_NOT_PLACE;
        }
    }
    if (special_flags & PlannerFlags::Road && additional_req_param1 == true) {
        if (!map_terrain_is(end.grid_offset, TERRAIN_ROAD)) {
            immediate_warning_id = additional_req_param2;
            can_place = CAN_NOT_PLACE;
        }
    }
    if (special_flags & PlannerFlags::FancyRoad) {
        if (!map_tiles_is_paved_road(end.grid_offset)) {
            can_place = CAN_NOT_PLACE;
        }
    }
    if (special_flags & PlannerFlags::RiverAccess) {
        if (!building_dock_is_connected_to_open_water(end.x, end.y)) {
            immediate_warning_id = WARNING_DOCK_OPEN_WATER_NEEDED;
        }
    }
}
void BuildPlanner::update_special_case_orientations_check() {
    int dir_absolute, dir_relative;

    // for special buildings that require oriented terrain
    if (special_flags & PlannerFlags::ShoreLine) {
        bool match = map_shore_determine_orientation(end.x, end.y, additional_req_param1, true, &dir_absolute);
        if (special_flags & PlannerFlags::FloodplainShore) {
            variant = 0;
            if (!match) {
                match = map_shore_determine_orientation(end.x, end.y, additional_req_param1, true, &dir_absolute, true, TERRAIN_FLOODPLAIN);
                if (match && !map_terrain_exists_tile_in_area_with_type(end.x, end.y, size.x, TERRAIN_WATER)) // correct for water
                    variant = 1;
                else
                    match = false;
            } else if (map_terrain_exists_tile_in_area_with_type(end.x, end.y, size.x, TERRAIN_FLOODPLAIN)) // correct the ShoreLine check for floodplains!
                match = false;
        }
        dir_relative = (4 + dir_absolute - city_view_orientation() / 2) % 4;
        if (!match) {
            immediate_warning_id = WARNING_SHORE_NEEDED;
            can_place = CAN_NOT_PLACE;
        } else if (orientation != dir_relative) {
            orientation = dir_relative;
            update_orientations(false);
        }
    }
    if (special_flags & PlannerFlags::Intersection) {
        bool match = map_orientation_for_venue_with_map_orientation(end.x, end.y, additional_req_param1, &dir_relative);
        if (!match) {
            immediate_warning_id = additional_req_param2;
            can_place = CAN_NOT_PLACE;
        } else if (orientation != dir_relative) {
            orientation = dir_relative;
            update_orientations(false);
        }
    }
    if (special_flags & PlannerFlags::TempleUpgrade) {
        building *target = building_at(end.grid_offset)->main();
        if (!building_at(end.grid_offset) || !building_is_large_temple(target->type)) {
            immediate_warning_id = WARNING_TEMPLE_UPGRADE_PLACEMENT_NEED_TEMPLE;
            can_place = CAN_NOT_PLACE;
        } else if (target->data.monuments.temple_complex_attachments & additional_req_param1) {
            immediate_warning_id = WARNING_TEMPLE_UPGRADE_ONLY_ONE;
            can_place = CAN_NOT_PLACE;
        } else {
            dir_absolute = (5 - (target->data.monuments.variant / 2)) % 4;
            dir_relative = (4 + dir_absolute - city_view_orientation() / 2) % 4;
            orientation = (1 + dir_relative) % 2;
            end = temple_complex_part_target(target, additional_req_param1);
            update_orientations(false);
        }
    }
}
void BuildPlanner::update_unique_only_one_check() {
    bool unique_already_placed = false;

    // for unique buildings - only one can be placed inside the mission
    switch (build_type) {
        case BUILDING_VILLAGE_PALACE:
        case BUILDING_CITY_PALACE:
        case BUILDING_TOWN_PALACE:
            if (city_buildings_has_palace())
                unique_already_placed = true;
            break;
        case BUILDING_PERSONAL_MANSION:
        case BUILDING_FAMILY_MANSION:
        case BUILDING_DYNASTY_MANSION:
            if (city_buildings_has_mansion())
                unique_already_placed = true;
            break;
        case BUILDING_RECRUITER:
            if (city_buildings_has_recruiter() && !config_get(CONFIG_GP_CH_MULTIPLE_BARRACKS))
                unique_already_placed = true;
            break;
        case BUILDING_FESTIVAL_SQUARE:
            if (city_building_has_festival_square())
                unique_already_placed = true;
            break;
        case BUILDING_TEMPLE_COMPLEX_OSIRIS:
        case BUILDING_TEMPLE_COMPLEX_RA:
        case BUILDING_TEMPLE_COMPLEX_PTAH:
        case BUILDING_TEMPLE_COMPLEX_SETH:
        case BUILDING_TEMPLE_COMPLEX_BAST:
//        case BUILDING_TEMPLE_COMPLEX_ALTAR:
//        case BUILDING_TEMPLE_COMPLEX_ORACLE:
            if (city_buildings_has_temple_complex() && !config_get(CONFIG_GP_CH_MULTIPLE_TEMPLE_COMPLEXES))
                unique_already_placed = true;
            break;
    }
    if (unique_already_placed) {
        immediate_warning_id = WARNING_ONE_BUILDING_OF_TYPE;
        can_place = CAN_NOT_PLACE;
    }
}
void BuildPlanner::update_coord_caches() {
    pixel_coordinate view_tile = city_view_grid_offset_to_pixel(end.grid_offset);
    if (view_tile.x == 0 && view_tile.y == 0)
        // this prevents graphics from being drawn on the top left corner
        // of the screen when the current "end" tile isn't valid.
        city_view_get_selected_tile_pixels(&view_tile.x, &view_tile.y);
    int orientation = city_view_orientation() / 2;
    for (int row = 0; row < size.y; row++) {
        for (int column = 0; column < size.x; column++) {

            // get tile offset
            int x_offset = (column - pivot.x);
            int y_offset = (row - pivot.y);

            // get abs. tile
            int tile_x = 0;
            int tile_y = 0;
            switch (orientation) {
                case 0:
                    tile_x = end.x + x_offset;
                    tile_y = end.y + y_offset;
                    break;
                case 1:
                    tile_x = end.x - y_offset;
                    tile_y = end.y + x_offset;
                    break;
                case 2:
                    tile_x = end.x - x_offset;
                    tile_y = end.y - y_offset;
                    break;
                case 3:
                    tile_x = end.x + y_offset;
                    tile_y = end.y - x_offset;
                    break;
            }

            // get tile pixel coords
            int current_x = view_tile.x + x_offset * 30 - y_offset * 30;
            int current_y = view_tile.y + x_offset * 15 + y_offset * 15;

            // save values in cache
            tile_coord_cache[row][column] = {tile_x, tile_y, map_grid_offset(tile_x, tile_y)};
            pixel_coords_cache[row][column] = {current_x, current_y};
        }
    }
    switch (orientation) {
        case 0: // north
            north_tile = tile_coord_cache[0][0];
            east_tile = tile_coord_cache[0][size.x - 1];
            south_tile = tile_coord_cache[size.y - 1][size.x - 1];
            west_tile = tile_coord_cache[size.y - 1][0];
            break;
        case 1: // east
            north_tile = tile_coord_cache[size.y - 1][0];
            east_tile = tile_coord_cache[0][0];
            south_tile = tile_coord_cache[0][size.x - 1];
            west_tile = tile_coord_cache[size.y - 1][size.x - 1];
            break;
        case 2: // south
            north_tile = tile_coord_cache[size.y - 1][size.x - 1];
            east_tile = tile_coord_cache[size.y - 1][0];
            south_tile = tile_coord_cache[0][0];
            west_tile = tile_coord_cache[0][size.x - 1];
            break;
        case 3: // west
            north_tile = tile_coord_cache[0][size.x - 1];
            east_tile = tile_coord_cache[size.y - 1][size.x - 1];
            south_tile = tile_coord_cache[size.y - 1][0];
            west_tile = tile_coord_cache[0][0];
            break;
    }
}
void BuildPlanner::update_orientations(bool check_if_changed) {
    int prev_orientation = orientation;
    int prev_variant = variant;
    switch (build_type) {
        case BUILDING_SMALL_STATUE:
        case BUILDING_MEDIUM_STATUE:
        case BUILDING_LARGE_STATUE:
            orientation = building_rotation_get_rotation() + 1;
            variant = building_rotation_get_building_variant();
            break;
        case BUILDING_TEMPLE_COMPLEX_OSIRIS:
        case BUILDING_TEMPLE_COMPLEX_RA:
        case BUILDING_TEMPLE_COMPLEX_PTAH:
        case BUILDING_TEMPLE_COMPLEX_SETH:
        case BUILDING_TEMPLE_COMPLEX_BAST: // CHANGE: in the original game, only two orientations are allowed
            orientation = building_rotation_get_rotation() + 1;
            variant = 0;
            break;
    }
    orientation = orientation % 4;

    // do not refresh graphics if nothing changed
    if (check_if_changed && orientation == prev_orientation && variant == prev_variant)
        return;
    setup_build_graphics(); // reload graphics, tiles, etc.
    update_coord_caches(); // refresh caches
}
void BuildPlanner::construction_record_view_position(int view_x, int view_y, int grid_offset) {
    if (grid_offset == start.grid_offset) {
        start_offset_x_view = view_x;
        start_offset_y_view = view_y;
    }
}
void BuildPlanner::dispatch_warnings() {
    if (immediate_warning_id > -1)
        city_warning_show(immediate_warning_id);
    if (extra_warning_id > -1)
        city_warning_show(extra_warning_id);
}

int BuildPlanner::get_total_drag_size(int *x, int *y) {
    if (!config_get(CONFIG_UI_SHOW_CONSTRUCTION_SIZE) ||
        !(special_flags & PlannerFlags::Draggable) ||
        (build_type != BUILDING_CLEAR_LAND && !total_cost)) {
        return 0;
    }
    int size_x = end.x - start.x;
    int size_y = end.y - start.y;
    if (size_x < 0)
        size_x = -size_x;

    if (size_y < 0)
        size_y = -size_y;

    size_x++;
    size_y++;
    *x = size_x;
    *y = size_y;
    return 1;
}
void BuildPlanner::construction_start(int x, int y, int grid_offset) {
    start.grid_offset = grid_offset;
    start.x = end.x = x;
    start.y = end.y = y;

    if (game_undo_start_build(build_type)) {
        in_progress = true;
        int can_start = true;
        switch (build_type) {
            case BUILDING_ROAD:
                can_start = map_routing_calculate_distances_for_building(ROUTED_BUILDING_ROAD, start.x, start.y);
                break;
            case BUILDING_IRRIGATION_DITCH:
//            case BUILDING_WATER_LIFT:
                can_start = map_routing_calculate_distances_for_building(ROUTED_BUILDING_AQUEDUCT, start.x, start.y);
                break;
            case BUILDING_WALL:
                can_start = map_routing_calculate_distances_for_building(ROUTED_BUILDING_WALL, start.x, start.y);
                break;
            default:
                break;
        }
        if (!can_start)
            construction_cancel();
    }
}
void BuildPlanner::construction_cancel() {
    map_property_clear_constructing_and_deleted();
    if (in_progress && special_flags & PlannerFlags::Draggable) {
        game_undo_restore_map(1);
        in_progress = false;
    } else {
        setup_build(BUILDING_NONE);
        widget_sidebar_city_release_build_buttons();
    }
    building_rotation_reset_rotation();
}
void BuildPlanner::construction_update(int x, int y, int grid_offset) {
    if (grid_offset) {
        end.x = x;
        end.y = y;
        end.grid_offset = grid_offset;
    } else {
        x = end.x;
        y = end.y;
        grid_offset = end.grid_offset;
    }
    if (!build_type || city_finance_out_of_money()) {
        total_cost = 0;
        return;
    }
    map_property_clear_constructing_and_deleted();
    int current_cost = model_get_building(build_type)->cost;

    int items_placed = 1;
    switch (build_type) {
        case BUILDING_CLEAR_LAND:
            items_placed = last_items_cleared = building_construction_clear_land(1, start.x, start.y, x, y);
            break;
        case BUILDING_WALL:
            items_placed = building_construction_place_wall(1, start.x, start.y, x, y);
            break;
        case BUILDING_ROAD:
            items_placed = building_construction_place_road(1, start.x, start.y, x, y);
            break;
        case BUILDING_PLAZA:
            items_placed = place_plaza(start.x, start.y, x, y);
            break;
        case BUILDING_GARDENS:
            items_placed = place_garden(start.x, start.y, x, y);
            break;
        case BUILDING_IRRIGATION_DITCH:
            items_placed = building_construction_place_aqueduct(start.x, start.y, x, y, &current_cost);
            map_tiles_update_all_aqueducts(0);
            break;
        case BUILDING_LOW_BRIDGE:
        case BUILDING_SHIP_BRIDGE:
            items_placed = map_bridge_building_length();
            break;
        case BUILDING_HOUSE_VACANT_LOT:
            items_placed = place_houses(1, start.x, start.y, x, y);
            break;
        case BUILDING_GATEHOUSE_PH:
            mark_construction(x, y, 1, 3, ~TERRAIN_ROAD, false); // TODO
            break;
        case BUILDING_TRIUMPHAL_ARCH:
            mark_construction(x, y, 3, 3, ~TERRAIN_ROAD, false);
            break;
        case BUILDING_WAREHOUSE:
            mark_construction(x, y, 3, 3, TERRAIN_ALL, false);
            break;
        case BUILDING_WATER_LIFT:
        case BUILDING_FISHING_WHARF:
        case BUILDING_TRANSPORT_WHARF:
        case BUILDING_SHIPYARD:
        case BUILDING_DOCK:
        case BUILDING_WARSHIP_WHARF:
        case BUILDING_FERRY:
            if (map_shore_determine_orientation(end.x, end.y, additional_req_param1, true, nullptr))
                draw_as_constructing = true;
            else
                draw_as_constructing = false;
            break;
        default:
            if (special_flags & PlannerFlags::Meadow || special_flags & PlannerFlags::Rock || special_flags & PlannerFlags::Trees ||
                special_flags & PlannerFlags::NearbyWater || special_flags & PlannerFlags::Walls || special_flags & PlannerFlags::Groundwater ||
                special_flags & PlannerFlags::Water || special_flags & PlannerFlags::ShoreLine || special_flags & PlannerFlags::Road ||
                special_flags & PlannerFlags::Intersection) {
                // never draw as constructing
            } else {
                if ((city_view_orientation() / 2) % 2 == 0)
                    mark_construction(north_tile.x, north_tile.y, size.x, size.y, TERRAIN_ALL, true);
                else
                    mark_construction(north_tile.x, north_tile.y, size.y, size.x, TERRAIN_ALL, true);
            }
    }
    if (items_placed >= 0)
        current_cost *= items_placed;
    total_cost = current_cost;

    if (building_is_fort(build_type)) {
        if (formation_get_num_legions_cached() < 6) {
            if (map_building_tiles_are_clear(x, y, 3, TERRAIN_ALL) &&
                map_building_tiles_are_clear(
                        x + FORT_X_OFFSET[building_rotation_get_rotation()][city_view_orientation() / 2],
                        y + FORT_Y_OFFSET[building_rotation_get_rotation()][city_view_orientation() / 2], 4,
                        TERRAIN_ALL)) {
                mark_construction(x, y, 3, 3, TERRAIN_ALL, false);
            }
        }
    }
}
void BuildPlanner::construction_finalize() { // confirm final placement
    in_progress = false;

    dispatch_warnings();
    if (can_place != CAN_PLACE) // this is the FINAL check!
        return;

    // attempt placing, restore terrain data on failure
    if (!place()) {
        map_property_clear_constructing_and_deleted();
        if (build_type == BUILDING_WALL || build_type == BUILDING_ROAD || build_type == BUILDING_IRRIGATION_DITCH)
            game_undo_restore_map(0);
        else if (build_type == BUILDING_PLAZA || build_type == BUILDING_GARDENS)
            game_undo_restore_map(1);
        else if (build_type == BUILDING_LOW_BRIDGE || build_type == BUILDING_SHIP_BRIDGE)
            map_bridge_reset_building_length();
        return;
    }

    // final generic building warnings - these are in another file
    // TODO: bring these warnings over.
    building_construction_warning_generic_checks(build_type, end.x, end.y, size.x);

    // update city building info with newly created
    // building for special/unique constructions
    switch (build_type) {
        case BUILDING_DOCK:
            city_buildings_add_dock();
            break;
        case BUILDING_FESTIVAL_SQUARE:
            city_buildings_add_festival_square(last_created_building);
            break;
        case BUILDING_TEMPLE_COMPLEX_OSIRIS:
        case BUILDING_TEMPLE_COMPLEX_RA:
        case BUILDING_TEMPLE_COMPLEX_PTAH:
        case BUILDING_TEMPLE_COMPLEX_SETH:
        case BUILDING_TEMPLE_COMPLEX_BAST:
            city_buildings_add_temple_complex(last_created_building);
            break;
        case BUILDING_VILLAGE_PALACE:
        case BUILDING_TOWN_PALACE:
        case BUILDING_CITY_PALACE:
            city_buildings_add_palace(last_created_building);
            break;
        case BUILDING_PERSONAL_MANSION:
        case BUILDING_FAMILY_MANSION:
        case BUILDING_DYNASTY_MANSION:
            city_buildings_add_mansion(last_created_building);
            break;
        case BUILDING_RECRUITER:
            city_buildings_add_recruiter(last_created_building);
            break;
    }

    // update terrain data for certain special cases
    if (special_flags & PlannerFlags::Meadow) {
        for (int y = end.y; y < end.y + size.y; y++)
            for (int x = end.x; x < end.x + size.x; x++)
                map_set_floodplain_growth(map_grid_offset(x, y), 0);
    }
    if (special_flags & PlannerFlags::Road) {
        map_terrain_add_in_area(end.x, end.y, end.x + size.x - 1, end.y + size.y - 1, TERRAIN_ROAD);
        map_tiles_update_area_roads(end.x, end.y, 5);
        map_tiles_update_all_plazas();
    }
    if (special_flags & PlannerFlags::Walls)
        map_tiles_update_area_walls(end.x, end.y, 5);

    // consume resources for specific buildings (e.g. marble, granite)
    if (special_flags & PlannerFlags::Resources)
        building_warehouses_remove_resource(additional_req_param1, additional_req_param2);

    // finally, go over the rest of the stuff for all building types
    formation_move_herds_away(end.x, end.y);
    city_finance_process_construction(total_cost);
    game_undo_finish_build(total_cost);
    map_tiles_update_region_empty_land(false, start.x - 2, start.y - 2, end.x + size.x + 2, end.y + size.y + 2);
    map_routing_update_land();
    map_routing_update_walls();
}

//////////////////////

void BuildPlanner::update(const map_tile *cursor_tile) {
    end = *cursor_tile;
    update_coord_caches();

    immediate_warning_id = -1;
    can_place = CAN_PLACE;
    update_obstructions_check();
    update_requirements_check();
    update_special_case_orientations_check();
    update_unique_only_one_check();
}
bool BuildPlanner::place() {

    // attempt placing!
    int x = end.x;
    int y = end.y;

    // for debugging...
    SDL_Log("Attempting to place at: %03i %03i %06i", x, y, map_grid_offset(x, y));

    // Check warnings for placement and create building/update tiles accordingly.
    // Some of the buildings below have specific warning messages (e.g. roadblocks)
    // that can't be easily put in `building_construction_can_place_on_terrain()`!
    int placement_cost = model_get_building(build_type)->cost;
    switch (build_type) {
        case BUILDING_CLEAR_LAND: {
            // BUG in original (keep this behaviour): if confirmation has to be asked (bridge/fort),
            // the previous cost is deducted from treasury and if user chooses 'no', they still pay for removal.
            // If we don't do it this way, the user doesn't pay for the removal at all since we don't come back
            // here when the user says yes.
            int items_placed = building_construction_clear_land(0, start.x, start.y, end.x, end.y);
            if (items_placed < 0)
                items_placed = last_items_cleared;
            placement_cost *= items_placed;
            map_property_clear_constructing_and_deleted();
            break;
        }
        case BUILDING_WALL:
            placement_cost *= building_construction_place_wall(0, start.x, start.y, end.x, end.y);
            break;
        case BUILDING_ROAD:
            placement_cost *= building_construction_place_road(0, start.x, start.y, end.x, end.y);
            break;
        case BUILDING_PLAZA:
            placement_cost *= place_plaza(start.x, start.y, end.x, end.y);
            break;
        case BUILDING_GARDENS:
            placement_cost *= place_garden(start.x, start.y, end.x, end.y);
            map_routing_update_land();
            break;
        case BUILDING_LOW_BRIDGE:
        case BUILDING_SHIP_BRIDGE: {
            placement_cost *= map_bridge_add(x, y, build_type == BUILDING_SHIP_BRIDGE);
//            if (length <= 1) {
//                city_warning_show(WARNING_SHORE_NEEDED);
//                return false;
//            }
//            placement_cost *= length;
            break;
        }
        case BUILDING_IRRIGATION_DITCH: {
            int cost;
            if (!building_construction_place_aqueduct(start.x, start.y, end.x, end.y, &cost)) {
                city_warning_show(WARNING_CLEAR_LAND_NEEDED);
                return false;
            }
            placement_cost = cost;
            map_tiles_update_all_aqueducts(0);
            map_routing_update_land();
            break;
        }
        case BUILDING_HOUSE_VACANT_LOT:
            placement_cost *= place_houses(0, start.x, start.y, end.x, end.y);
            if (placement_cost == 0) {
                city_warning_show(WARNING_CLEAR_LAND_NEEDED);
                return false;
            }
            break;
        case BUILDING_TEMPLE_COMPLEX_ALTAR:
        case BUILDING_TEMPLE_COMPLEX_ORACLE:
            if (!attach_temple_upgrade(additional_req_param1, end.grid_offset))
                return false;
            break;
        default:
            if (!place_building(build_type, end.x, end.y, orientation, variant))
                return false;
            break;
    }

    // TODO
//    if (building_is_fort(building_type)) {
//        const int offsets_x[] = {3, -1, -4, 0};
//        const int offsets_y[] = {-1, -4, 0, 3};
//        int orient_index = building_rotation_get_rotation();
//        int x_offset = offsets_x[orient_index];
//        int y_offset = offsets_y[orient_index];
//        if (!map_tiles_are_clear(x + x_offset, y + y_offset, 4, terrain_mask)) {
//            city_warning_show(WARNING_CLEAR_LAND_NEEDED);
//            return 0;
//        }
//        if (formation_get_num_legions_cached() >= formation_get_max_legions()) {
//            city_warning_show(WARNING_MAX_LEGIONS_REACHED);
//            return 0;
//        }
//    }

    total_cost = placement_cost;
    if (total_cost == 0)
        return false;
    return true;
}