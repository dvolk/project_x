#include <allegro5/allegro.h>
#include <allegro5/allegro_color.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>

#include <cstdint>

#include <string>
#include <unordered_map>
#include <vector>
#include <set>
#include <iostream>
#include <cmath>
#include <algorithm>

#include <sigc++/sigc++.h>

#include "./util.h"

const bool DEBUG_VISIBILITY = false;

using namespace std;
using namespace sigc;

struct MessageLog;
struct Grid;
struct UI;
struct Widget;
struct Button;
struct GridSystem;
struct TileMap;
struct Character;
struct MiniMap;
struct MainMapUI;
struct MiniMapUI;
struct ItemsUI;
struct VehicleUI;
struct EncounterUI;
struct CraftingUI;
struct SkillsUI;
struct ConditionUI;
struct CampUI;
struct ScavengeUI;
struct HardpointInfo;
struct ItemInfo;
struct LocationInfo;
struct Item;

// global state
struct Game {
    // display dimensions
    int display_x;
    int display_y;

    ALLEGRO_DISPLAY *display;
    ALLEGRO_FONT *font;

    ALLEGRO_COLOR color_white;
    ALLEGRO_COLOR color_black;
    ALLEGRO_COLOR color_grey;
    ALLEGRO_COLOR color_grey2;
    ALLEGRO_COLOR color_grey3;
    ALLEGRO_COLOR color_darkgrey;
    ALLEGRO_COLOR color_tile_tint;
    ALLEGRO_COLOR color_active_tile_tint;
    ALLEGRO_COLOR color_bg;

    Button *button_MainMap;
    Button *button_MiniMap;
    Button *button_Skills;
    Button *button_Crafting;
    Button *button_Items;
    Button *button_Condition;
    Button *button_Camp;
    Button *button_Vehicle;
    Button *button_endturn;
    Button *button_scavenge;

    TileMap *map;
    MiniMap *minimap;

    vector<ItemInfo> item_info;
    vector<LocationInfo> location_info;

    // Items UI
    HardpointInfo *right_hand_hold;
    HardpointInfo *left_hand_hold;
    HardpointInfo *right_hand;
    HardpointInfo *left_hand;
    HardpointInfo *back;
    HardpointInfo *head;
    HardpointInfo *neck;
    HardpointInfo *right_shoulder;
    HardpointInfo *left_shoulder;
    HardpointInfo *torso;
    HardpointInfo *legs;
    HardpointInfo *right_foot;
    HardpointInfo *left_foot;
    HardpointInfo *vehicle;

    // encounter UI
    HardpointInfo *encounter_selected;

    // condition UI
    HardpointInfo *medical_upper_torso;
    HardpointInfo *medical_lower_torso;
    HardpointInfo *medical_left_upper_leg;
    HardpointInfo *medical_right_upper_leg;
    HardpointInfo *medical_left_lower_leg;
    HardpointInfo *medical_right_lower_leg;
    HardpointInfo *medical_left_upper_arm;
    HardpointInfo *medical_right_upper_arm;
    HardpointInfo *medical_left_lower_arm;
    HardpointInfo *medical_right_lower_arm;

    vector<ALLEGRO_BITMAP *> bitmaps;

    UI *ui; // current UI

    MainMapUI *ui_MainMap;
    MiniMapUI *ui_MiniMap;
    SkillsUI *ui_Skills;
    CraftingUI *ui_Crafting;
    ItemsUI *ui_Items;
    ConditionUI *ui_Condition;
    CampUI *ui_Camp;
    VehicleUI *ui_Vehicle;
    EncounterUI *ui_Encounter;
    ScavengeUI *ui_Scavenge;

    MessageLog *log;

    Item *skills[64];

    // current mouse state
    int mouse_x;
    int mouse_y;
    int mouse_button;
    // if holding an item, this is its offset from the mouse pointer
    int hold_off_x;
    int hold_off_y;
    int key;

    // the main 8 buttons
    set<Button *> main_buttons;

    set<Widget *> all_widgets;

    // add a message to the message log
    void AddMessage(string str);
};

Game g;

struct Rect {
    float x1;
    float y1;
    float x2;
    float y2;

    void print(void);
};

__attribute__ ((const))
bool rectIntersect(int a_x, int a_y, int a_width, int a_height,
                   int b_x, int b_y, int b_width, int b_height) {
    return
        !(b_x >= a_x + a_width
          || b_x + b_width <= a_x
          || b_y >= a_y + a_height
          || b_y + b_height <= a_y);
}

void Rect::print(void) {
    printf("x1: %f y1: %f x2: %f y2: %f\n", x1, y1, x2, y2);
}

struct ItemInfo {
    string name;
    int grid_size_x;
    int grid_size_y;
    int maxStack;
    int weight; // [g]
    bool isVehicle;
    float condition;
    bool isContainer;
    int container_size_x;
    int container_size_y;
    bool skill;
    // can it be applied to a body part (i.e. bandages and disinfectants)?
    bool apply_to_body;
    // when applied to a body part, is it consumed (i.e. disinfectants)?
    bool consumed_on_application;
    // when used, is it consumed (i.e. food, water, medicine)?
    bool consumed_on_use;
    ALLEGRO_BITMAP *sprite;
};

void init_iteminfo(void) {
    ItemInfo tmp;
    /* 00 */
    tmp.name = "";
    tmp.grid_size_x = 1;
    tmp.grid_size_y = 1;
    tmp.maxStack = 1;
    tmp.sprite = NULL;
    tmp.isVehicle = false;
    tmp.isContainer = false;
    tmp.container_size_x = 0;
    tmp.container_size_y = 0;
    tmp.skill = false;
    tmp.apply_to_body = false;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = false;
    g.item_info.push_back(tmp);

    /* 01 */
    tmp.name = "Backpack";
    tmp.grid_size_x = 7;
    tmp.grid_size_y = 7;
    tmp.maxStack = 1;
    tmp.weight = 1000;
    tmp.sprite = g.bitmaps[24];
    tmp.isVehicle = false;
    tmp.isContainer = true;
    tmp.container_size_x = 10;
    tmp.container_size_y = 10;
    tmp.skill = false;
    tmp.apply_to_body = false;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = false;
    g.item_info.push_back(tmp);

    /* 02 */
    tmp.name = "First aid kit";
    tmp.grid_size_x = 6;
    tmp.grid_size_y = 6;
    tmp.maxStack = 1;
    tmp.weight = 750;
    tmp.sprite = g.bitmaps[22];
    tmp.isVehicle = false;
    tmp.isContainer = true;
    tmp.container_size_x = 5;
    tmp.container_size_y = 5;
    tmp.skill = false;
    tmp.apply_to_body = false;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = false;
    g.item_info.push_back(tmp);

    /* 03 */
    tmp.name = "Crowbar";
    tmp.grid_size_x = 14;
    tmp.grid_size_y = 2;
    tmp.maxStack = 1;
    tmp.weight = 2500;
    tmp.sprite = g.bitmaps[20];
    tmp.isVehicle = false;
    tmp.isContainer = false;
    tmp.container_size_x = 0;
    tmp.container_size_y = 0;
    tmp.skill = false;
    tmp.apply_to_body = false;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = false;
    g.item_info.push_back(tmp);

    /* 04 */
    tmp.name = "Shopping trolley";
    tmp.grid_size_x = 12;
    tmp.grid_size_y = 12;
    tmp.maxStack = 1;
    tmp.weight = 5000;
    tmp.sprite = g.bitmaps[26];
    tmp.isVehicle = true;
    tmp.isContainer = true;
    tmp.container_size_x = 20;
    tmp.container_size_y = 20;
    tmp.skill = false;
    tmp.apply_to_body = false;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = false;
    g.item_info.push_back(tmp);

    /* 05 */
    tmp.name = "Pill bottle";
    tmp.grid_size_x = 1;
    tmp.grid_size_y = 2;
    tmp.maxStack = 1;
    tmp.weight = 50;
    tmp.sprite = g.bitmaps[27];
    tmp.isVehicle = false;
    tmp.isContainer = true;
    tmp.container_size_x = 1;
    tmp.container_size_y = 2;
    tmp.skill = false;
    tmp.apply_to_body = false;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = false;
    g.item_info.push_back(tmp);

    /* 06 */
    tmp.name = "Flee";
    tmp.grid_size_x = 2;
    tmp.grid_size_y = 2;
    tmp.maxStack = 1;
    tmp.weight = -1;
    tmp.sprite = g.bitmaps[31];
    tmp.isVehicle = false;
    tmp.isContainer = false;
    tmp.container_size_x = 0;
    tmp.container_size_y = 0;
    tmp.skill = false;
    tmp.apply_to_body = false;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = false;
    g.item_info.push_back(tmp);

    /* 07 */
    tmp.name = "Single attack";
    tmp.grid_size_x = 2;
    tmp.grid_size_y = 2;
    tmp.maxStack = 1;
    tmp.weight = -1;
    tmp.sprite = g.bitmaps[32];
    tmp.isVehicle = false;
    tmp.isContainer = false;
    tmp.container_size_x = 0;
    tmp.container_size_y = 0;
    tmp.skill = false;
    tmp.apply_to_body = false;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = false;
    g.item_info.push_back(tmp);

    /* 08 */
    tmp.name = "Bullet";
    tmp.grid_size_x = 1;
    tmp.grid_size_y = 1;
    tmp.maxStack = 5;
    tmp.weight = 5;
    tmp.sprite = g.bitmaps[34];
    tmp.isVehicle = false;
    tmp.isContainer = false;
    tmp.container_size_x = 0;
    tmp.container_size_y = 0;
    tmp.skill = false;
    tmp.apply_to_body = false;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = false;
    g.item_info.push_back(tmp);

    /* 09 */
    tmp.name = "Arrow";
    tmp.grid_size_x = 6;
    tmp.grid_size_y = 1;
    tmp.maxStack = 5;
    tmp.weight = 75;
    tmp.sprite = g.bitmaps[35];
    tmp.isVehicle = false;
    tmp.isContainer = false;
    tmp.container_size_x = 0;
    tmp.container_size_y = 0;
    tmp.skill = false;
    tmp.apply_to_body = false;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = false;
    g.item_info.push_back(tmp);

    /* 10 */
    tmp.name = "Quick";
    tmp.grid_size_x = 6;
    tmp.grid_size_y = 2;
    tmp.maxStack = 1;
    tmp.weight = -1;
    tmp.sprite = g.bitmaps[36];
    tmp.isVehicle = false;
    tmp.isContainer = false;
    tmp.container_size_x = 0;
    tmp.container_size_y = 0;
    tmp.skill = true;
    tmp.apply_to_body = false;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = false;
    g.item_info.push_back(tmp);

    /* 11 */
    tmp.name = "Lockpicking";
    tmp.grid_size_x = 6;
    tmp.grid_size_y = 2;
    tmp.maxStack = 1;
    tmp.weight = -1;
    tmp.sprite = g.bitmaps[37];
    tmp.isVehicle = false;
    tmp.isContainer = false;
    tmp.container_size_x = 0;
    tmp.container_size_y = 0;
    tmp.skill = true;
    tmp.apply_to_body = false;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = false;
    g.item_info.push_back(tmp);

    /* 12 */
    tmp.name = "Myopia";
    tmp.grid_size_x = 6;
    tmp.grid_size_y = 2;
    tmp.maxStack = 1;
    tmp.weight = -1;
    tmp.sprite = g.bitmaps[38];
    tmp.isVehicle = false;
    tmp.isContainer = false;
    tmp.container_size_x = 0;
    tmp.container_size_y = 0;
    tmp.skill = true;
    tmp.apply_to_body = false;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = false;
    g.item_info.push_back(tmp);

    /* 13 */
    tmp.name = "Metabolism";
    tmp.grid_size_x = 6;
    tmp.grid_size_y = 2;
    tmp.maxStack = 1;
    tmp.weight = -1;
    tmp.sprite = g.bitmaps[39];
    tmp.isVehicle = false;
    tmp.isContainer = false;
    tmp.container_size_x = 0;
    tmp.container_size_y = 0;
    tmp.skill = true;
    tmp.apply_to_body = false;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = false;
    g.item_info.push_back(tmp);

    /* 14 */
    tmp.name = "Whiskey";
    tmp.grid_size_x = 1;
    tmp.grid_size_y = 2;
    tmp.maxStack = 5;
    tmp.weight = 50;
    tmp.sprite = g.bitmaps[40];
    tmp.isVehicle = false;
    tmp.isContainer = false;
    tmp.container_size_x = 0;
    tmp.container_size_y = 0;
    tmp.skill = false;
    tmp.apply_to_body = true;
    tmp.consumed_on_application = true;
    tmp.consumed_on_use = true;
    g.item_info.push_back(tmp);

    /* 14 */
    tmp.name = "Clean rag";
    tmp.grid_size_x = 3;
    tmp.grid_size_y = 2;
    tmp.maxStack = 10;
    tmp.weight = 50;
    tmp.sprite = g.bitmaps[41];
    tmp.isVehicle = false;
    tmp.isContainer = false;
    tmp.container_size_x = 0;
    tmp.container_size_y = 0;
    tmp.skill = false;
    tmp.apply_to_body = true;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = false;
    g.item_info.push_back(tmp);

    /* 16 */
    tmp.name = "Factory";
    tmp.grid_size_x = 2;
    tmp.grid_size_y = 2;
    tmp.maxStack = 1;
    tmp.weight = -1;
    tmp.sprite = g.bitmaps[42];
    tmp.isVehicle = false;
    tmp.isContainer = false;
    tmp.container_size_x = 0;
    tmp.container_size_y = 0;
    tmp.skill = false;
    tmp.apply_to_body = false;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = false;
    g.item_info.push_back(tmp);

    /* 17 */
    tmp.name = "Shack in the woods";
    tmp.grid_size_x = 2;
    tmp.grid_size_y = 2;
    tmp.maxStack = 1;
    tmp.weight = -1;
    tmp.sprite = g.bitmaps[43];
    tmp.isVehicle = false;
    tmp.isContainer = false;
    tmp.container_size_x = 0;
    tmp.container_size_y = 0;
    tmp.skill = false;
    tmp.apply_to_body = false;
    tmp.consumed_on_application = false;
    tmp.consumed_on_use = false;
    g.item_info.push_back(tmp);
}



struct Item {
    // position and size of the item on the grid
    // x1, y1 - grid offset in grid units
    // x2, y2 - size in grid units
    Rect pos;
    // index into g.item_info, which contains
    // information that's common to the item
    int info_index;
    // items can stack onto one item
    int cur_stack;
    // items can have their own storage space e.g.
    // a backpack. NULL if they don't
    Grid *storage;
    // the grid that the item belongs to. set to NULL
    // when the item is held by the player
    Grid *parent;
    // when the item is held, this is set to the parent
    // so that the item can be returned if it can't
    // be placed
    Grid *old_parent;
    // item's condition. Once it goes negative the item should be
    // destroyed. Item conditions should be decreased a bit on
    // each turn, and seperately when the item is used, so all items
    // should be checked at the end of the turn, and individual items
    // when they're used float condition;
    // float condition;

    Item(const Item& i) {
        this->pos = i.pos;
        this->cur_stack = i.cur_stack;
        this->info_index = i.info_index;
        this->parent = i.parent;
        this->old_parent = i.old_parent;
        this->storage = i.storage;
    }
    Item(int info_index);
    Item(string item_name);
    Item(string item_name, int num_stack);
    ~Item();

    void init(int info_index);
    void init_from_name(string name);
    void resetHardpointPos(void);

    void draw(void);

    const char *getName(void) {
        return g.item_info[info_index].name.c_str();
    }
    bool isVehicle(void) {
        return g.item_info[info_index].isVehicle;
    }
    bool isMedical(void) {
        return g.item_info[info_index].apply_to_body;
    }
    bool isConsumedOnApplication(void) {
        return g.item_info[info_index].consumed_on_application;
    }
    bool isUsable(void) {
        return
            storage == NULL;
    }
    bool isConsumedOnUse(void) {
        return g.item_info[info_index].consumed_on_use;
    }
};

struct Widget {
    Rect pos;
    bool visible;

    signal<void> onMouseDown;
    signal<void> onMouseUp;
    signal<void> onKeyDown;

    virtual void mouseDown(void) = 0;
    virtual void mouseUp(void) = 0;
    virtual void keyDown(void) = 0;
    virtual void hoverOver(void) { };

    Widget() {
        visible = true;
        g.all_widgets.insert(this);
    }

    virtual ~Widget() {
        // info("~Widget()");
    };

    virtual void draw(void) = 0;
    virtual void update() { };
};

struct MessageLog : public Widget {
    ALLEGRO_BITMAP *background;
    ALLEGRO_FONT *font;
    vector<string> lines;

    MessageLog() { }
    ~MessageLog();

    void mouseDown(void) { }
    void mouseUp(void) { }
    void keyDown(void) { }

    void draw(void);
};

struct HardpointInfo {
    ALLEGRO_BITMAP *sprite;
    int maxItems;
    bool vehiclepoint;
    bool medical;

    HardpointInfo() {
        vehiclepoint = false;
        maxItems = 1;
        sprite = NULL;
        medical = false;
    }
};

void init_hardpointinfo(void) {
    g.right_hand_hold = new HardpointInfo;
    g.left_hand_hold = new HardpointInfo;
    g.right_hand = new HardpointInfo;
    g.left_hand = new HardpointInfo;
    g.back = new HardpointInfo;
    g.head = new HardpointInfo;
    g.neck = new HardpointInfo;
    g.right_shoulder = new HardpointInfo;
    g.left_shoulder = new HardpointInfo;
    g.torso = new HardpointInfo;
    g.legs = new HardpointInfo;
    g.right_foot = new HardpointInfo;
    g.left_foot = new HardpointInfo;
    g.vehicle = new HardpointInfo;
    g.encounter_selected = new HardpointInfo;
    g.medical_upper_torso = new HardpointInfo;
    g.medical_lower_torso = new HardpointInfo;
    g.medical_left_upper_leg = new HardpointInfo;
    g.medical_right_upper_leg = new HardpointInfo;
    g.medical_left_lower_leg = new HardpointInfo;
    g.medical_right_lower_leg = new HardpointInfo;
    g.medical_left_upper_arm = new HardpointInfo;
    g.medical_right_upper_arm = new HardpointInfo;
    g.medical_left_lower_arm = new HardpointInfo;
    g.medical_right_lower_arm = new HardpointInfo;

    g.medical_upper_torso->medical = true;
    g.medical_lower_torso->medical = true;
    g.medical_left_upper_leg->medical = true;;
    g.medical_right_upper_leg->medical = true;;
    g.medical_left_lower_leg->medical = true;;
    g.medical_right_lower_leg->medical = true;;
    g.medical_left_upper_arm->medical = true;;
    g.medical_right_upper_arm->medical = true;;
    g.medical_left_lower_arm->medical = true;;
    g.medical_right_lower_arm->medical = true;;
    g.torso->maxItems = 3;
    g.vehicle->vehiclepoint = true;
}

struct Button : public Widget {
    bool pressed;
    ALLEGRO_BITMAP *up;
    ALLEGRO_BITMAP *down;

    Button();
    ~Button();

    void mouseDown(void);
    void mouseUp(void);
    void keyDown(void);
    void hoverOver(void);

    void press(void);
    void draw(void);
    void update(void);
};

struct GridSortButton : public Widget {
    ALLEGRO_BITMAP *up;
    Grid *parent;

    GridSortButton(Grid *parent);

    void draw(void);

    void reset(void);
    void mouseDown(void);
    void mouseUp(void) { }
    void keyDown(void) { }
    void hoverOver(void) { }
};

struct Grid {
    /* TODO: since only the player needs this, pos could be moved
       to GridSystem to save memory
    */
    // "widget" dimensions
    Rect pos;

    // grid spacing in pixels.
    // can be changed, but items won't be scaled
    static const int8_t grid_px_x = 18;
    static const int8_t grid_px_y = 18;

    // grid size in grid units
    int8_t grid_size_x;
    int8_t grid_size_y;

    vector<Item *> items;

    HardpointInfo *hpinfo;

    // seems like these could be part of InventoryGridSystem, etc
    GridSortButton *gsb;
    bool gsb_displayed;

    static bool PlaceItemWantsStacking;

    Grid(int w_pos_x, int w_pos_y, int size_x, int size_y, HardpointInfo *h) {
        hpinfo = h;
        grid_size_x = size_x;
        grid_size_y = size_y;
        pos.x1 = w_pos_x;
        pos.y1 = w_pos_y;
        resetPos();
        gsb = NULL;
        if(hpinfo == NULL)
            gsb = new GridSortButton (this);
        gsb_displayed = false;
    }

    ~Grid() {
        // info("~Grid()");
        // a grid owns its items
        for(auto &item : items)
            delete item;
    }

    void draw(void);
    void drawAt(float x, float y);

    void AddItem(Item *item);
    void RemoveItem(Item *item);
    Item *PlaceItem(Item *to_place);
    void Sort(void);
    void Sort(bool (*comp)(Item *l, Item *r));

    void resetPos(void) {
        pos.x2 = pos.x1 + grid_size_x * grid_px_x;
        pos.y2 = pos.y1 + grid_size_y * grid_px_y;
    }
    Item *grab_item(int x, int y); // get item at screen position
    void unstack_item(int x, int y); // unstack item at screen position

    bool item_compatible(Item *i);
};

bool Grid::PlaceItemWantsStacking = true;

bool BiggerItemsFirst(Item *l, Item *r) {
    return l->pos.x2 * l->pos.y2 > r->pos.x2 * r->pos.y2;
}

bool SmallerItemsFirst(Item *l, Item *r) {
    return l->pos.x2 * l->pos.y2 < r->pos.x2 * r->pos.y2;
}

void Grid::Sort(void) {
    Sort(BiggerItemsFirst);
}

vector<Grid *> *ground_at_player();
void PlaceItemOnMultiGrid(vector<Grid *> *ground, Item *item);

void Grid::Sort(bool (*comp)(Item *l, Item *r)) {
    int num_items = items.size();
    for(auto& item : items) {
        item->pos.x1 = -999;
        item->pos.y1 = -999;
    }

    sort(items.begin(), items.end(), comp);

    items.reserve(2 * num_items);

    // placing items with stacking on may delete an item, so
    // it's turned off here for now.
    PlaceItemWantsStacking = false;
    for(auto& item : items) {
        Item *ret = PlaceItem(item);
        if(ret != NULL) {
            // TODO: this can fail
        }
    }
    PlaceItemWantsStacking = true;

    items.erase(items.begin(), items.begin() + num_items);

    assert((int)items.size() == num_items);
}

void Item::init(int info_index) {
    pos.x1 = 0;
    pos.y1 = 0;
    pos.x2 = g.item_info[info_index].grid_size_x;
    pos.y2 = g.item_info[info_index].grid_size_y;
    parent = NULL;
    old_parent = NULL;
    cur_stack = 1;
    storage = NULL;
    this->info_index = info_index;
    if(g.item_info[info_index].isContainer == true)
        storage = new Grid(0,
                           0,
                           g.item_info[info_index].container_size_x,
                           g.item_info[info_index].container_size_y,
                           NULL);
}

Item::Item(int info_index) {
    init(info_index);
}

Item::Item(string item_name, int num_stack) {
    init_from_name(item_name);
    this->cur_stack = num_stack;
}

Item::Item(string item_name) {
    init_from_name(item_name);
}

void Item::init_from_name(string item_name) {
    int i = 0;
    for(auto& info : g.item_info) {
        if(info.name == item_name) {
            init(i);
            return;
        }
        i++;
    }

    char buf[75];
    snprintf(buf, sizeof(buf), "Unknown item: %s", item_name.c_str());
    errorQuit(buf);
}

void Grid::AddItem(Item *item) {
    item->old_parent = item->parent;
    item->parent = this;
    items.push_back(item);
}

void Grid::RemoveItem(Item *to_remove) {
    bool found = false;
    int c = 0;
    for(auto& item: items) {
        if(item == to_remove) {
            found = true;
            break;
        }
        c++;
    }
    assert(found == true);
    items.erase(items.begin() + c);
}

/*
  TODO: this function is too big
*/
// automatically find a place to place the item on
// return NULL if successful, otherwise return the item
Item *Grid::PlaceItem(Item *to_place) {
    if(hpinfo != NULL) {
        // if we're on a hard point just check how many item there
        // are there already
        if((int)items.size() < hpinfo->maxItems) {
            to_place->pos.x1 = 0;
            to_place->pos.y1 = 0;
            AddItem(to_place);
            return NULL;
        }
        return to_place;
    }

    // if it's a grid we try placing it on every grid square
    bool found = false;
    int drop_x = 0;
    int drop_y = 0;
    // item to merge with if we're stacking it
    Item *merge_with = NULL;
    // skip grid squares that place the item beyond grid bounds
    const int limit_x = grid_size_x - to_place->pos.x2;
    const int limit_y = grid_size_y - to_place->pos.y2;
    // we check every grid square
    for(drop_y = 0; drop_y <= limit_y ; drop_y++) {
        for(drop_x = 0; drop_x <= limit_x; drop_x++) {
            int collides_with = 0;
            for(auto& item : items) {
                // check if we can stack it with an existing item
                if(PlaceItemWantsStacking == true) {
                    // if this is the same item type
                    if(item->info_index == to_place->info_index) {
                        // check if we can add it to the stack
                        if(item->cur_stack < g.item_info[item->info_index].maxStack) {
                            if(item != to_place) {
                                // don't merge with yourself
                                merge_with = item;
                                found = true;
                                goto done;
                            }
                        }
                    }
                }

                // check how many items it collides with
                if(rectIntersect(item->pos.x1 * grid_px_x,
                                 item->pos.y1 * grid_px_y,
                                 item->pos.x2 * grid_px_x,
                                 item->pos.y2 * grid_px_y,
                                 drop_x * grid_px_x,
                                 drop_y * grid_px_y,
                                 to_place->pos.x2 * grid_px_x,
                                 to_place->pos.y2 * grid_px_y)) {
                    collides_with += 1;
                }
            }
            if(collides_with == 0) {
                found = true;
                goto done;
            }
        }
    }
 done:
    if(found == true) {
        cout << "placing " << g.item_info[to_place->info_index].name << " on " << this << endl;

        to_place->pos.x1 = drop_x;
        to_place->pos.y1 = drop_y;
        // if we're merging it
        if(merge_with != NULL) {
            // check if we can merge all of it into the item
            if(merge_with->cur_stack + to_place->cur_stack <= g.item_info[to_place->info_index].maxStack) {
                merge_with->cur_stack += to_place->cur_stack;
                // if so, delete the item
                delete to_place;
                return NULL;
            } else {
                // otherwise we still have some number of them to place
                int moved = g.item_info[to_place->info_index].maxStack - merge_with->cur_stack;
                merge_with->cur_stack += moved;
                to_place->cur_stack -= moved;
                // recursively place the rest
                Item *ret = PlaceItem(to_place);
                return ret;
            }
            return NULL;
        }
        // clear the grid sort button if the new grid isn't a hardpoint
        if(to_place->storage != NULL &&
           this->hpinfo == NULL) {
            to_place->storage->gsb_displayed = false;
        }
        AddItem(to_place);
        return NULL;
    }
    else {
        // couldn't place it
        return to_place;
    }
}

bool Grid::item_compatible(Item *i) {
    // you can place anything in a grid
    if(hpinfo == NULL)
        return true;

    // can't place/use non-medical items on body parts
    if(hpinfo->medical == true &&
       i->isMedical() == false)
        return false;

    // can only place vehicles in vehicle hardpoint
    if(hpinfo->vehiclepoint == true &&
       i->isVehicle() == false)
        return false;

    // can't place vehicle on non-vehicle hardpoints
    if(i->isVehicle() == true &&
       hpinfo->vehiclepoint == false)
        return false;

    // allow by default
    return true;
}

Item::~Item() {
    delete storage;
    //    info("~Item()");
}

void Item::resetHardpointPos(void) {
    if(parent != NULL) {
        storage->pos.x1 = parent->pos.x1 + pos.x2 * Grid::grid_px_x + 10;
        storage->pos.y1 = parent->pos.y1;
        storage->resetPos();
        storage->gsb->reset();
    } else {
        info("WARNING: no item parent");
    }
}

struct Character {
    int n; // position by offset

    Grid *right_hand_hold;
    Grid *left_hand_hold;
    Grid *right_hand;
    Grid *left_hand;
    Grid *back;
    Grid *head;
    Grid *neck;
    Grid *right_shoulder;
    Grid *left_shoulder;
    Grid *torso;
    Grid *legs;
    Grid *right_foot;
    Grid *left_foot;
    Grid *vehicle;

    Grid *medical_upper_torso;
    Grid *medical_lower_torso;
    Grid *medical_left_upper_leg;
    Grid *medical_right_upper_leg;
    Grid *medical_left_lower_leg;
    Grid *medical_right_lower_leg;
    Grid *medical_left_upper_arm;
    Grid *medical_right_upper_arm;
    Grid *medical_left_lower_arm;
    Grid *medical_right_lower_arm;

    vector<Grid *> inventory_hardpoints;

    ALLEGRO_BITMAP *sprite;
    vector<int> currently_seeing;
    // 0 - sees only tile they're on
    // 1 - sees immediate neighbours
    // etc
    int current_los_distance;
    int nextMove;

    uint64_t skills;

    Character();

    ~Character() {
        for(auto& hardpoint : inventory_hardpoints) {
            delete hardpoint;
        }
        delete vehicle;
        info("~Character()");
    };

    void update_visibility(void);

    void draw(void);
    void drawOffset(int offset_x, int offset_y);

    /*
      TODO why is this here
    */
    void addInventoryHardpoints(GridSystem *gs);
    void addVehicleHardpoint(GridSystem *gs);

    void do_AI_map_turn(void);
    void die(void);
    void drop_all_items(void);
    void randomMove(void);
    void move(int new_n);

    bool hasSkill(int n);
    void enableSkill(int n);
    void disableSkill(int n);

    void useItem(Item *i);
};

void Character::useItem(Item *i) {
    if(i->isConsumedOnUse()) {
        i->parent->RemoveItem(i);
        delete i;
    }
    cout << "used " << i << endl;
}

bool Character::hasSkill(int n) {
    return skills & (uint64_t(1) << n);
}

void Character::enableSkill(int n) {
    skills |= uint64_t(1) << n;
}

void Character::disableSkill(int n) {
    skills &= ~(uint64_t(1) << n);
}

bool isCraftingSkill(int n) {
    return
        n == 1 || // lockpicking
        n == 1;
}

Character::Character(void) {
    this->n = 0;
    this->sprite = g.bitmaps[21];

    const int off_x = 555;
    const int off_y = 85;

    neck = new Grid (off_x, off_y - 75, 2, 2, g.neck);
    head = new Grid (off_x, off_y - 15, 2, 3, g.head);
    right_shoulder = new Grid (off_x - 53, off_y + 10, 2, 2, g.right_shoulder);
    left_shoulder = new Grid (off_x + 53, off_y + 10, 2, 2, g.left_shoulder);
    torso = new Grid (off_x - 22, off_y + 50, 5, 9, g.torso);
    right_hand = new Grid (off_x - 75, off_y + 190, 2, 2, g.right_hand_hold);
    left_hand = new Grid (off_x + 85, off_y + 190, 2, 2, g.left_hand_hold);
    legs = new Grid (off_x - 12, off_y + 220, 4, 11, g.legs);
    right_foot = new Grid (off_x - 25, off_y + 430, 2, 2, g.right_foot);
    left_foot = new Grid (off_x + 30, off_y + 430, 2, 2, g.left_foot);

    back = new Grid(680, 10, 2, 2, g.back);
    right_hand_hold = new Grid (680, 195, 2, 2, g.right_hand_hold);
    left_hand_hold = new Grid (680, 380, 2, 2, g.left_hand_hold);

    vehicle = new Grid(500, 150, 2, 2, g.vehicle);

    medical_upper_torso = new Grid(off_x - 22, off_y + 50, 5, 4, g.medical_upper_torso);
    medical_lower_torso = new Grid(off_x - 22, off_y + 132, 5, 4, g.medical_lower_torso);
    medical_left_upper_leg = new Grid(off_x - 22, off_y + 214, 2, 5, g.medical_left_upper_leg);
    medical_right_upper_leg = new Grid(off_x + 32, off_y + 214, 2, 5, g.medical_right_upper_leg);
    medical_left_lower_leg = new Grid(off_x - 22, off_y + 314, 2, 6, g.medical_left_lower_leg);
    medical_right_lower_leg = new Grid(off_x + 32, off_y + 314, 2, 6, g.medical_left_lower_leg);
    medical_left_upper_arm = new Grid(off_x - 62, off_y + 50, 2, 4, g.medical_upper_torso);
    medical_right_upper_arm = new Grid(off_x + 72, off_y + 50, 2, 4, g.medical_upper_torso);
    medical_left_lower_arm = new Grid(off_x - 62, off_y + 132, 2, 5, g.medical_upper_torso);
    medical_right_lower_arm = new Grid(off_x + 72, off_y + 132, 2, 5, g.medical_upper_torso);

    inventory_hardpoints.push_back(right_hand_hold);
    inventory_hardpoints.push_back(left_hand_hold);
    inventory_hardpoints.push_back(right_hand);
    inventory_hardpoints.push_back(left_hand);
    inventory_hardpoints.push_back(back);
    inventory_hardpoints.push_back(head);
    inventory_hardpoints.push_back(neck);
    inventory_hardpoints.push_back(right_shoulder);
    inventory_hardpoints.push_back(left_shoulder);
    inventory_hardpoints.push_back(torso);
    inventory_hardpoints.push_back(legs);
    inventory_hardpoints.push_back(right_foot);
    inventory_hardpoints.push_back(left_foot);

    current_los_distance = 3;
    currently_seeing.reserve(50);
    nextMove = 1000;

    skills = 0;

    info("Character()");
}

int dir_transform(int n, int dir);

// data that's common to all tiles of the same type
struct TileInfo {
    int bitmap_index;
    ALLEGRO_COLOR minimap_color;
    bool blocks_los;
    bool blocks_movement;
    bool has_locations;
};

// data that's common to all locations of the same type
struct LocationInfo {
    Item *location_item;
    string description;

    // these are base values. The actual values depend on where on the map
    // you are
    int8_t base_loot_level;
    int8_t base_safety_level;
    int8_t base_sneak_level;
};

void init_locationdata(void) {
    Item *factory = new Item ("Factory");
    Item *shack = new Item ("Shack in the woods");

    LocationInfo tmp;
    tmp.base_loot_level = 100;
    tmp.base_safety_level = 100;
    tmp.base_sneak_level = 100;
    tmp.location_item = factory;
    tmp.description = "An abandoned factory, full of decaying industrial equipment.";
    g.location_info.push_back(tmp);

    tmp.description = "A shack in the woods.";
    tmp.location_item = shack;
    g.location_info.push_back(tmp);
}

// a location on the
struct Location {
    int8_t info_index; // index into LocationInfo
    int last_looted;
};

// a hex tile
struct Tile {
    int8_t info_index; // index into TileInfo
    int8_t visible;
    vector<Grid *> *ground_items;
    vector<Location *> *locations;
};

struct TileMap : public Widget {
    // display dimensions
    int cols;
    int rows;
    // current view
    int view_x;
    int view_y;
    // dimensions of the map
    int size_x;
    int size_y;
    // rendering offset
    int r_off_x;
    int r_off_y;
    /*
      TODO: replace the magic numbers throughout with these
    */
    // size of the hex bitmaps
    const int hex_size_x = 100;
    const int hex_size_y = 80;
    // size of the hex render steps
    const int hex_step_x = 80;
    const int hex_step_y = 40;

    // stuff that's constant between TileMap::draw
    // and TileMap::drawTile
    int max_t;
    int start;
    int mouse_n;

    // map from positions to characters on that position
    unordered_multimap<int, Character *> charsByPos;

    vector<Character *> characters;
    Character *player;

    // array of size_x * size_y tiles, allocated in constructor
    // TODO: would a 2D array be better?
    vector <Tile> tiles;

    vector<TileInfo> tile_info;
    vector<ALLEGRO_BITMAP *>bitmaps;

    int pos_to_kill;

    TileMap(int sx, int sy, int cols, int rows);
    ~TileMap();

    void handleKeyDown(void);

    void mouseDown(void);
    void mouseUp(void) { }
    void keyDown(void) { handleKeyDown(); }

    // must be called after the bitmaps vector is filled
    void generate(void);

    void draw(void);
    void drawTile(int i, int x, int y);
    void drawTopHalfOfTileAt(int x, int y);
    void mouseToTileXY(int &x, int &y);

    int mouseToTileN(void);
    void focusOnPlayer(void);
    Character *characterAt(int n); // this is never used?
    bool playerSees(int n);

    void updateCharsByPos(void);
    void removeCharacter(Character *to_kill);
    void addRandomCharacter(void);
};

bool good_index(int n) {
    return n >= 0 && n <= g.map->max_t;
}

int dir_transform(int n, int dir);

void TileMap::addRandomCharacter(void) {
    Character *new_char = new Character;
    new_char->move(rand() % (max_t - 1));
    Item *pill_bottle = new Item("Pill bottle");
    Item *shopping_trolley = new Item("Shopping trolley");
    Item *first_aid_kit = new Item("First aid kit");
    new_char->left_hand_hold->PlaceItem(pill_bottle);
    new_char->vehicle->PlaceItem(shopping_trolley);
    new_char->right_hand_hold->PlaceItem(first_aid_kit);
    new_char->nextMove = characters.front()->nextMove + rand() % 1234;
    characters.push_back(new_char);
}

void TileMap::removeCharacter(Character *to_kill) {
    assert(to_kill != NULL);

    if(to_kill == player)
        // this comes later
        return;

    to_kill->drop_all_items();

    int i = 0;
    for(auto& character : g.map->characters) {
        if(character == to_kill) {
            g.map->characters.erase(g.map->characters.begin() + i);
            delete to_kill;
            updateCharsByPos();
            return;
        }
        i++;
    }
}

void TileMap::updateCharsByPos(void) {
    cout << "updateCharsByPos" << endl;
    charsByPos.clear();

    charsByPos.emplace(player->n, player);

    for(auto& npc : characters) {
        assert(npc != NULL);
        charsByPos.emplace(npc->n, npc);
    }
}

// do stuff on the map
void Character::do_AI_map_turn(void) {
}

void Character::randomMove(void) {
    int new_n;

    do {
        new_n = dir_transform(n, 1 + rand() % 5);
    } while(good_index(new_n) == false ||
            g.map->tile_info[g.map->tiles[new_n].info_index].blocks_movement == true);

    move(new_n);
}

void runEncounter(void);

void Character::move(int new_n) {
    if(good_index(new_n) == true) {
        n = new_n;
        g.map->updateCharsByPos();
        if(this != g.map->player &&
           n == g.map->player->n) {
            runEncounter();
        }
    }
    else {
        info("WARNING: tried to Character::move() to invalid index");
    }
    nextMove += 1000 + rand() % 100;
}

void Character::die(void) {
    g.map->removeCharacter(this);
}

vector<Grid *> *ground_at_character(Character *character);
void PlaceItemOnMultiGrid(vector<Grid *> *multigrid, Item *item);

void Character::drop_all_items(void) {
    cout << this << " drop_all_items at " << this->n << endl;
    vector<Grid *> *ground = ground_at_character(this);
    for(auto& hardpoint : inventory_hardpoints) {
        for(auto& item : hardpoint->items) {
            hardpoint->RemoveItem(item);
            PlaceItemOnMultiGrid(ground, item);
        }
    }
    for(auto& item : vehicle->items) {
        vehicle->RemoveItem(item);
        PlaceItemOnMultiGrid(ground, item);
    }
}

Button::Button() {
    pressed = false;
    up = NULL;
    down = NULL;
}

struct GridSystem : public Widget {
    vector<Grid *> grids;

    // grids for which we can't manually take or place items (the crafting results)
    vector<Grid *> interaction_forbidden;

    Item *held;
    bool auto_move_to_ground;
    Grid *auto_target;

    // fires when an item -may- have been placed or removed
    // used to recompute crafting output
    signal<void> change;

    // fires when an item is applied to/removed from a hardpoint (used for
    // applying disinfectant to wounds
    signal<void> applied;
    signal<void> removed; /* TODO */

    /*
      TODO: work out how to connect signal with parameters
    */
    pair<Grid*, Item*> applied_params;

    GridSystem(void) {
        auto_move_to_ground = false;
        auto_target = NULL;
        held = NULL;
    }
    ~GridSystem(void) {
        delete held;
        info("~GridSystem()");
    }

    // resets the visibility of the grid sort buttons
    virtual void reset(void) {
        for(auto& grid : grids) {
            for(auto& item : grid->items) {
                if(item->storage != NULL &&
                   item->parent->hpinfo != NULL) {
                    item->storage->gsb_displayed = true;
                }
            }
        }
    }

    void mouseDown(void) {
        gsMouseDownEvent();
    }
    void mouseUp(void) {
        gsMouseUpEvent();
    }
    void keyDown(void);
    void hoverOver(void);

    void gsMouseDownEvent(void);
    void gsMouseUpEvent(void);

    void draw(void);
    void drawItemTooltip(void);

    void addStorageGrid(void);
    void reparent(void);
    void countTotalItems(void);
    void GrabItem(void);

    void AutoMoveItem(Item *item, Grid *from, Grid *to);
    void MouseAutoMoveItemToGround();
    void MouseAutoMoveItemToTarget();
};

vector<Grid *> *ground_at_player(void);

void GridSystem::keyDown(void) {
}

void GridSystem::AutoMoveItem(Item *item, Grid *from, Grid *to) {
    from->RemoveItem(item);
    to->PlaceItem(item);
}

// moves items under mouse cursor to the ground, if possible
// this is always for the player
void GridSystem::MouseAutoMoveItemToGround() {
    vector<Grid *> *ground = ground_at_player();

    Item *item = NULL;
    Grid *from = NULL;

    for(auto& grid : grids) {
        item = grid->grab_item(g.mouse_x, g.mouse_y);
        if(item != NULL) {
            from = grid;
            goto got_it;
        }
    }
    return;

 got_it:
    assert(item != NULL);
    assert(from != NULL);

    PlaceItemOnMultiGrid(ground, item);
    reset();
}

// moves item under mouse cursor to the auto_target, if it exists
// this is always for the player
void GridSystem::MouseAutoMoveItemToTarget() {
    assert(auto_target != NULL);

    Item *item = NULL;
    Grid *from = NULL;

    for(auto& grid : grids) {
        item = grid->grab_item(g.mouse_x, g.mouse_y);
        if(item != NULL) {
            from = grid;
            goto got_it;
        }
    }
    return;

 got_it:
    assert(item != NULL);
    assert(from != NULL);

    if(auto_target->hpinfo != NULL) {
        // if we're auto-moving to a hard point
        if((int)auto_target->items.size() >= auto_target->hpinfo->maxItems) {
            // and there's no space
            Item *prev = auto_target->items.front();
            assert(prev->old_parent);
            // then replace the item that's already there.
            prev->old_parent->PlaceItem(prev);
            auto_target->RemoveItem(prev);
        }
    }
    auto_target->PlaceItem(item);
    reset();
}

// set the parent grids of all the items in the gridsystem
// this shouldn't be needed any more
void GridSystem::reparent(void) {
    for(auto& grid : grids)
        for(auto& item : grid->items)
            item->parent = grid;
}

// Debugging: display the number of grids and items
void GridSystem::countTotalItems(void) {
    int i = 0;
    int g = 0;
    for(auto& grid : grids) {
        g++;
        for(__attribute__ ((unused)) auto& item : grid->items)
            i++;
    }
    cout << "GridSystem: " << g << " grids " << i << " items." << endl;
}

void GridSystem::hoverOver(void) {
}

// sets the hardpoint's item's parents
// and the item's storage grid positions
void Character::addInventoryHardpoints(GridSystem *gs) {
    // add the grids at the appropriate places
    for(auto &hardpoint : inventory_hardpoints) {
        for(auto& item : hardpoint->items) {
            if(item->storage != NULL) {
                item->parent = hardpoint;
                item->resetHardpointPos();
                gs->grids.push_back(item->storage);
            }
        }
    }
}

void Character::addVehicleHardpoint(GridSystem *gs) {
    // add the grids at the appropriate places
    for(auto& item : vehicle->items) {
        if(item->storage != NULL) {
            item->parent = vehicle;
            item->resetHardpointPos();
            gs->grids.push_back(item->storage);
        }
    }
}

Character *TileMap::characterAt(int n) {
    if(player->n == n)
        return player;
    for(auto& character : characters)
        if(character->n == n)
            return character;
    return NULL;
}

TileMap::~TileMap() {
    // tilemap owns characters
    delete player;
    player = NULL;
    for(auto& character : characters) {
        delete character;
        character = NULL;
    }
    for(auto& tile : tiles)
        if(tile.ground_items != NULL)
            for(auto& grid : *tile.ground_items)
                delete grid;
}

TileMap::TileMap(int sx, int sy, int c, int r) {
    if(sx % 2 != 0) {
        info("WARNING: TileMap dimensions must be even.");
        sx += 1;
    }
    if(sy % 2 != 0) {
        info("WARNING: TileMap dimensions must be even.");
        sy += 1;
    }

    cols = min(sx, c);
    rows = min(sy, r);

    size_x = sx;
    size_y = sy;

    // hex_size_x = 100;
    // hex_size_y = 80;

    // hex_step_x = 80;
    // hex_step_y = 40;

    r_off_x = 0;
    r_off_y = 0;

    if(size_x * hex_step_x < g.display_x)
        r_off_x = (g.display_x - (size_x + 0.5) * hex_step_x) / 2;
    if(size_y * hex_step_y < g.display_y)
        r_off_y = (g.display_y - size_y * hex_step_y) / 4;

    pos.x1 = r_off_x;
    pos.y1 = r_off_y;
    pos.x2 = min(g.display_x, (int)pos.x1 + (size_x + 1) * hex_step_x);
    pos.y2 = min(g.display_y, (int)pos.y1 + (size_y + 1) * hex_step_y);

    max_t = size_x * size_y - 1;

    cout << "Tilemap rendering dimensions: "
         << pos.x1 << " " << pos.y1 << " " << pos.x2 << " " << pos.y2 << endl;
}

void TileMap::focusOnPlayer(void) {
    int p_x = player->n % size_x;
    int p_y = player->n / size_x;
    view_x = max(0, p_x - cols/2 + 1);
    view_y = max(0, p_y - rows/2 + 2);
    view_x = max(0, min(view_x, size_x - cols));
    view_y = max(0, min(view_y, size_y - rows));
}

struct MiniMap : public Widget {
    MiniMap();
    ~MiniMap();

    int size_x;
    int size_y;
    int off_x;
    int off_y;

    ALLEGRO_BITMAP *buf;

    void mouseDown(void) { }
    void mouseUp(void) { }
    void keyDown(void) { }

    void draw(void);
    void update(void) { }
    void recreate(void);
};

MiniMap::MiniMap() {
    size_x = g.map->size_x * 2 + 4;
    size_y = g.map->size_y * 2 + 4;

    off_x = (g.display_x - size_x) / 2;
    off_y = (g.display_y - size_y) / 3;

    pos.x1 = off_x;
    pos.y1 = off_y;
    pos.x2 = off_x + size_x;
    pos.y2 = off_y + size_y;

    buf = al_create_bitmap(size_x, size_y);
}

MiniMap::~MiniMap() {
    al_destroy_bitmap(buf);
    info("~MiniMap()");
}

void MiniMap::recreate() {
    al_set_target_bitmap(buf);

    al_clear_to_color(g.color_black);
    al_draw_rectangle(1, 1, size_x - 1, size_y - 1, g.color_white, 2);

    // draw the minimap
    Tile *t = NULL;
    for(int y = 0; y < g.map->size_y; y++) {
        int off_y = g.map->size_x * y;
        for(int x = 0; x < g.map->size_x; x++) {
            t = &g.map->tiles[off_y + x];
            if(t->visible) {
                ALLEGRO_COLOR col = g.map->tile_info[t->info_index].minimap_color;
                // four pixels per hex cell
                al_put_pixel(x * 2 + 2, y * 2 + 2, col);
                al_put_pixel(x * 2 + 3, y * 2 + 2, col);
                al_put_pixel(x * 2 + 2, y * 2 + 3, col);
                al_put_pixel(x * 2 + 3, y * 2 + 3, col);
            }
        }
    }

    ALLEGRO_COLOR red = al_map_rgb(255, 0, 0);

    // if the tilemap dimensions are larger than the screen
    if(g.map->r_off_x == 0 || g.map->r_off_y == 0) {
        // draw player location lines
        int p_x = g.map->player->n % g.map->size_x;
        int p_y = g.map->player->n / g.map->size_x;
        al_draw_line(p_x * 2 + 2, 2, p_x * 2 + 2, size_y - 2, red, 2);
        al_draw_line(2, p_y * 2 + 2, size_x, p_y * 2 + 2, red, 2);

        // draw mainmap view rectangle
        al_draw_rectangle(g.map->view_x * 2 + 2,
                          g.map->view_y * 2 + 2,
                          (g.map->view_x + 20) * 2 + 2,
                          (g.map->view_y + 18) * 2 + 2, red, 2);
    }
    al_set_target_backbuffer(g.display);
}

void MiniMap::draw(void) {
    al_draw_bitmap(buf, off_x, off_y, 0);
}

void TileMap::mouseToTileXY(int &x, int &y) {
    x = view_x + floor((g.mouse_x - r_off_x) / 80);
    int off_y = x % 2 == 0 ? 0 : 20;
    y = view_y + floor((g.mouse_y - 40 - off_y - r_off_y) / 40);
}

int TileMap::mouseToTileN(void) {
    if(g.mouse_x < pos.x1 ||
       g.mouse_y < pos.y1 + 40 ||
       g.mouse_x > pos.x2 - 81 ||
       g.mouse_y > pos.y2 - 1) {
        return -1;
    }
    int x, y;
    mouseToTileXY(x, y);
    return size_x * y + x;
}

void end_turn(void);

// check if the clicked tile is next to the player
// if so, move them there.
void TileMap::mouseDown(void) {
    int clicked_n = mouseToTileN();
    int player_n = player->n;

    if(clicked_n == player_n ||
       clicked_n == -1)
        return;

    int clicked_nearby = -1;
    for(int dir = 1; dir <= 6; dir++) {
        if(dir_transform(player_n, dir) == clicked_n) {
            clicked_nearby = clicked_n;
        }
    }

    if(clicked_nearby == -1)
        return;
    else {
        if(g.map->tile_info[g.map->tiles[clicked_nearby].info_index].blocks_movement == false) {
            player->move(clicked_nearby);
            end_turn();
            player->update_visibility();
        }
    }

    // char buf[100];
    // snprintf(buf, sizeof(buf), "clicked on n=%d %d %d %f %f %f %f",
    //          clicked_nearby, g.mouse_x, g.mouse_y, pos.x1, pos.y1, pos.x2, pos.y2);
    // g.AddMessage(buf);
}

bool tile_blocks_los(int n) {
    return g.map->tile_info[g.map->tiles[n].info_index].blocks_los;
}

constexpr int lines2[12][2] =
    { { 1, 1 },
      { 2, 1 },
      { 2, 2 },
      { 2, 3 },
      { 3, 3 },
      { 3, 4 },
      { 4, 4 },
      { 4, 5 },
      { 5, 5 },
      { 5, 6 },
      { 6, 6 },
      { 6, 1 } };
constexpr int lines3[18][3] =
    { { 1, 1, 1 },
      { 1, 2, 1 },
      { 2, 1, 2 },
      { 2, 2, 2 },
      { 2, 3, 2 },
      { 3, 2, 3 },
      { 3, 3, 3 },
      { 3, 4, 3 },
      { 4, 3, 4 },
      { 4, 4, 4 },
      { 4, 5, 4 },
      { 4, 5, 5 },
      { 5, 5, 5 },
      { 5, 6, 5 },
      { 5, 6, 6 },
      { 6, 6, 6 },
      { 6, 1, 6 },
      { 1, 6, 1 } };

// updates which tiles the player is currently seeing and marks
// those tiles as known to the player.
void Character::update_visibility(void) {
    currently_seeing.clear();
    currently_seeing.push_back(n);
    g.map->tiles[n].visible = true;

    // manual raycasting
    switch(current_los_distance) {
    case 0:
        return;
    case 1:
        for(int dir = 1; dir <= 6; dir++) {
            int n2 = dir_transform(n, dir);
            if(good_index(n2) == false)
                continue;
            currently_seeing.push_back(n2);
            g.map->tiles[n2].visible = true;
        }
        return;
    case 2:
        for(int cur_line = 0; cur_line <= 11; cur_line++) {
            int n2 = dir_transform(n, lines2[cur_line][0]);
            if(good_index(n2) == false)
                continue;
            currently_seeing.push_back(n2);
            g.map->tiles[n2].visible = true;
            if(tile_blocks_los(n2))
                continue;
            int n3 = dir_transform(n2, lines2[cur_line][1]);
            if(good_index(n3) == false)
                continue;
            currently_seeing.push_back(n3);
            g.map->tiles[n3].visible = true;
        }
        return;
    default:
    case 3:
        for(int cur_line = 0; cur_line <= 17; cur_line++) {
            int n2 = dir_transform(n, lines3[cur_line][0]);
            if(good_index(n2) == false)
                continue;
            currently_seeing.push_back(n2);
            g.map->tiles[n2].visible = true;
            if(tile_blocks_los(n2))
                continue;
            int n3 = dir_transform(n2, lines3[cur_line][1]);
            if(good_index(n3) == false)
                continue;
            currently_seeing.push_back(n3);
            g.map->tiles[n3].visible = true;
            if(tile_blocks_los(n3))
                continue;
            int n4 = dir_transform(n3, lines3[cur_line][2]);
            if(good_index(n4) == false)
                continue;
            currently_seeing.push_back(n4);
            g.map->tiles[n4].visible = true;
        }
        cout << "added " << (int)currently_seeing.size() << " tiles" << endl;
        return;
    }
}

/*
 dir_transform(n, dir) transforms the tilemap position offset n
 in the direction given by dir. It returns -1 if the new position
 wraps around left or right

 dir:
 6 1 2
  \|/
   0
  /|\
 5 4 3
 */
int dir_transform(int n, int dir) {
    if(n % 2 == 0) {
        // even
        switch(dir) {
        case 0:
            return n;
        case 1:
            return n - g.map->size_x;
        case 2: {
            int mx = g.map->size_x;
            bool left_end = (n + 1) % mx == 0;
            if(left_end)
                return -1;
            else
                return n - mx + 1;
        }
        case 3: {
            int left_end = (n + 1) % g.map->size_x == 0;
            if(left_end)
                return -1;
            else
                return n + 1;
        }
        case 4:
            return n + g.map->size_x;
        case 5: {
            bool right_end = (n + 1) % g.map->size_x == 1;
            if(right_end)
                return -1;
            else
                return n - 1;
        }
        case 6: {
            int mx = g.map->size_x;
            bool right_end = (n + 1) % mx == 1;
            if(right_end)
                return -1;
            else
                return n - mx - 1;
        }
        }
    } else {
        // odd
        switch(dir) {
        case 0:
            return n;
        case 1:
            return n - g.map->size_x;
        case 2: {
            bool left_end = (n + 1) % g.map->size_x == 0;
            if(left_end)
                return -1;
            else
                return n + 1;
        }
        case 3: {
            int mx = g.map->size_x;
            bool left_end = (n + 1) % mx == 0;
            if(left_end)
                return -1;
            else
                return n + mx + 1;
        }
        case 4:
            return n + g.map->size_x;
        case 5: {
            int mx = g.map->size_x;
            bool right_end = (n + 1) % mx == 1;
            if(right_end)
                return -1;
            else
                return n + mx - 1;
        }
        case 6: {
            bool right_end = (n + 1) % g.map->size_x == 1;
            if(right_end)
                return -1;
            else
                return n - 1;
        }
        }
    }
    return n;
}

void Character::draw(void) {
    drawOffset(0, 0);
}

void Character::drawOffset(int offset_x, int offset_y) {
    assert(sprite != NULL);

    int ch_x = n % g.map->size_x;
    int ch_y = n / g.map->size_x;

    // is it on the screen?
    if(ch_y >= g.map->view_y + g.map->rows ||
       ch_x >= g.map->view_x + g.map->cols ||
       ch_y < g.map->view_y ||
       ch_x < g.map->view_x) {
        return;
    }

    int r_x = ch_x - g.map->view_x;
    int r_y = ch_y - g.map->view_y;

    int off_x = 80 * r_x;
    int off_y = n % 2  == 0 ? 0 : 20;
    off_y += 40 * r_y;

    al_draw_bitmap(sprite,
                   g.map->r_off_x + off_x + 25 + offset_x,
                   g.map->r_off_y + off_y + offset_y, 0);

    if(ch_y + 1 >= g.map->size_y)
        return;

    if(DEBUG_VISIBILITY ||
       g.map->tiles[g.map->size_x * (ch_y + 1) + ch_x].visible == true) {
        g.map->drawTopHalfOfTileAt(ch_x, ch_y + 1);
    }
}

void TileMap::handleKeyDown(void) {
    if(g.key == ALLEGRO_KEY_UP)
        if(view_y > 0)
            view_y--;
    if(g.key == ALLEGRO_KEY_LEFT)
        if(view_x > 0)
            view_x--;
    if(g.key == ALLEGRO_KEY_DOWN)
        if(view_y < size_y - rows)
            view_y++;
    if(g.key == ALLEGRO_KEY_RIGHT)
        if(view_x < size_x - cols)
            view_x++;
    if(g.key == ALLEGRO_KEY_C)
        g.map->focusOnPlayer();
    if(g.key == ALLEGRO_KEY_M) {
        if(g.log->visible)
            g.log->visible = false;
        else
            g.log->visible = true;
    }
    if(g.key == ALLEGRO_KEY_SPACE)
        end_turn();

    char buf[35];
    snprintf(buf, sizeof(buf),
             "Tile map view: x = %d, y = %d", view_x, view_y);
    g.AddMessage(buf);
}

struct UI {
    vector<Widget *> widgets;

    virtual ~UI() {
        info("~UI()");
        for(auto& widget : widgets) {
            delete widget;
            widget = NULL;
        }
    }

    static void switch_to(UI *to);

    void mouseDownEvent(void);
    void mouseUpEvent(void);
    void keyDownEvent(void);
    void hoverOverEvent(void);

    void update(void);
    virtual void draw(void);

    void toggleMessageLog(void);
};

void UI::update(void) {
    for(auto& widget : widgets)
        widget->update();
}

void UI::mouseDownEvent(void) {
    for(auto& widget : widgets) {
        assert(widget);
        if(widget->pos.x1 <= g.mouse_x &&
           widget->pos.y1 <= g.mouse_y &&
           widget->pos.x1 + widget->pos.x2 >= g.mouse_x &&
           widget->pos.y1 + widget->pos.y2 >= g.mouse_y) {
            widget->mouseDown();
            widget->onMouseDown.emit();
            // break;
        }
    }
}

void UI::mouseUpEvent(void) {
    for(auto& widget : widgets) {
        assert(widget);
        if(widget->pos.x1 <= g.mouse_x &&
           widget->pos.y1 <= g.mouse_y &&
           widget->pos.x1 + widget->pos.x2 >= g.mouse_x &&
           widget->pos.y1 + widget->pos.y2 >= g.mouse_y) {
            widget->mouseUp();
            widget->onMouseUp.emit();
            // break;
        }
    }
}

void UI::keyDownEvent(void) {
    for(auto& widget : widgets) {
        assert(widget);
        if(widget->pos.x1 <= g.mouse_x &&
           widget->pos.y1 <= g.mouse_y &&
           widget->pos.x1 + widget->pos.x2 >= g.mouse_x &&
           widget->pos.y1 + widget->pos.y2 >= g.mouse_y) {
            widget->keyDown();
            widget->onKeyDown.emit();
            // break;
        }
    }
}

void UI::hoverOverEvent(void) {
    for(auto& widget : widgets) {
        assert(widget);
        if(widget->pos.x1 <= g.mouse_x &&
           widget->pos.y1 <= g.mouse_y &&
           widget->pos.x1 + widget->pos.x2 >= g.mouse_x &&
           widget->pos.y1 + widget->pos.y2 >= g.mouse_y) {
            widget->hoverOver();
            // break;
        }
    }
}

void UI::draw(void) {
    for(auto& widget : widgets) {
        if(widget->visible)
            widget->draw();
    }
}

void Button::mouseUp(void) {

}

void Button::keyDown(void) {

}

void Button::hoverOver(void) {
    cout << "button!" << endl;
}

void Button::mouseDown(void) { press(); }

GridSortButton::GridSortButton(Grid *parent) {
    assert(parent);
    this->parent = parent;
    reset();
    this->up = g.bitmaps[28];
}

void GridSortButton::reset(void) {
    this->pos.x1 = parent->pos.x2;
    this->pos.y1 = parent->pos.y1;
    this->pos.x2 = this->pos.x1 + 20;
    this->pos.y2 = this->pos.y1 + 20;
}

void GridSortButton::mouseDown(void) {
    if(g.mouse_button == 1)
        parent->Sort(BiggerItemsFirst);
    else if(g.mouse_button == 2)
        parent->Sort(SmallerItemsFirst);
}

void GridSortButton::draw(void) {
    if(up != NULL)
        al_draw_bitmap(up, pos.x1, pos.y1, 0);
    else
        al_draw_filled_rectangle(pos.x1, pos.y1, pos.x2, pos.y2, g.color_black);
}

void Game::AddMessage(string str) {
    if(log != NULL)
        log->lines.push_back(str);
}

void Button::press(void) {
    //    pressed = !pressed;
    //    g.AddMessage("pressed button");
}

Button::~Button(void) {
    info("~Button()");
}

MessageLog::~MessageLog(void) {
    info("~MessageLog()");
}

void MessageLog::draw(void) {
    if(background != NULL) {
        al_draw_bitmap(background, pos.x1, pos.y1, 0);
    }

    int off_y = 8;
    int lines_n = lines.size();
    int start = max(0, lines_n - 16);
    for(int i = start; i < lines_n; i++) {
        al_draw_text(font, g.color_grey3, pos.x1 + 8, pos.y1 + off_y, 0, lines[i].c_str());
        off_y = off_y + 8;
    }
}

void Button::draw(void) {
    if(pressed == true && down != NULL)
        al_draw_bitmap(down, pos.x1, pos.y1, 0);
    else if(up != NULL)
        al_draw_bitmap(up, pos.x1, pos.y1, 0);
    else
        al_draw_filled_rectangle(pos.x1, pos.y1,
                                 pos.x1 + pos.x2,
                                 pos.y1 + pos.y2,
                                 g.color_black);
}

void Button::update() {
}

void mkRingM(int n, int m) {
    for(int i = 1; i <= 6; i++) {
        for(int j = 0; j < m; j++) {
            g.map->tiles[n].info_index = 2;
            n = dir_transform(n, i);
        }
    }
}

void TileMap::generate(void) {
    tiles.reserve(size_x * size_y);
    int up_to = bitmaps.size();
    for(int i = 0; i <= max_t; i++) {
        tiles[i].visible = false;
        tiles[i].info_index = rand() % up_to;
        tiles[i].ground_items = NULL;
        tiles[i].locations = NULL;
    }
    mkRingM(4 * size_x + 5, 1);
    mkRingM(5 * size_x + 3, 3);
    info("Finished generating map");
}

bool TileMap::playerSees(int n) {
    for(auto& cs : player->currently_seeing) {
        if(n == cs) {
            return true;
        }
    }
    return false;
}

void TileMap::drawTile(int i, int x, int y) {
    int t = start + (size_x * y) + x;

    if(DEBUG_VISIBILITY || tiles[t].visible == true) {
        // can the player currently see the tile?
        bool currently_seeing = playerSees(t);

        int off_x = (i % cols) * 80;
        int off_y = (i + view_x) % 2  == 0 ? 0 : 20;
        off_y = off_y + (40 * floor(i / cols));

        if(currently_seeing == true) {
            // draw the tile at full brightness
            al_draw_bitmap(bitmaps[tile_info[tiles[t].info_index].bitmap_index],
                           r_off_x + off_x,
                           r_off_y + off_y,
                           0);
        }
        else {
            /*
              TODO: this causes allocations? so it might be better
              generate and store tinted bitmaps
            */
            // otherwise draw it 50% tinted
            al_draw_tinted_bitmap(bitmaps[tile_info[tiles[t].info_index].bitmap_index],
                                  g.color_tile_tint,
                                  r_off_x + off_x,
                                  r_off_y + off_y, 0);
        }
        if(t == mouse_n) {
            // brighten tile if the mouse is on it
            al_draw_tinted_bitmap(bitmaps[tile_info[tiles[t].info_index].bitmap_index],
                                  g.color_active_tile_tint,
                                  r_off_x + off_x,
                                  r_off_y + off_y, 0);
        }
    }
}

void TileMap::draw(void) {
    start = size_x * view_y + view_x;
    mouse_n = mouseToTileN();

    // draw the hex grid
    int i = 0;
    for(int y = 0; y < rows; y++) {
        for(int x = 0; x < cols; x++) {
            drawTile(i, x, y);
            i++;
        }
    }

    // characters are drawn as part of the map
    int j = 0;
    if(DEBUG_VISIBILITY == true) {
        player->draw();
        for(auto& character : characters)
            character->draw();
    } else {
        for(auto& see_n : player->currently_seeing) {
            int num_there = charsByPos.count(see_n);
            if(num_there > 0) {
                auto pos = charsByPos.equal_range(see_n);
                int off_x = 0;
                int off_y = 0;
                if(num_there > 1) {
                    off_x = 5 * num_there;
                    off_y = -2.5 * num_there;
                }
                for(auto& it = pos.first; it != pos.second; it++) {
                    it->second->drawOffset(off_x, off_y);
                    j++;
                    off_x -= 20;
                    off_y += 10;
                }
            }
        }
        // cout << "drew " << j << endl;
    }

    if(DEBUG_VISIBILITY == true) {
        al_draw_rectangle(pos.x1, pos.y1, pos.x2, pos.y2, g.color_grey2, 1);
        al_draw_rectangle(pos.x1, pos.y1 + 40, pos.x2 - 81, pos.y2 - 1, g.color_white, 1);
    }
}

// redraws the top half of a tile (the part that overlaps with the
// tile above it). Used for occluding character sprites on the map
void TileMap::drawTopHalfOfTileAt(int x, int y) {
    if(y >= g.map->size_y - 1)
        return;

    int n = size_x * y + x;

    int r_x = x - g.map->view_x;
    int r_y = y - g.map->view_y;

    int off_x = 80 * r_x;
    int off_y = n % 2  == 0 ? 0 : 20;
    off_y += 40 * r_y;

    if(mouse_n == n) {
        al_draw_tinted_bitmap(bitmaps[tile_info[tiles[n].info_index].bitmap_index],
                                     g.color_active_tile_tint,
                                     r_off_x + off_x,
                                     r_off_y + off_y, 0);
    } else {
        al_draw_bitmap_region(bitmaps[tile_info[tiles[n].info_index].bitmap_index],
                              0, 0, 100, 40,
                              r_off_x + off_x,
                              r_off_y + off_y, 0);

    }
    // check if there's a character on that tile
    // Character *interloper = characterAt(n);
    // if(interloper != NULL)
    //     interloper->draw();
}

void GridSystem::drawItemTooltip(void) {
    // draw item tooltips
    // this should probably work some other way
    for(auto& grid : grids) {
        for(auto& item : grid->items) {
            if(item->parent->pos.x1 + item->pos.x1 * Grid::grid_px_x <= g.mouse_x &&
               item->parent->pos.y1 + item->pos.y1 * Grid::grid_px_y <= g.mouse_y &&
               item->parent->pos.x1 + (item->pos.x1 + item->pos.x2) * Grid::grid_px_x >= g.mouse_x &&
               item->parent->pos.y1 + (item->pos.y1 + item->pos.y2) * Grid::grid_px_y >= g.mouse_y) {

                al_draw_filled_rectangle(g.mouse_x + 16, g.mouse_y,
                                         g.mouse_x + 150, g.mouse_y + 48, g.color_black);
                al_draw_text(g.font, g.color_grey3, g.mouse_x + 24, g.mouse_y + 8,
                             0, g.item_info[item->info_index].name.c_str());
                int weight = g.item_info[item->info_index].weight;
                if(weight > 0) {
                    al_draw_textf(g.font, g.color_grey3, g.mouse_x + 24, g.mouse_y + 24,
                                  0, "%d g", weight * item->cur_stack);
                }

                // if the item has a grid, draw it under the text
                if(item->storage != NULL &&
                   held == NULL &&
                   item->parent->hpinfo == NULL)
                    // ^^ unless it's on a hardpoint
                    item->storage->drawAt(g.mouse_x + 16, g.mouse_y + 48);

                return;
            }
        }
    }
}

void GridSystem::draw(void) {
    for (auto& g : grids) {
        g->draw();
    }

    if(held != NULL)
        held->draw();
    else {
        drawItemTooltip();
    }
}

void GridSystem::gsMouseDownEvent() {
    if(g.mouse_button == 1) {
        GrabItem();
    }
    else if(g.mouse_button == 2) {
        if(auto_move_to_ground == true) {
            MouseAutoMoveItemToGround();
        } else {
            if(auto_target != NULL) {
                MouseAutoMoveItemToTarget();
            }
        }
    } else if(g.mouse_button == 4) {
        for(auto& grid : grids) {
            bool skip = false;
            for(auto& forbidden : interaction_forbidden) {
                if(grid == forbidden) {
                    // we can't grab items out of forbidden grids
                    skip = true;
                }
            }
            if(skip == false)
                grid->unstack_item(g.mouse_x, g.mouse_y);
        }
    }

    // check if we're clicking the sort buttons
    for(auto& grid : grids) {
        if(grid->gsb == NULL)
            continue;
        if(g.mouse_x > grid->gsb->pos.x1 && g.mouse_y > grid->gsb->pos.y1 &&
           g.mouse_x < grid->gsb->pos.x2 && g.mouse_y < grid->gsb->pos.y2) {
            if(grid->gsb_displayed == true) {
                grid->gsb->mouseDown();
                break;
            }
        }
    }
}

void GridSystem::GrabItem() {
    // does any grid contain an item at that position?
    Item *i = NULL;
    for(auto& grid : grids) {
        bool skip = false;
        for(auto& forbidden : interaction_forbidden) {
            if(grid == forbidden) {
                // we can't grab items out of forbidden grids
                skip = true;
            }
        }
        if(skip == false) {
            i = grid->grab_item(g.mouse_x, g.mouse_y);
            if(i != NULL)
                goto got_it;
        }
    }

    // no
    return;

 got_it:
    held = i;
    g.hold_off_x =
        g.mouse_x - (i->parent->pos.x1 + i->pos.x1 * Grid::grid_px_x);
    g.hold_off_y =
        g.mouse_y - (i->parent->pos.y1 + i->pos.y1 * Grid::grid_px_y);
    held->old_parent = held->parent;
    held->parent = NULL;
    change.emit();

    if(held->storage != NULL) {
        // if this item was on a hardpoint, we need to remove
        // the grid from the gridsystem
        int i = 0;
        bool found = false;
        for(auto& grid : grids) {
            if(grid == held->storage) {
                found = true;
                break;
            }
            i++;
        }
        if(found)
            grids.erase(grids.begin() + i);

        held->storage->gsb_displayed = false;
    }
}

// adds the storage grid of the held item (if there is one)
// to the grid system
void GridSystem::addStorageGrid(void) {
    if(held != NULL &&
       held->storage != NULL &&
       held->parent->hpinfo != NULL) {
        // ^^ we only want to add it if it's on a hardpoint
        held->storage->gsb_displayed = true;
        held->resetHardpointPos();
        grids.push_back(held->storage);
    }
}

/*
  TODO: this function is too big
*/
void GridSystem::gsMouseUpEvent() {
    // are we holding an item?
    if(held == NULL)
        return;

    // proposed position to drop it into relative to the
    // grid currently being examined
    int drop_x = 0;
    int drop_y = 0;

    Item *merge_with = NULL;
    Grid *merge_grid = NULL;
    // find a grid to drop it on
    int i = 0;
    for(auto& grid : grids) {
        // the bounds check depends if the grid is a real grid
        // or a hardpoint
        bool in_bounds = false;
        if(grid->hpinfo == NULL) {
            // grid
            drop_x = ((g.mouse_x - g.hold_off_x) - grid->pos.x1) / Grid::grid_px_x;
            drop_y = ((g.mouse_y - g.hold_off_y) - grid->pos.y1) / Grid::grid_px_y;
            in_bounds =
                g.mouse_x - g.hold_off_x >= grid->pos.x1 &&
                g.mouse_y - g.hold_off_y >= grid->pos.y1 &&
                g.mouse_x - g.hold_off_x + Grid::grid_px_x * held->pos.x2
                <= grid->pos.x2 + 8 &&
                g.mouse_y - g.hold_off_y + Grid::grid_px_y * held->pos.y2
                <= grid->pos.y2 + 8;
        }
        else {
            // hardpoint
            in_bounds =
                g.mouse_x >= grid->pos.x1 &&
                g.mouse_y >= grid->pos.y1 &&
                g.mouse_x <= grid->pos.x2 + 17 &&
                g.mouse_y <= grid->pos.y2 + 17;
        }

        if(in_bounds == true) {
            // we can't drop stuff on some grids
            for(auto& forbidden : interaction_forbidden) {
                if(grid == forbidden) {
                    goto blocked;
                }
            }

            // is this a real grid?
            if(grid->hpinfo == NULL) {
                // bounds check items in grid
                int blocks = 0;
                for(auto& item : grid->items) {
                    if(rectIntersect(item->pos.x1 * Grid::grid_px_x,
                                     item->pos.y1 * Grid::grid_px_y,
                                     item->pos.x2 * Grid::grid_px_x,
                                     item->pos.y2 * Grid::grid_px_y,
                                     drop_x * Grid::grid_px_x,
                                     drop_y * Grid::grid_px_y,
                                     held->pos.x2 * Grid::grid_px_x,
                                     held->pos.y2 * Grid::grid_px_y)) {
                        blocks++;
                        if(item->storage != NULL) {
                            // we're blocked by an item but it has storage
                            // try to place the held item into it
                            Item *ret = item->storage->PlaceItem(held);
                            if(ret == NULL) {
                                held = NULL;
                                return;
                            } else {
                                // We couldn't place it in storage.
                                // dump it on the ground instead
                                vector<Grid *> *ground = ground_at_player();
                                PlaceItemOnMultiGrid(ground, ret);
                                held = NULL;
                                return;
                            }
                        }
                        // there's an item there already:
                        // abort
                        if(item->info_index != held->info_index)
                            goto blocked;
                        /*
                          TODO: inspect this logic here
                        */
                        if(item->pos.x1 == drop_x &&
                           item->pos.y1 == drop_y) {
                            merge_with = item;
                            merge_grid = grid;
                            goto stack_it;
                        }
                    }
                }
                if(blocks > 0)
                    goto blocked;
            }
            // or a hardpoint?
            else if(grid->hpinfo->maxItems < (int)grid->items.size() + held->cur_stack) {
                // too many items on the hardpoint
                goto blocked;
            }

            // is this item compatible with the grid?
            if(grid->item_compatible(held) == false)
               goto blocked;

            // if we put something on a body part, we need to emit a signal
            // so that the logic can take place
            if(grid->hpinfo != NULL &&
               grid->hpinfo->medical == true) {

                applied_params = make_pair(grid, held);
                applied.emit();

                // this is for putting water/whiskey on a wound. A charge is
                // consumed and we emit a signal that an item was applied.
                if(held->isConsumedOnApplication() == true) {
                    if(held->cur_stack == 1) {
                        delete held;
                        held = NULL;
                        return;
                    } else {
                        held->cur_stack--;
                        goto blocked;
                    }
                }
            }

            // we've found the grid and there's nothing blocking
            // the placement there, so drop it
            held->parent = grid;
            held->old_parent = NULL;
            held->pos.x1 = drop_x;
            held->pos.y1 = drop_y;
            grid->items.push_back(held);
            change.emit();
            addStorageGrid();

            char b[60];
            snprintf(b, sizeof(b), "Moved %s onto grid %d",
                     g.item_info[held->info_index].name.c_str(), i);
            g.AddMessage(b);
            // the item is placed. we're done
            held = NULL;
            return;
        }
        i++;
    }

 blocked:
    // couldn't place it anywhere. send it back to where
    // it was before we picked it up
    held->parent = held->old_parent;
    held->old_parent->items.push_back(held);

    addStorageGrid();

    held = NULL;

    char b[40];
    snprintf(b, sizeof(b), "Blocked on grid %d", i);
    g.AddMessage(b);
    return;

 stack_it:
    held->parent = merge_grid;
    held->pos.x1 = drop_x;
    held->pos.y1 = drop_y;

    assert(merge_with != NULL);

    // check if we can merge all of it into the item
    if(merge_with->cur_stack + held->cur_stack <= g.item_info[held->info_index].maxStack) {
        merge_with->cur_stack += held->cur_stack;
        // if so, delete the item
        delete held;
        held = NULL;
        return;
    } else {
        // otherwise we still have some number of them to place
        int moved = g.item_info[held->info_index].maxStack - merge_with->cur_stack;
        merge_with->cur_stack += moved;
        held->cur_stack -= moved;
        // place the rest

        Item *returned = merge_grid->PlaceItem(held);
        // if we can't place the rest, sent it to the ground
        if(returned != NULL) {
            vector<Grid *> *ground = ground_at_player();
            PlaceItemOnMultiGrid(ground, returned);
        }
        held = NULL;
    }
    change.emit();
    return;
}

void Grid::draw(void) {
    al_draw_filled_rectangle(pos.x1, pos.y1, pos.x2, pos.y2, g.color_grey2);

    if(hpinfo == NULL) {
        for (int x = pos.x1 + grid_px_x; x < pos.x2; x = x + grid_px_x) {
            al_draw_line(x, pos.y1, x, pos.y2, g.color_grey3, 1);
        }
        for (int y = pos.y1 + grid_px_y; y < pos.y2; y = y + grid_px_y) {
            al_draw_line(pos.x1, y, pos.x2, y, g.color_grey3, 1);
        }
    }

    if(gsb_displayed == true)
        gsb->draw();

    for (auto& i : items)
        i->draw();
}

void Grid::drawAt(float x, float y) {
    Rect old = pos;
    pos.x1 = x;
    pos.y1 = y;
    pos.x2 = x + float(grid_size_x * grid_px_x);
    pos.y2 = y + float(grid_size_y * grid_px_y);
    draw();
    pos = old;
}

void Item::draw(void) {
    ALLEGRO_BITMAP *sprite = g.item_info[info_index].sprite;

    if(parent != NULL) {
        // we're on a grid
        int x1 = parent->pos.x1 + pos.x1 * Grid::grid_px_x;
        int y1 = parent->pos.y1 + pos.y1 * Grid::grid_px_y;
        int x2 = parent->pos.x1 + (pos.x1 + pos.x2) * Grid::grid_px_x;
        int y2 = parent->pos.y1 + (pos.y1 + pos.y2) * Grid::grid_px_y;

        if(sprite == NULL) {
            al_draw_filled_rectangle(x1, y1, x2, y2, g.color_grey3);
            al_draw_rectangle(x1, y1, x2, y2, g.color_black, 1);
        }
        else
            al_draw_bitmap(sprite, x1, y1, 0);

        if(cur_stack > 1) {
            // draw number of stacked items
            char buf[4];
            sprintf(buf, "%d", cur_stack);
            al_draw_text(g.font, g.color_black, x2 - 9, y2 - 9, 0, buf);
        }
    } else {
        // we're held by the mouse
        int x1 = g.mouse_x - g.hold_off_x;
        int y1 = g.mouse_y - g.hold_off_y;
        int x2 = g.mouse_x - g.hold_off_x + pos.x2 * 16;
        int y2 = g.mouse_y - g.hold_off_y + pos.y2 * 16;

        if(sprite == NULL) {
            al_draw_filled_rectangle(x1, y1, x2, y2, g.color_grey3);
            al_draw_rectangle(x1, y1, x2, y2, g.color_black, 1);
        }
        else
            al_draw_bitmap(sprite, x1, y1, 0);
    }
}
/*
  valgrind reports a loss here
*/
// this does a lot more than unstack now
void Grid::unstack_item(int x, int y) {
    int c = 0;
    for (auto& i : items) {
        if(i->parent->pos.x1 + i->pos.x1 * grid_px_x <= x &&
           i->parent->pos.y1 + i->pos.y1 * grid_px_y <= y &&
           i->parent->pos.x1 + (i->pos.x1 + i->pos.x2) * grid_px_x >= x &&
           i->parent->pos.y1 + (i->pos.y1 + i->pos.y2) * grid_px_y >= y) {
            if(i->cur_stack > 1) {
                vector<Grid *> *ground = ground_at_player();
                // if the item is stacked
                Item *other = new Item (*i);
                other->cur_stack = i->cur_stack / 2;
                i->cur_stack = i->cur_stack - other->cur_stack;
                PlaceItemWantsStacking = false;
                PlaceItemOnMultiGrid(ground, other);
                PlaceItemWantsStacking = true;
                return;
            }
            if(i->storage != NULL) {
                // the item has storage, empty it out (onto the ground)
                vector<Grid *> *ground = ground_at_player();
                while(i->storage->items.empty() == false) {
                    Item *moving = i->storage->items.front();
                    PlaceItemOnMultiGrid(ground, moving);
                    i->storage->RemoveItem(moving);
                }
                return;
            }
            if(i->isUsable()) {
                // we're using an item
                g.map->player->useItem(i);
            }
            return;
        }
        c++;
    }
}

Item *Grid::grab_item(int x, int y) {
    int c = 0;
    for (auto& i : items) {
        if(i->parent->pos.x1 + i->pos.x1 * grid_px_x <= x &&
           i->parent->pos.y1 + i->pos.y1 * grid_px_y <= y &&
           i->parent->pos.x1 + (i->pos.x1 + i->pos.x2) * grid_px_x >= x &&
           i->parent->pos.y1 + (i->pos.y1 + i->pos.y2) * grid_px_y >= y) {
            Item *a = items[c];
            items.erase(items.begin() + c);
            return a;
        }
        c++;
    }
    return NULL;
}

// find the next character that gets to move: it's the character
// with the lowest nextMove value.
Character *next(void) {
    if(g.map->characters.empty() == true)
        return g.map->player;

    Character *smallest = g.map->characters.front();
    for(auto& ch : g.map->characters) {
        if(ch->nextMove < smallest->nextMove) {
            smallest = ch;
        }
    }

    if(smallest->nextMove < g.map->player->nextMove)
        return smallest;
    else
        return g.map->player;
}

void end_turn() {
    Character *c;
    // process characters until it's the player's turn again
    while((c = next()) != g.map->player) {
        cout << c << ": " << c->nextMove << endl;
        c->randomMove();
    }

    cout << "turn ends with " << (int)g.map->characters.size() << " characters" << endl; g.AddMessage("Turn ends.");
}

struct CraftingGridSystem : public GridSystem {
    Grid *ingredients;
    Grid *results;

    int current_ground_page;

    CraftingGridSystem();
    ~CraftingGridSystem();

    void reset(void);
    void exit(void);
};

// clean up the crafting grids when we're leaving the UI
void CraftingGridSystem::exit(void) {
    // dump ingredients onto the ground
    while(ingredients->items.empty() == false) {
        Item *moving = ingredients->items.front();
        // don't move crafting skills
        if(g.item_info[moving->info_index].skill == false) {
            PlaceItemOnMultiGrid(ground_at_player(), moving);
        }
        ingredients->RemoveItem(moving);
    }
    // remove skills from the ground
    Grid *ground = ground_at_player()->at(current_ground_page);
    vector<Item *>::iterator it;
    for(it = ground->items.begin(); it != ground->items.end();) {
        if(g.item_info[(*it)->info_index].skill == true) {
            it = ground->items.erase(it);
        }
        else {
            ++it;
        }
    }
}

struct CraftingUI : public UI {
    CraftingGridSystem *craftGrids;
    Button *button_confirm;
    Button *button_next_recipe;
    Button *button_prev_recipe;
    int current_recipe;

    CraftingUI();
    ~CraftingUI();

    void draw(void) override;
    void setup(void);
};

void CraftingUI::draw(void) {
    al_draw_text(g.font, g.color_white, 200, 8, 0, "ground:");
    al_draw_text(g.font, g.color_white, 700, 8, 0, "ingredients:");
    al_draw_text(g.font, g.color_white, 800, 295, 0, "preview:");
    UI::draw();
}

// count the total number of item of type, taking into account stacking
int countItemsOfType(Grid* grid, int searching_for) {
    int c = 0;
    for(auto& item : grid->items) {
        if(item->info_index == searching_for) {
            c += item->cur_stack;
        }
    }
    cout << searching_for << " " << c << endl;
    return c;
}

struct Recipe {
    string name;
    int time_cost;
    // item info_index and amount
    vector<pair<int, int>> ingredients;
    // item info_index and amount
    vector<pair<int, int>> results;
    // tools are ingredients that aren't consumed
    // item info_index
    vector<int> tools;

    bool player_has_ingredients(Grid *on);
};

vector<Recipe *> recipes;

bool Recipe::player_has_ingredients(Grid *on) {
    for(auto& tool : tools) {
        bool there = false;
        for(auto& item : on->items) {
            if(item->info_index == tool) {
                there = true;
            }
        }
        if(there == false) {
            return false;
        }
    }
    for(auto& ingredient : ingredients) {
        if(countItemsOfType(on, ingredient.first) < ingredient.second) {
            return false;
        }
    }
    return true;
}

vector<Recipe *> find_craftable_recipe(Grid *ingredients) {
    vector<Recipe *> ret;

    for(auto& recipe : recipes) {
        if(recipe->player_has_ingredients(ingredients) == true)
            ret.push_back(recipe);
    }
    return ret;
}

void remove_amount_from_grid(Grid *ingredients, int index, int amount) {
    // remove items' stack size
    for(auto& item : ingredients->items) {
        if(item->info_index == index) {
            int removed = min(amount, item->cur_stack);
            item->cur_stack -= removed;
            amount -= removed;
            if(amount == 0) {
                break;
            }
        }
    }

    assert(amount == 0);

    // remove items that have 0 stack size
    vector<Item *>::iterator it;
    for(it = ingredients->items.begin(); it != ingredients->items.end();) {
        if((*it)->cur_stack <= 0) {
            delete *it;
            it = ingredients->items.erase(it);
        }
        else {
            ++it;
        }
    }
}

void create_results(Recipe *recipe) {
    // create and add crafted items to the ground
    for(auto& result : recipe->results) {
        int amount = result.second;
        while(amount > 0) {
            Item *crafted = new Item (result.first);
            PlaceItemOnMultiGrid(ground_at_player(), crafted);
            amount--;
        }
    }
    // remove ingredients
    for(auto& ingredient : recipe->ingredients) {
        remove_amount_from_grid(g.ui_Crafting->craftGrids->ingredients,
                                ingredient.first,
                                ingredient.second);
    }
}

void init_recipes(void) {
    Recipe *two_arrows_to_kit = new Recipe;
    two_arrows_to_kit->name = "First aid kit";
    two_arrows_to_kit->time_cost = 1234;
    two_arrows_to_kit->ingredients.push_back(make_pair(9, 2));
    two_arrows_to_kit->tools.push_back(3);
    two_arrows_to_kit->results.push_back(make_pair(2, 1));

    Recipe *arrow_to_bullet = new Recipe;
    arrow_to_bullet->name = "Bullet";
    arrow_to_bullet->time_cost = 1234;
    arrow_to_bullet->ingredients.push_back(make_pair(9, 1));
    arrow_to_bullet->tools.push_back(11);
    arrow_to_bullet->results.push_back(make_pair(8, 1));

    Recipe *shopping_trolley_to_3crowbars = new Recipe;
    shopping_trolley_to_3crowbars->name = "Crowbars";
    shopping_trolley_to_3crowbars->time_cost = 1234;
    shopping_trolley_to_3crowbars->ingredients.push_back(make_pair(4, 1));
    shopping_trolley_to_3crowbars->results.push_back(make_pair(3, 3));

    recipes.push_back(two_arrows_to_kit);
    recipes.push_back(arrow_to_bullet);
    recipes.push_back(shopping_trolley_to_3crowbars);
}

struct MainMapUI : public UI {
    MainMapUI();
    ~MainMapUI();
};

struct MiniMapUI : public UI {
    MiniMapUI();
    ~MiniMapUI();
};

CraftingGridSystem::CraftingGridSystem() {
    ingredients = new Grid(500, 25, 33, 12, NULL);
    results = new Grid(500, 310, 33, 12, NULL);
    current_ground_page = 0;
}

CraftingGridSystem::~CraftingGridSystem() {
    info("~CraftingGridSystem()");
    delete ingredients;
    delete results;
}

void runCrafting(void);

void updateCraftingOutput(void) {
    cout << "something changed! maybe" << endl;

    Grid *results = g.ui_Crafting->craftGrids->results;

    // delete the items on the output grid
    vector<Item *>::iterator it;
    for(it = results->items.begin(); it != results->items.end();) {
        // it would be more efficient to have a global store of all items and then
        // add and remove pointers rather than creating all these objects
        delete *it;
        it = results->items.erase(it);
    }

    if(g.ui_Crafting->craftGrids->ingredients->items.empty() == true)
        // no ingredients
        return;

    vector<Recipe *> rs = find_craftable_recipe(g.ui_Crafting->craftGrids->ingredients);

    // check next/prev recipe buttons bounds
    if(g.ui_Crafting->current_recipe > (int)rs.size() - 1)
        g.ui_Crafting->current_recipe = (int)rs.size() - 1;

    if(g.ui_Crafting->current_recipe < 0)
        g.ui_Crafting->current_recipe = 0;

    // no recipes
    if(rs.empty())
        return;

    // output of the recipe
    vector<pair<int, int>> recipe_output = rs.at(g.ui_Crafting->current_recipe)->results;

    // fill the live preview
    for(auto& result : recipe_output) {
        int amount = result.second;
        while(amount > 0) {
            Item *crafted = new Item (result.first);
            results->PlaceItem(crafted);
            amount--;
        }
    }
}

void craftingNextRecipe(void) {
    cout << g.ui_Crafting->current_recipe++ << endl;
    updateCraftingOutput();
}

void craftingPrevRecipe(void) {
    cout << g.ui_Crafting->current_recipe-- << endl;
    updateCraftingOutput();
}

CraftingUI::CraftingUI() {
    craftGrids = new CraftingGridSystem;
    craftGrids->change.connect(ptr_fun(updateCraftingOutput));

    button_prev_recipe = new Button;
    button_prev_recipe->pos.x1 = 500;
    button_prev_recipe->pos.y1 = 260;
    button_prev_recipe->pos.x2 = 75;
    button_prev_recipe->pos.y2 = 45;
    button_prev_recipe->up = g.bitmaps[33];
    button_prev_recipe->down = NULL;
    button_prev_recipe->onMouseDown.connect(ptr_fun(craftingPrevRecipe));

    button_confirm = new Button;
    button_confirm->pos.x1 = 580;
    button_confirm->pos.y1 = 260;
    button_confirm->pos.x2 = 75;
    button_confirm->pos.y2 = 45;
    button_confirm->up = g.bitmaps[33];
    button_confirm->down = NULL;
    button_confirm->onMouseDown.connect(ptr_fun(runCrafting));

    button_next_recipe = new Button;
    button_next_recipe->pos.x1 = 660;
    button_next_recipe->pos.y1 = 260;
    button_next_recipe->pos.x2 = 75;
    button_next_recipe->pos.y2 = 45;
    button_next_recipe->up = g.bitmaps[33];
    button_next_recipe->down = NULL;
    button_next_recipe->onMouseDown.connect(ptr_fun(craftingNextRecipe));

    current_recipe = 0;
    setup();
}

CraftingUI::~CraftingUI() {
}

void CraftingGridSystem::reset(void) {
    ingredients->items.clear();
    results->items.clear();
    interaction_forbidden.clear();

    grids.clear();
    grids.push_back(ingredients);
    grids.push_back(results);
    vector<Grid *> *ground = ground_at_player();
    (*ground)[current_ground_page]->gsb_displayed = true;

    // add relevant skills to the ground
    for(int i = 0; i < 64; i++) {
        if(g.map->player->hasSkill(i) &&
           isCraftingSkill(i)) {
            (*ground)[current_ground_page]->PlaceItem(g.skills[i]);
        }
    }

    grids.push_back((*ground)[current_ground_page]);
    interaction_forbidden.push_back(results);

    pos.x1 = 0;
    pos.y1 = 0;
    pos.x2 = 1280;
    pos.y2 = 720;

    countTotalItems();
    GridSystem::reset();
}

void CraftingUI::setup(void) {
    widgets.clear();
    widgets.push_back(button_prev_recipe);
    widgets.push_back(button_confirm);
    widgets.push_back(button_next_recipe);
    widgets.push_back(g.log);
    widgets.push_back(g.button_MainMap);
    widgets.push_back(g.button_MiniMap);
    widgets.push_back(g.button_Skills);
    widgets.push_back(g.button_Crafting);
    widgets.push_back(g.button_Items);
    widgets.push_back(g.button_Condition);
    widgets.push_back(g.button_Camp);
    widgets.push_back(g.button_Vehicle);
    widgets.push_back(g.button_scavenge);
    widgets.push_back(g.button_endturn);

    craftGrids->reset();

    widgets.push_back(craftGrids);
    craftGrids->exit();
}

void runCrafting(void) {
    Grid *in = g.ui_Crafting->craftGrids->ingredients;
    int selected_recipe = g.ui_Crafting->current_recipe;

    cout << "hello" << endl;

    vector<Recipe *> rs = find_craftable_recipe(in);
    if(!rs.empty()) {
        cout << "created something" << endl;
        create_results(rs.at(selected_recipe));
        g.AddMessage("Crafted " + rs.at(selected_recipe)->name);
    }

    updateCraftingOutput();
}

struct EncounterGridSystem : public GridSystem {
    Grid *options;
    Grid *selected;

    EncounterGridSystem();
    ~EncounterGridSystem();
};

struct EncounterUI : public UI {
    Character *player;
    Character *npc;
    EncounterGridSystem *encounterGrids;
    ALLEGRO_BITMAP *cur_tile_sprite;
    Button *button_confirm;
    Item *flee;
    Item *single_attack;

    EncounterUI();
    ~EncounterUI();

    void draw(void) override;

    void setup(void);
};

EncounterGridSystem::EncounterGridSystem() {
    options = new Grid (105, 300, 16, 10, NULL);
    selected = new Grid (410, 300, 16, 10, g.encounter_selected);

    grids.push_back(options);
    grids.push_back(selected);

    auto_target = selected;

    pos.x1 = 0;
    pos.y1 = 0;
    pos.x2 = 1280;
    pos.y2 = 720;
}

EncounterGridSystem::~EncounterGridSystem() {
    delete options;
    delete selected;
}

void runEncounterStep(void);

EncounterUI::EncounterUI() {
    encounterGrids = new EncounterGridSystem;
    flee = new Item ("Flee");
    single_attack = new Item ("Single attack");

    button_confirm = new Button;
    button_confirm->pos.x1 = 715;
    button_confirm->pos.y1 = 300;
    button_confirm->pos.x2 = 75;
    button_confirm->pos.y2 = 45;
    button_confirm->up = g.bitmaps[33];
    button_confirm->down = NULL;
    button_confirm->onMouseDown.connect(ptr_fun(runEncounterStep));
}

void button_MainMap_press(void);

void runEncounter(void) {
    g.ui_Encounter->setup();
    g.ui = g.ui_Encounter;
}

// runs one step of the encounter after the player pressed the
// confirm button. Could in theory accept multiple actions
void runEncounterStep(void) {
    Character *npc = g.ui_Encounter->npc;
    vector<Item *> *actions = &g.ui_Encounter->encounterGrids->selected->items;

    if(actions->empty()) {
        g.AddMessage("Ah! The old human nightmare: endless varieties of stupidity, endless varieties of suffering, endless varieties of banality.");
        return;
    }

    string action1 = g.item_info[actions->front()->info_index].name;

    if(action1 == "Flee") {

        g.map->player->randomMove();
        g.AddMessage("You successfully flee from the encounter.");
        g.AddMessage("Encounter ends.");

        button_MainMap_press();
    }
    else if(action1 == "Single attack") {

        g.AddMessage("KA-POW!");
        g.AddMessage("The adversary succumbs to their wounds.");
        g.AddMessage("Encounter ends.");
        g.map->removeCharacter(npc);
        g.map->updateCharsByPos();

        button_MainMap_press();
    } else {

    }
}

void EncounterUI::setup(void) {
    g.color_bg = g.color_grey;

    widgets.clear();
    widgets.push_back(button_confirm);
    widgets.push_back(g.log);

    encounterGrids->selected->items.clear();
    encounterGrids->options->items.clear();

    encounterGrids->options->PlaceItem(flee);
    encounterGrids->options->PlaceItem(single_attack);
    widgets.push_back(encounterGrids);

    // the player
    player = g.map->player;
    // the tile sprite
    cur_tile_sprite = g.map->bitmaps[g.map->tile_info[g.map->tiles[player->n].info_index].bitmap_index];

    // find the npc
    g.map->updateCharsByPos();
    npc = g.map->charsByPos.equal_range(player->n).first->second;

    cout << "running encounter at: " << player->n << ' ' << npc->n << endl;
}

struct ScavengeGridSystem : public GridSystem {
    Grid *options;
    Grid *selected;

    ScavengeGridSystem();
    ~ScavengeGridSystem();
};

struct ScavengeUI : public UI {
    Character *player;
    Tile *current_tile;
    ScavengeGridSystem *gridsystem;
    // ALLEGRO_BITMAP *cur_tile_sprite;
    Button *button_confirm;
    int current_stage;
    unordered_map<Item *, Location *> items_to_locations;
    Item *selected_location;
    vector<Item *> selected_tools;

    ScavengeUI();
    ~ScavengeUI();

    void draw(void) override { UI::draw(); };

    void setup(void);
    void reset(void);
};

void ScavengeUI::reset(void) {
    current_stage = 0;
    gridsystem->visible = true;
}

ScavengeGridSystem::ScavengeGridSystem() {
    options = new Grid (105, 380, 20, 10, NULL);
    selected = new Grid (480, 380, 20, 10, NULL);

    grids.push_back(options);
    grids.push_back(selected);

    auto_target = selected;

    pos.x1 = 0;
    pos.y1 = 0;
    pos.x2 = 1280;
    pos.y2 = 720;

    reset();
}

ScavengeGridSystem::~ScavengeGridSystem() {
    delete options;
    delete selected;
}

void runScavenging(void) {
    int *stage = &g.ui_Scavenge->current_stage;
    cout << *stage << endl;

    vector<Item *> *selected = &g.ui_Scavenge->gridsystem->selected->items;

    if(*stage == 0) {
        if(selected->empty()) {
            return;
        }
        // after the first stage, we picked a location
        g.ui_Scavenge->selected_location = selected->front();
    } else if(*stage == 1) {
        // after the second step, we picked the tools used for scavenging
        g.ui_Scavenge->selected_tools = *selected;
    }


    Item *location_item = g.ui_Scavenge->selected_location;
    Location *location =g.ui_Scavenge->items_to_locations.find(location_item)->second;
    assert(location != NULL);

    if(*stage == 0) {
        // show options for this tile
        (*stage)++;
        g.ui_Scavenge->setup();
    } else if(*stage == 1) {
        // take location and options, generate items scavenged,
        // and show results ("found something"/"found nothing")
        PlaceItemOnMultiGrid(ground_at_player(), new Item ("Crowbar"));
        location->last_looted = g.map->player->nextMove;
        (*stage)++;
        g.ui_Scavenge->setup();
    } else if(*stage == 2) {
        button_MainMap_press();
    }
}

ScavengeUI::ScavengeUI() {
    gridsystem = new ScavengeGridSystem;

    button_confirm = new Button;
    button_confirm->pos.x1 = 855;
    button_confirm->pos.y1 = 380;
    button_confirm->pos.x2 = 75;
    button_confirm->pos.y2 = 45;
    button_confirm->up = g.bitmaps[33];
    button_confirm->down = NULL;
    button_confirm->onMouseDown.connect(ptr_fun(runScavenging));
}

ScavengeUI::~ScavengeUI() { }

vector<Location *> *locations_at_character(Character *character);

void ScavengeUI::setup(void) {
    g.color_bg = g.color_grey;

    widgets.clear();
    widgets.push_back(button_confirm);
    widgets.push_back(g.log);

    gridsystem->selected->items.clear();
    gridsystem->options->items.clear();

    if(current_stage == 0) {
        items_to_locations.clear();
        player = g.map->player;
        vector<Location *> *locations = locations_at_character(player);
        for(auto& location : *locations) {
            if(location->last_looted == 0 ||
               location->last_looted + 10000 < player->nextMove) {
                Item *i = g.location_info[location->info_index].location_item;
                // we add the location items to the options grid
                gridsystem->options->PlaceItem(i);
                // we make a map from location items to locations
                // so runScavenging can know which location we're looting
                // based on the item selected
                items_to_locations.insert(pair<Item *, Location *>(i, location));
            }
        }
    } else if(current_stage == 1) {
        gridsystem->options->PlaceItem(new Item ("Crowbar"));
        // show options for looting
    } else if(current_stage == 2) {
        gridsystem->visible = false;
        // show what we got
    }
    widgets.push_back(g.log);
    widgets.push_back(g.button_MainMap);
    widgets.push_back(g.button_MiniMap);
    widgets.push_back(g.button_Skills);
    widgets.push_back(g.button_Crafting);
    widgets.push_back(g.button_Items);
    widgets.push_back(g.button_Condition);
    widgets.push_back(g.button_Camp);
    widgets.push_back(g.button_Vehicle);
    widgets.push_back(g.button_scavenge);
    widgets.push_back(g.button_endturn);
    widgets.push_back(gridsystem);
}

void EncounterUI::draw(void) {
    al_draw_filled_rectangle(105, 25, 405, 295, g.color_grey2);
    al_draw_filled_rectangle(410, 25, 680, 295, g.color_grey2);
    al_draw_filled_rectangle(685, 25, 985, 295, g.color_grey2);
    al_draw_text(g.font, g.color_white, 200, 10, 0,
                 "Zwei Männer, einander in höherer Stellung, vermutend, begegnen sich:");
    al_draw_bitmap(player->sprite, 120, 40, 0);
    al_draw_bitmap(npc->sprite, 700, 40, 0);
    al_draw_bitmap(cur_tile_sprite, 490, 200, 0);

    UI::draw();
}

EncounterUI::~EncounterUI() {
    delete flee;
    delete single_attack;
}

struct InventoryGridSystem;
struct VehicleGridSystem;
struct ConditionGridSystem;
struct CampGridSystem;

struct ItemsUI : public UI {
    InventoryGridSystem *gridsystem;
    Button *ground_next_page;
    Button *ground_prev_page;

    ItemsUI();
    ~ItemsUI();
};

struct CampUI : public UI {
    CampGridSystem *gridsystem;
    Button *ground_next_page;
    Button *ground_prev_page;

    CampUI();
    ~CampUI();
};

struct ConditionUI : public UI {
    ConditionGridSystem *gridsystem;
    Button *ground_next_page;
    Button *ground_prev_page;

    ConditionUI();
    ~ConditionUI();
};

struct VehicleUI : public UI {
    VehicleGridSystem *gridsystem;
    Button *ground_next_page;
    Button *ground_prev_page;

    VehicleUI();
    ~VehicleUI();
};

struct VehicleGridSystem : public GridSystem {
    int current_ground_page;

    VehicleGridSystem() {
        auto_move_to_ground = true;
        current_ground_page = 0;
        reset();
    };

    ~VehicleGridSystem() {
        info("~VehicleGridSystem()");
    };

    void reset(void);
    void draw(void) {
        al_draw_text(g.font, g.color_white, 200, 10, 0, "Ground:");
        al_draw_text(g.font, g.color_white, 500, 135, 0, "Vehicle:");
        GridSystem::draw();
    }

    void keyDown(void) override;
};

struct CampGridSystem : public GridSystem {
    int current_ground_page;
    Grid *current_campsite;
    Grid *available_campsites;

    CampGridSystem() {
        current_campsite = new Grid (520, 133, 20, 24, NULL);
        available_campsites = new Grid (950, 133, 10, 24, NULL);
        auto_move_to_ground = true;
        current_ground_page = 0;
        reset();
    };

    ~CampGridSystem() {
        info("~CampGridSystem()");
    };

    void reset(void);
    void draw(void) override {
        al_draw_text(g.font, g.color_white, 200, 10, 0, "Ground:");
        al_draw_text(g.font, g.color_white, 600, 10, 0, "Current campsite::");
        al_draw_text(g.font, g.color_white, 950, 10, 0, "Available campsites:");
        GridSystem::draw();
    }

    void keyDown(void) override;
};

void VehicleGridSystem::keyDown(void) {
    vector<Grid *> *ground = ground_at_player();
    if(g.key == ALLEGRO_KEY_M) {
        if(current_ground_page < (int)(*ground).size() - 1) {
            current_ground_page++;
            reset();
        }
    }
    if(g.key == ALLEGRO_KEY_N) {
        if(current_ground_page > 0) {
            current_ground_page--;
            reset();
        }
    }
}

void CampGridSystem::keyDown(void) {
    vector<Grid *> *ground = ground_at_player();
    if(g.key == ALLEGRO_KEY_M) {
        if(current_ground_page < (int)(*ground).size() - 1) {
            current_ground_page++;
            reset();
        }
    }
    if(g.key == ALLEGRO_KEY_N) {
        if(current_ground_page > 0) {
            current_ground_page--;
            reset();
        }
    }
}

struct InventoryGridSystem : public GridSystem {
    int current_ground_page;

    InventoryGridSystem();
    ~InventoryGridSystem() {
        info("~InventoryGridSystem()");
    };

    void reset(void);
    void draw(void) {
        al_draw_text(g.font, g.color_white, 200, 10, 0, "Ground:");

        GridSystem::draw();
    }

    void keyDown(void) override;
};

struct ConditionGridSystem : public GridSystem {
    int current_ground_page;

    ConditionGridSystem();
    ~ConditionGridSystem() {
        info("~ConditionGridSystem()");
    };

    void reset(void);
    void draw(void) {
        al_draw_text(g.font, g.color_white, 200, 10, 0, "Ground:");
        al_draw_filled_rectangle(1000, 50, 1175, 500, g.color_white);
        al_draw_text(g.font, g.color_black, 1008, 58, 0, "Current conditions:");
        GridSystem::draw();
    }

    void keyDown(void) override;
};

void appliedCB(void) {
    Grid *applied_to = g.ui_Condition->gridsystem->applied_params.first;
    Item *was_applied = g.ui_Condition->gridsystem->applied_params.second;

    char buf[50];
    sprintf(buf, "Applied %s to %p",
            was_applied->getName(),
            (void*)applied_to);
    g.AddMessage(buf);
    return;
}

InventoryGridSystem::InventoryGridSystem() {
    auto_move_to_ground = true;
    current_ground_page = 0;
    reset();
}

ConditionGridSystem::ConditionGridSystem() {
    auto_move_to_ground = true;
    current_ground_page = 0;
    applied.connect(ptr_fun(appliedCB));
    reset();
}

// holy duplication
void ConditionPrevGroundPage(void) {
    cout << "prev" << endl;
    if(g.ui_Condition->gridsystem->current_ground_page > 0) {
        g.ui_Condition->gridsystem->current_ground_page--;
        g.ui_Condition->gridsystem->reset();
    }
}

void ConditionNextGroundPage(void) {
    cout << "next" << endl;
    vector<Grid *> *ground = ground_at_player();
    if(g.ui_Condition->gridsystem->current_ground_page< (int)(*ground).size()-1) {
        g.ui_Condition->gridsystem->current_ground_page++;
        g.ui_Condition->gridsystem->reset();
    }
}

void CampPrevGroundPage(void) {
    cout << "prev" << endl;
    if(g.ui_Camp->gridsystem->current_ground_page > 0) {
        g.ui_Camp->gridsystem->current_ground_page--;
        g.ui_Camp->gridsystem->reset();
    }
}

void CampNextGroundPage(void) {
    cout << "next" << endl;
    vector<Grid *> *ground = ground_at_player();
    if(g.ui_Camp->gridsystem->current_ground_page< (int)(*ground).size()-1) {
        g.ui_Camp->gridsystem->current_ground_page++;
        g.ui_Camp->gridsystem->reset();
    }
}

void InventoryNextGroundPage(void) {
    cout << "next" << endl;
    vector<Grid *> *ground = ground_at_player();
    if(g.ui_Items->gridsystem->current_ground_page< (int)(*ground).size()-1) {
        g.ui_Items->gridsystem->current_ground_page++;
        g.ui_Items->gridsystem->reset();
    }
}

void InventoryPrevGroundPage(void) {
    cout << "prev" << endl;
    if(g.ui_Items->gridsystem->current_ground_page > 0) {
        g.ui_Items->gridsystem->current_ground_page--;
        g.ui_Items->gridsystem->reset();
    }
}

void VehicleNextGroundPage(void) {
    cout << "next" << endl;
    vector<Grid *> *ground = ground_at_player();
    if(g.ui_Vehicle->gridsystem->current_ground_page< (int)(*ground).size()-1) {
        g.ui_Vehicle->gridsystem->current_ground_page++;
        g.ui_Vehicle->gridsystem->reset();
    }
}

void VehiclePrevGroundPage(void) {
    cout << "prev" << endl;
    if(g.ui_Vehicle->gridsystem->current_ground_page > 0) {
        g.ui_Vehicle->gridsystem->current_ground_page--;
        g.ui_Vehicle->gridsystem->reset();
    }
}


void InventoryGridSystem::keyDown(void) {
    if(g.key == ALLEGRO_KEY_M) {
        InventoryNextGroundPage();
    }
    if(g.key == ALLEGRO_KEY_N) {
        InventoryPrevGroundPage();
    }
}

void ConditionGridSystem::keyDown(void) {
    if(g.key == ALLEGRO_KEY_M) {
        InventoryNextGroundPage();
    }
    if(g.key == ALLEGRO_KEY_N) {
        InventoryPrevGroundPage();
    }
}

VehicleUI::VehicleUI() {
    gridsystem = new VehicleGridSystem;

    ground_next_page = new Button;
    ground_next_page->pos.x1 = 485;
    ground_next_page->pos.y1 = 50;
    ground_next_page->pos.x2 = 20;
    ground_next_page->pos.y2 = 20;
    ground_next_page->up = g.bitmaps[29];
    ground_next_page->down = NULL;
    ground_next_page->onMouseDown.connect(ptr_fun(VehicleNextGroundPage));

    ground_prev_page = new Button;
    ground_prev_page->pos.x1 = 465;
    ground_prev_page->pos.y1 = 50;
    ground_prev_page->pos.x2 = 20;
    ground_prev_page->pos.y2 = 20;
    ground_prev_page->up = g.bitmaps[30];
    ground_prev_page->down = NULL;
    ground_prev_page->onMouseDown.connect(ptr_fun(VehiclePrevGroundPage));

    widgets.push_back(gridsystem);
    widgets.push_back(g.log);
    widgets.push_back(g.button_MainMap);
    widgets.push_back(g.button_MiniMap);
    widgets.push_back(g.button_Skills);
    widgets.push_back(g.button_Crafting);
    widgets.push_back(g.button_Items);
    widgets.push_back(g.button_Condition);
    widgets.push_back(g.button_Camp);
    widgets.push_back(g.button_Vehicle);
    widgets.push_back(g.button_endturn);
    widgets.push_back(g.button_scavenge);
    widgets.push_back(ground_next_page);
    widgets.push_back(ground_prev_page);
}

CampUI::CampUI() {
    gridsystem = new CampGridSystem;

    ground_next_page = new Button;
    ground_next_page->pos.x1 = 485;
    ground_next_page->pos.y1 = 50;
    ground_next_page->pos.x2 = 20;
    ground_next_page->pos.y2 = 20;
    ground_next_page->up = g.bitmaps[29];
    ground_next_page->down = NULL;
    ground_next_page->onMouseDown.connect(ptr_fun(CampNextGroundPage));

    ground_prev_page = new Button;
    ground_prev_page->pos.x1 = 465;
    ground_prev_page->pos.y1 = 50;
    ground_prev_page->pos.x2 = 20;
    ground_prev_page->pos.y2 = 20;
    ground_prev_page->up = g.bitmaps[30];
    ground_prev_page->down = NULL;
    ground_prev_page->onMouseDown.connect(ptr_fun(CampPrevGroundPage));

    widgets.push_back(gridsystem);
    widgets.push_back(g.log);
    widgets.push_back(g.button_MainMap);
    widgets.push_back(g.button_MiniMap);
    widgets.push_back(g.button_Skills);
    widgets.push_back(g.button_Crafting);
    widgets.push_back(g.button_Items);
    widgets.push_back(g.button_Condition);
    widgets.push_back(g.button_Camp);
    widgets.push_back(g.button_Vehicle);
    widgets.push_back(g.button_endturn);
    widgets.push_back(g.button_scavenge);
    widgets.push_back(ground_next_page);
    widgets.push_back(ground_prev_page);
}

VehicleUI::~VehicleUI() {
    info("~VehicleUI()");
}

CampUI::~CampUI() {
    info("~CampUI()");
}

ItemsUI::ItemsUI() {
    gridsystem  = new InventoryGridSystem;

    ground_next_page = new Button;
    ground_next_page->pos.x1 = 485;
    ground_next_page->pos.y1 = 50;
    ground_next_page->pos.x2 = 20;
    ground_next_page->pos.y2 = 20;
    ground_next_page->up = g.bitmaps[29];
    ground_next_page->down = NULL;
    ground_next_page->onMouseDown.connect(ptr_fun(InventoryNextGroundPage));

    ground_prev_page = new Button;
    ground_prev_page->pos.x1 = 465;
    ground_prev_page->pos.y1 = 50;
    ground_prev_page->pos.x2 = 20;
    ground_prev_page->pos.y2 = 20;
    ground_prev_page->up = g.bitmaps[30];
    ground_prev_page->down = NULL;
    ground_prev_page->onMouseDown.connect(ptr_fun(InventoryPrevGroundPage));

    widgets.push_back(gridsystem);
    widgets.push_back(g.log);
    widgets.push_back(g.button_MainMap);
    widgets.push_back(g.button_MiniMap);
    widgets.push_back(g.button_Skills);
    widgets.push_back(g.button_Crafting);
    widgets.push_back(g.button_Items);
    widgets.push_back(g.button_Condition);
    widgets.push_back(g.button_Camp);
    widgets.push_back(g.button_Vehicle);
    widgets.push_back(g.button_endturn);
    widgets.push_back(g.button_scavenge);
    widgets.push_back(ground_next_page);
    widgets.push_back(ground_prev_page);
}

ConditionUI::ConditionUI() {
    gridsystem  = new ConditionGridSystem;

    ground_next_page = new Button;
    ground_next_page->pos.x1 = 485;
    ground_next_page->pos.y1 = 50;
    ground_next_page->pos.x2 = 20;
    ground_next_page->pos.y2 = 20;
    ground_next_page->up = g.bitmaps[29];
    ground_next_page->down = NULL;
    ground_next_page->onMouseDown.connect(ptr_fun(ConditionNextGroundPage));

    ground_prev_page = new Button;
    ground_prev_page->pos.x1 = 465;
    ground_prev_page->pos.y1 = 50;
    ground_prev_page->pos.x2 = 20;
    ground_prev_page->pos.y2 = 20;
    ground_prev_page->up = g.bitmaps[30];
    ground_prev_page->down = NULL;
    ground_prev_page->onMouseDown.connect(ptr_fun(ConditionPrevGroundPage));

    widgets.push_back(gridsystem);
    widgets.push_back(g.log);
    widgets.push_back(g.button_MainMap);
    widgets.push_back(g.button_MiniMap);
    widgets.push_back(g.button_Skills);
    widgets.push_back(g.button_Crafting);
    widgets.push_back(g.button_Items);
    widgets.push_back(g.button_Condition);
    widgets.push_back(g.button_Camp);
    widgets.push_back(g.button_Vehicle);
    widgets.push_back(g.button_endturn);
    widgets.push_back(g.button_scavenge);
    widgets.push_back(ground_next_page);
    widgets.push_back(ground_prev_page);
}

ItemsUI::~ItemsUI() {
    info("~ItemsUI()");
}

ConditionUI::~ConditionUI() {
    info("~ConditionUI()");
}

struct SkillsGridSystem : public GridSystem {
    Grid *good;
    Grid *bad;

    SkillsGridSystem() {
        // skills bitmaps are 6x2
        good = new Grid(400, 20, 4 * 6, 6 * 2, NULL);
        bad = new Grid(400, 300, 4 * 6, 6 * 2, NULL);
        reset();
    };

    ~SkillsGridSystem() {
        info("~SkillsGridSystem()");
    };

    void reset(void);
};

void init_skills(void) {
    Item *metabolism = new Item ("Metabolism");
    Item *quick = new Item ("Quick");
    Item *lockpicking = new Item ("Lockpicking");
    Item *myopia = new Item ("Myopia");

    // the first 32 are good, the last 32 are bad
    g.skills[0] = quick;
    g.skills[1] = lockpicking;
    g.skills[32] = metabolism;
    g.skills[33] = myopia;
}

void SkillsGridSystem::reset(void) {
    good->items.clear();
    bad->items.clear();

    for(int i = 0; i < 32; i++) {
        if(g.map->player->hasSkill(i)) {
            good->PlaceItem(g.skills[i]);
        }
    }

    for(int i = 32; i < 63; i++) {
        if(g.map->player->hasSkill(i)) {
            bad->PlaceItem(g.skills[i]);
        }
    }

    interaction_forbidden.clear();

    grids.clear();
    grids.push_back(good);
    grids.push_back(bad);
    interaction_forbidden.push_back(good);
    interaction_forbidden.push_back(bad);

    pos.x1 = 0;
    pos.y1 = 0;
    pos.x2 = 1280;
    pos.y2 = 720;

    countTotalItems();
    GridSystem::reset();
}

struct SkillsUI : public UI {
    SkillsGridSystem *skillsGrid;

    SkillsUI();
    ~SkillsUI();

    // void draw(void) override;

    void setup(void);
};

void SkillsUI::setup(void) {
    skillsGrid->reset();
}

SkillsUI::SkillsUI() {
    skillsGrid = new SkillsGridSystem;

    widgets.push_back(skillsGrid);
    widgets.push_back(g.log);
    widgets.push_back(g.button_MainMap);
    widgets.push_back(g.button_MiniMap);
    widgets.push_back(g.button_Skills);
    widgets.push_back(g.button_Crafting);
    widgets.push_back(g.button_Items);
    widgets.push_back(g.button_Condition);
    widgets.push_back(g.button_Camp);
    widgets.push_back(g.button_Vehicle);
    widgets.push_back(g.button_endturn);
    widgets.push_back(g.button_scavenge);
}

SkillsUI::~SkillsUI() {
    info("~SkillsUI()");
}

void PlaceItemOnMultiGrid(vector<Grid *> *multigrid, Item *item) {
    // try to place it on the existing pages
    for(auto& grid : *multigrid) {
        Item *returned = grid->PlaceItem(item);
        if(returned == NULL)
            // woooooo
            return;
    }
    // we couldn't place it on an existing page
    // make a new grid
    Grid *new_grid = new Grid  (105, 25, 20, 30, NULL);

    if(new_grid == NULL) {
        // if we can't allocate any more grids
        info("WARNING: Couldn't allocate grid");
        delete item;
        return;
    }

    // try to place it on the new grid
    Item *returned = new_grid->PlaceItem(item);
    // return it to the parent if it can't be placed
    if(returned != NULL) {
        assert(item->parent);
        item->parent->AddItem(returned);
        delete new_grid;
        info("WARNING: Item too big for empty ground");
    }
    // add a new grid to the multigrid
    (*multigrid).push_back(new_grid);
}

int tile_to_locations_index(const Tile t) {
    return 0;
}

// returns, or creates, locations at a tile
vector<Location *> *locations_at_character(Character *character) {
    // get locations at player position
    assert(character != NULL);
    assert(good_index(character->n) == true);
    vector<Location *> *locations = g.map->tiles[character->n].locations;

    if(locations == NULL) {
        locations = new vector<Location *>;
    }
    assert(locations);

    if((*locations).empty()) {
        // create first location if it doesn't exist
        Location *location1 = new Location;
        location1->info_index = 0;
        location1->last_looted = 0;
        Location *location2 = new Location;
        location2->info_index = 1;
        location2->last_looted = 0;

        locations->push_back(location1);
        locations->push_back(location2);
    }
    g.map->tiles[character->n].locations = locations;
    return locations;
}

/*
  TODO: valgrind reports a loss here
*/
vector<Grid *> *ground_at_character(Character *character) {
    // get ground inventory at player position
    assert(character != NULL);
    assert(good_index(character->n) == true);
    vector<Grid *> *ground = g.map->tiles[character->n].ground_items;

    if(ground == NULL) {
        ground = new vector<Grid *>;
    }
    assert(ground);

    if((*ground).empty()) {
        // create first grid if it doesn't exist
        Grid *ground_grid = new Grid (105, 25, 20, 30, NULL);
        assert(ground_grid != NULL);
        ground->push_back(ground_grid);
        // test items
        Item *crowbar = new Item("Crowbar");
        Item *shopping_trolley1 = new Item("Shopping trolley");
        Item *pill_bottle = new Item("Pill bottle");
        Item *arrows1 = new Item("Arrow", 4);
        Item *arrows2 = new Item("Arrow", 3);
        Item *whiskey = new Item("Whiskey", 10);
        Item *clean_rag = new Item("Clean rag", 30);
        ground_grid->PlaceItem(crowbar);
        ground_grid->PlaceItem(shopping_trolley1);
        ground_grid->PlaceItem(pill_bottle);
        ground_grid->PlaceItem(arrows1);
        ground_grid->PlaceItem(arrows2);
        ground_grid->PlaceItem(whiskey);
        ground_grid->PlaceItem(clean_rag);
    }
    g.map->tiles[character->n].ground_items = ground;
    return ground;
}

vector<Grid *> *ground_at_player(void) {
    return ground_at_character(g.map->player);
}

void VehicleGridSystem::reset(void) {
    grids.clear();
    grids.push_back(g.map->player->vehicle);

    g.map->player->addVehicleHardpoint(this);

    vector<Grid *> *ground = ground_at_player();
    (*ground)[current_ground_page]->gsb_displayed = true;
    grids.push_back((*ground)[current_ground_page]);
    // reparent();

    pos.x1 = 0;
    pos.y1 = 0;
    pos.x2 = 1280;
    pos.y2 = 720;

    countTotalItems();
    GridSystem::reset();
}

void CampGridSystem::reset(void) {
    grids.clear();
    grids.push_back(current_campsite);
    grids.push_back(available_campsites);

    vector<Grid *> *ground = ground_at_player();
    (*ground)[current_ground_page]->gsb_displayed = true;
    grids.push_back((*ground)[current_ground_page]);
    // reparent();

    pos.x1 = 0;
    pos.y1 = 0;
    pos.x2 = 1280;
    pos.y2 = 720;

    countTotalItems();
    GridSystem::reset();
}

void InventoryGridSystem::reset(void) {
    // add player inventory
    grids.clear();
    grids.push_back(g.map->player->right_hand_hold);
    grids.push_back(g.map->player->left_hand_hold);
    grids.push_back(g.map->player->right_hand);
    grids.push_back(g.map->player->left_hand);
    grids.push_back(g.map->player->back);
    grids.push_back(g.map->player->head);
    grids.push_back(g.map->player->neck);
    grids.push_back(g.map->player->right_shoulder);
    grids.push_back(g.map->player->left_shoulder);
    grids.push_back(g.map->player->torso);
    grids.push_back(g.map->player->legs);
    grids.push_back(g.map->player->right_foot);
    grids.push_back(g.map->player->left_foot);

    g.map->player->addInventoryHardpoints(this);

    vector<Grid *> *ground = ground_at_player();
    (*ground)[current_ground_page]->gsb_displayed = true;
    grids.push_back((*ground)[current_ground_page]);
    // reparent();

    pos.x1 = 0;
    pos.y1 = 0;
    pos.x2 = 1280;
    pos.y2 = 720;

    countTotalItems();
    GridSystem::reset();
}

void ConditionGridSystem::reset(void) {
    // add backpack and hand hardpoints
    grids.clear();
    grids.push_back(g.map->player->right_hand_hold);
    grids.push_back(g.map->player->left_hand_hold);
    grids.push_back(g.map->player->back);

    // body hardpoints
    grids.push_back(g.map->player->medical_upper_torso);
    grids.push_back(g.map->player->medical_lower_torso);
    grids.push_back(g.map->player->medical_left_upper_leg);
    grids.push_back(g.map->player->medical_right_upper_leg);
    grids.push_back(g.map->player->medical_left_lower_leg);
    grids.push_back(g.map->player->medical_right_lower_leg);
    grids.push_back(g.map->player->medical_left_upper_arm);
    grids.push_back(g.map->player->medical_right_upper_arm);
    grids.push_back(g.map->player->medical_left_lower_arm);
    grids.push_back(g.map->player->medical_right_lower_arm);

    g.map->player->addInventoryHardpoints(this);

    vector<Grid *> *ground = ground_at_player();
    (*ground)[current_ground_page]->gsb_displayed = true;
    grids.push_back((*ground)[current_ground_page]);
    // reparent();

    pos.x1 = 0;
    pos.y1 = 0;
    pos.x2 = 1280;
    pos.y2 = 720;

    countTotalItems();
    GridSystem::reset();
}

void main_buttons_update(void) {
    for(auto& b : g.main_buttons)
        b->pressed = false;
    if(g.ui == g.ui_MainMap)
        g.button_MainMap->pressed = true;
    else if(g.ui == g.ui_MiniMap)
        g.button_MiniMap->pressed = true;
    else if(g.ui == g.ui_Items)
        g.button_Items->pressed = true;
    else if(g.ui == g.ui_Vehicle)
        g.button_Vehicle->pressed = true;
    else if(g.ui == g.ui_Crafting)
        g.button_Crafting->pressed = true;
    else if(g.ui == g.ui_Skills)
        g.button_Skills->pressed = true;
    else if(g.ui == g.ui_Condition)
        g.button_Condition->pressed = true;
    else if(g.ui == g.ui_Camp)
        g.button_Camp->pressed = true;
}

// these could probably be a single function
void button_MainMap_press(void) {
    if(g.ui != g.ui_MainMap) {
        if(g.ui == g.ui_Crafting)
            g.ui_Crafting->craftGrids->exit();
        g.ui = g.ui_MainMap;
        g.color_bg = g.color_black;
    }
    main_buttons_update();
}

void button_Items_press(void) {
    if(g.ui != g.ui_Items) {
        if(g.ui == g.ui_Crafting)
            g.ui_Crafting->craftGrids->exit();
        g.ui_Items->gridsystem->reset();
        g.ui = g.ui_Items;
        g.color_bg = g.color_grey;
    }
    main_buttons_update();
}

void button_Vehicle_press(void) {
    if(g.ui != g.ui_Vehicle) {
        if(g.ui == g.ui_Crafting)
            g.ui_Crafting->craftGrids->exit();
        g.ui_Vehicle->gridsystem->reset();
        g.ui = g.ui_Vehicle;
        g.color_bg = g.color_grey;
    }
    main_buttons_update();
}

void button_MiniMap_press(void) {
    if(g.ui != g.ui_MiniMap) {
        if(g.ui == g.ui_Crafting)
            g.ui_Crafting->craftGrids->exit();
        g.minimap->recreate();
        g.ui = g.ui_MiniMap;
        g.color_bg = g.color_grey;
    }
    main_buttons_update();
}

void button_Skills_press(void) {
    if(g.ui != g.ui_Skills) {
        if(g.ui == g.ui_Crafting)
            g.ui_Crafting->craftGrids->exit();
        g.ui_Skills->skillsGrid->reset();
        g.ui = g.ui_Skills;
        g.color_bg = g.color_grey;
    }
    main_buttons_update();
}

void button_Condition_press(void) {
    if(g.ui != g.ui_Condition) {
        if(g.ui == g.ui_Crafting)
            g.ui_Crafting->craftGrids->exit();
        g.ui_Condition->gridsystem->reset();
        g.ui = g.ui_Condition;
        g.color_bg = g.color_grey;
    }
    main_buttons_update();
}

void button_Camp_press(void) {
    if(g.ui != g.ui_Camp) {
        if(g.ui == g.ui_Crafting)
            g.ui_Crafting->craftGrids->exit();
        g.ui_Camp->gridsystem->reset();
        g.ui = g.ui_Camp;
        g.color_bg = g.color_grey;
    }
    main_buttons_update();
}

void button_Scavenge_press(void) {
    if(g.ui != g.ui_Scavenge) {
        if(g.ui == g.ui_Crafting)
            g.ui_Crafting->craftGrids->exit();
        g.ui_Scavenge->reset();
        g.ui_Scavenge->setup();
        g.ui_Scavenge->gridsystem->reset();
        g.ui = g.ui_Scavenge;
        g.color_bg = g.color_grey;
    }
    main_buttons_update();
}

void button_Crafting_press(void) {
    if(g.ui != g.ui_Crafting) {
        g.ui_Crafting->craftGrids->reset();
        g.ui = g.ui_Crafting;
        g.color_bg = g.color_grey;
    }
    main_buttons_update();
}

void button_endturn_press(void) {
    g.map->player->nextMove += 1000;
    end_turn();
}

void load_bitmaps(void) {
    vector<string> filenames;
    /* 00 */ filenames.push_back("media/buttons/mainmap_up.png");
    /* 01 */ filenames.push_back("media/buttons/mainmap_down.png");
    /* 02 */ filenames.push_back("media/buttons/minimap_up.png");
    /* 03 */ filenames.push_back("media/buttons/minimap_down.png");
    /* 04 */ filenames.push_back("media/buttons/skills_up.png");
    /* 05 */ filenames.push_back("media/buttons/skills_down.png");
    /* 06 */ filenames.push_back("media/buttons/crafting_up.png");
    /* 07 */ filenames.push_back("media/buttons/crafting_down.png");
    /* 08 */ filenames.push_back("media/buttons/items_up.png");
    /* 09 */ filenames.push_back("media/buttons/items_down.png");
    /* 10 */ filenames.push_back("media/buttons/condition_up.png");
    /* 11 */ filenames.push_back("media/buttons/condition_down.png");
    /* 12 */ filenames.push_back("media/buttons/camp_up.png");
    /* 13 */ filenames.push_back("media/buttons/camp_down.png");
    /* 14 */ filenames.push_back("media/buttons/vehicle_up.png");
    /* 15 */ filenames.push_back("media/buttons/vehicle_down.png");
    /* 16 */ filenames.push_back("media/backgrounds/messagelogbg.png");
    /* 17 */ filenames.push_back("media/tile/grass.png");
    /* 18 */ filenames.push_back("media/tile/tree.png");
    /* 19 */ filenames.push_back("media/tile/city.png");
    /* 20 */ filenames.push_back("media/items/crowbar.png");
    /* 21 */ filenames.push_back("media/characters/test_character.png");
    /* 22 */ filenames.push_back("media/items/first_aid_kit.png");
    /* 23 */ filenames.push_back("media/tile/lake.png");
    /* 24 */ filenames.push_back("media/items/backpack.png");
    /* 25 */ filenames.push_back("media/buttons/end_turn_up.png");
    /* 26 */ filenames.push_back("media/items/shopping_trolley.png");
    /* 27 */ filenames.push_back("media/items/pill_bottle.png");
    /* 28 */ filenames.push_back("media/buttons/sort_grid.png");
    /* 29 */ filenames.push_back("media/buttons/next_page.png");
    /* 30 */ filenames.push_back("media/buttons/prev_page.png");
    /* 31 */ filenames.push_back("media/items/abstract/flee.png");
    /* 32 */ filenames.push_back("media/items/abstract/single_attack.png");
    /* 33 */ filenames.push_back("media/buttons/confirm.png");
    /* 34 */ filenames.push_back("media/items/bullets.png");
    /* 35 */ filenames.push_back("media/items/arrow.png");
    /* 36 */ filenames.push_back("media/items/abstract/skill_quick.png");
    /* 37 */ filenames.push_back("media/items/abstract/skill_lockpicking.png");
    /* 38 */ filenames.push_back("media/items/abstract/skill_myopia.png");
    /* 39 */ filenames.push_back("media/items/abstract/skill_metabolism.png");
    /* 40 */ filenames.push_back("media/items/whiskey.png");
    /* 41 */ filenames.push_back("media/items/clean_rag.png");
    /* 42 */ filenames.push_back("media/items/locations/factory.png");
    /* 43 */ filenames.push_back("media/items/locations/wood_shack.png");
    /* 44 */ filenames.push_back("media/buttons/scavenge.png");

    for(auto& filename : filenames) {
        ALLEGRO_BITMAP *bitmap = al_load_bitmap(filename.c_str());
        if(bitmap)
            info("Loaded file: " + filename);
        else
            errorQuit("Failed to load file: " + filename);
        g.bitmaps.push_back(bitmap);
    }
}

void unload_bitmaps(void) {
    for(auto& bitmap : g.bitmaps)
        al_destroy_bitmap(bitmap);
}

void init_buttons(void) {
    g.button_MainMap   = new Button;
    g.button_MiniMap   = new Button;
    g.button_Skills    = new Button;
    g.button_Crafting  = new Button;
    g.button_Items     = new Button;
    g.button_Condition = new Button;
    g.button_Camp      = new Button;
    g.button_Vehicle   = new Button;
    g.button_endturn   = new Button;
    g.button_scavenge  = new Button;

    // left
    g.button_MainMap->pos.x1 = 0;
    g.button_MainMap->pos.y1 = 480;
    g.button_MainMap->pos.x2 = 100;
    g.button_MainMap->pos.y2 = 60;
    g.button_MainMap->up = g.bitmaps[0];
    g.button_MainMap->down = g.bitmaps[1];
    g.button_MainMap->onMouseDown.connect(ptr_fun(button_MainMap_press));

    g.button_MiniMap->pos.x1 = 0;
    g.button_MiniMap->pos.y1 = 540;
    g.button_MiniMap->pos.x2 = 100;
    g.button_MiniMap->pos.y2 = 60;
    g.button_MiniMap->up = g.bitmaps[2];
    g.button_MiniMap->down = g.bitmaps[3];
    g.button_MiniMap->onMouseDown.connect(ptr_fun(button_MiniMap_press));

    g.button_Skills->pos.x1 = 0;
    g.button_Skills->pos.y1 = 600;
    g.button_Skills->pos.x2 = 100;
    g.button_Skills->pos.y2 = 60;
    g.button_Skills->up = g.bitmaps[4];
    g.button_Skills->down = g.bitmaps[5];
    g.button_Skills->onMouseDown.connect(ptr_fun(button_Skills_press));

    g.button_Crafting->pos.x1 = 0;
    g.button_Crafting->pos.y1 = 660;
    g.button_Crafting->pos.x2 = 100;
    g.button_Crafting->pos.y2 = 60;
    g.button_Crafting->up = g.bitmaps[6];
    g.button_Crafting->down = g.bitmaps[7];
    g.button_Crafting->onMouseDown.connect(ptr_fun(button_Crafting_press));

    // right
    g.button_Items->pos.x1 = 1180;
    g.button_Items->pos.y1 = 280;
    g.button_Items->pos.x2 = 100;
    g.button_Items->pos.y2 = 60;
    g.button_Items->up = g.bitmaps[8];
    g.button_Items->down = g.bitmaps[9];
    g.button_Items->onMouseDown.connect(ptr_fun(button_Items_press));

    g.button_Condition->pos.x1 = 1180;
    g.button_Condition->pos.y1 = 340;
    g.button_Condition->pos.x2 = 100;
    g.button_Condition->pos.y2 = 60;
    g.button_Condition->up = g.bitmaps[10];
    g.button_Condition->down = g.bitmaps[11];
    g.button_Condition->onMouseDown.connect(ptr_fun(button_Condition_press));

    g.button_Camp->pos.x1 = 1180;
    g.button_Camp->pos.y1 = 400;
    g.button_Camp->pos.x2 = 100;
    g.button_Camp->pos.y2 = 60;
    g.button_Camp->up = g.bitmaps[12];
    g.button_Camp->down = g.bitmaps[13];
    g.button_Camp->onMouseDown.connect(ptr_fun(button_Camp_press));

    g.button_Vehicle->pos.x1 = 1180;
    g.button_Vehicle->pos.y1 = 460;
    g.button_Vehicle->pos.x2 = 100;
    g.button_Vehicle->pos.y2 = 60;
    g.button_Vehicle->up = g.bitmaps[14];
    g.button_Vehicle->down = g.bitmaps[15];
    g.button_Vehicle->onMouseDown.connect(ptr_fun(button_Vehicle_press));

    g.button_endturn->pos.x1 = 1180;
    g.button_endturn->pos.y1 = 0;
    g.button_endturn->pos.x2 = 100;
    g.button_endturn->pos.y2 = 30;
    g.button_endturn->up = g.bitmaps[25];
    g.button_endturn->down = NULL;
    g.button_endturn->onMouseDown.connect(ptr_fun(button_endturn_press));

    g.button_scavenge->pos.x1 = 1180;
    g.button_scavenge->pos.y1 = 30;
    g.button_scavenge->pos.x2 = 100;
    g.button_scavenge->pos.y2 = 30;
    g.button_scavenge->up = g.bitmaps[44];
    g.button_scavenge->down = NULL;
    g.button_scavenge->onMouseDown.connect(ptr_fun(button_Scavenge_press));

    g.main_buttons.insert(g.button_MainMap);
    g.main_buttons.insert(g.button_MiniMap);
    g.main_buttons.insert(g.button_Skills);
    g.main_buttons.insert(g.button_Crafting);
    g.main_buttons.insert(g.button_Items);
    g.main_buttons.insert(g.button_Condition);
    g.main_buttons.insert(g.button_Camp);
    g.main_buttons.insert(g.button_Vehicle);
}

void init_messagelog(void) {
    g.log = new(MessageLog);
    g.log->pos.x1 = 100;
    g.log->pos.y1 = 570;
    g.log->pos.x2 = 1080;
    g.log->pos.y2 = 200;
    g.log->background = g.bitmaps[16];
    g.log->font = g.font;
}

void init_tilemap(void) {
    g.map = new TileMap (10, 10, 16, 16);

    TileInfo i;
    // grass
    i.minimap_color = al_map_rgb(0, 255, 0);
    i.bitmap_index = 0;
    i.blocks_los = false;
    i.blocks_movement = false;
    i.has_locations = false;
    g.map->tile_info.push_back(i);
    // tree
    i.minimap_color = al_map_rgb(0, 150, 0);
    i.bitmap_index = 1;
    i.blocks_los = false;
    i.blocks_movement = false;
    i.has_locations = true;
    g.map->tile_info.push_back(i);
    // city
    i.minimap_color = al_map_rgb(255, 255, 255);
    i.bitmap_index = 2;
    i.blocks_los = true;
    i.blocks_movement = false;
    i.has_locations = true;
    g.map->tile_info.push_back(i);
    // swamp
    i.minimap_color = al_map_rgb(0, 0, 200);
    i.bitmap_index = 3;
    i.blocks_los = false;
    i.blocks_movement = false;
    i.has_locations = true;
    g.map->tile_info.push_back(i);

    g.map->bitmaps.push_back(g.bitmaps[17]);
    g.map->bitmaps.push_back(g.bitmaps[18]);
    g.map->bitmaps.push_back(g.bitmaps[19]);
    g.map->bitmaps.push_back(g.bitmaps[23]);

    g.map->generate();
}

void init_minimap(void) {
    g.minimap = new(MiniMap);
}

MiniMapUI::MiniMapUI(void) {
    widgets.push_back(g.log);
    widgets.push_back(g.button_MainMap);
    widgets.push_back(g.button_MiniMap);
    widgets.push_back(g.button_Skills);
    widgets.push_back(g.button_Crafting);
    widgets.push_back(g.button_Items);
    widgets.push_back(g.button_Condition);
    widgets.push_back(g.button_Camp);
    widgets.push_back(g.button_Vehicle);
    widgets.push_back(g.minimap);
    widgets.push_back(g.button_endturn);
    widgets.push_back(g.button_scavenge);
}

MiniMapUI::~MiniMapUI(void) {
    info("~MiniMapUI()");
}

// creates the player and npcs
// must be called after init_tilemap();
void init_characters(void) {
    for(int i = 0; i < 5; i++) {
        int n = rand() % g.map->max_t;

        Character *c = new Character;

        if(i == 0) { // player is character 0
            g.map->player = c;
        }
        else // everyone else is an NPC
            g.map->characters.push_back(c);

        c->n = n;

        // add starting items
        Item *backpack = new Item("Backpack");
        c->back->PlaceItem(backpack);
        Item *first_aid_kit1 = new Item("First aid kit");
        backpack->storage->PlaceItem(first_aid_kit1);
        Item *first_aid_kit2 = new Item("First aid kit");
        c->left_hand_hold->PlaceItem(first_aid_kit2);
        Item *pill_bottle2 = new Item("Pill bottle");
        first_aid_kit1->storage->PlaceItem(pill_bottle2);
        Item *pill_bottle3 = new Item("Pill bottle");
        first_aid_kit2->storage->PlaceItem(pill_bottle3);

        backpack->storage->PlaceItem(new Item("Bullet", 5));
        backpack->storage->PlaceItem(new Item("Bullet", 3));
    }

    g.map->updateCharsByPos();
    g.map->player->update_visibility();
    g.map->focusOnPlayer();
    g.map->player->enableSkill(0);
    g.map->player->enableSkill(1);
    g.map->player->enableSkill(32);
    g.map->player->enableSkill(33);
}

MainMapUI::MainMapUI() {
    widgets.push_back(g.map);
    widgets.push_back(g.log);
    widgets.push_back(g.button_MainMap);
    widgets.push_back(g.button_MiniMap);
    widgets.push_back(g.button_Skills);
    widgets.push_back(g.button_Crafting);
    widgets.push_back(g.button_Items);
    widgets.push_back(g.button_Condition);
    widgets.push_back(g.button_Camp);
    widgets.push_back(g.button_Vehicle);
    widgets.push_back(g.button_scavenge);
    widgets.push_back(g.button_endturn);
}

MainMapUI::~MainMapUI(void) {
    info("~MainMapUI()");
}

void init_colors(void) {
    g.color_grey = al_color_name("grey");
    g.color_grey2 = al_map_rgb(200, 200, 200);
    g.color_grey3 = al_map_rgb(220, 220, 220);
    g.color_darkgrey = al_map_rgb(100, 100, 110);
    g.color_black = al_map_rgb(0, 0, 0);
    g.color_white = al_map_rgb(255, 255, 255);
    g.color_tile_tint = al_map_rgba_f(0.5, 0.5, 0.5, 1.0);
    g.color_active_tile_tint = al_map_rgba_f(1, 1, 1, 0.2);
    g.color_bg = g.color_black;
}

void allegro_init(void) {
    int ret = 0;

    al_init(); // no return value
    info("Probably initialized core allegro library.");

    g.display_x = 1280;
    g.display_y = 720;
    g.display = al_create_display(g.display_x, g.display_y);

    if(g.display == NULL)
        errorQuit("Failed to create display.");
    else
        info("Created display.");

    ret = al_init_primitives_addon();
    if(ret == false)
        errorQuit("Failed to initialize allegro addon: primitives.");
    else
        info("Initialized allegro addon: primitives.");

    ret = al_init_image_addon();
    if(ret == false)
        errorQuit("Failed to initialize allegro addon: image.");
    else
        info("Initialized allegro addon: image.");

    al_init_font_addon(); // no return value
    info("Probably initialized allegro addon: font.");

    ret = al_install_keyboard();
    if(ret == false)
        errorQuit("Failed to initialize keyboard.");
    else
        info("Initialized keyboard.");

    ret = al_install_mouse();
    if(ret == false)
        errorQuit("Failed to initialize mouse.");
    else
        info("Initialized mouse.");

    g.font = al_create_builtin_font();
    if(g.font == NULL)
        errorQuit("failed to load builtin font.");
    else
        info("Loaded builtin font.");

}

int main(void) {
    allegro_init();

    ALLEGRO_EVENT_QUEUE *event_queue = al_create_event_queue();
    if(event_queue == NULL)
        errorQuit("Failed to create event queue.");
    else
        info("Created event queue.");

    ALLEGRO_TIMER *timer = al_create_timer(1.0 / 60);
    if(timer == NULL)
        errorQuit("Error: failed to create timer.");
    else
        info("Created timer.");

    ALLEGRO_EVENT ev;
    ALLEGRO_MOUSE_STATE mouse_state;
    ALLEGRO_KEYBOARD_STATE keyboard_state;

    al_start_timer(timer);
    al_set_target_backbuffer(g.display);

    al_register_event_source(event_queue, al_get_display_event_source(g.display));
    al_register_event_source(event_queue, al_get_timer_event_source(timer));
    al_register_event_source(event_queue, al_get_keyboard_event_source());

    load_bitmaps();
    init_colors();
    init_iteminfo();
    init_tilemap();
    init_hardpointinfo();
    init_characters();
    init_buttons();
    init_messagelog();
    init_minimap();
    init_recipes();
    init_skills();
    init_locationdata();

    {
        g.ui_MainMap   = new MainMapUI;
        g.ui_MiniMap   = new MiniMapUI;
        g.ui_Items     = new ItemsUI;
        g.ui_Vehicle   = new VehicleUI;
        g.ui_Encounter = new EncounterUI;
        g.ui_Crafting  = new CraftingUI;
        g.ui_Skills    = new SkillsUI;
        g.ui_Condition = new ConditionUI;
        g.ui_Camp      = new CampUI;
        g.ui_Scavenge  = new ScavengeUI;

        // start on the main map
        button_MainMap_press();
    }

    bool redraw = true;
    bool was_mouse_down = false;

    // counter;
    int i = 0;

    // main loop
    while(1) {
        al_get_mouse_state(&mouse_state);
        al_get_keyboard_state(&keyboard_state);

        g.mouse_x = mouse_state.x;
        g.mouse_y = mouse_state.y;
        // 1 - RMB
        // 2 - LMB
        // 4 - wheel
        g.mouse_button = mouse_state.buttons;

        if (mouse_state.buttons != 0) {
            if (!was_mouse_down) {
                // mouse button down event
                g.ui->mouseDownEvent();
                was_mouse_down = true;
            }
        }
        else if (was_mouse_down) {
            g.ui->mouseUpEvent();
            // mouse up event
            was_mouse_down = false;
        } else {
            // if(i % 10 == 0) {
            //     g.ui->hoverOverEvent();
            // }
        }

        al_wait_for_event(event_queue, &ev);

        if(ev.type == ALLEGRO_EVENT_KEY_DOWN) {
            g.key = ev.keyboard.keycode;
            if(g.key == ALLEGRO_KEY_ESCAPE)
                if(g.ui == g.ui_MainMap)
                   break;
                else
                    button_MainMap_press();
            else
                g.ui->keyDownEvent();
        }
        else if(ev.type == ALLEGRO_EVENT_TIMER) {
            { // logic goes here
                // if(i < 1000) {
                //     end_turn();
                // }
                g.ui->update();
            }
            redraw = true;
        }
        else if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            break;
        }

        if(redraw && al_is_event_queue_empty(event_queue)) {
            redraw = false;
            al_clear_to_color(g.color_bg);

            { // drawing goes here
                g.ui->draw();
            }
            al_flip_display();
        }
        i++;
    }

    unload_bitmaps();

    /*
      allegro is automatically unloaded
      al_uninstall_system();
    */

    for(auto& widget : g.all_widgets) {
        delete widget;
    }

    info("Exiting");

    return 0;
}
