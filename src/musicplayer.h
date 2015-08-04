#pragma once

#include <allegro5/allegro.h>

void init_music_player(void);
void exit_music_player(void);

void *music_player(ALLEGRO_THREAD *thrd, void *arg);

void music_player_set_volume(float vol);
