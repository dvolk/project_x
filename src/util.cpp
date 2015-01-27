#include <string>
#include <iostream>

using namespace std;

void errorQuit(string str) {
    cout << "Error: " << str << endl;
    exit(1);
}

void info(string str) {
    cout << "Info: " << str << endl;
}
