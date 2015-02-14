#include <string>
#include <iostream>
#include <cstdlib>

using namespace std;

#ifdef _WIN32
void *__gxx_personality_v0 = NULL;
void *_Unwind_Resume = NULL;
#endif

void errorQuit(string str) {
    cout << "Error: " << str << endl;
    exit(1);
}

void info(string str) {
    cout << "Info: " << str << endl;
}
