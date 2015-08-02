#pragma once

#include <cstdint>

struct Config {
    int8_t frame_rate;
    bool vsync;
    bool fullscreen;
    bool resolutionScaling;
    bool debugVisibility;
    bool playerInvulnerable;
    int16_t displayX;
    int16_t displayY;
    char *font_filename;
    int8_t font_height;
    bool sorting;
    int8_t aa_samples;
    bool start_nag;
    bool ui_fading;
    bool alt_grid_movement;
    bool setClipRectangle;
    bool custom_cursor;
    bool playMusic;
    bool showFPS;
    float musicVolume;

    void save(const char *filename);
    void load(const char *filename);
};
