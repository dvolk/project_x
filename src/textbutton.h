#pragma once

#include "./widget.h"

/*
  Button with text in it
 */
struct TextButton : public Widget {
    float text_offset_x;
    float text_offset_y;
    const char *name;

    TextButton(const char *name, float x, float y, float sx, float sy);

    void draw(void) override;
    void hoverOver(void) override;
};
