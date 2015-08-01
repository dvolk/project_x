#pragma once

#include <sstream>

struct Wound {
    int age;
    float severity;
    float bleeding;

    Wound();

    void save(std::ostream &os);
    void load(std::istream &is);

    void modify_severity(float amount);
    void modify_bleeding(float amount);
};
