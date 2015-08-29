#pragma once

void init_logging();
void stop_logging();

void error(const char *format_string, ...);
void fatal_error(const char *format_string, ...);
void info(const char *format_string, ...);
void debug(const char *format_string, ...);

bool rectIntersect(int a_x, int a_y, int a_width, int a_height,
                   int b_x, int b_y, int b_width, int b_height);
