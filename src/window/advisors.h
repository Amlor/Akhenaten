#pragma once

#include "city/constants.h"
#include "graphics/elements/ui.h"
#include "core/typename.h"
#include "core/log.h"
#include "window/autoconfig_window.h"

struct mouse;

using advisor_window = autoconfig_window;

template<typename T>
using advisor_window_t = autoconfig_window_t<T>;

int window_advisors_get_advisor();
bool is_advisor_available(e_advisor advisor);
void window_advisors_show();
void window_advisors_show_checked();
int window_advisors_show_advisor(e_advisor advisor);
