#pragma once

#include <allegro5/allegro.h>

struct GridInfo {
    ALLEGRO_BITMAP *sprite;
    int16_t maxItems;
    bool noGrid;
    bool canHoldLiquid;
    bool vehiclepoint;
    bool medical;
    bool visible;
    bool pleaseRotateIt;

    GridInfo();
};
