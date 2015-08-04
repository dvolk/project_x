#pragma once

#include "./widget.h"

#include <vector>

struct UI {
    std::vector<Widget *> widgets;

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
