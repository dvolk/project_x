#pragma once

#include "./ui.h"
#include "./widget.h"

/*
  Fades out from the `from' UI into the `to' UI

  If from is NULL, it only fades-in
 */
struct FadeTransitionUI : public UI {
    UI *from;
    UI *to;
    int fade_frames = 30;
    int frame;

    void start(UI *f, UI *t);

    void draw(void) override;
    void updateFrame(void);
    void set_duration(float s);

    // a widget whose only purpose is to call FadeTransitionUI::updateFrame
    /*
      TODO: is this better than making UI::update virtual?
    */
    struct UpdateCaller : public Widget {
        FadeTransitionUI *parent;
        UpdateCaller() { visible = false; }
        void update(void) override { parent->updateFrame(); }
    };
    UpdateCaller upc;
};

