#pragma once

#include "window/advisors.h"
#include "graphics/elements/ui.h"

namespace ui {

struct advisor_imperial_window : public advisor_window_t<advisor_imperial_window> {
    virtual int handle_mouse(const mouse *m) override { return 0; }
    virtual int get_tooltip_text() override { return 0;  }
    virtual void ui_draw_foreground(UiFlags flags) override;
    virtual int draw_background() override;
    virtual void draw_foreground() override;
    virtual void init() override {}

    virtual void load(archive arch, pcstr) override;

    void handle_request(int index);

    static advisor_window *instance();
};

}

