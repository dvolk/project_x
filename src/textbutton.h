#pragma once

#include "./widget.h"
#include "./sound.h"

/*
  Button with text in it
 */
struct TextButton : public Widget {
    float text_offset_x;
    float text_offset_y;
    const char *name;

    enum UI_SOUND mouseDownSound;

    TextButton(const char *name, float x, float y, float sx, float sy);

    void draw(void) override;
    void hoverOver(void) override;
    void mouseDown(void) override;
};
