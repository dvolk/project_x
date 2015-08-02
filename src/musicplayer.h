#pragma once

#include <allegro5/allegro.h>

void *music_player(ALLEGRO_THREAD *thrd, void *arg);
void music_player_set_volume(float vol);
