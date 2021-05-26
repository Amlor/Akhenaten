#include "object.h"

#include "core/calc.h"
#include "core/image.h"
#include "empire/city.h"
#include "empire/trade_route.h"
#include "empire/type.h"
#include "game/animation.h"
#include "scenario/empire.h"
#include "core/game_environment.h"

#define MAX_OBJECTS 200

typedef struct {
    int in_use;
    int city_type;
    int city_name_id;
    int trade_route_open;
    int trade_route_cost;
    int city_sells_resource[10];
    int city_buys_resource[8];
    int trade40;
    int trade25;
    int trade15;
    empire_object obj;
} full_empire_object;

typedef struct {
    int type;
    //
    int unk01;
    //
    int image_x;
    int image_y;
    int border_offset_right;
    int border_offset_bottom;
    int image_id;
    //
    //
    //
    //
    //
    int text_align;
    //
    //
    //
    //
    int unk_flag_graphics;
    int name_id;
    //
    //
    int unk02[9];
    int unk03[25];
    int unk04[32];
//    int unk_32bit_pair1_a;
//    int unk_32bit_pair1_b;
//    int unk_32bit_pair2_a;
//    int unk_32bit_pair2_b;
//    int unk_32bit_pair3_a;
//    int unk_32bit_pair3_b;
//    int unk_32bit_pair4_a;
//    int unk_32bit_pair4_b;
    int unk_32bit_full_twos[4];

} full_empire_object_ph;

static full_empire_object objects[MAX_OBJECTS];

static int get_trade_amount_code(int index, int resource);
static int is_sea_trade_route(int route_id);

static void fix_image_ids(void) {
    int image_id = 0;
    for (int i = 0; i < MAX_OBJECTS; i++) {
        if (objects[i].in_use
            && objects[i].obj.type == EMPIRE_OBJECT_CITY
            && objects[i].city_type == EMPIRE_CITY_OURS) {
            image_id = objects[i].obj.image_id;
            break;
        }
    }
    if (image_id > 0 && image_id != image_id_from_group(GROUP_EMPIRE_CITY)) {
        // empire map uses old version of graphics: increase every graphic id
        int offset = image_id_from_group(GROUP_EMPIRE_CITY) - image_id;
        for (int i = 0; i < MAX_OBJECTS; i++) {
            if (!objects[i].in_use)
                continue;

            if (objects[i].obj.image_id) {
                objects[i].obj.image_id += offset;
                if (objects[i].obj.expanded.image_id)
                    objects[i].obj.expanded.image_id += offset;

            }
        }
    }
}

void empire_object_load(buffer *buf) {
    for (int i = 0; i < MAX_OBJECTS; i++) {
        full_empire_object *full = &objects[i];
        empire_object *obj = &full->obj;
        obj->id = i;
        obj->type = buf->read_u8();
        full->in_use = buf->read_u8();
        obj->animation_index = buf->read_u8();
        buf->skip(1);
        obj->x = buf->read_i16();
        obj->y = buf->read_i16();
        obj->width = buf->read_i16();
        obj->height = buf->read_i16();
        obj->image_id = buf->read_i16();
        obj->expanded.image_id = buf->read_i16();
        buf->skip(1);
        obj->distant_battle_travel_months = buf->read_u8();
        buf->skip(1);
        obj->text_align = buf->read_u8();
        obj->expanded.x = buf->read_i16();
        obj->expanded.y = buf->read_i16();
        full->city_type = buf->read_u8();
        full->city_name_id = buf->read_u8();
        obj->trade_route_id = buf->read_u8();
        full->trade_route_open = buf->read_u8();
        full->trade_route_cost = buf->read_i16();
        for (int r = 0; r < 10; r++) {
            full->city_sells_resource[r] = buf->read_u8();
        }
        buf->skip(2);
        for (int r = 0; r < 8; r++) {
            full->city_buys_resource[r] = buf->read_u8();
        }
        obj->invasion_path_id = buf->read_u8();
        obj->invasion_years = buf->read_u8();
        full->trade40 = buf->read_u16();
        full->trade25 = buf->read_u16();
        full->trade15 = buf->read_u16();
        buf->skip(6);
        if (GAME_ENV == ENGINE_ENV_PHARAOH)
            buf->skip(34);
    }

    fix_image_ids();
}

void empire_object_init_cities(void) {
    empire_city_clear_all();
    int route_index = 1;
    for (int i = 0; i < MAX_OBJECTS; i++) {
        if (!objects[i].in_use || objects[i].obj.type != EMPIRE_OBJECT_CITY)
            continue;

        full_empire_object *obj = &objects[i];
        empire_city *city = empire_city_get(route_index++);
        city->in_use = 1;
        city->type = obj->city_type;
        city->name_id = obj->city_name_id;
        if (obj->obj.trade_route_id < 0)
            obj->obj.trade_route_id = 0;

        if (obj->obj.trade_route_id >= 20)
            obj->obj.trade_route_id = 19;

        city->route_id = obj->obj.trade_route_id;
        city->is_open = obj->trade_route_open;
        city->cost_to_open = obj->trade_route_cost;
        city->is_sea_trade = is_sea_trade_route(obj->obj.trade_route_id);

        for (int resource = RESOURCE_MIN; resource < RESOURCE_MAX[GAME_ENV]; resource++) {
            city->sells_resource[resource] = 0;
            city->buys_resource[resource] = 0;
            if (city->type == EMPIRE_CITY_DISTANT_ROMAN
                || city->type == EMPIRE_CITY_DISTANT_FOREIGN
                || city->type == EMPIRE_CITY_VULNERABLE_ROMAN
                || city->type == EMPIRE_CITY_FUTURE_ROMAN) {
                continue;
            }
            if (empire_object_city_sells_resource(i, resource))
                city->sells_resource[resource] = 1;

            if (empire_object_city_buys_resource(i, resource))
                city->buys_resource[resource] = 1;

            int amount;
            switch (get_trade_amount_code(i, resource)) {
                case 1:
                    amount = 15;
                    break;
                case 2:
                    amount = 25;
                    break;
                case 3:
                    amount = 40;
                    break;
                default:
                    amount = 0;
                    break;
            }
            trade_route_init(city->route_id, resource, amount);
        }
        city->trader_entry_delay = 4;
        city->trader_figure_ids[0] = 0;
        city->trader_figure_ids[1] = 0;
        city->trader_figure_ids[2] = 0;
        city->empire_object_id = i;
    }
}

int empire_object_init_distant_battle_travel_months(int object_type) {
    int month = 0;
    for (int i = 0; i < MAX_OBJECTS; i++) {
        if (objects[i].in_use && objects[i].obj.type == object_type) {
            month++;
            objects[i].obj.distant_battle_travel_months = month;
        }
    }
    return month;
}

const empire_object *empire_object_get(int object_id) {
    return &objects[object_id].obj;
}

const empire_object *empire_object_get_our_city(void) {
    for (int i = 0; i < MAX_OBJECTS; i++) {
        if (objects[i].in_use) {
            const empire_object *obj = &objects[i].obj;
            if (obj->type == EMPIRE_OBJECT_CITY && objects[i].city_type == EMPIRE_CITY_OURS)
                return obj;

        }
    }
    return 0;
}

void empire_object_foreach(void (*callback)(const empire_object *)) {
    for (int i = 0; i < MAX_OBJECTS; i++) {
        if (objects[i].in_use)
            callback(&objects[i].obj);

    }
}

const empire_object *empire_object_get_battle_icon(int path_id, int year) {
    for (int i = 0; i < MAX_OBJECTS; i++) {
        if (objects[i].in_use) {
            empire_object *obj = &objects[i].obj;
            if (obj->type == EMPIRE_OBJECT_BATTLE_ICON &&
                obj->invasion_path_id == path_id && obj->invasion_years == year) {
                return obj;
            }
        }
    }
    return 0;
}

int empire_object_get_max_invasion_path(void) {
    int max_path = 0;
    for (int i = 0; i < MAX_OBJECTS; i++) {
        if (objects[i].in_use && objects[i].obj.type == EMPIRE_OBJECT_BATTLE_ICON) {
            if (objects[i].obj.invasion_path_id > max_path)
                max_path = objects[i].obj.invasion_path_id;

        }
    }
    return max_path;
}

int empire_object_get_closest(int x, int y) {
    int min_dist = 10000;
    int min_obj_id = 0;
    for (int i = 0; i < MAX_OBJECTS && objects[i].in_use; i++) {
        const empire_object *obj = &objects[i].obj;
        int obj_x, obj_y;
        if (scenario_empire_is_expanded()) {
            obj_x = obj->expanded.x;
            obj_y = obj->expanded.y;
        } else {
            obj_x = obj->x;
            obj_y = obj->y;
        }
        if (obj_x - 8 > x || obj_x + obj->width + 8 <= x)
            continue;

        if (obj_y - 8 > y || obj_y + obj->height + 8 <= y)
            continue;

        int dist = calc_maximum_distance(x, y, obj_x + obj->width / 2, obj_y + obj->height / 2);
        if (dist < min_dist) {
            min_dist = dist;
            min_obj_id = i + 1;
        }
    }
    return min_obj_id;
}

void empire_object_set_expanded(int object_id, int new_city_type) {
    objects[object_id].city_type = new_city_type;
    if (new_city_type == EMPIRE_CITY_TRADE)
        objects[object_id].obj.expanded.image_id = image_id_from_group(GROUP_EMPIRE_CITY_TRADE);
    else if (new_city_type == EMPIRE_CITY_DISTANT_ROMAN)
        objects[object_id].obj.expanded.image_id = image_id_from_group(GROUP_EMPIRE_CITY_DISTANT_ROMAN);

}

int empire_object_city_buys_resource(int object_id, int resource) {
    const full_empire_object *object = &objects[object_id];
    for (int i = 0; i < 8; i++) {
        if (object->city_buys_resource[i] == resource)
            return 1;

    }
    return 0;
}

int empire_object_city_sells_resource(int object_id, int resource) {
    const full_empire_object *object = &objects[object_id];
    for (int i = 0; i < 10; i++) {
        if (object->city_sells_resource[i] == resource)
            return 1;

    }
    return 0;
}

static int is_trade_city(int index) {
    if (objects[index].obj.type != EMPIRE_OBJECT_CITY)
        return 0;

    return objects[index].city_type > EMPIRE_CITY_OURS && objects[index].city_type < EMPIRE_CITY_FUTURE_ROMAN;
}

static int get_trade_amount_code(int index, int resource) {
    if (!is_trade_city(index))
        return 0;

    int resource_flag = 1 << resource;
    if (objects[index].trade40 & resource_flag)
        return 3;

    if (objects[index].trade25 & resource_flag)
        return 2;

    if (objects[index].trade15 & resource_flag)
        return 1;

    return 0;
}

static int is_sea_trade_route(int route_id) {
    for (int i = 0; i < MAX_OBJECTS; i++) {
        if (objects[i].in_use && objects[i].obj.trade_route_id == route_id) {
            if (objects[i].obj.type == EMPIRE_OBJECT_SEA_TRADE_ROUTE)
                return 1;

            if (objects[i].obj.type == EMPIRE_OBJECT_LAND_TRADE_ROUTE)
                return 0;

        }
    }
    return 0;
}

static int get_animation_offset(int image_id, int current_index) {
    if (current_index <= 0)
        current_index = 1;

    const image *img = image_get(image_id);
    int animation_speed = img->animation_speed_id;
    if (!game_animation_should_advance(animation_speed))
        return current_index;

    if (img->animation_can_reverse) {
        int is_reverse = 0;
        if (current_index & 0x80)
            is_reverse = 1;

        int current_sprite = current_index & 0x7f;
        if (is_reverse) {
            current_index = current_sprite - 1;
            if (current_index < 1) {
                current_index = 1;
                is_reverse = 0;
            }
        } else {
            current_index = current_sprite + 1;
            if (current_index > img->num_animation_sprites) {
                current_index = img->num_animation_sprites;
                is_reverse = 1;
            }
        }
        if (is_reverse)
            current_index = current_index | 0x80;

    } else {
        // Absolutely normal case
        current_index++;
        if (current_index > img->num_animation_sprites)
            current_index = 1;

    }
    return current_index;
}

int empire_object_update_animation(const empire_object *obj, int image_id) {
    return objects[obj->id].obj.animation_index = get_animation_offset(image_id, obj->animation_index);
}
