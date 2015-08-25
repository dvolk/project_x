#include "./weatherinfo.h"

std::vector<WeatherInfo> * get_global_weatherinfo(void);

void init_weatherinfo(void) {
    WeatherInfo clear;
    clear.name = "Clear";
    clear.bmaps = { -1 }; // -1: no bitmaps
    clear.fps = 1;
    clear.duration = 7000;
    clear.encounter_range_mult = 1;

    WeatherInfo raining;
    raining.name = "Raining";
    raining.bmaps = { 118, 119 };
    raining.fps = 10;
    raining.duration = 3000;
    raining.encounter_range_mult = 0.7;

    WeatherInfo foggy;
    foggy.name = "Fog";
    foggy.bmaps = { 122 };
    foggy.fps = 1;
    foggy.duration = 2000;
    foggy.encounter_range_mult = 0.5;

    get_global_weatherinfo()->push_back(clear);
    get_global_weatherinfo()->push_back(raining);
    get_global_weatherinfo()->push_back(foggy);
}
