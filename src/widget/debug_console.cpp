#include "debug_console.h"

#include "graphics/screen.h"
#include "graphics/graphics.h"
#include "graphics/text.h"
#include "building/destruction.h"
#include "game/game.h"
#include "dev/imgui_qconsole.h"
#include "building/building.h"

#include <SDL.h>
#include <SDL_keyboard.h>
#include "platform/keyboard_input.h"
#include "platform/renderer.h"

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/backends/imgui_impl_sdlrenderer.h"
#include "imgui/backends/imgui_impl_sdl.h"
#include "dev/debug.h"

#include <iostream>

dev::imgui_qconsole *_debug_console = nullptr;

dev::imgui_qconsole &debug_console() {
    if (!_debug_console) {
        _debug_console = new dev::imgui_qconsole();
    }
    return *_debug_console;
}

void game_debug_cli_draw() {
    auto renderer = graphics_renderer();
    SDL_Point platform_window_size;
    SDL_GetWindowSize(renderer->window(), &platform_window_size.x, &platform_window_size.y);

    debug_console().render("##console", game.debug_console, platform_window_size.x, platform_window_size.y * 0.33f);
}

void game_debug_show_property_value(pcstr field, const float &v, bool disabled) {
    ImGui::InputFloat("##value", (float*)&v, 1.0f);
}

void game_debug_show_property_value(pcstr field, const int &v, bool disabled) {
    ImGuiInputTextFlags_ flags = disabled ? ImGuiInputTextFlags_ReadOnly : ImGuiInputTextFlags_None;
    ImGui::InputInt("##value", (int*)&v, 1, 100, flags);
}

void game_debug_show_property_value(pcstr field, const e_move_type &v, bool disabled) {
    ImGui::InputScalar(field, ImGuiDataType_S8, (void *)&v);
}

void game_debug_show_property_value(pcstr field, const int8_t &v, bool disabled) {
    ImGui::InputScalar(field, ImGuiDataType_S8, (void *)&v);
}

void game_debug_show_property_value(pcstr field, const short &v, bool disabled) {
    ImGui::InputScalar(field, ImGuiDataType_S16, (void *)&v);
}

void game_debug_show_property_value(pcstr field, const uint8_t &v, bool disabled) {
    ImGui::InputScalar(field, ImGuiDataType_U8, (void *)&v);
}

void game_debug_show_property_value(pcstr field, const uint16_t &v, bool disabled) {
    ImGui::InputScalar(field, ImGuiDataType_U16, (void *)&v);
}

void game_debug_show_property_value(pcstr field, const bool &v, bool disabled) {
    ImGui::Checkbox("", (bool*)&v);
}

void game_debug_show_property_value(pcstr field, const bstring64 &v, bool disabled) {
    ImGui::Text(!!v ? v.c_str() : "none");
}

void game_debug_show_property_value(pcstr field, const bstring256 &v, bool disabled) {
    ImGui::Text(!!v ? v.c_str() : "none");
}

void game_debug_show_property_value(pcstr field, const xstring &v, bool disabled) {
    ImGui::Text(!!v ? v.c_str() : "none");
}

void game_debug_show_property_value(pcstr field, const vec2i &v, bool disabled) {
    ImGui::InputInt2(field, (int*)&v);
}

void game_debug_show_property_value(pcstr field, const tile2i &v, bool disabled) {
    ImGui::InputInt2(field, (int*)&v);
}

void game_debug_show_property_value(pcstr field, const std::function<void()> &f, bool disabled) {
    if (ImGui::Button(field)) {
        f();
    }
}

template<typename T>
void game_debug_show_property_t(int &i, pcstr field, const T &v, bool disabled = false) {
    ImGui::PushID(i); 
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::AlignTextToFramePadding();
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Bullet;
    ImGui::TreeNodeEx("Field", flags, "%s", field);
    ImGui::TableSetColumnIndex(1);
    ImGui::SetNextItemWidth(-FLT_MIN);

    if (disabled) {
        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
    }

    game_debug_show_property_value(field, v, disabled);
    
    if (disabled) {
        ImGui::PopItemFlag();
    }
    ImGui::NextColumn();
    ImGui::PopID();
    ++i;
}

void game_debug_show_property_t(int &i, pcstr field, pcstr v) {
    bstring256 _v(v);
    game_debug_show_property_t(i, field, _v);
}

void game_debug_show_property(int &i, pcstr field, const int &v, bool disabled)  { game_debug_show_property_t(i, field, v, disabled); }
void game_debug_show_property(int &i, pcstr field, const e_move_type &v, bool disabled)  { game_debug_show_property_t(i, field, v, disabled); }
void game_debug_show_property(int &i, pcstr field, const int8_t &v, bool disabled)  { game_debug_show_property_t(i, field, v, disabled); }
void game_debug_show_property(int &i, pcstr field, const short &v, bool disabled) { game_debug_show_property_t(i, field, v, disabled); }
void game_debug_show_property(int &i, pcstr field, const uint8_t &v, bool disabled) { game_debug_show_property_t(i, field, v, disabled); }
void game_debug_show_property(int &i, pcstr field, const uint16_t &v, bool disabled) { game_debug_show_property_t(i, field, v, disabled); }
void game_debug_show_property(int &i, pcstr field, const bool &v, bool disabled) { game_debug_show_property_t(i, field, v, disabled); }
void game_debug_show_property(int &i, pcstr field, const bstring64 &v, bool disabled) { game_debug_show_property_t(i, field, v, disabled); }
void game_debug_show_property(int &i, pcstr field, const bstring256 &v, bool disabled) { game_debug_show_property_t(i, field, v, disabled); }
void game_debug_show_property(int &i, pcstr field, const xstring &v, bool disabled) { game_debug_show_property_t(i, field, v, disabled); }
void game_debug_show_property(int &i, pcstr field, const vec2i &v, bool disabled) { game_debug_show_property_t(i, field, v, disabled); }
void game_debug_show_property(int &i, pcstr field, const tile2i &v, bool disabled) { game_debug_show_property_t(i, field, v, disabled); }
void game_debug_show_property(int &i, pcstr field, const std::function<void()> &f, bool disabled)  { game_debug_show_property_t(i, field, f, disabled); }

static void game_debug_show_properties_object(pcstr prefix, const event_ph_t &e) {
    ImGui::PushID(0x80000000 | e.event_id);

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::AlignTextToFramePadding();
    bool common_open = ImGui::TreeNodeEx("Event", ImGuiTreeNodeFlags_DefaultOpen, "Event %d", e.event_id);
    ImGui::TableSetColumnIndex(1); 

    int i = 0;
    if (common_open) {
        game_debug_show_property_t(i, "event_id", e.event_id, true);

        bstring256 type_name;
        
        type_name.printf("%s [%d]", token::find_name(e_event_type_tokens, e.type), e.type);
        game_debug_show_property_t(i, "<type>", type_name);
        game_debug_show_property_t(i, "month", e.month);
        game_debug_show_property_t(i, "time.year", e.time.value);
        game_debug_show_property_t(i, "time.f_fixed", e.time.f_fixed);
        game_debug_show_property_t(i, "months_initial", e.months_initial);
        game_debug_show_property_t(i, "quest_months_left", e.quest_months_left);

        type_name.printf("%s [%d]", token::find_name(e_event_state_tokens, e.event_state), e.event_state);
        game_debug_show_property_t(i, "<event_state>", type_name);
        game_debug_show_property_t(i, "event_state", e.event_state);
        game_debug_show_property_t(i, "is_overdue", e.is_overdue);
        game_debug_show_property_t(i, "is_active", e.is_active);
        game_debug_show_property_t(i, "can_comply_dialog_shown", e.can_comply_dialog_shown);
        game_debug_show_property_t(i, "festival_deity", e.festival_deity);
        game_debug_show_property_t(i, "on_too_late_action", e.on_too_late_action);
        game_debug_show_property_t(i, "on_defeat_action", e.on_defeat_action);
        game_debug_show_property_t(i, "sender_faction", e.sender_faction);

        type_name.printf("%s [%d]", token::find_name(e_event_trigger_type_tokens, e.event_trigger_type), e.event_trigger_type);
        game_debug_show_property_t(i, "<event_trigger_type>", type_name);

        if (e.type == EVENT_TYPE_REQUEST) {
            type_name.printf("%s [%d]", resource_name((e_resource)e.item.value), e.item.value);
            game_debug_show_property_t(i, "<item.value>", type_name);
        }

        game_debug_show_property_t(i, "item.value", e.item.value);
        game_debug_show_property_t(i, "item.f_fixed", e.item.f_fixed);
        
        ImGui::TreePop();
    }
    ImGui::PopID();
}

void game_debug_properties_draw() {
    if (!game.debug_properties) {
        return;
    }

    static bool _debug_events_open = false;

    ImGui::SetNextWindowSize(ImVec2(430, 450), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Properties", &game.debug_properties)) {
        ImGui::End();
        return;
    }

    for (debug::PropertiesIterator *s = debug::PropertiesIterator::tail; s; s = s->next) {
        s->func(/*header*/true);
        if (s->next) {
            ImGui::SameLine();
        }
    }

    ImGui::Checkbox("Events", &_debug_events_open);

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
    for (debug::PropertiesIterator *s = debug::PropertiesIterator::tail; s; s = s->next) {
        s->func(/*body*/false);
    }

    if (_debug_events_open && ImGui::BeginTable("split", 2, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_Resizable)) {
        for (int i = 0; i < g_scenario_data.events.events_count(); ++i) {
            const event_ph_t *evt = g_scenario_data.events.at(i);
            assert(evt);
            game_debug_show_properties_object("Events", *evt);
        }
        ImGui::EndTable();
    }
    ImGui::PopStyleVar();

    ImGui::End();

    //g_debug_figure_id = 0;
}

void game_debug_cli_message(pcstr msg) {
    debug_console() << msg << std::endl;
}

void game_imgui_overlay_init() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    // Setup Platform/Renderer backends
    auto renderer = graphics_renderer();
    ImGui_ImplSDL2_InitForSDLRenderer(renderer->window(), renderer->renderer());
    ImGui_ImplSDLRenderer_Init(renderer->renderer());

    debug_console().con.bind_command("close", [] (auto &, auto &) { game.debug_console = false; });
}

void game_imgui_overlay_destroy() {
    ImGui_ImplSDLRenderer_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

void game_imgui_overlay_begin_frame() {
    if (!game.debug_console) {
        //return;
    }

    ImGui_ImplSDLRenderer_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
}

void game_imgui_overlay_draw() {
    if (!game.debug_console) {
    //    return;
    }

    ImGui::Render();
    ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
}

bool game_imgui_overlay_handle_event(void *e) {
    const SDL_Event *event = (const SDL_Event *)e;

    if (event->type == SDL_KEYDOWN) {
        int key = event->key.keysym.scancode;

        if (key == SDL_SCANCODE_GRAVE) {
            game_toggle_debug_console();
        }
    }

    if (event->type == SDL_TEXTINPUT && *event->text.text == '`') {
        debug_console().skip_event = true;
    }

    if (!(game.debug_console || game.debug_properties)) {
        return false;
    }

    if (debug_console().skip_event) {
        debug_console().skip_event = false;
        return false;
    }

    ImGui_ImplSDL2_ProcessEvent(event);
    return false;
}

void game_toggle_debug_console() {
    game.debug_console = !game.debug_console;
    if (game.debug_console) {
        debug_console().is.reclaim_focus = true;
    }
}

void bind_debug_command(pcstr cmd, std::function<void(std::istream &, std::ostream &)> f) {
    debug_console().con.bind_command(cmd, f);
}

void bind_debug_console_var_int(pcstr var, int &ref) {
    debug_console().con.bind_cvar(var, ref);
}

void bind_debug_console_var_int8(pcstr var, int8_t &ref) {
    debug_console().con.bind_cvar(var, ref);
}

void bind_debug_console_var_int16(pcstr var, int16_t &ref) {
    debug_console().con.bind_cvar(var, ref);
}

void bind_debug_console_var_bool(pcstr var, bool &ref) {
    debug_console().con.bind_cvar(var, ref);
}
