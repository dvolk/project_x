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

    colors.seen_tile_dawn_tint = al_map_rgba_f(1.0, 1.0, 0.8, 1.0);
    colors.notseen_tile_dawn_tint = al_map_rgba_f(0.6, 0.6, 0.4, 1.0);
    colors.mouseover_tile_dawn_tint = al_map_rgba_f(1, 1, 1, 0.4);

    colors.seen_tile_day_tint = al_map_rgba_f(1, 1, 1, 1.0);
    colors.notseen_tile_day_tint = al_map_rgba_f(0.5, 0.5, 0.5, 1.0);
    colors.mouseover_tile_day_tint = al_map_rgba_f(1, 1, 1, 0.2);

    colors.seen_tile_dusk_tint = al_map_rgba_f(1.0, 0.8, 0.8, 1.0);
    colors.notseen_tile_dusk_tint = al_map_rgba_f(0.5, 0.4, 0.4, 1.0);
    colors.mouseover_tile_dusk_tint = al_map_rgba_f(1, 1, 1, 0.3);

    colors.seen_tile_night_tint = al_map_rgba_f(0.5, 0.5, 0.8, 1.0);
    colors.notseen_tile_night_tint = al_map_rgba_f(0.2, 0.2, 0.5, 1.0);
    colors.mouseover_tile_night_tint = al_map_rgba_f(1, 1, 1, 0.4);
}
