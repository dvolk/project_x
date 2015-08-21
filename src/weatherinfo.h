#pragma once

#include <vector>

struct WeatherInfo {
    const char *name;
    std::vector<int> bmaps;
    int fps;
    int duration;
};
