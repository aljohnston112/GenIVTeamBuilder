#include <chrono>
#include <functional>
#include <iostream>
#include <ranges>
#include <unordered_set>

#include "models.h"
#include "pokemon_data_source.h"
#include "type_chart_data_source.h"

constexpr int level = 50;

std::vector<std::reference_wrapper<const Attack>> get_all_attacks(
    const Pokemon& pokemon,
    const std::string& form = ""
) {
    std::vector<std::reference_wrapper<const Attack>> attacks{};
    //Reserve space for attacks

    size_t number_of_attacks = 0;
    for (const auto& attack_vector : std::views::values(pokemon.level_to_attacks)) {
        number_of_attacks += attack_vector.size();
    }
    if (pokemon.tm_or_hm_to_attack.has_value()) {
        number_of_attacks += pokemon.tm_or_hm_to_attack->size();
    }
    if (pokemon.egg_moves.has_value()) {
        number_of_attacks += pokemon.egg_moves->size();
    }
    if (pokemon.pre_evolution_index_to_level_to_moves.has_value()) {
        for (const auto& attack_vector : std::views::values(*pokemon.pre_evolution_index_to_level_to_moves)) {
            number_of_attacks += attack_vector.size();
        }
    }
    if (pokemon.move_tutor_attacks.has_value()) {
        number_of_attacks += pokemon.move_tutor_attacks->size();
    }
    if (pokemon.game_to_level_to_moves.has_value()) {
        for (const auto& attack_map : std::views::values(*pokemon.game_to_level_to_moves)) {
            for (const std::vector<Attack>& level_up_attacks : std::views::values(attack_map)) {
                number_of_attacks += level_up_attacks.size();
            }
        }
    }
    if (pokemon.special_moves.has_value()) {
        number_of_attacks += pokemon.special_moves->size();
    }
    if (!form.empty()) {
        if (pokemon.form_to_level_up_attacks.has_value()) {
            number_of_attacks += pokemon.form_to_level_up_attacks->at(form).size();
        }
        if (pokemon.form_to_tm_or_hm_to_attack.has_value()) {
            number_of_attacks += pokemon.form_to_tm_or_hm_to_attack->at(form).size();
        }
        if (pokemon.form_to_move_tutor_attacks.has_value()) {
            number_of_attacks += pokemon.form_to_move_tutor_attacks->at(form).size();
        }
    }
    attacks.reserve(number_of_attacks);


    for (const std::vector<Attack>& level_up_attacks : std::views::values(pokemon.level_to_attacks)) {
        for (const Attack& attack : level_up_attacks) {
            attacks.push_back(std::cref(attack));
        }
    }

    if (pokemon.tm_or_hm_to_attack.has_value()) {
        for (const Attack& attack : std::views::values(*pokemon.tm_or_hm_to_attack)) {
            attacks.push_back(std::cref(attack));
        }
    }

    if (pokemon.egg_moves.has_value()) {
        for (const Attack& attack : *pokemon.egg_moves) {
            attacks.push_back(std::cref(attack));
        }
    }

    if (pokemon.pre_evolution_index_to_level_to_moves.has_value()) {
        for (const std::map<int, std::vector<Attack>>& attack_map : std::views::values(
                 (*pokemon.pre_evolution_index_to_level_to_moves))) {
            for (const std::vector<Attack>& level_up_attacks : std::views::values(attack_map)) {
                for (const Attack& attack : level_up_attacks) {
                    attacks.push_back(std::cref(attack));
                }
            }
        }
    }

    if (pokemon.move_tutor_attacks.has_value()) {
        for (const Attack& attack : *pokemon.move_tutor_attacks) {
            attacks.push_back(std::cref(attack));
        }
    }

    if (pokemon.game_to_level_to_moves.has_value()) {
        for (const std::map<int, std::vector<Attack>>& attack_map : std::views::values(
                 (*pokemon.game_to_level_to_moves))) {
            for (const std::vector<Attack>& level_up_attacks : std::views::values(attack_map)) {
                for (const Attack& attack : level_up_attacks) {
                    attacks.push_back(std::cref(attack));
                }
            }
        }
    }

    if (pokemon.special_moves.has_value()) {
        for (const Attack& attack : *pokemon.special_moves) {
            attacks.push_back(std::cref(attack));
        }
    }

    if (!form.empty()) {
        if (pokemon.form_to_level_up_attacks.has_value()) {
            for (const std::vector<Attack>& level_up_form_attacks : std::views::values(
                     pokemon.form_to_level_up_attacks->at(form))) {
                for (const Attack& attack : level_up_form_attacks) {
                    attacks.push_back(std::cref(attack));
                }
            }
        }

        if (pokemon.form_to_tm_or_hm_to_attack.has_value()) {
            for (const Attack& attack : std::views::values(pokemon.form_to_tm_or_hm_to_attack->at(form))) {
                attacks.push_back(std::cref(attack));
            }
        }

        if (pokemon.form_to_move_tutor_attacks.has_value()) {
            for (const Attack& attack : pokemon.form_to_move_tutor_attacks->at(form)) {
                attacks.push_back(std::cref(attack));
            }
        }
    }

    std::unordered_set<std::string_view> seen_names;
    seen_names.reserve(number_of_attacks);
    const auto new_end = std::ranges::remove_if(
        attacks,
        [&](const auto& attack_ref) {
            const auto& name = attack_ref.get().name;
            return !seen_names.insert(name).second;
        }
    ).begin();
    attacks.erase(new_end, attacks.end());

    return std::move(attacks);
}

std::vector<std::reference_wrapper<const Attack>> get_one_hit_ko_attacks(
    const BattleState& battle_state,
    DefenderTypeChart& defender_type_chart,
    bool with_max_stats
) {
    std::vector<std::reference_wrapper<const Attack>> attacks_that_ko{};

    auto& pokemon0 = battle_state.pokemon0.pokemon.get();
    auto& pokemon1 = battle_state.pokemon1.pokemon.get();

    std::vector<std::reference_wrapper<const Attack>> attacks0 = get_all_attacks(pokemon0);
    for (std::reference_wrapper<const Attack>& wrapped_attack : attacks0) {
        double damage = ((2.0 * level) / 5.0) + 2;

        const Attack& attack = wrapped_attack.get();
        double power = attack.power;
        const std::string attack_name = attack.name;
        if (attack_name == "Grass Knot") {
            const auto& defender_weight = pokemon1.pokemon_information.pounds;
            if (defender_weight < 21.9) {
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

        double attackStat = pokemon0.all_stats.level_50_min_stats.attack;
        double defenseStat = pokemon1.all_stats.level_50_min_stats.defense;
        if (attack.category == Category::SPECIAL) {
            attackStat = pokemon0.all_stats.level_50_min_stats.special_attack;
            defenseStat = pokemon1.all_stats.level_50_min_stats.special_defense;
        }
        if (with_max_stats) {
            attackStat = pokemon0.all_stats.level_50_max_stats.attack;
            defenseStat = pokemon1.all_stats.level_50_max_stats.defense;
            if (attack.category == Category::SPECIAL) {
                attackStat = pokemon0.all_stats.level_50_max_stats.special_attack;
                defenseStat = pokemon1.all_stats.level_50_max_stats.special_defense;
            }
        }

        damage *= (attackStat / defenseStat);
        damage /= 50;
        if (battle_state.pokemon0.is_burned) {
            damage *= 0.5;
        }

        const Category attack_category = attack.category;
        if (attack_category == Category::PHYSICAL && battle_state.pokemon1.has_reflect_up) {
            damage *= 0.5;
        }
        if (attack_category == Category::SPECIAL && battle_state.pokemon1.has_light_screen_up) {
            damage *= 0.5;
        }

        const PokemonType attack_type = attack.pokemon_type;
        const auto& attacker_ability = pokemon0.pokemon_information.ability;
        const auto& defender_ability = pokemon1.pokemon_information.ability;
        if (!attacker_ability.contains("Cloud Nine") &&
            !attacker_ability.contains("Air Lock") &&
            !defender_ability.contains("Cloud Nine") &&
            !defender_ability.contains("Air Lock")) {
            if (attack_type == PokemonType::WATER && battle_state.is_raining) {
                damage *= 1.5;
            }
            if (attack_type == PokemonType::FIRE && battle_state.is_sunny) {
                damage *= 1.5;
            }
            if (attack_type == PokemonType::WATER && battle_state.is_sunny) {
                damage *= 0.5;
            }
            if (attack_type == PokemonType::FIRE && battle_state.is_raining) {
                damage *= 0.5;
            }
            if (attack_name == "Solarbeam" &&
                (battle_state.is_raining || battle_state.is_foggy ||
                    battle_state.is_hailing || battle_state.is_sandstorming)
            ) {
                damage = 0;
            }
        }

        if (attack_type == PokemonType::FIRE && battle_state.pokemon0.is_flash_fire_activated) {
            damage *= 1.5;
        }
        damage += 2;
        if (attack_type == PokemonType::FIRE &&
            pokemon1.pokemon_information.ability.contains("Flash Fire")) {
            damage = 0;
        }

        const auto& pokemon0_types = pokemon0.pokemon_information.pokemon_types;
        if (std::ranges::find(pokemon0_types, attack_type) != pokemon0_types.end()) {
            damage *= 1.5;
        }

        const auto& pokemon1_types = pokemon1.pokemon_information.pokemon_types;
        for (const auto& defender_type : pokemon1_types) {
            const auto& multiplier = defender_type_chart[defender_type][attack_type];
            damage *= multiplier;
            if (multiplier > 1 &&
                (defender_ability.contains("Solid Rock")) ||
                defender_ability.contains("Filter") &&
                !attacker_ability.contains("Mold Breaker")) {
                damage *= 0.75;
            }
            if (multiplier < 1 && attacker_ability.contains("Tinted Lens")) {
                damage *= 2;
            }
        }

        if (attack_name == "Solarbeam" && !battle_state.is_sunny) {
            damage = 0;
        }
        if (damage > battle_state.pokemon1.hp_left() && attack.accuracy == 100) {
            attacks_that_ko.emplace_back(attack);
        }
    }
    return attacks_that_ko;
}

bool battle(
    const Pokemon& pokemon0,
    const Pokemon& pokemon1,
    DefenderTypeChart& defender_type_chart,
    bool with_max_stats
) {
    BattleState battle_state = {
        .pokemon0 = PokemonState{
            .with_max_stats = with_max_stats,
            .pokemon = std::cref(pokemon0)
        },
        .pokemon1 = PokemonState{
            .with_max_stats = with_max_stats,
            .pokemon = std::cref(pokemon1)
        },
    };
    std::vector<std::reference_wrapper<const Attack>> attacks_that_ko = get_one_hit_ko_attacks(
        battle_state,
        defender_type_chart,
        with_max_stats
    );
    if (!attacks_that_ko.empty()) {
        return true;
    }
    return false;
}

void battle_tournament(const PokemonData& pokemon_data, bool with_max_stats) {
    const auto values = std::views::values(pokemon_data);
    DefenderTypeChart defender_type_chart = get_defender_type_chart();
    std::unordered_map<std::string_view, std::vector<std::string_view>> result_map;
    for (const auto& value0 : values
    ) {
        for (const auto& value1 : values) {
            if (value0.pokemon_information.id != value1.pokemon_information.id) {
                const auto& battle_result = battle(value0, value1, defender_type_chart, with_max_stats);
                if (battle_result) {
                    result_map[value0.pokemon_information.name].emplace_back(value1.pokemon_information.name);
                }
            }
        }
    }

    std::string one_hitter_file_name = "../data/generated/one_hitters_min_stats.txt";
    if (with_max_stats) {
        one_hitter_file_name = "../data/generated/one_hitters_max_stats.txt";
    }
    std::ofstream one_hitter_output_file(one_hitter_file_name);
    if (!one_hitter_output_file) {
        throw std::ios_base::failure("Failed to open the one hitter file for writing");
    }
    std::vector<std::pair<std::string_view, std::vector<std::string_view>>> sorted_result;
    sorted_result.reserve(result_map.size());
    for (const auto& entry : result_map) {
        sorted_result.emplace_back(entry);
    }
    std::ranges::sort(
        sorted_result,
        [](const auto& a, const auto& b) {
            return a.second.size() > b.second.size();
        }
    );
    for (const auto& [fst, snd] : sorted_result) {
        one_hitter_output_file << fst << " defeated " << snd.size() << " Pokemon: ";
        for (const auto& name : snd) {
            one_hitter_output_file << name << " ";
        }
        one_hitter_output_file << std::endl;
    }

    std::string times_fainted_file_name = "../data/generated/times_fainted_min_stats.txt";
    if (with_max_stats) {
        times_fainted_file_name = "../data/generated/times_fainted_max_stats.txt";
    }
    std::ofstream times_fainted_output_file(times_fainted_file_name);
    if (!times_fainted_output_file) {
        throw std::ios_base::failure("Failed to open the one hitter file for writing");
    }
    std::unordered_map<std::string_view, int> loser_count;
    for (const auto& snd : result_map | std::views::values) {
        for (const auto& losers = snd; const auto& loser : losers) {
            loser_count[loser]++;
        }
    }
    std::vector<std::pair<std::string_view, int>> sorted_losers(loser_count.begin(), loser_count.end());
    std::ranges::sort(
        sorted_losers,
        [](const auto& a, const auto& b) {
            return a.second > b.second;
        }
    );
    for (const auto& [fst, snd] : sorted_losers) {
        times_fainted_output_file << fst << " has " << snd << " losses." << std::endl;
    }
}


void timer(const std::function<void()>& func, const std::string& message) {
    std::cout << message << '\n';

    const auto start = std::chrono::high_resolution_clock::now();
    func();
    const auto end = std::chrono::high_resolution_clock::now();

    const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "Execution time: " << duration << " ms\n";
}

int main(int argc, char* argv[]) {

    // There are 216 fully evolved Pokemon
    static const std::initializer_list<int> fully_evolved_pokemon_indices = {
        12, 15, 18, 20, 22, 24, 26, 28, 31, 34, 36, 38, 40,
        45, 47, 49, 51, 53, 55, 57, 59, 62, 65, 68, 71, 73,
        76, 78, 80, 82, 83, 85, 87, 89, 91, 94, 97, 99, 101,
        103, 105, 106, 107, 110, 115, 119, 120, 122, 124,
        127, 128, 130, 131, 132, 134, 135, 136, 139, 141,
        142, 143, 149, 162, 164, 166, 168, 169, 171, 178,
        181, 182, 184, 185, 186, 189, 192, 195, 196, 197,
        199, 201, 202, 203, 205, 206, 208, 210, 211, 212,
        213, 214, 217, 219, 222, 224, 225, 226, 227, 229,
        230, 232, 233, 234, 235, 237, 241, 242, 243, 248,
        262, 264, 267, 269, 272, 275, 277, 279, 284, 286,
        288, 289, 290, 292, 295, 297, 301, 302, 303, 306,
        308, 310, 311, 312, 313, 314, 317, 319, 321, 323,
        324, 326, 327, 332, 334, 335, 336, 337, 338, 340,
        342, 344, 346, 348, 350, 351, 352, 354, 357, 358,
        359, 362, 365, 369, 370, 373, 376, 389, 392, 395,
        398, 400, 402, 405, 407, 409, 411, 413, 414, 416,
        417, 419, 421, 423, 424, 426, 428, 437, 441, 442,
        445, 448, 450, 452, 454, 455, 457, 460, 461, 462,
        463, 464, 465, 466, 467, 468, 469, 470, 471, 472,
        473, 474, 475, 476, 477, 478, 479
    };

    PokemonData pokemon_data;
    timer(
        [&pokemon_data] {
            pokemon_data = get_all_fully_evolved_pokemon(fully_evolved_pokemon_indices);
        },
        "Loading Pokemon"
    );

    timer(
        [&pokemon_data] {
            constexpr bool with_max_stats = true;
            battle_tournament(pokemon_data, with_max_stats);
        },
        "Pokemon Battles"
    );

    timer(
        [&pokemon_data] {
            constexpr bool with_max_stats = false;
            battle_tournament(pokemon_data, with_max_stats);
        },
        "Pokemon Battles"
    );

    return 0;
}
