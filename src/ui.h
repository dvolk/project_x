#pragma once

#include "./widget.h"

#include <vector>

#include <allegro5/allegro.h>
#include <allegro5/allegro_color.h>

struct UI {
    std::vector<Widget *> widgets;

    ALLEGRO_COLOR clear_to;

    virtual ~UI() {
        // info("~UI()");
    }

    void mouseDownEvent(void);
    void mouseUpEvent(void);
    void keyDownEvent(void);
    void hoverOverEvent(void);

    void update(void);
    virtual void draw(void);

    /*
      defined in main.cpp
     */
    void addIndicatorWidgets(void);
    void addLogAndButtons(void);
};
