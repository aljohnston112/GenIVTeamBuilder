#include <chrono>
#include <functional>
#include <iostream>
#include <ranges>

#include "attack_util.h"
#include "models.h"
#include "pokemon_data_source.h"
#include "type_chart_data_source.h"


bool battle(
    const Pokemon& pokemon0,
    const Pokemon& pokemon1,
    const DefenderTypeChart& defender_type_chart,
    const bool with_max_stats
) {
    const BattleState battle_state = {
        .pokemon0 = PokemonState{
            .with_max_stats = with_max_stats,
            .pokemon = std::cref(pokemon0)
        },
        .pokemon1 = PokemonState{
            .with_max_stats = with_max_stats,
            .pokemon = std::cref(pokemon1)
        },
    };
    constexpr bool is_opponent = false;
    const std::vector<const Attack*> attacks_that_ko =
        get_one_hit_ko_attacks(
            battle_state,
            defender_type_chart,
            with_max_stats,
            is_opponent
        );
    if (!attacks_that_ko.empty()) {
        return true;
    }
    return false;
}

void battle_tournament(
    const PokemonData& pokemon_data,
    const bool with_max_stats
) {
    const auto values = std::views::values(pokemon_data);
    DefenderTypeChart defender_type_chart = get_defender_type_chart();
    std::unordered_map<std::string_view, std::vector<std::string_view>>
        result_map;
    for (const auto& pokemon0 : values) {
        for (const auto& pokemon1 : values) {
            auto& pokemon0_information = pokemon0.pokemon_information;
            if (auto& pokemon1_information = pokemon1.pokemon_information;
                pokemon0_information.id != pokemon1_information.id
            ) {
                const auto& battle_result = battle(
                    pokemon0,
                    pokemon1,
                    defender_type_chart,
                    with_max_stats
                );
                if (battle_result) {
                    result_map[pokemon0_information.name].emplace_back(
                        pokemon1_information.name
                    );
                }
            }
        }
    }

    std::string one_hitter_file_name =
        "../data/generated/one_hitters_min_stats.txt";
    if (with_max_stats) {
        one_hitter_file_name = "../data/generated/one_hitters_max_stats.txt";
    }
    std::ofstream one_hitter_output_file(one_hitter_file_name);
    if (!one_hitter_output_file) {
        throw std::ios_base::failure(
            "Failed to open the one hitter file for writing");
    }
    std::vector<std::pair<std::string_view, std::vector<std::string_view>>>
        sorted_result;
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
        one_hitter_output_file << fst << " defeated " << snd.size() <<
            " Pokemon: ";
        for (const auto& name : snd) {
            one_hitter_output_file << name << " ";
        }
        one_hitter_output_file << std::endl;
    }

    std::string times_fainted_file_name =
        "../data/generated/times_fainted_min_stats.txt";
    if (with_max_stats) {
        times_fainted_file_name =
            "../data/generated/times_fainted_max_stats.txt";
    }
    std::ofstream times_fainted_output_file(times_fainted_file_name);
    if (!times_fainted_output_file) {
        throw std::ios_base::failure(
            "Failed to open the one hitter file for writing");
    }
    std::unordered_map<std::string_view, int> loser_count;
    for (const auto& snd : result_map | std::views::values) {
        for (const auto& losers = snd;
             const auto& loser : losers
        ) {
            loser_count[loser]++;
        }
    }
    std::vector<std::pair<std::string_view, int>> sorted_losers(
        loser_count.begin(), loser_count.end());
    std::ranges::sort(
        sorted_losers,
        [](const auto& a, const auto& b) {
            return a.second > b.second;
        }
    );
    for (const auto& [fst, snd] : sorted_losers) {
        times_fainted_output_file << fst << " has " << snd << " losses." <<
            std::endl;
    }
}


void timer(const std::function<void()>& func, const std::string& message) {
    std::cout << message << '\n';

    const auto start = std::chrono::high_resolution_clock::now();
    func();
    const auto end = std::chrono::high_resolution_clock::now();

    const auto duration = std::chrono::duration_cast<
        std::chrono::milliseconds>(end - start).count();
    std::cout << "Execution time: " << duration << " ms\n";
}

int main(int argc, char* argv[]) {
    PokemonData pokemon_data;
    timer(
        [&pokemon_data] {
            pokemon_data = get_all_fully_evolved_pokemon();
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
