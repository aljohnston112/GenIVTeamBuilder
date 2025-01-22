#ifndef MODELS_H
#define MODELS_H

#include <glaze/glaze.hpp>

#include <map>
#include <optional>
#include <string>
#include <vector>

enum class PokemonType {
    NORMAL,
    FIGHTING,
    FLYING,
    POISON,
    GROUND,
    ROCK,
    BUG,
    GHOST,
    STEEL,
    FIRE,
    WATER,
    GRASS,
    ELECTRIC,
    PSYCHIC,
    ICE,
    DRAGON,
    DARK
};

template <>
struct glz::meta<PokemonType> {
    static constexpr auto value = enumerate(
        "normal", PokemonType::NORMAL,
        "fighting", PokemonType::FIGHTING,
        "flying", PokemonType::FLYING,
        "poison", PokemonType::POISON,
        "ground", PokemonType::GROUND,
        "rock", PokemonType::ROCK,
        "bug", PokemonType::BUG,
        "ghost", PokemonType::GHOST,
        "steel", PokemonType::STEEL,
        "fire", PokemonType::FIRE,
        "water", PokemonType::WATER,
        "grass", PokemonType::GRASS,
        "electric", PokemonType::ELECTRIC,
        "psychic", PokemonType::PSYCHIC,
        "ice", PokemonType::ICE,
        "dragon", PokemonType::DRAGON,
        "dark", PokemonType::DARK
    );
};

enum class Category {
    PHYSICAL,
    SPECIAL,
    STATUS
};

template <>
struct glz::meta<Category> {
    static constexpr auto value = enumerate(
        "physical", Category::PHYSICAL,
        "special", Category::SPECIAL,
        "status", Category::STATUS
    );
};


struct Attack {
    std::string name;
    PokemonType pokemon_type;
    Category category;
    int power;
    int accuracy;
    int effect_percent;
};

struct PokemonInformation {
    std::string name;
    std::vector<PokemonType> pokemon_types;
    int id;
    std::string ability;
    float pounds;
};

struct Stats {
    std::string name;
    int health;
    int attack;
    int defense;
    int special_attack;
    int special_defense;
    int speed;
};

struct BaseStats {
    std::string name;
    Stats stats;
};

struct AllStats {
    std::string name;
    BaseStats base_stats;
    Stats level_50_min_stats;
    Stats level_50_max_stats;
    Stats level_100_min_stats;
    Stats level_100_max_stats;
};


struct Pokemon {
    PokemonInformation pokemon_information;
    AllStats all_stats;

    std::map<int, std::vector<Attack>> level_to_attacks;
    std::optional<std::map<std::string, Attack>> tm_or_hm_to_attack;

    std::optional<std::vector<Attack>> egg_moves;
    std::optional<std::map<int, std::map<int, std::vector<Attack>>>> pre_evolution_index_to_level_to_moves;
    std::optional<std::vector<Attack>> move_tutor_attacks;

    std::optional<std::map<std::string, std::map<int, std::vector<Attack>>>> game_to_level_to_moves;
    std::optional<std::vector<Attack>> special_moves;

    std::optional<std::map<std::string, AllStats>> form_to_all_stats;
    std::optional<std::map<std::string, std::map<int, std::vector<Attack>>>> form_to_level_up_attacks;
    std::optional<std::map<std::string, std::map<std::string, Attack>>> form_to_tm_or_hm_to_attack;
    std::optional<std::map<std::string, std::vector<Attack>>> form_to_move_tutor_attacks;
};

typedef std::map<int, Pokemon> PokemonData;

struct PokemonState {
    bool with_max_stats;
    std::reference_wrapper<const Pokemon> pokemon;

    int damage = 0;
    bool is_burned = false;
    bool has_reflect_up = false;
    bool has_light_screen_up = false;
    bool is_flash_fire_activated = false;

    [[nodiscard]] int hp_left() const {
        if (with_max_stats) {
            return pokemon.get().all_stats.level_50_max_stats.health - damage;
        }
        return pokemon.get().all_stats.level_50_min_stats.health - damage;
    }

    bool add_damage(const int damage) {
        this->damage += damage;
        return hp_left() > 0;
    }
};

struct BattleState {
    PokemonState pokemon0;
    PokemonState pokemon1;

    bool is_raining = false;
    bool is_sunny = false;
    bool is_sandstorming = false;
    bool is_hailing = false;
    bool is_foggy = false;
};



#endif //MODELS_H
