#pragma once

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace models {

// Forward declaration — full definition is in couple.hpp.
struct Couple;

enum class EGender : std::uint8_t {
    Male,
    Female
};

class Person {
public:
    explicit Person(const std::string& name, EGender gender)
        : name_(name), gender_(gender) {}

    const std::string& GetName() const {
        return name_;
    }
    EGender GetGender() const {
        return gender_;
    }

    // -----------------------------------------------------------------------
    // Smart accessors — bodies are defined in couple.hpp (after Couple is
    // fully declared) to break the circular dependency.
    // -----------------------------------------------------------------------
    Person* GetSpouse() const;
    std::pair<Person*, Person*> GetParents() const;
    const std::vector<Person*>& GetChildren() const;

    // Raw couple-pointer access used by FamilyTree and the parser.
    void SetCouple(Couple* c) {
        couple_ = c;
    }
    void SetParentCouple(Couple* c) {
        parent_couple_ = c;
    }
    Couple* GetCouple() const {
        return couple_;
    }
    Couple* GetParentCouple() const {
        return parent_couple_;
    }

private:
    const std::string name_;
    const EGender gender_;

    // Points to the Couple this person belongs to as a partner (null if
    // single).
    Couple* couple_ = nullptr;

    // Points to the Couple that are this person's parents (null if unknown).
    Couple* parent_couple_ = nullptr;
};

} // namespace models
