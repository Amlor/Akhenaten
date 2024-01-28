#pragma once

#include <cstdint>

enum e_image_id {
#define REGISTER_IMG(a, b) a = b,
#include "graphics/indexes.h"
    IMG_SIZE
#undef REGISTER_IMG
};

struct image_desc {
    int pack = 0;
    int id = 0;
    int offset = 0;
};

image_desc get_image_desc(e_image_id t);
void set_image_desc(int type, int pack, int id, int offset);