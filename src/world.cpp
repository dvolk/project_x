#include "./world.h"

void World::save(std::ostream &os) {
    os << player_faction_kills.size() << ' ';
    for(auto&& k : player_faction_kills) os << k << ' ';
    os << '\n';
    os << visited_strange_building;
    os << '\n';
    os << lake_quest_state;
    os << '\n';
}

void World::load(std::istream &is) {
    int pfk_size;
    is >> pfk_size;
    player_faction_kills.resize(pfk_size);
    for(auto&& k : player_faction_kills) is >> k;
    is >> visited_strange_building;
    is >> lake_quest_state;
}
