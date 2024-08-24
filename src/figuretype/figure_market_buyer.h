#pragma once

#include "figure/figure.h"

class figure_market_buyer : public figure_impl {
public:
    FIGURE_METAINFO(FIGURE_MARKET_BUYER, figure_market_buyer)
    figure_market_buyer(figure *f) : figure_impl(f) {}

    virtual void on_create() override {}
    virtual void figure_before_action() override;
    virtual void figure_action() override;
    virtual e_figure_sound phrase() const override { return  {FIGURE_MARKET_BUYER, "mkt_buyer"}; }
    virtual e_overlay get_overlay() const override { return OVERLAY_BAZAAR_ACCESS; }
    virtual sound_key phrase_key() const override;
    virtual int provide_service() override;
    virtual figure_sound_t get_sound_reaction(xstring key) const override;
    virtual bool window_info_background(object_info &ctx) override;
    virtual const animations_t &anim() const override;

    bool take_resource_from_storageyard(building *warehouse);
    int take_food_from_granary(building *market, building *granary);
    int create_delivery_boy(int leader_id);
};

int provide_market_goods(building *market, tile2i tile);
