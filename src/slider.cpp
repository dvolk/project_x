#include "./slider.h"

#include "./colors.h"
#include "./fontmanager.h"
#include "./sound.h"

#include <allegro5/allegro.h>
#include <allegro5/allegro_color.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>

#include <stdio.h>

extern Colors colors;
extern ALLEGRO_FONT *g_font;

void Slider::apply(void) {
    *cfg_state = state;
}

void Slider::reset(void) {
    state = *cfg_state;
}

Slider::Slider(float x, float y, const char *name, float *cfg_state) {
    this->name = name;

    if(cfg_state == NULL) {
        state = false;
    } else {
        this->cfg_state = cfg_state;
        this->state = *cfg_state;
    }
    this->callback = nullptr;

    pos.x1 = x;
    pos.y1 = y;
    pos.x2 = size_x;
    pos.y2 = 16;

    mouseDownSound = SOUND_CLICK2;
}

void Slider::draw(void) {
    al_draw_rectangle(pos.x1, pos.y1, pos.x1 + pos.x2, pos.y1 + 16, colors.black, 1);
    al_draw_filled_rectangle(pos.x1, pos.y1, pos.x1 + state * size_x, pos.y1 + 16, colors.grey);

    if(name != NULL)
        al_draw_text(g_font, colors.black, pos.x1 + size_x + 4, pos.y1, 0, name);
}

extern float get_mouse_x(void);

void Slider::mouseDown(void) {
    if(mouseDownSound != SOUND_NONE)
        play_ui_sound(mouseDownSound);

    state = (get_mouse_x() - pos.x1) / size_x;
    if(state < 0.1)
        state = 0.0;
    if(state > 0.9)
        state = 1.0;

    if(callback != NULL)
        callback(state);

    printf("Slider::mouseDown state: %f\n", state);
}

