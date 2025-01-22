#ifndef DEFENDER_TYPE_EFFECTIVENESS_DATA_SOURCE_H
#define DEFENDER_TYPE_EFFECTIVENESS_DATA_SOURCE_H

#include <map>
#include <vector>

#include "models.h"

typedef std::map<PokemonType, std::map<PokemonType, double>> DefenderTypeChart;
typedef std::vector<std::map<PokemonType, std::vector<PokemonType>>> JsonDefenderTypeChart;

DefenderTypeChart get_defender_type_chart();

#endif //DEFENDER_TYPE_EFFECTIVENESS_DATA_SOURCE_H
