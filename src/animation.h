#pragma once

#include <vector>

struct Animation {
    std::vector<int> *bitmaps;
    int bitmap; // index into g.bitmaps[], -1 = don't draw
    int frame;
    int fps;
    int update_counter;
    int frame_max;
    int frame_change;

    void setup(std::vector<int> *bms, int _fps);
    void next_frame(void);
    void update(void);
};
