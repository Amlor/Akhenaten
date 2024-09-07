#pragma once

#include "window/advisors.h"
#include "graphics/elements/ui.h"

namespace ui {
struct advisor_labors_window : public advisor_window_t<advisor_labors_window> {
    virtual int handle_mouse(const mouse *m) override { return 0; }
    virtual int get_tooltip_text() override { return 0; }
    virtual void draw_foreground() override {};
    virtual void ui_draw_foreground() override;
    virtual int draw_background() override;
    virtual void init() override {};

    static advisor_window *instance();
};
}
