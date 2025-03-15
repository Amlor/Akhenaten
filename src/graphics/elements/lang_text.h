#pragma once

#include "graphics/color.h"
#include "core/vec2i.h"
#include "graphics/font.h"
#include "core/string.h"
#include "ui.h"

pcstr lang_text_from_key(pcstr key);
textid loc_text_from_key(pcstr key);

int lang_text_get_width(int group, int number, e_font font);
int lang_text_get_width(const char* str, e_font font);

int lang_text_draw(int group, int number, int x_offset, int y_offset, e_font font, int box_width = 0);
inline int lang_text_draw(int group, int number, vec2i offset, e_font font, int box_width = 0) { return lang_text_draw(group, number, offset.x, offset.y, font, box_width); }
int lang_text_draw(pcstr str, vec2i pos, e_font font, int box_width = 0);
int lang_text_draw_colored(int group, int number, int x_offset, int y_offset, e_font font, color color);
int lang_text_draw_colored(pcstr tx, int x_offset, int y_offset, e_font font, color color);

int lang_text_draw_left(int group, int number, int x_offset, int y_offset, e_font font);
int lang_text_draw_left_colored(int group, int number, int x_offset, int y_offset, e_font font, color color);

void lang_text_draw_centered(int group, int number, int x_offset, int y_offset, int box_width, e_font font);
void lang_text_draw_centered_colored(int group, int number, int x_offset, int y_offset, int box_width, e_font font, color color);

int lang_text_draw_amount(int group, int number, int amount, int x_offset, int y_offset, e_font font, const char* postfix = " ");
inline int lang_text_draw_amount(int group, int number, int amount, vec2i offset, e_font font, const char *postfix = " ") {
    return lang_text_draw_amount(group, number, amount, offset.x, offset.y, font, postfix);
}

int lang_text_draw_year(int year, int x_offset, int y_offset, e_font font);
int lang_text_draw_multiline(int group, int number, vec2i offset, int box_width, e_font font);