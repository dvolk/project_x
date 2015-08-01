#include "./labelledcheckbox.h"

#include "./colors.h"
#include "./fontmanager.h"

#include <allegro5/allegro.h>
#include <allegro5/allegro_color.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>

extern Colors colors;
extern ALLEGRO_FONT *g_font;

void LabelledCheckBox::apply(void) {
    *cfg_state = state;
}

void LabelledCheckBox::reset(void) {
    state = *cfg_state;
}

LabelledCheckBox::LabelledCheckBox(float x, float y, const char *name, bool *cfg_state) {
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
    pos.x2 = 16;
    pos.y2 = 16;
}

void LabelledCheckBox::draw(void) {
    if(state == true)
        al_draw_filled_rectangle(pos.x1, pos.y1, pos.x1 + 16, pos.y1 + 16, colors.black);
    else
        al_draw_rectangle(pos.x1, pos.y1, pos.x1 + 16, pos.y1 + 16, colors.black, 1);

    // TODO or draw cross?
    // al_draw_line(pos.x1, pos.y1, pos.x1 + pos.x2, pos.y1 + pos.y2, colors.black, 1);
    // al_draw_line(pos.x1, pos.y1 + pos.y2, pos.x1 + pos.x2, pos.y1, colors.black, 1);

    al_draw_text(g_font, colors.black, pos.x1 + 20, pos.y1, 0, name);
}

void LabelledCheckBox::mouseDown(void) {
    if(callback != nullptr)
        callback();

    state = !state;
}

