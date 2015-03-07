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
    WEAPON_BOW
};

struct Grid;

struct ItemInfo {
    const char *name;

    ItemSlot slot;

    int grid_size_x;
    int grid_size_y;
    int container_size_x;
    int container_size_y;

    int maxStack;
    int weight; // [g]

    bool isVehicle;
    bool isContainer;
    bool isSkill;
    bool isLocation;
    // can it be applied to a body part (i.e. bandages and disinfectants)?
    bool apply_to_body;
    // when applied to a body part, is it consumed (i.e. disinfectants)?
    bool consumed_on_application;
    // when used, is it consumed (i.e. food, water, medicine)?
    bool consumed_on_use;

    float warmth;

    float weapon_damage;
    int weapon_range;

    ALLEGRO_BITMAP *sprite;
    ALLEGRO_BITMAP *sprite_on_hp;
};

void init_iteminfo(void) {
    ItemInfo tmp;
    /* 00 */
    tmp.name = "";
    tmp.grid_size_x = 1;
    tmp.grid_size_y = 1;
    tmp.maxStack = 1;
    tmp.sprite = NULL;
    tmp.sprite_on_hp = NULL;
    tmp.isVehicle = false;
    tmp.isContainer = false;
    tmp.isLocation = false;
    tmp.container_size_x = 0;
    tmp.container_size_y = 0;
    tmp.isSkill = false;
    tmp.apply_to_body = false;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = false;
    tmp.slot = SLOT_NONE;
    tmp.warmth = 0;
    tmp.weapon_damage = 0.001;
    tmp.weapon_range = 1;
    g.item_info.push_back(tmp);

    /* 01 */
    tmp.name = "backpack";
    tmp.grid_size_x = 6;
    tmp.grid_size_y = 6;
    tmp.maxStack = 1;
    tmp.weight = 1000;
    tmp.sprite = g.bitmaps[24];
    tmp.sprite_on_hp = NULL;
    tmp.isVehicle = false;
    tmp.isContainer = true;
    tmp.container_size_x = 10;
    tmp.container_size_y = 10;
    tmp.isSkill = false;
    tmp.apply_to_body = false;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = false;
    tmp.slot = ARMOR_BACK;
    tmp.warmth = 0.05;
    g.item_info.push_back(tmp);

    /* 02 */
    tmp.name = "first aid kit";
    tmp.grid_size_x = 4;
    tmp.grid_size_y = 4;
    tmp.maxStack = 1;
    tmp.weight = 750;
    tmp.sprite = g.bitmaps[22];
    tmp.sprite_on_hp = NULL;
    tmp.isVehicle = false;
    tmp.isContainer = true;
    tmp.container_size_x = 4;
    tmp.container_size_y = 4;
    tmp.isSkill = false;
    tmp.apply_to_body = false;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = false;
    tmp.slot = SLOT_NONE;
    tmp.warmth = 0;
    g.item_info.push_back(tmp);

    /* 03 */
    tmp.name = "crowbar";
    tmp.grid_size_x = 14;
    tmp.grid_size_y = 2;
    tmp.maxStack = 1;
    tmp.weight = 2500;
    tmp.sprite = g.bitmaps[20];
    tmp.sprite_on_hp = NULL;
    tmp.isVehicle = false;
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
    g.item_info.push_back(tmp);

    /* 04 */
    tmp.name = "shopping trolley";
    tmp.grid_size_x = 12;
    tmp.grid_size_y = 12;
    tmp.maxStack = 1;
    tmp.weight = 5000;
    tmp.sprite = g.bitmaps[26];
    tmp.sprite_on_hp = NULL;
    tmp.isVehicle = true;
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
    g.item_info.push_back(tmp);

    /* 05 */
    tmp.name = "pill bottle";
    tmp.grid_size_x = 1;
    tmp.grid_size_y = 2;
    tmp.maxStack = 1;
    tmp.weight = 50;
    tmp.sprite = g.bitmaps[27];
    tmp.sprite_on_hp = NULL;
    tmp.isVehicle = false;
    tmp.isContainer = true;
    tmp.container_size_x = 1;
    tmp.container_size_y = 2;
    tmp.isSkill = false;
    tmp.apply_to_body = false;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = false;
    tmp.slot = SLOT_NONE;
    g.item_info.push_back(tmp);

    /* 06 */
    tmp.name = "Flee";
    tmp.grid_size_x = 2;
    tmp.grid_size_y = 2;
    tmp.maxStack = 1;
    tmp.weight = -1;
    tmp.sprite = g.bitmaps[31];
    tmp.sprite_on_hp = NULL;
    tmp.isVehicle = false;
    tmp.isContainer = false;
    tmp.container_size_x = 0;
    tmp.container_size_y = 0;
    tmp.isSkill = false;
    tmp.apply_to_body = false;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = false;
    tmp.slot = SLOT_NONE;
    g.item_info.push_back(tmp);

    /* 07 */
    tmp.name = "Single attack";
    tmp.grid_size_x = 2;
    tmp.grid_size_y = 2;
    tmp.maxStack = 1;
    tmp.weight = -1;
    tmp.sprite = g.bitmaps[32];
    tmp.sprite_on_hp = NULL;
    tmp.isVehicle = false;
    tmp.isContainer = false;
    tmp.container_size_x = 0;
    tmp.container_size_y = 0;
    tmp.isSkill = false;
    tmp.apply_to_body = false;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = false;
    tmp.slot = SLOT_NONE;
    g.item_info.push_back(tmp);

    /* 08 */
    tmp.name = "bullet";
    tmp.grid_size_x = 1;
    tmp.grid_size_y = 1;
    tmp.maxStack = 5;
    tmp.weight = 5;
    tmp.sprite = g.bitmaps[34];
    tmp.sprite_on_hp = NULL;
    tmp.isVehicle = false;
    tmp.isContainer = false;
    tmp.container_size_x = 0;
    tmp.container_size_y = 0;
    tmp.isSkill = false;
    tmp.apply_to_body = false;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = false;
    tmp.slot = SLOT_NONE;
    tmp.weapon_damage = 0.001;
    g.item_info.push_back(tmp);

    /* 09 */
    tmp.name = "arrow";
    tmp.grid_size_x = 6;
    tmp.grid_size_y = 1;
    tmp.maxStack = 5;
    tmp.weight = 75;
    tmp.sprite = g.bitmaps[35];
    tmp.sprite_on_hp = NULL;
    tmp.isVehicle = false;
    tmp.isContainer = false;
    tmp.container_size_x = 0;
    tmp.container_size_y = 0;
    tmp.isSkill = false;
    tmp.apply_to_body = false;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = false;
    tmp.slot = SLOT_NONE;
    tmp.weapon_damage = 0.005;
    g.item_info.push_back(tmp);

    /* 10 */
    tmp.name = "Quick";
    tmp.grid_size_x = 6;
    tmp.grid_size_y = 2;
    tmp.maxStack = 1;
    tmp.weight = -1;
    tmp.sprite = g.bitmaps[36];
    tmp.sprite_on_hp = NULL;
    tmp.isVehicle = false;
    tmp.isContainer = false;
    tmp.container_size_x = 0;
    tmp.container_size_y = 0;
    tmp.isSkill = true;
    tmp.apply_to_body = false;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = false;
    tmp.slot = SLOT_NONE;
    tmp.weapon_damage = 0.0;
    g.item_info.push_back(tmp);

    /* 11 */
    tmp.name = "Lockpicking";
    tmp.grid_size_x = 6;
    tmp.grid_size_y = 2;
    tmp.maxStack = 1;
    tmp.weight = -1;
    tmp.sprite = g.bitmaps[37];
    tmp.sprite_on_hp = NULL;
    tmp.isVehicle = false;
    tmp.isContainer = false;
    tmp.container_size_x = 0;
    tmp.container_size_y = 0;
    tmp.isSkill = true;
    tmp.apply_to_body = false;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = false;
    tmp.slot = SLOT_NONE;
    g.item_info.push_back(tmp);

    /* 12 */
    tmp.name = "Myopia";
    tmp.grid_size_x = 6;
    tmp.grid_size_y = 2;
    tmp.maxStack = 1;
    tmp.weight = -1;
    tmp.sprite = g.bitmaps[38];
    tmp.sprite_on_hp = NULL;
    tmp.isVehicle = false;
    tmp.isContainer = false;
    tmp.container_size_x = 0;
    tmp.container_size_y = 0;
    tmp.isSkill = true;
    tmp.apply_to_body = false;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = false;
    tmp.slot = SLOT_NONE;
    g.item_info.push_back(tmp);

    /* 13 */
    tmp.name = "Metabolism";
    tmp.grid_size_x = 6;
    tmp.grid_size_y = 2;
    tmp.maxStack = 1;
    tmp.weight = -1;
    tmp.sprite = g.bitmaps[39];
    tmp.sprite_on_hp = NULL;
    tmp.isVehicle = false;
    tmp.isContainer = false;
    tmp.container_size_x = 0;
    tmp.container_size_y = 0;
    tmp.isSkill = true;
    tmp.apply_to_body = false;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = false;
    tmp.slot = SLOT_NONE;
    g.item_info.push_back(tmp);

    /* 14 */
    tmp.name = "whiskey";
    tmp.grid_size_x = 1;
    tmp.grid_size_y = 2;
    tmp.maxStack = 5;
    tmp.weight = 50;
    tmp.sprite = g.bitmaps[40];
    tmp.sprite_on_hp = NULL;
    tmp.isVehicle = false;
    tmp.isContainer = false;
    tmp.container_size_x = 0;
    tmp.container_size_y = 0;
    tmp.isSkill = false;
    tmp.apply_to_body = true;
    tmp.consumed_on_application = true;
    tmp.consumed_on_use = true;
    tmp.slot = WATER_BOTTLE;
    g.item_info.push_back(tmp);

    /* 15 */
    tmp.name = "clean rag";
    tmp.grid_size_x = 3;
    tmp.grid_size_y = 2;
    tmp.maxStack = 10;
    tmp.weight = 50;
    tmp.sprite = g.bitmaps[41];
    tmp.sprite_on_hp = NULL;
    tmp.isVehicle = false;
    tmp.isContainer = false;
    tmp.container_size_x = 0;
    tmp.container_size_y = 0;
    tmp.isSkill = false;
    tmp.apply_to_body = true;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = false;
    tmp.slot = SLOT_NONE;
    g.item_info.push_back(tmp);

    /* 16 */
    tmp.name = "Factory";
    tmp.grid_size_x = 2;
    tmp.grid_size_y = 2;
    tmp.maxStack = 1;
    tmp.weight = -1;
    tmp.sprite = g.bitmaps[42];
    tmp.sprite_on_hp = NULL;
    tmp.isVehicle = false;
    tmp.isContainer = false;
    tmp.isLocation = true;
    tmp.container_size_x = 0;
    tmp.container_size_y = 0;
    tmp.isSkill = false;
    tmp.apply_to_body = false;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = false;
    tmp.slot = SLOT_NONE;
    g.item_info.push_back(tmp);

    /* 17 */
    tmp.name = "Shack in the woods";
    tmp.grid_size_x = 2;
    tmp.grid_size_y = 2;
    tmp.maxStack = 1;
    tmp.weight = -1;
    tmp.sprite = g.bitmaps[43];
    tmp.sprite_on_hp = NULL;
    tmp.isVehicle = false;
    tmp.isContainer = false;
    tmp.isLocation = true;
    tmp.container_size_x = 0;
    tmp.container_size_y = 0;
    tmp.isSkill = false;
    tmp.apply_to_body = false;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = false;
    tmp.slot = SLOT_NONE;
    g.item_info.push_back(tmp);

    /* 18 */
    tmp.name = "water bottle";
    tmp.grid_size_x = 1;
    tmp.grid_size_y = 4;
    tmp.maxStack = 1;
    tmp.weight = 100;
    tmp.sprite = g.bitmaps[49];
    tmp.sprite_on_hp = NULL;
    tmp.isVehicle = false;
    tmp.isContainer = true;
    tmp.isLocation = false;
    tmp.container_size_x = 1;
    tmp.container_size_y = 2;
    tmp.isSkill = false;
    tmp.apply_to_body = false;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = false;
    tmp.slot = SLOT_NONE;
    g.item_info.push_back(tmp);

    /* 19 */
    tmp.name = "red shirt";
    tmp.grid_size_x = 3;
    tmp.grid_size_y = 2;
    tmp.maxStack = 1;
    tmp.weight = 500;
    tmp.sprite_on_hp = g.bitmaps[50];
    tmp.sprite = g.bitmaps[51];
    tmp.isVehicle = false;
    tmp.isContainer = false;
    tmp.container_size_x = 0;
    tmp.container_size_y = 0;
    tmp.isSkill = false;
    tmp.apply_to_body = false;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = false;
    tmp.slot = ARMOR_TORSO;
    tmp.warmth = 0.1;
    g.item_info.push_back(tmp);

    /* 20 */
    tmp.name = "blue pants";
    tmp.grid_size_x = 3;
    tmp.grid_size_y = 2;
    tmp.maxStack = 1;
    tmp.weight = 500;
    tmp.sprite_on_hp = g.bitmaps[52];
    tmp.sprite = g.bitmaps[53];
    tmp.isVehicle = false;
    tmp.isContainer = false;
    tmp.container_size_x = 0;
    tmp.container_size_y = 0;
    tmp.isSkill = false;
    tmp.apply_to_body = false;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = false;
    tmp.slot = ARMOR_LEGS;
    tmp.warmth = 0.5;
    g.item_info.push_back(tmp);

    /* 21 */
    tmp.name = "ski mask";
    tmp.grid_size_x = 2;
    tmp.grid_size_y = 2;
    tmp.maxStack = 1;
    tmp.weight = 500;
    tmp.sprite_on_hp = g.bitmaps[54];
    tmp.sprite = g.bitmaps[55];
    tmp.isVehicle = false;
    tmp.isContainer = false;
    tmp.container_size_x = 0;
    tmp.container_size_y = 0;
    tmp.isSkill = false;
    tmp.apply_to_body = false;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = false;
    tmp.slot = ARMOR_HEAD;
    tmp.warmth = 0.1;
    g.item_info.push_back(tmp);

    /* 22 */
    tmp.name = "right glove";
    tmp.grid_size_x = 3;
    tmp.grid_size_y = 3;
    tmp.maxStack = 1;
    tmp.weight = 500;
    tmp.sprite_on_hp = NULL;
    tmp.sprite = g.bitmaps[56];
    tmp.isVehicle = false;
    tmp.isContainer = false;
    tmp.container_size_x = 0;
    tmp.container_size_y = 0;
    tmp.isSkill = false;
    tmp.apply_to_body = false;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = false;
    tmp.slot = ARMOR_RIGHT_HAND;
    tmp.warmth = 0.1;
    g.item_info.push_back(tmp);

    /* 23 */
    tmp.name = "left glove";
    tmp.grid_size_x = 3;
    tmp.grid_size_y = 3;
    tmp.maxStack = 1;
    tmp.weight = 500;
    tmp.sprite_on_hp = NULL;
    tmp.sprite = g.bitmaps[57];
    tmp.isVehicle = false;
    tmp.isContainer = false;
    tmp.container_size_x = 0;
    tmp.container_size_y = 0;
    tmp.isSkill = false;
    tmp.apply_to_body = false;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = false;
    tmp.slot = ARMOR_LEFT_HAND;
    tmp.warmth = 0.1;
    g.item_info.push_back(tmp);

    /* 24 */
    tmp.name = "right shoe";
    tmp.grid_size_x = 3;
    tmp.grid_size_y = 3;
    tmp.maxStack = 1;
    tmp.weight = 500;
    tmp.sprite_on_hp = NULL;
    tmp.sprite = g.bitmaps[58];
    tmp.isVehicle = false;
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
    g.item_info.push_back(tmp);

    /* 25 */
    tmp.name = "left shoe";
    tmp.grid_size_x = 3;
    tmp.grid_size_y = 3;
    tmp.maxStack = 1;
    tmp.weight = 500;
    tmp.sprite_on_hp = NULL;
    tmp.sprite = g.bitmaps[59];
    tmp.isVehicle = false;
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
    g.item_info.push_back(tmp);

    /* 26 */
    tmp.name = "Retreat";
    tmp.grid_size_x = 2;
    tmp.grid_size_y = 2;
    tmp.maxStack = 1;
    tmp.weight = -1;
    tmp.sprite = g.bitmaps[80];
    tmp.sprite_on_hp = NULL;
    tmp.isVehicle = false;
    tmp.isContainer = false;
    tmp.container_size_x = 0;
    tmp.container_size_y = 0;
    tmp.isSkill = false;
    tmp.apply_to_body = false;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = false;
    tmp.slot = SLOT_NONE;
    g.item_info.push_back(tmp);

    /* 27 */
    tmp.name = "Advance";
    tmp.grid_size_x = 2;
    tmp.grid_size_y = 2;
    tmp.maxStack = 1;
    tmp.weight = -1;
    tmp.sprite = g.bitmaps[81];
    tmp.sprite_on_hp = NULL;
    tmp.isVehicle = false;
    tmp.isContainer = false;
    tmp.container_size_x = 0;
    tmp.container_size_y = 0;
    tmp.isSkill = false;
    tmp.apply_to_body = false;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = false;
    tmp.slot = SLOT_NONE;
    g.item_info.push_back(tmp);

    /* 28 */
    tmp.name = "fist";
    tmp.grid_size_x = 3;
    tmp.grid_size_y = 3;
    tmp.maxStack = 1;
    tmp.weight = 500;
    tmp.sprite_on_hp = NULL;
    tmp.sprite = g.bitmaps[82];
    tmp.isVehicle = false;
    tmp.isContainer = false;
    tmp.container_size_x = 0;
    tmp.container_size_y = 0;
    tmp.isSkill = false;
    tmp.apply_to_body = false;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = false;
    tmp.slot = SLOT_NONE;
    tmp.warmth = 0.1;
    tmp.weapon_damage = 0.1;
    tmp.weapon_range = 1;
    g.item_info.push_back(tmp);

    /* 28 */
    tmp.name = "makeshift wood bow";
    tmp.grid_size_x = 14;
    tmp.grid_size_y = 3;
    tmp.maxStack = 1;
    tmp.weight = 1500;
    tmp.sprite_on_hp = NULL;
    tmp.sprite = g.bitmaps[84];
    tmp.isVehicle = false;
    tmp.isContainer = true;
    tmp.container_size_x = 6;
    tmp.container_size_y = 1;
    tmp.isSkill = false;
    tmp.apply_to_body = false;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = false;
    tmp.slot = WEAPON_BOW;
    tmp.warmth = 0.0;
    tmp.weapon_damage = 0.2;
    tmp.weapon_range = 8;
    g.item_info.push_back(tmp);
}
