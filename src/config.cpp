#include "./config.h"
#include "./util.h"

#include <stdio.h>
#include <allegro5/allegro.h>

static const char *with_default(const char *str, const char *def) {
    if(str == NULL) return def;
    else return str;
}

void Config::save(const char *filename) {
    ALLEGRO_CONFIG *cfg = al_create_config();
    char buf[512];

    snprintf(buf, sizeof(buf), "%d", frame_rate);
    al_set_config_value(cfg, NULL, "frame-rate", buf);
    snprintf(buf, sizeof(buf), "%d", vsync);
    al_set_config_value(cfg, NULL, "v-sync", buf);
    snprintf(buf, sizeof(buf), "%d", fullscreen);
    al_set_config_value(cfg, NULL, "fullscreen", buf);
    snprintf(buf, sizeof(buf), "%d", resolutionScaling);
    al_set_config_value(cfg, NULL, "resolution-scaling", buf);
    snprintf(buf, sizeof(buf), "%d", debugVisibility);
    al_set_config_value(cfg, NULL, "debug-visibility", buf);
    snprintf(buf, sizeof(buf), "%d", playerInvulnerable);
    al_set_config_value(cfg, NULL, "player-invulnerable", buf);
    snprintf(buf, sizeof(buf), "%d", displayX);
    al_set_config_value(cfg, NULL, "display-x", buf);
    snprintf(buf, sizeof(buf), "%d", displayY);
    al_set_config_value(cfg, NULL, "display-y", buf);
    snprintf(buf, sizeof(buf), "%s", font_filename);
    al_set_config_value(cfg, NULL, "font-filename", buf);
    snprintf(buf, sizeof(buf), "%d", font_height);
    al_set_config_value(cfg, NULL, "font-height", buf);
    snprintf(buf, sizeof(buf), "%d", sorting);
    al_set_config_value(cfg, NULL, "grid-sorting", buf);
    snprintf(buf, sizeof(buf), "%d", aa_samples);
    al_set_config_value(cfg, NULL, "aa-samples", buf);
    snprintf(buf, sizeof(buf), "%d", start_nag);
    al_set_config_value(cfg, NULL, "start-nag", buf);
    snprintf(buf, sizeof(buf), "%d", ui_fading);
    al_set_config_value(cfg, NULL, "ui-fading", buf);
    snprintf(buf, sizeof(buf), "%d", alt_grid_movement);
    al_set_config_value(cfg, NULL, "alt-grid-movement", buf);
    snprintf(buf, sizeof(buf), "%d", setClipRectangle);
    al_set_config_value(cfg, NULL, "set-clip-rectangle", buf);
    snprintf(buf, sizeof(buf), "%d", playMusic);
    al_set_config_value(cfg, NULL, "playMusic", buf);
    snprintf(buf, sizeof(buf), "%d", showFPS);
    al_set_config_value(cfg, NULL, "showFPS", buf);
    snprintf(buf, sizeof(buf), "%d", int(musicVolume * 100));
    al_set_config_value(cfg, NULL, "music-volume", buf);

    if(custom_cursor == NULL) {
        al_set_config_value(cfg, NULL, "custom-cursor", "0");
    }
    else {
        snprintf(buf, sizeof(buf), "%s", custom_cursor);
        al_set_config_value(cfg, NULL, "custom-cursor", buf);
    }
    al_save_config_file(filename, cfg);
}

void Config::load(const char *filename) {
    ALLEGRO_CONFIG *cfg = al_load_config_file(filename);

    if(cfg == NULL) {
        info("Couldn't load game.conf");
        cfg = al_create_config();
    } else {
        info("Loaded game.conf");
    }

    const char *s;

    s = al_get_config_value(cfg, 0, "frame-rate");
    frame_rate = atoi(with_default(s, "60"));

    s = al_get_config_value(cfg, 0, "v-sync");
    vsync = atoi(with_default(s, "1"));

    s = al_get_config_value(cfg, 0, "fullscreen");
    fullscreen = atoi(with_default(s, "0"));

    s = al_get_config_value(cfg, 0, "resolution-scaling");
    resolutionScaling = atoi(with_default(s, "1"));

    s = al_get_config_value(cfg, 0, "debug-visibility");
    debugVisibility = atoi(with_default(s, "0"));

    s = al_get_config_value(cfg, 0, "player-invulnerable");
    playerInvulnerable = atoi(with_default(s, "0"));

    s = al_get_config_value(cfg, 0, "display-x");
    displayX = atoi(with_default(s, "1280"));

    s = al_get_config_value(cfg, 0, "display-y");
    displayY = atoi(with_default(s, "720"));

    s = al_get_config_value(cfg, 0, "font-filename");
    font_filename = strdup(with_default(s, "media/fonts/DejaVuSans-Bold.ttf"));

    s = al_get_config_value(cfg, 0, "font-height");
    font_height = atoi(with_default(s, "14"));

    s = al_get_config_value(cfg, 0, "grid-sorting");
    sorting = atoi(with_default(s, "0"));

    s = al_get_config_value(cfg, 0, "aa-samples");
    aa_samples = atoi(with_default(s, "4"));

    s = al_get_config_value(cfg, 0, "start-nag");
    start_nag = atoi(with_default(s, "1"));

    s = al_get_config_value(cfg, 0, "ui-fading");
    ui_fading = atoi(with_default(s, "1"));

    s = al_get_config_value(cfg, 0, "alt-grid-movement");
    alt_grid_movement = atoi(with_default(s, "1"));

    s = al_get_config_value(cfg, 0, "set-clip-rectangle");
    setClipRectangle = atoi(with_default(s, "1"));

    s = al_get_config_value(cfg, 0, "playMusic");
    playMusic = atoi(with_default(s, "1"));

    s = al_get_config_value(cfg, 0, "showFPS");
    showFPS = atoi(with_default(s, "0"));

    s = al_get_config_value(cfg, 0, "music-volume");
    musicVolume = float(atoi(with_default(s, "50"))) / 100.0;

    s = al_get_config_value(cfg, 0, "custom-cursor");
    if(s != NULL && strcmp(s, "0") != 0)
        custom_cursor = strdup(s);
    else
        custom_cursor = NULL;

    al_destroy_config(cfg);
}
