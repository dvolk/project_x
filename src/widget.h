#pragma once

#include "./rect.h"

struct Widget {
    Rect pos;
    bool visible;

    void (*onMouseDown)(void);
    void (*onMouseUp)(void);
    void (*onKeyDown)(void);

    virtual void mouseDown(void) { };
    virtual void mouseUp(void) { };
    virtual void keyDown(void) { };
    virtual void hoverOver(void) { };

    Widget();
    virtual ~Widget();

    virtual void draw(void) { };
    virtual void update(void) { };
};
