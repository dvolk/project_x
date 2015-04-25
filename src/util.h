#pragma once

#include <string>

void errorQuit(std::string str);
void info(std::string str);
bool rectIntersect(int a_x, int a_y, int a_width, int a_height,
                   int b_x, int b_y, int b_width, int b_height);

