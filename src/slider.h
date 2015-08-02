#pragma once

#include "./widget.h"

/*
  Checkbox with text to the right of it
*/
struct Slider : public Widget {
    const char *name;
    float *cfg_state;
    float state;

    const float size_x = 100;

    Slider(float x, float y, const char *name, float *cfg_state);

    void draw(void) override;
    void mouseDown(void) override;

    void (*callback)(float state);

    void apply(void);
    void reset(void);
};
