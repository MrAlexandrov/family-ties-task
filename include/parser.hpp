#pragma once

#include "graph.hpp"
#include <string>
#include <vector>

namespace parser {

struct Relation {
    std::string path;
    std::string male_term;
    std::string female_term;
};

void ParseInputFile(const std::string& filename, models::FamilyTree& tree);

std::vector<Relation> ParseRelationsFile(const std::string& filename);

} // namespace parser
