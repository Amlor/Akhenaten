#include "figure/figure.h"

class figure_immigrant : public figure_impl {
public:
    FIGURE_METAINFO(FIGURE_IMMIGRANT, figure_immigrant)
    figure_immigrant(figure *f) : figure_impl(f) {}

    virtual void on_create() override {}
    virtual void figure_action() override;
    virtual void figure_before_action() override;
    virtual void figure_roaming_action() override { /*nothing*/ }
    virtual void update_animation() override;
    virtual bool can_move_by_water() const override;
    virtual figure_immigrant *dcast_immigrant() { return this; }
    virtual int y_correction(int y) const override { return 10; }
    virtual e_figure_sound phrase() const override { return {FIGURE_IMMIGRANT, "immigrant"}; }
    virtual figure_sound_t get_sound_reaction(pcstr key) const override;
    virtual sound_key phrase_key() const override;
    virtual const animations_t &anim() const override;

    static void create(building *house, int num_people);

    building* immigrant_home();

    void set_immigrant_home(int _id) {
        base.immigrant_home_building_id = _id;
    };

    void set_immigrant_home(building* b) {
        base.immigrant_home_building_id = b->id;
    };
};