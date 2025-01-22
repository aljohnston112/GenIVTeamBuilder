#include <fstream>
#include <iostream>
#include <ostream>

#include <glaze/json/read.hpp>

#include "pokemon_data_source.h"

PokemonData& get_all_fully_evolved_pokemon(const std::list<int>& fully_evolved_pokemon_indices) {
    std::ifstream file("../data/all_pokemon.json");
    if (!file.is_open()) {
        std::cerr << "Failed to open file." << std::endl;
    }

    std::string json_content(
        (std::istreambuf_iterator(file)),
        std::istreambuf_iterator<char>()
    );

    static PokemonData pokemon_data;
    if (auto ec = glz::read_json<PokemonData>(
            pokemon_data,
            json_content
        )
    ) {
        std::string descriptive_error = format_error(ec, json_content);
        std::cerr << descriptive_error << std::endl;
    }

    for (int i = 0; i < 494; i++) {
        if (std::ranges::find(fully_evolved_pokemon_indices, i) == fully_evolved_pokemon_indices.end()) {
            pokemon_data.erase(i);
        }
    }

    return pokemon_data;
}
