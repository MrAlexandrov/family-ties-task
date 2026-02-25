#pragma once

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace models {

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

    Person* GetSpouse() const;
    std::pair<Person*, Person*> GetParents() const;
    const std::vector<Person*>& GetChildren() const;

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

    Couple* couple_ = nullptr;
    Couple* parent_couple_ = nullptr;
};

} // namespace models
