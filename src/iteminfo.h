#pragma once

#include <allegro5/allegro.h>

#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>

enum ItemSlot {
    SLOT_NONE,
    ARMOR_HEAD,
    ARMOR_TORSO,
    ARMOR_LEGS,
    ARMOR_LEFT_SHOE,
    ARMOR_RIGHT_SHOE,
    ARMOR_LEFT_HAND,
    ARMOR_RIGHT_HAND,
    ARMOR_BACK,
    WATER_BOTTLE,
    WEAPON_BOW,
    AMMO_BOW,
};

enum ItemFlag {
    /*
      TODO Replace slots with these
     */
    _SLOT_NONE = 0,
    _ARMOR_HEAD,
    _ARMOR_TORSO,
    _ARMOR_LEGS,
    _ARMOR_LEFT_SHOE,
    _ARMOR_RIGHT_SHOE,
    _ARMOR_LEFT_HAND,
    _ARMOR_RIGHT_HAND,
    _ARMOR_BACK,
    _WATER_BOTTLE,
    _WEAPON_BOW,
    _AMMO_BOW,
    VEHICLE,
    CRAFTING_ONLY,
    PUT_ON_SHOULDER,
    PUT_ON_NECK,
    WEAPON_WITH_AMMO,
};

struct ItemInfo {
    const char *name;

    ItemSlot slot; // remove this

    std::vector<enum ItemFlag> flags;

    // the size of the item in grid units
    int grid_size_x;
    int grid_size_y;
    // the size when deployed on a hardpoint
    int grid_size_on_hp_x;
    int grid_size_on_hp_y;
    // the size of the container attached to the item
    int container_size_x;
    int container_size_y;
    // where to offset the item's container in the inventory display
    float container_offset_x;
    float container_offset_y;

    // how many items can be stacked on top of each other. Stacked items
    // are one item
    int16_t maxStack;
    int weight; // [g]

    /*
      TODO replace these bools with flags
     */
    bool isContainer;
    bool isSkill;
    bool isLocation;
    bool isLiquid;
    bool isEncounterAction;
    // can the item be damaged. Some items can't be damaged, those
    // are already treated as undamagable. This is for items that don't
    // meet those conditions.
    bool canBeDamaged;
    // can it be applied to a body part (i.e. bandages and disinfectants)?
    bool apply_to_body;
    // when applied to a body part, is it consumed (i.e. disinfectants)?
    bool consumed_on_application;
    // when used, is it consumed (i.e. food, water, medicine)?
    bool consumed_on_use;
    float improves_hydration;
    float improves_satiety;
    // how much warmth this item gives when equipped on a hardpoint
    float warmth;
    // weapon damage [0,1]
    float weapon_damage;
    // weapon range [0-]
    int weapon_range;
    // item with runtime text
    bool is_text_item;
    bool is_scavenge_tool;
    float scavenge_loot_mult;
    float scavenge_safety_mult;
    float scavenge_sneak_mult;

    // sprite when item is on a normal grid
    ALLEGRO_BITMAP *sprite;
    // sprite when items is on a hardpoint
    ALLEGRO_BITMAP *sprite_on_hp;

    void save(std::ostream &os);
    void load(std::istream &is);

    /*
      values filled at runtime in calc_iteminfo_params
     */

    // the size of the tooltip
    float tooltip_size_x;
    // derived from WEAPON_WITH_AMMO flag
    bool weapon_with_ammo;
};
