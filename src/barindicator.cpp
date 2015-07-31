#include "./barindicator.h"

#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>

#include "./fontmanager.h"
#include "./colors.h"

extern Colors colors;
extern ALLEGRO_FONT *g_font;

BarIndicator::BarIndicator() {
    up = NULL;
    quantity = NULL;
}

void BarIndicator::draw(void) {
    if(up != NULL && quantity != NULL && bars != NULL) {
        al_draw_text(g_font, colors.white, pos.x1 + 2, pos.y1 - 6, 0, indicator_name);
        al_draw_bitmap(up, pos.x1, pos.y1 + 10, 0);
        al_draw_bitmap_region(bars, 0, 0, *quantity * pos.x2, pos.y2, pos.x1, pos.y1 + 10, 0);
    }
}
