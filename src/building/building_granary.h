#pragma once

#include "building/building_storage.h"
#include "core/vec2i.h"
#include "grid/point.h"
#include "graphics/color.h"

enum e_granary_task {
    GRANARY_TASK_NONE = 0,
    GRANARY_TASK_GETTING = 1,
    GRANARY_TASK_EMPTYING = 2,
};

struct granary_task_status {
    e_granary_task status;
    e_resource resource;
};

struct granary_getting_result {
    int building_id;
    tile2i tile;
};

class building_granary : public building_storage {
public:
    BUILDING_METAINFO(BUILDING_GRANARY, building_granary)

    building_granary(building &b) : building_storage(b) {}
    virtual building_granary *dcast_granary() override { return this; }

    virtual void on_create(int orientation) override;
    virtual void spawn_figure() override;
    virtual e_sound_channel_city sound_channel() const override { return SOUND_CHANNEL_CITY_GRANARY; }
    virtual bool draw_ornaments_and_animations_height(painter &ctx, vec2i point, tile2i tile, color mask) override;
    virtual std::pair<int, int> get_tooltip() const;
    virtual void window_info_background(object_info &ctx) override;
    virtual void window_info_foreground(object_info &ctx) override;
    virtual int window_info_handle_mouse(const mouse *m, object_info &c) override;
    virtual void on_place_checks() override;
    virtual void update_day() override;

    virtual int amount(e_resource resource) const override;
    bool is_accepting(e_resource resource);
    int is_not_accepting(e_resource resource);
    bool is_getting(e_resource resource);
    granary_getting_result for_getting();
    int add_resource(e_resource resource, int is_produced, int amount);
    int total_stored() const;
    int space_for() const;
    int capacity_stored() const { return 3200; }
    int allow_food_types() const { return 4; }

    void bless();
    granary_task_status determine_worker_task();
    int remove_resource(e_resource resource, int amount);
    void draw_stores(vec2i point, color color_mask, painter &ctx);
    static int remove_for_getting_deliveryman(building* src, building* dst, e_resource& resource);

    void draw_orders_foreground(object_info &c);
};

void city_granaries_calculate_stocks();
int building_granary_for_storing(tile2i tile, e_resource resource, int distance_from_entry, int road_network_id, int force_on_stockpile, int* understaffed, tile2i* dst);
int building_getting_granary_for_storing(tile2i tile, e_resource resource, int distance_from_entry, int road_network_id, tile2i* dst);