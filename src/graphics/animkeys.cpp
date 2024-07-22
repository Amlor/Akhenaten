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
const xstring id_anim_t::fight = "fight";
const xstring id_anim_t::hunt = "hunt";
const xstring id_anim_t::pack = "pack";
const xstring id_anim_t::unpack = "unpack";
const xstring id_anim_t::move_pack = "move_pack";
const xstring id_anim_t::fight_fire = "fight_fire";
const xstring id_anim_t::back = "back";
const xstring id_anim_t::bubbles = "bubbles";
const xstring id_anim_t::point = "point";

const id_anim_t &animkeys() {
    return g_animkeys;
}