#include "animkeys.h"

static id_anim_t g_animkeys;
const xstring id_anim_t::base = "base";
const xstring id_anim_t::preview = "preview";
const xstring id_anim_t::work = "work";
const xstring id_anim_t::none = "none";
const xstring id_anim_t::poof = "poof";
const xstring id_anim_t::walk = "walk";
const xstring id_anim_t::death = "death";
const xstring id_anim_t::swim = "swim";

const id_anim_t &animkeys() {
    return g_animkeys;
}