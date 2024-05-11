#include "image.h"

#include "graphics/image.h"
#include "graphics/image_desc.h"
#include "graphics/image_groups.h"
#include "graphics/animation.h"

#include "js/js_game.h"

ANK_REGISTER_CONFIG_ITERATOR(config_load_cart_offsets);
ANK_REGISTER_CONFIG_ITERATOR(config_load_sled_offsets);
ANK_REGISTER_CONFIG_ITERATOR(config_load_cart_images);

static const int CORPSE_IMAGE_OFFSETS[128] = {
    0, 1, 2, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7
};

static const int MISSILE_LAUNCHER_OFFSETS[128] = {
    0, 1, 2, 3, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};


static vec2i CART_OFFSETS[] = {{17, -7}, {22, -1}, {17, 7}, {0, 11}, {-17, 6}, {-22, -1}, {-17, -7}, {0, -12}};
void config_load_cart_offsets() {
    int i = 0;
    g_config_arch.r_array("cart_offsets", [&i] (archive arch) {
        int x = arch.r_int("x");
        int y = arch.r_int("y");
        CART_OFFSETS[i] = vec2i{x, y};
        i++;
    });
}

static vec2i SLED_OFFSETS[] = {{17, -7}, {22, -1}, {17, 7}, {0, 11}, {-17, 6}, {-22, -1}, {-17, -7}, {0, -12}};
void config_load_sled_offsets() {
    int i = 0;
    g_config_arch.r_array("sled_offsets", [&i] (archive arch) {
        int x = arch.r_int("x");
        int y = arch.r_int("y");
        SLED_OFFSETS[i] = vec2i{x, y};
        i++;
    });
}

static image_desc g_cart_images[RESOURCES_MAX] = {{}};
void config_load_cart_images() {
    g_config_arch.r_array("cart_images", [] (archive arch) {
        e_resource res = arch.r_type<e_resource>("resource");
        int pack = arch.r_int("pack");
        int id = arch.r_int("id");
        int offset = arch.r_int("offset");
        g_cart_images[res] = {pack, id, offset};
    });
}

static void cc_coords_to_pixel_offset(int cross_country_x, int cross_country_y, int* pixel_x, int* pixel_y) {
    int dir = city_view_orientation();
    if (dir == DIR_0_TOP_RIGHT || dir == DIR_4_BOTTOM_LEFT) {
        int base_pixel_x = 2 * cross_country_x - 2 * cross_country_y;
        int base_pixel_y = cross_country_x + cross_country_y;
        *pixel_x = dir == DIR_0_TOP_RIGHT ? base_pixel_x : -base_pixel_x;
        *pixel_y = dir == DIR_0_TOP_RIGHT ? base_pixel_y : -base_pixel_y;
    } else {
        int base_pixel_x = 2 * cross_country_x + 2 * cross_country_y;
        int base_pixel_y = cross_country_x - cross_country_y;
        *pixel_x = dir == DIR_2_BOTTOM_RIGHT ? base_pixel_x : -base_pixel_x;
        *pixel_y = dir == DIR_6_TOP_LEFT ? base_pixel_y : -base_pixel_y;
    }
}

vec2i figure::tile_pixel_coords() {
    int x;
    int y;
    if (!use_cross_country) {
        // todo? too complicated...
        //        cc_coords_to_pixel_offset((cc_coords.x - 15) % 15, (cc_coords.y) % 15, &x, &y);
        int prg_r = progress_on_tile > 7 ? progress_on_tile - 15 : progress_on_tile;
        int prg_x = 2 * (prg_r);
        int prg_y = (prg_r);
        switch ((8 + direction - city_view_orientation()) % 8) {
        case 0:
            return {prg_x, -prg_y};
            break;
        case 1:
            return {2 * prg_x, 0};
            break;
        case 2:
            return {prg_x, prg_y};
            break;
        case 3:
            return {0, 2 * prg_y};
            break;
        case 4:
            return {-prg_x, prg_y};
            break;
        case 5:
            return {-2 * prg_x, 0};
            break;
        case 6:
            return {-prg_x, -prg_y};
            break;
        case 7:
            return {0, -2 * prg_y};
            break;
        }
    } else {
        cc_coords_to_pixel_offset(cc_coords.x % 15, cc_coords.y % 15, &x, &y);
    }

    return {x, y};
}

void figure::image_set_die_animation(const animation_t &anim) {
    sprite_image_id = image_group(anim.anim_id);
}

void figure::image_set_animation(const animation_t &anim) {
    if (anim.iid > 0) {
        image_set_animation(anim.pack, anim.iid, anim.offset, anim.max_frames, anim.duration);
        return;
    }
    image_desc desc = get_image_desc(anim.anim_id);
    image_set_animation(desc.pack, desc.id, desc.offset, anim.max_frames, anim.duration);
}

void figure::image_set_animation(e_image_id img, int offset, int max_frames, int duration) {
    image_desc desc = get_image_desc(img);
    image_set_animation(desc.pack, desc.id, offset, max_frames, duration);
}

void figure::image_set_animation(int collection, int group, int offset, int max_frames, int duration) {
    anim_base = image_id_from_group(collection, group);
    anim_offset = offset;
    anim_max_frames = max_frames;
    if (duration <= 0)
        duration = 1;
    anim_frame_duration = duration;
}

void figure::figure_image_update(bool refresh_only) {
    // update pixel coords
    //    sprite_pixel_on_tile = {HALF_TILE_WIDTH_PIXELS, HALF_TILE_HEIGHT_PIXELS};
    //    if (use_cross_country) {
    //        int prg_x;
    //        int prg_y;
    //        cc_coords_to_pixel_offset(cc_coords.x % 15, cc_coords.y % 15,
    //                                  &prg_x, &prg_y);
    //        sprite_pixel_on_tile += {prg_x, prg_y};
    //    } else {
    //        int prg_r = progress_on_tile > 7 ? progress_on_tile - 16 : progress_on_tile;
    //        int prg_x = 2 * (prg_r);
    //        int prg_y = (prg_r);
    //        switch ((8 + direction - city_view_orientation()) % 8) {
    //            case 0: sprite_pixel_on_tile += {prg_x, -prg_y}; break;
    //            case 1: sprite_pixel_on_tile += {2 * prg_x, 0}; break;
    //            case 2: sprite_pixel_on_tile += {prg_x, prg_y}; break;
    //            case 3: sprite_pixel_on_tile += {0, 2 * prg_y}; break;
    //            case 4: sprite_pixel_on_tile += {-prg_x, prg_y}; break;
    //            case 5: sprite_pixel_on_tile += {-2 * prg_x, 0}; break;
    //            case 6: sprite_pixel_on_tile += {-prg_x, -prg_y}; break;
    //            case 7: sprite_pixel_on_tile += {0, -2 * prg_y}; break;
    //        }
    //    }

    // null images
    if (anim_base <= 0)
        return;

    // advance animation frame
    if (!refresh_only)
        anim_frame++;
    if (anim_frame >= anim_max_frames * anim_frame_duration)
        anim_frame = 0;

    switch (type) {
    case FIGURE_FISHING_POINT:
        break;

    default:
        dcast()->cart_update_image();
    }

    switch (type) {
    case FIGURE_JAVELIN:
    case FIGURE_ARROW:
    case FIGURE_HUNTER_ARROW: {
        int dir = (16 + direction - 2 * city_view_orientation()) % 16;
        sprite_image_id = anim_base + 16 + dir;
        break;
    }

    case FIGURE_EXPLOSION:
        break;

    case FIGURE_FISHING_POINT: {
        int effective_frame = anim_frame / anim_frame_duration;
        sprite_image_id = anim_base + anim_offset + effective_frame;
        break;
    }

    default:
        // play death animation if it's dying, otherwise always follow the same pattern - offsets are set during action
        // logic
        if (state == FIGURE_STATE_DYING)
            sprite_image_id = anim_base + figure_image_corpse_offset();
        else {
            int effective_frame = anim_frame / anim_frame_duration;
            sprite_image_id = anim_base + anim_offset + figure_image_direction() + 8 * effective_frame;
        }
        break;
    }
}

e_image_id resource_to_sled_image(e_resource res) {
    switch (res) {
    case RESOURCE_STONE: return IMG_SLED_STONE_SMALL;
    case RESOURCE_GRANITE: return IMG_SLED_GRANITE_SMALL;
    case RESOURCE_SANDSTONE: return IMG_SLED_SANDSTONE_SMALL;
    case RESOURCE_LIMESTONE: return IMG_SLED_LIMESTONE_SMALL;
    case RESOURCE_BRICKS: return IMG_SLED_BRICKS;
    }

    return IMG_SLED_EMPTY_SMALL;
}

int cart_image_offset_from_amount(int amount) {
    if (amount <= 100) {
        return 0;
    } 
    
    if (amount <= 200) {
        return 1;
    } 
    
    if (amount <= 400) {
        return 2;
    }

    return 2;
}

image_desc resource2cartanim(e_resource resource_id) {
    image_desc ret = g_cart_images[resource_id];
    if (ret.pack && ret.id) {
        return ret;
    }

    return g_cart_images[RESOURCE_NONE];
}

void figure::cart_update_image() {
    // determine cart sprite
    switch (resource_id) {
    case RESOURCE_STONE:
    case RESOURCE_LIMESTONE:
    case RESOURCE_GRANITE:
    case RESOURCE_SANDSTONE:
    case RESOURCE_BRICKS:
        if (resource_amount_full > 0) {
            e_image_id image_id = resource_to_sled_image(resource_id);
            cart_image_id = image_group(image_id);
        } else {
            cart_image_id = image_group(IMG_SLED_EMPTY_SMALL);
        }
        break;

    case RESOURCE_BARLEY:
    case RESOURCE_COPPER:
    case RESOURCE_BEER:
        cart_image_id = image_group(resource2cartanim(RESOURCE_NONE));
        if (resource_amount_full > 0) {
            cart_image_id = image_group(resource2cartanim(resource_id));
            int amount_offset = cart_image_offset_from_amount(resource_amount_full);
            cart_image_id += 8 * amount_offset;
        }
        break;

    default:
        cart_image_id = image_group(resource2cartanim(RESOURCE_NONE));
        if (resource_amount_full > 0) {
            int amount_offset = cart_image_offset_from_amount(resource_amount_full);
            cart_image_id += 8 + 24 * (resource_id - 1) + 8 * amount_offset;
        }
    }
    //} else if (resource_amount_full == 100) {
    //    cart_image_id = image_id_from_group(GROUP_FIGURE_CARTPUSHER_CART_MULTIPLE_FOOD) + 8 * resource_id - 8
    //                    + resource_image_offset(resource_id, RESOURCE_IMAGE_FOOD_CART);
    //} else {
    //    cart_image_id = image_id_from_group(GROUP_FIGURE_CARTPUSHER_CART) + 8 * resource_id;
    //    cart_image_id += resource_image_offset(resource_id, RESOURCE_IMAGE_CART);
    //}

    int dir = figure_image_normalize_direction(direction < 8 ? direction : previous_tile_direction);

    if (action_state == FIGURE_ACTION_149_CORPSE) {
        sprite_image_id = image_group(ANIM_CARTPUSHER_DEATH);
    } else {
        sprite_image_id = image_group(ANIM_CARTPUSHER_WALK) + dir + 8 * anim_frame;
    }

    switch (resource_id) {
    case RESOURCE_GRANITE:
    case RESOURCE_STONE:
    case RESOURCE_SANDSTONE:
    case RESOURCE_LIMESTONE:
    case RESOURCE_BRICKS:
        if (cart_image_id) {
            cart_image_id += dir;
            figure_image_set_sled_offset(dir);
        }
        break;

    default:
        if (cart_image_id) {
            cart_image_id += dir;
            figure_image_set_cart_offset(dir);
        }
    }
}
int figure::figure_image_corpse_offset() {
    int type_offset = 96;
    switch (type) {
    case FIGURE_BIRDS:
        type_offset = 104;
        break;
    case FIGURE_INDIGENOUS_NATIVE:
        type_offset = 441;
        break;
    case FIGURE_TOWER_SENTRY:
        type_offset = 136;
        break;
    case FIGURE_JAVELIN:
    case FIGURE_FCHARIOTEER:
        type_offset = 144;
        break;
    case FIGURE_ENEMY44_SWORD:
    case FIGURE_ENEMY45_SWORD:
    case FIGURE_ENEMY50_SWORD:
        type_offset = 593;
        break;
    case FIGURE_ENEMY47_ELEPHANT:
        type_offset = 705;
        break;
    case FIGURE_ENEMY46_CAMEL:
    case FIGURE_ENEMY48_CHARIOT:
    case FIGURE_ENEMY52_MOUNTED_ARCHER:
    case FIGURE_ENEMY53_AXE:
        type_offset = 745;
        break;
    case FIGURE_ENEMY51_SPEAR:
        type_offset = 641;
        break;
    case FIGURE_ENEMY49_FAST_SWORD:
        formation* m = formation_get(formation_id);
        if (m->enemy_type == ENEMY_0_BARBARIAN)
            type_offset = 441;
        else if (m->enemy_type == ENEMY_1_NUMIDIAN)
            type_offset = 641;
        else if (m->enemy_type == ENEMY_4_GOTH)
            type_offset = 593;
        break;
    }
    return CORPSE_IMAGE_OFFSETS[wait_ticks / 2] + type_offset;
}
void figure::figure_image_set_sled_offset(int direction) {
    cart_offset = SLED_OFFSETS[direction];
}

void figure::figure_image_set_cart_offset(int direction) {
    cart_offset = CART_OFFSETS[direction];
}

int figure::figure_image_missile_launcher_offset() {
    return MISSILE_LAUNCHER_OFFSETS[attack_image_offset / 2];
}
int figure::figure_image_direction() {
    int dir = direction - city_view_orientation();
    if (dir < 0) {
        dir += 8;
    }
    return dir;
}

int figure_image_normalize_direction(int direction) {
    int normalized_direction = direction - city_view_orientation();
    if (normalized_direction < 0)
        normalized_direction += 8;

    return normalized_direction;
}
