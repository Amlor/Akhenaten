var _format = function() {
    var formatted = arguments[0]
    for (var arg in arguments) {
                if(arg==0)
                    continue
        formatted = formatted.replace("{" + (arg-1) + "}", arguments[arg])
    }
    return formatted
};

var building_sounds = []
var building_info = []
var mission_sounds = []
var walker_sounds = []
var city_sounds = []
var overlays = []
var images = []
var cart_images = []
var cart_offsets = []
var sled_offsets = []
var localization = []
var top_menu_bar = {}
// buildings
var building_water_lift = {}
var building_weaponsmith = {}
var building_granary = {}
var building_academy = {}
var building_firehouse = {}
var building_police_station = {}
var building_cattle_ranch = {}
var building_fort = {}
var building_fort_charioteers = {}
var building_fort_infantry = {}
var building_fort_archers = {}
var building_fort_ground = {}
var building_courthouse = {}
var building_temple_osiris = {}
var building_temple_ra = {}
var building_temple_ptah = {}
var building_temple_seth = {}
var building_temple_bast = {}
var building_temple = {}
var building_mine_copper = {}
var building_mine_gems = {}
var building_physician = {}
var building_small_statue = {}
var building_medium_statue = {}
var building_large_statue = {}
var building_booth = {}
var building_bandstand = {}
var building_pavilion = {}
var building_festival_square = {}
var building_ferry = {}
var building_dentist = {}
var building_mortuary = {}
var building_dancer_school = {}
var building_conservatory = {}
var building_pottery = {}
var building_mine_gold = {}
var building_clay_pit = {}
var building_hunting_lodge = {}
var building_village_palace = {}
var building_town_palace = {}
var building_city_palace = {}
var building_roadblock = {}
var building_brick_gatehouse = {}
var building_mud_gatehouse = {}
var building_personal_mansion = {}
var building_dynasty_mansion = {}
var building_village_mansion = {}
var building_scribal_school = {}
var building_library = {}
var building_military_academy = {}
var building_brewery = {}
var building_juggler_school = {}
var building_bazaar = {}
var building_architect_post = {}
var building_apothecary = {}
var building_storage_yard = {}
var building_storage_room = {}
var building_work_camp = {}
var building_wood_cutter = {}
var building_water_supply = {}
var building_well = {}
var building_reed_gatherer = {}
var building_papyrus_maker = {}
var building_guild_stonemasons = {}
var building_bricklayers_guild = {}
var building_carpenters_guild = {}
var building_dock = {}
var building_small_mastaba = {}
var building_small_mastaba_part_side = {}
var building_small_mastaba_part_wall = {}
var building_small_mastaba_part_entrance = {}
var building_tax_collector = {}
var building_tax_collector_up = {}
var building_recruiter = {}
var building_shrine_osiris = {}
var building_shrine_ra = {}
var building_shrine_ptah = {}
var building_shrine_seth = {}
var building_shrine_bast = {}
var building_fishing_wharf = {}
var building_transport_wharf = {}
var building_warship_wharf = {}
var building_shipyard = {}
var building_bricks_workshop = {}
var building_chariots_workshop = {}
var building_paint_workshop = {}
var building_jewels_workshop = {}
var building_lamp_workshop = {}
var building_vacant_lot = {}
var building_senet_house = {}
var building_bullfight_school = {}
var building_sandstone_quarry = {}
var building_stone_quarry = {}
var building_limestone_quarry = {}
var building_granite_quarry = {}
var building_farm_grain = {}
var building_farm_chickpeas = {}
var building_farm_lettuce = {}
var building_farm_pomegranates = {}
var building_farm_barley = {}
var building_farm_flax = {}
var building_farm_henna = {}
var building_farm_figs = {}
var building_plaza = {}
var building_garden = {}
var building_weaver = {}
var building_brick_tower = {}
var building_mud_tower = {}
var building_road = {}
var building_burning_ruin = {}
var building_marble_quarry = {}
// houses
var building_house_crude_hut = {}
var building_house_sturdy_hut = {}
var building_house_meager_shanty = {}
var building_house_common_shanty = {}
var building_house_rough_cottage = {}
var building_house_ordinary_cottage = {}
var building_house_modest_homestead = {}
var building_house_spacious_homestead = {}
var building_house_modest_apartment = {}
var building_house_spacious_apartment = {}
var building_house_common_residence = {}
var building_house_spacious_residence = {}
var building_house_elegant_residence = {}
var building_house_fancy_residence = {}
var building_house_common_manor = {}
var building_house_spacious_manor = {}
var building_house_elegant_manor = {}
var building_house_stately_manor = {}
var building_house_modest_estate = {}
var building_house_palatial_estate = {}

// misc
var empire_window = {}
var empire_images_remap = []
var imagepaks = []

// ui
var advisor_chief_window = {}
var advisor_rating_window = {}
var mission_briefing_window = {}
var granary_info_window = {}
var advisor_religion_window = {}
var workshop_info_window = {}
var main_menu_window = {}
var bazaar_info_window = {}
var well_info_window = {}
var minimap_window = {}
var advisor_trade_window = {}

//figures
var figure_fireman = {}
var figure_water_carrier = {}
var figure_priest = {}
var figure_ostrich = {}
var figure_hippo = {}
var figure_immigrant = {}
var figure_worker = {}
var figure_architector = {}
var figure_market_buyer = {}
var figure_delivery_boy = {}
var figure_cartpusher = {}
var figure_storageyard_cart = {}
var figure_trade_ship = {}
var figure_sled = {}
var figure_tax_collector = {}
var figure_physician = {}
var figure_herbalist = {}
var figure_emigrant = {}
var figure_market_trader = {}
var figure_juggler = {}
var figure_dancer = {}
var figure_musician = {}
var figure_labor_seeker = {}
var figure_explosion = {}
var figure_reed_gatherer = {}
var figure_caravan_donkey = {}
var figure_trade_caravan = {}
var figure_homeless = {}
var figure_teacher = {}
var figure_docker = {}
var figure_bricklayer = {}
var figure_ostrich_hunter = {}
var figure_lumberjack = {}
var figure_magistrate = {}
var figure_soldier_infantry = {}
var figure_soldier_archer = {}
var figure_soldier_charioteer = {}
var figure_fishing_boat = {}
var figure_flotsam = {}
var figure_festival_guy = {}
var figure_scriber = {}
var figure_dentist = {}
var figure_fishing_point = {}
var figure_standard_bearer = {}
var figure_native_trader = {}
var figure_librarian = {}
var figure_constable = {}
var figure_shipwreck = {}
var figure_governor = {}
var figure_arrow = {}
var figure_hunter_arrow = {}
var figure_spear = {}
var figure_javelin = {}
var figure_bolt = {}
// missions
var mission0 = {}
var mission1 = {}
var mission2 = {}
var mission3 = {}
var mission4 = {}
var mission5 = {}
var mission6 = {}
