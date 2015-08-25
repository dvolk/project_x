#pragma once

#include <vector>

void init_weatherinfo(void);

struct WeatherInfo {
    const char *name;
    std::vector<int> bmaps;
    int fps;
    int duration;
    float encounter_range_mult;
};
