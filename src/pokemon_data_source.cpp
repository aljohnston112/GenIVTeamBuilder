#include <fstream>
#include <iostream>
#include <ostream>

#include <glaze/json/read.hpp>

#include "pokemon_data_source.h"

#include <bitset>
#include <mutex>

static constexpr unsigned short NUMBER_OF_POKEMON = 494;

static constexpr std::bitset<NUMBER_OF_POKEMON>
get_fully_evolved_pokemon_indices() {
    std::bitset<NUMBER_OF_POKEMON> indices{};
    for (const unsigned short i : {
             12, 15, 18, 20, 22, 24, 26, 28, 31, 34, 36, 38, 40, 45, 47, 49, 51,
             53, 55, 57, 59, 62, 65, 68, 71, 73, 76, 78, 80, 82, 83, 85, 87, 89,
             91, 94, 97, 99, 101, 103, 105, 106, 107, 110, 115, 119, 121, 122,
             124, 127, 128, 130, 131, 132, 134, 135, 136, 139, 141, 142, 143,
             162, 164, 166, 168, 169, 171, 178, 181, 182, 184, 185, 186, 189,
             192, 195, 196, 197, 199, 201, 202, 203, 205, 206, 208, 210, 211,
             212, 213, 214, 217, 219, 222, 224, 225, 226, 227, 229, 230, 232,
             234, 235, 237, 241, 242, 262, 264, 267, 269, 272, 275, 277, 279,
             282, 284, 286, 288, 289, 290, 291, 292, 295, 297, 301, 302, 303,
             306, 308, 310, 311, 312, 313, 314, 317, 319, 321, 323, 324, 326,
             327, 332, 334, 335, 336, 337, 338, 340, 342, 344, 346, 348, 350,
             351, 352, 354, 357, 358, 359, 362, 365, 367, 368, 369, 370, 389,
             392, 395, 398, 400, 402, 405, 407, 409, 411, 413, 414, 416, 417,
             419, 421, 423, 424, 426, 428, 429, 430, 432, 435, 437, 441, 442,
             448, 450, 452, 454, 455, 457, 460, 461, 462, 463, 464, 465, 466,
             467, 468, 469, 470, 471, 472, 473, 474, 475, 476, 477, 478, 479
         }) {
        indices.set(i);
    }
    return indices;
}

PokemonData& get_all_fully_evolved_pokemon() {
    static std::once_flag init_flag;
    static PokemonData pokemon_data;

    std::call_once(init_flag, [] {
        std::ifstream file("../data/all_pokemon.json");
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open all_pokemon.json");
        }

        const std::string json_content(
            (std::istreambuf_iterator(file)),
            std::istreambuf_iterator<char>()
        );

        if (auto ec = glz::read_json<PokemonData>(
                pokemon_data,
                json_content
            )
        ) {
            std::string descriptive_error = format_error(ec, json_content);
            std::cerr << descriptive_error << std::endl;
        }

        constexpr auto indices =
            get_fully_evolved_pokemon_indices();
        for (int i = 0; i < NUMBER_OF_POKEMON; i++) {
            if (!indices.test(i)) {
                pokemon_data.erase(i);
            }
        }
    });

    return pokemon_data;
}
