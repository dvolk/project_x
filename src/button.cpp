#include "./button.h"

#include "./fontmanager.h"
#include "./colors.h"
#include "./sound.h"

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>

extern ALLEGRO_FONT *g_font;
extern Colors colors;

Button::Button(void) {
    name = NULL;
    pressed = false;
    up = NULL;
    down = NULL;
}

Button::Button(const char *_name) {
    name = _name;
    name_len = 16 + al_get_text_width(g_font, _name);
    pressed = false;
    up = NULL;
    down = NULL;
    mouseDownSound = SOUND_CLICK2;
}

void Button::mouseUp(void) {

}

void Button::keyDown(void) {

}

void Button::hoverOver(void) {
    if(name != NULL) {
        al_draw_filled_rectangle(pos.x1, pos.y1, pos.x1 + name_len, pos.y1 + 30, colors.black);
        al_draw_text(g_font, colors.white, pos.x1 + 8, pos.y1 + 8, 0, name);
    }
}

void Button::mouseDown(void) {
    if(mouseDownSound != SOUND_NONE) {
        play_ui_sound(mouseDownSound);
    }
}

Button::~Button(void) {
    // info("~Button()");
}

void Button::draw(void) {
    if(pressed == true && down != NULL)
        al_draw_bitmap(down, pos.x1, pos.y1, 0);
    else if(up != NULL)
        al_draw_bitmap(up, pos.x1, pos.y1, 0);
    else
        al_draw_filled_rectangle(pos.x1, pos.y1,
                                 pos.x1 + pos.x2,
                                 pos.y1 + pos.y2,
                                 colors.black);
}

void Button::update() {
}
