#ifndef ATTACK_UTIL_H
#define ATTACK_UTIL_H
#include <unordered_set>
#include <vector>

#include "models.h"
#include "type_chart_data_source.h"


std::unordered_set<const Attack*>  get_all_attacks(
    const Pokemon& pokemon,
    int max_level,
    const std::string& form
);

std::vector<const Attack*> get_one_hit_ko_attacks(
    const BattleState& battle_state,
    const DefenderTypeChart& defender_type_chart,
    bool with_max_stats,
    bool is_opponent
);

#endif //ATTACK_UTIL_H
