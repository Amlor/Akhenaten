#pragma once

#include "graphics/elements/menu.h"
#include "graphics/image_desc.h"

#define SIDEBAR_COLLAPSED_WIDTH 42
#define SIDEBAR_MAIN_SECTION_HEIGHT 450

#define MINIMAP_WIDTH 73
#define MINIMAP_HEIGHT 111

int sidebar_common_get_x_offset_expanded(void);

int sidebar_common_get_x_offset_collapsed(void);

int sidebar_common_get_height(void);

void sidebar_common_draw_relief(int x_offset, int y_offset, e_image_id img, bool is_collapsed);

