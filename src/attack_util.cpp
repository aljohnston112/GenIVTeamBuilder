#include "attack_util.h"

#include <set>
#include <unordered_set>


std::unordered_set<const Attack*> get_all_attacks(
    const Pokemon& pokemon,
    const int max_level = 60,
    const std::string& form = ""
) {
    std::unordered_set<const Attack*> attacks{};

    // Add the attacks
    for (const auto& [level, level_up_attacks] :
         pokemon.level_to_attacks
    ) {
        if (level <= max_level) {
            for (const Attack& attack : level_up_attacks) {
                attacks.emplace(&attack);
            }
        }
    }

    if (pokemon.tm_or_hm_to_attack.has_value()) {
        for (const Attack& attack :
             std::views::values(*pokemon.tm_or_hm_to_attack)
        ) {
            attacks.emplace(&attack);
        }
    }

    if (pokemon.egg_moves.has_value()) {
        for (const Attack& attack : *pokemon.egg_moves) {
            attacks.emplace(&attack);
        }
    }

    if (pokemon.pre_evolution_index_to_level_to_moves.has_value()) {
        for (const std::map<int, std::vector<Attack>>& attack_map :
             std::views::values(
                 *pokemon.pre_evolution_index_to_level_to_moves
             )
        ) {
            for (const auto& [level, level_up_attacks] :
                 attack_map
            ) {
                if (level <= max_level) {
                    for (const Attack& attack : level_up_attacks) {
                        attacks.emplace(&attack);
                    }
                }
            }
        }
    }

    if (pokemon.move_tutor_attacks.has_value()) {
        for (const Attack& attack : *pokemon.move_tutor_attacks) {
            attacks.emplace(&attack);
        }
    }

    if (pokemon.game_to_level_to_moves.has_value()) {
        for (const std::map<int, std::vector<Attack>>& attack_map :
             std::views::values(
                 *pokemon.game_to_level_to_moves
             )
        ) {
            for (const auto& [level, level_up_attacks] :
                 attack_map
            ) {
                if (level <= max_level) {
                    for (const Attack& attack : level_up_attacks) {
                        attacks.emplace(&attack);
                    }
                }
            }
        }
    }

    if (pokemon.special_moves.has_value()) {
        for (const Attack& attack : *pokemon.special_moves) {
            attacks.emplace(&attack);
        }
    }

    if (!form.empty()) {
        if (pokemon.form_to_level_up_attacks.has_value()) {
            for (const auto& [level, level_up_form_attacks] :
                pokemon.form_to_level_up_attacks->at(form)
                 ) {
                if (level < max_level) {
                    for (const Attack& attack : level_up_form_attacks) {
                        attacks.emplace(&attack);
                    }
                }
            }
        }

        if (pokemon.form_to_tm_or_hm_to_attack.has_value()) {
            for (const Attack& attack : std::views::values(
                     pokemon.form_to_tm_or_hm_to_attack->at(form))) {
                attacks.emplace(&attack);
            }
        }

        if (pokemon.form_to_move_tutor_attacks.has_value()) {
            for (const Attack& attack :
                 pokemon.form_to_move_tutor_attacks->at(form)
            ) {
                attacks.emplace(&attack);
            }
        }
    }

    return std::move(attacks);
}

std::vector<const Attack*> get_one_hit_ko_attacks(
    const BattleState& battle_state,
    const DefenderTypeChart& defender_type_chart,
    const bool with_max_stats,
    const bool is_opponent
) {
    std::vector<const Attack*> attacks_that_ko{};

    const auto pokemon1 = battle_state.pokemon1.pokemon.get();
    const auto pokemon0 = battle_state.pokemon0.pokemon.get();
    auto& attacking_pokemon = is_opponent ? pokemon1 : pokemon0;
    auto& defender_pokemon = is_opponent ? pokemon0 : pokemon1;
    auto& attacker_state =
        is_opponent ? battle_state.pokemon1 : battle_state.pokemon0;
    auto& defender_state =
        is_opponent ? battle_state.pokemon0 : battle_state.pokemon1;

    for (const std::unordered_set<const Attack*> attacker_attacks =
             get_all_attacks(attacking_pokemon);
         const Attack* attack : attacker_attacks
    ) {
        constexpr unsigned int level = 50;
        unsigned int damage = std::floor((2 * level) / 5.0) + 2;
        double power = attack->power;
        const std::string_view attack_name = attack->name;
        auto defender_pokemon_information =
            defender_pokemon.pokemon_information;
        if (attack_name == "Grass Knot") {
            if (const auto& defender_weight =
                    defender_pokemon_information.pounds;
                defender_weight < 21.9
            ) {
                power = 20;
            } else if (defender_weight < 55.1) {
                power = 40;
            } else if (defender_weight < 110.2) {
                power = 60;
            } else if (defender_weight < 220.4) {
                power = 80;
            } else if (defender_weight < 440.9) {
                power = 100;
            } else {
                power = 120;
            }
        }
        damage *= power;

        auto all_attacker_stats = attacking_pokemon.all_stats;
        const auto& stats =
            with_max_stats
                ? all_attacker_stats.level_50_max_stats
                : all_attacker_stats.level_50_min_stats;
        const auto& defender_stats =
            with_max_stats
                ? all_attacker_stats.level_50_max_stats
                : all_attacker_stats.level_50_min_stats;
        const bool attack_is_special = attack->category == Category::SPECIAL;
        const double attackStat =
            attack_is_special ? stats.special_attack : stats.attack;
        const double defense_stat =
            attack_is_special
                ? defender_stats.special_defense
                : defender_stats.defense;

        damage *= attackStat;
        damage = std::floor(damage / defense_stat);
        damage = std::floor(damage / 50.0);
        // TODO guts, facade, etc
        if (attacker_state.is_burned) {
            damage = std::floor(damage / 2.0);
        }

        const Category attack_category = attack->category;
        if (const bool attack_is_physical =
                attack_category == Category::PHYSICAL;
            attack_is_physical && defender_state.has_reflect_up
        ) {
            damage = std::floor(damage / 2);
        } else if (attack_is_special && defender_state.has_light_screen_up) {
            damage = std::floor(damage / 2);
        }

        const PokemonType attack_type = attack->pokemon_type;
        auto attacker_pokemon_information =
            attacking_pokemon.pokemon_information;
        const std::string_view attacker_ability =
            attacker_pokemon_information.ability;
        const std::string_view defender_ability =
            defender_pokemon_information.ability;
        if (!attacker_ability.contains("Cloud Nine") &&
            !attacker_ability.contains("Air Lock") &&
            !defender_ability.contains("Cloud Nine") &&
            !defender_ability.contains("Air Lock")) {
            if (attack_type == PokemonType::WATER && battle_state.is_raining) {
                damage = std::floor(damage * 1.5);
            }
            if (attack_type == PokemonType::FIRE && battle_state.is_sunny) {
                damage = std::floor(damage * 1.5);
            }
            if (attack_type == PokemonType::WATER && battle_state.is_sunny) {
                damage = std::floor(damage / 2.0);
            }
            if (attack_type == PokemonType::FIRE && battle_state.is_raining) {
                damage = std::floor(damage / 2.0);
            }
            if (attack_name == "Solarbeam" &&
                (battle_state.is_raining || battle_state.is_foggy ||
                    battle_state.is_hailing || battle_state.is_sandstorming)
            ) {
                // TODO
                damage = 0;
            }
        }

        if (attack_type == PokemonType::FIRE &&
            attacker_state.is_flash_fire_activated
        ) {
            damage = std::floor(damage * 1.5);
        }
        damage += 2;
        if (attack_type == PokemonType::FIRE &&
            defender_ability.contains("Flash Fire")
        ) {
            damage = 0;
        }

        if (const auto& attacker_types =
                attacker_pokemon_information.pokemon_types;
            std::ranges::find(attacker_types, attack_type) !=
            attacker_types.end()
        ) {
            damage = std::floor(damage * 1.5);
        }

        auto multiplier = 1.0;
        for (const auto& defender_types =
                 defender_pokemon_information.pokemon_types;
             const auto& defender_type : defender_types
        ) {
            multiplier *= defender_type_chart.at(defender_type).at(attack_type);
        }
        damage = std::floor(damage * multiplier);
        if (multiplier > 1 &&
            (defender_ability.contains("Solid Rock")) ||
            (defender_ability.contains("Filter") &&
                !attacker_ability.contains("Mold Breaker"))
        ) {
            damage = std::floor(damage * 0.75);
        }
        if (multiplier < 1 && attacker_ability.contains("Tinted Lens")) {
            damage = std::floor(damage * 2);
        }

        if (attack_name == "Solarbeam" && !battle_state.is_sunny) {
            // TODO
            damage = 0;
        }
        if (damage > defender_stats.health) {
            attacks_that_ko.emplace_back(attack);
        }
    }
    return attacks_that_ko;
}
