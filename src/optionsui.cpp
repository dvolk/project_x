#include "./optionsui.h"

#include "./config.h"
#include "./musicplayer.h"

#include <allegro5/allegro.h>
#include <stdio.h>
#include <algorithm>

extern Config config;

void OptionsUI::reset_settings(void) {
    for(auto&& cb : checkboxes) cb->reset();
    for(auto&& cb : resolution_checkboxes)
        // reset all but the resolution we're in!
        if(cb->res_data.x != config.displayX ||
           cb->res_data.y != config.displayY)
            cb->state = false;
        else
            cb->state = true;
    musicVolumeSlider->state = config.musicVolume;
    music_player_set_volume(config.musicVolume);
}

void OptionsUI::apply_settings(void) {
    for(auto&& cb : checkboxes) cb->apply();
    for(auto&& cb : resolution_checkboxes) {
        if(cb->state == true) {
            config.displayX = cb->res_data.x;
            config.displayY = cb->res_data.y;
            printf("selected resolution: %d %d\n", cb->res_data.x, cb->res_data.y);
        }
    }
    config.musicVolume = musicVolumeSlider->state;
    music_player_set_volume(config.musicVolume);
}

void runMainMenu(void);

extern OptionsUI *ui_Options;

OptionsUI::OptionsUI() {
    button_cancel = new TextButton("Cancel", round((config.displayX - 215) / 2),
                                 (config.displayY - 720) / 2 + 630, 85, 45);

    button_cancel->onMouseDown = [] { ui_Options->reset_settings();
                                      runMainMenu();
    };

    button_apply = new TextButton("Apply", round((config.displayX + 41) / 2),
                                 (config.displayY - 720) / 2 + 630, 85, 45);

    button_apply->onMouseDown = [] { ui_Options->apply_settings();
                                     config.save("game.conf");
                                     runMainMenu();
    };

    float start_x = (config.displayX - 485) / 2;
    float start_y = 100;
    float step_y = 25;
    int i = 0;

    fullscreenCB
        = new LabelledCheckBox(start_x, start_y,
                               "Fullscreen", &config.fullscreen);
    i++;
    vsyncCB
        = new LabelledCheckBox(start_x, start_y + i * step_y,
                               "V-Sync", &config.vsync);
    i++;
    resolutionScalingCB
        = new LabelledCheckBox(start_x, start_y + i * step_y,
                               "Resolution scaling", &config.resolutionScaling);
    i++;
    sortingCB
        = new LabelledCheckBox(start_x, start_y + i * step_y,
                               "Inventory Sorting", &config.sorting);
    i++;
    startNagCB
        = new LabelledCheckBox(start_x, start_y + i * step_y,
                               "Start Notifications", &config.start_nag);
    i++;
    uiFadingCB
        = new LabelledCheckBox(start_x, start_y + i * step_y,
                               "UI Fading", &config.ui_fading);
    i++;
    altGridMovementCB
        = new LabelledCheckBox(start_x, start_y + i * step_y,
                               "Alternative Grid Movement", &config.alt_grid_movement);
    i++;
    clipRectangleCB
        = new LabelledCheckBox(start_x, start_y + i * step_y,
                               "Clip Rectangle", &config.setClipRectangle);
    i++;
    showFPSCB
        = new LabelledCheckBox(start_x, start_y + i * step_y,
                               "Show FPS in terminal", &config.showFPS);

    i++; i++;
    playMusicCB
        = new LabelledCheckBox(start_x, start_y + i * step_y,
                               "Music", &config.playMusic);

    musicVolumeSlider = new Slider(start_x + 80, start_y + i * step_y,
                                   NULL, &config.musicVolume);
    musicVolumeSlider->callback = music_player_set_volume;

    i++;

    /*
      debug options
    */
    i++;
    debugVisibilityCB
        = new LabelledCheckBox(start_x, start_y + i * step_y,
                               "Reveal Map", &config.debugVisibility);
    i++;
    playerInvulnerableCB
        = new LabelledCheckBox(start_x, start_y + i * step_y,
                               "Player Invulnerable", &config.debugVisibility);

    checkboxes.push_back(fullscreenCB);
    checkboxes.push_back(vsyncCB);
    checkboxes.push_back(resolutionScalingCB);
    checkboxes.push_back(debugVisibilityCB);
    checkboxes.push_back(playerInvulnerableCB);
    checkboxes.push_back(sortingCB);
    checkboxes.push_back(startNagCB);
    checkboxes.push_back(uiFadingCB);
    checkboxes.push_back(altGridMovementCB);
    checkboxes.push_back(clipRectangleCB);
    checkboxes.push_back(playMusicCB);
    checkboxes.push_back(showFPSCB);

    for(auto&& cb : checkboxes) widgets.push_back(cb);
    widgets.push_back(musicVolumeSlider);

    /*
      dynamically add resolution checkboxes
    */
    char buf[32];
    start_x = (config.displayX + 275) / 2;
    start_y = 100;
    step_y = 0;

    for (int i = 0 ; i < al_get_num_display_modes() ; ++i) {
        ALLEGRO_DISPLAY_MODE mode;
        if (al_get_display_mode(i , &mode) == &mode) {
            // filter out different refresh rates/depths for the same resolution
            bool skip = false;
            for(auto&& cb : resolution_checkboxes) {
                if(mode.width == cb->res_data.x && mode.height == cb->res_data.y)
                    skip = true;
            }
            if(skip == true)
                continue;

            // only add resolutions that are higher than 720p
            if(mode.width >= 1280 && mode.height >= 720) {
                snprintf(buf, sizeof(buf), "%dx%d", mode.width, mode.height);

                LabelledCheckBox *cb = new LabelledCheckBox(start_x, start_y + 25 * step_y, strdup(buf), NULL);
                cb->res_data.x = mode.width;
                cb->res_data.y = mode.height;
                if(cb->res_data.x == config.displayX &&
                   cb->res_data.y == config.displayY)
                    cb->state = true;

                cb->callback = []{
                    for(auto&& cb : ui_Options->resolution_checkboxes)
                        cb->state = false;
                };

                resolution_checkboxes.push_back(cb);
                step_y++;
            }
        }
    }

    for(auto&& cb : resolution_checkboxes) widgets.push_back(cb);

    widgets.push_back(button_cancel);
    widgets.push_back(button_apply);
}

OptionsUI::~OptionsUI() {
    for(auto&& cb : resolution_checkboxes) {
        free((char*)cb->name);
    }
    for(auto&& widget : widgets) {
        delete widget;
    }
}

