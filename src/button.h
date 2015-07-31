#pragma once

#include "./widget.h"

#include <allegro5/allegro.h>

struct Button : public Widget {
    bool pressed;
    ALLEGRO_BITMAP *up;
    ALLEGRO_BITMAP *down;
    const char *name;
    int name_len;

    Button(void);
    Button(const char *_name);
    ~Button();

    void mouseDown(void);
    void mouseUp(void);
    void keyDown(void);
    void hoverOver(void);

    void press(void);
    void draw(void);
    void update(void);
};
