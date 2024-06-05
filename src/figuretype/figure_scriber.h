#pragma once

#include "figure/figure.h"

class figure_scriber : public figure_impl {
public:
    FIGURE_METAINFO(FIGURE_SCRIBER, figure_scriber)
    figure_scriber(figure *f) : figure_impl(f) {}

    virtual void on_create() override {}
    virtual void figure_action() override;
    virtual e_figure_sound phrase() const override { return {FIGURE_SCRIBER, "scribe"}; }
    //virtual sound_key phrase_key() const override;
    virtual int provide_service() override;
    virtual e_overlay get_overlay() const override { return OVERLAY_SCRIBAL_SCHOOL; }
    virtual const animations_t &anim() const override;
    //virtual figure_sound_t get_sound_reaction(pcstr key) const override;
};