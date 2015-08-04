#pragma once

enum UI_SOUND {
    SOUND_CLICK = 0,
    SOUND_CLICK2,
    SOUND_NONE
};

void set_ui_volume(float vol);
void load_ui_sounds(void);
void play_ui_sound(enum UI_SOUND s);
