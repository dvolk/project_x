#include "./animation.h"

#include "./config.h"

extern Config config;

void Animation::setup(std::vector<int> *bms, int _fps) {
    bitmaps = bms;
    bitmap = bitmaps->front();
    fps = _fps;
    frame_change = 1.0/fps * (float)config.frame_rate;
    frame = 0;
    update_counter = 0;
}

void Animation::next_frame(void) {
    frame++;
    if(frame > (int)bitmaps->size() - 1)
        frame = 0;
    bitmap = bitmaps->at(frame);
}

void Animation::update(void) {
    if(update_counter >= frame_change) {
        next_frame();
        update_counter = 0;
    }
    else {
        update_counter++;
    }
}
