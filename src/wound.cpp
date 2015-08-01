#include "./wound.h"

#include <algorithm>

Wound::Wound() {
    severity = 0;
    bleeding = 0;
    age = 0;
}

void Wound::modify_severity(float amount) {
    severity = std::max((float)0.0, severity + amount);

    // start bleeding at wound severity 0.005
    bleeding = std::max(0.0, severity - 0.005);

    if(severity < 0.001) {
        severity = 0;
        age = 0;
    }
}

void Wound::modify_bleeding(float amount) {
    bleeding = std::max((float)0.0, bleeding + amount);
}

void Wound::save(std::ostream &os) {
    os << age << ' ' << severity << ' ' << bleeding << ' ';
}
void Wound::load(std::istream &is) {
    is >> age >> severity >> bleeding;
}
