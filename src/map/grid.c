#include "grid.h"

#include "map/data.h"

#include <string.h>

struct map_data_t map_data;

static const int DIRECTION_DELTA_C3[] = {-OFFSET_C3(0,1), OFFSET_C3(1,-1), 1, OFFSET_C3(1,1), OFFSET_C3(0,1), OFFSET_C3(-1,1), -1, -OFFSET_C3(1,1)};
static const int DIRECTION_DELTA_PH[] = {-OFFSET_PH(0,1), OFFSET_PH(1,-1), 1, OFFSET_PH(1,1), OFFSET_PH(0,1), OFFSET_PH(-1,1), -1, -OFFSET_PH(1,1)};

static const int ADJACENT_OFFSETS_C3[][21] = {
    {0},
    {OFFSET_C3(0,-1), OFFSET_C3(1,0), OFFSET_C3(0,1), OFFSET_C3(-1,0), 0},
    {OFFSET_C3(0,-1), OFFSET_C3(1,-1), OFFSET_C3(2,0), OFFSET_C3(2,1), OFFSET_C3(1,2), OFFSET_C3(0,2), OFFSET_C3(-1,1), OFFSET_C3(-1,0), 0},
    {
        OFFSET_C3(0,-1), OFFSET_C3(1,-1), OFFSET_C3(2,-1),
        OFFSET_C3(3,0), OFFSET_C3(3,1), OFFSET_C3(3,2),
        OFFSET_C3(2,3), OFFSET_C3(1,3), OFFSET_C3(0,3),
        OFFSET_C3(-1,2), OFFSET_C3(-1,1), OFFSET_C3(-1,0), 0
    },
    {
        OFFSET_C3(0,-1), OFFSET_C3(1,-1), OFFSET_C3(2,-1), OFFSET_C3(3,-1),
        OFFSET_C3(4,0), OFFSET_C3(4,1), OFFSET_C3(4,2), OFFSET_C3(4,3),
        OFFSET_C3(3,4), OFFSET_C3(2,4), OFFSET_C3(1,4), OFFSET_C3(0,4),
        OFFSET_C3(-1,3), OFFSET_C3(-1,2), OFFSET_C3(-1,1), OFFSET_C3(-1,0), 0
    },
    {
        OFFSET_C3(0,-1), OFFSET_C3(1,-1), OFFSET_C3(2,-1), OFFSET_C3(3,-1), OFFSET_C3(4,-1),
        OFFSET_C3(5,0), OFFSET_C3(5,1), OFFSET_C3(5,2), OFFSET_C3(5,3), OFFSET_C3(5,4),
        OFFSET_C3(4,5), OFFSET_C3(3,5), OFFSET_C3(2,5), OFFSET_C3(1,5), OFFSET_C3(0,5),
        OFFSET_C3(-1,4), OFFSET_C3(-1,3), OFFSET_C3(-1,2), OFFSET_C3(-1,1), OFFSET_C3(-1,0), 0
    },
};
static const int ADJACENT_OFFSETS_PH[][21] = {
    {0},
    {OFFSET_PH(0,-1), OFFSET_PH(1,0), OFFSET_PH(0,1), OFFSET_PH(-1,0), 0},
    {OFFSET_PH(0,-1), OFFSET_PH(1,-1), OFFSET_PH(2,0), OFFSET_PH(2,1), OFFSET_PH(1,2), OFFSET_PH(0,2), OFFSET_PH(-1,1), OFFSET_PH(-1,0), 0},
    {
        OFFSET_PH(0,-1), OFFSET_PH(1,-1), OFFSET_PH(2,-1),
        OFFSET_PH(3,0), OFFSET_PH(3,1), OFFSET_PH(3,2),
        OFFSET_PH(2,3), OFFSET_PH(1,3), OFFSET_PH(0,3),
        OFFSET_PH(-1,2), OFFSET_PH(-1,1), OFFSET_PH(-1,0), 0
    },
    {
        OFFSET_PH(0,-1), OFFSET_PH(1,-1), OFFSET_PH(2,-1), OFFSET_PH(3,-1),
        OFFSET_PH(4,0), OFFSET_PH(4,1), OFFSET_PH(4,2), OFFSET_PH(4,3),
        OFFSET_PH(3,4), OFFSET_PH(2,4), OFFSET_PH(1,4), OFFSET_PH(0,4),
        OFFSET_PH(-1,3), OFFSET_PH(-1,2), OFFSET_PH(-1,1), OFFSET_PH(-1,0), 0
    },
    {
        OFFSET_PH(0,-1), OFFSET_PH(1,-1), OFFSET_PH(2,-1), OFFSET_PH(3,-1), OFFSET_PH(4,-1),
        OFFSET_PH(5,0), OFFSET_PH(5,1), OFFSET_PH(5,2), OFFSET_PH(5,3), OFFSET_PH(5,4),
        OFFSET_PH(4,5), OFFSET_PH(3,5), OFFSET_PH(2,5), OFFSET_PH(1,5), OFFSET_PH(0,5),
        OFFSET_PH(-1,4), OFFSET_PH(-1,3), OFFSET_PH(-1,2), OFFSET_PH(-1,1), OFFSET_PH(-1,0), 0
    },
};


#include <stdlib.h>
#include "core/game_environment.h"
#include "assert.h"

void map_grid_init(grid_xx *grid)
{
    grid->size_field = gr_sizes[grid->datatype];
    grid->size_total = grid->size_field * grid_total_size[GAME_ENV];
    grid->items_xx = malloc(grid->size_total);
    grid->initialized = 1;
}
int map_grid_get(grid_xx *grid, uint32_t at)
{
    if (!grid->initialized)
        map_grid_init(grid);

    if (at >= grid_total_size[GAME_ENV]) {
        return 0;
    }
//    assert(at < grid_total_size[GAME_ENV]);
    switch (grid->datatype) {
        case FS_UINT8:
            return (int)((uint8_t*)grid->items_xx)[at];
        case FS_INT8:
            return (int)((int8_t*)grid->items_xx)[at];
        case FS_UINT16:
            return (int)((uint16_t*)grid->items_xx)[at];
        case FS_INT16:
            return (int)((int16_t*)grid->items_xx)[at];
    }
}
void map_grid_set(grid_xx *grid, uint32_t at, int value)
{
    if (!grid->initialized)
        map_grid_init(grid);
    if (at >= grid_total_size[GAME_ENV]) {
        return;
    }
//    assert(at < grid_total_size[GAME_ENV]);
    switch (grid->datatype) {
        case FS_UINT8:
            ((uint8_t*)grid->items_xx)[at] = (uint8_t)value;
            break;
        case FS_INT8:
            ((int8_t*)grid->items_xx)[at] = (int8_t)value;
            break;
        case FS_UINT16:
            ((uint16_t*)grid->items_xx)[at] = (uint16_t)value;
            break;
        case FS_INT16:
            ((int16_t*)grid->items_xx)[at] = (int16_t)value;
            break;
    }
}
void map_grid_fill(grid_xx *grid, int value)
{
    if (!grid->initialized)
        map_grid_init(grid);
    switch (grid->datatype) {
        case FS_UINT8:
            memset(grid->items_xx, (uint8_t)value, grid->size_total);
            break;
        case FS_INT8:
            memset(grid->items_xx, (int8_t)value, grid->size_total);
            break;
        case FS_UINT16:
            memset(grid->items_xx, (uint16_t)value, grid->size_total);
            break;
        case FS_INT16:
            memset(grid->items_xx, (int16_t)value, grid->size_total);
            break;
    }
}
void map_grid_clear(grid_xx *grid)
{
    if (!grid->initialized)
        map_grid_init(grid);
    memset(grid->items_xx, 0, grid->size_total);
}
void map_grid_copy(grid_xx *src, grid_xx *dst)
{
    if (!src->initialized)
        map_grid_init(src);
    if (!dst->initialized)
        map_grid_init(dst);

    assert(src->datatype == dst->datatype);
    assert(src->size_total == dst->size_total);

    memcpy(dst->items_xx, src->items_xx, src->size_total);
}

void map_grid_and(grid_xx *grid, int at, int mask)
{
    if (!grid->initialized)
        map_grid_init(grid);
    int v = map_grid_get(grid, at);
    v &= mask;
    map_grid_set(grid, at, v);
}
void map_grid_or(grid_xx *grid, int at, int mask)
{
    if (!grid->initialized)
        map_grid_init(grid);
    int v = map_grid_get(grid, at);
    v |= mask;
    map_grid_set(grid, at, v);
}
void map_grid_and_all(grid_xx *grid, int mask)
{
    if (!grid->initialized)
        map_grid_init(grid);
    for (int i = 0; i < grid_total_size[GAME_ENV]; i++) {
        switch (grid->datatype) {
            case FS_UINT8:
                ((uint8_t*)grid->items_xx)[i] &= (uint8_t)mask;
                break;
            case FS_INT8:
                ((int8_t*)grid->items_xx)[i] &= (int8_t)mask;
                break;
            case FS_UINT16:
                ((uint16_t*)grid->items_xx)[i] &= (uint16_t)mask;
                break;
            case FS_INT16:
                ((int16_t*)grid->items_xx)[i] &= (int16_t)mask;
                break;
        }
    }
}

void map_grid_save_state(grid_xx *grid, buffer *buf)
{
    if (!grid->initialized)
        map_grid_init(grid);
    for (int i = 0; i < grid_total_size[GAME_ENV]; i++) {
        switch (grid->datatype) {
            case FS_UINT8:
                buffer_write_raw(buf, grid->items_xx, grid_total_size[GAME_ENV]);
                break;
            case FS_INT8:
                buffer_write_raw(buf, grid->items_xx, grid_total_size[GAME_ENV]);
                break;
            case FS_UINT16:
                for (int i = 0; i < grid_total_size[GAME_ENV]; i++)
                    buffer_write_u16(buf, ((uint16_t*)grid->items_xx)[i]);
                break;
            case FS_INT16:
                for (int i = 0; i < grid_total_size[GAME_ENV]; i++)
                    buffer_write_i16(buf, ((int16_t*)grid->items_xx)[i]);
                break;
        }
    }
}
void map_grid_load_state(grid_xx *grid, buffer *buf)
{
    if (!grid->initialized)
        map_grid_init(grid);
    switch (grid->datatype) {
        case FS_UINT8:
            buffer_read_raw(buf, grid->items_xx, grid_total_size[GAME_ENV]);
            break;
        case FS_INT8:
            buffer_read_raw(buf, grid->items_xx, grid_total_size[GAME_ENV]);
            break;
        case FS_UINT16: {
            uint16_t *dr_data = (uint16_t *) grid->items_xx;
            for (int i = 0; i < grid_total_size[GAME_ENV]; i++)
                dr_data[i] = buffer_read_u16(buf);
            break;
        }
        case FS_INT16: {
            int16_t *dr_data = (int16_t *) grid->items_xx;
            for (int i = 0; i < grid_total_size[GAME_ENV]; i++)
                dr_data[i] = buffer_read_i16(buf);
            break;
        }
    }
    return;
}

//void grid_array_init_u8(grid_u8_x *grid)
//{
//    grid->items = malloc(sizeof(uint8_t) * grid_total_size[GAME_ENV]);
//    grid->initialized = 1;
//}
//void grid_array_init_i8(grid_i8_x *grid)
//{
//    grid->items = malloc(sizeof(int8_t) * grid_total_size[GAME_ENV]);
//    grid->initialized = 1;
//}
//void grid_array_init_u16(grid_u16_x *grid)
//{
//    grid->items = malloc(sizeof(uint16_t) * grid_total_size[GAME_ENV]);
//    grid->initialized = 1;
//}
//void grid_array_init_i16(grid_i16_x *grid)
//{
//    grid->items = malloc(sizeof(int16_t) * grid_total_size[GAME_ENV]);
//    grid->initialized = 1;
//}
//
//grid_u8_x *safe_u8(grid_u8_x *gr)
//{
//    if (!gr->initialized)
//        grid_array_init_u8(gr);
//    return gr;
//}
//grid_i8_x *safe_i8(grid_i8_x *gr)
//{
//    if (!gr->initialized)
//        grid_array_init_i8(gr);
//    return gr;
//}
//grid_u16_x *safe_u16(grid_u16_x *gr)
//{
//    if (!gr->initialized)
//        grid_array_init_u16(gr);
//    return gr;
//}
//grid_i16_x *safe_i16(grid_i16_x *gr)
//{
//    if (!gr->initialized)
//        grid_array_init_i16(gr);
//    return gr;
//}

void map_data_init(int width, int height, int start_offset, int border_size)
{
    map_data.width = width;
    map_data.height = height;
    map_data.start_offset = start_offset;
    map_data.border_size = border_size;
}

int map_grid_is_valid_offset(int grid_offset)
{
    return grid_offset >= 0 && grid_offset < grid_total_size[GAME_ENV];
}
int map_grid_offset(int x, int y)
{
    return map_data.start_offset + x + y * grid_size[GAME_ENV];
}
int map_grid_offset_to_x(int grid_offset)
{
    return (grid_offset - map_data.start_offset) % grid_size[GAME_ENV];
}
int map_grid_offset_to_y(int grid_offset)
{
    return (grid_offset - map_data.start_offset) / grid_size[GAME_ENV];
}

int map_grid_delta(int x, int y)
{
    return y * grid_size[GAME_ENV] + x;
}
int map_grid_add_delta(int grid_offset, int x, int y)
{
    int raw_x = grid_offset % grid_size[GAME_ENV];
    int raw_y = grid_offset / grid_size[GAME_ENV];
    if (raw_x + x < 0 || raw_x + x >= grid_size[GAME_ENV] ||
        raw_y + y < 0 || raw_y + y >= grid_size[GAME_ENV]) {
        return -1;
    }
    return grid_offset + map_grid_delta(x, y);
}
int map_grid_direction_delta(int direction)
{
    if (direction >= 0 && direction < 8) {
        switch (GAME_ENV) {
            case ENGINE_ENV_C3:
                return DIRECTION_DELTA_C3[direction];
            case ENGINE_ENV_PHARAOH:
                return DIRECTION_DELTA_PH[direction];
        }
    } else {
        return 0;
    }
}
void map_grid_size(int *width, int *height)
{
    *width = map_data.width;
    *height = map_data.height;
}
int map_grid_width(void)
{
    return map_data.width;
}
int map_grid_height(void)
{
    return map_data.height;
}
void map_grid_bound(int *x, int *y)
{
    if (*x < 0) {
        *x = 0;
    }
    if (*y < 0) {
        *y = 0;
    }
    if (*x >= map_data.width) {
        *x = map_data.width - 1;
    }
    if (*y >= map_data.height) {
        *y = map_data.height - 1;
    }
}
void map_grid_bound_area(int *x_min, int *y_min, int *x_max, int *y_max)
{
    if (*x_min < 0) {
        *x_min = 0;
    }
    if (*y_min < 0) {
        *y_min = 0;
    }
    if (*x_max >= map_data.width) {
        *x_max = map_data.width - 1;
    }
    if (*y_max >= map_data.height) {
        *y_max = map_data.height - 1;
    }
}
void map_grid_get_area(int x, int y, int size, int radius, int *x_min, int *y_min, int *x_max, int *y_max)
{
    *x_min = x - radius;
    *y_min = y - radius;
    *x_max = x + size + radius - 1;
    *y_max = y + size + radius - 1;
    map_grid_bound_area(x_min, y_min, x_max, y_max);
}
void map_grid_start_end_to_area(int x_start, int y_start, int x_end, int y_end, int *x_min, int *y_min, int *x_max, int *y_max)
{
    if (x_start < x_end) {
        *x_min = x_start;
        *x_max = x_end;
    } else {
        *x_min = x_end;
        *x_max = x_start;
    }
    if (y_start < y_end) {
        *y_min = y_start;
        *y_max = y_end;
    } else {
        *y_min = y_end;
        *y_max = y_start;
    }
    map_grid_bound_area(x_min, y_min, x_max, y_max);
}
int map_grid_is_inside(int x, int y, int size)
{
    return x >= 0 && x + size <= map_data.width && y >= 0 && y + size <= map_data.height;
}

const int *map_grid_adjacent_offsets(int size)
{
    switch (GAME_ENV) {
        case ENGINE_ENV_C3:
            return ADJACENT_OFFSETS_C3[size];
        case ENGINE_ENV_PHARAOH:
            return ADJACENT_OFFSETS_PH[size];
    }
}

//void map_grid_init_i8(int8_t *grid, int8_t value)
//{
//    memset(grid, value, grid_total_size[GAME_ENV] * sizeof(int8_t));
//}
//
//void map_grid_clear_i8(int8_t *grid)
//{
//    memset(grid, 0, grid_total_size[GAME_ENV] * sizeof(int8_t));
//}
//void map_grid_clear_u8(uint8_t *grid)
//{
//    memset(grid, 0, grid_total_size[GAME_ENV] * sizeof(uint8_t));
//}
//void map_grid_clear_u16(uint16_t *grid)
//{
//    memset(grid, 0, grid_total_size[GAME_ENV] * sizeof(uint16_t));
//}
//void map_grid_clear_i16(int16_t *grid)
//{
//    memset(grid, 0, grid_total_size[GAME_ENV] * sizeof(int16_t));
//}
//
//void map_grid_and_u8(uint8_t *grid, uint8_t mask)
//{
//    for (int i = 0; i < grid_total_size[GAME_ENV]; i++) {
//        grid[i] &= mask;
//    }
//}
//void map_grid_and_u16(uint16_t *grid, uint16_t mask)
//{
//    for (int i = 0; i < grid_total_size[GAME_ENV]; i++) {
//        grid[i] &= mask;
//    }
//}
//void map_grid_copy_u8(const uint8_t *src, uint8_t *dst)
//{
//    memcpy(dst, src, grid_total_size[GAME_ENV] * sizeof(uint8_t));
//}
//void map_grid_copy_u16(const uint16_t *src, uint16_t *dst)
//{
//    memcpy(dst, src, grid_total_size[GAME_ENV] * sizeof(uint16_t));
//}
//void map_grid_save_state_u8(const uint8_t *grid, buffer *buf)
//{
//    buffer_write_raw(buf, grid, grid_total_size[GAME_ENV]);
//}
//void map_grid_save_state_i8(const int8_t *grid, buffer *buf)
//{
//    buffer_write_raw(buf, grid, grid_total_size[GAME_ENV]);
//}
//void map_grid_save_state_u16(const uint16_t *grid, buffer *buf)
//{
//    for (int i = 0; i < grid_total_size[GAME_ENV]; i++) {
//        buffer_write_u16(buf, grid[i]);
//    }
//}
//void map_grid_load_state_u8(uint8_t *grid, buffer *buf)
//{
//    buffer_read_raw(buf, grid, grid_total_size[GAME_ENV]);
//}
//void map_grid_load_state_i8(int8_t *grid, buffer *buf)
//{
//    buffer_read_raw(buf, grid, grid_total_size[GAME_ENV]);
//}
//void map_grid_load_state_u16(uint16_t *grid, buffer *buf)
//{
//    for (int i = 0; i < grid_total_size[GAME_ENV]; i++) {
//        grid[i] = buffer_read_u16(buf);
//    }
//}


