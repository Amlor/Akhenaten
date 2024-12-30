#pragma once

#include "figure/figure.h"

class figure_market_trader : public figure_impl {
public:
    FIGURE_METAINFO(FIGURE_MARKET_TRADER, figure_market_trader)
    figure_market_trader(figure *f) : figure_impl(f) {}

    virtual void on_create() override {}
    virtual void figure_action() override;
    virtual figure_phrase_t phrase() const override { return {FIGURE_MARKET_TRADER, "mkt_seller"}; }
    virtual figure_sound_t get_sound_reaction(xstring key) const override;
    virtual e_overlay get_overlay() const override { return OVERLAY_BAZAAR_ACCESS; }
    virtual sound_key phrase_key() const override;
    virtual int provide_service() override;
};