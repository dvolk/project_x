#pragma once

#include "./widget.h"
#include "./sound.h"

struct ResolutionData {
    int x, y;
};

/*
  Checkbox with text to the right of it
*/
struct LabelledCheckBox : public Widget {
    const char *name;
    bool *cfg_state;
    bool state;

    ResolutionData res_data; // TODO: mmm

    enum UI_SOUND mouseDownSound;

    LabelledCheckBox(float x, float y, const char *name, bool *cfg_state);

    void draw(void) override;
    void mouseDown(void) override;

    void (*callback)(void);

    void apply(void);
    void reset(void);
};
