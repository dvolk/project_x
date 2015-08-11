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

    ALLEGRO_COLOR seen_tile_day_tint;
    ALLEGRO_COLOR notseen_tile_day_tint;
    ALLEGRO_COLOR mouseover_tile_day_tint;

    ALLEGRO_COLOR seen_tile_night_tint;
    ALLEGRO_COLOR notseen_tile_night_tint;
    ALLEGRO_COLOR mouseover_tile_night_tint;

    ALLEGRO_COLOR seen_tile_dawn_tint;
    ALLEGRO_COLOR notseen_tile_dawn_tint;
    ALLEGRO_COLOR mouseover_tile_dawn_tint;

    ALLEGRO_COLOR seen_tile_dusk_tint;
    ALLEGRO_COLOR notseen_tile_dusk_tint;
    ALLEGRO_COLOR mouseover_tile_dusk_tint;
};

void init_colors(void);
