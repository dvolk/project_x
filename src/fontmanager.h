#pragma once

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>

void font_load(const char *filepath, int height);
void font_unload(void);
