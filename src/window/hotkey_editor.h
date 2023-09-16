#pragma once

#include "input/keys.h"
#include "config/hotkeys.h"

void window_hotkey_editor_show(int action, int index, void (*callback)(int, int, int, int));

void window_hotkey_editor_key_pressed(int key, int modifiers);
void window_hotkey_editor_key_released(int key, int modifiers);