#include "core/archive.h"

#include "mujs/mujs.h"

void archive::getproperty(int idx, pcstr name) {
    js_getproperty((js_State*)state, idx, name);
}

void archive::getproperty(archive arch, int idx, pcstr name) {
    js_getproperty((js_State *)(arch.state), idx, name);
}

bool archive::isarray(int idx) {
    return js_isarray((js_State*)state, idx);
}

int archive::getlength(int idx) {
    return js_getlength((js_State*)state, idx);
}

void archive::getindex(int idx, int i) {
    js_getindex((js_State*)state, idx, i);
}

bool archive::isnumber(int idx) {
    return js_isnumber((js_State*)state, idx);
}

double archive::tonumber(int idx) {
    return js_tonumber((js_State*)state, idx);
}

void archive::pop(int num) {
    js_pop((js_State *)state, num);
}

void archive::pop(archive arch, int n) {
    js_pop((js_State *)(arch.state), n);
}

bool archive::isobject(int idx) {
    return js_isobject((js_State *)state, idx);
}

bool archive::isobject(archive arch, int idx) {
    return js_isobject((js_State *)(arch.state), idx);
}

void archive::pushiterator(archive arch, int idx, int own) {
    js_pushiterator((js_State *)(arch.state), idx, own);
}

pcstr archive::nextiterator(archive arch, int idx) {
    return js_nextiterator((js_State *)(arch.state), idx);
}

void archive::getglobal(pcstr name) {
    js_getglobal((js_State *)state, name);
}

const uint8_t *lang_get_string(int group, int index);
pcstr archive::r_string(pcstr name) {
    auto vm = (js_State *)state;
    js_getproperty(vm, -1, name);
    const char *result = "";
    if (js_isundefined(vm, -1)) {
        ;
    } else if (js_isstring(vm, -1)) {
        result = js_tostring(vm, -1);
    } else if (js_isobject(vm, -1)) {
        js_getproperty(vm, -1, "group"); int group = js_isundefined(vm, -1) ? 0 : js_tointeger(vm, -1); js_pop(vm, 1);
        js_getproperty(vm, -1, "id"); int id = js_isundefined(vm, -1) ? 0 : js_tointeger(vm, -1); js_pop(vm, 1);
        result = (pcstr)lang_get_string(group, id);
    }
    js_pop(vm, 1);
    return result;
}

std::vector<std::string> archive::r_array_str(pcstr name) {
    auto vm = (js_State *)state;
    js_getproperty(vm, -1, name);
    std::vector<std::string> result;
    if (js_isarray(vm, -1)) {
        int length = js_getlength(vm, -1);

        for (int i = 0; i < length; ++i) {
            js_getindex(vm, -1, i);
            std::string v = js_tostring(vm, -1);
            result.push_back(v);
            js_pop(vm, 1);
        }
        js_pop(vm, 1);
    }
    return result;
}

int archive::r_int(pcstr name, int def) {
    auto vm = (js_State *)state;
    js_getproperty(vm, -1, name);
    int result = js_isundefined(vm, -1) ? def : js_tointeger(vm, -1);
    js_pop(vm, 1);
    return result;
}

uint32_t archive::r_uint(pcstr name, uint32_t def) {
    auto vm = (js_State *)state;
    js_getproperty(vm, -1, name);
    uint32_t result = js_isundefined(vm, -1) ? def : js_touint32(vm, -1);
    js_pop(vm, 1);
    return result;
}

e_image_id archive::r_image(pcstr name) { 
    return (e_image_id)r_int(name);
}

bool archive::r_bool(pcstr name, bool def) {
    auto vm = (js_State *)state;
    js_getproperty(vm, -1, name);
    bool result = js_isundefined(vm, -1) ? def : js_toboolean(vm, -1);
    js_pop(vm, 1);
    return result;
}

vec2i archive::r_size2i(pcstr name, pcstr w, pcstr h) {
    return r_vec2i(name, w, h);
}

vec2i archive::r_vec2i(pcstr name, pcstr x, pcstr y) {
    auto vm = (js_State *)state;
    vec2i result(0, 0);
    js_getproperty(vm, -1, name);
    if (js_isobject(vm, -1)) {
        if (js_isarray(vm, -1)) {
            int length = js_getlength(vm, -1);
            if (length > 0) {
                js_getindex(vm, -1, 0); result.x = !js_isundefined(vm, -1) ? js_tointeger(vm, -1) : 0; js_pop(vm, 1);
                if (length > 1) {
                    js_getindex(vm, -1, 1); result.y = !js_isundefined(vm, -1) ? js_tointeger(vm, -1) : 0; js_pop(vm, 1);
                }
            }
        } else {
            js_getproperty(vm, -1, x); result.x = !js_isundefined(vm, -1) ? js_tointeger(vm, -1) : 0; js_pop(vm, 1);
            js_getproperty(vm, -1, y); result.y = !js_isundefined(vm, -1) ? js_tointeger(vm, -1) : 0; js_pop(vm, 1);
        }
    }
    js_pop(vm, 1);

    return result;
}
