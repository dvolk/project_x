#include "./sound.h"

#include "./config.h"

#include <allegro5/allegro.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>

#include <vector>

std::vector<ALLEGRO_SAMPLE *> ui_sounds;

extern Config config;

float ui_volume;

void set_ui_volume(float vol) {
    ui_volume = vol;
}

void load_ui_sounds(void) {
    std::vector<const char *> sound_filenames;

    sound_filenames.push_back("media/sounds/click3.wav");

    for(auto&& fn : sound_filenames) { // TODO echo progress
        ALLEGRO_SAMPLE *t = al_load_sample(fn);
        assert(t); // TODO error handling
        ui_sounds.push_back(t);
    }
}
    
void play_ui_sound(enum SOUNDS s) {
    if(config.playUISounds)
        al_play_sample(ui_sounds[s], ui_volume, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
}