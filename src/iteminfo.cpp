#include "./iteminfo.h"

using namespace std;

int find_bitmap_index(ALLEGRO_BITMAP *searched);
ALLEGRO_BITMAP *get_global_bitmap(size_t n);

void ItemInfo::save(std::ostream &os) {
    os << "name " << strlen(name) << ' ' << name << endl;
    os << "\tItemSlot " << slot << endl;
    os << "\tgrid_size_x " << grid_size_x << endl;
    os << "\tgrid_size_y " << grid_size_y << endl;
    os << "\tgrid_size_on_hp_x " << grid_size_on_hp_x << endl;
    os << "\tgrid_size_on_hp_y " << grid_size_on_hp_y << endl;
    os << "\tcontainer_size_x " << container_size_x << endl;
    os << "\tcontainer_size_y " << container_size_y << endl;
    os << "\tcontainer_offset_x " << container_offset_x << endl;
    os << "\tcontainer_offset_y " << container_offset_y << endl;
    os << "\tmaxStack " << maxStack << endl;
    os << "\tweight " << weight << endl;
    os << "\tisContainer " << isContainer << endl;
    os << "\tisSkill " << isSkill << endl;
    os << "\tisLocation " << isLocation << endl;
    os << "\tisLiquid " << isLiquid << endl;
    os << "\tisEncounterAction " << isEncounterAction << endl;
    os << "\tcanBeDamaged " << canBeDamaged << endl;
    os << "\tapply_to_body " << apply_to_body << endl;
    os << "\tconsumed_on_application " << consumed_on_application << endl;
    os << "\tconsumed_on_use " << consumed_on_use << endl;
    os << "\timproves_hydration " << improves_hydration << endl;
    os << "\timproves_satiety " << improves_satiety << endl;
    os << "\twarmth " << warmth << endl;
    os << "\tweapon_damage " << weapon_damage << endl;
    os << "\tweapon_range " << weapon_range << endl;
    os << "\tis_text_item " << is_text_item << endl;
    os << "\tis_scavenge_tool " << is_scavenge_tool << endl;
    os << "\tscavenge_loot_mult " << scavenge_loot_mult << endl;
    os << "\tscavenge_safety_mult " << scavenge_safety_mult << endl;
    os << "\tscavenge_sneak_mult " << scavenge_sneak_mult << endl;
    os << "\tsprite " << find_bitmap_index(sprite) << endl;
    os << "\tsprite_on_hp " << find_bitmap_index(sprite_on_hp) << endl;
    os << "\tflags: " << flags.size();
    for(auto&& flag : flags) {
        os << ' '<< flag;
    }
    os << endl;
}

void ItemInfo::load(std::istream &is) {
    is.ignore(256, ' ');
    int len;
    is >> len;
    is.ignore(1);
    char *read_name = (char *)malloc(len + 1);
    is.read(&read_name[0], len);
    read_name[len] = '\0';
    this->name = read_name;

    int slot_i;
    is.ignore(256, ' ');    is >> slot_i;
    slot = (ItemSlot)slot_i;
    is.ignore(256, ' ');    is >> grid_size_x;
    is.ignore(256, ' ');    is >> grid_size_y;
    is.ignore(256, ' ');    is >> grid_size_on_hp_x;
    is.ignore(256, ' ');    is >> grid_size_on_hp_y;
    is.ignore(256, ' ');    is >> container_size_x;
    is.ignore(256, ' ');    is >> container_size_y;
    is.ignore(256, ' ');    is >> container_offset_x;
    is.ignore(256, ' ');    is >> container_offset_y;
    is.ignore(256, ' ');    is >> maxStack;
    is.ignore(256, ' ');    is >> weight;
    is.ignore(256, ' ');    is >> isContainer;
    is.ignore(256, ' ');    is >> isSkill;
    is.ignore(256, ' ');    is >> isLocation;
    is.ignore(256, ' ');    is >> isLiquid;
    is.ignore(256, ' ');    is >> isEncounterAction;
    is.ignore(256, ' ');    is >> canBeDamaged;
    is.ignore(256, ' ');    is >> apply_to_body;
    is.ignore(256, ' ');    is >> consumed_on_application;
    is.ignore(256, ' ');    is >> consumed_on_use;
    is.ignore(256, ' ');    is >> improves_hydration;
    is.ignore(256, ' ');    is >> improves_satiety;
    is.ignore(256, ' ');    is >> warmth;
    is.ignore(256, ' ');    is >> weapon_damage;
    is.ignore(256, ' ');    is >> weapon_range;
    is.ignore(256, ' ');    is >> is_text_item;
    is.ignore(256, ' ');    is >> is_scavenge_tool;
    is.ignore(256, ' ');    is >> scavenge_loot_mult;
    is.ignore(256, ' ');    is >> scavenge_safety_mult;
    is.ignore(256, ' ');    is >> scavenge_sneak_mult;

    is.ignore(256, ' ');
    int sprite_i;
    is >> sprite_i;
    if(sprite_i < 0) sprite = NULL; else sprite = get_global_bitmap(sprite_i);
    is.ignore(256, ' ');
    is >> sprite_i;
    if(sprite_i < 0) sprite_on_hp = NULL; else sprite_on_hp = get_global_bitmap(sprite_i);

    /*
      read in iteminfo flags
     */
    is.ignore(256, ' ');
    int flags_size = -1;
    is >> flags_size;
    flags.resize(flags_size);
    for(auto&& flag : flags) {
        int read_flag = -1;
        is >> read_flag;
        flag = (ItemFlag)read_flag;
    }
    is.ignore(1); // \n
}
