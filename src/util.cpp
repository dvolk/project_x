#include <string>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cstdarg>

using namespace std;

#include "./config.h"
#include "./util.h"

extern Config config;

static fstream logstream;

void init_logging(void) {
    if(config.log_to_file == false)
        return;

    const char *filename = "./projectx.log";

    logstream.open(filename, fstream::app);

    if(logstream.is_open() == false) {
        fatal_error("Couldn't open log file %s for write/append", filename);
    }
}

void stop_logging(void) {
    if(logstream.is_open() == true)
        logstream.close();
}

enum MESSAGE_LEVEL {
    MESSAGE_FATAL_ERROR,
    MESSAGE_ERROR,
    MESSAGE_INFO,
    MESSAGE_DEBUG,
};

static inline void log(enum MESSAGE_LEVEL msgl, const char *format_string, va_list argp) {
    char str[1024];
    const char *prefix;

    vsnprintf(str, sizeof(str), format_string, argp);

    switch(msgl)
        {
        case MESSAGE_FATAL_ERROR: { prefix = "Fatal error: "; } break;
        case MESSAGE_ERROR:       { prefix = "Error: "; } break;
        case MESSAGE_INFO:        { prefix = "Info: "; } break;
        case MESSAGE_DEBUG:       { prefix = "Debug: "; } break;
        default: { } break;
        }

    cout << prefix << str << endl;

    if(logstream.is_open() == true) {
        logstream << prefix << str << endl;
    }

    if(msgl == MESSAGE_FATAL_ERROR) {
        exit(1);
    }
}

void info(const char *format_string, ...) {
    va_list args;
    va_start(args, format_string);
    log(MESSAGE_INFO, format_string, args);
    va_end(args);
}

void error(const char *format_string, ...) {
    va_list args;
    va_start(args, format_string);
    log(MESSAGE_ERROR, format_string, args);
    va_end(args);
}

void fatal_error(const char *format_string, ...) {
    va_list args;
    va_start(args, format_string);
    log(MESSAGE_FATAL_ERROR, format_string, args);
    va_end(args);
}

void debug(const char *format_string, ...) {
    if(config.debug_output == false)
        return;

    va_list args;
    va_start(args, format_string);
    log(MESSAGE_DEBUG, format_string, args);
    va_end(args);
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
