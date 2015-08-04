#pragma once

enum SOUNDS {
    SOUND_CLICK = 0
};

void set_ui_volume(float vol);
void load_ui_sounds(void);
void play_ui_sound(enum SOUNDS s);
