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
// Format:
//   # Имена:
//   Name (М)           <- male
//   Name (Ж)           <- female
//   ...
//   # Кто на ком женат:
//   Name1 <-> Name2
//   ...
//   # Кто чей ребёнок:
//   Parent -> Child
//   ...
// ---------------------------------------------------------------------------

void ParseInputFile(const std::string& filename, models::FamilyTree& tree) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open: " + filename);
    }

    enum class Section {
        None,
        Names,
        Marriages,
        Children
    };
    Section section = Section::None;
    std::string line;

    while (std::getline(file, line)) {
        TrimRight(line);
        if (line.empty()) {
            continue;
        }

        // Section headers (UTF-8 Cyrillic string literals, source is UTF-8)
        if (line.find("# Имена") != std::string::npos) {
            section = Section::Names;
            continue;
        }
        if (line.find("# Кто на ком женат") != std::string::npos) {
            section = Section::Marriages;
            continue;
        }
        if (line.find("# Кто чей ребёнок") != std::string::npos) {
            section = Section::Children;
            continue;
        }
        if (line.front() == '#') {
            continue; // other comment lines
        }

        // ---- Names ----
        // Line format: "Name (М)" or "Name (Ж)"
        if (section == Section::Names) {
            // Find the last " (" to split name from gender annotation.
            auto paren = line.rfind(" (");
            if (paren == std::string::npos) {
                continue;
            }
            std::string name = line.substr(0, paren);
            // "М" is U+041C, UTF-8: 0xD0 0x9C
            bool is_male = (line.find("(М)") != std::string::npos);
            tree.AddPerson(
                name, is_male ? models::EGender::Male : models::EGender::Female
            );
        }

        // ---- Marriages ----
        // Line format: "Name1 <-> Name2"
        else if (section == Section::Marriages) {
            auto arrow = line.find("<->");
            if (arrow == std::string::npos) {
                continue;
            }

            std::string n1 = line.substr(0, arrow);
            std::string n2 = line.substr(arrow + 3);
            TrimRight(n1);
            TrimLeft(n2);

            auto* p1 = tree.FindPerson(n1);
            auto* p2 = tree.FindPerson(n2);
            if (p1 && p2) {
                tree.AddCouple(p1, p2);
            }
        }

        // ---- Parent-child ----
        // Line format: "Parent -> Child"
        else if (section == Section::Children) {
            auto arrow = line.find("->");
            if (arrow == std::string::npos) {
                continue;
            }

            std::string parent_name = line.substr(0, arrow);
            std::string child_name = line.substr(arrow + 2);
            TrimRight(parent_name);
            TrimLeft(child_name);

            auto* parent = tree.FindPerson(parent_name);
            auto* child = tree.FindPerson(child_name);
            if (!parent || !child) {
                continue;
            }

            // The couple owns the shared children vector; the child stores a
            // back-pointer to that couple so GetParents() and GetChildren()
            // both work without any duplication.
            auto* couple = parent->GetCouple();
            if (couple) {
                child->SetParentCouple(couple);
                couple->children.push_back(child);
            }
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
