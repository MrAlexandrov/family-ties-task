#pragma once

#include "couple.hpp"
#include <list>
#include <string>
#include <unordered_map>

namespace models {

// Stable storage for Person and Couple objects (std::list never invalidates
// pointers on insertion). A hash map provides O(1) lookup by name.
class FamilyTree {
public:
    FamilyTree() = default;

    Person* AddPerson(const std::string& name, EGender gender) {
        persons_.emplace_back(name, gender);
        Person* ptr = &persons_.back();
        by_name_[name] = ptr;
        return ptr;
    }

    // Create a Couple from two existing persons and link them to each other.
    Couple* AddCouple(Person* p1, Person* p2) {
        couples_.push_back({p1, p2, {}});
        Couple* c = &couples_.back();
        p1->SetCouple(c);
        p2->SetCouple(c);
        return c;
    }

    Person* FindPerson(const std::string& name) const {
        auto it = by_name_.find(name);
        return it != by_name_.end() ? it->second : nullptr;
    }

    const std::unordered_map<std::string, Person*>& GetAll() const {
        return by_name_;
    }

private:
    std::list<Person> persons_;
    std::list<Couple> couples_;
    std::unordered_map<std::string, Person*> by_name_;
};

} // namespace models
