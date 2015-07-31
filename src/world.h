#pragma once

#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>

// misc data that is stored with the save game file
struct World {
    std::vector<int> player_faction_kills;
    bool visited_strange_building;
    // 1 - quest accepted
    // 2 - questgiver is dead
    // 3 - quest completed
    int lake_quest_state;

    void save(std::ostream &os);
    void load(std::istream &os);
};
