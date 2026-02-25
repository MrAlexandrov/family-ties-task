#pragma once

#include "graph.hpp"
#include <string>
#include <vector>

namespace parser {

// A relation defined in relations.txt.
// path: traversal code using P/C/S/W/H letters.
// male_term / female_term: Russian kinship word based on found person's gender.
struct Relation {
    std::string path;
    std::string male_term;
    std::string female_term;
};

// Parse input.txt: names, marriages, parent-child links into tree.
// Marriages must be parsed before children so spouse pointers are available.
void ParseInputFile(const std::string& filename, models::FamilyTree& tree);

// Parse relations.txt: each non-empty line -> one Relation entry.
std::vector<Relation> ParseRelationsFile(const std::string& filename);

} // namespace parser
