#pragma once

#include "mujs/mujs.h"

#include "core/bstring.h"
#include "core/core.h"
#include "js/js_defines.h"
#include "core/vec2i.h"
#include "core/archive.h"

#include <vector>
#include <string>

void js_register_game_functions(js_State *J);
void js_register_game_objects(js_State *J);

namespace config {

void refresh(archive);
archive load(pcstr filename);

using config_iterator_function_cb = void ();
using ArchiveIterator = FuncLinkedList<config_iterator_function_cb*>;

} // end namespace config

#define ANK_REGISTER_CONFIG_ITERATOR(func) void func(); \
    namespace config {int ANK_CONFIG_PULL_VAR_NAME(func) = 1;} \
    static config::ArchiveIterator ANK_CONFIG_CC1(config_handler, __LINE__)(func)
