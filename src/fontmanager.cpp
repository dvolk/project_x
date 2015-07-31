#include "./fontmanager.h"

#include "./util.h"

#include <string>

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>

extern ALLEGRO_FONT *g_font;

void font_load(const char *filepath, int height) {
    g_font = al_load_font(filepath, height, 0);
    if(g_font == NULL)
        errorQuit("failed to load font: " + std::string(filepath));
    else
        info("Loaded font: " + std::string(filepath));
}

void font_unload(void) {
    al_destroy_font(g_font);
}
