#ifndef FIGURE_COMBAT_H
#define FIGURE_COMBAT_H
//
#include "figure/figure.h"
#include "grid/point.h"
//
////void figure_combat_handle_corpse();
////void figure_combat_handle_attack();
////
int figure_combat_get_target_for_soldier(int x, int y, int max_distance);
int figure_combat_get_target_for_wolf(int x, int y, int max_distance);
int figure_combat_get_target_for_enemy(int x, int y);
////
int figure_combat_get_missile_target_for_soldier(figure *shooter, int max_distance, map_point *tile);
int figure_combat_get_missile_target_for_enemy(figure *enemy, int max_distance, int attack_citizens, map_point *tile);
////
////void figure_combat_attack_figure_at(int grid_offset_figure);
//
#endif // FIGURE_COMBAT_H
