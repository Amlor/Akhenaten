#include "building/building_house.h"

#include "building/model.h"
#include "window/building/common.h"
#include "city/city.h"
#include "core/calc.h"
#include "core/game_environment.h"
#include "game/resource.h"
#include "graphics/image.h"
#include "graphics/graphics.h"
#include "graphics/view/view.h"
#include "graphics/elements/lang_text.h"
#include "graphics/elements/panel.h"
#include "graphics/text.h"
#include "grid/road_access.h"
#include "window/building/figures.h"
#include "window/window_building_info.h"
#include "window/window_figure_info.h"
#include "io/gamefiles/lang.h"
#include "game/game.h"

struct info_window_house : public building_info_window {
    int resource_text_group;
    int help_id;

    using building_info_window::load;
    virtual void load(archive arch, pcstr section) override {
        common_info_window::load(arch, section);

        resource_text_group = arch.r_int("resource_text_group");
        help_id = arch.r_int("help_id");
    }

    virtual void window_info_background(object_info& c) override;
    virtual bool check(object_info &c) override {
        building_house *h = building_get(c.building_id)->dcast_house();
        if (!h) {
            return false;
        }

        return !h->is_vacant_lot();
    }
};

struct info_window_vacant_lot : building_info_window {
    virtual void window_info_background(object_info &c) override;
    virtual bool check(object_info &c) override {
        building_house *h = building_get(c.building_id)->dcast_house();
        if (!h) {
            return false;
        }

        return h->is_vacant_lot();
    }
};

info_window_house house_infow;
info_window_vacant_lot vacant_lot_infow;

ANK_REGISTER_CONFIG_ITERATOR(config_load_house_info_window);
void config_load_house_info_window() {
    house_infow.load("info_window_house");
    vacant_lot_infow.load("info_window_vacant_lot");
}

void info_window_vacant_lot::window_info_background(object_info &c) {
    building_info_window::window_info_background(c);

    //window_figure_info_prepare_figures(c);
    //window_building_draw_figure_list(&c);

    building *b = building_get(c.building_id);
    map_point road_tile = map_closest_road_within_radius(b->tile, 1, 2);
    int text_id = road_tile.valid() ? 1 : 2;

    ui["title"] = ui::str(128, 0);
    ui["describe"] = ui::str(128, text_id);
}

void info_window_house::window_info_background(object_info &c) {
    building_info_window::window_info_background(c);

    c.help_id = help_id;
    building *b = building_get(c.building_id);

    assert(b->house_population > 0);

    int level = b->type - 10;
    ui["title"] = ui::str(29, level);

    if (b->data.house.evolve_text_id == 62) { // is about to devolve
        bstring512 text;
        text.printf("%s @Y%s&) %s",
            ui::str(127, 40 + b->data.house.evolve_text_id),
            ui::str(41, building_get(c.worst_desirability_building_id)->type),
            ui::str(127, 41 + b->data.house.evolve_text_id));
        ui["evolve_reason"] = text;
    } else { // needs something to evolve 
        ui["evolve_reason"] = ui::str(127, 40 + b->data.house.evolve_text_id);
    }

    int resource_image = image_id_resource_icon(0);

    auto food_icon = [] (int i) { bstring32 id_icon; id_icon.printf("food%u_icon", i); return id_icon; };
    auto food_text = [] (int i) { bstring32 id_text; id_text.printf("food%u_text", i); return id_text; };

    for (int i = 0; i < 4; ++i) {
        e_resource resource = g_city.allowed_foods(i);
        int stored = b->data.house.foods[i];
        ui[food_icon(i)].image(resource);
        ui[food_text(i)].text_var(resource ? "%u" : "", stored);
    }

    auto good_icon = [] (int i) { bstring32 id_icon; id_icon.printf("good%u_icon", i); return id_icon; };
    auto good_text = [] (int i) { bstring32 id_text; id_text.printf("good%u_text", i); return id_text; };

    // goods inventory
    e_resource house_goods[] = { RESOURCE_POTTERY, RESOURCE_LUXURY_GOODS, RESOURCE_LINEN, RESOURCE_BEER };
    for (int i = 0; i < 4; ++i) {
        e_resource resource = house_goods[i];
        int stored = b->data.house.inventory[INVENTORY_GOOD1 + i];
        ui[good_icon(i)].image(resource);
        ui[good_text(i)].text_var("%u", stored);
    }

    bstring256 people_text, adv_people_text;
    people_text.printf("%u %s", b->house_population, ui::str(127, 20));
    if (b->house_population_room < 0) {
        adv_people_text.printf("%u %s", -b->house_population_room, ui::str(127, 21));
    } else if (b->house_population_room > 0) {
        adv_people_text.printf("%s %u", ui::str(127, 22), b->house_population_room);
    }
    ui["people_text"].text_var("%s ( %s )", people_text, adv_people_text);

    bstring256 tax_info_text;
    if (b->house_tax_coverage) {
        int pct = calc_adjust_with_percentage(b->tax_income_or_storage / 2, city_finance_tax_percentage());
        tax_info_text.printf("%s %u %s", ui::str(127, 24), pct, ui::str(127, 25));
    } else {
        tax_info_text = ui::str(127, 23);
    }
    ui["tax_info"] = tax_info_text;

    int happiness = b->sentiment.house_happiness;
    int happiness_text_id;
    if (happiness >= 50) { happiness_text_id = 26; } else if (happiness >= 40) { happiness_text_id = 27; } else if (happiness >= 30) { happiness_text_id = 28; } else if (happiness >= 20) { happiness_text_id = 29; } else if (happiness >= 10) { happiness_text_id = 30; } else if (happiness >= 1) { happiness_text_id = 31; } else { happiness_text_id = 32; }

    ui["happiness_info"] = ui::str(127, happiness_text_id);

    if (!model_get_house(b->subtype.house_level)->food_types) { // no foods
        ui["additional_info"] = ui::str(127, 33);
    }
}