#pragma once

#include <allegro5/allegro.h>
#include <allegro5/allegro_color.h>

struct Colors {
    ALLEGRO_COLOR white;
    ALLEGRO_COLOR black;
    ALLEGRO_COLOR red;
    ALLEGRO_COLOR grey;
    ALLEGRO_COLOR grey2;
    ALLEGRO_COLOR grey3;
    ALLEGRO_COLOR darkgrey;
    ALLEGRO_COLOR tile_tint;
    ALLEGRO_COLOR active_tile_tint;
    ALLEGRO_COLOR bg;
};

void init_colors(void);
