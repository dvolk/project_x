#include "./fadetransitionui.h"

#include <allegro5/allegro.h>
#include <allegro5/allegro_color.h>
#include <allegro5/allegro_primitives.h>

#include "./config.h"
#include <cstdio>

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

ALLEGRO_DISPLAY *get_global_display(void);

void FadeTransitionUI::takeScreenshot(UI *ui) {
    scr = al_create_bitmap(config.displayX, config.displayY);
    if(scr == NULL) {
        al_set_new_bitmap_flags(ALLEGRO_MEMORY_BITMAP);
        scr = al_create_bitmap(config.displayX, config.displayY);
        al_set_new_bitmap_flags(ALLEGRO_VIDEO_BITMAP);
    }
    if(scr != NULL) {
        al_set_target_bitmap(scr);
        al_clear_to_color(ui->clear_to);
        ui->draw();
        al_set_target_backbuffer(get_global_display());
    }
}

void FadeTransitionUI::draw(void) {
    if(frame <= 0) {
        if(scr != NULL) {
            al_draw_bitmap(scr, 0, 0, 0);
        }
        else {
            al_clear_to_color(from->clear_to);
            from->draw();
        }
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
    if(frame <= 0)
        from->update();
    if(frame > 0)
        to->update();

    if(frame >= fade_frames) {
        widgets.clear();
        set_global_ui(to);
        if(scr != NULL) {
            al_destroy_bitmap(scr);
            scr = NULL;
        }
    }
}
