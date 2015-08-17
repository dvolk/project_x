#include "./fadetransitionui.h"

#include <allegro5/allegro.h>
#include <allegro5/allegro_color.h>
#include <allegro5/allegro_primitives.h>

#include "./config.h"

extern Config config;
extern void set_global_ui(UI *ui);

void FadeTransitionUI::set_duration(float s) {
    fade_frames = s * (float)config.frame_rate / 2.0;
}

void FadeTransitionUI::start(UI *f, UI *t) {
    from = f;
    to = t;

    if(from != NULL)
        frame = - fade_frames;
    else
        frame = 0;

    upc.parent = this;
    widgets.push_back(&upc);
}

void FadeTransitionUI::draw(void) {
    if(frame <= 0) {
        al_clear_to_color(from->clear_to);
        from->draw();
        float alpha = 1.0 - ((-frame) / (float)fade_frames);
        al_draw_filled_rectangle(0, 0, config.displayX, config.displayY,
                                 al_map_rgba_f(0, 0, 0, alpha));
    }
    else if(frame > 0) {
        al_clear_to_color(to->clear_to);
        to->draw();
        float alpha = 1.0 - (frame / (float)fade_frames);
        al_draw_filled_rectangle(0, 0, config.displayX, config.displayY,
                                 al_map_rgba_f(0, 0, 0, alpha));
    }
}

void FadeTransitionUI::updateFrame(void) {
    frame++;

    if(frame >= fade_frames) {
        widgets.clear();
        set_global_ui(to);
    }
}
