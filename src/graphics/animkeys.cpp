#include "animkeys.h"

static id_anim_t g_animkeys;
const xstring id_anim_t::base = "base";
const xstring id_anim_t::base_work = "base_work";
const xstring id_anim_t::fancy = "fancy";
const xstring id_anim_t::fancy_work = "fancy_work";
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
const xstring id_anim_t::work_n = "work_n";
const xstring id_anim_t::work_w = "work_w";
const xstring id_anim_t::work_s = "work_s";
const xstring id_anim_t::work_e = "work_e";
const xstring id_anim_t::wait_n = "wait_n";
const xstring id_anim_t::wait_w = "wait_w";
const xstring id_anim_t::wait_s = "wait_s";
const xstring id_anim_t::wait_e = "wait_e";
const xstring id_anim_t::eating = "eating";
const xstring id_anim_t::idle = "idle";
const xstring id_anim_t::booth = "booth";
const xstring id_anim_t::stand_sn_s = "stand_sn_s";
const xstring id_anim_t::stand_sn_n = "stand_sn_n";
const xstring id_anim_t::stand_sn_e = "stand_sn_e";
const xstring id_anim_t::dancer = "dancer";
const xstring id_anim_t::musician_sn = "musician_sn";
const xstring id_anim_t::musician_we = "musician_we";
const xstring id_anim_t::attack = "attack";
const xstring id_anim_t::farmland = "farmland";
const xstring id_anim_t::square = "square";
const xstring id_anim_t::juggler = "juggler";
const xstring id_anim_t::barley = "barley";
const xstring id_anim_t::wood = "wood";
const xstring id_anim_t::work_warship = "work_warship";
const xstring id_anim_t::work_fishing_boat = "work_fishing_boat";
const xstring id_anim_t::work_transport = "work_transport";
const xstring id_anim_t::flax = "flax";
const xstring id_anim_t::clay = "clay";

const id_anim_t &animkeys() {
    return g_animkeys;
}