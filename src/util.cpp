#include <string>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cstdarg>

using namespace std;

#include "./config.h"

extern Config config;

fstream logstream;

void init_logging(void) {
    if(config.log_to_file == false)
        return;

    const char *filename = "projectx.log";

    logstream.open("projectx.log", fstream::app);

    if(logstream.is_open() == false) {
        cout << "Couldn't open " << filename << " for write/append" << endl;
        exit(1);
    }
}

void stop_logging(void) {
    if(logstream.is_open() == true)
        logstream.close();
}

void errorQuit(const char *format_string, ...) {
    char str[1024];
    va_list args;
    va_start(args, format_string);
    vsnprintf(str, sizeof(str), format_string, args);
    va_end(args);

    cout << "Error: " << str << endl;
    if(logstream.is_open() == true)
        logstream << "Error: " << str << endl;
    exit(1);
}

void info(const char *format_string, ...) {
    char str[1024];
    va_list args;
    va_start(args, format_string);
    vsnprintf(str, sizeof(str), format_string, args);
    va_end(args);

    if(logstream.is_open() == true)
        logstream << "Info: " << str << endl;
    cout << "Info: " << str << endl;
}

__attribute__ ((const))
bool rectIntersect(int a_x, int a_y, int a_width, int a_height,
                   int b_x, int b_y, int b_width, int b_height) {
    return
        !(b_x >= a_x + a_width
          || b_x + b_width <= a_x
          || b_y >= a_y + a_height
          || b_y + b_height <= a_y);
}
