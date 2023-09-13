#pragma once

#include <cstdint>

#include "game/settings.h"

int platform_screen_create(const char * title,
                           const char* renderer,
                           bool fullscreen,
                           int display_scale_percentage,
                           display_size screen_size);
void platform_screen_destroy();

int platform_screen_resize(int pixel_width, int pixel_height, int save);
void platform_screen_move(int x, int y);

void platform_screen_set_fullscreen();
void platform_screen_set_windowed();
void platform_screen_set_window_size(int logical_width, int logical_height);
void platform_screen_center_window();

#ifdef _WIN32
void platform_screen_recreate_texture(void);
#endif // _WIN32

int platform_screen_get_scale();

void platform_screen_show_error_message_box(const char* title, const char* message);
