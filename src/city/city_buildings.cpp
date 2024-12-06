#include "city_buildings.h"

#include "game/undo.h"
#include "core/profiler.h"
#include "city/warnings.h"
#include "city/population.h"
#include "grid/building.h"
#include "grid/tiles.h"
#include "grid/building_tiles.h"
#include "grid/routing/routing_terrain.h"
#include "io/io_buffer.h"

building g_all_buildings[5000];
std::span<building> g_city_buildings = make_span(g_all_buildings);

building *building_get(int id) {
    return &g_all_buildings[id];
}

std::span<building> &city_buildings() {
    return g_city_buildings;
}

building *building_next(int i, e_building_type type) {
    for (; i < MAX_BUILDINGS; ++i) {
        building *b = building_get(i);
        if (b->state == BUILDING_STATE_VALID && b->type == type)
            return b;
    }
    return nullptr;
}

int building_id_first(e_building_type type) {
    for (int i = 1; i < MAX_BUILDINGS; ++i) {
        building *b = building_get(i);
        if (b->state == BUILDING_STATE_VALID && b->type == type)
            return i;
    }
    return MAX_BUILDINGS;
}

building *building_first(e_building_type type) {
    for (int i = 1; i < MAX_BUILDINGS; ++i) {
        building *b = building_get(i);
        if (b->state == BUILDING_STATE_VALID && b->type == type)
            return b;
    }
    return nullptr;
}

building *building_create(e_building_type type, tile2i tile, int orientation) {
    building *b = nullptr;
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        if (g_all_buildings[i].state == BUILDING_STATE_UNUSED && !game_undo_contains_building(i)) {
            b = &g_all_buildings[i];
            break;
        }
    }

    if (!b) {
        city_warning_show(WARNING_DATA_LIMIT_REACHED);
        return &g_all_buildings[0];
    }

    b->clear_impl();

    memset(&(b->data), 0, sizeof(b->data));
    b->new_fill_in_data_for_type(type, tile, orientation);

    b->data.house.health = 100;

    return b;
}

building *building_at(int grid_offset) {
    return building_get(map_building_at(grid_offset));
}

building *building_at(int x, int y) {
    return building_get(map_building_at(tile2i(x, y)));
}

building *building_at(tile2i point) {
    return building_get(map_building_at(point.grid_offset()));
}

bool building_exists_at(int grid_offset, building *b) {
    b = nullptr;
    int b_id = map_building_at(grid_offset);
    if (b_id > 0) {
        b = building_get(b_id);
        if (b->state > BUILDING_STATE_UNUSED) {
            return true;
        } else {
            b = nullptr;
        }
    }
    return false;
}

bool building_exists_at(tile2i tile, building *b) {
    b = nullptr;
    int b_id = map_building_at(tile);
    if (b_id > 0) {
        b = building_get(b_id);
        if (b->state > BUILDING_STATE_UNUSED)
            return true;
        else
            b = nullptr;
    }
    return false;
}

struct building_extra_data_t {
    int highest_id_in_use;
    int highest_id_ever;
    int created_sequence;
    //    int incorrect_houses;
    //    int unfixable_houses;
};

building_extra_data_t building_extra_data = { 0, 0, 0 };

void building_clear_all() {
    for (int i = 0; i < MAX_BUILDINGS; i++) {
        memset(&g_all_buildings[i], 0, sizeof(building));
        g_all_buildings[i].id = i;
    }
    building_extra_data.highest_id_in_use = 0;
    building_extra_data.highest_id_ever = 0;
    building_extra_data.created_sequence = 0;
    //    extra.incorrect_houses = 0;
    //    extra.unfixable_houses = 0;
}

void building_update_highest_id() {
    OZZY_PROFILER_SECTION("Game/Run/Tick/Update Highest Id");
    auto &extra = building_extra_data;

    extra.highest_id_in_use = 0;
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        if (g_all_buildings[i].state != BUILDING_STATE_UNUSED)
            extra.highest_id_in_use = i;
    }
    if (extra.highest_id_in_use > extra.highest_id_ever)
        extra.highest_id_ever = extra.highest_id_in_use;
}

int building_get_highest_id() {
    return building_extra_data.highest_id_in_use;
}

static void building_delete_UNSAFE(building *b) {
    b->clear_related_data();
    int id = b->id;
    memset(b, 0, sizeof(building));
    b->id = id;
}

void building_update_state(void) {
    OZZY_PROFILER_SECTION("Game/Run/Tick/Building State Update");
    bool land_recalc = false;
    bool wall_recalc = false;
    bool road_recalc = false;
    bool water_routes_recalc = false;
    bool aqueduct_recalc = false;
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = &g_all_buildings[i];
        if (b->state == BUILDING_STATE_CREATED)
            b->state = BUILDING_STATE_VALID;

        if (b->state != BUILDING_STATE_VALID || !b->house_size) {
            if (b->state == BUILDING_STATE_UNDO || b->state == BUILDING_STATE_DELETED_BY_PLAYER) {
                if (b->type == BUILDING_MUD_TOWER || b->type == BUILDING_MUD_GATEHOUSE) {
                    wall_recalc = true;
                    road_recalc = true;
                } else if (b->type == BUILDING_WATER_LIFT) {
                    aqueduct_recalc = true;
                } else if (b->type == BUILDING_GRANARY) {
                    road_recalc = true;
                } else if (b->type == BUILDING_FERRY) {
                    water_routes_recalc = true;
                }

                map_building_tiles_remove(i, b->tile);
                road_recalc = true; // always recalc underlying road tiles
                land_recalc = true;
                building_delete_UNSAFE(b);
            } else if (b->state == BUILDING_STATE_RUBBLE) {
                if (b->house_size > 0) {
                    city_population_remove_home_removed(b->house_population);
                }

                building_delete_UNSAFE(b);
            } else if (b->state == BUILDING_STATE_DELETED_BY_GAME) {
                building_delete_UNSAFE(b);
            }
        }
    }

    if (wall_recalc) {
        map_tiles_update_all_walls();
    }

    if (aqueduct_recalc) {
        map_tiles_update_all_canals(0);
    }

    if (land_recalc) {
        map_routing_update_land();
    }

    if (road_recalc) {
        map_tiles_update_all_roads();
    }

    if (water_routes_recalc) {
        map_routing_update_ferry_routes();
    }
}

static void io_type_data(io_buffer *iob, building *b, size_t version) {
    auto &data = b->data;

    b->dcast()->bind_dynamic(iob, version);
    if (building_is_large_temple(b->type) || building_is_monument(b->type)) {
        iob->bind____skip(38);
        iob->bind(BIND_SIGNATURE_UINT8, &data.monuments.orientation);
        for (int i = 0; i < 5; i++) {
            iob->bind(BIND_SIGNATURE_UINT16, &data.monuments.workers[i]);
        }
        iob->bind(BIND_SIGNATURE_UINT8, &data.monuments.phase);
        iob->bind(BIND_SIGNATURE_UINT8, &data.monuments.statue_offset);
        iob->bind(BIND_SIGNATURE_UINT8, &data.monuments.temple_complex_attachments);
        iob->bind(BIND_SIGNATURE_UINT8, &data.monuments.variant);

        for (int i = 0; i < RESOURCES_MAX; i++) {
            iob->bind(BIND_SIGNATURE_UINT8, &data.monuments.resources_pct[i]);
        }
    } else {
        ; // nothing
    }
}

io_buffer *iob_buildings = new io_buffer([] (io_buffer *iob, size_t version) {
    for (int i = 0; i < MAX_BUILDINGS; i++) {
        //        building_state_load_from_buffer(buf, &all_buildings[i]);
        auto b = &g_all_buildings[i];
        int sind = (int)iob->get_offset();
        if (sind == 640) {
            int a = 2134;
        }

        iob->bind(BIND_SIGNATURE_UINT8, &b->state);
        iob->bind(BIND_SIGNATURE_UINT8, &b->faction_id);
        iob->bind(BIND_SIGNATURE_UINT8, &b->reserved_id);
        iob->bind(BIND_SIGNATURE_UINT8, &b->size);
        iob->bind(BIND_SIGNATURE_UINT8, &b->house_is_merged);
        iob->bind(BIND_SIGNATURE_UINT8, &b->house_size);
        iob->bind(BIND_SIGNATURE_TILE2I, b->tile);
        iob->bind____skip(2);
        iob->bind____skip(4);
        iob->bind(BIND_SIGNATURE_INT16, &b->type);
        iob->bind(BIND_SIGNATURE_INT16, &b->subtype.data); // which union field we use does not matter
        iob->bind(BIND_SIGNATURE_UINT16, &b->road_network_id);
        iob->bind____skip(2); //iob->bind(BIND_SIGNATURE_UINT16, &b->creation_sequence_index);
        iob->bind(BIND_SIGNATURE_INT16, &b->houses_covered);
        iob->bind(BIND_SIGNATURE_INT16, &b->percentage_houses_covered);

        iob->bind(BIND_SIGNATURE_INT16, &b->house_population);
        iob->bind(BIND_SIGNATURE_INT16, &b->house_population_room);
        iob->bind(BIND_SIGNATURE_INT16, &b->distance_from_entry);
        iob->bind(BIND_SIGNATURE_INT16, &b->house_highest_population);

        iob->bind(BIND_SIGNATURE_INT16, &b->house_unreachable_ticks);
        iob->bind(BIND_SIGNATURE_TILE2I, b->road_access);

        b->bind_iob_figures(iob);

        iob->bind(BIND_SIGNATURE_INT16, &b->figure_spawn_delay);
        iob->bind(BIND_SIGNATURE_UINT8, &b->figure_roam_direction);
        iob->bind(BIND_SIGNATURE_UINT8, &b->has_water_access);

        iob->bind(BIND_SIGNATURE_UINT8, &b->common_health);
        iob->bind(BIND_SIGNATURE_UINT8, &b->malaria_risk);
        iob->bind(BIND_SIGNATURE_INT16, &b->prev_part_building_id);
        iob->bind(BIND_SIGNATURE_INT16, &b->next_part_building_id);
        iob->bind(BIND_SIGNATURE_INT16, &b->stored_amount_first);
        iob->bind(BIND_SIGNATURE_UINT8, &b->disease_days);
        iob->bind(BIND_SIGNATURE_UINT8, &b->has_well_access);

        iob->bind(BIND_SIGNATURE_INT16, &b->num_workers);
        iob->bind(BIND_SIGNATURE_UINT8, &b->labor_category); // FF
        iob->bind(BIND_SIGNATURE_UINT8, &b->output_resource_first_id);
        iob->bind(BIND_SIGNATURE_UINT8, &b->has_road_access);
        iob->bind(BIND_SIGNATURE_UINT8, &b->house_criminal_active);

        iob->bind(BIND_SIGNATURE_INT16, &b->damage_risk);
        iob->bind(BIND_SIGNATURE_INT16, &b->fire_risk);
        iob->bind(BIND_SIGNATURE_INT16, &b->fire_duration);
        iob->bind(BIND_SIGNATURE_UINT8, &b->fire_proof);

        iob->bind(BIND_SIGNATURE_UINT8, &b->map_random_7bit); // 20 (workcamp 1)
        iob->bind(BIND_SIGNATURE_UINT8, &b->house_tax_coverage);
        iob->bind(BIND_SIGNATURE_UINT8, &b->health_proof);
        iob->bind(BIND_SIGNATURE_INT16, &b->formation_id);

        io_type_data(iob, b, version); // 102 for PH

        int currind = iob->get_offset() - sind;
        assert(currind > 0);
        iob->bind____skip(184 - currind);

        iob->bind(BIND_SIGNATURE_INT16, &b->tax_income_or_storage);
        iob->bind(BIND_SIGNATURE_INT16, &b->stored_amount_second);
        iob->bind(BIND_SIGNATURE_UINT8, &b->house_days_without_food);
        iob->bind(BIND_SIGNATURE_UINT8, &b->has_plague); // 1

        iob->bind(BIND_SIGNATURE_INT8, &b->desirability);
        iob->bind(BIND_SIGNATURE_UINT8, &b->is_deleted);
        iob->bind(BIND_SIGNATURE_UINT8, &b->is_adjacent_to_water);

        iob->bind(BIND_SIGNATURE_UINT8, &b->storage_id);
        iob->bind(BIND_SIGNATURE_INT8, &b->sentiment.house_happiness); // which union field we use does not matter // 90 for house, 50 for wells
        iob->bind(BIND_SIGNATURE_UINT8, &b->show_on_problem_overlay); // 1
        iob->bind(BIND_SIGNATURE_UINT16, &b->deben_storage); // 2
        iob->bind(BIND_SIGNATURE_UINT8, &b->has_open_water_access); // 1
        iob->bind(BIND_SIGNATURE_UINT8, &b->output_resource_second_id); // 1
        iob->bind(BIND_SIGNATURE_UINT8, &b->output_resource_second_rate); // 1

        iob->bind(BIND_SIGNATURE_INT16, &b->fancy_state); // 2
        iob->bind(BIND_SIGNATURE_INT8, &b->first_material_id);
        iob->bind(BIND_SIGNATURE_INT8, &b->second_material_id);
        // 59 additional bytes
        iob->bind____skip(59); // temp for debugging
        //            assert(iob->get_offset() - sind == 264);
        g_all_buildings[i].id = i;

        if (version <= 164) {
            b->common_health = 100;
            b->malaria_risk = 0;
            b->disease_days = 0;
            b->health_proof = 0;
        }
    }
    building_extra_data.created_sequence = 0;
});