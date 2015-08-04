#include "./textbutton.h"

#include "./colors.h"
#include "./config.h"
#include "./fontmanager.h"
#include "./sound.h"

#include <algorithm>

#include <allegro5/allegro.h>
#include <allegro5/allegro_color.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>

extern Colors colors;
extern ALLEGRO_FONT *g_font;
extern Config config;

TextButton::TextButton(const char *name, float x, float y, float sx, float sy) {
    this->name = name;
    pos.x1 = x;
    pos.y1 = y;
    pos.x2 = sx;
    pos.y2 = sy;
    text_offset_x = round((pos.x2 - al_get_text_width(g_font, name)) / 2);
    text_offset_y = round((pos.y2 - config.font_height) / 2);
    mouseDownSound = SOUND_CLICK2;
}

void TextButton::draw(void) {
    al_draw_filled_rectangle(pos.x1, pos.y1, pos.x1 + pos.x2,
                             pos.y1 + pos.y2, colors.darkgrey);
    al_draw_rectangle(pos.x1, pos.y1, pos.x1 + pos.x2, pos.y1 + pos.y2,
                      colors.black, 1);
    al_draw_text(g_font, colors.white,
                 pos.x1 + text_offset_x, pos.y1 + text_offset_y, 0, name);
}

void TextButton::hoverOver(void) {
    al_draw_filled_rectangle(pos.x1, pos.y1, pos.x1 + pos.x2,
                             pos.y1 + pos.y2, colors.grey2);
    al_draw_rectangle(pos.x1, pos.y1, pos.x1 + pos.x2, pos.y1 + pos.y2,
                      colors.black, 1);
    al_draw_text(g_font, colors.white,
                 pos.x1 + text_offset_x, pos.y1 + text_offset_y, 0, name);
}

void TextButton::mouseDown(void) {
    if(mouseDownSound != SOUND_NONE) {
        play_ui_sound(mouseDownSound);
    }
}
