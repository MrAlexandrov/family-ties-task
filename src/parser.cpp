#include "parser.hpp"

#include <fstream>
#include <stdexcept>
#include <string>

namespace parser {

namespace {

void TrimRight(std::string& s) {
    while (!s.empty() && s.back() == ' ') {
        s.pop_back();
    }
}

void TrimLeft(std::string& s) {
    auto pos = s.find_first_not_of(' ');
    s = (pos != std::string::npos) ? s.substr(pos) : "";
}

} // anonymous namespace

void ParseInputFile(const std::string& filename, models::FamilyTree& tree) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open: " + filename);
    }

    std::vector<std::string> names;
    std::vector<std::string> marriages;
    std::vector<std::string> children;

    std::string line;
    while (std::getline(file, line)) {
        TrimRight(line);
        if (line.empty() || line.front() == '#') {
            continue;
        }

        if (line.find("<->") != std::string::npos) {
            marriages.push_back(std::move(line));
        } else if (line.find("->") != std::string::npos) {
            children.push_back(std::move(line));
        } else if (line.find("(М)") != std::string::npos ||
                   line.find("(Ж)") != std::string::npos) {
            names.push_back(std::move(line));
        }
    }

    for (const auto& l : names) {
        auto paren = l.rfind(" (");
        if (paren == std::string::npos) {
            continue;
        }
        std::string name = l.substr(0, paren);
        bool is_male = (l.find("(М)") != std::string::npos);
        tree.AddPerson(
            name, is_male ? models::EGender::Male : models::EGender::Female
        );
    }

    for (const auto& l : marriages) {
        auto arrow = l.find("<->");
        std::string n1 = l.substr(0, arrow);
        std::string n2 = l.substr(arrow + 3);
        TrimRight(n1);
        TrimLeft(n2);

        auto* p1 = tree.FindPerson(n1);
        auto* p2 = tree.FindPerson(n2);
        if (p1 && p2) {
            tree.AddCouple(p1, p2);
        }
    }

    for (const auto& l : children) {
        auto arrow = l.find("->");
        std::string parent_name = l.substr(0, arrow);
        std::string child_name = l.substr(arrow + 2);
        TrimRight(parent_name);
        TrimLeft(child_name);

        auto* parent = tree.FindPerson(parent_name);
        auto* child = tree.FindPerson(child_name);
        if (!parent || !child) {
            continue;
        }

        auto* couple = parent->GetCouple();
        if (couple) {
            child->SetParentCouple(couple);
            couple->children.push_back(child);
        }
    }
}

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
