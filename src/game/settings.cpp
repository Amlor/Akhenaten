#include "settings.h"
#include <core/encoding.h>

#include "city/constants.h"
#include "core/buffer.h"
#include "core/calc.h"
#include "core/game_environment.h"
#include "core/string.h"
#include "io/io.h"

#define INF_SIZE 564

game_settings g_settings;

game_settings::game_settings() {
    inf_file = new buffer(INF_SIZE);
}

void game_settings::load_default_settings() {
    fullscreen = true;
    cli_fullscreen = false;
    display_size = {800, 600};

    sound_effects.enabled = true;
    sound_effects.volume = 100;
    sound_music.enabled = true;
    sound_music.volume = 80;
    sound_speech.enabled = true;
    sound_speech.volume = 100;
    sound_city.enabled = true;
    sound_city.volume = 100;

    game_speed = 90;
    scroll_speed = 70;

    difficulty = DIFFICULTY_HARD;
    tooltips = TOOLTIPS_FULL;
    warnings = true;
    gods_enabled = true;
    victory_video = false;
    last_advisor = ADVISOR_LABOR;

    popup_messages = 0;
    city_names_style = CITIES_OLD_NAMES;
    pyramid_speedup = false;

    setting_clear_personal_savings();
}

void game_settings::load_settings(buffer* buf) {
    buf->skip(4);
    fullscreen = buf->read_i32();
    buf->skip(3);
    sound_effects.enabled = buf->read_u8();
    sound_music.enabled = buf->read_u8();
    sound_speech.enabled = buf->read_u8();
    buf->skip(6);
    game_speed = buf->read_i32();
    game_speed = 80; // todo: fix settings
    scroll_speed = buf->read_i32();
    buf->read_raw(player_name.data(), player_name.capacity);
    buf->skip(16);
    last_advisor = buf->read_i32();
    last_advisor = ADVISOR_TRADE; // debug
    buf->skip(4);                      // int save_game_mission_id;
    tooltips = buf->read_i32();
    buf->skip(4); // int starting_kingdom;
    buf->skip(4); // int personal_savings_last_mission;
    buf->skip(4); // int current_mission_id;
    buf->skip(4); // int is_custom_scenario;
    sound_city.enabled = buf->read_u8();
    warnings = buf->read_u8();
    monthly_autosave = buf->read_u8();
    buf->skip(1); // unsigned char autoclear_enabled;
    sound_effects.volume = buf->read_i32();
    sound_music.volume = buf->read_i32();
    sound_speech.volume = buf->read_i32();
    sound_city.volume = buf->read_i32();
    buf->skip(8); // ram
    display_size.x = buf->read_i32();
    display_size.y = buf->read_i32();
    display_size = {0, 0};

    buf->skip(8); // int max_confirmed_resolution;
    for (int i = 0; i < MAX_PERSONAL_SAVINGS; i++) {
        personal_savings[i] = buf->read_i32();
    }
    victory_video = buf->read_i32();

    if (buf->at_end()) {
        // Settings file is from unpatched C3, use default values
        difficulty = DIFFICULTY_HARD;
        gods_enabled = true;
    } else {
        difficulty = (e_difficulty)buf->read_i32();
        gods_enabled = buf->read_i32();
    }
}

void game_settings::load() {
    load_default_settings();

    // TODO: load <Pharaoh.inf>
    int size = io_read_file_into_buffer("pharaoh.inf", NOT_LOCALIZED, inf_file, INF_SIZE);
    if (!size) {
        return;
    }

    load_settings(inf_file);

    if (display_size.x + display_size.y < 500) {
        // most likely migration from Caesar 3
        display_size = {800, 600};
    }
}

void game_settings::save() {
    buffer* buf = inf_file;

    buf->skip(4);
    buf->write_i32(fullscreen);
    buf->skip(3);
    buf->write_u8(sound_effects.enabled);
    buf->write_u8(sound_music.enabled);
    buf->write_u8(sound_speech.enabled);
    buf->skip(6);
    buf->write_i32(game_speed);
    buf->write_i32(scroll_speed);
    buf->write_raw(player_name.data(), player_name.capacity);
    buf->skip(16);
    buf->write_i32(last_advisor);
    buf->skip(4); // int save_game_mission_id;
    buf->write_i32(tooltips);
    buf->skip(4); // int starting_kingdom;
    buf->skip(4); // int personal_savings_last_mission;
    buf->skip(4); // int current_mission_id;
    buf->skip(4); // int is_custom_scenario;
    buf->write_u8(sound_city.enabled);
    buf->write_u8(warnings);
    buf->write_u8(monthly_autosave);
    buf->skip(1); // unsigned char autoclear_enabled;
    buf->write_i32(sound_effects.volume);
    buf->write_i32(sound_music.volume);
    buf->write_i32(sound_speech.volume);
    buf->write_i32(sound_city.volume);
    buf->skip(8); // ram
    buf->write_i32(display_size.x);
    buf->write_i32(display_size.y);
    buf->skip(8); // int max_confirmed_resolution;
    for (int i = 0; i < MAX_PERSONAL_SAVINGS; i++) {
        buf->write_i32(personal_savings[i]);
    }
    buf->write_i32(victory_video);
    buf->write_i32(difficulty);
    buf->write_i32(gods_enabled);

    io_write_buffer_to_file("pharaoh.inf", inf_file, INF_SIZE);
}

sound_settings* game_settings::get_sound(int type) {
    auto& data = g_settings;
    switch (type) {
    case SOUND_MUSIC:
        return &data.sound_music;
    case SOUND_EFFECTS:
        return &data.sound_effects;
    case SOUND_SPEECH:
        return &data.sound_speech;
    case SOUND_CITY:
        return &data.sound_city;
    default:
        return 0;
    }
}

void game_settings::increase_game_speed(void) {
    if (game_speed >= 100) {
        if (game_speed < 1000)
            game_speed += 100;
    } else {
        game_speed = calc_bound(game_speed + 10, 10, 100);
    }
}

void game_settings::decrease_game_speed(void) {
    if (game_speed > 100) {
        game_speed -= 100;
    } else {
        game_speed = calc_bound(game_speed - 10, 10, 100);
    }
}

void game_settings::toggle_tooltips() {
    auto& data = g_settings;
    switch (data.tooltips) {
    case TOOLTIPS_NONE:
        data.tooltips = TOOLTIPS_SOME;
        break;
    case TOOLTIPS_SOME:
        data.tooltips = TOOLTIPS_FULL;
        break;
    default:
        data.tooltips = TOOLTIPS_NONE;
        break;
    }
}

void game_settings::set_player_name(const uint8_t* name) {
    string_copy(name, player_name, MAX_PLAYER_NAME);
    encoding_to_utf8(name, player_name_utf8, MAX_PLAYER_NAME, 0);
}

int setting_personal_savings_for_mission(int mission_id) {
    auto& data = g_settings;
    return data.personal_savings[mission_id];
}
void setting_set_personal_savings_for_mission(int mission_id, int savings) {
    auto& data = g_settings;
    data.personal_savings[mission_id] = savings;
}
void setting_clear_personal_savings() {
    auto& data = g_settings;
    for (int i = 0; i < g_settings.MAX_PERSONAL_SAVINGS; i++) {
        data.personal_savings[i] = 0;
    }
}
