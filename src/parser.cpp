#include "parser.hpp"

#include <fstream>
#include <stdexcept>
#include <string>

namespace parser {

namespace {

// Remove trailing CR and spaces (handles Windows line endings too).
void TrimRight(std::string& s) {
    while (!s.empty() && (s.back() == '\r' || s.back() == ' ')) {
        s.pop_back();
    }
}

// Remove leading spaces.
void TrimLeft(std::string& s) {
    auto pos = s.find_first_not_of(' ');
    s = (pos != std::string::npos) ? s.substr(pos) : "";
}

} // anonymous namespace

// ---------------------------------------------------------------------------
// input.txt parser
// ---------------------------------------------------------------------------
// Each non-comment line is classified by its content pattern alone:
//   "(М)" or "(Ж)"  →  person name with gender
//   "<->"           →  marriage  (checked before "->" to avoid false match)
//   "->"            →  parent-child relation
//
// Lines are collected into three buckets in a single pass, then processed in
// the required dependency order: names first, then marriages (to build
// Couple objects), then parent-child links. This means the relation entries
// can live in any order in the file, or even in a separate file entirely.
// ---------------------------------------------------------------------------

void ParseInputFile(const std::string& filename, models::FamilyTree& tree) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open: " + filename);
    }

    std::vector<std::string> names;
    std::vector<std::string> marriages;
    std::vector<std::string> children;

    // Single pass: classify each line by pattern.
    std::string line;
    while (std::getline(file, line)) {
        TrimRight(line);
        if (line.empty() || line.front() == '#') continue;

        // "<->" must be checked before "->" because "<->" contains "->".
        if (line.find("<->") != std::string::npos) {
            marriages.push_back(std::move(line));
        } else if (line.find("->") != std::string::npos) {
            children.push_back(std::move(line));
        } else if (line.find("(М)") != std::string::npos ||
                   line.find("(Ж)") != std::string::npos) {
            names.push_back(std::move(line));
        }
        // Lines that match none of the above are silently ignored.
    }

    // Pass 1: add all persons so that subsequent lookups always succeed.
    for (const auto& l : names) {
        auto paren = l.rfind(" (");
        if (paren == std::string::npos) continue;
        std::string name = l.substr(0, paren);
        bool is_male = (l.find("(М)") != std::string::npos);
        tree.AddPerson(name, is_male ? models::EGender::Male
                                     : models::EGender::Female);
    }

    // Pass 2: create Couple objects for every married pair.
    for (const auto& l : marriages) {
        auto arrow = l.find("<->");
        std::string n1 = l.substr(0, arrow);
        std::string n2 = l.substr(arrow + 3);
        TrimRight(n1);
        TrimLeft(n2);

        auto* p1 = tree.FindPerson(n1);
        auto* p2 = tree.FindPerson(n2);
        if (p1 && p2) tree.AddCouple(p1, p2);
    }

    // Pass 3: link children to their parents' shared Couple.
    for (const auto& l : children) {
        auto arrow = l.find("->");
        std::string parent_name = l.substr(0, arrow);
        std::string child_name  = l.substr(arrow + 2);
        TrimRight(parent_name);
        TrimLeft(child_name);

        auto* parent = tree.FindPerson(parent_name);
        auto* child  = tree.FindPerson(child_name);
        if (!parent || !child) continue;

        auto* couple = parent->GetCouple();
        if (couple) {
            child->SetParentCouple(couple);
            couple->children.push_back(child);
        }
    }
}

// ---------------------------------------------------------------------------
// relations.txt parser
// ---------------------------------------------------------------------------
// Each non-empty line: "PATH (male_term|female_term)"
// Empty lines separate logical groups but are not meaningful for parsing.
// ---------------------------------------------------------------------------

std::vector<Relation> ParseRelationsFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open: " + filename);
    }

    std::vector<Relation> relations;
    std::string line;

    while (std::getline(file, line)) {
        TrimRight(line);
        if (line.empty()) {
            continue;
        }

        // Split on first space: PATH and "(male|female)"
        auto space = line.find(' ');
        if (space == std::string::npos) {
            continue;
        }

        std::string path = line.substr(0, space);

        auto open = line.find('(', space);
        auto close = line.find(')', open != std::string::npos ? open : 0);
        if (open == std::string::npos || close == std::string::npos) {
            continue;
        }

        std::string terms = line.substr(open + 1, close - open - 1);

        auto pipe = terms.find('|');
        if (pipe == std::string::npos) {
            continue;
        }

        std::string male_term = terms.substr(0, pipe);
        std::string female_term = terms.substr(pipe + 1);

        relations.push_back(
            {std::move(path), std::move(male_term), std::move(female_term)}
        );
    }

    return relations;
}

} // namespace parser
