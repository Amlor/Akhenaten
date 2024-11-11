#include "game.h"

#include "building/construction/build_planner.h"
#include "building/model.h"
#include "building/building_barracks.h"
#include "building/building_granary.h"
#include "core/game_environment.h"
#include "core/random.h"
#include "core/profiler.h"
#include "core/log.h"
#include "grid/vegetation.h"
#include "grid/trees.h"
#include "grid/canals.h"
#include "grid/natives.h"
#include "editor/editor.h"
#include "game/file_editor.h"
#include "game/settings.h"
#include "game/state.h"
#include "game/time.h"
#include "game/tutorial.h"
#include "graphics/elements/warning.h"
#include "graphics/font.h"
#include "graphics/image.h"
#include "graphics/video.h"
#include "graphics/window.h"
#include "input/scroll.h"
#include "config/config.h"
#include "config/hotkeys.h"
#include "io/gamefiles/lang.h"
#include "content/content.h"
#include "mission.h"
#include "figure/formation.h"
#include "scenario/events.h"
#include "scenario/scenario.h"
#include "sound/sound_city.h"
#include "sound/sound.h"
#include "translation/translation.h"
#include "window/window_city.h"
#include "window/editor/map.h"
#include "window/logo.h"
#include "window/main_menu.h"
#include "graphics/view/view.h"
#include "platform/renderer.h"
#include "io/movie_writer.h"
#include "graphics/screen.h"
#include "widget/widget_minimap.h"
#include "city/city.h"
#include "city/trade.h"
#include "city/sentiment.h"
#include "city/city_floods.h"
#include "city/population.h"
#include "city/message.h"
#include "building/maintenance.h"
#include "building/industry.h"
#include "io/gamestate/boilerplate.h"
#include "scenario/distant_battle.h"
#include "scenario/empire.h"
#include "empire/empire.h"
#include "window/file_dialog.h"
#include "grid/routing/routing_terrain.h"
#include "grid/tiles.h"
#include "undo.h"

#include "dev/debug.h"
#include <iostream>

game_t game;
game_time_t game_time;

declare_console_ref_int16(gameyear, game_time.year)
declare_console_ref_int16(gamemonth, game_time.month)
declare_console_command_p(nextyear, game_cheat_next_year)
void game_cheat_next_year(std::istream &is, std::ostream &os) {
    game_time.advance_year();
    game.advance_year();
}

namespace {
    static const time_millis MILLIS_PER_TICK_PER_SPEED[] = {0, 20, 35, 55, 80, 110, 160, 240, 350, 500, 700};
    static time_millis last_update;
}

const game_time_t& gametime() {
    return game_time;
}

void game_t::animation_timers_init() {
    for (auto &timer: animation_timers) {
        timer.last_update = 0;
        timer.should_update = false;
    }
}

void game_t::animation_timers_update() {
    time_millis now_millis = time_get_millis();
    for (auto &timer : animation_timers) {
        timer.should_update = false;
    }

    unsigned int delay_millis = 0;
    for (auto &timer: animation_timers) {
        if (now_millis - timer.last_update >= delay_millis) {
            timer.should_update = true;
            timer.last_update = now_millis;
        }
        delay_millis += 20;
    }
}

bool game_t::animation_should_advance(uint32_t speed) {
    if (!animation || paused) {
        return false;
    }

    if (speed >= MAX_ANIM_TIMERS) {
        return false;
    }

    return animation_timers[speed].should_update;
}

void game_t::update_impl(int ticks) {
    OZZY_PROFILER_SECTION("Game/Update/Impl");
    if (editor_is_active()) {
        random_generate_next(); // update random to randomize native huts
        g_city.figures.update(); // just update the flag figures
        return;
    }

    random_generate_next();
    game_undo_reduce_time_available();

    g_tutorials_flags.update_starting_message();
    floodplains_tick_update(false);

    update_city(ticks);

    if (game_time.advance_tick()) {
        advance_day();
    }

    g_city.figures.update();

    g_scenario_data.update();
    g_city.victory_check();
}

void game_t::update_city(int ticks) {
    g_city.buildings.update_tick(game.paused);

    switch (gametime().tick) {
    case 1:
        g_city.religion.update();
        g_city.coverage_update();
        break;
    case 2:
        g_sound.music_update(false);
        break;
    case 3:
        widget_minimap_invalidate();
        break;
    case 4:
        g_city.kingdome.update();
        break;
    case 5:
        formation_update_all(false);
        break;
    case 6:
        map_natives_check_land();
        break;
    case 7:
        g_city.map.update_road_network();
        break;
    case 8:
        city_granaries_calculate_stocks();
        break;
    case 9:
        // nothing yet
    case 10:
        building_update_highest_id();
        break;
    case 12:
        g_city.house_service_decay_houses_covered();
        break;
    case 16:
        city_resource_calculate_storageyard_stocks();
        break;
    case 17:
        city_resource_calculate_food_stocks_and_supply_wheat();
        break;
    case 18:
        map_vegetation_growth_update();
        map_tree_growth_update();
        break;
    case 19:
        g_city.buildings_update_open_water_access();
        break;
    case 20:
        building_industry_update_production();
        break;
    case 21:
        building_maintenance_check_kingdome_access();
        break;
    case 22:
        g_city.house_population_update_room();
        break;
    case 23:
        g_city.migration_update();
        g_city.house_population_update_migration();
        break;
    case 24:
        g_city.house_population_evict_overcrowded();
        break;
    case 25:
        g_city.labor.update();
        break;
    case 27:
        g_city.buildings.update_wells_range();
        g_city.buildings.update_canals_from_water_lifts();
        map_update_canals();
        break;
    case 28:
        g_city.buildings.update_water_supply_houses();
        g_city.buildings.update_religion_supply_houses();
        break;
    case 29:
        formation_update_all(true);
        break;
    case 30:
        widget_minimap_invalidate();
        break;
    case 31:
        building_barracks_decay_tower_sentry_request();
        g_city.buildings_generate_figure();
        break;
    case 32:
        city_trade_update();
        break;
    case 33:
        g_city.buildings.update_counters();
        g_city.avg_coverage.update();
        g_city.health.update_coverage();
        building_industry_update_farms();
        break;
    case 34:
        g_city.government_distribute_treasury();
        break;
    case 35:
        g_city.house_service_decay_services();
        g_city.house_service_update_health();
        break;
    case 36:
        g_city.house_service_calculate_culture_aggregates();
        break;
    case 37:
        g_desirability.update();
        break;
    case 38:
        building_update_desirability();
        break;
    case 39:
        g_city.house_process_evolve_and_consume_goods();
        break;
    case 40:
        building_update_state();
        break;
    case 43:
        building_maintenance_update_burning_ruins();
        break;
    case 44:
        building_maintenance_check_fire_collapse();
        break;
    case 45:
        g_city.figures_generate_criminals();
        break;
    case 46:
        building_industry_update_wheat_production();
        break;
    case 48:
        g_city.house_service_decay_tax_collector();
        break;
    case 49:
        g_city.avg_coverage.update();
        g_city.festival.calculate_costs();
        break;
    case 50:
        break;
    }
}

void game_t::advance_year() {
    scenario_empire_process_expansion();
    game_undo_disable();
    game_time.advance_year();
    city_population_request_yearly_update();
    city_finance_handle_year_change();
    g_city.migration_advance_year();
    g_empire.reset_yearly_trade_amounts();
    building_maintenance_update_fire_direction();
    g_city.ratings_update(/*yearly_update*/true);
    //    city_gods_reset_yearly_blessings();
}

void game_t::advance_month() {
    g_city.migration_reset_newcomers();
    g_city.health.update();
    city_finance_handle_month_change();
    city_resource_consume_food();
    scenario_distant_battle_process();

    random_generate_next();                  // TODO: find out the source / reason for this
    g_scenario_data.events.process_random_events();
    g_scenario_data.events.process_events();

    g_city.victory_state.update_months_to_govern();
    g_city.update_allowed_foods();
    formation_update_monthly_morale_at_rest();
    city_message_decrease_delays();

    map_tiles_update_all_roads();
    //    map_tiles_river_refresh_entire();
    map_routing_update_land_citizen();
    //    city_message_sort_and_compact();

    if (game_time.advance_month()) {
        advance_year();
    } else {
        g_city.ratings_update(/*yearly_update*/false);
    }

    city_population_record_monthly();
    g_city.festival.update();
    g_city.buildings.update_month();

    if (g_city.can_produce_resource(RESOURCE_FISH)) {
        g_city.fishing_points.update();
    }

    if (g_settings.monthly_autosave) {
        bstring256 autosave_file("autosave_month.", saved_game_data_expanded.extension);
        GamestateIO::write_savegame(autosave_file);
    }
}

void game_t::advance_day() {
    OZZY_PROFILER_SECTION("Game/Run/Tick/Advance Day");
    //    map_advance_floodplain_growth();

    if (game_time.advance_day()) {
        advance_month();
    }

    city_sentiment_update_day();
    city_criminals_update_day();
    city_plague_update_day();
    g_city.environment.river_update_flotsam();
    g_city.buildings.update_day();
    g_city.figures_update_day();

    tutorial_on_day_tick();
}

void game_t::shutdown() {
    free(frame_pixels);
}

void game_t::set_write_video(bool v) {
    if (!write_video && v) {
        assert(!mvwriter);
        mvwriter = new MovieWriter("test.mp4", screen_width(), screen_height(), 4);
    } else if (write_video && !v) {
        assert(mvwriter);
        delete mvwriter;
        mvwriter = nullptr;
    }
    write_video = v;
}

void game_t::write_frame() {
    if (!write_video) {
        return;
    }

    if (!mvwriter) {
        return;
    }

    last_frame_tick++;
    if (last_frame_tick % 30 != 0) {
        return;
    }
    last_frame_tick = 0;
    
    ::painter ctx = this->painter();
    if (!frame_pixels) {
        frame_pixels = (color*)malloc(sizeof(color) * screen_width() * screen_height());
    }

    if (!graphics_renderer()->save_screen_buffer(ctx, frame_pixels, 0, 0, screen_width(), screen_height(), screen_width())) {
        free(frame_pixels);
        return;
    }

    mvwriter->addFrame((const uint8_t*)frame_pixels);
}

::painter game_t::painter() {
    ::painter ctx;
    ctx.view = &city_view_data_unsafe();
    ctx.renderer = graphics_renderer()->renderer();
    ctx.global_render_scale = graphics_renderer()->scale();

    return ctx;
}


static int is_unpatched(void) {
    const uint8_t* delete_game = lang_get_string(1, 6);
    const uint8_t* option_menu = lang_get_string(2, 0);
    const uint8_t* difficulty_option = lang_get_string(2, 6);
    const uint8_t* help_menu = lang_get_string(3, 0);
    // Without patch, the difficulty option string does not exist and
    // getting it "falls through" to the next text group, or, for some
    // languages (pt_BR): delete game falls through to option menu
    return difficulty_option == help_menu || delete_game == option_menu;
}

static encoding_type update_encoding(void) {
    int language = locale_determine_language();
    encoding_type encoding = encoding_determine(language);
    logs::info("Detected encoding: %i", encoding);
    font_set_encoding(encoding);
    translation_load(language);
    return encoding;
}

static bool reload_language(int is_editor, int reload_images) {
    if (!lang_load(is_editor)) {
        if (is_editor)
            logs::error("'c3_map.eng' or 'c3_map_mm.eng' files not found or too large.");
        else
            logs::error("'c3.eng' or 'c3_mm.eng' files not found or too large.");
        return false;
    }
    encoding_type encoding = update_encoding();

    if (!image_set_font_pak(encoding)) {
        logs::error("unable to load font graphics");
        return false;
    }

    return true;
}

static int get_elapsed_ticks() {
    if (game.paused || !city_has_loaded) {
        return 0;
    }

    int game_speed_index = 0;
    int ticks_per_frame = 1;
    switch (window_get_id()) {
    default:
        return 0;

    case WINDOW_CITY:
    case WINDOW_CITY_MILITARY:
    case WINDOW_SLIDING_SIDEBAR:
    case WINDOW_OVERLAY_MENU:
    case WINDOW_BUILD_MENU:
        game_speed_index = (100 - g_settings.game_speed) / 10;
        if (game_speed_index >= 10) {
            return 0;
        } else if (game_speed_index < 0) {
            ticks_per_frame = g_settings.game_speed / 100;
            game_speed_index = 0;
        }
        break;

    case WINDOW_EDITOR_MAP:
        game_speed_index = 3; // 70%, nice speed for flag animations
        break;
    }

    if (Planner.in_progress) {
        return 0;
    }

    if (scroll_in_progress() && !scroll_is_smooth()) {
        return 0;
    }

    time_millis now = time_get_millis();
    time_millis diff = now - last_update;
    if (diff < MILLIS_PER_TICK_PER_SPEED[game_speed_index] + 2) {
        return 0;
    }

    last_update = now;
    return ticks_per_frame;
}

bool game_t::check_valid() {
    vfs::content_cache_paths();

    if (!lang_load(0)) {
        return false;
    }

    logs::switch_output(vfs::platform_file_manager_get_base_path());
    update_encoding();
    g_settings.load(); // c3.inf
    config_load();   // akhenaten.ini
    hotkey_config_load();
    scenario_settings_init();
    random_init();

    paused = false;
    mt.reset(4);
    mtrpc.reset(4);

    return true;
}

bool game_init() {
    int missing_fonts = 0;
    if (!image_set_font_pak(encoding_get())) {
        logs::error("unable to load font graphics");
        if (encoding_get() == ENCODING_KOREAN)
            missing_fonts = 1;
        else
            return false;
    }

    if (!image_load_paks()) {
        logs::error("unable to load main graphics");
        return false;
    }

    if (!model_load()) {
        logs::error("unable to load model data");
        return false;
    }

    if (!g_scenario_data.events.msg_load()) {
        logs::error("unable to load eventmsg.txt");
        return false;
    }

    if (!g_scenario_data.events.msg_auto_phrases_load()) {
        logs::error("unable to load event auto reason phrases");
        return false;
    }

    if (!game_load_campaign_file()) {
        logs::error("unable to load campaign data");
        return false;
    }

    //    mods_init();
    g_sound.init();
    game_state_init();
    window_logo_show(missing_fonts ? MESSAGE_MISSING_FONTS : (is_unpatched() ? MESSAGE_MISSING_PATCH : MESSAGE_NONE));

    return true;
}

bool game_init_editor() {
    if (!reload_language(1, 0))
        return false;

    game_file_editor_clear_data();
    game_file_editor_create_scenario(2);

    if (city_view_is_sidebar_collapsed()) {
        city_view_toggle_sidebar();
    }

    editor_set_active(1);
    window_editor_map_show();
    return true;
}

void game_exit_editor() {
    if (!reload_language(0, 0))
        return;
    editor_set_active(0);
    window_main_menu_show(1);
}
int game_reload_language() {
    return reload_language(0, 1);
}

void game_t::update() {
    OZZY_PROFILER_SECTION("Game/Update");
    animation_timers_update();

    int num_ticks = get_elapsed_ticks();
    for (int i = 0; i < num_ticks; i++) {
        update_impl(1);

        //if (window_is_invalid()) {
        //    break;
        //}
    }

    if (window_is(WINDOW_CITY)) {
        anti_scum_random_15bit();
    }
}

void game_t::frame_begin() {
    OZZY_PROFILER_SECTION("Render/Frame");
    frame++;
    window_draw(false);
}

void game_t::frame_end() {
    animation = true;
}

void game_t::time_init(int year) {
    game_time.init(year);
}

void game_t::sound_frame_begin() {
    OZZY_PROFILER_SECTION("Sound/Frame");
    sound_city_play();
}

void game_handle_input_frame() {
    OZZY_PROFILER_SECTION("Input/Frame/Current");
    const mouse *m = mouse_get();
    const hotkeys *h = hotkey_state();

    window_type* w = window_current();
    w->handle_input(m, h);
    tooltip_handle(m, w->get_tooltip);
}

void game_draw_frame_warning() {
    OZZY_PROFILER_SECTION("Render/Frame/Warning");
    warning_draw();
}

void game_handle_input_after() {
    OZZY_PROFILER_SECTION("Input/Frame/After");
    window_update_input_after();
}

void game_exit() {
    video_shutdown();
    g_settings.save();
    config_save();
    g_sound.shutdown();
}
