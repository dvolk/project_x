#include "./iteminfo.h"

#include <vector>
#include <allegro5/allegro.h>

std::vector<ItemInfo>& get_global_iteminfo(void);
ALLEGRO_BITMAP *get_global_bitmap(size_t n);

#define REWRITE_ITEMDEFS
#ifdef REWRITE_ITEMDEFS
void init_iteminfo(void) {
    ItemInfo tmp;
    // 00
    tmp.name = "none";
    tmp.grid_size_x = 1;
    tmp.grid_size_y = 1;
    tmp.grid_size_on_hp_x = -1;
    tmp.grid_size_on_hp_y = -1;
    tmp.maxStack = 1;
    tmp.weight = -1;
    tmp.sprite = NULL;
    tmp.sprite_on_hp = NULL;
    tmp.isLiquid = false;
    tmp.isContainer = false;
    tmp.isLocation = false;
    tmp.isEncounterAction = false;
    tmp.container_size_x = 0;
    tmp.container_size_y = 0;
    tmp.container_offset_x = 0;
    tmp.container_offset_y = 0;
    tmp.isSkill = false;
    tmp.apply_to_body = false;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = false;
    tmp.improves_hydration = 0.0;
    tmp.improves_satiety = 0.0;
    tmp.canBeDamaged = true;
    tmp.slot = SLOT_NONE;
    tmp.warmth = 0;
    tmp.weapon_damage = 0.001;
    tmp.weapon_range = 1;
    tmp.is_text_item = false;
    tmp.is_scavenge_tool = false;
    tmp.scavenge_loot_mult = 0;
    tmp.scavenge_safety_mult = 0;
    tmp.scavenge_sneak_mult = 0;
    tmp.flags = { };
    tmp.tooltip_size_x = -1;
    tmp.weapon_with_ammo = false;
    get_global_iteminfo().push_back(tmp);

    // 01
    tmp.name = "backpack";
    tmp.grid_size_x = 6;
    tmp.grid_size_y = 6;
    tmp.maxStack = 1;
    tmp.weight = 1000;
    tmp.sprite = get_global_bitmap(24);
    tmp.sprite_on_hp = NULL;
    tmp.isContainer = true;
    tmp.container_size_x = 10;
    tmp.container_size_y = 10;
    tmp.isSkill = false;
    tmp.apply_to_body = false;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = false;
    tmp.slot = ARMOR_BACK;
    tmp.warmth = 0.05;
    get_global_iteminfo().push_back(tmp);

    // 02
    tmp.name = "first aid kit";
    tmp.grid_size_x = 4;
    tmp.grid_size_y = 4;
    tmp.maxStack = 1;
    tmp.weight = 750;
    tmp.sprite = get_global_bitmap(22);
    tmp.sprite_on_hp = NULL;
    tmp.isContainer = true;
    tmp.container_size_x = 4;
    tmp.container_size_y = 4;
    tmp.isSkill = false;
    tmp.apply_to_body = false;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = false;
    tmp.slot = SLOT_NONE;
    tmp.warmth = 0;
    get_global_iteminfo().push_back(tmp);

    // 03
    tmp.name = "crowbar";
    tmp.grid_size_x = 14;
    tmp.grid_size_y = 2;
    tmp.maxStack = 1;
    tmp.weight = 2500;
    tmp.sprite = get_global_bitmap(20);
    tmp.sprite_on_hp = NULL;
    tmp.isContainer = false;
    tmp.container_size_x = 0;
    tmp.container_size_y = 0;
    tmp.isSkill = false;
    tmp.apply_to_body = false;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = false;
    tmp.slot = SLOT_NONE;
    tmp.weapon_damage = 0.15;
    tmp.weapon_range = 3;
    tmp.is_scavenge_tool = true;
    tmp.scavenge_loot_mult = 10.0;
    tmp.scavenge_safety_mult = 1.0;
    tmp.scavenge_sneak_mult = 1.0;
    tmp.flags = { PUT_ON_SHOULDER };
    get_global_iteminfo().push_back(tmp);

    // 04
    tmp.name = "shopping trolley";
    tmp.grid_size_x = 12;
    tmp.grid_size_y = 12;
    tmp.maxStack = 1;
    tmp.weight = 5000;
    tmp.sprite = get_global_bitmap(26);
    tmp.sprite_on_hp = NULL;
    tmp.isContainer = true;
    tmp.container_size_x = 20;
    tmp.container_size_y = 20;
    tmp.isSkill = false;
    tmp.apply_to_body = false;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = false;
    tmp.slot = SLOT_NONE;
    tmp.weapon_damage = 0.0;
    tmp.weapon_range = 1;
    tmp.is_scavenge_tool = false;
    tmp.scavenge_loot_mult = 0;
    tmp.scavenge_safety_mult = 0;
    tmp.scavenge_sneak_mult = 0;
    tmp.flags = { VEHICLE };
    get_global_iteminfo().push_back(tmp);

    // 05
    tmp.name = "pill bottle";
    tmp.grid_size_x = 1;
    tmp.grid_size_y = 2;
    tmp.maxStack = 1;
    tmp.weight = 50;
    tmp.sprite = get_global_bitmap(27);
    tmp.sprite_on_hp = NULL;
    tmp.isContainer = true;
    tmp.container_size_x = 1;
    tmp.container_size_y = 2;
    tmp.isSkill = false;
    tmp.apply_to_body = false;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = false;
    tmp.slot = SLOT_NONE;
    tmp.flags = { };
    get_global_iteminfo().push_back(tmp);

    // 06
    tmp.name = "Flee";
    tmp.grid_size_x = 2;
    tmp.grid_size_y = 2;
    tmp.maxStack = 1;
    tmp.weight = -1;
    tmp.sprite = get_global_bitmap(31);
    tmp.sprite_on_hp = NULL;
    tmp.isContainer = false;
    tmp.isEncounterAction = true;
    tmp.container_size_x = 0;
    tmp.container_size_y = 0;
    tmp.isSkill = false;
    tmp.apply_to_body = false;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = false;
    tmp.slot = SLOT_NONE;
    get_global_iteminfo().push_back(tmp);

    // 07
    tmp.name = "Single attack";
    tmp.grid_size_x = 2;
    tmp.grid_size_y = 2;
    tmp.maxStack = 1;
    tmp.weight = -1;
    tmp.sprite = get_global_bitmap(32);
    tmp.sprite_on_hp = NULL;
    tmp.isContainer = false;
    tmp.isEncounterAction = true;
    tmp.container_size_x = 0;
    tmp.container_size_y = 0;
    tmp.isSkill = false;
    tmp.apply_to_body = false;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = false;
    tmp.slot = SLOT_NONE;
    get_global_iteminfo().push_back(tmp);

    // 08
    tmp.name = "bullet";
    tmp.grid_size_x = 1;
    tmp.grid_size_y = 1;
    tmp.maxStack = 5;
    tmp.weight = 5;
    tmp.sprite = get_global_bitmap(34);
    tmp.sprite_on_hp = NULL;
    tmp.isContainer = false;
    tmp.isEncounterAction = false;
    tmp.container_size_x = 0;
    tmp.container_size_y = 0;
    tmp.isSkill = false;
    tmp.apply_to_body = false;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = false;
    tmp.slot = SLOT_NONE;
    tmp.weapon_damage = 0.001;
    get_global_iteminfo().push_back(tmp);

    // 09
    tmp.name = "arrow";
    tmp.grid_size_x = 10;
    tmp.grid_size_y = 1;
    tmp.maxStack = 5;
    tmp.weight = 75;
    tmp.sprite = get_global_bitmap(35);
    tmp.sprite_on_hp = NULL;
    tmp.isContainer = false;
    tmp.container_size_x = 0;
    tmp.container_size_y = 0;
    tmp.isSkill = false;
    tmp.apply_to_body = false;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = false;
    tmp.slot = AMMO_BOW;
    tmp.weapon_damage = 0.005;
    get_global_iteminfo().push_back(tmp);

    // 10
    tmp.name = "Quick";
    tmp.grid_size_x = 6;
    tmp.grid_size_y = 2;
    tmp.maxStack = 1;
    tmp.weight = -1;
    tmp.sprite = get_global_bitmap(36);
    tmp.sprite_on_hp = NULL;
    tmp.isContainer = false;
    tmp.container_size_x = 0;
    tmp.container_size_y = 0;
    tmp.isSkill = true;
    tmp.apply_to_body = false;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = false;
    tmp.slot = SLOT_NONE;
    tmp.weapon_damage = 0.0;
    get_global_iteminfo().push_back(tmp);

    // 11
    tmp.name = "Lockpicking";
    tmp.grid_size_x = 6;
    tmp.grid_size_y = 2;
    tmp.maxStack = 1;
    tmp.weight = -1;
    tmp.sprite = get_global_bitmap(37);
    tmp.sprite_on_hp = NULL;
    tmp.isContainer = false;
    tmp.container_size_x = 0;
    tmp.container_size_y = 0;
    tmp.isSkill = true;
    tmp.apply_to_body = false;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = false;
    tmp.slot = SLOT_NONE;
    get_global_iteminfo().push_back(tmp);

    // 12
    tmp.name = "Myopia";
    tmp.grid_size_x = 6;
    tmp.grid_size_y = 2;
    tmp.maxStack = 1;
    tmp.weight = -1;
    tmp.sprite = get_global_bitmap(38);
    tmp.sprite_on_hp = NULL;
    tmp.isContainer = false;
    tmp.container_size_x = 0;
    tmp.container_size_y = 0;
    tmp.isSkill = true;
    tmp.apply_to_body = false;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = false;
    tmp.slot = SLOT_NONE;
    get_global_iteminfo().push_back(tmp);

    // 13
    tmp.name = "Metabolism";
    tmp.grid_size_x = 6;
    tmp.grid_size_y = 2;
    tmp.maxStack = 1;
    tmp.weight = -1;
    tmp.sprite = get_global_bitmap(39);
    tmp.sprite_on_hp = NULL;
    tmp.isContainer = false;
    tmp.container_size_x = 0;
    tmp.container_size_y = 0;
    tmp.isSkill = true;
    tmp.apply_to_body = false;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = false;
    tmp.slot = SLOT_NONE;
    get_global_iteminfo().push_back(tmp);

    // 14
    tmp.name = "whiskey";
    tmp.grid_size_x = 1;
    tmp.grid_size_y = 2;
    tmp.maxStack = 5;
    tmp.weight = 50;
    tmp.sprite = get_global_bitmap(40);
    tmp.sprite_on_hp = NULL;
    tmp.isLiquid = true;
    tmp.isContainer = false;
    tmp.container_size_x = 0;
    tmp.container_size_y = 0;
    tmp.isSkill = false;
    tmp.apply_to_body = true;
    tmp.consumed_on_application = true;
    tmp.consumed_on_use = true;
    tmp.improves_hydration = 0.1;
    tmp.improves_satiety = 0.05;
    tmp.slot = WATER_BOTTLE;
    get_global_iteminfo().push_back(tmp);

    // 15
    tmp.name = "clean rag";
    tmp.grid_size_x = 3;
    tmp.grid_size_y = 2;
    tmp.maxStack = 10;
    tmp.weight = 50;
    tmp.sprite = get_global_bitmap(41);
    tmp.sprite_on_hp = NULL;
    tmp.isLiquid = false;
    tmp.isContainer = false;
    tmp.container_size_x = 0;
    tmp.container_size_y = 0;
    tmp.isSkill = false;
    tmp.apply_to_body = true;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = false;
    tmp.improves_hydration = 0.0;
    tmp.improves_satiety = 0.0;
    tmp.slot = SLOT_NONE;
    get_global_iteminfo().push_back(tmp);

    // 16
    tmp.name = "Factory";
    tmp.grid_size_x = 2;
    tmp.grid_size_y = 2;
    tmp.maxStack = 1;
    tmp.weight = -1;
    tmp.sprite = get_global_bitmap(42);
    tmp.sprite_on_hp = NULL;
    tmp.isContainer = false;
    tmp.isLocation = true;
    tmp.container_size_x = 0;
    tmp.container_size_y = 0;
    tmp.isSkill = false;
    tmp.apply_to_body = false;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = false;
    tmp.slot = SLOT_NONE;
    get_global_iteminfo().push_back(tmp);

    // 17
    tmp.name = "Shack in the woods";
    tmp.grid_size_x = 2;
    tmp.grid_size_y = 2;
    tmp.maxStack = 1;
    tmp.weight = -1;
    tmp.sprite = get_global_bitmap(43);
    tmp.sprite_on_hp = NULL;
    tmp.isContainer = false;
    tmp.isLocation = true;
    tmp.container_size_x = 0;
    tmp.container_size_y = 0;
    tmp.isSkill = false;
    tmp.apply_to_body = false;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = false;
    tmp.slot = SLOT_NONE;
    get_global_iteminfo().push_back(tmp);

    // 18
    tmp.name = "water bottle";
    tmp.grid_size_x = 1;
    tmp.grid_size_y = 4;
    tmp.maxStack = 1;
    tmp.weight = 100;
    tmp.sprite = get_global_bitmap(49);
    tmp.sprite_on_hp = NULL;
    tmp.isContainer = true;
    tmp.isLocation = false;
    tmp.container_size_x = 1;
    tmp.container_size_y = 2;
    tmp.isSkill = false;
    tmp.apply_to_body = false;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = false;
    tmp.slot = SLOT_NONE;
    get_global_iteminfo().push_back(tmp);

    // 19
    tmp.name = "red hoodie";
    tmp.grid_size_x = 3;
    tmp.grid_size_y = 2;
    tmp.grid_size_on_hp_x = 9;
    tmp.grid_size_on_hp_y = 9;
    tmp.maxStack = 1;
    tmp.weight = 500;
    tmp.sprite_on_hp = get_global_bitmap(50);
    tmp.sprite = get_global_bitmap(51);
    tmp.isContainer = true;
    tmp.container_size_x = 2;
    tmp.container_size_y = 4;
    tmp.container_offset_x = -185;
    tmp.container_offset_y = 220;
    tmp.isSkill = false;
    tmp.apply_to_body = false;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = false;
    tmp.slot = ARMOR_TORSO;
    tmp.warmth = 0.1;
    get_global_iteminfo().push_back(tmp);

    // 20
    tmp.name = "blue jeans";
    tmp.grid_size_x = 3;
    tmp.grid_size_y = 2;
    tmp.grid_size_on_hp_x = 6;
    tmp.grid_size_on_hp_y = 11;
    tmp.maxStack = 1;
    tmp.weight = 500;
    tmp.sprite_on_hp = get_global_bitmap(52);
    tmp.sprite = get_global_bitmap(53);
    tmp.isContainer = true;
    tmp.container_size_x = 2;
    tmp.container_size_y = 2;
    tmp.container_offset_x = -165;
    tmp.container_offset_y = +150;
    tmp.isSkill = false;
    tmp.apply_to_body = false;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = false;
    tmp.slot = ARMOR_LEGS;
    tmp.warmth = 0.5;
    get_global_iteminfo().push_back(tmp);

    // 21
    tmp.name = "ski mask";
    tmp.grid_size_x = 2;
    tmp.grid_size_y = 2;
    tmp.grid_size_on_hp_x = -1;
    tmp.grid_size_on_hp_y = -1;
    tmp.maxStack = 1;
    tmp.weight = 500;
    tmp.sprite_on_hp = get_global_bitmap(54);
    tmp.sprite = get_global_bitmap(55);
    tmp.isContainer = false;
    tmp.container_size_x = 0;
    tmp.container_size_y = 0;
    tmp.container_offset_x = 0;
    tmp.container_offset_y = 0;
    tmp.isSkill = false;
    tmp.apply_to_body = false;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = false;
    tmp.slot = ARMOR_HEAD;
    tmp.warmth = 0.1;
    get_global_iteminfo().push_back(tmp);

    // 22
    tmp.name = "right glove";
    tmp.grid_size_x = 3;
    tmp.grid_size_y = 3;
    tmp.maxStack = 1;
    tmp.weight = 500;
    tmp.sprite_on_hp = NULL;
    tmp.sprite = get_global_bitmap(56);
    tmp.isContainer = false;
    tmp.container_size_x = 0;
    tmp.container_size_y = 0;
    tmp.isSkill = false;
    tmp.apply_to_body = false;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = false;
    tmp.slot = ARMOR_RIGHT_HAND;
    tmp.warmth = 0.1;
    get_global_iteminfo().push_back(tmp);

    // 23
    tmp.name = "left glove";
    tmp.grid_size_x = 3;
    tmp.grid_size_y = 3;
    tmp.maxStack = 1;
    tmp.weight = 500;
    tmp.sprite_on_hp = NULL;
    tmp.sprite = get_global_bitmap(57);
    tmp.isContainer = false;
    tmp.container_size_x = 0;
    tmp.container_size_y = 0;
    tmp.isSkill = false;
    tmp.apply_to_body = false;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = false;
    tmp.slot = ARMOR_LEFT_HAND;
    tmp.warmth = 0.1;
    get_global_iteminfo().push_back(tmp);

    // 24
    tmp.name = "right shoe";
    tmp.grid_size_x = 2;
    tmp.grid_size_y = 2;
    tmp.grid_size_on_hp_x = 3;
    tmp.grid_size_on_hp_y = 3;
    tmp.maxStack = 1;
    tmp.weight = 500;
    tmp.sprite_on_hp = NULL;
    tmp.sprite = get_global_bitmap(85);
    tmp.sprite_on_hp = get_global_bitmap(58);
    tmp.isContainer = false;
    tmp.container_size_x = 0;
    tmp.container_size_y = 0;
    tmp.isSkill = false;
    tmp.apply_to_body = false;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = false;
    tmp.slot = ARMOR_RIGHT_SHOE;
    tmp.warmth = 0.1;
    tmp.weapon_damage = 0.005;
    get_global_iteminfo().push_back(tmp);

    // 25
    tmp.name = "left shoe";
    tmp.grid_size_x = 2;
    tmp.grid_size_y = 2;
    tmp.grid_size_on_hp_x = 3;
    tmp.grid_size_on_hp_y = 3;
    tmp.maxStack = 1;
    tmp.weight = 500;
    tmp.sprite_on_hp = NULL;
    tmp.sprite = get_global_bitmap(86);
    tmp.sprite_on_hp = get_global_bitmap(59);
    tmp.isContainer = false;
    tmp.container_size_x = 0;
    tmp.container_size_y = 0;
    tmp.isSkill = false;
    tmp.apply_to_body = false;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = false;
    tmp.slot = ARMOR_LEFT_SHOE;
    tmp.warmth = 0.1;
    tmp.weapon_damage = 0.005;
    get_global_iteminfo().push_back(tmp);

    // 26
    tmp.name = "Retreat";
    tmp.grid_size_x = 2;
    tmp.grid_size_y = 2;
    tmp.grid_size_on_hp_x = -1;
    tmp.grid_size_on_hp_y = -1;
    tmp.maxStack = 1;
    tmp.weight = -1;
    tmp.sprite = get_global_bitmap(80);
    tmp.sprite_on_hp = NULL;
    tmp.isContainer = false;
    tmp.isEncounterAction = true;
    tmp.container_size_x = 0;
    tmp.container_size_y = 0;
    tmp.isSkill = false;
    tmp.apply_to_body = false;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = false;
    tmp.slot = SLOT_NONE;
    get_global_iteminfo().push_back(tmp);

    // 27
    tmp.name = "Advance";
    tmp.grid_size_x = 2;
    tmp.grid_size_y = 2;
    tmp.maxStack = 1;
    tmp.weight = -1;
    tmp.sprite = get_global_bitmap(81);
    tmp.sprite_on_hp = NULL;
    tmp.isContainer = false;
    tmp.isEncounterAction = true;
    tmp.container_size_x = 0;
    tmp.container_size_y = 0;
    tmp.isSkill = false;
    tmp.apply_to_body = false;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = false;
    tmp.slot = SLOT_NONE;
    get_global_iteminfo().push_back(tmp);

    // 28
    tmp.name = "fist";
    tmp.grid_size_x = 3;
    tmp.grid_size_y = 3;
    tmp.maxStack = 1;
    tmp.weight = 500;
    tmp.sprite_on_hp = NULL;
    tmp.sprite = get_global_bitmap(82);
    tmp.isContainer = false;
    tmp.container_size_x = 0;
    tmp.container_size_y = 0;
    tmp.isSkill = false;
    tmp.isEncounterAction = false;
    tmp.apply_to_body = false;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = false;
    tmp.canBeDamaged = false;
    tmp.slot = SLOT_NONE;
    tmp.warmth = 0.1;
    tmp.weapon_damage = 0.1;
    tmp.weapon_range = 1;
    get_global_iteminfo().push_back(tmp);

    // 29
    tmp.name = "makeshift wood bow";
    tmp.grid_size_x = 14;
    tmp.grid_size_y = 3;
    tmp.maxStack = 1;
    tmp.weight = 1500;
    tmp.sprite_on_hp = NULL;
    tmp.sprite = get_global_bitmap(84);
    tmp.isContainer = true;
    tmp.container_size_x = 10;
    tmp.container_size_y = 1;
    tmp.container_offset_x = -250;
    tmp.container_offset_y = 80;
    tmp.isSkill = false;
    tmp.apply_to_body = false;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = false;
    tmp.canBeDamaged = true;
    tmp.slot = WEAPON_BOW;
    tmp.flags = { WEAPON_WITH_AMMO };
    tmp.warmth = 0.0;
    tmp.weapon_damage = 0.2;
    tmp.weapon_range = 8;
    get_global_iteminfo().push_back(tmp);

    // 30
    tmp.name = "moldy bread";
    tmp.grid_size_x = 2;
    tmp.grid_size_y = 2;
    tmp.maxStack = 1;
    tmp.weight = 300;
    tmp.sprite_on_hp = NULL;
    tmp.sprite = get_global_bitmap(96);
    tmp.isContainer = false;
    tmp.container_size_x = -1;
    tmp.container_size_y = -1;
    tmp.container_offset_x = 0;
    tmp.container_offset_y = 0;
    tmp.isSkill = false;
    tmp.apply_to_body = false;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = true;
    tmp.improves_hydration = 0.005;
    tmp.improves_satiety = 0.1;
    tmp.canBeDamaged = true;
    tmp.slot = SLOT_NONE;
    tmp.flags = { };
    tmp.warmth = 0.0;
    tmp.weapon_damage = 0.0001;
    tmp.weapon_range = 1;
    get_global_iteminfo().push_back(tmp);

    // 31
    tmp.name = "water";
    tmp.grid_size_x = 1;
    tmp.grid_size_y = 2;
    tmp.maxStack = 5;
    tmp.weight = 50;
    tmp.sprite = get_global_bitmap(97);
    tmp.sprite_on_hp = NULL;
    tmp.isLiquid = true;
    tmp.isContainer = false;
    tmp.container_size_x = 0;
    tmp.container_size_y = 0;
    tmp.isSkill = false;
    tmp.apply_to_body = true;
    tmp.consumed_on_application = true;
    tmp.consumed_on_use = true;
    tmp.improves_hydration = 0.3;
    tmp.improves_satiety = 0.05;
    tmp.slot = WATER_BOTTLE;
    get_global_iteminfo().push_back(tmp);

    // 32
    tmp.name = "clear water";
    tmp.grid_size_x = 1;
    tmp.grid_size_y = 2;
    tmp.maxStack = 5;
    tmp.weight = 50;
    tmp.sprite = get_global_bitmap(97);
    tmp.sprite_on_hp = NULL;
    tmp.isLiquid = true;
    tmp.isContainer = false;
    tmp.container_size_x = 0;
    tmp.container_size_y = 0;
    tmp.isSkill = false;
    tmp.apply_to_body = true;
    tmp.consumed_on_application = true;
    tmp.consumed_on_use = true;
    tmp.improves_hydration = 0.3;
    tmp.improves_satiety = 0.05;
    tmp.slot = WATER_BOTTLE;
    get_global_iteminfo().push_back(tmp);

    // 33
    tmp.name = "wild berries";
    tmp.grid_size_x = 2;
    tmp.grid_size_y = 2;
    tmp.maxStack = 1;
    tmp.weight = 300;
    tmp.sprite = get_global_bitmap(100);
    tmp.sprite_on_hp = NULL;
    tmp.isLiquid = false;
    tmp.isContainer = false;
    tmp.container_size_x = 0;
    tmp.container_size_y = 0;
    tmp.isSkill = false;
    tmp.apply_to_body = false;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = true;
    tmp.improves_hydration = 0.1;
    tmp.improves_satiety = 0.2;
    tmp.slot = SLOT_NONE;
    get_global_iteminfo().push_back(tmp);

    // 34
    tmp.name = "matches";
    tmp.grid_size_x = 1;
    tmp.grid_size_y = 1;
    tmp.maxStack = 5;
    tmp.weight = 50;
    tmp.sprite = get_global_bitmap(101);
    tmp.sprite_on_hp = NULL;
    tmp.isLiquid = false;
    tmp.isContainer = false;
    tmp.container_size_x = 0;
    tmp.container_size_y = 0;
    tmp.isSkill = false;
    tmp.apply_to_body = false;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = false;
    tmp.improves_hydration = 0.0;
    tmp.improves_satiety = 0.0;
    tmp.slot = SLOT_NONE;
    tmp.is_scavenge_tool = true;
    tmp.scavenge_loot_mult = 1.0;
    tmp.scavenge_safety_mult = 2.0;
    tmp.scavenge_sneak_mult = 0.5;
    get_global_iteminfo().push_back(tmp);

    // 35
    tmp.name = "rope";
    tmp.grid_size_x = 5;
    tmp.grid_size_y = 5;
    tmp.maxStack = 1;
    tmp.weight = 1000;
    tmp.sprite = get_global_bitmap(102);
    tmp.sprite_on_hp = NULL;
    tmp.isLiquid = false;
    tmp.isContainer = false;
    tmp.container_size_x = 0;
    tmp.container_size_y = 0;
    tmp.isSkill = false;
    tmp.apply_to_body = false;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = false;
    tmp.improves_hydration = 0.0;
    tmp.improves_satiety = 0.0;
    tmp.slot = WATER_BOTTLE;
    tmp.is_scavenge_tool = true;
    tmp.scavenge_loot_mult = 1.0;
    tmp.scavenge_safety_mult = 4.0;
    tmp.scavenge_sneak_mult = 1.0;
    get_global_iteminfo().push_back(tmp);

    // 36
    tmp.name = "gasoline can";
    tmp.grid_size_x = 4;
    tmp.grid_size_y = 5;
    tmp.maxStack = 1;
    tmp.weight = 750;
    tmp.sprite = get_global_bitmap(103);
    tmp.sprite_on_hp = NULL;
    tmp.isLiquid = false;
    tmp.isContainer = true;
    tmp.container_size_x = 3;
    tmp.container_size_y = 3;
    tmp.isSkill = false;
    tmp.apply_to_body = false;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = false;
    tmp.improves_hydration = 0.0;
    tmp.improves_satiety = 0.0;
    tmp.slot = SLOT_NONE;
    tmp.is_scavenge_tool = false;
    tmp.scavenge_loot_mult = 0;
    tmp.scavenge_safety_mult = 0;
    tmp.scavenge_sneak_mult = 0;
    get_global_iteminfo().push_back(tmp);

    // 37
    tmp.name = "wood";
    tmp.grid_size_x = 8;
    tmp.grid_size_y = 2;
    tmp.maxStack = 1;
    tmp.weight = 5000;
    tmp.sprite = get_global_bitmap(104);
    tmp.sprite_on_hp = NULL;
    tmp.isLiquid = false;
    tmp.isContainer = false;
    tmp.container_size_x = -1;
    tmp.container_size_y = -1;
    tmp.isSkill = false;
    tmp.apply_to_body = false;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = false;
    tmp.improves_hydration = 0.0;
    tmp.improves_satiety = 0.0;
    tmp.slot = SLOT_NONE;
    get_global_iteminfo().push_back(tmp);

    // 38
    tmp.name = "fire";
    tmp.grid_size_x = 3;
    tmp.grid_size_y = 3;
    tmp.maxStack = 1;
    tmp.weight = -1;
    tmp.sprite = get_global_bitmap(105);
    tmp.sprite_on_hp = NULL;
    tmp.isLiquid = false;
    tmp.isContainer = false;
    tmp.container_size_x = -1;
    tmp.container_size_y = -1;
    tmp.isSkill = false;
    tmp.apply_to_body = false;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = false;
    tmp.improves_hydration = 0.0;
    tmp.improves_satiety = 0.0;
    tmp.slot = SLOT_NONE;
    tmp.flags = { CRAFTING_ONLY };
    get_global_iteminfo().push_back(tmp);

    // 39
    tmp.name = "cooking pot";
    tmp.grid_size_x = 4;
    tmp.grid_size_y = 3;
    tmp.maxStack = 1;
    tmp.weight = 1000;
    tmp.sprite = get_global_bitmap(106);
    tmp.sprite_on_hp = NULL;
    tmp.isLiquid = false;
    tmp.isContainer = false;
    tmp.container_size_x = -1;
    tmp.container_size_y = -1;
    tmp.isSkill = false;
    tmp.apply_to_body = false;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = false;
    tmp.improves_hydration = 0.0;
    tmp.improves_satiety = 0.0;
    tmp.slot = SLOT_NONE;
    tmp.flags = { };
    get_global_iteminfo().push_back(tmp);

    // 40
    tmp.name = "Wait";
    tmp.grid_size_x = 2;
    tmp.grid_size_y = 2;
    tmp.maxStack = 1;
    tmp.weight = -1;
    tmp.sprite = get_global_bitmap(115);
    tmp.sprite_on_hp = NULL;
    tmp.isContainer = false;
    tmp.isEncounterAction = true;
    tmp.container_size_x = 0;
    tmp.container_size_y = 0;
    tmp.isSkill = false;
    tmp.apply_to_body = false;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = false;
    tmp.slot = SLOT_NONE;
    get_global_iteminfo().push_back(tmp);

    // 41
    tmp.name = "Warn";
    tmp.grid_size_x = 2;
    tmp.grid_size_y = 2;
    tmp.maxStack = 1;
    tmp.weight = -1;
    tmp.sprite = get_global_bitmap(116);
    tmp.sprite_on_hp = NULL;
    tmp.isContainer = false;
    tmp.isEncounterAction = true;
    tmp.container_size_x = 0;
    tmp.container_size_y = 0;
    tmp.isSkill = false;
    tmp.apply_to_body = false;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = false;
    tmp.slot = SLOT_NONE;
    get_global_iteminfo().push_back(tmp);

    // 42
    tmp.name = "Leave";
    tmp.grid_size_x = 2;
    tmp.grid_size_y = 2;
    tmp.maxStack = 1;
    tmp.weight = -1;
    tmp.sprite = get_global_bitmap(117);
    tmp.sprite_on_hp = NULL;
    tmp.isContainer = false;
    tmp.isEncounterAction = true;
    tmp.container_size_x = 0;
    tmp.container_size_y = 0;
    tmp.isSkill = false;
    tmp.apply_to_body = false;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = false;
    tmp.slot = SLOT_NONE;
    get_global_iteminfo().push_back(tmp);

    // 43
    tmp.name = "Enter cover";
    tmp.grid_size_x = 2;
    tmp.grid_size_y = 2;
    tmp.maxStack = 1;
    tmp.weight = -1;
    tmp.sprite = get_global_bitmap(120);
    tmp.sprite_on_hp = NULL;
    tmp.isContainer = false;
    tmp.isEncounterAction = true;
    tmp.container_size_x = 0;
    tmp.container_size_y = 0;
    tmp.isSkill = false;
    tmp.apply_to_body = false;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = false;
    tmp.slot = SLOT_NONE;
    get_global_iteminfo().push_back(tmp);

    // 44
    tmp.name = "Leave cover";
    tmp.grid_size_x = 2;
    tmp.grid_size_y = 2;
    tmp.maxStack = 1;
    tmp.weight = -1;
    tmp.sprite = get_global_bitmap(121);
    tmp.sprite_on_hp = NULL;
    tmp.isContainer = false;
    tmp.isEncounterAction = true;
    tmp.container_size_x = 0;
    tmp.container_size_y = 0;
    tmp.isSkill = false;
    tmp.apply_to_body = false;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = false;
    tmp.slot = SLOT_NONE;
    get_global_iteminfo().push_back(tmp);

    // 45
    tmp.name = "Access inventory";
    tmp.grid_size_x = 2;
    tmp.grid_size_y = 2;
    tmp.maxStack = 1;
    tmp.weight = -1;
    tmp.sprite = get_global_bitmap(123);
    tmp.sprite_on_hp = NULL;
    tmp.isContainer = false;
    tmp.isEncounterAction = true;
    tmp.container_size_x = 0;
    tmp.container_size_y = 0;
    tmp.isSkill = false;
    tmp.apply_to_body = false;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = false;
    tmp.slot = SLOT_NONE;
    get_global_iteminfo().push_back(tmp);

    // 46
    tmp.name = "hunting knife";
    tmp.grid_size_x = 1;
    tmp.grid_size_y = 2;
    tmp.maxStack = 1;
    tmp.weight = 350;
    tmp.sprite = get_global_bitmap(126);
    tmp.sprite_on_hp = NULL;
    tmp.isLiquid = false;
    tmp.isContainer = false;
    tmp.isEncounterAction = false;
    tmp.container_size_x = -1;
    tmp.container_size_y = -1;
    tmp.isSkill = false;
    tmp.apply_to_body = false;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = false;
    tmp.improves_hydration = 0.0;
    tmp.improves_satiety = 0.0;
    tmp.canBeDamaged = true;
    tmp.slot = SLOT_NONE;
    tmp.weapon_damage = 0.2;
    tmp.weapon_range = 1;
    tmp.flags = { };
    get_global_iteminfo().push_back(tmp);

    // 47
    tmp.name = "meat chunk";
    tmp.grid_size_x = 1;
    tmp.grid_size_y = 1;
    tmp.maxStack = 1;
    tmp.weight = 100;
    tmp.sprite_on_hp = NULL;
    tmp.sprite = get_global_bitmap(127);
    tmp.isContainer = false;
    tmp.container_size_x = -1;
    tmp.container_size_y = -1;
    tmp.container_offset_x = 0;
    tmp.container_offset_y = 0;
    tmp.isSkill = false;
    tmp.apply_to_body = false;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = true;
    tmp.improves_hydration = 0.005;
    tmp.improves_satiety = 0.15;
    tmp.canBeDamaged = true;
    tmp.slot = SLOT_NONE;
    tmp.flags = { DECAYS };
    tmp.warmth = 0.0;
    tmp.weapon_damage = 0.0001;
    tmp.weapon_range = 1;
    get_global_iteminfo().push_back(tmp);
}
#endif
