#pragma once

#include "./widget.h"

#include <allegro5/allegro.h>

struct BarIndicator : public Widget {
    float *quantity;
    ALLEGRO_BITMAP *up;
    ALLEGRO_BITMAP *bars;
    const char *indicator_name;

    BarIndicator();
    ~BarIndicator() { };

    void mouseDown(void) { };
    void mouseUp(void) { };
    void keyDown(void) { };
    void hoverOver(void) { };

    void press(void) { };
    void draw(void);
    void update(void) { };
};
