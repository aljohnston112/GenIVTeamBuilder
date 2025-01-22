#include <fstream>
#include <iostream>

#include "type_chart_data_source.h"

DefenderTypeChart get_defender_type_chart() {
    std::ifstream file("../data/defender_types.json");
    if (!file.is_open()) {
        std::cerr << "Failed to open file." << std::endl;
    }

    std::string json_content(
        (std::istreambuf_iterator(file)),
        std::istreambuf_iterator<char>()
    );

    static JsonDefenderTypeChart pokemon_data;
    if (auto ec = glz::read_json<JsonDefenderTypeChart>(
            pokemon_data,
            json_content
        )
    ) {
        std::string descriptive_error = format_error(ec, json_content);
        std::cerr << descriptive_error << std::endl;
    }

    DefenderTypeChart defender_type_chart{};
    const auto& no_effect = pokemon_data[0];
    for (const auto& [defender_type, attacker_types]: no_effect) {
        defender_type_chart.emplace(defender_type, std::map<PokemonType, double>{});
        for (const auto& attacker_type: attacker_types) {
            defender_type_chart[defender_type].emplace(attacker_type, 0);
        }
    }
    const auto& little_effect = pokemon_data[1];
    for (const auto& [defender_type, attacker_types]: little_effect) {
        defender_type_chart.emplace(defender_type, std::map<PokemonType, double>{});
        for (const auto& attacker_type: attacker_types) {
            defender_type_chart[defender_type].emplace(attacker_type, 0.5);
        }
    }
    const auto& normal_effect = pokemon_data[2];
    for (const auto& [defender_type, attacker_types]: normal_effect) {
        defender_type_chart.emplace(defender_type, std::map<PokemonType, double>{});
        for (const auto& attacker_type: attacker_types) {
            defender_type_chart[defender_type].emplace(attacker_type, 1.0);
        }
    }
    const auto& super_effect = pokemon_data[3];
    for (const auto& [defender_type, attacker_types]: super_effect) {
        defender_type_chart.emplace(defender_type, std::map<PokemonType, double>{});
        for (const auto& attacker_type: attacker_types) {
            defender_type_chart[defender_type].emplace(attacker_type, 2.0);
        }
    }

    return std::move(defender_type_chart);
}
