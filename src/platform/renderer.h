#pragma once

#include "graphics/color.h"

#include "content/file_formats.h"
#include "content/vfs.h"
#include "core/string.h"
#include "core/vec2i.h"
#include "graphics/image.h"

#include <string>
#include <vector>

struct painter;

enum e_custome_image_type {
    CUSTOM_IMAGE_NONE = 0,
    CUSTOM_IMAGE_EXTERNAL = 1,
    CUSTOM_IMAGE_MINIMAP = 2,
    CUSTOM_IMAGE_VIDEO = 3,
    CUSTOM_IMAGE_EMPIRE_MAP = 4,
    CUSTOM_IMAGE_RED_FOOTPRINT = 5,
    CUSTOM_IMAGE_GREEN_FOOTPRINT = 6,
    CUSTOM_IMAGE_MAX = 7
};

enum { IMAGE_FILTER_NEAREST = 0, IMAGE_FILTER_LINEAR = 1 };

int platform_renderer_init(SDL_Window* window, std::string renderer);
int platform_renderer_create_render_texture(int width, int height);
int platform_renderer_lost_render_texture(void);
void platform_renderer_invalidate_target_textures(void);
void platform_renderer_generate_mouse_cursor_texture(int cursor_id, int size, const color* pixels, int hotspot_x, int hotspot_y);
void platform_renderer_clear();
void platform_renderer_render();
void platform_renderer_pause();
void platform_renderer_resume();
void platform_renderer_destroy();

struct video_mode : public vec2i {
    bstring64 str;
    inline video_mode() : vec2i(0, 0) {}
    inline video_mode(int _w, int _h) : vec2i(_w, _h) {
        char buffer[64] = {0};
        snprintf(buffer, 64, "%u x %u", x, y);
        str = buffer;
    }
    inline bool operator<(const video_mode& o) const {
        return ((int64_t(x) << 32) + y) < ((int64_t(o.x) << 32) + o.y);
    }
};

class graphics_renderer_interface {
public:
    void clear_screen();
    void set_viewport(int x, int y, int width, int height);
    void reset_viewport();
    void set_clip_rectangle(vec2i pos, int width, int height);
    bool inside_clip_rectangle(vec2i pos);
    rect clip_rectangle();
    void reset_clip_rectangle();

    void draw_line(vec2i start, vec2i end, color color);
    void draw_pixel(vec2i pixel, color color);
    void draw_rect(vec2i start, vec2i size, color color);
    void fill_rect(vec2i start, vec2i size, color color);

    void draw_image_part(painter &ctx, const image_t *img, int offset, float x, float y, color color = COLOR_WHITE, float scale = 1.f, bool mirrored = false , bool alpha = false);
    void draw_image(painter &ctx, const image_t* img, float x, float y, color color = COLOR_WHITE, float scale = 1.f, bool mirrored = false, bool alpha = false);
    bool save_screen_buffer(painter &ctx, color *pixels, int x, int y, int width, int height, int row_width);

    void create_custom_texture(int type, int width, int height);
    int has_custom_texture(int type);
    color* get_custom_texture_buffer(int type, int* actual_texture_width);
    void release_custom_texture_buffer(int type);
    void update_custom_texture(int type);
    void update_custom_texture_yuv(int type, const uint8_t* y_data, int y_width, const uint8_t* cb_data, int cb_width, const uint8_t* cr_data, int cr_width);
    void draw_custom_texture(int type, int x, int y, float scale);

    int save_texture_from_screen(int image_id, int x, int y, int width, int height);
    void delete_saved_texture(int image_id);
    void draw_saved_texture_to_screen(int image_id, int x, int y, int width, int height);
    void set_texture_scale_mode(SDL_Texture *texture, float scale_factor);
    unsigned int premult_alpha();

    vec2i get_max_image_size();

    SDL_Texture* create_texture_from_buffer(color* p_data, int width, int height);
    SDL_Texture* create_texture_from_png_buffer(void* p_data, int size, vec2i &txsize);

    bool has_texture_scale_mode();

    bool save_texture_to_file(const char* filename, SDL_Texture* tex, e_file_format file_format = FILE_FORMAT_BMP);
    float scale();
    SDL_Renderer *renderer();
    SDL_Window *window();
};

graphics_renderer_interface* graphics_renderer();
extern graphics_renderer_interface g_render;

void set_render_scale(painter &ctx, float scale);
std::vector<video_mode> get_video_modes();
std::vector<std::string> get_video_drivers(bool log);
