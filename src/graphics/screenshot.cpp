#include "screenshot.h"

#include "city/warning.h"
#include "core/buffer.h"
#include "core/string.h"
#include "core/log.h"
#include "color.h"
#include "config/config.h"
#include "content/vfs.h"
#include "grid/grid.h"
#include "platform/renderer.h"
#include "graphics/screen.h"
#include "graphics/boilerplate.h"
#include "graphics/graphics.h"
#include "graphics/elements/menu.h"
#include "graphics/screen.h"
#include "graphics/window.h"
#include "graphics/view/view.h"
#include "scenario/scenario.h"
#include "widget/minimap.h"
#include "widget/sidebar/common.h"
#include "widget/city/figures_cached_draw.h"
#include "widget/city.h"

#include "png.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <thread>

#define TILE_X_SIZE 60
#define TILE_Y_SIZE 30
#define IMAGE_HEIGHT_CHUNK (TILE_Y_SIZE * 15)
#define IMAGE_BYTES_PER_PIXEL 3
#define MINIMAP_SCALE 2.0f

static struct {
    int width;
    int height;
    int row_size;
    int rows_in_memory;
    int current_y;
    int final_y;
    int alpha_channel;
    uint8_t *pixels;
    FILE *fp;
    png_structp png_ptr;
    png_infop info_ptr;
} screenshot;

static void image_free(void)
{
    screenshot.width = 0;
    screenshot.height = 0;
    screenshot.row_size = 0;
    screenshot.rows_in_memory = 0;
    free(screenshot.pixels);
    screenshot.pixels = 0;
    if (screenshot.fp) {
        vfs::file_close(screenshot.fp);
        screenshot.fp = 0;
    }
    png_destroy_write_struct(&screenshot.png_ptr, &screenshot.info_ptr);
}

static int image_create(vec2i size, int has_alpha_channel, int rows_in_memory) {
    image_free();
    if (!size.x || !size.y || !rows_in_memory) {
        return 0;
    }
    screenshot.png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
    if (!screenshot.png_ptr) {
        return 0;
    }
    screenshot.info_ptr = png_create_info_struct(screenshot.png_ptr);
    if (!screenshot.info_ptr) {
        image_free();
        return 0;
    }
    png_set_compression_level(screenshot.png_ptr, 3);
    screenshot.alpha_channel = has_alpha_channel;
    screenshot.width = size.x;
    screenshot.height = size.y;
    screenshot.row_size = size.x * IMAGE_BYTES_PER_PIXEL;
    if (screenshot.alpha_channel) {
        screenshot.row_size += size.x;
    }
    screenshot.rows_in_memory = rows_in_memory;
    screenshot.pixels = (uint8_t *) malloc(screenshot.row_size);
    if (!screenshot.pixels) {
        image_free();
        return 0;
    }
    memset(screenshot.pixels, 0, screenshot.row_size);
    return 1;
}

static const char *generate_filename(screenshot_type type) {
    static bstring256 filename;
    time_t curtime = time(NULL);
    struct tm *loctime = localtime(&curtime);
    switch (type) {
    case SCREENSHOT_FULL_CITY:
        strftime(filename, bstring256::capacity, "full_city_%Y_%m_%d_%H_%M_%S.png", loctime);
        break;

    case SCREENSHOT_MINIMAP:
        strftime(filename, bstring256::capacity, "minimap_%Y_%m_%d_%H_%M_%S.png", loctime);
        break;

    case SCREENSHOT_DISPLAY:
    default:
        strftime(filename, bstring256::capacity, "city_%Y_%m_%d_%H_%M_%S.png", loctime);
        break;
    }    
    return filename;
}

static int image_begin_io(const char *filename) {
    vfs::path fs_file = vfs::content_path(filename);

    FILE *fp = vfs::file_open(fs_file, "wb");
    if (!fp) {
        return 0;
    }
    screenshot.fp = fp;
    png_init_io(screenshot.png_ptr, fp);
    return 1;
}

static int image_write_header(void) {
    if (setjmp(png_jmpbuf(screenshot.png_ptr))) {
        return 0;
    }
    int color_type = screenshot.alpha_channel ? PNG_COLOR_TYPE_RGBA : PNG_COLOR_TYPE_RGB;
    png_set_IHDR(screenshot.png_ptr, screenshot.info_ptr, screenshot.width, screenshot.height, 8, color_type,
                 PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_write_info(screenshot.png_ptr, screenshot.info_ptr);
    return 1;
}

static int image_set_loop_height_limits(int min, int max) {
    screenshot.current_y = min;
    screenshot.final_y = max;
    return screenshot.current_y;
}

static int image_request_rows(void) {
    if (screenshot.current_y < screenshot.final_y) {
        screenshot.current_y += screenshot.rows_in_memory;
        return screenshot.rows_in_memory;
    }
    return 0;
}

static int image_write_rows(const color *canvas, int canvas_width) {
    if (setjmp(png_jmpbuf(screenshot.png_ptr))) {
        return 0;
    }
    for (int y = 0; y < screenshot.rows_in_memory; ++y) {
        uint8_t *pixel = screenshot.pixels;
        if (screenshot.alpha_channel) {
            for (int x = 0; x < screenshot.width; x++) {
                color input = canvas[y * canvas_width + x];
                *(pixel + 0) = (uint8_t) COLOR_COMPONENT(input, COLOR_BITSHIFT_RED);
                *(pixel + 1) = (uint8_t) COLOR_COMPONENT(input, COLOR_BITSHIFT_GREEN);
                *(pixel + 2) = (uint8_t) COLOR_COMPONENT(input, COLOR_BITSHIFT_BLUE);
                *(pixel + 3) = (uint8_t) COLOR_COMPONENT(input, COLOR_BITSHIFT_ALPHA);
                pixel += IMAGE_BYTES_PER_PIXEL + 1;
            }
        } else {
            for (int x = 0; x < screenshot.width; x++) {
                color input = canvas[y * canvas_width + x];
                *(pixel + 0) = (uint8_t) COLOR_COMPONENT(input, COLOR_BITSHIFT_RED);
                *(pixel + 1) = (uint8_t) COLOR_COMPONENT(input, COLOR_BITSHIFT_GREEN);
                *(pixel + 2) = (uint8_t) COLOR_COMPONENT(input, COLOR_BITSHIFT_BLUE);
                pixel += IMAGE_BYTES_PER_PIXEL;
            }
        }
        png_write_row(screenshot.png_ptr, screenshot.pixels);
    }
    return 1;
}

static int image_write_canvas() {
    const color *canvas;
    color *pixels = 0;
    pixels = (color *)malloc(sizeof(color) * screenshot.width * screenshot.height);
    if (!graphics_renderer()->save_screen_buffer(pixels, 0, 0, screen_width(), screen_height(), screen_width())) {
        free(pixels);
        return 0;
    }
    canvas = pixels;
    int current_height = image_set_loop_height_limits(0, screenshot.height);
    int size;
    while ((size = image_request_rows()) != 0) {
        if (!image_write_rows(canvas + current_height * screenshot.width, screenshot.width)) {
            free(pixels);
            return 0;
        }
        current_height += size;
    }
    free(pixels);
    return 1;
}

static void image_finish(void) {
    png_write_end(screenshot.png_ptr, screenshot.info_ptr);
}

static void show_saved_notice(const char *filename) {
    bstring256 notice_text( "Screenshot saved to ", filename);

    city_warning_show_custom(notice_text);
}

static void create_window_screenshot(void) {
    if (!image_create(screen_size(), 0, 1)) {
        logs::error("Unable to create memory for screenshot");
        return;
    }

    const char *filename = generate_filename(SCREENSHOT_DISPLAY);
    if (!image_begin_io(filename) || !image_write_header()) {
        logs::error("Unable to write screenshot to: %s", filename);
        image_free();
        return;
    }

    if (!image_write_canvas()) {
        logs::error("Error writing image");
        image_free();
        return;
    }

    image_finish();
    logs::info("Saved screenshot: %s", filename);
    show_saved_notice(filename);
    image_free();
}

static void create_full_city_screenshot() {
    if (!window_is(WINDOW_CITY) && !window_is(WINDOW_CITY_MILITARY)) {
        return;
    }
    vec2i original_camera_pixels = camera_get_position();

    vec2i min_pos, max_pos;
    city_view_get_camera_scrollable_pixel_limits(city_view_data_unsafe(), min_pos, max_pos);

    vec2i view_pos, view_size;
    city_view_get_viewport(view_pos, view_size);

    max_pos += view_size;

    vec2i city_canvas_pixels = max_pos - min_pos;

    int canvas_width = city_canvas_pixels.x / 5;
    int canvas_height = city_canvas_pixels.y / 5;

    if (!image_create(city_canvas_pixels, 0, canvas_height)) {
        logs::error("Unable to set memory for full city screenshot", 0, 0);
        return;
    }

    const char *filename = generate_filename(SCREENSHOT_FULL_CITY);
    if (!image_begin_io(filename) || !image_write_header()) {
        logs::error("Unable to write screenshot to:", filename, 0);
        image_free();
        return;
    }

    color *canvas = (color*)malloc(sizeof(color) * city_canvas_pixels.x * canvas_height);
    if (!canvas) {
        image_free();
        return;
    }
    memset(canvas, 0, sizeof(color) * city_canvas_pixels.x * canvas_height);

    int old_scale = zoom_get_scale() * 100;

    int error = 0;
    int base_height = image_set_loop_height_limits(min_pos.y, max_pos.y);
    int size;
    zoom_set_scale(100);
    graphics_set_clip_rectangle(0, TOP_MENU_HEIGHT, canvas_width, canvas_height);
    
    vec2i viewport_offset, viewport_size;
    city_view_get_viewport(viewport_offset, viewport_size);
    city_view_set_viewport(canvas_width + (city_view_is_sidebar_collapsed() ? SIDEBAR_COLLAPSED_WIDTH : SIDEBAR_EXPANDED_WIDTH), canvas_height + TOP_MENU_HEIGHT);
    int current_height = base_height;

    int yy = 0;
    while ((size = image_request_rows()) != 0) {
        
        int y_offset = (current_height + canvas_height > max_pos.y) ? canvas_height - (max_pos.y - current_height) - TILE_Y_SIZE : 0;

        std::vector<std::thread> threads;

        int i = 0;
        for (int width = 0; width < city_canvas_pixels.x; width += canvas_width) {
            int image_section_width = canvas_width;
            int x_offset = 0;
            if (canvas_width + width > city_canvas_pixels.x) {
                image_section_width = city_canvas_pixels.x - width;
                x_offset = canvas_width - image_section_width - TILE_X_SIZE * 2;
            }

            //threads.push_back(std::thread([] (vec2i min_pos, int width, int canvas_width, int current_height, color *canvas, int x_offset, int y_offset, int image_section_width, int canvas_height, vec2i city_canvas_pixels, int color) {
                SDL_Surface *surface = SDL_CreateRGBSurface(SDL_SWSURFACE, canvas_width, canvas_height, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
                SDL_Renderer *renderer = SDL_CreateSoftwareRenderer(surface);
                
                //SDL_Rect rect{0, 0, canvas_width, canvas_height};
                //SDL_FillRect(surface, &rect, color );
                figure_draw_cache_data_t local_figure_data;
                view_data_t local_view_data = city_view_data_unsafe();
                view_context local_context;
                local_context.figure_cache = &local_figure_data;
                local_context.view_data = &city_view_data_unsafe(); // &local_view_data;

                camera_go_to_pixel(local_context, vec2i{min_pos.x + width, current_height}, false);
                tile2i dummy_tile(0, 0);
                widget_city_draw_without_overlay(0, 0, dummy_tile, local_context);
                graphics_renderer()->save_screen_buffer(&canvas[width], x_offset, TOP_MENU_HEIGHT + y_offset, image_section_width, canvas_height - y_offset, city_canvas_pixels.x);
                //SDL_Rect rect2 = {x_offset, y_offset, canvas_width, canvas_height};
                //bool ok = SDL_RenderReadPixels(renderer, &rect2, SDL_PIXELFORMAT_ARGB8888, &canvas[width], city_canvas_pixels.x * sizeof(color)) == 0;
                SDL_DestroyRenderer(renderer);
                SDL_FreeSurface(surface);
            //}, min_pos, width, canvas_width, current_height, canvas, x_offset, y_offset, image_section_width, canvas_height, city_canvas_pixels, ((yy + i) % 2) ? 0xff00ff00 : 0xff0000ff));
            i++;
        }

        for (std::thread& t : threads) {
           // t.join();
        }

        if (!image_write_rows(canvas, city_canvas_pixels.x)) {
            logs::error("Error writing image", 0, 0);
            error = 1;
            break;
        }
        yy++;
        current_height += canvas_height;
    }

    city_view_set_viewport(viewport_size.x + (city_view_is_sidebar_collapsed() ? SIDEBAR_COLLAPSED_WIDTH : SIDEBAR_EXPANDED_WIDTH), viewport_size.y + TOP_MENU_HEIGHT);
    zoom_set_scale(old_scale);

    graphics_reset_clip_rectangle();
    camera_go_to_pixel(view_context_main(), original_camera_pixels, true);
    
    if (!error) {
        image_finish();
        logs::info("Saved full city screenshot: %s", filename);
        show_saved_notice(filename);
    }
    
    image_free();
    window_invalidate();
}

static void create_minimap_screenshot() {
    if (!window_is(WINDOW_CITY) && !window_is(WINDOW_CITY_MILITARY)) {
        return;
    }

    int width_pixels = map_grid_width() * (int) MINIMAP_SCALE * 2;
    int height_pixels = map_grid_height() * (int) MINIMAP_SCALE * 2;

    if (!image_create({width_pixels, height_pixels}, 1, height_pixels)) {
        logs::error("Unable to set memory for minimap screenshot", 0, 0);
        return;
    }
    const char *filename = generate_filename(SCREENSHOT_MINIMAP);
    if (!image_begin_io(filename) || !image_write_header()) {
        logs::error("Unable to write screenshot to:", filename, 0);
        image_free();
        return;
    }

    color *canvas = (color*)malloc(sizeof(color) * width_pixels * height_pixels);
    if (!canvas) {
        image_free();
        return;
    }
    memset(canvas, 0, sizeof(color) * width_pixels * height_pixels);
    widget_minimap_draw({0, 0}, width_pixels, height_pixels, 1);
    graphics_clear_screen();
    graphics_renderer()->draw_custom_texture(CUSTOM_IMAGE_MINIMAP, 0, 0, 1 / MINIMAP_SCALE);
    graphics_renderer()->save_screen_buffer(canvas, 0, 0, width_pixels, height_pixels, width_pixels);
    if (image_write_rows(canvas, width_pixels)) {
        image_finish();
        logs::info("Saved city map screenshot:", filename, 0);
        show_saved_notice(filename);
    }
    image_free();
    window_invalidate();
}

void graphics_save_screenshot(screenshot_type type)
{
    switch (type) {
    case SCREENSHOT_FULL_CITY:
        create_full_city_screenshot();
        return;
    case SCREENSHOT_MINIMAP:
        create_minimap_screenshot();
        return;
    case SCREENSHOT_DISPLAY:
    default:
        create_window_screenshot();
        return;
    }
}