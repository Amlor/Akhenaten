#include "js.h"

#include "content/dir.h"
#include "core/log.h"
#include "graphics/window.h"
#include "js/js_constants.h"
#include "js/js_defines.h"
#include "js/js_folder_notifier.h"
#include "js/js_game.h"
//#include "js/js_graphics.h"
//#include "js/js_hotkey.h"
//#include "js/js_ui.h"
//#include "js/js_mouse.h"
#include "graphics/elements/panel.h"
#include "mujs/mujs.h"
#include "mujs/jsi.h"
#include "mujs/jsvalue.h"
#include "platform/platform.h"

#include <filesystem>

#define MAX_FILES_RELOAD 255

struct {
    vfs::path scripts_folder;
    vfs::path files2load[MAX_FILES_RELOAD];
    int files2load_num;
    int have_error;
    bstring256 error_str;
    js_State *J;
} vm;

void js_reset_vm_state();

int js_vm_trypcall(js_State *J, int params) {
    if (vm.have_error) {
        return 0;
    }

    int error = js_pcall(vm.J, params);
    if (error) {
        vm.have_error = 1;
        const char *cur_symbol = js_tostring(vm.J, -1);
        const char *start_str = cur_symbol;
        while (*cur_symbol) {
            if (*cur_symbol != '\n') {
                cur_symbol++;
                continue;
            }

            vm.error_str.printf("%.*s", cur_symbol - start_str, start_str);
            start_str = cur_symbol + 1;
            cur_symbol += 2;
            logs::info("!!! pcall error %s", vm.error_str.c_str());
        }
        logs::info("!!! pcall error %s", start_str);
        js_pop(J, 1);
        return 0;
    }

    js_pop(vm.J, -1);
    return 1;
}

int js_vm_load_file_and_exec(const char *path) {
    int error = 0;

    if (!path || !*path) {
        return 0;
    }

    const char *npath = (*path == ':') ? (path + 1) : path;

    bstring256 rpath = path;
    if (!vm.scripts_folder.empty()) {
        rpath = js_vm_get_absolute_path(npath);
    } 

    vfs::reader reader = vfs::file_open(rpath);

    if (!reader) {
        logs::info("!!! Cant find script at %s", rpath.c_str());
        return 0;
    }

    ((char*)reader->data())[reader->size() - 1] = 0; // EOF -> 0, that make it as string
    error = js_ploadstring(vm.J, rpath, (const char*)reader->begin());
    if (error) {
        logs::info("!!! Error on open file %s", js_tostring(vm.J, -1));
        return 0;
    }

    js_getglobal(vm.J, "");
    int ok = js_vm_trypcall(vm.J, 0);
    if (!ok) {
        logs::info("Fatal error on call base after load %s", path);
        //js_pop(internal_J, 1);
        return 0;
    }
    return 1;
}

void js_vm_reload_configs() {
    js_config_load_building_sounds(vm.J);
    js_config_load_mission_sounds(vm.J);
    js_config_load_walker_sounds(vm.J);
    js_config_load_city_sounds(vm.J);
}

void js_vm_sync() {
    if (!vm.files2load_num) {
        return;
    }

    if (vm.have_error) {
        js_reset_vm_state();
    }

    if (vm.files2load_num > 0) {
        for (int i = 0; i < vm.files2load_num; i++) {
            logs::info("JS: script reloaded %s", vm.files2load[i].c_str());
            js_vm_load_file_and_exec(vm.files2load[i]);
        }
    }

    for (int i = 0; i < MAX_FILES_RELOAD; ++i) {
        vm.files2load[i].clear();
    }

    js_vm_reload_configs();

    vm.files2load_num = 0;
    vm.have_error = 0;
    window_request_refresh();
}

void js_vm_reload_file(const char *path) {
    vm.files2load[vm.files2load_num] = path;
    vm.files2load_num++;
}

int js_vm_exec_function_args(const char *funcname, const char *szTypes, ...) {
    if (vm.have_error)
        return 0;
    int i, ok, savetop;
    char msg[2] = { 0, 0 };
    va_list vl;

    if (vm.J == 0)
        return 1;

    //log_info("script-if:// exec function ", funcname, 0);

    savetop = js_gettop(vm.J);
    js_getglobal(vm.J, funcname);
    js_pushnull(vm.J);

    //  szTypes is the last argument specified; you must access
    //  all others using the variable-argument macros.
    va_start( vl, szTypes );

    // Step through the list.
    for( i = 0; szTypes[i] != '\0'; ++i ) {
        switch( szTypes[i] ) {   // Type to expect.
            case 'i':
                js_pushnumber(vm.J, va_arg(vl, int));
                break;
            case 'f':
                js_pushnumber(vm.J, va_arg(vl, float));
                break;
            case 'c':
                msg[0] = va_arg(vl, char);
                js_pushstring(vm.J, msg);
                break;
            case 's':
                js_pushstring(vm.J, va_arg(vl, char *));
                break;

            default:
                js_pushnull(vm.J);
                logs::info("!!! Undefined value for js.pcall engine_js_push when find ");
                break;
        }
    }
    va_end( vl );

    ok = js_vm_trypcall(vm.J, (int)strlen(szTypes));
    if (!ok) {
        logs::info("Fatal error on call function %s", funcname);
        return 0;
    }

    js_pop(vm.J, 2);
    if( savetop - js_gettop(vm.J) != 0 ) {
        logs::info( "STACK grow for %s [%d]", funcname, js_gettop(vm.J) );
    }
    return ok;
}

int js_vm_exec_function(const char *funcname) {
    return js_vm_exec_function_args(funcname, "");
}

void js_vm_load_module(js_State *J) {
    const char *scriptName = js_tostring(J, 1);

    vm.files2load[vm.files2load_num] = scriptName;
    vm.files2load_num++;
}

void js_game_panic(js_State *J) {
    logs::info("JSE !!! Uncaught exception: %s", js_tostring(J, -1));
}

void js_register_vm_functions(js_State *J) {
    REGISTER_GLOBAL_FUNCTION(J, js_vm_load_module, "include", 1);
}

void js_reset_vm_state() {
    if (vm.J) {
        js_freestate(vm.J);
        vm.J = NULL;
    }

    for (int i = 0; i < MAX_FILES_RELOAD; ++i) {
        vm.files2load[i].clear();
    }
    vm.files2load_num = 0;
    vm.have_error = 0;

    vm.J = js_newstate(NULL, NULL, JS_STRICT);
    js_atpanic(vm.J, js_game_panic);

    js_register_vm_functions(vm.J);
    //js_register_graphics_functions(vm.J);
    js_register_game_functions(vm.J);
    //js_register_mouse_functions(vm.J);
    //js_register_hotkey_functions(vm.J);
    js_register_game_constants(vm.J);
    js_register_city_sound_constants(vm.J);

    int ok = js_vm_load_file_and_exec(":modules.js");
    if (ok) {
        js_pop(vm.J, 2); //restore stack after call js-function
    }
    logs::info( "STACK state %d", js_gettop(vm.J));
}

void js_vm_set_scripts_folder(vfs::path folder) {
    vm.scripts_folder = folder;
}

vfs::path js_vm_get_absolute_path(vfs::path path) {
#if defined(GAME_PLATFORM_WIN)
    bool is_absolute_path = path.data()[1] == ':' && path.len() > 2;
#else
    bool is_absolute_path = path.data()[0] == '/' && path.len() > 1;
#endif
    if (is_absolute_path) {
        return path;
    }

    vfs::path buffer;
    if (!vm.scripts_folder.empty()) {
        vfs::path conpath(vm.scripts_folder, "/", path);

#if defined(GAME_PLATFORM_WIN)
        char *p = _fullpath(buffer, conpath, buffer.capacity);
#elif defined(GAME_PLATFORM_LINUX) || defined(GAME_PLATFORM_MACOSX)
        realpath(conpath, buffer);
#endif
        buffer.replace('\\', '/');
        return buffer;
    }

    buffer = vfs::content_path(vfs::SCRIPTS_FOLDER);
    return buffer;
}

void js_vm_setup() {
    vm.J = nullptr;
    js_reset_vm_state();

    vfs::path abspath = js_vm_get_absolute_path("");
    vfs::path modules_file(abspath, "/modules.js");
    bool scripts_folder_exists = std::filesystem::exists(modules_file.c_str());
    if (scripts_folder_exists) {
        js_vm_notifier_watch_directory_init(abspath);
    }
}