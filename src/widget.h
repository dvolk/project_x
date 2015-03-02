#pragma once

#include "./rect.h"

struct Widget {
    Rect pos;
    bool visible;

    void (*onMouseDown)(void);
    void (*onMouseUp)(void);
    void (*onKeyDown)(void);

    virtual void mouseDown(void) = 0;
    virtual void mouseUp(void) = 0;
    virtual void keyDown(void) = 0;
    virtual void hoverOver(void) { };

    Widget();
    virtual ~Widget();

    virtual void draw(void) = 0;
    virtual void update() { };
};
