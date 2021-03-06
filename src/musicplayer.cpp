#include "./musicplayer.h"

#include "./util.h"
#include "./config.h"

#include <stdio.h>

#include <array>

#include <allegro5/allegro.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>

extern bool running;
extern Config config;

static ALLEGRO_AUDIO_STREAM *stream = NULL;
static ALLEGRO_THREAD *music_thread = NULL;

void init_music_player(void) {
    music_thread = al_create_thread(music_player, NULL);
    if(music_thread != NULL) {
        al_start_thread(music_thread);
    } else {
        info("init_music_player: couldn't start thread");
    }
}

void exit_music_player(void) {
    if(music_thread != NULL) {
        al_join_thread(music_thread, NULL);
        al_destroy_thread(music_thread);
    }
}

void music_player_set_volume(float vol) {
    if(stream == NULL)
        return;

    info("music_player: changing volume to %f", vol);
    al_set_audio_stream_gain(stream, vol);
}
void *music_player(ALLEGRO_THREAD *thrd, void *arg) {
    info("music_player(): started");

    // wait for the main loop to start
    while(running == false) {
        al_rest(0.1);
    }

    std::array<const char *, 3> tracks =
        { "media/music/Harp.ogg",
          "media/music/Sadness_0.ogg",
          "media/music/Sadness_0.ogg",
        };

    int current_track = 0;

    while(running == true) {
        if(config.playMusic == false) {
            al_rest(0.1);
            continue;
        }

        stream = al_load_audio_stream(tracks.at(current_track), 3, 1024);
        if(stream == NULL) {
            info("music_player(): couldn't load %s", tracks.at(current_track));
            return NULL;
        }

        bool ok;
        
        ok = al_set_audio_stream_playmode(stream, ALLEGRO_PLAYMODE_ONCE);
        if(ok == false) {
            info("music_player(): al_set_audio_stream_playmode failed");
            stream = NULL;
            return NULL;
        }
        ok = al_attach_audio_stream_to_mixer(stream, al_get_default_mixer());
        if(ok == false) {
            info("music_player(): al_attach_audio_stream_to_mixer failed");
            stream = NULL;
            return NULL;
        }
        ok = al_set_audio_stream_gain(stream, config.musicVolume);
        if(ok == false) {
            info("music_player(): al_set_audio_stream_gain failed");
            stream = NULL;
            return NULL;
        }

        info("music_player(): playing %s", tracks.at(current_track));

        while(running == true &&
              config.playMusic == true &&
              al_get_audio_stream_playing(stream) == true)
            {
                al_rest(0.1);
            }

        al_destroy_audio_stream(stream);
        stream = NULL;

        if(current_track >= (int)tracks.size() - 1)
            current_track = 0;
        else
            current_track++;
    }

    info("music_player(): exiting");
    stream = NULL;
    return NULL;
}
