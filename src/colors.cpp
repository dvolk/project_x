#include "./colors.h"

#include <allegro5/allegro.h>
#include <allegro5/allegro_color.h>

extern Colors colors;

void init_colors(void) {
    colors.grey = al_color_name("grey");
    colors.grey2 = al_map_rgb(200, 200, 200);
    colors.grey3 = al_map_rgb(220, 220, 220);
    colors.darkgrey = al_map_rgb(100, 100, 110);
    colors.black = al_map_rgb(0, 0, 0);
    colors.white = al_map_rgb(255, 255, 255);
    colors.red = al_map_rgb(200, 0, 0);
    colors.tile_tint = al_map_rgba_f(0.5, 0.5, 0.5, 1.0);
    colors.active_tile_tint = al_map_rgba_f(1, 1, 1, 0.2);
    colors.bg = colors.black;
}
