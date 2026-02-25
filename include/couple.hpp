#pragma once

#include "person.hpp"
#include <vector>

namespace models {

struct Couple {
    Person* first = nullptr;
    Person* second = nullptr;
    std::vector<Person*> children;

    Person* GetSpouseOf(const Person* p) const {
        if (p == first) {
            return second;
        }
        if (p == second) {
            return first;
        }
        return nullptr;
    }
};

inline Person* Person::GetSpouse() const {
    return couple_ ? couple_->GetSpouseOf(this) : nullptr;
}

inline std::pair<Person*, Person*> Person::GetParents() const {
    if (!parent_couple_) {
        return {nullptr, nullptr};
    }
    return {parent_couple_->first, parent_couple_->second};
}

inline const std::vector<Person*>& Person::GetChildren() const {
    static const std::vector<Person*> empty;
    return couple_ ? couple_->children : empty;
}

} // namespace models
