#pragma once

#include "building/building_storage.h"
#include "grid/point.h"

enum e_storageyard_state {
    STORAGEYARD_ROOM = 0,
    STORAGEYARD_FULL = 1,
    STORAGEYARD_SOME_ROOM = 2
};

enum e_storageyard_task {
    STORAGEYARD_TASK_NONE = -1,
    STORAGEYARD_TASK_GETTING = 0,
    STORAGEYARD_TASK_DELIVERING = 1,
    STORAGEYARD_TASK_EMPTYING = 2,
    STORAGEYARD_TASK_MONUMENT = 3,
    //
    STORAGEYARD_TASK_GETTING_MOAR = 9,
};

struct storage_worker_task {
    e_storageyard_task result = STORAGEYARD_TASK_NONE;
    building *space = nullptr;
    int amount = 0;
    e_resource resource = RESOURCE_NONE;
    building *dest = nullptr;
};

class building_storage_yard : public building_storage {
public:
    BUILDING_METAINFO(BUILDING_STORAGE_YARD, building_storage_yard)

    building_storage_yard(building &b) : building_storage(b), stored_full_amount(b.stored_full_amount) {}
    virtual void on_create(int orientation) override;
    virtual void on_place_update_tiles(int orientation, int variant) override;
    virtual void on_place_checks() override;
    virtual void spawn_figure() override;
    virtual void window_info_background(object_info &ctx) override;
    virtual void window_info_foreground(object_info &ctx) override;
    virtual e_sound_channel_city sound_channel() const { return SOUND_CHANNEL_CITY_STORAGE_YARD; }

    virtual building_storage_yard *dcast_storage_yard() override { return this; }
    virtual bool draw_ornaments_and_animations_height(painter &ctx, vec2i point, tile2i tile, color mask) override;
    
    building_storage_room *room() { return next()->dcast_storage_room(); }

    int amount(e_resource resource) const;
    int total_stored() const;
    bool is_not_accepting(e_resource resource);

    int remove_resource(e_resource resource, int amount);
    int add_resource(e_resource resource, int amount);
    bool is_getting(e_resource resource);

    int freespace(e_resource resource);
    int accepting_amount(e_resource resource);

    int for_getting(e_resource resource, tile2i *dst);

    static void ghost_preview(vec2i tile, painter &ctx);
    static storage_worker_task deliver_food_to_gettingup_granary(building *warehouse);

    storage_worker_task determine_worker_task();
    short &stored_full_amount;

private:
    void draw_warehouse_orders(object_info *c);
    void draw_warehouse(object_info *c);
    void draw_warehouse_orders_foreground(object_info *c);
    void draw_warehouse_foreground(object_info *c);
    int get_space_info() const;
    bool is_accepting(e_resource resource);
    building *add_storageyard_space(int x, int y, building *prev);
};

building_storage_yard *storage_yard_cast(building *b);

int get_storage_accepting_amount(building *b, e_resource resource);
int building_storage_yard_for_storing(tile2i tile, e_resource resource, int distance_from_entry, int road_network_id, int *understaffed, tile2i &dst);
void building_storageyard_remove_resource_curse(building* warehouse, int amount);
void building_storageyards_add_resource(e_resource resource, int amount);
int building_storageyards_remove_resource(e_resource resource, int amount);