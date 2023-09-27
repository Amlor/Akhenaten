#include <SDL.h>

#ifdef __APPLE__
#include <SDL2_mixer/SDL_mixer.h>
#else
#include <SDL_mixer.h>
#endif

#include "core/game_environment.h"
#include "content/file_formats.h"
#include "core/log.h"
#include "game/settings.h"
#include "content/vfs.h"
#include "platform/platform.h"
#include "platform/vita/vita.h"
#include "sound/device.h"

#include "lame_helper.h"

#include <stdlib.h>
#include <string.h>
#include <vector>
#include <map>

#ifdef __vita__
#include <psp2/io/fcntl.h>
#endif

#define AUDIO_RATE 22050
#define AUDIO_FORMAT AUDIO_S16
#define AUDIO_CHANNELS 2
#define AUDIO_BUFFERS 1024

#define MAX_CHANNELS 150

#if SDL_VERSION_ATLEAST(2, 0, 7)
#define USE_SDL_AUDIOSTREAM
#endif
#define HAS_AUDIOSTREAM() (platform_sdl_version_at_least(2, 0, 7))

#ifdef __vita__
static struct {
    char filename[FILE_NAME_MAX];
    char* buffer;
    int size;
} vita_music_data;
#endif

struct sound_channel {
    vfs::path filename;
    Mix_Chunk* chunk;
};

struct sound_device_data_t {
    int initialized;
    Mix_Music* music;
    sound_channel channels[MAX_CHANNELS];
};

sound_device_data_t g_sound_device_data;

struct music_format {
    const int flag;
    const char* desc;
};

struct custom_music_t {
    SDL_AudioFormat format;
#ifdef USE_SDL_AUDIOSTREAM
    SDL_AudioStream* stream;
    int use_audiostream;
#endif
    SDL_AudioCVT cvt;
    unsigned char* buffer;
    int buffer_size;
    int cur_read;
    int cur_write;
    std::map<std::string, vfs::reader> cached_chunks;
};

custom_music_t g_custom_music;

static int percentage_to_volume(int percentage) {
    return percentage * SDL_MIX_MAXVOLUME / 100;
}

static vfs::reader load_cached_chunk(vfs::path filename) {
    auto &data = g_custom_music;

    auto it = data.cached_chunks.insert({filename.c_str(), vfs::reader()});

    if (!it.second) {
        return it.first->second;
    }

    vfs::path converted_wav(filename);
    bool need_converting = false;
    auto format = get_format_from_file(filename);
    if (format == FILE_FORMAT_MP3) {
        // first check we have converted file on the disk
        vfs::file_change_extension(converted_wav.data(), "wav");
        need_converting = !vfs::file_exists(converted_wav);
    }
    
    if (need_converting) {
        lame_helper helper;
        it.first->second = helper.decode(filename);
    } else {
        it.first->second = vfs::file_open(converted_wav);
    }

    return it.first->second;
}

static Mix_Chunk* load_chunk(const char* filename) {
    if (filename && *filename) {
        auto format = get_format_from_file(filename);
        if (format == FILE_FORMAT_MP3) {
            vfs::reader r = load_cached_chunk(filename);

            if (!r) {
                return nullptr;
            }

            SDL_RWops* sdl_fp = SDL_RWFromConstMem(r->data(), r->size());
            return Mix_LoadWAV_RW(sdl_fp, SDL_FALSE);
        }

#if defined(__vita__) || defined(GAME_PLATFORM_ANDROID)
        FILE* fp = vfs::file_open(filename, "rb");
        if (!fp) {
            return NULL;
        }

        SDL_RWops* sdl_fp = SDL_RWFromFP(fp, SDL_TRUE);
        return Mix_LoadWAV_RW(sdl_fp, 1);
#else
        return Mix_LoadWAV_RW(SDL_RWFromFile(filename, "rb"), 1);
#endif
    } else {
        return NULL;
    }
}

static int load_channel(sound_channel* channel) {
    if (!channel->chunk && !channel->filename.empty()) {
        channel->chunk = load_chunk(channel->filename);
    }

    return channel->chunk ? 1 : 0;
}

static void init_channels(void) {
    auto &data = g_sound_device_data;
    data.initialized = 1;
    for (int i = 0; i < MAX_CHANNELS; i++) {
        data.channels[i].chunk = 0;
    }
}

void sound_device_init_channel(int index, vfs::path filename) {
    auto &data = g_sound_device_data;
    data.channels[index].chunk = nullptr;
    data.channels[index].filename = filename;
}

void sound_device_init_channels(std::span<vfs::path> channels) {
    auto &data = g_sound_device_data;
    if (!data.initialized) {
        return;
    }
    int num_channels = channels.size();
    if (num_channels > MAX_CHANNELS) {
        num_channels = MAX_CHANNELS;
    }

    Mix_AllocateChannels(num_channels);
    logs::info("Loading audio files");

    for (int i = 0; i < num_channels; i++) {
        sound_device_init_channel(i, channels[i]);
    }
}

void sound_device_open(void) {
#ifdef USE_SDL_AUDIOSTREAM
    g_custom_music.use_audiostream = HAS_AUDIOSTREAM();
#endif
    if (0 == Mix_OpenAudio(AUDIO_RATE, AUDIO_FORMAT, AUDIO_CHANNELS, AUDIO_BUFFERS)) {
        init_channels();
        return;
    }
    logs::error("Sound failed to initialize using default driver: %s", Mix_GetError());
    // Try to work around SDL choosing the wrong driver on Windows sometimes
    for (int i = 0; i < SDL_GetNumAudioDrivers(); i++) {
        const char* driver_name = SDL_GetAudioDriver(i);
        if (SDL_strcmp(driver_name, "disk") == 0 || SDL_strcmp(driver_name, "dummy") == 0) {
            // Skip "write-to-disk" and dummy drivers
            continue;
        }

        if (0 == SDL_AudioInit(driver_name)
            && 0 == Mix_OpenAudio(AUDIO_RATE, AUDIO_FORMAT, AUDIO_CHANNELS, AUDIO_BUFFERS)) {
            logs::info("Using audio driver: %s", driver_name);
            init_channels();
            return;
        } else {
            logs::info("Not using audio driver %s, reason: %s", driver_name, SDL_GetError());
        }
    }

    logs::error("Sound failed to initialize: %s", Mix_GetError());
    int max = SDL_GetNumAudioDevices(0);
    logs::info("Number of audio devices: %d", max);

    for (int i = 0; i < max; i++) {
        logs::info("Audio device: %s", SDL_GetAudioDeviceName(i, 0));
    }
}

void sound_device_close(void) {
    auto &data = g_sound_device_data;
    if (data.initialized) {
        for (int i = 0; i < MAX_CHANNELS; i++) {
            sound_device_stop_channel(i);
        }
        Mix_CloseAudio();
        data.initialized = 0;
    }
}

void sound_device_load_formats() {
    auto &data = g_sound_device_data;
    if (!data.initialized) {
        return;
    }

    const int format_desc_max_chars = 4;
    const music_format formats[] = {{MIX_INIT_FLAC, "FLAC"},
                                    {MIX_INIT_MOD, "MOD"},
                                    {MIX_INIT_MP3, "MP3"},
                                    {MIX_INIT_OGG, "OGG"},
                                    {MIX_INIT_MID, "MIDI"},
                                    {MIX_INIT_OPUS, "Opus"}};

    const int max_num_formats = sizeof(formats) / sizeof(formats[0]);

    int all_flags = 0;
    for (int i = 0; i < max_num_formats; ++i) {
        all_flags |= formats[i].flag;
    }

    const int initialized_flags = Mix_Init(all_flags);
    if (initialized_flags == 0) {
        logs::error("Could not load any music formats: %s", Mix_GetError());
    } else {
        const char* seperator = ", ";
        const int seperator_length = strlen(seperator);
        const int max_format_length = format_desc_max_chars + seperator_length; // desc + seperator
        std::vector<char> data(max_num_formats * max_format_length + 1);
        char* buf = data.data();

        int buf_pos = 0;
        for (int i = 0; i < max_num_formats; ++i) {
            if (initialized_flags & formats[i].flag) {
                int desc_length = strlen(formats[i].desc);
                if (desc_length > format_desc_max_chars) {
                    desc_length = format_desc_max_chars;
                }

                memcpy(buf + buf_pos, formats[i].desc, desc_length);
                buf_pos += desc_length;
                memcpy(buf + buf_pos, seperator, seperator_length);
                buf_pos += seperator_length;
            }
        }
        if (buf_pos >= seperator_length) {
            // remove last seperator
            buf_pos -= seperator_length;
        }
        buf[buf_pos] = 0;

        logs::info("music formats initialized: %s (%i)", buf, initialized_flags);
    }
}

void sound_device_unload_formats(void) {
    Mix_Quit();
}

int sound_device_is_channel_playing(int channel) {
    auto &data = g_sound_device_data;
    return data.channels[channel].chunk && Mix_Playing(channel);
}

void sound_device_set_music_volume(int volume_pct) {
    Mix_VolumeMusic(percentage_to_volume(volume_pct));
}

void sound_device_set_channel_volume(int channel, int volume_pct) {
    auto &data = g_sound_device_data;
    if (data.channels[channel].chunk) {
        Mix_VolumeChunk(data.channels[channel].chunk, percentage_to_volume(volume_pct));
    }
}

#ifdef __vita__
static void load_music_for_vita(const char* filename) {
    if (vita_music_data.buffer) {
        if (strcmp(filename, vita_music_data.filename) == 0)
            return;
        free(vita_music_data.buffer);
        vita_music_data.buffer = 0;
    }
    strncpy(vita_music_data.filename, filename, FILE_NAME_MAX - 1);
    char* resolved_filename = vita_prepend_path(filename);
    SceUID fd = sceIoOpen(resolved_filename, SCE_O_RDONLY, 0777);
    free(resolved_filename);
    if (fd < 0)
        return;
    vita_music_data.size = sceIoLseek(fd, 0, SCE_SEEK_END);
    sceIoLseek(fd, 0, SCE_SEEK_SET);
    vita_music_data.buffer = malloc(sizeof(char) * vita_music_data.size);
    sceIoRead(fd, vita_music_data.buffer, vita_music_data.size);
    sceIoClose(fd);
}
#endif

int sound_device_play_music(const char* filename, int volume_pct) {
    auto &data = g_sound_device_data;
    if (data.initialized) {
        sound_device_stop_music();
#ifdef __vita__
        load_music_for_vita(filename);
        if (!vita_music_data.buffer)
            return 0;

        SDL_RWops* sdl_music = SDL_RWFromMem(vita_music_data.buffer, vita_music_data.size);
        data.music = Mix_LoadMUSType_RW(sdl_music, vfs::file_has_extension(filename, "mp3") ? MUS_MP3 : MUS_WAV, SDL_TRUE);
#else
        data.music = Mix_LoadMUS(filename);
#endif
        if (!data.music) {
            logs::warn("Error opening music file '%s'. Reason: %s", filename, Mix_GetError());
        } else {
            if (Mix_PlayMusic(data.music, -1) == -1) {
                data.music = 0;
                logs::warn("Error playing music file '%s'. Reason: %s", filename, Mix_GetError());
            } else {
                sound_device_set_music_volume(volume_pct);
            }
        }
        return data.music ? 1 : 0;
    }
    return 0;
}

void sound_device_play_file_on_channel(const char* filename, int channel, int volume_pct) {
    auto &data = g_sound_device_data;
    if (!data.initialized) {
        return;
    }

    sound_device_stop_channel(channel);
    data.channels[channel].chunk = load_chunk(filename);
    if (data.channels[channel].chunk) {
        sound_device_set_channel_volume(channel, volume_pct);
        Mix_PlayChannel(channel, data.channels[channel].chunk, 0);
    }
}

void sound_device_play_channel(int channel, int volume_pct) {
    auto &data = g_sound_device_data;
    if (data.initialized) {
        sound_channel* ch = &data.channels[channel];
        if (load_channel(ch)) {
            const char* mp3_track;
            sound_device_set_channel_volume(channel, volume_pct * 0.4);

            Mix_PlayChannel(channel, ch->chunk, 0);
        }
    }
}

void sound_device_play_channel_panned(int channel, int volume_pct, int left_pct, int right_pct) {
    auto &data = g_sound_device_data;
    if (data.initialized) {
        sound_channel* ch = &data.channels[channel];
        if (load_channel(ch)) {
            Mix_SetPanning(channel, left_pct * 255 / 100, right_pct * 255 / 100);
            sound_device_set_channel_volume(channel, volume_pct);
            Mix_PlayChannel(channel, ch->chunk, 0);
        }
    }
}

void sound_device_stop_music(void) {
    auto &data = g_sound_device_data;
    if (data.initialized) {
        if (data.music) {
            Mix_HaltMusic();
            Mix_FreeMusic(data.music);
            data.music = 0;
        }
    }
}

void sound_device_stop_channel(int channel) {
    auto &data = g_sound_device_data;
    if (data.initialized) {
        sound_channel* ch = &data.channels[channel];
        if (ch->chunk) {
            Mix_HaltChannel(channel);
            Mix_FreeChunk(ch->chunk);
            ch->chunk = 0;
        }
    }
}

static void free_custom_audio_stream(void) {
    auto &custom_music = g_custom_music;
#ifdef USE_SDL_AUDIOSTREAM
    if (custom_music.use_audiostream) {
        if (custom_music.stream) {
            SDL_FreeAudioStream(custom_music.stream);
            custom_music.stream = 0;
        }
        return;
    }
#endif

    if (custom_music.buffer) {
        free(custom_music.buffer);
        custom_music.buffer = 0;
    }
}

static int create_custom_audio_stream(SDL_AudioFormat src_format, Uint8 src_channels, int src_rate, SDL_AudioFormat dst_format, Uint8 dst_channels, int dst_rate) {
    auto &custom_music = g_custom_music;
    free_custom_audio_stream();

#ifdef USE_SDL_AUDIOSTREAM
    if (custom_music.use_audiostream) {
        custom_music.stream = SDL_NewAudioStream(src_format, src_channels, src_rate, dst_format, dst_channels, dst_rate);
        return custom_music.stream != 0;
    }
#endif

    int result = SDL_BuildAudioCVT(&custom_music.cvt, src_format, src_channels, src_rate, dst_format, dst_channels, dst_rate);
    if (result < 0) {
        return 0;
    }

    // Allocate buffer large enough for 2 seconds of 16-bit audio
    custom_music.buffer_size = dst_rate * dst_channels * 2 * 2;
    custom_music.buffer = (unsigned char*)malloc(custom_music.buffer_size);
    if (!custom_music.buffer)
        return 0;

    custom_music.cur_read = 0;
    custom_music.cur_write = 0;
    return 1;
}

static int custom_audio_stream_active(void) {
    auto &custom_music = g_custom_music;
#ifdef USE_SDL_AUDIOSTREAM
    if (custom_music.use_audiostream) {
        return custom_music.stream != 0;
    }
#endif
    return custom_music.buffer != 0;
}

static int put_custom_audio_stream(Uint8* audio_data, int len) {
    auto &custom_music = g_custom_music;
    if (!audio_data || len <= 0 || !custom_audio_stream_active()) {
        return 0;
    }

#ifdef USE_SDL_AUDIOSTREAM
    if (custom_music.use_audiostream) {
        return SDL_AudioStreamPut(custom_music.stream, audio_data, len) == 0;
    }
#endif

    // Convert audio to SDL format
    custom_music.cvt.buf = (Uint8*)malloc((size_t)(len * custom_music.cvt.len_mult));
    if (!custom_music.cvt.buf) {
        return 0;
    }

    memcpy(custom_music.cvt.buf, audio_data, len);
    custom_music.cvt.len = len;
    SDL_ConvertAudio(&custom_music.cvt);
    int converted_len = custom_music.cvt.len_cvt;

    // Copy data to circular buffer
    if (converted_len + custom_music.cur_write <= custom_music.buffer_size) {
        memcpy(&custom_music.buffer[custom_music.cur_write], custom_music.cvt.buf, converted_len);
    } else {
        int end_len = custom_music.buffer_size - custom_music.cur_write;
        memcpy(&custom_music.buffer[custom_music.cur_write], custom_music.cvt.buf, end_len);
        memcpy(custom_music.buffer, &custom_music.cvt.buf[end_len], converted_len - end_len);
    }
    custom_music.cur_write = (custom_music.cur_write + converted_len) % custom_music.buffer_size;

    // Clean up
    free(custom_music.cvt.buf);
    custom_music.cvt.buf = 0;
    custom_music.cvt.len = 0;

    return 1;
}

static int get_custom_audio_stream(Uint8* dst, int len) {
    auto &custom_music = g_custom_music;
    if (!dst || len <= 0 || !custom_audio_stream_active()) {
        return 0;
    }

#ifdef USE_SDL_AUDIOSTREAM
    if (custom_music.use_audiostream) {
        return SDL_AudioStreamGet(custom_music.stream, dst, len);
    }
#endif

    int bytes_copied = 0;
    if (custom_music.cur_read < custom_music.cur_write) {
        int bytes_available = custom_music.cur_write - custom_music.cur_read;
        int bytes_to_copy = bytes_available < len ? bytes_available : len;
        memcpy(dst, &custom_music.buffer[custom_music.cur_read], bytes_to_copy);
        bytes_copied = bytes_to_copy;
    } else {
        int bytes_available = custom_music.buffer_size - custom_music.cur_read;
        int bytes_to_copy = bytes_available < len ? bytes_available : len;
        memcpy(dst, &custom_music.buffer[custom_music.cur_read], bytes_to_copy);
        bytes_copied = bytes_to_copy;
        if (bytes_copied < len) {
            int second_part_len = len - bytes_copied;
            bytes_available = custom_music.cur_write;
            bytes_to_copy = bytes_available < second_part_len ? bytes_available : second_part_len;
            memcpy(&dst[bytes_copied], custom_music.buffer, bytes_to_copy);
            bytes_copied += bytes_to_copy;
        }
    }
    custom_music.cur_read = (custom_music.cur_read + bytes_copied) % custom_music.buffer_size;

    return bytes_copied;
}

static void custom_music_callback(void* dummy, Uint8* stream, int len) {
    int bytes_copied = get_custom_audio_stream(stream, len);

    if (bytes_copied < len) {
        // end of stream, write silence
        memset(&stream[bytes_copied], 0, len - bytes_copied);
    }
}

void sound_device_use_custom_music_player(int bitdepth,
                                          int num_channels,
                                          int rate,
                                          const unsigned char* audio_data,
                                          int len) {
    auto &custom_music = g_custom_music;
    SDL_AudioFormat format;
    if (bitdepth == 8)
        format = AUDIO_U8;
    else if (bitdepth == 16)
        format = AUDIO_S16;
    else {
        logs::error("Custom music bitdepth not supported: %u", bitdepth);
        return;
    }
    int device_rate;
    Uint16 device_format;
    int device_channels;
    Mix_QuerySpec(&device_rate, &device_format, &device_channels);
    custom_music.format = format;

    int result = create_custom_audio_stream(format, num_channels, rate, device_format, device_channels, device_rate);
    if (!result)
        return;

    sound_device_write_custom_music_data(audio_data, len);

    Mix_HookMusic(custom_music_callback, 0);
}

void sound_device_write_custom_music_data(const unsigned char* audio_data, int len) {
    auto &custom_music = g_custom_music;
    if (!audio_data || len <= 0 || !custom_audio_stream_active())
        return;
    // Mix audio to sound effect volume
    Uint8* mix_buffer = (Uint8*)malloc(len);
    if (!mix_buffer)
        return;
    memset(mix_buffer, (custom_music.format == AUDIO_U8) ? 128 : 0, len);
    SDL_MixAudioFormat(mix_buffer, audio_data, custom_music.format, len, percentage_to_volume(setting_sound(SOUND_EFFECTS)->volume));

    put_custom_audio_stream(mix_buffer, len);
    free(mix_buffer);
}

void sound_device_use_default_music_player(void) {
    Mix_HookMusic(0, 0);
    free_custom_audio_stream();
}