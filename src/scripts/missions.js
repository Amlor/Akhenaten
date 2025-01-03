log_info("akhenaten: missions started")

mission0 = { // Nubt
	start_message : 150, //TUTORIAL_HOUSING_AND_ROADS, 248 = 150 + 99 - 1
	city_has_animals : true,
	gods_least_mood : 50,
	religion_enabled : false,
	player_rank : 0,
	stages : {
		tutorial_fire: [BUILDING_FIREHOUSE],
		tutorial_food: [BUILDING_HUNTING_LODGE, BUILDING_GRANARY, BUILDING_BAZAAR],
		tutorial_water: [BUILDING_WATER_SUPPLY],
		tutorial_collapse: [BUILDING_ARCHITECT_POST],
	}
}

mission1 = {
	start_message : 241 - 99, //MESSAGE_TUTORIAL_GOLD_AND_CRIME
	city_has_animals : true,
	gods_least_mood : 50,
	player_rank : 0,
	buildings : [],
	stages : {
		tutorial_start: [BUILDING_FIREHOUSE, BUILDING_ARCHITECT_POST, BUILDING_POLICE_STATION, BUILDING_BAZAAR, BUILDING_GRANARY, BUILDING_WATER_SUPPLY,
						 BUILDING_GOLD_MINE, BUILDING_VILLAGE_PALACE, BUILDING_HUNTING_LODGE],
		tutorial_gods : [BUILDING_TEMPLE_BAST, BUILDING_SHRINE_BAST, BUILDING_FESTIVAL_SQUARE],
		tutorial_entertainment : [BUILDING_BOOTH, BUILDING_JUGGLER_SCHOOL]
	}
}

mission2 = {
	start_message : 242 - 99,
	city_has_animals : true,
	gods_least_mood : 50,
	player_rank : 0,
	buildings : [],
	stages : {
		tutorial_start: [BUILDING_FIREHOUSE, BUILDING_ARCHITECT_POST, BUILDING_POLICE_STATION, BUILDING_VILLAGE_PALACE, BUILDING_WATER_SUPPLY,
						 BUILDING_FIGS_FARM, BUILDING_WORK_CAMP, BUILDING_BOOTH, BUILDING_JUGGLER_SCHOOL, BUILDING_BAZAAR, BUILDING_GRANARY,
						 BUILDING_TEMPLE_OSIRIS, BUILDING_SHRINE_OSIRIS, BUILDING_FESTIVAL_SQUARE],
		tutorial_industry: [BUILDING_CLAY_PIT, BUILDING_POTTERY_WORKSHOP, BUILDING_STORAGE_YARD],
		tutorial_health: [BUILDING_WATER_SUPPLY, BUILDING_APOTHECARY, BUILDING_PHYSICIAN],
		tutorial_gardens: [BUILDING_ROADBLOCK, BUILDING_FERRY, BUILDING_SMALL_STATUE, BUILDING_MEDIUM_STATUE, BUILDING_LARGE_STATUE, BUILDING_GARDENS, BUILDING_PLAZA],
	}
}

mission3 = {
	start_message : 241 - 99,
	city_has_animals : true,
	player_rank : 0,
	buildings : [],
	stages : {
		tutorial_start: [BUILDING_ROADBLOCK, BUILDING_FIREHOUSE, BUILDING_ARCHITECT_POST, BUILDING_POLICE_STATION, BUILDING_VILLAGE_PALACE,
						 BUILDING_WATER_SUPPLY, BUILDING_APOTHECARY, BUILDING_PHYSICIAN, BUILDING_GRAIN_FARM, BUILDING_BARLEY_FARM, BUILDING_WORK_CAMP,
						 BUILDING_BOOTH, BUILDING_JUGGLER_SCHOOL, BUILDING_BANDSTAND, BUILDING_CONSERVATORY, BUILDING_BAZAAR, BUILDING_GRANARY, BUILDING_FERRY,
						 BUILDING_CLAY_PIT, BUILDING_POTTERY_WORKSHOP, BUILDING_STORAGE_YARD,
						 BUILDING_TEMPLE_OSIRIS, BUILDING_SHRINE_OSIRIS, BUILDING_TEMPLE_RA, BUILDING_SHRINE_RA, BUILDING_TEMPLE_BAST, BUILDING_SHRINE_BAST,
						 BUILDING_BREWERY_WORKSHOP],
		tutorial_finance: [BUILDING_TAX_COLLECTOR, BUILDING_PERSONAL_MANSION]
	}
}

mission4 = {
	start_message : 241 - 99,
	city_has_animals : true,
	player_rank : 1,
	buildings : [],
	stages : {
		tutorial_start: [BUILDING_SMALL_STATUE, BUILDING_MEDIUM_STATUE, BUILDING_LARGE_STATUE, BUILDING_GARDENS, BUILDING_PLAZA,
						 BUILDING_ROADBLOCK, BUILDING_FIREHOUSE, BUILDING_ARCHITECT_POST, BUILDING_POLICE_STATION, BUILDING_VILLAGE_PALACE,
						 BUILDING_TAX_COLLECTOR, BUILDING_COURTHOUSE, BUILDING_PERSONAL_MANSION,
						 BUILDING_WATER_SUPPLY, BUILDING_APOTHECARY, BUILDING_PHYSICIAN,
						 BUILDING_BOOTH, BUILDING_JUGGLER_SCHOOL, BUILDING_BANDSTAND, BUILDING_CONSERVATORY,
						 BUILDING_TEMPLE_OSIRIS, BUILDING_SHRINE_OSIRIS, BUILDING_TEMPLE_PTAH, BUILDING_SHRINE_PTAH, BUILDING_TEMPLE_BAST, BUILDING_SHRINE_BAST,
						 BUILDING_CLAY_PIT, BUILDING_POTTERY_WORKSHOP, BUILDING_BREWERY_WORKSHOP, BUILDING_BAZAAR, BUILDING_GRANARY, BUILDING_STORAGE_YARD,
						 BUILDING_CHICKPEAS_FARM, BUILDING_BARLEY_FARM, BUILDING_WORK_CAMP ],
		tutorial_education: [BUILDING_REED_GATHERER, BUILDING_PAPYRUS_WORKSHOP, BUILDING_SCRIBAL_SCHOOL],
		tutorial_trading: [BUILDING_DOCK],
		tutorial_monuments: [BUILDING_BRICKLAYERS_GUILD, BUILDING_SMALL_MASTABA]
	}
}

mission5 = { // Timna
	start_message : 146, //TUTORIAL_SOLDIERS_AND_FORT, 245 = 146 + 99 - 1
	city_has_animals : true,
	player_rank : 1,
	stages : {
		tutorial_start: [BUILDING_SMALL_STATUE, BUILDING_MEDIUM_STATUE, BUILDING_LARGE_STATUE, BUILDING_GARDENS, BUILDING_PLAZA,
						 BUILDING_ROADBLOCK, BUILDING_FIREHOUSE, BUILDING_ARCHITECT_POST, BUILDING_POLICE_STATION, BUILDING_VILLAGE_PALACE,
						 BUILDING_TAX_COLLECTOR, BUILDING_COURTHOUSE, BUILDING_PERSONAL_MANSION,
						 BUILDING_WATER_SUPPLY, BUILDING_APOTHECARY, BUILDING_PHYSICIAN,
						 BUILDING_BOOTH, BUILDING_JUGGLER_SCHOOL, BUILDING_BANDSTAND, BUILDING_CONSERVATORY,
						 BUILDING_TAX_COLLECTOR, BUILDING_COURTHOUSE, BUILDING_PERSONAL_MANSION, BUILDING_BAZAAR, BUILDING_GRANARY, BUILDING_STORAGE_YARD,
						 BUILDING_RECRUITER, BUILDING_FORT_INFANTRY, BUILDING_FORT_ARCHERS, BUILDING_WEAPONSMITH,
						 BUILDING_SCRIBAL_SCHOOL, BUILDING_CLAY_PIT, BUILDING_GEMSTONE_MINE, BUILDING_GOLD_MINE, BUILDING_COPPER_MINE, BUILDING_POTTERY_WORKSHOP,
						 BUILDING_WEAVER_WORKSHOP, BUILDING_HUNTING_LODGE, BUILDING_TEMPLE_SETH, BUILDING_SHRINE_SETH,
						 BUILDING_BOOTH, BUILDING_JUGGLER_SCHOOL, BUILDING_BANDSTAND, BUILDING_CONSERVATORY, BUILDING_PAVILLION, BUILDING_DANCE_SCHOOL],
		tutorial_irrigation : [BUILDING_WATER_LIFT, BUILDING_IRRIGATION_DITCH, ],
		tutorial_guilds : [BUILDING_STORAGE_YARD, BUILDING_TAX_COLLECTOR, BUILDING_BOOTH, BUILDING_JUGGLER_SCHOOL]
	},
	enable_scenario_events : false,
	events : [
		{
			year : 2850,
			resource : "copper",
			amount : 500,
			deadline : 12,
		},
		{
			year : 2849,
			resource : "gems",
			amount : 1500,
			deadline : 12,
		},
		{
			year : 2848,
			resource : "deben",
			amount : [800, 1000],
			deadline : 12,
		}
	],

	attacks : [
		{
			year : 2848,
			type : "bedouin",
			amount : 4,
			pepeat_after : [6],
		}
	],

	gifts : [
		{
				from: "pharaoh",
				condition: {
					kingdome : 65,
					resource : RESOURCE_CHICKPEAS,
					amount : [1600, 2400],
				}
		}
	],

	trade_routes : [
		{
			city : "meninefer",
			reputation : 60,
		}
	]

}

mission6 = { // Behdet
	start_message : 245, //TUTORIAL_SOLDIERS_AND_FORT, 245 = 146 + 99 - 1
	city_has_animals : true,
	player_rank : 1,
	stages : {
		tutorial_start: [BUILDING_SMALL_STATUE, BUILDING_MEDIUM_STATUE, BUILDING_LARGE_STATUE, BUILDING_GARDENS, BUILDING_PLAZA,
						 BUILDING_ROADBLOCK, BUILDING_FIREHOUSE, BUILDING_ARCHITECT_POST, BUILDING_POLICE_STATION, BUILDING_VILLAGE_PALACE,
						 BUILDING_TAX_COLLECTOR, BUILDING_COURTHOUSE, BUILDING_PERSONAL_MANSION,
						 BUILDING_WATER_SUPPLY, BUILDING_APOTHECARY, BUILDING_PHYSICIAN,
						 BUILDING_WORK_CAMP, BUILDING_CHICKPEAS_FARM, BUILDING_BARLEY_FARM,
						 BUILDING_BOOTH, BUILDING_JUGGLER_SCHOOL, BUILDING_BANDSTAND, BUILDING_CONSERVATORY,
						 BUILDING_TAX_COLLECTOR, BUILDING_COURTHOUSE, BUILDING_PERSONAL_MANSION, BUILDING_BAZAAR, BUILDING_GRANARY, BUILDING_STORAGE_YARD,
						 BUILDING_RECRUITER, BUILDING_FORT_INFANTRY, BUILDING_FORT_ARCHERS, BUILDING_WEAPONSMITH,
						 BUILDING_SCRIBAL_SCHOOL,
						 BUILDING_CLAY_PIT, BUILDING_REED_GATHERER, BUILDING_GOLD_MINE,
						 BUILDING_POTTERY_WORKSHOP, BUILDING_WEAVER_WORKSHOP, BUILDING_BREWERY_WORKSHOP, BUILDING_PAPYRUS_WORKSHOP,
						 BUILDING_SHIPWRIGHT, BUILDING_FISHING_WHARF, BUILDING_FERRY, BUILDING_DOCK,
						 BUILDING_WARSHIP_WHARF, BUILDING_TRANSPORT_WHARF,
						 BUILDING_TEMPLE_OSIRIS, BUILDING_SHRINE_OSIRIS, BUILDING_TEMPLE_RA, BUILDING_SHRINE_RA,
						 BUILDING_FESTIVAL_SQUARE, BUILDING_BOOTH, BUILDING_JUGGLER_SCHOOL, BUILDING_BANDSTAND, BUILDING_CONSERVATORY, BUILDING_PAVILLION, BUILDING_DANCE_SCHOOL],
	},

	enable_scenario_events : false,
	events : [
		{
			year : 2684,
			resource : "pottery",
			amount : 1400,
			deadline : 9,
		},
		{
			year : 2683,
			resource : "beer",
			amount : 1100,
			deadline : 12,
		},
	],

	cities : [
		{
			name : "Byblos",
			is_sea_trade : true,
		},

		{
			name: "Perwadjyt",
			is_sea_trade : true,
		},

		{
			name: "Men-nefer",
			is_sea_trade : true,
		}
	],

	choice_background : {pack:PACK_UNLOADED, id:12},
	choice_image1 : {pack:PACK_UNLOADED, id:13, offset:0},
	choice_image1_pos : [192, 144],
	choice_title : [144, 19],
	choice : [
		{
			name : "Abydos",
			id : 6,
			image: {pack:PACK_UNLOADED, id:20, offset:0},
			tooltip : [144, 20],
			pos : [620, 420],
		},

		{
			name : "Behdet",
			id : 7,
			image: {pack:PACK_UNLOADED, id:20},
			tooltip : [144, 21],
			pos : [640, 480],
		}
	],
}

mission7 = { // Abydos
	start_message : 146, //TUTORIAL_SOLDIERS_AND_FORT, 245 = 146 + 99 - 1
	city_has_animals : true,
	player_rank : 1,
	stages : {
		tutorial_start: [BUILDING_SMALL_STATUE, BUILDING_MEDIUM_STATUE, BUILDING_LARGE_STATUE, BUILDING_GARDENS, BUILDING_PLAZA,
						 BUILDING_ROADBLOCK, BUILDING_FIREHOUSE, BUILDING_ARCHITECT_POST, BUILDING_POLICE_STATION, BUILDING_VILLAGE_PALACE,
						 BUILDING_TAX_COLLECTOR, BUILDING_COURTHOUSE, BUILDING_PERSONAL_MANSION,
						 BUILDING_WATER_SUPPLY, BUILDING_APOTHECARY, BUILDING_PHYSICIAN,
						 BUILDING_WORK_CAMP, BUILDING_FLAX_FARM, BUILDING_BARLEY_FARM,
						 BUILDING_BOOTH, BUILDING_JUGGLER_SCHOOL, BUILDING_BANDSTAND, BUILDING_CONSERVATORY,
						 BUILDING_TAX_COLLECTOR, BUILDING_COURTHOUSE, BUILDING_PERSONAL_MANSION, BUILDING_BAZAAR, BUILDING_GRANARY, BUILDING_STORAGE_YARD,
						 BUILDING_RECRUITER, BUILDING_FORT_INFANTRY, BUILDING_FORT_ARCHERS, BUILDING_WEAPONSMITH,
						 BUILDING_SCRIBAL_SCHOOL,
						 BUILDING_CLAY_PIT, BUILDING_REED_GATHERER,
						 BUILDING_POTTERY_WORKSHOP, BUILDING_WEAVER_WORKSHOP, BUILDING_BREWERY_WORKSHOP, BUILDING_PAPYRUS_WORKSHOP, BUILDING_BRICKS_WORKSHOP,
						 BUILDING_SHIPWRIGHT, BUILDING_FISHING_WHARF, BUILDING_FERRY,
						 BUILDING_WARSHIP_WHARF, BUILDING_TRANSPORT_WHARF, BUILDING_DOCK,
						 BUILDING_TEMPLE_OSIRIS, BUILDING_SHRINE_OSIRIS,
						 BUILDING_BRICKLAYERS_GUILD,
						 BUILDING_SMALL_MASTABA, BUILDING_MEDIUM_MASTABA,
						 BUILDING_FESTIVAL_SQUARE, BUILDING_BOOTH, BUILDING_JUGGLER_SCHOOL, BUILDING_BANDSTAND, BUILDING_CONSERVATORY, BUILDING_PAVILLION, BUILDING_DANCE_SCHOOL],
	},

	enable_scenario_events : false,
	events : [
		{
			type: EVENT_TYPE_REQUEST,
			year : 2684,
			month : 1,
			resource : RESOURCE_POTTERY,
			amount : 1400,
			deadline : 9,
		},

		{
			type: EVENT_TYPE_INVASION,
			year : 2670,
			month : 2,
		},

		{
			type: "pharaoh_gift", //EVENT_TYPE_INVASION,
			year: 2670,
			month: 8,
		},
	],

	cities : [
		{
			name : "Byblos",
			is_sea_trade : true,
		},
	],

	choice_background : {pack:PACK_UNLOADED, id:12},
	choice_image1 : {pack:PACK_UNLOADED, id:13, offset:0},
	choice_image1_pos : [192, 144],
	choice_title : [144, 19],
	choice : [
		{
			name : "Abydos",
			id : 6,
			image: {pack:PACK_UNLOADED, id:20, offset:0},
			tooltip : [144, 20],
			pos : [620, 420],
		},

		{
			name : "Behdet",
			id : 7,
			image: {pack:PACK_UNLOADED, id:20},
			tooltip : [144, 21],
			pos : [640, 480],
		}
	],
}
