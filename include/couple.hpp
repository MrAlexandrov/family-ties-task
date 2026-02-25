#pragma once

// couple.hpp must be included by any file that calls Person::GetSpouse(),
// GetParents(), or GetChildren(), because those method bodies live here —
// they need the full definition of Couple to be compiled.

#include "person.hpp"
#include <vector>

namespace models {

// A married pair with their shared list of children.
// Using a struct with public members because FamilyTree and the parser need
// to push into children directly; no invariants to enforce here.
struct Couple {
    Person* first = nullptr;
    Person* second = nullptr;
    std::vector<Person*> children;

    // Returns the partner of `p`, or nullptr if `p` is not in this couple.
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

// -----------------------------------------------------------------------
// Inline bodies of Person's smart accessors (declared in person.hpp).
// Placed here so they can reference the fully-defined Couple.
// -----------------------------------------------------------------------

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
