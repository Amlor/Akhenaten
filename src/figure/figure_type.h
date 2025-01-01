#pragma once

#include <cstdint>
#include "core/tokenum.h"

enum e_enemy_type {
    ENEMY_0_BARBARIAN = 0,
    ENEMY_1_NUMIDIAN = 1,
    ENEMY_2_GAUL = 2,
    ENEMY_3_CELT = 3,
    ENEMY_4_GOTH = 4,
    ENEMY_5_PERGAMUM = 5,
    ENEMY_6_SELEUCID = 6,
    ENEMY_7_ETRUSCAN = 7,
    ENEMY_8_GREEK = 8,
    ENEMY_9_EGYPTIAN = 9,
    ENEMY_10_CARTHAGINIAN = 10,
    ENEMY_11_CAESAR = 11
};

enum e_figure_state {
    FIGURE_STATE_NONE = 0,
    FIGURE_STATE_ALIVE = 1,
    FIGURE_STATE_DEAD = 2,
    FIGURE_STATE_DYING = 3,
};

enum e_terrain_usage {
    TERRAIN_USAGE_ANY = 0,
    TERRAIN_USAGE_ROADS = 1,
    TERRAIN_USAGE_ENEMY = 2,
    TERRAIN_USAGE_PREFER_ROADS = 3,
    TERRAIN_USAGE_WALLS = 4,
    TERRAIN_USAGE_ANIMAL = 5,
};

enum e_figure_type {
    FIGURE_NONE = 0,
    FIGURE_IMMIGRANT = 1,
    FIGURE_EMIGRANT = 2,
    FIGURE_HOMELESS = 3,
    FIGURE_CART_PUSHER = 4,
    FIGURE_LABOR_SEEKER = 5,
    FIGURE_EXPLOSION = 6,
    FIGURE_TAX_COLLECTOR = 7,
    FIGURE_ARCHITECT = 8,
    FIGURE_STORAGEYARD_CART = 9,
    FIGURE_FIREMAN = 10,
    FIGURE_ARCHER = 11,
    FIGURE_FCHARIOTEER = 12,
    FIGURE_INFANTRY = 13,
    FIGURE_STANDARD_BEARER = 14,
    FIGURE_JUGGLER = 15,
    FIGURE_MUSICIAN = 16,
    FIGURE_DANCER = 17,
    FIGURE_SENET_PLAYER = 18,
    FIGURE_TRADE_CARAVAN = 19,
    FIGURE_TRADE_SHIP = 20,
    FIGURE_TRADE_CARAVAN_DONKEY = 21,
    FIGURE_PROTESTER = 22,
    FIGURE_CRIMINAL = 23,
    FIGURE_TOMB_ROBER = 24,
    FIGURE_FISHING_BOAT = 25,
    FIGURE_MARKET_TRADER = 26,
    FIGURE_PRIEST = 27,
    FIGURE_TEACHER = 28,
    FIGURE_SCRIBER = 29,
    FIGURE_LIBRARIAN = 30,
    FIGURE_DENTIST = 31,
    FIGURE_PHYSICIAN = 32,
    FIGURE_HERBALIST = 33,
    FIGURE_EMBALMER = 34,
    FIGURE_WORKER = 35,
    FIGURE_MAP_FLAG = 36,
    FIGURE_FLOTSAM = 37,
    FIGURE_DOCKER = 38,
    FIGURE_MARKET_BUYER = 39,
    FIGURE_NOBLES = 40,
    FIGURE_INDIGENOUS_NATIVE = 41,
    FIGURE_TOWER_SENTRY = 42,
    FIGURE_ENEMY43_SPEAR = 43,
    FIGURE_ENEMY44_SWORD = 44,
    FIGURE_ENEMY45_SWORD = 45,
    FIGURE_ENEMY46_CAMEL = 46,
    FIGURE_ENEMY47_ELEPHANT = 47,
    FIGURE_ENEMY48_CHARIOT = 48,
    FIGURE_ENEMY49_FAST_SWORD = 49,
    FIGURE_ENEMY50_SWORD = 50,
    FIGURE_ENEMY51_SPEAR = 51,
    FIGURE_ENEMY52_MOUNTED_ARCHER = 52,
    FIGURE_ENEMY53_AXE = 53,
    FIGURE_ENEMY54_GLADIATOR = 54,
    FIGURE_ENEMY_CAESAR_JAVELIN = 55,
    FIGURE_ENEMY_CAESAR_MOUNTED = 56,
    FIGURE_ENEMY_CAESAR_LEGIONARY = 57,
    FIGURE_NATIVE_TRADER = 58,
    FIGURE_ARROW = 59,
    FIGURE_JAVELIN = 60,
    FIGURE_BOLT = 61,
    FIGURE_BALLISTA = 62,
    FIGURE_CREATURE = 63,
    FIGURE_MISSIONARY = 64,
    FIGURE_FISHING_POINT = 65,
    FIGURE_DELIVERY_BOY = 66,
    FIGURE_SHIPWRECK = 67,
    FIGURE_BIRDS = 68,
    FIGURE_OSTRICH = 69,
    FIGURE_ANTELOPE = 70,
    FIGURE_SPEAR = 71,
    FIGURE_CHARIOR_RACER = 72,
    FIGURE_OSTRICH_HUNTER = 73,
    FIGURE_HUNTER_ARROW = 74,
    FIGURE_LUMBERJACK = 75,
    FIGURE_FERRY_BOAT = 76,
    FIGURE_TRANSPORT_SHIP = 77,
    FIGURE_WARSHIP = 78,
    FIGURE_CARPENTER = 79,
    FIGURE_BRICKLAYER = 80,
    FIGURE_STONEMASON = 81,
    FIGURE_CROCODILE = 82,
    FIGURE_HYENA = 83,
    FIGURE_HIPPO = 84,
    FIGURE_LABORER = 85,
    FIGURE_SLED = 86,
    FIGURE_WATER_CARRIER = 87,
    FIGURE_CONSTABLE = 88,
    FIGURE_MAGISTRATE = 89,
    FIGURE_REED_GATHERER = 90,
    FIGURE_FESTIVAL_PRIEST = 91,
    FIGURE_ENEMY_TRANSPORT = 92,
    FIGURE_ENEMY_WARSHIP = 93,
    FIGURE_FUNERAL_WALKER = 94,
    FIGURE_FISHING_SPOT = 95,
    FIGURE_SLED_PULLER = 96,
    FIGURE_SHOWMAN = 97,
    FIGURE_PLAGUED_CITIZEN = 98,
    FIGURE_BEDOUIN_INFANTRY = 99,
    FIGURE_EGYPTIAN_WARSHIP = 100,
    FIGURE_EGYPTIAN_TRANSPORT = 101,
    FIGURE_ASP = 102,
    FIGURE_LION = 103,
    FIGURE_SCORPION = 104,
    FIGURE_ZOOKEEPER = 105,
    FIGURE_FROG = 106,
    FIGURE_LOCUST = 107,
    FIGURE_TOMB_ARTISAN = 108,
    FIGURE_MUMMY = 109,
    FIGURE_PHARAOH = 110,
    FIGURE_GOVERNOR = 111,

    FIGURE_MAX,
};

using figure_id = uint16_t;
extern const token_holder<e_figure_type, FIGURE_NONE, FIGURE_MAX> e_figure_type_tokens;