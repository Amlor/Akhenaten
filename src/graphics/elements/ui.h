#pragma once

#include <cstdint>
#include <memory>

#include "core/vec2i.h"
#include "core/svector.h"

#include "input/hotkey.h"
#include "graphics/elements/generic_button.h"
#include "graphics/elements/image_button.h"
#include "graphics/elements/arrow_button.h"
#include "graphics/elements/scrollbar.h"
#include "graphics/elements/lang_text.h"
#include "graphics/elements/button.h"
#include "graphics/elements/panel.h"
#include "graphics/elements/menu.h"
#include "graphics/image_groups.h"

#include "js/js_game.h"
#include <functional>

struct mouse;

enum UiFlags_ {
    UiFlags_None = 0,
    UiFlags_LabelCentered = 1 << 1,
    UiFlags_PanelOuter = 1 << 2,
    UiFlags_PanelInner = 1 << 3,
    UiFlags_LabelMultiline = 1 << 4,
    UiFlags_LabelYCentered = 1 << 5,
    UiFlags_NoBody = 1 << 6,
    UiFlags_Rich = 1 << 7,
    UiFlags_Selected = 1 << 8,
    UiFlags_Readonly = 1 << 9,
};
using UiFlags = int;

namespace ui {

struct img_button_offsets { int data[4] = {0, 1, 2, 3}; };

void begin_frame();
void begin_widget(vec2i offset, bool relative = false);
void end_widget();
bool handle_mouse(const mouse *m);

int label(int group, int number, vec2i pos, e_font font = FONT_NORMAL_BLACK_ON_LIGHT, UiFlags flags = UiFlags_None, int box_width = 0);
int label(pcstr, vec2i pos, e_font font = FONT_NORMAL_BLACK_ON_LIGHT, UiFlags flags = UiFlags_None, int box_width = 0);
int label_amount(int group, int number, int amount, vec2i pos, e_font font = FONT_NORMAL_BLACK_ON_LIGHT, pcstr postfix = "");
int label_percent(int amount, vec2i pos, e_font font = FONT_NORMAL_BLACK_ON_LIGHT);
void eimage(e_image_id img, vec2i pos, int offset = 0);
void eimage(image_desc img, vec2i pos);
void panel(vec2i pos, vec2i size, UiFlags flags);
void icon(vec2i pos, e_resource img);
void icon(vec2i pos, e_advisor advisor);
int button_hover(const mouse *m);
generic_button &button(pcstr label, vec2i pos, vec2i size, e_font font = FONT_NORMAL_BLACK_ON_LIGHT, UiFlags flags = UiFlags_None, std::function<void(int, int)> cb = {});
generic_button &button(const svector<pcstr,4> &labels, vec2i pos, vec2i size, e_font font = FONT_NORMAL_BLACK_ON_LIGHT, UiFlags flags = UiFlags_None, std::function<void(int, int)> cb = {});
generic_button &link(pcstr label, vec2i pos, vec2i size, e_font font = FONT_NORMAL_WHITE_ON_DARK, UiFlags flags = UiFlags_None, std::function<void(int, int)> cb = {});
generic_button &large_button(pcstr label, vec2i pos, vec2i size, e_font font = FONT_NORMAL_BLACK_ON_LIGHT);
generic_button &button(uint32_t id);
image_button &img_button(uint32_t group, uint32_t id, vec2i pos, vec2i size, const img_button_offsets offsets = {}, UiFlags_ flags = UiFlags_None);
image_button &imgok_button(vec2i pos, std::function<void(int, int)> cb);
image_button &imgcancel_button(vec2i pos, std::function<void(int, int)> cb);
image_button &img_button(e_image_id img, vec2i pos, vec2i size, int offset = 0);
arrow_button &arw_button(vec2i pos, bool up, bool tiny = false);
scrollbar_t &scrollbar(scrollbar_t &scrollbar, vec2i pos, int &value, vec2i size = {-1, -1});

pcstr str(int group, int id);
inline pcstr str(std::pair<int, int> r) { return str(r.first, r.second); }

struct emenu_header;
struct eimage_button;

struct element {
    bstring64 id;
    vec2i pos;
    vec2i size;
    bool readonly = false;
    bool enabled = true;

    virtual void draw() {}
    virtual void load(archive);
    virtual void text(pcstr) {}
    virtual void tooltip(std::pair<int, int> t) {}
    virtual int text_width() { return 0; }
    virtual vec2i pxsize() const { return size; }
    inline void text(int font, pcstr v) { this->font(font); this->text(v); }
    virtual void color(int) {}
    virtual void image(int) {}
    virtual image_desc image() const { return {}; }
    virtual void font(int) {}
    virtual void width(int v) { size.x = v; }
    virtual int value() const { return 0; }
    virtual void select(bool v) {}
    virtual void max_value(int v) {}
    virtual void onclick(std::function<void(int, int)>) {}
            void onclick(std::function<void()> f) { onclick([f] (int, int) { f(); }); }
    virtual void onevent(std::function<void()>) {}

    virtual emenu_header *dcast_menu_header() { return nullptr; }
    virtual eimage_button *dcast_image_button() { return nullptr; }

    pcstr text_from_key(pcstr key);

    inline void operator=(pcstr t) { text(t); }
    inline void operator=(const bstring512 &t) { text(t); }

    template<class T>
    void preformat_text(T& str) {
        T result;
        bool inSubstr = false;
        bstring128 replacement;

        pcstr ptr = str.c_str();
        for (; *ptr != '\0'; ++ptr) {
            if (*ptr == '#') {
                inSubstr = true;
                replacement.append(*ptr);
                continue;
            }

            if (inSubstr) {
                if (*ptr == ' ') {
                    inSubstr = false;
                    result.append(text_from_key(replacement.c_str()));
                    replacement.clear();
                } else {
                    replacement.append(*ptr);
                }
            }

            if (!inSubstr) {
                result.append(*ptr);
            }
        }

        if (inSubstr) {
            result.append(text_from_key(replacement.c_str()));
        }

        str = result;
    }

    template<class ... Args> 
    inline void text_var(pcstr fmt, const Args&... args) {
        bstring512 formated_text;
        formated_text.printf(fmt, args...);
        preformat_text(formated_text);
        text(formated_text);
    }

    using ptr = std::shared_ptr<element>;
};

struct eimg : public element {
    e_image_id img;
    image_desc img_desc;

    virtual void draw() override;
    virtual void load(archive elem) override;
    virtual image_desc image() const override { return img_desc; }
};

struct ebackground : public element {
    image_desc img_desc;
    float scale = 1.f;

    virtual void draw() override;
    virtual void load(archive elem) override;
    virtual image_desc image() const override { return img_desc; }
};

struct eresource_icon : public element {
    e_resource res;

    virtual void draw() override;
    virtual void image(int image) override;
    virtual void load(archive elem) override;
};

struct eouter_panel : public element {
    virtual void draw() override;
    virtual vec2i pxsize() const { return size * 16; }
    virtual void load(archive elem) override;
};

struct einner_panel : public element {
    virtual void draw() override;
    virtual vec2i pxsize() const { return size * 16; }
    virtual void load(archive elem) override;
};

struct elabel : public element {
    std::string _text;
    e_font _font;
    e_font _link_font;
    vec2i _body;
    uint32_t _color;
    UiFlags _flags;
    int _wrap;
    bool _clip_area;

    virtual void draw() override;
    virtual void load(archive elem) override;
    virtual void text(pcstr) override;
    virtual void color(int) override;
    virtual void font(int) override;
    virtual void width(int) override;
};

struct etext : public elabel {
    virtual void draw() override;
    virtual void load(archive elem) override;
};

struct escrollbar : public element {
    scrollbar_t scrollbar;

    virtual int value() const override { return scrollbar.scroll_position; }
    virtual void max_value(int v) override { scrollbar.max_scroll_position = v; }
    virtual void onevent(std::function<void()> func) override { scrollbar.onscroll(func); }
    virtual void draw() override;
    virtual void load(archive elem) override;
};

struct emenu_header : public element {
    menu_header impl;
    e_font _font;

    virtual void load(archive elem) override;
            void load_items(archive elem, pcstr section);
    virtual void draw() override;
    virtual void font(int v) { _font = (e_font)v; }
    virtual void text(pcstr text) override { impl.text = text; }
    virtual int text_width() override;
            menu_item &item(int i) { static menu_item dummy; return i < impl.items.size() ? impl.items[i] : dummy; }
            menu_item &item(pcstr key);
            void onclick(std::function<void(menu_item&)> f) { impl._onclick = f; }
    virtual emenu_header *dcast_menu_header() override { return this; }
};

struct egeneric_button : public elabel {
    int mode = 0;
    int param1 = 0;
    int param2 = 0;
    std::function<void(int, int)> _func;
    std::pair<int, int> _tooltip;

    virtual void draw() override;
    virtual void load(archive arch) override;
    virtual void tooltip(std::pair<int, int> t) override { _tooltip = t; }
    virtual void onclick(std::function<void(int, int)> func) override { _func = func; }
};

struct eimage_button : public element {
    e_image_id img;
    float scale = 1.f;
    image_desc img_desc;
    void *icon_texture = nullptr;
    int param1 = 0;
    int param2 = 0;
    img_button_offsets offsets;
    bool selected = false;
    bool border = false;
    int texture_id = -1;

    std::function<void(int, int)> _func;

    virtual void load(archive elem) override;
    virtual void select(bool v) override { selected = v; }
    virtual void draw() override;

    using element::onclick;
    virtual void onclick(std::function<void(int, int)> func) override { _func = func; }

    virtual eimage_button *dcast_image_button() override { return this; }
};

struct widget {
    vec2i pos;
    std::vector<element::ptr> elements;

    virtual void draw();
    virtual void load(archive arch, pcstr section = "ui");
    void load(pcstr section);

    element& operator[](pcstr id);
    inline element &operator[](const bstring32 &id) { return (*this)[id.c_str()]; }

    template<typename ... Args>
    int label(const Args ... args) { return ui::label(args...); }
    
    template<typename ... Args>
    generic_button &button(const Args ... args) { return ui::button(args...); }
    
    template<typename ... Args>
    void icon(const Args ... args) { return ui::icon(args...); }

    inline void image(image_desc img, vec2i pos) { ui::eimage(img, pos); }
    inline void begin_widget(vec2i offset, bool relative = false) { ui::begin_widget(offset, relative); }
    inline void icon(vec2i pos, e_resource img) { ui::icon(pos, img); }
    void set_clip_rectangle(vec2i pos, vec2i size);
    void set_clip_rectangle(const element &e);
    void reset_clip_rectangle();
    virtual void begin_frame() { ui::begin_frame(); }
    bool handle_mouse(const mouse *m) { return ui::handle_mouse(m); }
};

struct info_window : public widget {
    bstring128 section;
    int resource_text_group;

    inline info_window(pcstr s) : section(s) {}
    virtual void load(archive arch, pcstr section = "ui") override;
    void load();
};

} // ui