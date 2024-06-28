#pragma once

enum e_music_track {
    TRACK_NONE,
    TRACK_MENU,
    TRACK_COMBAT_SHORT,
    TRACK_COMBAT_LONG,
    TRACK_CITY_1,
    TRACK_CITY_2,
    TRACK_CITY_3,
    TRACK_CITY_4,
    TRACK_CITY_5,
    TRACK_CITY_6,
    TRACK_CITY_7,
    TRACK_CITY_8,
    TRACK_CITY_9,
    TRACK_CITY_10,
    TRACK_CITY_11,
    TRACK_CITY_12,
    TRACK_CITY_13,
    TRACK_CITY_14,
    TRACK_CITY_15,
    TRACK_CITY_16,
    TRACK_CITY_17,
    TRACK_CITY_18,
    TRACK_CITY_19,
    TRACK_CITY_20,
    TRACK_CITY_21,
    TRACK_CITY_22,
    TRACK_CITY_23,
    TRACK_CITY_24,
    TRACK_CITY_25,
    TRACK_CITY_26,
    TRACK_CITY_27,
    TRACK_CITY_28,
    TRACK_CITY_29,
    TRACK_CITY_30,
    TRACK_CITY_31,
    TRACK_CITY_32,
    TRACK_CITY_33,
    TRACK_CITY_34,
    TRACK_CITY_35,
    TRACK_CITY_36,
    TRACK_CITY_37,
    TRACK_CITY_38,
    TRACK_CITY_39,
    TRACK_CITY_40,
    TRACK_MAX
};

void sound_music_play_intro();
void sound_music_play_track(int track);

void sound_music_play_editor();

void sound_music_update(bool force);

void sound_music_stop();
