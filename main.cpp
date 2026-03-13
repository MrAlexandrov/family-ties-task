#include "graph.hpp"
#include "parser.hpp"

#include <iostream>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace {

using State = std::pair<models::Person*, models::Person*>;

std::vector<models::Person*>
TraversePath(models::Person* start, const std::string& path) {
    std::vector<State> states = {
        {start, nullptr}
    };

    for (char step : path) {
        std::vector<State> next;

        for (auto [cur, from] : states) {
            auto add = [&](models::Person* p) {
                if (p && p != from) {
                    next.push_back({p, cur});
                }
            };

            switch (step) {
            case 'P':
                add(cur->GetParents().first);
                add(cur->GetParents().second);
                break;

            case 'C':
                for (auto* child : cur->GetChildren()) {
                    add(child);
                }
                break;

            case 'S':
                add(cur->GetSpouse());
                break;

            case 'W': {
                auto* sp = cur->GetSpouse();
                if (sp && sp->GetGender() == models::EGender::Female) {
                    add(sp);
                }
                break;
            }

            case 'H': {
                auto* sp = cur->GetSpouse();
                if (sp && sp->GetGender() == models::EGender::Male) {
                    add(sp);
                }
                break;
            }

            default:
                break;
            }
        }

        states = std::move(next);
    }

    std::set<models::Person*> seen;
    std::vector<models::Person*> result;
    for (auto [person, _] : states) {
        if (person != start && seen.insert(person).second) {
            result.push_back(person);
        }
    }
    return result;
}

} // namespace

int main() {
    models::FamilyTree tree;

    try {
        parser::ParseInputFile("input.txt", tree);
    } catch (const std::exception& e) {
        std::cerr << "Ошибка парсинга input.txt: " << e.what() << "\n";
        return 1;
    }

    std::vector<parser::Relation> relations;
    try {
        relations = parser::ParseRelationsFile("relations.txt");
    } catch (const std::exception& e) {
        std::cerr << "Ошибка парсинга relations.txt: " << e.what() << "\n";
        return 1;
    }

    std::string query;
    std::cout << "Введите имя: ";
    std::getline(std::cin, query);

    while (!query.empty() && (query.back() == '\r' || query.back() == ' ')) {
        query.pop_back();
    }
    auto start_pos = query.find_first_not_of(' ');
    if (start_pos != std::string::npos) {
        query = query.substr(start_pos);
    }

    auto* person = tree.FindPerson(query);
    if (!person) {
        std::cout << "Человек \"" << query << "\" не найден.\n";
        return 1;
    }

    std::cout << "\nРодственники для " << query << ":\n";

    bool found_any = false;
    for (const auto& rel : relations) {
        auto found = TraversePath(person, rel.path);
        if (found.empty()) {
            std::cout << rel.male_term << " и " << rel.female_term << ": нет\n";
            continue;
        }

        for (auto* p : found) {
            const std::string& term = (p->GetGender() == models::EGender::Male)
                                          ? rel.male_term
                                          : rel.female_term;
            std::cout << term << ": " << p->GetName() << "\n";
            found_any = true;
        }
    }

    if (!found_any) {
        std::cout << "(родственников не найдено)\n";
    }

    return 0;
}
