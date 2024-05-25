#include "sound_walker.h"

#include <map>
#include <string>

#include "js/js_game.h"

std::vector<figure_sound_t> g_walker_reaction;

ANK_REGISTER_CONFIG_ITERATOR(config_load_walker_sounds);
void config_load_walker_sounds() {
    g_walker_reaction.clear();

    g_config_arch.r_array("walker_sounds", [] (archive arch) {
        pcstr type = arch.r_string("type");
        pcstr sound = arch.r_string("sound");
        g_walker_reaction.push_back({type, sound});
    });
}

bstring64 snd::get_walker_reaction(pcstr reaction) {
    auto it = std::find_if(g_walker_reaction.begin(), g_walker_reaction.end(), [reaction] (auto &it) { return it.id.equals(reaction); });

    return (it == g_walker_reaction.end()) ? bstring64() : it->fname;
}

void figure_sound_t::load(archive arch) {
    fname = arch.r_string("sound");
    group = arch.r_int("group");
    text = arch.r_int("text");
    phrase = arch.r_string("phrase");
}

void figure_sounds_t::load(archive arch, pcstr section) {
    data.clear();
    arch.r_objects(section, [this](pcstr key, archive anim_arch) {
        data.push_back({});
        data.back().id = key;
        data.back().load(anim_arch);
    });
}
