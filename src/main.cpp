#include <allegro5/allegro.h>
#include <allegro5/allegro_color.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_native_dialog.h>

#include <cstdint>
#include <cmath>

#include <string>
#include <unordered_map>
#include <vector>
#include <queue>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>

#include "./version.h"
#include "./util.h"
#include "./rect.h"
#include "./widget.h"

const bool DEBUG_VISIBILITY = false;
const int COMPILED_VERSION = 8; // save game version

using namespace std;

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
struct InteractUI;
struct HelpUI;
struct GridInfo;
struct ItemInfo;
struct LocationInfo;
struct Item;
struct BarIndicator;
struct TimeDisplay;
struct WeaponSwitcher;
struct Interact;
struct MainMenuUI;
struct FadeTransitionUI;
struct NotificationUI;

// global state
struct Game {
    bool running;

    // display dimensions
    const int display_x = 1280;
    const int display_y = 720;

    const char *font_filename = "media/fonts/DejaVuSans-Bold.ttf";
    const int font_height = 14;

    ALLEGRO_DISPLAY *display;
    ALLEGRO_FONT *font;
    ALLEGRO_KEYBOARD_STATE keyboard_state;
    ALLEGRO_MOUSE_STATE mouse_state;

    ALLEGRO_FILECHOOSER *save_filechooser;
    ALLEGRO_FILECHOOSER *load_filechooser;

    // global random number god
    mt19937 *rng;

    ALLEGRO_COLOR color_white;
    ALLEGRO_COLOR color_black;
    ALLEGRO_COLOR color_red;
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
    Button *button_sleep;

    // the main 8 buttons
    vector<Button *> main_buttons;

    int tilemap_sx; // persistent over games
    int tilemap_sy;

    TileMap *map;
    MiniMap *minimap;
    MessageLog *log;

    Item *skills[64];
    // default weapon
    Item *hand_combat;

    vector<ItemInfo> item_info;
    vector<LocationInfo> location_info;
    vector<GridInfo *> gridinfo_store;

    // Items UI
    GridInfo *right_hand_hold;
    GridInfo *left_hand_hold;
    GridInfo *right_hand;
    GridInfo *left_hand;
    GridInfo *back;
    GridInfo *head;
    GridInfo *neck;
    GridInfo *right_shoulder;
    GridInfo *left_shoulder;
    GridInfo *torso;
    GridInfo *legs;
    GridInfo *right_foot;
    GridInfo *left_foot;
    GridInfo *vehicle;

    // encounter UI
    GridInfo *encounter_selected;

    // condition UI
    GridInfo *medical_upper_torso;
    GridInfo *medical_lower_torso;
    GridInfo *medical_left_upper_leg;
    GridInfo *medical_right_upper_leg;
    GridInfo *medical_left_lower_leg;
    GridInfo *medical_right_lower_leg;
    GridInfo *medical_left_upper_arm;
    GridInfo *medical_right_upper_arm;
    GridInfo *medical_left_lower_arm;
    GridInfo *medical_right_lower_arm;

    GridInfo *default_info;
    GridInfo *ground;
    GridInfo *bottle;
    GridInfo *ammo_bow;

    BarIndicator *health_indicator;
    BarIndicator *pain_indicator;
    BarIndicator *temperature_indicator;
    BarIndicator *fatigue_indicator;
    BarIndicator *hydration_indicator;
    BarIndicator *satiety_indicator;
    BarIndicator *burden_indicator;

    TimeDisplay *time_display;
    WeaponSwitcher *weapon_switcher;

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
    InteractUI *ui_Interact;
    MainMenuUI *ui_MainMenu;
    HelpUI *ui_Help;
    FadeTransitionUI *ui_FadeTransition;
    NotificationUI *ui_Notification;

    unordered_map<const char *, Interact *> stories;
    //              ^^
    //            story name
    unordered_multimap<int, const char *> map_stories;
    //                 ^^          ^^
    //           map position     story name

    bool encounterInterrupt; // player
    int ai_encounterInterrupt; // AI at index (value is position)

    // current mouse state
    // TODO: remove this
    int mouse_x;
    int mouse_y;
    int mouse_button;
    // if holding an item, this is its offset from the mouse pointer
    int hold_off_x;
    int hold_off_y;
    int key;
    // time in seconds since last ui update
    double dt;

    // add a message to the message log
    void AddMessage(string str);
};

static Game g;

#include "./itemdefs.h"

// an item is something that lives on the grid
struct Item {
    // position and size of the item on the grid
    // x1, y1 - grid offset in grid units
    // x2, y2 - size in grid units
    Rect pos;
    // index into g.item_info, which contains
    // information that's common to the item
    int16_t info_index;
    // items can stack onto one item
    int16_t cur_stack;
    // items can be rotated
    bool rotated;
    // items can have their own storage space e.g.
    // a backpack. NULL if they don't
    Grid *storage;
    // the grid that the item belongs to.
    Grid *parent;
    // item's condition. Once it goes negative the item should be
    // destroyed. Item conditions should be decreased a bit on
    // each turn, and seperately when the item is used, so all items
    // should be checked at the end of the turn, and individual items
    // when they're used float condition;
    float condition;

    Item() { }
    Item(const Item& i);
    Item(int16_t info_index);
    Item(const char *item_name);
    Item(const char *item_name, int16_t num_stack);
    ~Item();

    void save(ostream &os);
    void load(istream &is);

    void init(int16_t info_index);
    void init_from_name(const char *name);
    static int index_from_name(const char *name);
    void resetHardpointPos(void);

    void draw(void);
    void drawHeld(void);
    void rotate(void);

    ALLEGRO_BITMAP *get_sprite();
    ItemSlot getItemSlot(void);
    const char *getName(void);
    int get_grid_size_x(void);
    int get_grid_size_y(void);

    bool isVehicle(void);
    bool isMedical(void);
    bool isConsumedOnApplication(void);
    bool isUsable(void);
    bool isConsumedOnUse(void);
    bool isLiquid(void);
    bool isClothing(void);

    int get_weapon_range(void);
    float get_weapon_damage(void);
    int get_weight(void);
    float get_warmth(void);
    void setHpDims(void);
};

static ALLEGRO_BITMAP *little_pink_bitmap(void);

Item *make_text_item(const char *text, ALLEGRO_COLOR bg_col) {
    int text_len = al_get_text_width(g.font, text);
    int item_size_x = 0;
    int item_size_y = 2 * 18;
    // find  the size of the item in steps of two that fits the text
    for(int i = 1; i < 12; i++) {
        if(text_len < i * 18) {
            item_size_x = i * 18;
            break;
        }
    }
    if(item_size_x == 0)
        errorQuit("make_text_item(): supplied text is too long");

    // center text
    float offset_x = round((item_size_x - text_len) / 2);
    float offset_y = round((item_size_y - g.font_height) / 2) - 1;

    ALLEGRO_BITMAP *b = al_create_bitmap(item_size_x, item_size_y);
    if(b == NULL) {
        al_set_new_bitmap_flags(ALLEGRO_MEMORY_BITMAP);
        b = al_create_bitmap(item_size_x, item_size_y);
        al_set_new_bitmap_flags(ALLEGRO_VIDEO_BITMAP);
    }
    if(b == NULL)
        b = little_pink_bitmap();

    // draw item bitmap
    al_set_target_bitmap(b);
    al_clear_to_color(al_map_rgba(0, 0, 0, 0));
    al_draw_filled_rounded_rectangle(0, 0, item_size_x, item_size_y, 7, 7, bg_col);
    al_draw_text(g.font, g.color_white, offset_x, offset_y, 0, text);

    al_set_target_backbuffer(g.display);

    // construct new ItemInfo and assign the bitmap to it
    ItemInfo tmp = g.item_info.at(0);
    tmp.sprite = b;
    tmp.name = strdup(text);
    tmp.is_text_item = true;
    g.item_info.push_back(tmp);

    // construct new Item with the above ItemInfo
    Item *ret = new Item ("none");
    ret->pos.x1 = 0;
    ret->pos.y1 = 0;
    ret->pos.x2 = item_size_x / 18;
    ret->pos.y2 = 2;
    ret->cur_stack = 1;
    ret->rotated = false;
    ret->storage = NULL;
    ret->parent = NULL;
    ret->condition = -1;
    ret->info_index = g.item_info.size() - 1;

    return ret;
}

Item *make_text_item(const char *text) {
    return make_text_item(text, g.color_darkgrey);
}

Item::Item(const Item& i) {
    this->pos = i.pos;
    this->cur_stack = i.cur_stack;
    this->info_index = i.info_index;
    this->parent = i.parent;
    this->storage = i.storage;
    this->rotated = i.rotated;
    this->condition = i.condition;
}

bool Item::isLiquid(void) {
    return g.item_info[info_index].isLiquid;
}

bool Item::isClothing(void) {
    ItemSlot s = g.item_info[info_index].slot;
    return
        s == ARMOR_HEAD ||
        s == ARMOR_TORSO ||
        s == ARMOR_LEGS ||
        s == ARMOR_LEFT_SHOE ||
        s == ARMOR_RIGHT_SHOE ||
        s == ARMOR_LEFT_HAND ||
        s == ARMOR_RIGHT_HAND;
}

ALLEGRO_BITMAP *Item::get_sprite() {
    return g.item_info[info_index].sprite;
}

float Item::get_warmth(void) {
    return g.item_info[info_index].warmth;
}

float Item::get_weapon_damage(void) {
    return g.item_info[info_index].weapon_damage;
}

int Item::get_weapon_range(void) {
    return g.item_info[info_index].weapon_range;
}

int Item::get_grid_size_x(void) {
    return g.item_info[info_index].grid_size_x;
}

int Item::get_grid_size_y(void) {
    return g.item_info[info_index].grid_size_y;
}

const char *Item::getName(void) {
    return g.item_info[info_index].name;
}

bool Item::isVehicle(void) {
    return g.item_info[info_index].isVehicle;
}

bool Item::isMedical(void) {
    return g.item_info[info_index].apply_to_body;
}

bool Item::isConsumedOnApplication(void) {
    return g.item_info[info_index].consumed_on_application;
}

bool Item::isUsable(void) {
    return
        storage == NULL;
}

bool Item::isConsumedOnUse(void) {
    return g.item_info[info_index].consumed_on_use;
}

ItemSlot Item::getItemSlot(void) {
    return g.item_info[info_index].slot;
}

void Item::rotate(void) {
    rotated = rotated ? false : true;
    swap(pos.x2, pos.y2);
}

struct MessageLog : public Widget {
    ALLEGRO_BITMAP *background;
    ALLEGRO_FONT *font;
    vector<string> lines;

    void mouseDown(void) override;

    void draw(void);
};

struct GridInfo {
    ALLEGRO_BITMAP *sprite;
    int16_t maxItems;
    bool noGrid;
    bool canHoldLiquid;
    bool vehiclepoint;
    bool medical;
    bool visible;
    bool pleaseRotateIt;

    GridInfo() {
        vehiclepoint = false;
        maxItems = 1;
        noGrid = true;
        canHoldLiquid = false;
        sprite = NULL;
        medical = false;
        visible = true;
        pleaseRotateIt = false;
    }
};

void init_hardpointinfo(void) {
    g.right_hand_hold = new GridInfo;
    g.left_hand_hold = new GridInfo;
    g.right_hand = new GridInfo;
    g.left_hand = new GridInfo;
    g.back = new GridInfo;
    g.head = new GridInfo;
    g.neck = new GridInfo;
    g.right_shoulder = new GridInfo;
    g.left_shoulder = new GridInfo;
    g.torso = new GridInfo;
    g.legs = new GridInfo;
    g.right_foot = new GridInfo;
    g.left_foot = new GridInfo;
    g.vehicle = new GridInfo;
    g.medical_upper_torso = new GridInfo;
    g.medical_lower_torso = new GridInfo;
    g.medical_left_upper_leg = new GridInfo;
    g.medical_right_upper_leg = new GridInfo;
    g.medical_left_lower_leg = new GridInfo;
    g.medical_right_lower_leg = new GridInfo;
    g.medical_left_upper_arm = new GridInfo;
    g.medical_right_upper_arm = new GridInfo;
    g.medical_left_lower_arm = new GridInfo;
    g.medical_right_lower_arm = new GridInfo;

    g.bottle = new GridInfo;
    g.ammo_bow = new GridInfo;
    g.ground = new GridInfo;

    g.encounter_selected = new GridInfo;

    g.bottle->canHoldLiquid = true;
    g.bottle->noGrid = false;
    g.ground->canHoldLiquid = true;
    g.ground->noGrid = false;
    g.ammo_bow->noGrid = false;

    g.default_info = NULL;
    //    g.default_info->noGrid = false;

    g.medical_upper_torso->medical = true;
    g.medical_lower_torso->medical = true;
    g.medical_left_upper_leg->medical = true;
    g.medical_right_upper_leg->medical = true;
    g.medical_left_lower_leg->medical = true;
    g.medical_right_lower_leg->medical = true;
    g.medical_left_upper_arm->medical = true;
    g.medical_right_upper_arm->medical = true;
    g.medical_left_lower_arm->medical = true;
    g.medical_right_lower_arm->medical = true;
    g.torso->maxItems = 3;
    g.vehicle->vehiclepoint = true;

    g.torso->visible = false;
    g.legs->visible = false;
    g.head->visible = false;
    g.right_hand->visible = false;
    g.left_hand->visible = false;
    g.right_foot->visible = false;
    g.left_foot->visible = false;

    g.right_shoulder->pleaseRotateIt = true;
    g.left_shoulder->pleaseRotateIt = true;

    g.gridinfo_store.push_back(g.right_hand_hold);
    g.gridinfo_store.push_back(g.left_hand_hold);
    g.gridinfo_store.push_back(g.right_hand);
    g.gridinfo_store.push_back(g.left_hand);
    g.gridinfo_store.push_back(g.back);
    g.gridinfo_store.push_back(g.head);
    g.gridinfo_store.push_back(g.neck);
    g.gridinfo_store.push_back(g.right_shoulder);
    g.gridinfo_store.push_back(g.left_shoulder);
    g.gridinfo_store.push_back(g.torso);
    g.gridinfo_store.push_back(g.legs);
    g.gridinfo_store.push_back(g.right_foot);
    g.gridinfo_store.push_back(g.left_foot);
    g.gridinfo_store.push_back(g.vehicle);
    g.gridinfo_store.push_back(g.encounter_selected);
    g.gridinfo_store.push_back(g.medical_upper_torso);
    g.gridinfo_store.push_back(g.medical_lower_torso);
    g.gridinfo_store.push_back(g.medical_left_upper_leg);
    g.gridinfo_store.push_back(g.medical_right_upper_leg);
    g.gridinfo_store.push_back(g.medical_left_lower_leg);
    g.gridinfo_store.push_back(g.medical_right_lower_leg);
    g.gridinfo_store.push_back(g.medical_left_upper_arm);
    g.gridinfo_store.push_back(g.medical_right_upper_arm);
    g.gridinfo_store.push_back(g.medical_left_lower_arm);
    g.gridinfo_store.push_back(g.medical_right_lower_arm);
    g.gridinfo_store.push_back(g.default_info);
    g.gridinfo_store.push_back(g.ground);
    g.gridinfo_store.push_back(g.bottle);
    g.gridinfo_store.push_back(g.ammo_bow);
}

struct Button : public Widget {
    bool pressed;
    ALLEGRO_BITMAP *up;
    ALLEGRO_BITMAP *down;
    const char *name;
    int name_len;

    Button(void);
    Button(const char *_name);
    ~Button();

    void mouseDown(void);
    void mouseUp(void);
    void keyDown(void);
    void hoverOver(void);

    void press(void);
    void draw(void);
    void update(void);
};

struct BarIndicator : public Widget {
    float *quantity;
    ALLEGRO_BITMAP *up;
    ALLEGRO_BITMAP *bars;
    const char *indicator_name;

    BarIndicator();
    ~BarIndicator() { };

    void mouseDown(void) { };
    void mouseUp(void) { };
    void keyDown(void) { };
    void hoverOver(void) { };

    void press(void) { };
    void draw(void);
    void update(void) { };
};

BarIndicator::BarIndicator() {
    up = NULL;
    quantity = NULL;
}

void BarIndicator::draw(void) {
    if(up != NULL && quantity != NULL && bars != NULL) {
        al_draw_text(g.font, g.color_white, pos.x1 + 2, pos.y1 - 6, 0, indicator_name);
        al_draw_bitmap(up, pos.x1, pos.y1 + 10, 0);
        al_draw_bitmap_region(bars, 0, 0, *quantity * pos.x2, pos.y2, pos.x1, pos.y1 + 10, 0);
    }
}

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
    static constexpr int8_t grid_px_x = 18;
    static constexpr int8_t grid_px_y = 18;

    // grid size in grid units
    int8_t grid_size_x;
    int8_t grid_size_y;

    vector<Item *> items;

    GridInfo *info;

    // seems like these could be part of InventoryGridSystem, etc
    GridSortButton *gsb;
    bool gsb_displayed;

    static bool PlaceItemWantsStacking;

    Grid() { }
    Grid(int w_pos_x, int w_pos_y, int size_x, int size_y, GridInfo *h);
    ~Grid();

    void save(ostream &os);
    void load(istream &is);

    void draw(void);
    void drawAt(float x, float y);

    void AddItem(Item *item);
    void RemoveItem(Item *item);
    Item *PlaceItem(Item *to_place);
    void Sort(void);
    void Sort(bool (*comp)(Item *l, Item *r));

    void resetPos(void);
    void resetPos(float sx, float sy);
    Item *grab_item(int x, int y); // get item at screen position
    void unstack_item(int x, int y); // unstack item at screen position

    bool item_compatible(Item *i);
};

Grid::Grid(int w_pos_x, int w_pos_y, int size_x, int size_y, GridInfo *h) {
    if(h != NULL)
        info = h;
    else
        info = g.default_info;
    grid_size_x = size_x;
    grid_size_y = size_y;
    pos.x1 = w_pos_x;
    pos.y1 = w_pos_y;
    resetPos();
    gsb = NULL;
    if(info == NULL || info->noGrid == false)
        gsb = new GridSortButton (this);
    gsb_displayed = false;
}

Grid::~Grid() {
    // info("~Grid()");
    // a grid owns its items
    delete gsb;
    for(auto &item : items)
        delete item;
}

void Grid::resetPos(void) {
    pos.x2 = pos.x1 + grid_size_x * grid_px_x;
    pos.y2 = pos.y1 + grid_size_y * grid_px_y;
}

void Grid::resetPos(float off_x, float off_y) {
    pos.x1 += off_x;
    pos.y1 += off_y;
    resetPos();
}

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

static vector<Grid *> *ground_at_player();
static void PlaceItemOnMultiGrid(vector<Grid *> *ground, Item *item);

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

void Item::init(int16_t info_index) {
    pos.x1 = 0;
    pos.y1 = 0;
    pos.x2 = g.item_info[info_index].grid_size_x;
    pos.y2 = g.item_info[info_index].grid_size_y;
    parent = NULL;
    cur_stack = 1;
    storage = NULL;
    this->rotated = false;
    this->info_index = info_index;
    if(g.item_info[info_index].isContainer == true) {
        GridInfo *info = NULL;

        // set any special grids here
        if(info_index == 18)
            info = g.bottle;
        if(info_index == 29)
            info = g.ammo_bow;

        storage = new Grid(0,
                           0,
                           g.item_info[info_index].container_size_x,
                           g.item_info[info_index].container_size_y,
                           info);
    }

    uniform_real_distribution<> cond_dist(0.02,1);
    this->condition = cond_dist(*g.rng);

    if(g.item_info[info_index].canBeDamaged == false ||
       g.item_info[info_index].maxStack != 1 ||
       g.item_info[info_index].isSkill == true ||
       g.item_info[info_index].isLocation == true ||
       g.item_info[info_index].isEncounterAction == true) {
        // skills, locations and stackable items don't have condition
        this->condition = -1.0;
    }
}

Item::Item(int16_t info_index) {
    init(info_index);
}

Item::Item(const char *item_name, int16_t num_stack) {
    init_from_name(item_name);
    this->cur_stack = num_stack;
}

Item::Item(const char *item_name) {
    init_from_name(item_name);
}

static inline bool is_correct_slot(ItemSlot s, GridInfo *i) {
    return
        (s == ARMOR_TORSO && i == g.torso) ||
        (s == ARMOR_LEGS && i == g.legs) ||
        (s == ARMOR_LEFT_SHOE && i == g.left_foot) ||
        (s == ARMOR_RIGHT_SHOE && i == g.right_foot) ||
        (s == ARMOR_HEAD && i == g.head) ||
        (s == ARMOR_LEFT_HAND && i == g.left_hand) ||
        (s == ARMOR_RIGHT_HAND && i == g.right_hand) ||
        (s == WEAPON_BOW && i == g.right_hand_hold) ||
        (s == WEAPON_BOW && i == g.left_hand_hold) ||
        (s == ARMOR_BACK && i == g.back) ||
        (s == ARMOR_BACK && i == g.left_hand_hold) ||
        (s == ARMOR_BACK && i == g.right_hand_hold)
        ;
}

// an item on a hardpoint may have a different size than
// on a normal grid. e.g. clothing
void Item::setHpDims(void) {
    if( parent != NULL &&
        parent->info != g.default_info &&
        parent->info->noGrid == true &&
        g.item_info[info_index].grid_size_on_hp_x != -1 &&
        is_correct_slot(g.item_info[info_index].slot, parent->info) == true &&
        g.item_info[info_index].is_text_item == false) {

        pos.x2 = g.item_info[info_index].grid_size_on_hp_x;
        pos.y2 = g.item_info[info_index].grid_size_on_hp_y;

    } else {
        if(g.item_info[info_index].is_text_item == false) {
            pos.x2 = g.item_info[info_index].grid_size_x;
            pos.y2 = g.item_info[info_index].grid_size_y;
        }
    }
    if(rotated == true)
        swap(pos.x2, pos.y2);
}

int Item::index_from_name(const char *item_name) {
    int i = 0;
    for(auto& info : g.item_info) {
        if(strcmp(info.name, item_name) == 0) {
            return i;
        }
        i++;
    }

    char buf[75];
    snprintf(buf, sizeof(buf), "Unknown item: \"%s\"", item_name);
    errorQuit(buf);
    return -1;
}

void Item::init_from_name(const char *item_name) {
    init(index_from_name(item_name));
}

int Item::get_weight(void) {
    int total = g.item_info[info_index].weight * cur_stack;

    if(storage != NULL) {
        for(auto& item : storage->items) {
            total += item->get_weight();
        }
    }
    return total;
}

void Grid::AddItem(Item *item) {
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
    if(info != NULL && info->noGrid == true) {
        // if we're on a hard point with no grid just check how many
        // items there are there already
        if((int)items.size() < info->maxItems) {
            to_place->pos.x1 = 0;
            to_place->pos.y1 = 0;
            AddItem(to_place);
            to_place->setHpDims();
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
        // cout << "placing " << g.item_info[to_place->info_index].name << " on " << this << endl;

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
                int16_t moved = g.item_info[to_place->info_index].maxStack - merge_with->cur_stack;
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
           this->info == NULL) {
            to_place->storage->gsb_displayed = false;
        }
        AddItem(to_place);
        to_place->setHpDims();
        return NULL;
    }
    else {
        // couldn't place it
        return to_place;
    }
}

bool Grid::item_compatible(Item *i) {
    if(i->isLiquid() == true) {
        // liquids can only be placed in bottles & ground
        if(info != NULL && info->canHoldLiquid == true) {
            return true;
        } else {
            return false;
        }
    }

    // you can place anything in a grid
    if(info == NULL)
        return true;

    ItemSlot slot = i->getItemSlot();

    if(info == g.ammo_bow && slot != AMMO_BOW)
        return false;
    if(info == g.back && slot != ARMOR_BACK)
        return false;
    if(info == g.torso && slot != ARMOR_TORSO)
        return false;
    if(info == g.head && slot != ARMOR_HEAD)
        return false;
    if(info == g.legs && slot != ARMOR_LEGS)
        return false;
    if(info == g.right_foot && slot != ARMOR_RIGHT_SHOE)
        return false;
    if(info == g.left_foot && slot != ARMOR_LEFT_SHOE)
        return false;
    if(info == g.right_hand && slot != ARMOR_RIGHT_HAND)
        return false;
    if(info == g.left_hand && slot != ARMOR_LEFT_HAND)
        return false;

    // can't place/use non-medical items on body parts
    if(info->medical == true &&
       i->isMedical() == false)
        return false;

    // can only place vehicles in vehicle hardpoint
    if(info->vehiclepoint == true &&
       i->isVehicle() == false)
        return false;

    // can't place vehicle on non-vehicle hardpoints
    if(i->isVehicle() == true &&
       info->vehiclepoint == false)
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

        storage->resetPos(g.item_info[info_index].container_offset_x,
                          g.item_info[info_index].container_offset_y);

        if(storage->gsb != NULL)
            storage->gsb->reset();
    } else {
        info("WARNING: no item parent");
    }
}

enum ActivityKind {
    ACTIVITY_NONE,
    ACTIVITY_WAIT,
    ACTIVITY_SLEEP,
    ACTIVITY_MOVE,
    ACTIVITY_HURT,
    ACTIVITY_USE,
    ACTIVITY_COMBAT,
    ACTIVITY_SCAVENGE
};

struct Wound {
    int age;
    float severity;
    float bleeding;

    Wound();

    void save(ostream &os);
    void load(istream &is);

    void modify_severity(float amount);
    void modify_bleeding(float amount);
};

enum WoundKind {
    WOUND_UPPER_TORSO = 0,
    WOUND_LOWER_TORSO,
    WOUND_LEFT_UPPER_LEG,
    WOUND_RIGHT_UPPER_LEG,
    WOUND_LEFT_LOWER_LEG,
    WOUND_RIGHT_LOWER_LEG,
    WOUND_LEFT_UPPER_ARM,
    WOUND_RIGHT_UPPER_ARM,
    WOUND_LEFT_LOWER_ARM,
    WOUND_RIGHT_LOWER_ARM,
    WOUND_MAX
};

Wound::Wound() {
    severity = 0;
    bleeding = 0;
    age = 0;
}

void Wound::modify_severity(float amount) {
    severity = max((float)0.0, severity + amount);

    // start bleeding at wound severity 0.005
    bleeding = max(0.0, severity - 0.005);

    if(severity < 0.001) {
        severity = 0;
        age = 0;
    }
}

void Wound::modify_bleeding(float amount) {
    bleeding = max((float)0.0, bleeding + amount);
}

enum Faction {
    FACTION_NONE = 0,
    FACTION_SCIENTISTS,
    FACTION_DUTY,
    FACTION_BANDITS,
    FACTION_WILD,
    FACTION_PLAYER,
};

static const char *faction_to_string(Faction fac) {
    if(fac == FACTION_PLAYER)
        return "player";
    else if (fac == FACTION_SCIENTISTS)
        return "scientists";
    else if (fac == FACTION_DUTY)
        return "duty";
    else if (fac == FACTION_BANDITS)
        return "bandits";
    else if (fac == FACTION_WILD)
        return "wild";
    else
        return "no faction";
}

enum DiseaseKind {
    DISEASE_COMMON_COLD = 0,
    DISEASE_FLU = 1,
    DISEASE_MAX = 2
};

struct Disease {
    // time left active, -1 for not active
    int duration;

    // immunity
    float vulnerability; // [0-1]

    Disease();

    void save(ostream &os);
    void load(istream &is);
};

Disease::Disease() {
    duration = -1;
    vulnerability = 0.5;
}

struct Character {
    bool active;
    int n; // position by offset
    int x, y; // cache to avoid computations

    char *name;

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

    int carry_weight;

    /*
      The wounds, in order:
      upper_torso;
      lower_torso;
      left_upper_leg;
      right_upper_leg;
      left_lower_leg;
      right_lower_leg;
      left_upper_arm;
      right_upper_arm;
      left_lower_arm;
      right_lower_arm;
    */

    vector<Wound> wounds;

    vector<Disease> diseases;

    // allowed range: 0 - 1
    float health;
    float pain;
    float warmth;
    float fatigue;
    float hydration;
    float satiety;
    float burden;

    int maxBurden; // maximum carry weight in grams

    ActivityKind activity;

    vector<Grid *> inventory_hardpoints;
    vector<Grid *> clothing;
    vector<Grid *> medical_hardpoints;

    ALLEGRO_BITMAP *sprite;
    vector<int> currently_seeing;

    // 0 - sees only tile they're on
    // 1 - sees immediate neighbours
    // etc
    int current_los_distance;

    int dt;
    int nextMove;

    uint64_t skills;

    int8_t selected_weapon_slot;

    // faction the character belongs to
    Faction faction;
    // what the factions think about the character
    vector<int> faction_reps;

    Character();
    ~Character();

    void save(ostream &os);
    void load(istream &is);

    void update_visibility(void);

    void draw(void);
    void drawOffset(int offset_x, int offset_y);

    void setPos(int n);
    void setPos(int x, int y);

    /*
      TODO why is this here
    */
    void addInventoryHardpoints(GridSystem *gs);
    void addVehicleHardpoint(GridSystem *gs);

    void do_AI(void);
    void die(void);
    void drop_all_items(void);
    void randomMove(void);
    void move(int new_n);

    bool hasSkill(int n);
    void enableSkill(int n);
    void disableSkill(int n);

    void useItem(Item *i);

    void spendTime(int _dt);
    void update(void);
    void post_update(void);

    void print_stats(void);
    void hurt(float amount);
    void sleep(void);
    void wait(void);

    bool useWeapon(void);
    void abuseItem(Item *item, float dt);
    void switchWeaponHand(void);
    Item *getSelectedWeapon(void);
    bool weaponUsesAmmo(void);
    bool hasAmmoForWeapon(void);
    bool consumeWeaponAmmo(void);
    int16_t countWeaponAmmo(void);

    void recomputeCarryWeight(void);
    void recomputeWarmth(void);

    Wound *random_wound(void);

    void awaken(void);
};

/*
  TODO: these functions have confusing names and functionality
*/
bool Character::weaponUsesAmmo(void) {
    return getSelectedWeapon()->getItemSlot() == WEAPON_BOW;
}

int16_t Character::countWeaponAmmo(void) {
    return getSelectedWeapon()->storage->items.front()->cur_stack;
}

bool Character::hasAmmoForWeapon(void) {
    Item *w = getSelectedWeapon();

    if(w->getItemSlot() == WEAPON_BOW) {
        if(w->storage->items.empty() == true) {
            return false;
        }
    }
    return true;
}

// consume ammo if we can
bool Character::consumeWeaponAmmo(void) {
    if(weaponUsesAmmo() == true) {
        if(hasAmmoForWeapon() == true) {
            Item *ammo = getSelectedWeapon()->storage->items.front();
            if(ammo->cur_stack == 1) {
                getSelectedWeapon()->storage->RemoveItem(ammo);
                delete ammo;
            } else {
                ammo->cur_stack--;
            }
            return true;
        } else {
            return false;
        }
    }
    return true;
}

void Character::switchWeaponHand(void) {
    selected_weapon_slot = selected_weapon_slot == 0 ? 1 : 0;
}

Item *Character::getSelectedWeapon(void) {
    vector<Item *> *w;

    if(selected_weapon_slot == 0) {
        w = &right_hand_hold->items;
    } else {
        w = &left_hand_hold->items;
    }

    if(w->empty() == false) {
        return w->front();
    } else {
        /* shrug */
        return g.hand_combat;
    }
}

Character::~Character() {
    free(name);

    for(auto& hardpoint : inventory_hardpoints) {
        delete hardpoint;
    }

    delete medical_upper_torso;
    delete medical_lower_torso;
    delete medical_left_upper_leg;
    delete medical_right_upper_leg;
    delete medical_left_lower_leg;
    delete medical_right_lower_leg;
    delete medical_left_upper_arm;
    delete medical_right_upper_arm;
    delete medical_left_lower_arm;
    delete medical_right_lower_arm;

    delete vehicle;
    // info("~Character()");
}

void Character::sleep(void) {
    g.AddMessage("You go to sleep...");
    activity = ACTIVITY_SLEEP;
    spendTime(10000);
}

void Character::wait(void) {
    g.AddMessage("You play with yourself for a while...");
    activity = ACTIVITY_WAIT;
    spendTime(1000);
}

Wound * Character::random_wound(void) {
    uniform_int_distribution<> wounds_dist(0, wounds.size() - 1);
    return &wounds[wounds_dist(*g.rng)];
}

void Character::hurt(float amount) {
    health -= min(health, amount);

    int wounds_num = amount / 0.03;

    while(wounds_num > 0.99) {
        Wound *w = random_wound();
        w->modify_severity(amount / 5);
        w->age = 0;
        wounds_num--;
    }

    pain -= min(pain, amount * 3);
    activity = ACTIVITY_COMBAT;
    spendTime(0);
}

void Character::spendTime(int _dt) {
    this->dt += _dt;
    this->nextMove += _dt;
}

void Character::useItem(Item *i) {
    if(i->isConsumedOnUse()) {
        i->parent->RemoveItem(i);

        float ih = g.item_info[i->info_index].improves_hydration;
        float is = g.item_info[i->info_index].improves_satiety;

        hydration = min(1.0, hydration + (double)ih);
        satiety = min(1.0, satiety + (double)is);

        delete i;
    }
    /*
      TODO: spent time should be item specific
    */
    activity = ACTIVITY_USE;
    spendTime(0);
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

static bool isCraftingSkill(int n) {
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
    head = new Grid (off_x - 18, off_y - 18, 4, 5, g.head);
    right_shoulder = new Grid (off_x - 53, off_y + 28, 2, 2, g.right_shoulder);
    left_shoulder = new Grid (off_x + 53, off_y + 28, 2, 2, g.left_shoulder);
    torso = new Grid (off_x - 66, off_y + 68, 9, 9, g.torso);
    right_hand = new Grid (off_x - 85, off_y + 223, 3, 3, g.right_hand);
    left_hand = new Grid (off_x + 67, off_y + 215, 3, 3, g.left_hand);
    legs = new Grid (off_x - 33, off_y + 220, 6, 11, g.legs);
    right_foot = new Grid (off_x - 35, off_y + 392, 3, 3, g.right_foot);
    left_foot = new Grid (off_x + 25, off_y + 392, 3, 3, g.left_foot);

    back = new Grid(680, 10, 2, 2, g.back);
    right_hand_hold = new Grid (680, 195, 2, 2, g.right_hand_hold);
    left_hand_hold = new Grid (680, 380, 2, 2, g.left_hand_hold);

    vehicle = new Grid(500, 150, 2, 2, g.vehicle);

    medical_upper_torso = new Grid(off_x - 19, off_y + 90, 3, 4, g.medical_upper_torso);
    medical_lower_torso = new Grid(off_x - 19, off_y + 172, 4, 4, g.medical_lower_torso);
    medical_left_upper_leg = new Grid(off_x - 17, off_y + 280, 2, 4, g.medical_left_upper_leg);
    medical_right_upper_leg = new Grid(off_x + 27, off_y + 280, 2, 4, g.medical_right_upper_leg);
    medical_left_lower_leg = new Grid(off_x - 22, off_y + 360, 2, 3, g.medical_left_lower_leg);
    medical_right_lower_leg = new Grid(off_x + 32, off_y + 360, 2, 3, g.medical_right_lower_leg);
    medical_left_upper_arm = new Grid(off_x - 52, off_y + 120, 2, 2, g.medical_left_upper_arm);
    medical_right_upper_arm = new Grid(off_x + 52, off_y + 120, 2, 2, g.medical_right_upper_arm);
    medical_left_lower_arm = new Grid(off_x - 60, off_y + 172, 2, 2, g.medical_left_lower_arm);
    medical_right_lower_arm = new Grid(off_x + 60, off_y + 172, 2, 2, g.medical_right_lower_arm);

    medical_hardpoints.push_back(medical_upper_torso);
    medical_hardpoints.push_back(medical_lower_torso);
    medical_hardpoints.push_back(medical_left_upper_leg);
    medical_hardpoints.push_back(medical_right_upper_leg);
    medical_hardpoints.push_back(medical_left_lower_leg);
    medical_hardpoints.push_back(medical_right_lower_leg);
    medical_hardpoints.push_back(medical_left_upper_arm);
    medical_hardpoints.push_back(medical_right_upper_arm);
    medical_hardpoints.push_back(medical_left_lower_arm);
    medical_hardpoints.push_back(medical_right_lower_arm);

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

    clothing.push_back(right_hand);
    clothing.push_back(left_hand);
    clothing.push_back(back);
    clothing.push_back(head);
    clothing.push_back(neck);
    clothing.push_back(torso);
    clothing.push_back(legs);
    clothing.push_back(right_foot);
    clothing.push_back(left_foot);

    current_los_distance = 3;
    currently_seeing.reserve(50);
    nextMove = 9000;
    dt = 0;

    skills = 0;

    health = 1.0;
    pain = 0.95;
    warmth = 0.22;
    fatigue = 0.87;
    hydration = 0.43;
    satiety = 0.72;
    burden = 0.9;

    maxBurden = 30000; // 30kg

    activity = ACTIVITY_MOVE;

    wounds.resize(WOUND_MAX);
    diseases.resize(DISEASE_MAX);

    selected_weapon_slot = 0;

    faction_reps.resize(6);


    // info("Character()");
}

static int dir_transform(int n, int dir);

// data that's common to all tiles of the same type
struct TileInfo {
    const char *name;
    ALLEGRO_COLOR minimap_color;
    ALLEGRO_BITMAP *sprite;
    bool blocks_los;
    bool blocks_movement;
    bool has_locations;
};

// data that's common to all locations of the same type
struct LocationInfo {
    Item *location_item;
    const char *description;

    // probability & item index
    vector<pair<float, int>> loot_table;

    // these are base values. The actual values depend on where on the map
    // you are
    float base_loot_level;
    float base_safety_level;
    float base_sneak_level;

    // how long it takes the location to reset (appear again as a scavenging option)
    int reset_time;

    void add_loot(const char *item_name, float prob);
};

void LocationInfo::add_loot(const char *item_name, float prob) {
    loot_table.push_back(make_pair(prob, Item::index_from_name(item_name)));
}

static void init_locationdata(void) {
    LocationInfo tmp;

    Item *factory = new Item ("Factory");
    tmp.description = "An abandoned factory, full of decaying industrial equipment.";
    tmp.location_item = factory;
    tmp.base_loot_level = 0.1;
    tmp.base_safety_level = 0.1;
    tmp.base_sneak_level = 0.2;
    tmp.reset_time = 10000;
    tmp.add_loot("backpack", 0.15);
    tmp.add_loot("first aid kit", 0.15);
    tmp.add_loot("crowbar", 0.15);
    tmp.add_loot("shopping trolley", 0.1);
    tmp.add_loot("clean rag", 0.5);
    tmp.add_loot("clean rag", 0.5);
    tmp.add_loot("clean rag", 0.5);
    tmp.add_loot("water bottle", 0.2);
    tmp.add_loot("water bottle", 0.2);
    tmp.add_loot("whiskey", 0.4);
    tmp.add_loot("water", 0.4);
    tmp.add_loot("moldy bread", 0.4);
    tmp.add_loot("moldy bread", 0.4);
    tmp.add_loot("moldy bread", 0.4);
    tmp.add_loot("red hoodie", 0.05);
    tmp.add_loot("ski mask", 0.05);
    tmp.add_loot("right glove", 0.05);
    tmp.add_loot("left glove", 0.05);
    tmp.add_loot("blue jeans", 0.05);
    tmp.add_loot("left shoe", 0.1);
    tmp.add_loot("right shoe", 0.01);
    g.location_info.push_back(tmp);

    tmp.loot_table.clear();

    Item *shack = new Item ("Shack in the woods");
    tmp.description = "A shack in the woods.";
    tmp.location_item = shack;
    tmp.base_loot_level = 0.3;
    tmp.base_safety_level = 0.3;
    tmp.base_sneak_level = 0.5;
    tmp.reset_time = 20000;
    tmp.add_loot("clean rag", 0.5);
    tmp.add_loot("water bottle", 0.2);
    tmp.add_loot("water bottle", 0.2);
    tmp.add_loot("red hoodie", 0.2);
    tmp.add_loot("whiskey", 0.2);
    tmp.add_loot("water", 0.2);
    tmp.add_loot("water", 0.2);
    tmp.add_loot("water", 0.2);
    tmp.add_loot("arrow", 0.2);
    tmp.add_loot("arrow", 0.2);
    tmp.add_loot("arrow", 0.2);
    tmp.add_loot("arrow", 0.2);
    tmp.add_loot("moldy bread", 0.2);
    tmp.add_loot("moldy bread", 0.2);
    tmp.add_loot("moldy bread", 0.2);
    tmp.add_loot("moldy bread", 0.2);
    g.location_info.push_back(tmp);
    tmp.loot_table.clear();

    tmp.description = "A small lake";
    tmp.location_item = make_text_item("Small lake", al_map_rgb(50, 50, 200));
    tmp.base_loot_level = 0.3;
    tmp.base_safety_level = 0.3;
    tmp.base_sneak_level = 0.5;
    tmp.reset_time = 20000;
    tmp.add_loot("water", 0.8);
    tmp.add_loot("water", 0.8);
    tmp.add_loot("water", 0.8);
    tmp.add_loot("water", 0.8);
    tmp.add_loot("water", 0.8);
    tmp.add_loot("water", 0.8);
    g.location_info.push_back(tmp);
    tmp.loot_table.clear();
}

// a location on the
struct Location {
    int16_t info_index; // index into LocationInfo
    int last_looted;

    void save(ostream &os);
    void load(istream &is);

    int getResetTime(void);
    vector<pair<float, int>> getLootTable(void);
};

int Location::getResetTime(void) {
    return g.location_info[info_index].reset_time;
}

vector<pair<float, int>> Location::getLootTable(void) {
    return g.location_info[info_index].loot_table;
}

struct Label {
    int n;
    int x, y;
    const char *text;
    int offset_x;

    void save(ostream &os);
    void load(istream &is);

    void draw(void);
};

// a hex tile
struct Tile {
    int8_t info_index; // index into TileInfo
    int8_t visible;
    vector<Grid *> *ground_items;
    vector<Location *> *locations;

    ALLEGRO_BITMAP *getBitmap(void);
};

struct Ecology {
    int want_creatures;
};

struct RealityBubble {
    int position; // center of reality bubble
    int radius;
};

struct TileMap : public Widget {
    // display dimensions
    int cols;
    int rows;
    // current view (tile offset)
    int view_x;
    int view_y;
    // current view (pixel offset)
    int view_px;
    int view_py;
    // offset pixels from view_x and view_y
    int res_px;
    int res_py;
    // dimensions of the map tiles
    int size_x;
    int size_y;
    /*
      TODO: replace the magic numbers throughout with these
    */
    // size of the hex bitmaps
    static constexpr int hex_size_x = 100;
    static constexpr int hex_size_y = 80;
    // size of the hex render steps
    static constexpr int hex_step_x = 80;
    static constexpr int hex_step_y = 40;

    // stuff that's constant between TileMap::draw
    // and TileMap::drawTile
    int max_t;
    int start;
    int mouse_n;

    // map from positions to characters on that position
    unordered_multimap<int, Character *> charsByPos;

    vector<Character *> all_characters;
    vector<Character *> characters;
    Character *player;

    Ecology eco;
    RealityBubble bubble;

    // array of size_x * size_y tiles, allocated in constructor
    vector<Tile> tiles;

    vector<TileInfo> tile_info;

    vector<Label> labels;

    TileMap(int sx, int sy, int cols, int rows);
    ~TileMap();

    void save(ostream &os);
    void load(istream &is);

    void resetViewXY(void);
    void resetViewPXPY(void);

    void mouseDown(void);
    void keyDown(void);

    // must be called after the bitmaps vector is filled
    void generate(void);

    void draw(void);
    void update(void);

    void drawTile(int i, int x, int y);
    void drawTopHalfOfTileAt(int x, int y);
    void mouseToTileXY(int &x, int &y);

    int mouseToTileN(void);
    void focusOn(int n);
    void focusOnPlayer(void);
    Character *characterAt(int n);
    bool playerSees(int n);

    void updateCharsByPos(void);
    void removeCharacter(Character *to_kill);
    Character *addRandomCharacter(void);
    Character *addRandomCharacter(Faction fac);
    Character *addRandomCharacterNearPlayer(void);

    float getCurrentTemperature(int n);
    const char *getTileName(int n);
    ALLEGRO_BITMAP *getBitmap(int n);

    bool blocks_movement(int n);
    bool blocks_los(int n);

    void runEcology();

    int random_uninhabited_position(void);
    int random_uninhabited_position_in_rect(int x1, int y1, int x2, int y2);
    int distance(int n1, int n2);

    void addLabel(int n, const char *text);

    void DeleteGroundIfEmpty(int n);
    void CollectGroundGrids(int n);

    void updateActiveCharacters(void);
};

void TileMap::update(void) {
    // handles smooth scrolling
    int d = 1000 * g.dt;

    // keyboard scrolling
    if(al_key_down(&g.keyboard_state, ALLEGRO_KEY_UP))
        view_py -= d;
    else if(al_key_down(&g.keyboard_state, ALLEGRO_KEY_DOWN))
        view_py += d;

    if(al_key_down(&g.keyboard_state, ALLEGRO_KEY_LEFT))
        view_px -= d;
    else if(al_key_down(&g.keyboard_state, ALLEGRO_KEY_RIGHT))
        view_px += d;

    // mouse scrolling (hold MMB)
    static int old_mx; // stores mouse position before we pressed MMB
    static int old_my;

    // TODO: draw symbol on (old_mx, old_my)

    if (g.mouse_state.buttons & 4) {
        // scroll map proportional to distance from position before
        // we pressed MMB
        view_px -= (old_mx - g.mouse_x) * g.dt * 4;
        view_py -= (old_my - g.mouse_y) * g.dt * 4;
    } else {
        old_mx = g.mouse_x;
        old_my = g.mouse_y;
    }
    resetViewXY();
}

void TileMap::resetViewPXPY(void) {
    view_px = view_x * hex_step_x;
    view_py = view_y * hex_step_y;

    res_px = 0;
    res_py = 0;
}

void TileMap::resetViewXY(void) {
    // clamp view_x to [0, size_x - cols]
    view_x = max(0, min(size_x - cols, view_px / hex_step_x));
    view_y = max(0, min(size_y - rows, view_py / hex_step_y));

    // clamp view_px to [0, (size_x - cols) * hex_step_x]
    view_px = max(0, min(view_px, (size_x - cols) * hex_step_x));
    view_py = max(0, min(view_py, (size_y - rows) * hex_step_y));

    res_px = view_px % hex_step_x;
    res_py = view_py % hex_step_y;
}

void Character::awaken(void) {
    nextMove = g.map->player->nextMove + 100;
    dt = 1000;
}

void TileMap::updateActiveCharacters(void) {
    characters.clear();
    bubble.position = g.map->player->n;

    int i = 0;
    for(auto&& c : all_characters) {
        if(distance(c->n, bubble.position) < bubble.radius) {
            if(c->active == false)
                c->awaken();
            characters.push_back(c);
            i++;
        } else {
            c->active = false;
        }
    }

    cout << "added " << i << " characters to the active list" << endl;
}


void Label::draw(void) {
    // is it on the screen?
    if(y >= g.map->view_y + g.map->rows ||
       x >= g.map->view_x + g.map->cols ||
       y < g.map->view_y ||
       x < g.map->view_x) {
        return;
    }
    if(g.map->tiles[n].visible == false)
        return;

    int r_x = x - g.map->view_x;
    int r_y = y - g.map->view_y;

    int off_x = TileMap::hex_step_x * r_x - g.map->res_px;
    int off_y = n % 2  == 0 ? 0 : 20;
    off_y += TileMap::hex_step_y * r_y - g.map->res_py;

    al_draw_text(g.font,
                 g.color_white,
                 g.map->pos.x1 + off_x + offset_x,
                 g.map->pos.y1 + off_y + 55,
                 0,
                 text);
}

void TileMap::addLabel(int n, const char *text) {
    Label tmp;
    tmp.n = n;
    tmp.x = n % g.map->size_x;
    tmp.y = n / g.map->size_y;
    tmp.text = text;
    tmp.offset_x = (100 - (strlen(text) * 8)) / 2;
    labels.push_back(tmp);
}

static char *random_human_NPC_name(void);

void TileMap::runEcology() {
    if((int)all_characters.size() < eco.want_creatures) {
        Character *c = addRandomCharacterNearPlayer();
        printf("Ecology added character %s at %d\n", c->name, c->n);
    }
}

bool TileMap::blocks_movement(int n) {
    return tile_info[tiles[n].info_index].blocks_movement;
}

bool TileMap::blocks_los(int n) {
    return tile_info[tiles[n].info_index].blocks_los;
}

ALLEGRO_BITMAP *TileMap::getBitmap(int n) {
    return tile_info[tiles[n].info_index].sprite;
}

const char *TileMap::getTileName(int n) {
    return tile_info[tiles[n].info_index].name;
}

float TileMap::getCurrentTemperature(__attribute__ ((unused)) int n) {
    return 0.9;
}

static bool good_index(int n) {
    return n >= 0 && n <= g.map->max_t;
}

static int dir_transform(int n, int dir);

int TileMap::distance(int n1, int n2) {
    int x1 = n1 % size_x;
    int y1 = n1 / size_y;
    int x2 = n2 % size_x;
    int y2 = n2 / size_y;
    return pow(x1 - x2, 2) + pow(y1 - y2, 2);
}

int TileMap::random_uninhabited_position_in_rect(int x1, int y1, int x2, int y2) {
    uniform_int_distribution<> position_dist_x(x1, x2);
    uniform_int_distribution<> position_dist_y(y1, y2);

    int x, y, new_n;
    int trials = 0;
    bool ok;
    // find a position away from all the other characters
    do {
        ok = true;
        x = position_dist_x(*g.rng);
        y = position_dist_y(*g.rng);
        new_n = size_x * y + x;
        if(distance(new_n, player->n) < 10)
            ok = false;
        for(auto&& c : characters) {
            if(distance(new_n, c->n) < 10) {
                ok = false;
                break;
            }
        }
        trials++;
        if(trials >= 10) // map might be full
            ok = true;
    } while(ok == false);

    return new_n;
}

int TileMap::random_uninhabited_position(void) {
    uniform_int_distribution<> position_dist(0, max_t);
    int new_n;
    int trials = 0;
    bool ok;
    // find a position away from all the other characters
    do {
        ok = true;
        new_n = position_dist(*g.rng);
        if(distance(new_n, player->n) < 10)
            ok = false;
        for(auto&& c : characters) {
            if(distance(new_n, c->n) < 10) {
                ok = false;
                break;
            }
        }
        trials++;
        if(trials >= 10) // map might be full
            ok = true;
    } while(ok == false);

    return new_n;
}

void Character::setPos(int n) {
    this->n = n;
    x = n % g.map->size_x;
    y = n / g.map->size_x;
}

void Character::setPos(int x, int y) {
    this->x = x;
    this->y = y;
    n = y * g.map->size_x + x;
}

static char *random_human_NPC_name(void);

Character *TileMap::addRandomCharacterNearPlayer(void) {
    Character *c = addRandomCharacter();
    c->active = true;
    int r = 10;
    int x1 = max(0, g.map->player->x - r);
    int y1 = max(0, g.map->player->y - r);
    int x2 = max(size_x /* ?? */ - 1, g.map->player->x + r);
    int y2 = max(size_y /* ?? */ - 1, g.map->player->y + r);
    c->n = random_uninhabited_position_in_rect(x1, y1, x2, y2);
    return c;
}

Character *TileMap::addRandomCharacter(void) {
    // pick random faction
    Faction fac;
    {
        vector<int> faction_options =
            { 0, 0, 0, // none
              1, // scientists
              2, // duty
              3, 3, // bandits
              4, 4, 4 // animal
            };

        uniform_int_distribution<>
            faction_dist(0, faction_options.size() - 1);
        fac = (Faction)faction_options.at(faction_dist(*g.rng));
    }

    return addRandomCharacter(fac);
}

Character *TileMap::addRandomCharacter(Faction fac) {
    Character *new_char = new Character;
    new_char->active = false;
    new_char->faction = fac;

    for(auto&& fac_rep : new_char->faction_reps) fac_rep = -1;
    new_char->faction_reps[fac] = 1;

    // pick name
    if(fac == FACTION_WILD)
        new_char->name = strdup("dog");
    else
        new_char->name = random_human_NPC_name();

    // pick sprite
    if(fac == FACTION_NONE) {
        vector<int> human_sprite_map = { 21, 91, 94, 95 };
        uniform_int_distribution<>
            sprite_dist(0, human_sprite_map.size() - 1);
        new_char->sprite =
            g.bitmaps[human_sprite_map[sprite_dist(*g.rng)]];
    }
    else if(fac == FACTION_SCIENTISTS)
        new_char->sprite = g.bitmaps[91];
    else if(fac == FACTION_DUTY)
        new_char->sprite = g.bitmaps[21];
    else if(fac == FACTION_BANDITS)
        new_char->sprite = g.bitmaps[94];
    else if(fac == FACTION_WILD)
        new_char->sprite = g.bitmaps[90];
    else if(fac == FACTION_PLAYER)
        new_char->sprite = g.bitmaps[21];

    // position
    new_char->setPos(random_uninhabited_position());

    // // starting health
    // uniform_real_distribution<> health_dist(0.1, 1);
    // new_char->health = health_dist(*g.rng);

    // starting delay
    uniform_int_distribution<> delay_dist(0, 500);
    new_char->nextMove = g.map->player->nextMove + delay_dist(*g.rng);

    // add starting inventory, unless it's an animal
    if(fac != FACTION_WILD) {
        Item *backpack = new Item("backpack");
        new_char->back->PlaceItem(backpack);
        Item *first_aid_kit1 = new Item("first aid kit");
        backpack->storage->PlaceItem(first_aid_kit1);
        Item *first_aid_kit2 = new Item("first aid kit");
        new_char->left_hand_hold->PlaceItem(first_aid_kit2);
        Item *pill_bottle2 = new Item("pill bottle");
        first_aid_kit1->storage->PlaceItem(pill_bottle2);
        Item *pill_bottle3 = new Item("pill bottle");
        first_aid_kit2->storage->PlaceItem(pill_bottle3);

        backpack->storage->PlaceItem(new Item("bullet", 5));
        backpack->storage->PlaceItem(new Item("bullet", 3));
    }

    all_characters.push_back(new_char);
    return new_char;
}

static void runInteract(const char *name);

void TileMap::removeCharacter(Character *to_kill) {
    assert(to_kill != NULL);

    if(to_kill == player) {
        runInteract("player_dead");
        return;
    }

    to_kill->drop_all_items();
    DeleteGroundIfEmpty(to_kill->n);

    int i = 0;
    for(auto& character : g.map->all_characters) {
        if(character == to_kill) {
            g.map->all_characters.erase(g.map->all_characters.begin() + i);
            delete to_kill;
            updateActiveCharacters();
            updateCharsByPos();
            return;
        }
        i++;
    }
}

void TileMap::updateCharsByPos(void) {
    // cout << "updateCharsByPos" << endl;
    charsByPos.clear();

    charsByPos.emplace(player->n, player);

    for(auto& npc : characters) {
        assert(npc != NULL);
        charsByPos.emplace(npc->n, npc);
    }
}

// do stuff on the map
void Character::do_AI(void) {
    if(health < 0.01) {
        this->die();
        return;
    }
    update();
    randomMove();
    post_update();
}

static vector<Grid *> *ground_at_character(Character *character);

// decrease item's condition and destroy it if its condition is too low
void Character::abuseItem(Item *item, float amount) {
    if(item->condition < -0.5)
        // some items like the fist can't be damaged
        return;

    item->condition -= amount;

    if(item->condition < 0.01) {
        cout << name << " destroyed " << item->getName() << " at " << n << endl;
        // items is destroyed
        if(this == g.map->player) {
            // add message if character is player
            char buf[70];
            sprintf(buf, "Your %s is destroyed!", item->getName());
            g.AddMessage(buf);
        }
        if(item->storage != NULL) {
            // dump the contained items on the ground
            while(item->storage->items.empty() != true) {
                Item *moved = item->storage->items.front();
                item->storage->RemoveItem(moved);
                PlaceItemOnMultiGrid(ground_at_character(this), moved);
            }
        }

        if(item->parent == NULL) {
            /*
              TODO: wtf?
             */
            cout << name << "'s " << item->getName() << " has NULL parent" << endl;

            delete item;
            return;
        }

        assert(item->parent != NULL);
        item->parent->RemoveItem(item);

        delete item;
    }
}

bool Character::useWeapon(void) {
    if(consumeWeaponAmmo() == false)
        return false;

    Item *w = getSelectedWeapon();
    abuseItem(w, 0.00777);

    return true;
}

void Character::post_update(void) {
    if(health < 0.01) {
        this->die();
        return;
    }
}

void Character::recomputeWarmth(void) {
    float warmth_sum = 0.0;
    float map_temperature = g.map->getCurrentTemperature(this->n);

    for(auto& point : clothing) {
        for(auto& cloth : point->items) {
            warmth_sum += cloth->get_warmth();
        }
    }

    this->warmth = 1.0 - (map_temperature - warmth_sum);

    /*
      TODO: care about overheating?
    */
    this->warmth = min((float)1.0, this->warmth);
}

void Character::recomputeCarryWeight(void) {
    carry_weight = 0;

    for(auto& grid : inventory_hardpoints) {
        for(auto& item : grid->items) {
            carry_weight += item->get_weight();
        }
    }
    cout << "playing is carrying " << carry_weight << 'g' << endl;

    burden = 1.0 - (float)carry_weight / maxBurden;
}

void Character::update(void) {
    assert(activity != ACTIVITY_NONE || g.encounterInterrupt == true);

    // heal 0.5% over 1000 time units
    float healChange = 0.005 * 0.001 * dt;
    health = min((float)1.0, health + healChange);

    // increase fatigue by 3% per 1000 time units by default
    float fatigueChange = 0.03 * 0.001 * dt;
    if(activity == ACTIVITY_MOVE)
        fatigue = max((float)0.0, fatigue - fatigueChange);

    // else recover 3% per 1000 time units if we're sleeping
    // or waiting
    if(activity == ACTIVITY_SLEEP ||
       activity == ACTIVITY_WAIT)
        fatigue = min((float)1.0, fatigue + fatigueChange);

    // increase pain stat by 6% per 1000 time units
    float painChange = 0.06 * 0.001 * dt;
    pain = min((float)1.0, pain + painChange);
    pain = min(health, pain);

    // decrease hydration by 4% per 1000 time units
    float hydrationChange = 0.04 * 0.001 * dt;
    hydration = max((float)0.0, hydration - hydrationChange);

    // decrease satiety by 2% per 1000 time units
    float satietyChange = 0.02 * 0.001 * dt;
    satiety = max((float)0.0, satiety - satietyChange);

    // decrease health if the character is dangerously low on some
    // resource
    if(hydration < 0.15) {
        float hydration_damage = (0.15 - hydration) * dt / 4000;
        health = max((float)0.0, health - hydration_damage);
    }
    if(satiety < 0.15) {
        float satiety_damage = (0.15 - satiety) * dt / 4000;
        health = max((float)0.0, health - satiety_damage);
    }
    if(fatigue < 0.15) {
        float fatigue_damage = (0.15 - fatigue) * dt / 4000;
        health = max((float)0.0, health - fatigue_damage);
    }
    if(warmth < 0.15) {
        float warmth_damage = (0.15 - warmth) * dt / 4000;
        health = max((float)0.0, health - warmth_damage);
    }

    size_t i = 0;
    float wound_severity = 0;
    for(auto&& m_hp : medical_hardpoints) {
        // if there's a clean rag on the wound, then
        // decrease the wound severity
        if(m_hp->items.empty() == false) {
            wounds[i].modify_severity(-0.002 * dt * 0.001);

        // if we're wounded and there's nothing on the wound,
        // increase it's severity
        }
        else if(wounds[i].severity > 0) {
            // start healing after 5 hours
            if(wounds[i].age > 3000) {
                if(wounds[i].severity >= 0.004) {
                    wounds[i].modify_severity(-0.002 * dt * 0.001);
                }
            } else {
                wounds[i].modify_severity( 0.0005 * dt * 0.001);
            }
            wounds[i].age += dt;
        }

        wound_severity += wounds[i].severity;

        i++;
    }

    // hurt the player based on bleeding wounds
    float bleed_damage = 0;
    for(auto&& w : wounds) {
        bleed_damage += (float)w.bleeding;
    }
    health = max((float)0.0, health - bleed_damage);

    if(activity == ACTIVITY_MOVE) {
        /*
          decrease condition of items that the player is carrying
          on inventory hardpoints
        */
        for(auto& hp : inventory_hardpoints) {
            for(auto& item : hp->items) {
                abuseItem(item, dt * 0.002 * 0.001);
            }
        }
    }

    /*
      update diseases
    */
    int idx = 0;
    for(auto&& d : diseases) {
        d.duration -= dt;
        if(d.duration > 0) {
            if(idx == DISEASE_COMMON_COLD) {
                float hydrationChange = 0.04 * 0.001 * dt;
                hydration = max((float)0.0, hydration - hydrationChange);
            }
        }
        idx++;
    }

    /*
      get new diseases
    */
    if(warmth < 0.2 && diseases[DISEASE_COMMON_COLD].duration < -10000) {
        diseases[DISEASE_COMMON_COLD].duration = 10000;
    }

    dt = 0;
    activity = ACTIVITY_NONE;
}

void Character::print_stats(void) {
    cout << " H: " << health << " P: " << pain << " F: " << fatigue
         << " Hy: " << hydration << " S: " << satiety << " B: " << burden
         << endl;
}

void Character::randomMove(void) {
    int new_n;
    uniform_int_distribution<> dist(1, 6);

    do {
        new_n = dir_transform(n, dist(*g.rng));
    } while(good_index(new_n) == false ||
            g.map->blocks_movement(new_n) == true);

    move(new_n);
}

static void runPlayerEncounter(void);

// character c1 interrupts c2.
static void chInterruptsPlayer(Character *c1) {
    int playersNewDt = g.map->player->nextMove - c1->nextMove;

    cout << c1 << " interrupted player! nextMove: "
         << g.map->player->nextMove << " -> " << c1->nextMove << " dt: "
         << g.map->player->dt << " -> " << playersNewDt << endl;

    /*
      TODO: this fails
    */
    if(playersNewDt < 0)
        printf("chInterruptsPlayer(): player's dt is negative: %d\n", playersNewDt);

    g.map->player->dt = playersNewDt;
    g.map->player->nextMove = c1->nextMove;
    g.map->player->update();
}

static void runRandomMoveEvents(void) {
    uniform_int_distribution<> d100(0, 100);
    int roll = d100(*g.rng);

    if(roll == 0) {
        g.AddMessage("You disturb a flock of birds and they fly away loudly.");
    }
}

// return true if there was an event
static bool runRandomScavengingEvents(void) {
    uniform_int_distribution<> d100(0, 100);
    int roll = d100(*g.rng);

    if(roll > 80) {
        // 1 in 5 chance of falling down and hurting yourself
        runInteract("fall_down");
        return true;
    }
    return false;
}

static void runAIEncounter(int n);

// deletes empty ground grids at tile
void TileMap::CollectGroundGrids(int n) {
    vector<Grid *> *mg = tiles[n].ground_items;

    if(mg == NULL)
        return;

    vector<Grid *>::iterator it;
    for(it = mg->begin() ; it != mg->end() ; ) {
        if((*it)->items.empty() == true) {
            delete *it;
            it = mg->erase(it);
        } else {
            it++;
        }
    }

    if(mg->empty() == true) {
        delete mg;
        tiles[n].ground_items = NULL;
    }
}

// deletes the ground if there are zero items on the tile
void TileMap::DeleteGroundIfEmpty(int n) {
    vector<Grid *> *mg = tiles[n].ground_items;

    if(mg == NULL)
        return;

    for(auto&& grid : *mg)
        if(grid->items.empty() == false)
            return;

    for(auto&& grid : *mg)
        delete grid;

    mg->clear();
    delete mg;

    tiles[n].ground_items = NULL;
}

void Character::move(int new_n) {
    if(good_index(new_n) == true) {
        g.map->DeleteGroundIfEmpty(this->n);

        setPos(new_n);
        g.map->updateCharsByPos();

        // start an encounter if
        if(this == g.map->player) {
            // only update the active character list if we moved more than
            // 5 hexes
            int dist = g.map->distance(this->n, g.map->bubble.position);
            printf("distance: %d\n", dist);
            if(dist > 25)
                g.map->updateActiveCharacters();

            g.map->CollectGroundGrids(this->n);
            // this character is the player and there's more than
            // one character on that tile
            if(g.map->charsByPos.count(this->n) > 1) {
                g.encounterInterrupt = true;
                /*
                  TODO: player interrupts AIs
                */
            } else {
                // check if there are any interactions on this map tile
                if(g.map_stories.count(n) > 0) {
                    runInteract(g.map_stories.find(n)->second);
                } else {
                    runRandomMoveEvents();
                }
            }
        } else if(this->n == g.map->player->n) {
            // or if this npc moved into the player's position
            g.encounterInterrupt = true;
            // the npc interrupts the player
            chInterruptsPlayer(this);

        } else if(g.map->charsByPos.count(this->n) > 1) {
            g.ai_encounterInterrupt = this->n;
            /*
              TODO: AI interrupts AIs
            */
        }
    }
    else {
        info("WARNING: tried to Character::move() to invalid index");
    }

    activity = ACTIVITY_MOVE;
    uniform_int_distribution<> dist(-100, 100);
    spendTime( 1000 + dist(*g.rng) );
}

void Character::die(void) {
    cout << "Died: " << name << endl;
    g.map->removeCharacter(this);
}

static void PlaceItemOnMultiGrid(vector<Grid *> *multigrid, Item *item);

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

Button::Button(void) {
    name = NULL;
    pressed = false;
    up = NULL;
    down = NULL;
}

Button::Button(const char *_name) {
    name = _name;
    name_len = 16 + al_get_text_width(g.font, _name);
    pressed = false;
    up = NULL;
    down = NULL;
}

struct WeaponSwitcher : public Widget {
    WeaponSwitcher() { };
    ~WeaponSwitcher() { };

    void mouseDown(void) { };
    void mouseUp(void) { };
    void keyDown(void) { };
    void hoverOver(void) { };

    void press(void) { };
    void draw(void) override;
    void update(void) { };
};

void WeaponSwitcher::draw(void) {
    Item *w = g.map->player->getSelectedWeapon();

    assert(w != NULL);

    // could be cached...
    float weapon_x = pos.x1 + (pos.x2 - w->get_grid_size_x() * Grid::grid_px_x) / 2;
    float weapon_y = pos.y1 + (pos.y2 - w->get_grid_size_y() * Grid::grid_px_y) / 2;

    al_draw_bitmap(w->get_sprite(), weapon_x, weapon_y, 0);
    al_draw_text(g.font, g.color_white, pos.x1 + 8, pos.y1 + 8, 0, w->getName());

    if(w->storage != NULL) {
        char buf[20];
        if(w->storage->items.size() != 0) {
            sprintf(buf, "Ammo: %d", w->storage->items.front()->cur_stack);
        } else {
            sprintf(buf, "Ammo: empty");
        }
        al_draw_text(g.font, g.color_white, pos.x1 + 8, pos.y1 + 2 * g.font_height, 0, buf);
    }
}

struct TimeDisplay : public Widget {
    int tod;
    const char * const time_strings[4] =
        { "Morning",
          "Midday",
          "Afternoon",
          "Nighttime"
        };

    const char *current_time_string;

    TimeDisplay() { current_time_string = ""; };
    ~TimeDisplay() { };

    void mouseDown(void) { };
    void mouseUp(void) { };
    void keyDown(void) { };
    void hoverOver(void) { };

    void press(void) { };
    void draw(void);
    void update(void) { };
    void calculate_tod(void);
};

void TimeDisplay::calculate_tod(void) {
    tod = g.map->player->nextMove % 24000;

    int i = -1;

    if(     tod >     0 && tod <=  9000) i = 0;
    else if(tod >  9000 && tod <= 12000) i = 1;
    else if(tod > 12000 && tod <= 18000) i = 2;
    else if(tod > 18000 && tod <= 24000) i = 3;

    if(i == -1)
        current_time_string = "";
    else
        current_time_string = time_strings[i];
}

void TimeDisplay::draw(void) {
    al_draw_text(g.font, g.color_grey, pos.x1, pos.y1, 0, current_time_string);
}

struct GridSystem : public Widget {
    vector<Grid *> grids;

    // grids for which we can't manually take or place items (the crafting results)
    vector<Grid *> interaction_forbidden;

    Item *held;
    bool auto_move_to_ground;
    Grid *auto_target;

    // when placing an item back if the dropped position is blocked, we need
    // to know if it was originally rotated
    bool was_rotated;

    // fires when an item -may- have been placed or removed
    // used to recompute... stuff
    void (*change)(void);

    // fires when an item is applied to/removed from a hardpoint (used for
    // applying disinfectant to wounds
    void (*applied)(void);
    void (*removed)(void); /* TODO */

    /*
      TODO: work out how to connect signal with parameters
    */
    pair<Grid*, Item*> applied_params;

    GridSystem(void);
    virtual ~GridSystem(void);

    // resets the visibility of the grid sort buttons
    virtual void reset(void);

    void mouseDown(void);
    void mouseUp(void);
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

GridSystem::GridSystem(void) {
    pos.x1 = 100;
    pos.y1 = 0;
    pos.x2 = 1080;
    pos.y2 = 720;

    auto_move_to_ground = false;
    auto_target = NULL;
    held = NULL;
    change = NULL;
    applied = NULL;
    removed = NULL;
}

GridSystem::~GridSystem(void) {
    delete held;
    // info("~GridSystem()");
}

void GridSystem::reset(void) {
    for(auto& grid : grids) {
        for(auto& item : grid->items) {
            if(item->storage != NULL &&
               item->parent->info != NULL &&
               item->parent->info->noGrid == true) {
                item->storage->gsb_displayed = true;
            }
        }
    }
}

static vector<Grid *> *ground_at_player(void);

void GridSystem::keyDown(void) {
    if(g.key == ALLEGRO_KEY_R) {
        if(held != NULL) {
            held->rotate();
            g.hold_off_x = 0;
            g.hold_off_y = 0;
        }
    }
}

void GridSystem::mouseDown(void) {
    gsMouseDownEvent();
    if(change != NULL) change();
}

void GridSystem::mouseUp(void) {
    gsMouseUpEvent();
    if(change != NULL) change();
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

    if(item->storage != NULL)
        item->storage->gsb_displayed = false;
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

    if(auto_target->PlaceItem(item) != NULL) {
        assert(item->parent != NULL);
        item->parent->AddItem(item);
        return;
    }

    if(item->storage != NULL)
        item->storage->gsb_displayed = false;
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
        i += grid->items.size();
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
                if(hardpoint->info->noGrid == true)
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
    // info("~TileMap");
    // tilemap owns characters
    delete player;
    for(auto& character : all_characters) {
        delete character;
    }
    // delete ground item and location vectors and their contents
    for(auto& tile : tiles) {
        if(tile.ground_items != NULL) {
            for(auto& grid : *tile.ground_items)
                delete grid;
            delete tile.ground_items;
        }
        if(tile.locations != NULL) {
            for(auto& loc : *tile.locations)
                delete loc;
            delete tile.locations;
        }
    }
    for(auto&& label : labels)
        free((char*)label.text);
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

    size_x = sx;
    size_y = sy;

    cols = min(sx, c);
    rows = min(sy, r);

    view_px = 0;
    view_py = 0;
    res_px = 0;
    res_py = 0;

    pos.x1 = 100;
    pos.y1 = 0;
    pos.x2 = min(g.display_x, (int)pos.x1 + (size_x + 1) * hex_step_x);
    pos.y2 = min(g.display_y - 150, (int)pos.y1 + (size_y + 1) * hex_step_y);

    max_t = size_x * size_y - 1;

    printf("TileMap: (%f, %f) (%f, %f)\n", pos.x1, pos.y1, pos.x2, pos.y2);

    eco.want_creatures = 2 + max_t / 20;

    bubble.radius = 200;
}

void TileMap::focusOnPlayer(void) {
    focusOn(player->n);
}

void TileMap::focusOn(int n) {
    int p_x = n % size_x;
    int p_y = n / size_x;
    view_x = max(0, p_x - cols/2 + 1);
    view_y = max(0, p_y - rows/2 + 2);
    view_x = max(0, min(view_x, size_x - cols));
    view_y = max(0, min(view_y, size_y - rows));
    resetViewPXPY();
    resetViewXY();
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

    // TODO should this always be a memory bitmap?
    buf = al_create_bitmap(size_x, size_y);
    if(buf == NULL) {
        al_set_new_bitmap_flags(ALLEGRO_MEMORY_BITMAP);
        buf = al_create_bitmap(size_x, size_y);
        al_set_new_bitmap_flags(ALLEGRO_VIDEO_BITMAP);
    }
}

MiniMap::~MiniMap() {
    al_destroy_bitmap(buf);
    // info("~MiniMap()");
}

static void notify(const char *text);

void MiniMap::recreate() {
    if(buf == NULL)
        notify("Couldn't create minimap bitmap");

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
    if(g.map->pos.x1 == 0 || g.map->pos.y1 == 0) {
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
    if(buf == NULL) return;
    al_draw_bitmap(buf, off_x, off_y, 0);
}

void TileMap::mouseToTileXY(int &x, int &y) {
    x = view_x + floor((g.mouse_x - pos.x1 + res_px) / 80);
    int off_y = x % 2 == 0 ? 0 : 20;
    y = view_y + floor((g.mouse_y - 40 - off_y - pos.y1 + res_py) / 40);
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

static void end_turn(void);

// check if the clicked tile is next to the player
// if so, move them there.
void TileMap::mouseDown(void) {
    int clicked_n = mouseToTileN();
    int player_n = player->n;
    int clicked_nearby = -1;

    printf("clicked on n=%d m=(%d, %d) p=((%f, %f) (%f, %f))\n",
           clicked_n, g.mouse_x, g.mouse_y,
           pos.x1, pos.y1, pos.x2, pos.y2);

    if(clicked_n == -1)
        return;

    // LMB - movement
    if(g.mouse_button == 1) {
        if(clicked_n == player_n) {
            notify("Yes, that's you!");
            g.AddMessage("Hee.");
            return;
        }

        for(int dir = 1; dir <= 6; dir++) {
            if(dir_transform(player_n, dir) == clicked_n) {
                clicked_nearby = clicked_n;
            }
        }

        if(clicked_nearby != -1) {
            if(g.map->tile_info[g.map->tiles[clicked_nearby].info_index].blocks_movement == false) {
                player->move(clicked_nearby);
                end_turn();
                player->update_visibility();
            }
        }
    }
    // RMB - focus
    else if(g.mouse_button == 2) {
        focusOn(clicked_n);
    }
}

static bool tile_blocks_los(int n) {
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
    // set line of sight distance based on time of day
    int tod = nextMove % 24000;
    if(tod > 5000 && tod <= 8000) // morning
        current_los_distance = 1;
    else if(tod > 8000 && tod <= 18000) // early day
        current_los_distance = 2;
    else if(tod > 18000 && tod <= 22000) // evening
        current_los_distance = 1;
    else
        current_los_distance = 0; // night

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
        cout << "update_visibility(): Added " << (int)currently_seeing.size() << " tiles" << endl;
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
static int dir_transform(int n, int dir) {
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

    // is it on the screen?
    if(y >= g.map->view_y + g.map->rows ||
       x >= g.map->view_x + g.map->cols ||
       y < g.map->view_y ||
       x < g.map->view_x) {
        return;
    }

    int r_x = x - g.map->view_x;
    int r_y = y - g.map->view_y;

    int off_x = 80 * r_x - g.map->res_px;
    int off_y = n % 2  == 0 ? 0 : 20;
    off_y += 40 * r_y - g.map->res_py;

    al_draw_bitmap(sprite,
                   g.map->pos.x1 + off_x + 25 + offset_x,
                   g.map->pos.y1 + off_y + offset_y, 0);

    if(y + 1 >= g.map->size_y)
        return;

    if(DEBUG_VISIBILITY ||
       g.map->tiles[g.map->size_x * (y + 1) + x].visible == true) {
        g.map->drawTopHalfOfTileAt(x, y + 1);
    }
}

static void toggleMsgLogVisibility(void) {
    if(g.log->visible) {
        g.log->visible = false;
        g.map->pos.y2 += 150;
        g.weapon_switcher->visible = false;
    } else {
        g.log->visible = true;
        g.map->pos.y2 -= 150;
        g.weapon_switcher->visible = true;
    }
}

void TileMap::keyDown(void) {
    if(g.key == ALLEGRO_KEY_C) {
        g.map->focusOnPlayer();
    }
    else if(g.key == ALLEGRO_KEY_M) {
        toggleMsgLogVisibility();
    }
    else if(g.key == ALLEGRO_KEY_SPACE) {
        end_turn();
        player->wait();
    }

    char buf[35];
    snprintf(buf, sizeof(buf),
             "Tile map view: x = %d, y = %d", view_x, view_y);
    cout << buf << endl;
    // g.AddMessage(buf);
}

struct UI {
    vector<Widget *> widgets;

    virtual ~UI() {
        // info("~UI()");
    }

    void mouseDownEvent(void);
    void mouseUpEvent(void);
    void keyDownEvent(void);
    void hoverOverEvent(void);

    void update(void);
    virtual void draw(void);

    void addIndicatorWidgets(void);
    void addLogAndButtons(void);
};

void UI::addIndicatorWidgets(void) {
    widgets.push_back(g.health_indicator);
    widgets.push_back(g.pain_indicator);
    widgets.push_back(g.temperature_indicator);
    widgets.push_back(g.fatigue_indicator);
    widgets.push_back(g.hydration_indicator);
    widgets.push_back(g.satiety_indicator);
    widgets.push_back(g.burden_indicator);
    widgets.push_back(g.time_display);
}

void UI::addLogAndButtons(void) {
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
    widgets.push_back(g.button_sleep);
}

void UI::update(void) {
    for(auto& widget : widgets) {
        widget->update();
    }
}

void UI::mouseDownEvent(void) {
    for(auto& widget : widgets) {
        if(widget->pos.x1 <= g.mouse_x &&
           widget->pos.y1 <= g.mouse_y &&
           widget->pos.x1 + widget->pos.x2 >= g.mouse_x &&
           widget->pos.y1 + widget->pos.y2 >= g.mouse_y) {
            widget->mouseDown();
            if(widget->onMouseDown != NULL)
                widget->onMouseDown();
        }
    }
}

void UI::mouseUpEvent(void) {
    for(auto& widget : widgets) {
        if(widget->pos.x1 <= g.mouse_x &&
           widget->pos.y1 <= g.mouse_y &&
           widget->pos.x1 + widget->pos.x2 >= g.mouse_x &&
           widget->pos.y1 + widget->pos.y2 >= g.mouse_y) {
            widget->mouseUp();
            if(widget->onMouseUp != NULL)
                widget->onMouseUp();
            return;
        }
    }
}

void UI::keyDownEvent(void) {
    for(auto& widget : widgets) {
        if(widget->pos.x1 <= g.mouse_x &&
           widget->pos.y1 <= g.mouse_y &&
           widget->pos.x1 + widget->pos.x2 >= g.mouse_x &&
           widget->pos.y1 + widget->pos.y2 >= g.mouse_y) {
            widget->keyDown();
            if(widget->onKeyDown != NULL)
                widget->onKeyDown();
            return;
        }
    }
}

void UI::hoverOverEvent(void) {
    for(auto& widget : widgets) {
        if(widget->pos.x1 <= g.mouse_x &&
           widget->pos.y1 <= g.mouse_y &&
           widget->pos.x1 + widget->pos.x2 >= g.mouse_x &&
           widget->pos.y1 + widget->pos.y2 >= g.mouse_y) {
            widget->hoverOver();
            return;
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
    if(name != NULL) {
        al_draw_filled_rectangle(pos.x1, pos.y1, pos.x1 + name_len, pos.y1 + 30, g.color_black);
        al_draw_text(g.font, g.color_white, pos.x1 + 8, pos.y1 + 8, 0, name);
    }
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
    notify("Sorting is currently disabled.");
    return;
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

/*
  adapted from code from http://rosettacode.org/wiki/Word_wrap
*/
static vector<string> word_wrap(const char *text, int line_length_px) {
    istringstream words(text);
    string word;
    vector<string> ret;
    size_t line = 0;
    int space_px = al_get_text_width(g.font, " ");

    if(words >> word) {
        ret.resize(1);
        ret[line] += word;
        int space_left = line_length_px - al_get_text_width(g.font, word.c_str());

        while(words >> word) {
            if(space_left < al_get_text_width(g.font, word.c_str()) + space_px) {
                line++;
                ret.resize(line + 1);
                ret[line] += word;
                space_left = line_length_px - al_get_text_width(g.font, word.c_str());
            } else {
                ret[line] += ' ';
                ret[line] += word;
                space_left = space_left - (al_get_text_width(g.font, word.c_str()) + space_px);
            }
        }
    }
    return ret;
}

void Game::AddMessage(string str) {
    if(log == NULL)
        return;

    for(auto&& line : word_wrap(str.c_str(), 790))
        log->lines.push_back(line);
}

void Button::press(void) {
    //    pressed = !pressed;
    //    g.AddMessage("pressed button");
}

Button::~Button(void) {
    // info("~Button()");
}

void MessageLog::draw(void) {
    if(background != NULL) {
        al_draw_bitmap(background, pos.x1, pos.y1, 0);
    }

    int off_y = 8;
    int lines_n = lines.size();
    int start = max(0, lines_n - 9);
    for(int i = start; i < lines_n; i++) {
        al_draw_text(font, g.color_grey3, pos.x1 + 8, pos.y1 + off_y, 0, lines[i].c_str());
        off_y = off_y + 14;
    }

    // ...
    g.weapon_switcher->draw();
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

// static void mkRingM(int n, int m) {
//     for(int i = 1; i <= 6; i++) {
//         for(int j = 0; j < m; j++) {
//             g.map->tiles[n].info_index = 2;
//             n = dir_transform(n, i);
//         }
//     }
// }

void TileMap::generate(void) {
    vector<int8_t> options =
        { 0, 0, 0, 0, // grass
          1, 1, // wood
          2, // city
          3, // swamp
          4, 4, // hill
          5, 5, 5, // dirt
          6, 6 // cracked dirt ground
        };

    uniform_int_distribution<> tile_type_dist(0, options.size() - 1);

    tiles.resize(size_x * size_y);

    for(int i = 0; i <= max_t; i++) {
        tiles[i].visible = false;
        tiles[i].info_index = options.at(tile_type_dist(*g.rng));
        tiles[i].ground_items = NULL;
        tiles[i].locations = NULL;
    }
    // mkRingM(4 * size_x + 5, 1);
    // mkRingM(5 * size_x + 3, 3);
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

        int off_x = (i % cols) * hex_step_x - res_px;
        int off_y = (i + view_x) % 2  == 0 ? 0 : 20;
        off_y = off_y + (hex_step_y * floor(i / cols)) - res_py;

        if(currently_seeing == true) {
            // draw the tile at full brightness
            al_draw_bitmap(tile_info[tiles[t].info_index].sprite,
                           pos.x1 + off_x,
                           pos.y1 + off_y,
                           0);
            if(tiles[t].ground_items != NULL) {
                al_draw_bitmap(g.bitmaps[98],
                               pos.x1 + off_x,
                               pos.y1 + off_y,
                               0);
            }
        }
        else {
            /*
              TODO: this causes allocations? so it might be better
              generate and store tinted bitmaps
            */
            // otherwise draw it 50% tinted
            al_draw_tinted_bitmap(tile_info[tiles[t].info_index].sprite,
                                  g.color_tile_tint,
                                  pos.x1 + off_x,
                                  pos.y1 + off_y, 0);
        }
        if(t == mouse_n) {
            // brighten tile if the mouse is on it
            al_draw_tinted_bitmap(tile_info[tiles[t].info_index].sprite,
                                  g.color_active_tile_tint,
                                  pos.x1 + off_x,
                                  pos.y1 + off_y, 0);
        }
    }
}

void TileMap::draw(void) {
    // resetViewXY();
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
        al_draw_rectangle(pos.x1, pos.y1, pos.x2, pos.y2, g.color_red, 1);
        al_draw_rectangle(pos.x1, pos.y1 + 40, pos.x2 - 81, pos.y2 - 1, g.color_white, 1);
    }

    for(auto&& label : labels)
        label.draw();
}

// redraws the top half of a tile (the part that overlaps with the
// tile above it). Used for occluding character sprites on the map
void TileMap::drawTopHalfOfTileAt(int x, int y) {
    if(y >= g.map->size_y - 1)
        return;

    int n = size_x * y + x;

    int r_x = x - g.map->view_x;
    int r_y = y - g.map->view_y;

    int off_x = 80 * r_x - res_px;
    int off_y = n % 2  == 0 ? 0 : 20;
    off_y += 40 * r_y - res_py;

    if(mouse_n == n) {
        al_draw_tinted_bitmap(tile_info[tiles[n].info_index].sprite,
                                     g.color_active_tile_tint,
                                     pos.x1 + off_x,
                                     pos.y1 + off_y, 0);
    } else {
        al_draw_bitmap_region(tile_info[tiles[n].info_index].sprite,
                              0, 0, 100, 40,
                              pos.x1 + off_x,
                              pos.y1 + off_y, 0);

    }
    // check if there's a character on that tile
    // Character *interloper = characterAt(n);
    // if(interloper != NULL)
    //     interloper->draw();
}

void GridSystem::drawItemTooltip(void) {
    // draw item tooltips
    // this should probably work some other way
    float sx;
    float sy;

    for(auto& grid : grids) {
        for(auto& item : grid->items) {
            sx = item->pos.x2;
            sy = item->pos.y2;
            // item->getHpDims(sx, sy);

            if(item->parent->pos.x1 + item->pos.x1 * Grid::grid_px_x <= g.mouse_x &&
               item->parent->pos.y1 + item->pos.y1 * Grid::grid_px_y <= g.mouse_y &&
               item->parent->pos.x1 + (item->pos.x1 + sx) * Grid::grid_px_x >= g.mouse_x &&
               item->parent->pos.y1 + (item->pos.y1 + sy) * Grid::grid_px_y >= g.mouse_y) {

                if(g.item_info[item->info_index].is_text_item == true)
                    return;

                int weight = g.item_info[item->info_index].weight;
                bool display_weight = weight > 0;
                bool display_condition = item->condition > 0.0;
                float off_y = 24.0;
                float box_height =
                    off_y
                    + (display_weight == true ? g.font_height : 0)
                    + (display_condition == true ? g.font_height : 0) + 8;

                al_draw_filled_rectangle(g.mouse_x + 16, g.mouse_y,
                                         g.mouse_x + 200, g.mouse_y + box_height, g.color_black);

                al_draw_text(g.font, g.color_grey3, g.mouse_x + 24, g.mouse_y + 8,
                             0, g.item_info[item->info_index].name);

                if(display_condition == true) {
                    al_draw_textf(g.font, g.color_grey3, g.mouse_x + 24, g.mouse_y + off_y,
                                  0, "condition: %.1f%%", item->condition * 100);
                    off_y += 14;
                }

                if(display_weight == true) {
                    al_draw_textf(g.font, g.color_grey3, g.mouse_x + 24, g.mouse_y + off_y,
                                  0, "%d g", weight * item->cur_stack);
                    off_y += 14;
                }

                // if the item has a grid, draw it under the text
                if(item->storage != NULL &&
                   (item->parent->info == NULL ||
                    item->parent->info->noGrid == false))
                    // ^^ unless it's on a hardpoint
                    item->storage->drawAt(g.mouse_x + 16, g.mouse_y + box_height);

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
        held->drawHeld();
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
    was_rotated = i->rotated;
    i->setHpDims();
    cout << was_rotated << endl;
    g.hold_off_x =
        g.mouse_x - (i->parent->pos.x1 + i->pos.x1 * Grid::grid_px_x);
    g.hold_off_y =
        g.mouse_y - (i->parent->pos.y1 + i->pos.y1 * Grid::grid_px_y);

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
       // held->parent->info != NULL &&
       // held->parent->info->noGrid == true &&
       (is_correct_slot(g.item_info[held->info_index].slot,
                        held->parent->info) ||
        (held->isClothing() == false &&
         (held->parent->info == g.right_hand_hold ||
          held->parent->info == g.left_hand_hold)))) {
        // ^^ we only want to add it if it's on the correct hardpoint
        // for this item
        // or if it's on the left or right hand, but not if it's clothing
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
        if(grid->info == NULL || grid->info->noGrid == false) {
            // grid
            drop_x = ((g.mouse_x - g.hold_off_x) - grid->pos.x1) / Grid::grid_px_x;
            drop_y = ((g.mouse_y - g.hold_off_y) - grid->pos.y1) / Grid::grid_px_y;
            in_bounds =
                g.mouse_x - g.hold_off_x >= grid->pos.x1 &&
                g.mouse_y - g.hold_off_y >= grid->pos.y1 &&
                g.mouse_x - g.hold_off_x + Grid::grid_px_x * held->pos.x2 <= grid->pos.x2 + 17 &&
                g.mouse_y - g.hold_off_y + Grid::grid_px_y * held->pos.y2 <= grid->pos.y2 + 17;
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
            if(grid->info == NULL || grid->info->noGrid == false) {
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
                            // is this item compatible with the grid?
                            if(item->storage->item_compatible(held) == false)
                                goto blocked;
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
            else if(grid->info->maxItems < (int)grid->items.size() + held->cur_stack) {
                // too many items on the hardpoint
                goto blocked;
            } else {
                // is this item compatible with the grid?
                if(grid->item_compatible(held) == false)
                    goto blocked;
                // if it's a hardpoint, we always drop at 0, 0
                drop_x = 0;
                drop_y = 0;
                // and un-rotate it
                if(held->rotated == true) held->rotate();
                // some hardpoints like shoulders rotate their items
                if(grid->info->pleaseRotateIt == true)
                    held->rotate();
            }

            // is this item compatible with the grid?
            if(grid->item_compatible(held) == false)
                goto blocked;

            // if we put something on a body part, we need to emit a signal
            // so that the logic can take place
            if(grid->info != NULL &&
               grid->info->medical == true) {

                applied_params = make_pair(grid, held);
                if(applied != NULL)
                    applied();

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
            held->pos.x1 = drop_x;
            held->pos.y1 = drop_y;

            grid->items.push_back(held);

            held->setHpDims();
            addStorageGrid();

            char b[60];
            snprintf(b, sizeof(b), "Moved %s onto grid %d at %d, %d",
                     g.item_info[held->info_index].name, i, drop_x, drop_y);
            puts(b);
            // g.AddMessage(b);
            // the item is placed. we're done
            held = NULL;
            return;
        }
        i++;
    }

 blocked:
    // couldn't place it anywhere. send it back to where
    // it was before we picked it up
    if(was_rotated != held->rotated) {
        // rotate back to original orientation
        held->rotate();
    }
    held->parent->items.push_back(held);
    held->setHpDims();

    addStorageGrid();

    held = NULL;

    char b[40];
    snprintf(b, sizeof(b), "Blocked on grid %d", i);
    cout << b;
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
        int16_t moved = g.item_info[held->info_index].maxStack - merge_with->cur_stack;
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
    return;
}

void Grid::draw(void) {
    if(info != NULL && info->visible == false)
        goto draw_items;

    al_draw_filled_rectangle(pos.x1, pos.y1, pos.x2, pos.y2, g.color_grey2);

    if(info == NULL || info->noGrid == false) {
        for (int x = pos.x1 + grid_px_x; x < pos.x2; x = x + grid_px_x) {
            al_draw_line(x, pos.y1, x, pos.y2, g.color_grey3, 1);
        }
        for (int y = pos.y1 + grid_px_y; y < pos.y2; y = y + grid_px_y) {
            al_draw_line(pos.x1, y, pos.x2, y, g.color_grey3, 1);
        }
    }

    if(gsb_displayed == true)
        if(gsb != NULL)
            gsb->draw();

 draw_items:
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
    ALLEGRO_BITMAP *sprite_on_hp = g.item_info[info_index].sprite_on_hp;

    // we're on a grid
    float x1 = parent->pos.x1 + pos.x1 * Grid::grid_px_x;
    float y1 = parent->pos.y1 + pos.y1 * Grid::grid_px_y;
    float x2 = parent->pos.x1 + (pos.x1 + pos.x2) * Grid::grid_px_x;
    float y2 = parent->pos.y1 + (pos.y1 + pos.y2) * Grid::grid_px_y;

    if(sprite == NULL) {
        al_draw_filled_rectangle(x1, y1, x2, y2, g.color_grey3);
        al_draw_rectangle(x1, y1, x2, y2, g.color_black, 1);
    }
    else {
        if(sprite_on_hp != NULL &&
           g.item_info[info_index].grid_size_x != pos.x2) {
            al_draw_bitmap(sprite_on_hp, x1, y1, 0);
        } else {
            if(rotated == true) {
                float h = al_get_bitmap_height(sprite);
                al_draw_rotated_bitmap(sprite, 0, 0, x1+h, y1, ALLEGRO_PI / 2, 0);
            } else {
                al_draw_bitmap(sprite, x1, y1, 0);
            }
        }
    }

    if(cur_stack > 1) {
        // draw number of stacked items
        char buf[4];
        sprintf(buf, "%d", cur_stack);
        if(cur_stack > 9)
            al_draw_text(g.font, g.color_black, x2 - 19, y2 - 15, 0, buf);
        else
            al_draw_text(g.font, g.color_black, x2 - 11, y2 - 15, 0, buf);
    }
}

void Item::drawHeld(void) {
    ALLEGRO_BITMAP *sprite = get_sprite();

    // we're held by the mouse
    float x1 = g.mouse_x - g.hold_off_x;
    float y1 = g.mouse_y - g.hold_off_y;

    if(sprite == NULL) {
        float x2 = g.mouse_x - g.hold_off_x + pos.x2 * 16;
        float y2 = g.mouse_y - g.hold_off_y + pos.y2 * 16;
        al_draw_filled_rectangle(x1, y1, x2, y2, g.color_grey3);
        al_draw_rectangle(x1, y1, x2, y2, g.color_black, 1);
    }
    else {
        if(rotated == true) {
            float h = al_get_bitmap_height(sprite);
            al_draw_rotated_bitmap(sprite, 0, 0, x1+h, y1, ALLEGRO_PI / 2, 0);
        } else {
            al_draw_bitmap(sprite, x1, y1, 0);
        }
    }
}

void EmptyOut(Grid *storage) {
    vector<Grid *> *ground = ground_at_player();

    for(auto&& moving : storage->items) {
        PlaceItemOnMultiGrid(ground, moving);
    }
    storage->items.clear();
}

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
            else if(i->storage != NULL) {
                // if the item has storage, mmb empties onto the ground
                EmptyOut(i->storage);
                return;
            }
            else if(i->isUsable()) {
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
    float x2;
    float y2;
    for (auto& i : items) {

        x2 = i->pos.x2;
        y2 = i->pos.y2;

        if(i->parent->pos.x1 + i->pos.x1 * grid_px_x <= x &&
           i->parent->pos.y1 + i->pos.y1 * grid_px_y <= y &&
           i->parent->pos.x1 + (i->pos.x1 + x2) * grid_px_x >= x &&
           i->parent->pos.y1 + (i->pos.y1 + y2) * grid_px_y >= y) {
            Item *a = items[c];
            items.erase(items.begin() + c);
            return a;
        }
        c++;
    }
    return NULL;
}

/*
  TODO: replace this by changing characters to std::priority_queue ?
  vector: O(n) search, O(1) insert
  priority_queue: O(1) search, O(log(n)) insert
*/
// find the next character that gets to move: it's the character
// with the lowest nextMove value.
static Character *next(void) {
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

// static void chr_debug_print(Character *c) {
//     cout << "AI " << c << " (" << c->nextMove << ") " << c->n << endl;
// }

static void end_turn_debug_print(void) {
    printf("turn ends (%d) with %d characters (%d active). Interrupt: %d\n",
           g.map->player->nextMove,
           (int)g.map->all_characters.size(),
           (int)g.map->characters.size(),
           g.encounterInterrupt
           );

    // g.map->player->print_stats();
}

// process AI's without caring about player encounter interrupts.
// used when player is already in an encounter.
static void processAI(void) {
    Character *c;
    while((c = next()) != g.map->player)
        {
            c->do_AI();
            if(g.ai_encounterInterrupt != -1) {
                runAIEncounter(g.ai_encounterInterrupt);
            }
        }
}

static void player_hurt_messages(void) {
    if(g.map->player->hydration < 0.15) {
        g.AddMessage("Water...");
    } else if(g.map->player->satiety < 0.15) {
        g.AddMessage("Food...");
    } else if(g.map->player->warmth < 0.15) {
        g.AddMessage("Warmth...");
    } else if(g.map->player->fatigue < 0.15) {
        g.AddMessage("Rest...");
    }

    float bleeding = 0;
    for(auto&& w : g.map->player->wounds)
        bleeding += w.bleeding;

    if(bleeding > 0.05)
        g.AddMessage("You're bleeding heavily...");
    else if(bleeding > 0.001)
        g.AddMessage("You're bleeding...");

    if(g.map->player->diseases[DISEASE_COMMON_COLD].duration > 0)
        g.AddMessage("A-choo!");
    if(g.map->player->diseases[DISEASE_FLU].duration > 0)
        g.AddMessage("*Cough*");


    // cout << "bleed damage: " << bleeding << endl;

    // for(auto&& w : g.map->player->wounds)
    //     printf("%d,%f,%f ", w.age, w.severity, w.bleeding);
    // cout << endl << g.map->player->health << endl;
}

static void end_turn() {
    Character *c;

    // process characters until it's the player's turn again or we get
    // an encounter interrupt
    while((c = next()) != g.map->player && g.encounterInterrupt == false && g.ai_encounterInterrupt == -1)
        {
            printf("%s (%d), ", c->name, c->nextMove);

            c->do_AI();
            if(g.ai_encounterInterrupt != -1) {
                runAIEncounter(g.ai_encounterInterrupt);
            }
        }
    puts("\n");

    g.map->player->update_visibility();

    if(g.encounterInterrupt == true &&
       g.ui != (UI*)g.ui_Encounter /* <-- TODO: still needed? */)
        {
            runPlayerEncounter();
        }

    const char *time_string_before = g.time_display->current_time_string;

    g.map->player->update();

    g.time_display->calculate_tod();
    const char *time_string_after = g.time_display->current_time_string;
    if(strcmp(time_string_before, time_string_after) != 0) {
        g.AddMessage("Time changed to " + string(time_string_after));
    }

    if(g.map->player->health < 0.01) {
        g.map->removeCharacter(g.map->player);
    }
    player_hurt_messages();

    g.map->runEcology();

    end_turn_debug_print();
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
        if(g.item_info[moving->info_index].isSkill == false) {
            PlaceItemOnMultiGrid(ground_at_player(), moving);
        }
        ingredients->RemoveItem(moving);
    }
    // remove skills from the ground
    Grid *ground = ground_at_player()->at(current_ground_page);
    vector<Item *>::iterator it;
    for(it = ground->items.begin(); it != ground->items.end();) {
        if(g.item_info[(*it)->info_index].isSkill == true) {
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
static int countItemsOfType(Grid* grid, int searching_for) {
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
    const char *name;
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

static vector<Recipe *> find_craftable_recipe(Grid *ingredients) {
    vector<Recipe *> ret;

    for(auto& recipe : recipes) {
        if(recipe->player_has_ingredients(ingredients) == true)
            ret.push_back(recipe);
    }
    return ret;
}

static void remove_amount_from_grid(Grid *ingredients, int index, int16_t amount) {
    // remove items' stack size
    for(auto& item : ingredients->items) {
        if(item->info_index == index) {
            int16_t removed = min(amount, item->cur_stack);
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

static void create_results(Recipe *recipe) {
    // create and add crafted items to the ground
    for(auto& result : recipe->results) {
        int amount = result.second;
        while(amount > 0) {
            Item *crafted = new Item (result.first);
            /*
              TODO: average condition over ingredient conditions
             */
            crafted->condition = 1.0;
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

static void init_recipes(void) {
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
    // info("~CraftingGridSystem()");
    delete ingredients;
    delete results;
}

static void runCrafting(void);

static void updateCraftingOutput(void) {
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
            crafted->condition = 1.0;
            results->PlaceItem(crafted);
            amount--;
        }
    }
}

static void craftingNextRecipe(void) {
    cout << g.ui_Crafting->current_recipe++ << endl;
    updateCraftingOutput();
}

static void craftingPrevRecipe(void) {
    cout << g.ui_Crafting->current_recipe-- << endl;
    updateCraftingOutput();
}

CraftingUI::CraftingUI() {
    craftGrids = new CraftingGridSystem;
    craftGrids->change = updateCraftingOutput;

    button_prev_recipe = new Button ("Previous recipe");
    button_prev_recipe->pos.x1 = 500;
    button_prev_recipe->pos.y1 = 260;
    button_prev_recipe->pos.x2 = 75;
    button_prev_recipe->pos.y2 = 45;
    button_prev_recipe->up = g.bitmaps[33];
    button_prev_recipe->down = NULL;
    button_prev_recipe->onMouseDown = craftingPrevRecipe;

    button_confirm = new Button ("Commit selected");
    button_confirm->pos.x1 = 580;
    button_confirm->pos.y1 = 260;
    button_confirm->pos.x2 = 75;
    button_confirm->pos.y2 = 45;
    button_confirm->up = g.bitmaps[33];
    button_confirm->down = NULL;
    button_confirm->onMouseDown = runCrafting;

    button_next_recipe = new Button ("Next recipe");
    button_next_recipe->pos.x1 = 660;
    button_next_recipe->pos.y1 = 260;
    button_next_recipe->pos.x2 = 75;
    button_next_recipe->pos.y2 = 45;
    button_next_recipe->up = g.bitmaps[33];
    button_next_recipe->down = NULL;
    button_next_recipe->onMouseDown = craftingNextRecipe;

    current_recipe = 0;
    setup();
}

CraftingUI::~CraftingUI() {
    delete craftGrids;
    delete button_confirm;
    delete button_next_recipe;
    delete button_prev_recipe;
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

    // countTotalItems();
    GridSystem::reset();
}

void CraftingUI::setup(void) {
    widgets.clear();
    widgets.push_back(button_prev_recipe);
    widgets.push_back(button_confirm);
    widgets.push_back(button_next_recipe);
    addLogAndButtons();
    addIndicatorWidgets();

    craftGrids->reset();
    widgets.push_back(craftGrids);
    craftGrids->exit();
}

static void runCrafting(void) {
    Grid *in = g.ui_Crafting->craftGrids->ingredients;
    int selected_recipe = g.ui_Crafting->current_recipe;

    vector<Recipe *> rs = find_craftable_recipe(in);
    if(!rs.empty()) {
        create_results(rs.at(selected_recipe));

        char buf[100];
        snprintf(buf, sizeof(buf), "Crafted %s", rs.at(selected_recipe)->name);
        g.AddMessage(buf);
    }

    updateCraftingOutput();
}

// stats associated with characters but only needed during an
// encounter
struct EncounterCharacter {
    bool visible;
    Item *last_move;

    EncounterCharacter(void);
};

struct Encounter {
private:
    bool running;

    Character *c1;
    Character *c2;
    EncounterCharacter ec1;
    EncounterCharacter ec2;

    // the distance between the characters (approx. [m])
    int range;

    // general
    void advance(int steps);
    void retreat(int steps);
    bool npcRelocated(void);

    // AI vs AI
    void npcEncounterStep(int n);
    bool isEncounterNPCdead(int n);

    // player vs AI
    void endEncounter(void);
    void npcEncounterStep(void);
    bool isEncounterNPCdead(void);
    bool involvesPlayer(void);

public:
    Encounter(void);

    // general
    void setup(Character *c1, Character *c2);

    // AI vs AI
    void runAIEncounter(int n);

    // player vs AI
    void runPlayerEncounterStep(void);
    void do_encounter_messages(void);
    // info needed by EncounterUI
    float getHealth(int n);
    const char *getName(int n);
    const char *getFaction(int n);
    const char *getEquippedWeaponName(int n);
    const Character *getChar(int n);
    const char *getTerrainName(void);
    ALLEGRO_BITMAP *get_character_sprite(int i);
    int getRange(void);
    bool isRunning(void);
    bool playerInRange(void);
    bool seesOpponent(int n);
    void updateVisibility(void);
};

const Character *Encounter::getChar(int n) {
    return n == 0 ? c1 : c2;
}

void Encounter::updateVisibility(void) {
    if(range <= 6) {
        ec1.visible = true;
        ec2.visible = true;
    } else {
        ec1.visible = false;
        ec2.visible = false;
    }
}

struct EncounterGridSystem : public GridSystem {
    Grid *options;
    Grid *selected;

    EncounterGridSystem();
    ~EncounterGridSystem();
};

struct EncounterUI : public UI {
    const int off_x = 97;

    Encounter encounter;

    EncounterGridSystem *encounterGrids;

    ALLEGRO_BITMAP *cur_tile_sprite;
    ALLEGRO_BITMAP *unknown_character_sprite;
    Button *button_confirm;

    Item *flee;
    Item *single_attack;
    Item *retreat;
    Item *advance;

    EncounterUI();
    ~EncounterUI();

    void draw(void) override;

    void setup(void);
};

EncounterCharacter::EncounterCharacter(void) {
    visible = false;
    last_move = NULL;
}

Encounter::Encounter(void) {
    running = false;
}

bool Encounter::seesOpponent(int n) {
    return n == 1 ? ec2.visible : ec1.visible;
}

bool Encounter::isRunning(void) { return running; }

int Encounter::getRange(void) { return range; }

bool Encounter::involvesPlayer(void) { return c1 == g.map->player; }

bool Encounter::npcRelocated(void) {
    return c1->n != c2->n;
}

float Encounter::getHealth(int n) {
    return n == 0 ? c1->health : c2->health;
}

const char *Encounter::getName(int n) {
    return n == 0 ? c1->name : c2->name;
}

const char *Encounter::getFaction(int n) {
    return
        n == 0
        ? faction_to_string(c1->faction)
        : faction_to_string(c2->faction);
}

const char *Encounter::getEquippedWeaponName(int n) {
    return n == 0 ? c1->getSelectedWeapon()->getName() : c2->getSelectedWeapon()->getName();
}

const char *Encounter::getTerrainName(void) {
    return g.map->getTileName(c1->n);
}


ALLEGRO_BITMAP *Encounter::get_character_sprite(int i) {
    if(i == 0)
        return c1->sprite;
    else
        return c2->sprite;
}

bool Encounter::playerInRange(void) {
    Item *weapon = c1->getSelectedWeapon();

    return weapon->get_weapon_range() >= range;
}

void Encounter::advance(int steps) {
    if(steps >= range)
        range = 0;
    else
        range -= steps;
}

void Encounter::retreat(int steps) {
    range += steps;
}

void Encounter::setup(Character *c1, Character *c2) {
    this->c1 = c1;
    this->c2 = c2;
    running = true;
    range = 10;
}

static void runPlayerEncounter(void) {
    g.ui_Encounter->setup();
    g.ui_Encounter->encounter.do_encounter_messages();
    g.ui = g.ui_Encounter;
}

static void runAIEncounter(int n) {
    Encounter e;
    e.runAIEncounter(n);
}

// resolve all AI encounters at pos n
void Encounter::runAIEncounter(int n) {
    int num_there;
    // run encounters until only one character remains
    do {
        // setup
        auto cs = g.map->charsByPos.equal_range(n).first;

        if(cs == g.map->charsByPos.end()) {
            cout << "***********************************" << endl;
            info("runAIEncounter(): No characters?");
            cout << "***********************************" << endl;
            return;
        }
        c1 = cs->second;
        if(cs == g.map->charsByPos.end()) {
            cout << "***********************************" << endl;
            info("runAIEncounter(): Just one character?");
            cout << "***********************************" << endl;
            return;
        }
        cs++;
        c2 = cs->second;

        range = 10;

        printf("Fight!\n");
        printf("%15s\t%15s\n", c1->name, c2->name);

        while(true) {
            npcEncounterStep(0);
            if(isEncounterNPCdead(0) == true || npcRelocated() == true) break;

            npcEncounterStep(1);
            if(isEncounterNPCdead(1) == true || npcRelocated() == true) break;

            printf("%15f\t%15f\n", c1->health, c2->health);
        }
        c1->post_update();
        c2->post_update();

        num_there = g.map->charsByPos.count(n);
    } while(num_there >= 2);
    g.ai_encounterInterrupt = -1;
}

static void button_MainMap_press(void);

void Encounter::endEncounter(void) {
    running = false;
    g.encounterInterrupt = false;

    c2->do_AI(); // this removes the npc if they're dead
    processAI();

    int num_there = g.map->charsByPos.count(g.map->player->n);

    if(num_there > 1) {
        cout << "There are more encounters here: " << num_there - 1 << endl;
        runPlayerEncounter();
    } else {
        cout << "All encounters ended" << endl;
        button_MainMap_press();
        g.map->player->update_visibility();
    }
}

void Encounter::do_encounter_messages(void) {
    if(g.map->player->activity == ACTIVITY_SLEEP)
        g.AddMessage("Your sleep is interrupted and you become aware of an interloper!");
    else if(g.map->player->activity == ACTIVITY_WAIT)
        g.AddMessage("An interloper enters the vicinity!");
    else
        g.AddMessage("There's someone in the vicinity!");
}

// only called by the player
void Encounter::npcEncounterStep(void) {
    npcEncounterStep(0);
}

// c2 acts against c1 (n == 0)
// c1 acts against c2 (n == 1)
void Encounter::npcEncounterStep(int n) {
    updateVisibility();

    Character *_c1 = n == 0 ? c1 : c2;
    Character *_c2 = n == 0 ? c2 : c1;
    char buf[100];

    uniform_int_distribution<> will_dist(0, 10);
    bool wantsToFlee = will_dist(*g.rng) == 0;

    if(wantsToFlee == true) {
        uniform_int_distribution<> fled_dist(0, 2);
        bool successfully_fled = fled_dist(*g.rng) > 0;

        if(successfully_fled == true) {
            if(involvesPlayer() == true) {
                sprintf(buf, "%s flees from you!", _c2->name);
                g.AddMessage("Encounter ends.");
                endEncounter();
            }
            else {
                sprintf(buf, "%s flees from %s!", _c2->name, _c1->name);
                cout << _c2 << " fled successfully" << endl;
            }
            _c2->randomMove();
        } else {
            if(involvesPlayer() == true)
                sprintf(buf, "%s tries to flee but can't!", _c2->name);
            else
                sprintf(buf, "%s tries to flee from %s but can't!", _c2->name, _c1->name);
        }

        if(g.map->playerSees(_c1->n))
            g.AddMessage(buf);
    } else {
        if(_c2->health > 0.8) {
            if(range <= _c2->getSelectedWeapon()->get_weapon_range() && seesOpponent(n) && _c2->hasAmmoForWeapon()) {
                // we're in range, see the opponent and have enough ammo
                bool used = _c2->useWeapon();
                if(used == true) {
                    _c1->hurt(_c2->getSelectedWeapon()->get_weapon_damage());
                    if(involvesPlayer() == true)
                        sprintf(buf, "%s hits you with their %s!", _c2->name, _c2->getSelectedWeapon()->getName());
                    else
                        sprintf(buf, "%s hits %s with their %s!", _c2->name, _c1->name, _c2->getSelectedWeapon()->getName());
                } else {
                    sprintf(buf, "%s fumbled trying to use %s!", _c2->name, _c2->getSelectedWeapon()->getName());
                }
                if(g.map->playerSees(_c1->n))
                    g.AddMessage(buf);

            } else {
                if(involvesPlayer() == true)
                    sprintf(buf, "%s advances on your position!", _c2->name);
                else
                    sprintf(buf, "%s advances toward %s!", _c2->name, _c1->name);

                if(g.map->playerSees(_c1->n))
                    g.AddMessage(buf);

                advance(2);
            }
        } else {
            if(involvesPlayer() == true)
                sprintf(buf, "%s retreats from you!", _c2->name);
            else
                sprintf(buf, "%s retreats from %s!", _c2->name, _c1->name);

            if(g.map->playerSees(_c1->n))
                g.AddMessage(buf);

            retreat(1);
        }
    }
    if(involvesPlayer() == true && _c1->health < 0.01)
        g.map->removeCharacter(g.map->player);
}

bool Encounter::isEncounterNPCdead(void) {
    return isEncounterNPCdead(1);
}

bool Encounter::isEncounterNPCdead(int n) {
    Character *c = n == 0 ? c1 : c2;

    if(c->health < 0.01) {
        if(involvesPlayer() == true) {
            g.AddMessage("The adversary succumbs to their wounds.");
            g.AddMessage("Encounter ends.");
            // g.map->updateCharsByPos();
            endEncounter();
        } else if(g.map->playerSees(c->n)) {
            char buf[100];
            sprintf(buf, "%s dies!", c->name);
            g.AddMessage(buf);
        }
        return true;
    }
    return false;
}

// runs one step of the encounter after the player pressed the
// confirm button. Could in theory accept multiple actions
void Encounter::runPlayerEncounterStep(void) {
    vector<Item *> *actions = &g.ui_Encounter->encounterGrids->selected->items;

    if(actions->empty()) {
        g.AddMessage("Ah! The old human nightmare: endless varieties of stupidity, endless varieties of suffering, endless varieties of banality.");
        return;
    }

    char msg[100];
    string action1 = actions->front()->getName();

    if(action1 == "Flee") {
        uniform_int_distribution<> fled_dist(0, 2);
        bool successfully_fled = fled_dist(*g.rng) > 0;

        if(successfully_fled == true) {
            if(c2->useWeapon() == true)
                c1->hurt(c2->getSelectedWeapon()->get_weapon_damage() / 3);

            c1->randomMove();
            g.AddMessage("You successfully flee from the encounter taking only minor injuries.");
            g.AddMessage("Encounter ends.");
            endEncounter();
            return;
        } else {
            snprintf(msg, sizeof(msg), "You try to run away but %s prevents you!", c2->name);
            g.AddMessage(msg);
            npcEncounterStep();
            if(npcRelocated() == true) return;
        }
    } else if(action1 == "Single attack") {
        if(c1->useWeapon() == true) {
            c2->hurt(c1->getSelectedWeapon()->get_weapon_damage());
            sprintf(msg, "You hit %s with the %s!", c2->name, c1->getSelectedWeapon()->getName());
            g.AddMessage(msg);
        } else {
            g.AddMessage("Couldn't use weapon!");
        }

        if(isEncounterNPCdead() == true) return;
        npcEncounterStep();
        if(npcRelocated() == true) return;
    } else if(action1 == "Retreat") {
        retreat(2);
        npcEncounterStep();
        if(npcRelocated() == true) return;
    } else if(action1 == "Advance") {
        advance(2);
        npcEncounterStep();
        if(npcRelocated() == true) return;
    } else {

    }

    if(isEncounterNPCdead() == true) return;
    if(npcRelocated() == true) return;

    g.ui_Encounter->setup();
}

EncounterGridSystem::EncounterGridSystem() {
    options = new Grid (97 + 105, 300, 16, 10, NULL);
    //                  ^^ g.ui_Encounter->off_x
    selected = new Grid (97 + 398, 300, 16, 10, g.encounter_selected);

    grids.push_back(options);
    grids.push_back(selected);

    auto_target = selected;
}

EncounterGridSystem::~EncounterGridSystem() {
    // info("~EncounterGridSystem()");
    options->items.clear(); // deleted by EncounterUI
    selected->items.clear();
    delete options;
    delete selected;
}

static void runEncounterStepCB(void) {
    g.ui_Encounter->encounter.runPlayerEncounterStep();
}

EncounterUI::EncounterUI() {
    encounterGrids = new EncounterGridSystem;
    flee = new Item ("Flee");
    single_attack = new Item ("Single attack");
    retreat = new Item ("Retreat");
    advance = new Item ("Advance");
    unknown_character_sprite = g.bitmaps[83];

    button_confirm = new Button ("Commit selection");
    button_confirm->pos.x1 = off_x + 691;
    button_confirm->pos.y1 = 300;
    button_confirm->pos.x2 = 75;
    button_confirm->pos.y2 = 45;
    button_confirm->up = g.bitmaps[33];
    button_confirm->down = NULL;
    button_confirm->onMouseDown = runEncounterStepCB;
}

static void button_MainMap_press(void);

void EncounterUI::setup(void) {
    g.color_bg = g.color_grey;

    widgets.clear();
    widgets.push_back(button_confirm);
    widgets.push_back(g.log);
    addIndicatorWidgets();

    // the tile sprite
    cur_tile_sprite = g.map->tile_info[g.map->tiles[g.map->player->n].info_index].sprite;

    if(encounter.isRunning() == false) {
        Character *c1 = g.map->player;
        g.map->updateCharsByPos();
        Character *c2 = g.map->charsByPos.equal_range(g.map->player->n).first->second;

        encounter.setup(c1, c2);
        cout << "Running encounter at: " << c1->n << " with AI " << c2->name << endl;
    }

    encounterGrids->selected->items.clear();
    encounterGrids->options->items.clear();
    widgets.push_back(encounterGrids);

    encounterGrids->options->PlaceItem(flee);
    if(encounter.playerInRange() == true) {
        encounterGrids->options->PlaceItem(single_attack);
    }
    encounterGrids->options->PlaceItem(retreat);
    encounterGrids->options->PlaceItem(advance);

    encounter.updateVisibility();
}

void EncounterUI::draw(void) {
    // g.map->draw();
    // al_draw_filled_rectangle(off_x + 95, 0, off_x + 995, 500, g.color_grey);

    al_draw_filled_rectangle(off_x + 105, 25, off_x + 405, 295, g.color_grey2);
    al_draw_filled_rectangle(off_x + 410, 25, off_x + 680, 295, g.color_grey2);
    al_draw_filled_rectangle(off_x + 685, 25, off_x + 985, 295, g.color_grey2);
    al_draw_text(g.font, g.color_white, off_x + 105, 7, 0,
                 "Zwei Männer, einander in höherer Stellung, vermutend, begegnen sich:");

    char buf[30];

    // left pane
    al_draw_bitmap(encounter.get_character_sprite(0), off_x + 120, 40, 0);
    sprintf(buf, "Name: %s", encounter.getName(0));
    al_draw_text(g.font, g.color_black, off_x + 120, 110, 0, buf);

    if(encounter.seesOpponent(0) == true) {
        al_draw_text(g.font, g.color_black, off_x + 120, 110 + g.font_height, 0,
                     "Visible: yes");
    } else {
        al_draw_text(g.font, g.color_black, off_x + 120, 110 + g.font_height, 0,
                     "Visible: no");
    }

    sprintf(buf, "Weapon: %s", encounter.getEquippedWeaponName(0));
    al_draw_text(g.font, g.color_black, off_x + 120, 110 + 2 * g.font_height, 0, buf);
    // sprintf(buf, "Health: %f", encounter.getHealth(0));
    // al_draw_text(g.font, g.color_black, off_x + 120, 140, 0, buf);

    float wound_severity = 0;
    float wound_bleeding = 0;

    for(auto&& w : encounter.getChar(0)->wounds) {
        wound_severity += w.severity;
        wound_bleeding += w.bleeding;
    }

    if(wound_severity >= 0.001)
        al_draw_text(g.font, g.color_red, off_x + 120, 160, 0, "Wounded");
    if(wound_bleeding >= 0.001)
        al_draw_text(g.font, g.color_red, off_x + 120, 160 + g.font_height, 0, "Bleeding");

    // center pane
    al_draw_bitmap(cur_tile_sprite, off_x + 490, 40, 0);
    sprintf(buf, "Terrain: %s", encounter.getTerrainName());
    al_draw_text(g.font, g.color_black, off_x + 430, 160, 0, buf);
    sprintf(buf, "Range: %d", encounter.getRange());
    al_draw_text(g.font, g.color_black, off_x + 430, 160 + g.font_height, 0, buf);

    // right pane
    if(encounter.seesOpponent(1) == true) {
        al_draw_bitmap(encounter.get_character_sprite(1), off_x + 700, 40, 0);
        sprintf(buf, "Name: %s", encounter.getName(1));
        al_draw_text(g.font, g.color_black, off_x + 700, 110, 0, buf);
        sprintf(buf, "Faction: %s", encounter.getFaction(1));
        al_draw_text(g.font, g.color_black, off_x + 700, 110 + g.font_height, 0, buf);
        al_draw_text(g.font, g.color_black, off_x + 700, 110 + 2 * g.font_height, 0,
                     "visible: yes");
        sprintf(buf, "Weapon: %s", encounter.getEquippedWeaponName(1));
        al_draw_text(g.font, g.color_black, off_x + 700, 110 + 3 * g.font_height, 0, buf);
        // sprintf(buf, "Health: %f", encounter.getHealth(1));
        // al_draw_text(g.font, g.color_black, off_x + 700, 150, 0, buf);

        wound_severity = 0;
        wound_bleeding = 0;
        for(auto&& w : encounter.getChar(1)->wounds) {
            wound_severity += w.severity;
            wound_bleeding += w.bleeding;
        }

        if(wound_severity >= 0.001)
            al_draw_text(g.font, g.color_red, off_x + 700, 170, 0, "Wounded");
        if(wound_bleeding >= 0.001)
            al_draw_text(g.font, g.color_red, off_x + 700, 170 + g.font_height, 0, "Bleeding");

    } else {
        al_draw_bitmap(unknown_character_sprite, off_x + 700, 40, 0);
        al_draw_text(g.font, g.color_black, off_x + 700, 110, 0,
                     "Name: unknown");
        al_draw_text(g.font, g.color_black, off_x + 700, 110 + g.font_height, 0,
                     "Visible: no");
    }

    UI::draw();
}

EncounterUI::~EncounterUI() {
    delete encounterGrids;
    delete button_confirm;
    delete retreat;
    delete advance;
    delete flee;
    delete single_attack;
}

void MessageLog::mouseDown(void) {
    if(g.mouse_x > g.weapon_switcher->pos.x1) {
        g.map->player->switchWeaponHand();

        // ...
        if(g.ui == g.ui_Encounter)
            g.ui_Encounter->setup();
    }
}

struct ScavengeGridSystem : public GridSystem {
    Grid *options;
    Grid *selected;

    ScavengeGridSystem();
    ~ScavengeGridSystem();
};

struct ScavengeUI : public UI {
    Character *player;
    ScavengeGridSystem *gridsystem;
    Button *button_confirm;
    int current_stage;
    unordered_map<Item *, Location *> items_to_locations;
    Item *selected_location;
    vector<Item *> selected_tools;
    vector<Item *> scavenged_items;

    ScavengeUI();
    ~ScavengeUI();

    void draw(void);

    void setup(void);
    void reset(void);

    void runScavengeStep(void);
    void scavengeLocation(void);
    void resetLastLooted(void);
    void addLootedItems(void);
};

void ScavengeUI::draw(void) {
    int off_x = 420;
    int off_y = 365;

    al_draw_bitmap(g.map->getBitmap(player->n), off_x, off_y - 120, 0);
    if(current_stage == 0) {
        if(items_to_locations.empty() == true) {
            al_draw_text(g.font, g.color_white, 105, 365, 0, "There don't seem to be any worthwhile spots to scavenge here at the moment.");
        } else {
            al_draw_text(g.font, g.color_white, 105, 365, 0, "Choose a location to scavenge:");
        }
    } else if(current_stage == 1) {
        al_draw_text(g.font, g.color_white, 105, 365, 0, "Choose what to use during scavenging:");
    } else {
        al_draw_text(g.font, g.color_white, off_x, off_y - 20, 0, "Scavenging results:");

        if(scavenged_items.empty() == true) {
            al_draw_text(g.font, g.color_white, off_x, off_y, 0, "You didn't find any items this time.");
        } else {
            al_draw_text(g.font, g.color_white, off_x, off_y, 0, "You found some items:");
            int i = 0;
            for(auto& item : scavenged_items) {
                al_draw_text(g.font, g.color_white, off_x + 10, off_y + 20 + 10 * i, 0, item->getName());
                i++;
            }
        }
    }

    UI::draw();
}

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

    reset();
}

ScavengeGridSystem::~ScavengeGridSystem() {
    options->items.clear();
    selected->items.clear();
    delete options;
    delete selected;
}

static void runScavenging(void) {
    g.ui_Scavenge->runScavengeStep();
}

void ScavengeUI::scavengeLocation(void) {
    Location *location = items_to_locations.find(selected_location)->second;
    vector<pair<float, int>> loot_table = location->getLootTable();
    uniform_real_distribution<> prob(0, 1);

    for(auto& loot_entry : loot_table) {

        int loot_item_index = loot_entry.second;
        float loot_probability = loot_entry.first;
        float roll = prob(*g.rng);

        if(roll < loot_probability) {
            scavenged_items.push_back(new Item (loot_item_index));
        }
    }
}

void ScavengeUI::resetLastLooted(void) {
    Location *location = items_to_locations.find(selected_location)->second;
    assert(location != NULL);
    location->last_looted = player->nextMove;
}

void ScavengeUI::addLootedItems(void) {
    for(auto& item : scavenged_items) {
        PlaceItemOnMultiGrid(ground_at_player(), item);
    }
}

void ScavengeUI::runScavengeStep(void) {
    cout << "runScavenging() stage: " << current_stage << endl;

    vector<Item *> *selected = &gridsystem->selected->items;


    if(current_stage == 0) {
        if(selected->empty()) {
            if(items_to_locations.empty() == true) {
                // if we haven't selected anything because there's
                // nothing to select, exit
                button_MainMap_press();
            }
            // must select a location
            return;
        }
        selected_location = selected->front();
    }
    else if(current_stage == 1) {
        // after the second step, we picked the tools used for scavenging
        selected_tools = *selected;
        // take location and options, generate items scavenged
        scavengeLocation();
    }
    else if(current_stage == 2) {
        addLootedItems();
        resetLastLooted();
        player->activity = ACTIVITY_SCAVENGE;
        player->spendTime(1000);

        if(runRandomScavengingEvents() == false)
            // if we didn't get an event, manually return to the main map
            button_MainMap_press();

        return;
    } else {
        errorQuit("invalid scavenge step");
    }
    current_stage++;
    setup();
}

ScavengeUI::ScavengeUI() {
    gridsystem = new ScavengeGridSystem;

    button_confirm = new Button ("Commit selection");
    button_confirm->pos.x1 = 855;
    button_confirm->pos.y1 = 380;
    button_confirm->pos.x2 = 75;
    button_confirm->pos.y2 = 45;
    button_confirm->up = g.bitmaps[33];
    button_confirm->down = NULL;
    button_confirm->onMouseDown = runScavenging;
}

ScavengeUI::~ScavengeUI() {
    delete gridsystem;
    delete button_confirm;
}

static vector<Location *> *locations_at_character(Character *character);

void ScavengeUI::setup(void) {
    g.color_bg = g.color_grey;

    widgets.clear();
    widgets.push_back(button_confirm);

    gridsystem->selected->items.clear();
    gridsystem->options->items.clear();

    if(current_stage == 0) {
        scavenged_items.clear();
        items_to_locations.clear();
        player = g.map->player;
        vector<Location *> *locations = locations_at_character(player);

        // add viable locations to the options grid
        for(auto& location : *locations) {
            if(location->last_looted == 0 ||
               location->last_looted + location->getResetTime() < player->nextMove) {
                Item *loc_item = g.location_info[location->info_index].location_item;
                // we add the location items to the options grid
                gridsystem->options->PlaceItem(loc_item);
                // we make a map from location items to locations
                // so runScavenging can know which location we're looting
                // based on the item selected
                items_to_locations.insert({ loc_item, location });
            }
        }
    } else if(current_stage == 1) {
        /*
          TODO: grab scavenging tools from player's inventory and display them here
        */
        // gridsystem->options->PlaceItem(new Item ("crowbar")); // TODO: leak
    } else if(current_stage == 2) {
        // we're done. ScavengeUI::draw is showing what we got
        gridsystem->visible = false;
    }

    addLogAndButtons();
    widgets.push_back(gridsystem);
}

struct InteractGridSystem : public GridSystem {
    Grid *options;
    Grid *selected;

    InteractGridSystem();
    ~InteractGridSystem();
};

InteractGridSystem::~InteractGridSystem() {
    options->items.clear(); // deleted by InteractPage
    selected->items.clear();
    delete options;
    delete selected;
}

struct InteractPage {
    // top left and right panes
    ALLEGRO_BITMAP *left;
    ALLEGRO_BITMAP *right;

    // page description, shown upper left
    vector<char *> description;

    // choices that can be made on this page
    vector<pair<Item *, int>> choices;

    InteractPage();
    ~InteractPage();

    // functions that run when coming into and leaving the page
    void (*pre)(void);
    void (*post)(void);

    void (*frame_draw)(void);
    void (*frame_update)(void);

    // creates and destroys the left bitmap
    void switch_to(void);
    void switch_from(void);

    // add a line to the description
    void tell(const char *line);
    void wrap_and_tell(const char *text);

    void draw_description(void);
    void draw(void);
};

InteractPage::~InteractPage() {
    for(auto&& line : description)
        free(line);

    for(auto&& choice : choices)
        delete choice.first;
}

// an "interact" is a collection of pages, i.e. a short CYOA
struct Interact : public Widget {
    InteractPage *current_page;
    vector<InteractPage *> pages;

    // additional state
    /*
      TODO: how to not leak this
    */
    void *data;

    Interact();
    ~Interact();

    void draw(void) override;
    void update(void) override;
};

Interact::~Interact() {
    for(auto&& page : pages)
        delete page;
    /*
      TODO: delete data
     */
}

struct InteractUI : public UI {
    static const int off_x = 97;
    static const int top_off_y = 40;
    static const int top_size = 300;

    InteractGridSystem *gridsystem;
    Interact *current_interact;
    Button *button_confirm;

    InteractUI();
    ~InteractUI();

    void setup(void);
    void draw(void) override;
};

InteractUI::~InteractUI() {
    delete gridsystem;
    delete button_confirm;
}

InteractGridSystem::InteractGridSystem() {
    options = new Grid (97 + 105, InteractUI::top_off_y
                        + InteractUI::top_size + 5, 16, 10, NULL);
    selected = new Grid (97 + 398, InteractUI::top_off_y
                         + InteractUI::top_size + 5, 16, 10,
                         g.encounter_selected);

    auto_target = selected;

    grids.push_back(options);
    grids.push_back(selected);
}

static void runInteractStepCB(void);

InteractUI::InteractUI(void) {
    gridsystem = new InteractGridSystem;

    button_confirm = new Button ("Commit selection");
    button_confirm->pos.x1 = off_x + 691;
    button_confirm->pos.y1 = top_off_y + top_size + 5;
    button_confirm->pos.x2 = 75;
    button_confirm->pos.y2 = 45;
    button_confirm->up = g.bitmaps[33];
    button_confirm->down = NULL;
    button_confirm->onMouseDown = runInteractStepCB;

    current_interact = NULL;

}

void InteractUI::setup(void) {
    gridsystem->selected->items.clear();
    gridsystem->options->items.clear();

    widgets.clear();

    widgets.push_back(button_confirm);
    widgets.push_back(g.log);
    widgets.push_back(gridsystem);
    widgets.push_back(current_interact);

    addIndicatorWidgets();
    g.color_bg = g.color_grey;
}

Interact::Interact(void) {
    current_page = NULL;
}

InteractPage::InteractPage(void) {
    pre = NULL;
    post = NULL;
    left = NULL;
    frame_draw = NULL;
    frame_update = NULL;
    right = NULL;
}

void InteractPage::tell(const char *line) {
    description.push_back(strdup(line));
}

void InteractPage::wrap_and_tell(const char *text) {
    for(auto&& line : word_wrap(text, 545))
        description.push_back(strdup(line.c_str()));
}


void InteractPage::switch_to(void) {
    left = al_create_bitmap(555, InteractUI::top_size);
    if(left == NULL) {
        al_set_new_bitmap_flags(ALLEGRO_MEMORY_BITMAP);
        left = al_create_bitmap(555, InteractUI::top_size);
        al_set_new_bitmap_flags(ALLEGRO_VIDEO_BITMAP);
        if(left == NULL) {
            info("Couldn't create InteractPage bitmap: left");
            info("Sending text here instead ---");
            for(auto&& line : description)
                cout << line << endl;
            info("--- End of text");
            return;
        }
    }
    draw_description();
}

void InteractPage::switch_from(void) {
    al_destroy_bitmap(left);
}

void InteractPage::draw_description(void) {
    if(left == NULL)
        return;

    al_set_target_bitmap(left);
    al_clear_to_color(g.color_grey2);

    float y = (InteractUI::top_size - g.font_height * description.size()) / 3;
    for(auto&& line : description) {
        al_draw_text(g.font, g.color_black, 5, y, 0, line);
        y += 14;
    }

    al_set_target_backbuffer(g.display);
}

void InteractUI::draw(void) {
    assert(current_interact != NULL);
    al_draw_filled_rectangle(off_x + 595, top_off_y, off_x + 1035,
                             top_off_y + top_size, g.color_grey2);
    UI::draw();
    // current_interact->draw();
}

void Interact::update(void) {
    void (*update_func)(void) = current_page->frame_update;
    if(update_func != NULL) {
        update_func();
    }
}

void Interact::draw(void) {
    assert(current_page != NULL);
    current_page->draw();
}

void InteractPage::draw(void) {
    if(right != NULL) {
        al_draw_bitmap(right, InteractUI::off_x + 600,
                       InteractUI::top_off_y + 5, 0);
    }
    if(left != NULL) {
        al_draw_bitmap(left, InteractUI::off_x + 35,
                       InteractUI::top_off_y, 0);
    }
    if(frame_draw != NULL) {
        frame_draw();
    }
}

static void runInteractStep(InteractPage *x);

static void runInteract(const char *story_name) {
    Interact *x = g.stories.at(story_name);

    g.ui_Interact->current_interact = x;
    g.ui = g.ui_Interact;

    runInteractStep(x->pages.front());
}

static void runInteractStep(InteractPage *x) {
    x->switch_to();

    if(x->pre != NULL) {
        x->pre();
    }

    g.ui_Interact->current_interact->current_page = x;
    g.ui_Interact->setup();
    // add page choices to the options pane
    for(auto&& choice : x->choices) {
        g.ui_Interact->gridsystem->options->PlaceItem(choice.first);
    }
}

static void switchToMainMenu(void);
static void unload_game(void);

static void runInteractStepCB(void) {
    // find the page we're on
    InteractPage *p = g.ui_Interact->current_interact->current_page;

    // find page that the choice switches to
    Item *choice;

    if(g.ui_Interact->gridsystem->selected->items.empty()) {
        // if the player hasn't selected anything
        cout << "runInteractStepCB: nothing selected" << endl;
        if(g.ui_Interact->gridsystem->options->items.size() > 1) {
            // if there's more than one option, don't choose for the player
            return;
        } else {
            // if there's only one option, just select whatever it is
            choice = g.ui_Interact->gridsystem->options->items.front();
        }
    } else {
        // player selected something
        choice = g.ui_Interact->gridsystem->selected->items.front();
    }

    // -1: exit
    // -2: exit to main menu (game over)
    // -3: choice pair not found
    int new_page = -3;
    for(auto&& c : p->choices) {
        if(c.first == choice) {
            new_page = c.second;
            break;
        }
    }
    cout << "Switching to interact page " << new_page << endl;
    // switch page to new page
    g.ui_Interact->setup();

    if(p->post != NULL) {
        p->post();
    }

    if(new_page >= 0) {
        runInteractStep(g.ui_Interact->current_interact->pages.at(new_page));
        p->switch_from();
    } else if(new_page == -1) {
        button_MainMap_press();
    } else if(new_page == -2) {
        delete g.map;
        g.map = NULL;
        switchToMainMenu();
    } else {
        info("Interact Error: couldn't find new interact page index");
        switchToMainMenu();
     }
}

struct test_interact_data {
    struct circle {
        float x, y, r, s;
        const float max_size = 10.0;

        circle() { generate(); }

        void generate(void) {
            uniform_real_distribution<> gx(50.0, 1220.0);
            uniform_real_distribution<> gy(50.0, 670.0);
            uniform_real_distribution<> gr(0.0, max_size - 2.0);
            uniform_real_distribution<> gs(1, 2);
            x = gx(*g.rng);
            y = gy(*g.rng);
            r = gr(*g.rng);
            s = gs(*g.rng);
        }
        void draw(void) { al_draw_filled_circle(x, y, r, g.color_white); }
        void update(void) {
            r += g.dt * s * 3;
            if(r > max_size) { generate(); }
        }
    };

    vector<circle> circles;

    void go(void) { circles.resize(15); }
};

__attribute__ ((unused))
static void frame0_draw_test(void) {
    auto data = (test_interact_data *)g.ui_Interact->current_interact->data;
    for(auto&& c : data->circles) {
        c.draw();
    }
}

__attribute__ ((unused))
static void frame0_update_test(void) {
    auto data = (test_interact_data *)g.ui_Interact->current_interact->data;
    for(auto&& c : data->circles) {
        c.update();
    }
}

static void drop_item_at_player(const char *item_name) {
    PlaceItemOnMultiGrid(ground_at_player(), new Item (item_name));
}

/*
 * Constructs CYOA like stories.
 */
static void init_interactions(void) {
    Interact *intro = new Interact;

    { // "intro"
        // this plays when the game starts
        { // page 0
            Item *opt1 = make_text_item("Explore the field", al_map_rgb(200, 100, 100));
            Item *opt2 = make_text_item("Leave", al_map_rgb(100, 100, 200));
            InteractPage *page = new InteractPage;
            page->right = g.bitmaps[92];
            page->choices.push_back({ opt1, 1 });
            page->choices.push_back({ opt2, -1 });

            page->tell("You wake up.");
            page->tell("");
            page->tell("In front is a desolate field.");
            page->tell("");
            page->wrap_and_tell("Your mind feels like a broken dam as questions form but find no immediate answer. It seems that you have no memory of who you are or of recent events.");
            page->tell("");
            page->tell("You brush off the dirt from your clothes and look around.");

            page->pre = []()
                {
                    drop_item_at_player("pill bottle");
                };
            // page->frame_draw = frame0_draw_test;
            // page->frame_update = frame0_update_test;
            intro->pages.push_back(page);
            intro->current_page = page;
        }

        { // page 1
            Item *opt1 = make_text_item("Leave");
            Item *opt2 = make_text_item("Offer to help", al_map_rgb(100, 150, 100));
            InteractPage *page = new InteractPage;
            page->wrap_and_tell("You walk around the field and come upon a body. It's turned away from you. As you draw closer, the noise of your movement stirs it and it rolls to face you. The hoarse voice of a man obviously in pain yells at you:");
            page->tell("");
            page->tell("\"What are you doing? Get away!\"");

            page->right = g.bitmaps[92];
            page->choices.push_back({ opt1, -1 });
            page->choices.push_back({ opt2, 2 });
            intro->pages.push_back(page);
        }
        { // page 2
            Item *opt1 = make_text_item("Leave");
            InteractPage *page = new InteractPage;
            page->wrap_and_tell("\"Help?\" says the man, \"Are you insane? There's no help for me now\" He stops looking at you and turns his head toward the sky \"Don't you get it? I'm infected. I'll be dead soon, like everyone who gets the virus.\"");
            page->tell("");
            page->tell("He rolls his head again and looks at you. \"You too, probably.\"");
            page->tell("");
            page->wrap_and_tell("\"But since you're here you can take my gear. It should be clean. The virus doesn't live long without the host.\"");
            page->tell("");
            page->tell("\"Now leave me\" He looks toward the sky again.");

            page->right = g.bitmaps[92];
            page->pre = []()
                {
                    drop_item_at_player("backpack");
                    drop_item_at_player("blue jeans");
                    drop_item_at_player("makeshift wood bow");
                    drop_item_at_player("arrow");
                    drop_item_at_player("clean rag");
                    drop_item_at_player("clean rag");
                    drop_item_at_player("clean rag");
                    drop_item_at_player("clean rag");
                    drop_item_at_player("arrow");
                    drop_item_at_player("crowbar");
                    /*
                      TODO: infect with some disease?
                     */
                };
            page->choices.push_back({ opt1, -1 });
            intro->pages.push_back(page);
        }
        intro->data = NULL;
    }

    Interact *fall_down = new Interact;

    { // "fall_down"

        { // page 0
            Item *opt1 = make_text_item("Leave");
            InteractPage *page = new InteractPage;
            page->right = NULL;
            page->choices.push_back({ opt1, -1 });

            page->tell("You trip and fall down!");
            page->tell("");
            page->tell("It hurts, but at least nobody saw you.");

            page->pre = []()
                {
                    uniform_int_distribution<> hurt_dist(0.05, 0.1);
                    g.map->player->hurt(hurt_dist(*g.rng));
                };
            fall_down->pages.push_back(page);
            fall_down->current_page = page;
        }
    }

    Interact *player_dead = new Interact;

    { // "player_dead"

        { // page 0
            Item *opt1 = make_text_item("Poor me", al_map_rgb(0, 0, 0));
            InteractPage *page = new InteractPage;
            page->right = NULL;
            page->choices.push_back({ opt1, -2 });

            page->tell("Like most humans in these cursed times you too have perished.");

            player_dead->pages.push_back(page);
            player_dead->current_page = page;
        }
    }

    Interact *strange_building = new Interact;

    { // "strange_building"

        { // page 0
            Item *opt1 = make_text_item("Enter building", al_map_rgb(200, 100, 100));
            Item *opt2 = make_text_item("Leave", al_map_rgb(100, 100, 100));
            InteractPage *page = new InteractPage;
            page->right = NULL;
            page->choices.push_back({ opt1, 1 });
            page->choices.push_back({ opt2, -1 });

            page->wrap_and_tell("Most buildings in the vicinity are in gross disrepair, but one stands out as newer than the others.");
            page->tell("");
            page->tell("Perhaps there are people living in it?");
            page->pre = []() {
                cout << "hello";
                // mark as seen
            };

            strange_building->pages.push_back(page);
            strange_building->current_page = page;
        }

        { // page 1
            Item *opt1 = make_text_item("Leave", al_map_rgb(100, 100, 100));
            InteractPage *page = new InteractPage;
            page->right = NULL;
            page->choices.push_back({ opt1, -1 });

            page->wrap_and_tell("You enter and explore the building. There doesn't seem to be anything here.");
            page->tell("");
            page->tell("Disappointing, but not unexpected.");

            strange_building->pages.push_back(page);
            strange_building->current_page = page;
        }
    }

    g.stories.emplace("intro", intro);
    g.stories.emplace("fall_down", fall_down);
    g.stories.emplace("player_dead", player_dead);
    g.stories.emplace("strange_building", strange_building);
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
        // info("~VehicleGridSystem()");
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
        delete current_campsite;
        delete available_campsites;
        // info("~CampGridSystem()");
    };

    void reset(void);
    void draw(void) override {
        al_draw_text(g.font, g.color_white, 200, 10, 0, "Ground:");
        al_draw_text(g.font, g.color_white, 600, 10, 0, "Current campsite:");
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
    GridSystem::keyDown();
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
    GridSystem::keyDown();
}

struct InventoryGridSystem : public GridSystem {
    int current_ground_page;

    InventoryGridSystem();
    ~InventoryGridSystem() {
        // info("~InventoryGridSystem()");
    };

    void reset(void);
    void draw(void) {
        al_draw_bitmap(g.bitmaps[45], 480, 70, 0);
        al_draw_text(g.font, g.color_white, 200, 10, 0, "Ground:");
        GridSystem::draw();
    }

    void keyDown(void) override;
};

struct ConditionGridSystem : public GridSystem {
    int current_ground_page;

    ConditionGridSystem();
    ~ConditionGridSystem();

    void draw(void) override;
    void keyDown(void) override;

    void draw_medical_hardpoint(Grid *grid);
    void reset(void);
};

ConditionGridSystem::~ConditionGridSystem() {
    // info("~ConditionGridSystem()");
}

void ConditionGridSystem::draw(void) {
    al_draw_bitmap(g.bitmaps[45], 480, 70, 0);
    al_draw_text(g.font, g.color_white, 200, 10, 0, "Ground:");
    // al_draw_filled_rectangle(700, 50, 1175, 500, g.color_grey);
    // al_draw_text(g.font, g.color_black, 708, 58, 0, "Current conditions:");

    for (auto& g : grids) {
        // skip medical hardpoints
        if(g->info == NULL || g->info->medical == false) {
            g->draw();
        }
    }

    draw_medical_hardpoint(g.map->player->medical_upper_torso);
    draw_medical_hardpoint(g.map->player->medical_lower_torso);
    draw_medical_hardpoint(g.map->player->medical_left_upper_leg);
    draw_medical_hardpoint(g.map->player->medical_right_upper_leg);
    draw_medical_hardpoint(g.map->player->medical_left_lower_leg);
    draw_medical_hardpoint(g.map->player->medical_right_lower_leg);
    draw_medical_hardpoint(g.map->player->medical_left_upper_arm);
    draw_medical_hardpoint(g.map->player->medical_left_lower_arm);
    draw_medical_hardpoint(g.map->player->medical_right_upper_arm);
    draw_medical_hardpoint(g.map->player->medical_right_lower_arm);
    // GridSystem::draw();

    if(held != NULL)
        held->drawHeld();
    else {
        drawItemTooltip();
    }
}

void ConditionGridSystem::draw_medical_hardpoint(Grid *grid) {
    if(grid->items.size() == 0) {
        // TODO: draw wound if we're injured
        if(grid->info == g.medical_upper_torso
           && g.map->player->wounds[WOUND_UPPER_TORSO].severity >= 0.001)
            al_draw_bitmap(g.bitmaps[70], grid->pos.x1, grid->pos.y1, 0);
        else if(grid->info == g.medical_left_lower_leg
                && g.map->player->wounds[WOUND_LEFT_LOWER_LEG].severity >= 0.001)
            al_draw_bitmap(g.bitmaps[71], grid->pos.x1, grid->pos.y1, 0);
        else if(grid->info == g.medical_right_lower_leg
                && g.map->player->wounds[WOUND_RIGHT_LOWER_LEG].severity >= 0.001)
            al_draw_bitmap(g.bitmaps[72], grid->pos.x1, grid->pos.y1, 0);
        else if(grid->info == g.medical_right_upper_leg
                && g.map->player->wounds[WOUND_RIGHT_UPPER_LEG].severity >= 0.001)
            al_draw_bitmap(g.bitmaps[73], grid->pos.x1, grid->pos.y1, 0);
        else if(grid->info == g.medical_left_upper_leg
                && g.map->player->wounds[WOUND_LEFT_UPPER_LEG].severity >= 0.001)
            al_draw_bitmap(g.bitmaps[74], grid->pos.x1, grid->pos.y1, 0);
        else if(grid->info == g.medical_right_lower_arm
                && g.map->player->wounds[WOUND_RIGHT_LOWER_ARM].severity >= 0.001)
            al_draw_bitmap(g.bitmaps[75], grid->pos.x1, grid->pos.y1, 0);
        else if(grid->info == g.medical_right_upper_arm
                && g.map->player->wounds[WOUND_RIGHT_UPPER_ARM].severity >= 0.001)
            al_draw_bitmap(g.bitmaps[76], grid->pos.x1, grid->pos.y1, 0);
        else if(grid->info == g.medical_lower_torso
                && g.map->player->wounds[WOUND_LOWER_TORSO].severity >= 0.001)
            al_draw_bitmap(g.bitmaps[77], grid->pos.x1, grid->pos.y1, 0);
        else if(grid->info == g.medical_left_lower_arm
                && g.map->player->wounds[WOUND_LEFT_LOWER_ARM].severity >= 0.001)
            al_draw_bitmap(g.bitmaps[78], grid->pos.x1, grid->pos.y1, 0);
        else if(grid->info == g.medical_left_upper_arm
                && g.map->player->wounds[WOUND_LEFT_UPPER_ARM].severity >= 0.001)
            al_draw_bitmap(g.bitmaps[79], grid->pos.x1, grid->pos.y1, 0);
        return;
    }

    if(grid->items.front()->info_index == 15) {
        // clean rag applied to body part
        if(grid->info == g.medical_upper_torso)
            al_draw_bitmap(g.bitmaps[60], grid->pos.x1, grid->pos.y1, 0);
        else if(grid->info == g.medical_left_lower_leg)
            al_draw_bitmap(g.bitmaps[61], grid->pos.x1, grid->pos.y1, 0);
        else if(grid->info == g.medical_right_lower_leg)
            al_draw_bitmap(g.bitmaps[62], grid->pos.x1, grid->pos.y1, 0);
        else if(grid->info == g.medical_right_upper_leg)
            al_draw_bitmap(g.bitmaps[63], grid->pos.x1, grid->pos.y1, 0);
        else if(grid->info == g.medical_left_upper_leg)
            al_draw_bitmap(g.bitmaps[64], grid->pos.x1, grid->pos.y1, 0);
        else if(grid->info == g.medical_right_lower_arm)
            al_draw_bitmap(g.bitmaps[65], grid->pos.x1, grid->pos.y1, 0);
        else if(grid->info == g.medical_right_upper_arm)
            al_draw_bitmap(g.bitmaps[66], grid->pos.x1, grid->pos.y1, 0);
        else if(grid->info == g.medical_lower_torso)
            al_draw_bitmap(g.bitmaps[67], grid->pos.x1, grid->pos.y1, 0);
        else if(grid->info == g.medical_left_lower_arm)
            al_draw_bitmap(g.bitmaps[68], grid->pos.x1, grid->pos.y1, 0);
        else if(grid->info == g.medical_left_upper_arm)
            al_draw_bitmap(g.bitmaps[69], grid->pos.x1, grid->pos.y1, 0);
    }
}

static void appliedCB(void) {
    Grid *applied_to = g.ui_Condition->gridsystem->applied_params.first;
    Item *was_applied = g.ui_Condition->gridsystem->applied_params.second;

    printf("Applied %s to %p",
           was_applied->getName(),
           (void*)applied_to);

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
    applied = appliedCB;
    reset();
}

// holy duplication
static void ConditionPrevGroundPage(void) {
    if(g.ui_Condition->gridsystem->current_ground_page > 0) {
        g.ui_Condition->gridsystem->current_ground_page--;
        g.ui_Condition->gridsystem->reset();
    }
}

static void ConditionNextGroundPage(void) {
    vector<Grid *> *ground = ground_at_player();
    if(g.ui_Condition->gridsystem->current_ground_page< (int)(*ground).size()-1) {
        g.ui_Condition->gridsystem->current_ground_page++;
        g.ui_Condition->gridsystem->reset();
    }
}

static void CampPrevGroundPage(void) {
    if(g.ui_Camp->gridsystem->current_ground_page > 0) {
        g.ui_Camp->gridsystem->current_ground_page--;
        g.ui_Camp->gridsystem->reset();
    }
}

static void CampNextGroundPage(void) {
    vector<Grid *> *ground = ground_at_player();
    if(g.ui_Camp->gridsystem->current_ground_page< (int)(*ground).size()-1) {
        g.ui_Camp->gridsystem->current_ground_page++;
        g.ui_Camp->gridsystem->reset();
    }
}

static void InventoryNextGroundPage(void) {
    vector<Grid *> *ground = ground_at_player();
    if(g.ui_Items->gridsystem->current_ground_page< (int)(*ground).size()-1) {
        g.ui_Items->gridsystem->current_ground_page++;
        g.ui_Items->gridsystem->reset();
    }
}

static void InventoryPrevGroundPage(void) {
    if(g.ui_Items->gridsystem->current_ground_page > 0) {
        g.ui_Items->gridsystem->current_ground_page--;
        g.ui_Items->gridsystem->reset();
    }
}

static void VehicleNextGroundPage(void) {
    vector<Grid *> *ground = ground_at_player();
    if(g.ui_Vehicle->gridsystem->current_ground_page< (int)(*ground).size()-1) {
        g.ui_Vehicle->gridsystem->current_ground_page++;
        g.ui_Vehicle->gridsystem->reset();
    }
}

static void VehiclePrevGroundPage(void) {
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
    GridSystem::keyDown();
}

void ConditionGridSystem::keyDown(void) {
    if(g.key == ALLEGRO_KEY_M) {
        InventoryNextGroundPage();
    }
    if(g.key == ALLEGRO_KEY_N) {
        InventoryPrevGroundPage();
    }
    GridSystem::keyDown();
}

VehicleUI::VehicleUI() {
    gridsystem = new VehicleGridSystem;

    ground_next_page = new Button ("Next page");
    ground_next_page->pos.x1 = 485;
    ground_next_page->pos.y1 = 50;
    ground_next_page->pos.x2 = 20;
    ground_next_page->pos.y2 = 20;
    ground_next_page->up = g.bitmaps[29];
    ground_next_page->down = NULL;
    ground_next_page->onMouseDown = VehicleNextGroundPage;

    ground_prev_page = new Button ("Previous page");
    ground_prev_page->pos.x1 = 465;
    ground_prev_page->pos.y1 = 50;
    ground_prev_page->pos.x2 = 20;
    ground_prev_page->pos.y2 = 20;
    ground_prev_page->up = g.bitmaps[30];
    ground_prev_page->down = NULL;
    ground_prev_page->onMouseDown = VehiclePrevGroundPage;

    addLogAndButtons();
    addIndicatorWidgets();
    widgets.push_back(gridsystem);
    widgets.push_back(ground_next_page);
    widgets.push_back(ground_prev_page);
}

CampUI::CampUI() {
    gridsystem = new CampGridSystem;

    ground_next_page = new Button ("Next page");
    ground_next_page->pos.x1 = 485;
    ground_next_page->pos.y1 = 50;
    ground_next_page->pos.x2 = 20;
    ground_next_page->pos.y2 = 20;
    ground_next_page->up = g.bitmaps[29];
    ground_next_page->down = NULL;
    ground_next_page->onMouseDown = CampNextGroundPage;

    ground_prev_page = new Button ("Previous page");
    ground_prev_page->pos.x1 = 465;
    ground_prev_page->pos.y1 = 50;
    ground_prev_page->pos.x2 = 20;
    ground_prev_page->pos.y2 = 20;
    ground_prev_page->up = g.bitmaps[30];
    ground_prev_page->down = NULL;
    ground_prev_page->onMouseDown = CampPrevGroundPage;

    addLogAndButtons();
    addIndicatorWidgets();
    widgets.push_back(gridsystem);
    widgets.push_back(ground_next_page);
    widgets.push_back(ground_prev_page);
}

VehicleUI::~VehicleUI() {
    delete gridsystem;
    delete ground_next_page;
    delete ground_prev_page;
    // info("~VehicleUI()");
}

CampUI::~CampUI() {
    delete gridsystem;
    delete ground_next_page;
    delete ground_prev_page;
    // info("~CampUI()");
}

static void InventoryChangeCallback(void) {
    g.map->player->recomputeCarryWeight();
    g.map->player->recomputeWarmth();
}

ItemsUI::ItemsUI() {
    gridsystem  = new InventoryGridSystem;
    gridsystem->change = InventoryChangeCallback;

    ground_next_page = new Button ("Next page");
    ground_next_page->pos.x1 = 485;
    ground_next_page->pos.y1 = 50;
    ground_next_page->pos.x2 = 20;
    ground_next_page->pos.y2 = 20;
    ground_next_page->up = g.bitmaps[29];
    ground_next_page->down = NULL;
    ground_next_page->onMouseDown = InventoryNextGroundPage;

    ground_prev_page = new Button ("Previous page");
    ground_prev_page->pos.x1 = 465;
    ground_prev_page->pos.y1 = 50;
    ground_prev_page->pos.x2 = 20;
    ground_prev_page->pos.y2 = 20;
    ground_prev_page->up = g.bitmaps[30];
    ground_prev_page->down = NULL;
    ground_prev_page->onMouseDown = InventoryPrevGroundPage;

    addLogAndButtons();
    addIndicatorWidgets();
    widgets.push_back(ground_next_page);
    widgets.push_back(ground_prev_page);
    widgets.push_back(gridsystem);
}

ConditionUI::ConditionUI() {
    gridsystem  = new ConditionGridSystem;

    ground_next_page = new Button ("Next page");
    ground_next_page->pos.x1 = 485;
    ground_next_page->pos.y1 = 50;
    ground_next_page->pos.x2 = 20;
    ground_next_page->pos.y2 = 20;
    ground_next_page->up = g.bitmaps[29];
    ground_next_page->down = NULL;
    ground_next_page->onMouseDown = ConditionNextGroundPage;

    ground_prev_page = new Button ("Previous page");
    ground_prev_page->pos.x1 = 465;
    ground_prev_page->pos.y1 = 50;
    ground_prev_page->pos.x2 = 20;
    ground_prev_page->pos.y2 = 20;
    ground_prev_page->up = g.bitmaps[30];
    ground_prev_page->down = NULL;
    ground_prev_page->onMouseDown = ConditionPrevGroundPage;

    addLogAndButtons();
    addIndicatorWidgets();
    widgets.push_back(ground_next_page);
    widgets.push_back(ground_prev_page);
    widgets.push_back(gridsystem);
}

ItemsUI::~ItemsUI() {
    delete gridsystem;
    delete ground_next_page;
    delete ground_prev_page;
    // info("~ItemsUI()");
}

ConditionUI::~ConditionUI() {
    delete gridsystem;
    delete ground_next_page;
    delete ground_prev_page;
    // info("~ConditionUI()");
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
        // skills items are owned by struct Game
        bad->items.clear();
        good->items.clear();
        delete good;
        delete bad;
        // info("~SkillsGridSystem()");
    };

    void reset(void);
};

static void init_skills(void) {
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

    // countTotalItems();
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
    addLogAndButtons();
    addIndicatorWidgets();
}

SkillsUI::~SkillsUI() {
    delete skillsGrid;
    // info("~SkillsUI()");
}

static void PlaceItemOnMultiGrid(vector<Grid *> *multigrid, Item *item) {
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

// resolve tile to location_info index
static vector<int> tile_to_locations_indexes(Tile t) {
    vector<int> ret;
    /*
      TODO: make helper functions to translate strings to indexes
    */
    switch(t.info_index) {
    case 0: // grass
        // nothing
        break;
    case 1: // wood
        ret.push_back(1); // shack
        break;
    case 2: // city
        ret.push_back(0); // factory
        break;
    case 3: // swamp
        ret.push_back(2); // small lake
        break;
    }
    return ret;
}

// returns, or creates, locations at a tile
static vector<Location *> *locations_at_character(Character *character) {
    // get locations at player position
    assert(character != NULL);
    assert(good_index(character->n) == true);

    Tile char_tile = g.map->tiles[character->n];
    vector<Location *> *locations = char_tile.locations;

    if(locations == NULL) {
        locations = new vector<Location *>;
        assert(locations);
        g.map->tiles[character->n].locations = locations;

        vector<int> idxs = tile_to_locations_indexes(char_tile);

        for(auto& idx : idxs) {
            Location *location = new Location;
            location->info_index = idx;
            location->last_looted = 0;
            locations->push_back(location);
        }
    }

    return locations;
}

static void PlaceStartingGroundItems(__attribute__ ((unused)) Grid *ground, int n) {
    /*
      should there ever be items on the ground without the player
      having scavenged them?
    */
    const char *name = g.map->getTileName(n);

    if(strcmp(name, "Grassland") == 0) {

    } else if(strcmp(name, "Wood") == 0) {
    } else if(strcmp(name, "City") == 0) {
    } else if(strcmp(name, "Swamp") == 0) {
    } else if(strcmp(name, "Hill") == 0) {
    } else if(strcmp(name, "Dirt") == 0) {
    } else if(strcmp(name, "Cracked ground") == 0) {
    } else {
        info("PlaceStartingGroundItems(): invalid tile name");
    }
}

static vector<Grid *> *ground_at_character(Character *character) {
    // get ground inventory at player position
    assert(character != NULL);
    assert(good_index(character->n) == true);
    vector<Grid *> *ground = g.map->tiles[character->n].ground_items;

    if(ground == NULL) {
        ground = new vector<Grid *>;
        assert(ground != NULL);
    }

    if(ground->empty() == true) {
        // create first grid if it doesn't exist
        Grid *ground_grid = new Grid (105, 25, 20, 30, g.ground);
        assert(ground_grid != NULL);

        PlaceStartingGroundItems(ground_grid, character->n);

        ground->push_back(ground_grid);
        g.map->tiles[character->n].ground_items = ground;
    }

    return ground;
}

static vector<Grid *> *ground_at_player(void) {
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

    // countTotalItems();
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

    // countTotalItems();
    GridSystem::reset();
}

void InventoryGridSystem::reset(void) {
    // add player inventory
    grids.clear();
    grids.push_back(g.map->player->right_shoulder);
    grids.push_back(g.map->player->left_shoulder);
    grids.push_back(g.map->player->right_hand_hold);
    grids.push_back(g.map->player->left_hand_hold);
    grids.push_back(g.map->player->right_hand);
    grids.push_back(g.map->player->left_hand);
    grids.push_back(g.map->player->back);
    grids.push_back(g.map->player->head);
    grids.push_back(g.map->player->neck);
    grids.push_back(g.map->player->torso);
    grids.push_back(g.map->player->right_foot);
    grids.push_back(g.map->player->left_foot);
    grids.push_back(g.map->player->legs);

    g.map->player->addInventoryHardpoints(this);

    vector<Grid *> *ground = ground_at_player();
    ground->at(current_ground_page)->gsb_displayed = true;
    grids.push_back(ground->at(current_ground_page));
    // reparent();

    // countTotalItems();
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

    // countTotalItems();
    GridSystem::reset();
}

/*
  Fades out from the `from' UI into the `to' UI

  If from is NULL, it only fades-in
 */
struct FadeTransitionUI : public UI {
    UI *from;
    UI *to;
    const int fade_frames = 30;
    int frame;

    void start(UI *f, UI *t);

    void draw(void) override;
    void updateFrame(void);

    // a widget whose only purpose is to call FadeTransitionUI::updateFrame
    /*
      TODO: is this better than making UI::update virtual?
    */
    struct UpdateCaller : public Widget {
        FadeTransitionUI *parent;
        UpdateCaller() { visible = false; }
        void update(void) override { parent->updateFrame(); }
    };
    UpdateCaller upc;
};

void FadeTransitionUI::start(UI *f, UI *t) {
    from = f;
    to = t;

    if(from != NULL)
        frame = - fade_frames;
    else
        frame = 0;

    upc.parent = this;
    widgets.push_back(&upc);
}

void FadeTransitionUI::draw(void) {
    if(frame <= 0) {
        from->draw();
        float alpha = 1.0 - ((-frame) / (float)fade_frames);
        al_draw_filled_rectangle(0, 0, g.display_x, g.display_y,
                                 al_map_rgba_f(0, 0, 0, alpha));
    } else
    if(frame > 0) {
        to->draw();
        float alpha = 1.0 - (frame / (float)fade_frames);
        al_draw_filled_rectangle(0, 0, g.display_x, g.display_y,
                                 al_map_rgba_f(0, 0, 0, alpha));
    }
}

void FadeTransitionUI::updateFrame(void) {
    frame++;

    if(frame >= fade_frames) {
        widgets.clear();
        g.ui = to;
    }
}

static void fade_to_UI(UI *from, UI *to) {
    g.ui_FadeTransition->start(from, to);
    g.ui = g.ui_FadeTransition;
}

static void fade_to_UI(void) {
    g.ui_FadeTransition->start(NULL, g.ui);
    g.ui = g.ui_FadeTransition;
}

/*
  Blocking notifications

  Construct with notify("message");
*/
struct NotificationUI : public UI {
    Rect pos;
    UI *below;
    // current message
    const char *text;
    // all messages
    queue<const char *> messages;
    float bottom_text_offset_x;
    ALLEGRO_BITMAP *bg;

    NotificationUI();
    void message(const char *text);
    void go(void);

    void draw(void) override;

    void stop(void);

    /*
      runs parent->stop() when you click it
     */
    struct MouseDownCaller : public Widget {
        NotificationUI *parent;
        bool stop;
        MouseDownCaller() { visible = false; stop = false; }
        void mouseDown(void) override { stop = true; }
        void update(void) override { if(stop) parent->stop(); }
    };
    MouseDownCaller mouseDownCaller;
};

NotificationUI::NotificationUI() {
    bg = NULL;
    widgets.push_back(&mouseDownCaller);
}

void NotificationUI::message(const char *text) {
    messages.push(text);
    if(messages.size() == 1)
        go();
}

void NotificationUI::go() {
    if(bg != NULL)
        // we're already in this UI presumably
        goto alreadyThere;

    below = g.ui;

    // otherwise take a screenshot of the current UI
    bg = al_create_bitmap(g.display_x, g.display_y);
    if(bg == NULL) {
        al_set_new_bitmap_flags(ALLEGRO_MEMORY_BITMAP);
        bg = al_create_bitmap(g.display_x, g.display_y);
        al_set_new_bitmap_flags(ALLEGRO_VIDEO_BITMAP);
    }
    if(bg == NULL) {
        bg = little_pink_bitmap();
    }
    if(bg != NULL) {
        al_set_target_bitmap(bg);
        al_clear_to_color(g.color_bg);
        below->draw();
        al_set_target_backbuffer(g.display);
    }

 alreadyThere:
    // center text
    this->text = messages.front();
    pos.x1 = round((g.display_x - 100 - al_get_text_width(g.font, text)) / 2);
    pos.y1 = round((g.display_y - g.font_height - 100) / 2) + 10;
    pos.x2 = al_get_text_width(g.font, text) + 100;
    pos.y2 = 100;
    bottom_text_offset_x = round((pos.x2 - al_get_text_width(g.font, "Click to continue")) / 2);

    mouseDownCaller.stop = false;
    mouseDownCaller.parent = this;
    mouseDownCaller.pos = this->pos;
}

void NotificationUI::draw(void) {
    // tint the background
    if(bg != NULL)
        al_draw_tinted_bitmap(bg, al_map_rgba_f(0.5, 0.5, 0.5, 0.5), 0, 0, 0);

    // draw "the widget"
    al_draw_filled_rectangle(pos.x1, pos.y1, pos.x1 + pos.x2,
                             pos.y1 + pos.y2, g.color_grey2);
    al_draw_rectangle(pos.x1, pos.y1, pos.x1 + pos.x2, pos.y1 + pos.y2,
                      g.color_black, 3);
    al_draw_text(g.font, g.color_black, pos.x1 + 50, pos.y1 + 24, 0, text);
    al_draw_text(g.font, g.color_grey, pos.x1 + bottom_text_offset_x, pos.y1 + 65, 0, "Click to continue");
}

void NotificationUI::stop(void) {
    messages.pop();
    if(messages.empty() == true) {
        al_destroy_bitmap(bg);
        bg = NULL;
        g.ui = below;
    } else {
        // there are more messages to display
        go();
    }
}

static void notify(const char *text) {
    g.ui_Notification->message(text);
    g.ui = g.ui_Notification;
}

vector<ALLEGRO_COLOR> menu_fade;

struct MenuEntry : public Widget {
    const char *name;

    float text_offset_x;
    float text_offset_y;
    size_t fade_level;

    void draw(void) override;
    void update(void) override;

    void hoverOver(void) override;
    void mouseDown(void) override;
};

struct MainMenuUI : public UI {
    const float x = 590;
    const float y = 300;
    const float sx = 100;
    const float sy = 50;

    ALLEGRO_BITMAP *background;
    ALLEGRO_BITMAP *title;
    float title_offset;

    vector <MenuEntry *> entries;

    MainMenuUI();
    ~MainMenuUI();

    void draw(void) override;

    void addEntry(const char *name);
    void handlePress(const char *name);

    void createTitle(void);
    void setFadeColors(void);
    void resetFadeLevels(void);
};

MainMenuUI::~MainMenuUI() {
    for(auto&& entry : entries) delete entry;
    al_destroy_bitmap(title);
}

void MainMenuUI::draw(void) {
    al_draw_bitmap(background, 0, 0, 0);
    if(title != NULL)
        al_draw_bitmap(title, title_offset, 150, 0);
    UI::draw();
}

void MenuEntry::update(void) {
    if(fade_level < menu_fade.size() - 1)
        fade_level++;
}

void MenuEntry::draw(void) {
    al_draw_filled_rectangle(pos.x1, pos.y1, pos.x1 + pos.x2,
                             pos.y1 + pos.y2, menu_fade[fade_level]);
    al_draw_rectangle(pos.x1, pos.y1, pos.x1 + pos.x2, pos.y1 + pos.y2,
                      g.color_black, 1);
    al_draw_text(g.font, g.color_white,
                 pos.x1 + text_offset_x, pos.y1 + text_offset_y, 0, name);
}

void MenuEntry::hoverOver(void) {
    fade_level = 0;

    al_draw_filled_rectangle(pos.x1, pos.y1, pos.x1 + pos.x2,
                             pos.y1 + pos.y2, menu_fade[0]);
    al_draw_rectangle(pos.x1, pos.y1, pos.x1 + pos.x2, pos.y1 + pos.y2,
                      g.color_black, 1);
    al_draw_text(g.font, g.color_white,
                 pos.x1 + text_offset_x, pos.y1 + text_offset_y, 0, name);
}

void MenuEntry::mouseDown(void) {
    g.ui_MainMenu->handlePress(name);
}

static void new_game(void);
static bool save_game(const char *filename);
static bool load_game(const char *filename);
static void button_Help_press(void);

static void load_game_wrapper(void) {
    bool dialog_opened = al_show_native_file_dialog(NULL, g.load_filechooser);
    if(dialog_opened == true) {
        if(al_get_native_file_dialog_count(g.load_filechooser) != 0) {
            bool success = load_game(al_get_native_file_dialog_path(g.load_filechooser, 0));
            if(success == true) {
                g.AddMessage("Game loaded.");
                button_MainMap_press();
            } else {
                notify("Couldn't load game.");
            }
        }
    } else {
        notify("Error: Couldn't open file chooser!");
    }
}

static void save_game_wrapper(void) {
    // show save dialog
    bool dialog_opened = al_show_native_file_dialog(NULL, g.save_filechooser);
    if(dialog_opened == true) {
        // check if user selected a file
        if(al_get_native_file_dialog_count(g.save_filechooser) != 0) {
            // try to save
            bool success = save_game(al_get_native_file_dialog_path(g.save_filechooser, 0));
            if(success == true) {
                notify("Game saved.");
            } else {
                notify("Couldn't save game.");
            }
        }
    } else {
        notify("Error: Couldn't open file chooser!");
    }
}

void MainMenuUI::handlePress(const char *name) {
    if(strcmp(name, "Quit") == 0) {
        g.running = false;
    } else if(strcmp(name, "New") == 0) {
        new_game();
        button_MainMap_press();
        runInteract("intro");
        fade_to_UI(g.ui_MainMenu, g.ui_Interact);
    } else if(strcmp(name, "Continue") == 0) {
        if(g.map != NULL) {
            button_MainMap_press();
        } else {
            load_game_wrapper();
        }
    } else if(strcmp(name, "Save") == 0) {
        if(g.map == NULL) {
            notify("No game in progress.");
        } else {
            save_game_wrapper();
        }
    } else if(strcmp(name, "Load") == 0) {
        load_game_wrapper();
    } else if(strcmp(name, "Help") == 0) {
        button_Help_press();
    } else {
        errorQuit("Uknown menu option selected");
    }
}

void MainMenuUI::setFadeColors(void) {
    menu_fade.resize(25);
    int i = 0;
    for(auto&& color : menu_fade) {
        int level = 175 - i*4;
        color = al_map_rgb(level, level, level);
        i++;
    }
}
void MainMenuUI::resetFadeLevels(void) {
    for(auto&& entry : entries) {
        entry->fade_level = menu_fade.size() - 1;
    }
}

void MainMenuUI::addEntry(const char *name) {
    float offset_y = sy * entries.size();
    MenuEntry *e = new MenuEntry;
    e->name = name;
    e->pos.x1 = x;
    e->pos.y1 = y + offset_y;
    e->pos.x2 = 100;
    e->pos.y2 = sy - 5; // 5px spacing
    e->text_offset_x = round((e->pos.x2 - al_get_text_width(g.font, name)) / 2); // center the text
    // rounded, otherwise al_draw_text produces artifacts
    e->text_offset_y = round((e->pos.y2 - g.font_height) / 2);
    e->fade_level = 24;
    entries.push_back(e);
    widgets.push_back(e);
}

/*
  create the title bitmap containing the game name and version
*/
void MainMenuUI::createTitle(void) {
    const int font_height = 72;
    ALLEGRO_FONT *f = al_load_font(g.font_filename, font_height, 0);
    ALLEGRO_FONT *shadow_font = al_load_font(g.font_filename, font_height + 4, 0);

    assert(f);
    assert(shadow_font);

    const char *title_text = "Project X";

    const int title_text_len = al_get_text_width(f, title_text);
    const int title_shadow_len = al_get_text_width(shadow_font, title_text);
    const int title_version_len = al_get_text_width(g.font, VERSION) + al_get_text_width(g.font, "()");

    title_offset = round((g.display_x - title_shadow_len) / 2);

    float shadow_offset = round((title_shadow_len - title_text_len) / 2);
    float version_offset = round((title_shadow_len - title_version_len) / 2);

    title = al_create_bitmap(title_shadow_len, font_height * 2);
    al_set_target_bitmap(title);
    al_clear_to_color(al_map_rgba(0, 0, 0, 0));

    al_draw_text(shadow_font, g.color_black, 0, 0, 0, title_text);
    al_draw_text(f, g.color_white, shadow_offset, 0, 0, title_text);
    al_draw_textf(g.font, g.color_black, version_offset, font_height + 4 + 20, 0, "(%s)", VERSION);

    al_set_target_backbuffer(g.display);
    al_destroy_font(f);
    al_destroy_font(shadow_font);
}

MainMenuUI::MainMenuUI() {
    background = g.bitmaps[93];
    title = NULL;
    createTitle();
    setFadeColors();

    addEntry("New");
    addEntry("Continue");
    addEntry("Load");
    addEntry("Save");
    // addEntry("Options");
    addEntry("Help");
    addEntry("Quit");
}

struct TextButton : public Widget {
    float text_offset_x;
    float text_offset_y;
    const char *name;

    TextButton(const char *name, float x, float y, float sx, float sy);

    void draw(void) override;
    void hoverOver(void) override;
};

TextButton::TextButton(const char *name, float x, float y, float sx, float sy) {
    this->name = name;
    pos.x1 = x;
    pos.y1 = y;
    pos.x2 = sx;
    pos.y2 = sy;
    text_offset_x = round((pos.x2 - al_get_text_width(g.font, name)) / 2);
    text_offset_y = round((pos.y2 - g.font_height) / 2);
}

void TextButton::draw(void) {
    al_draw_filled_rectangle(pos.x1, pos.y1, pos.x1 + pos.x2,
                             pos.y1 + pos.y2, g.color_darkgrey);
    al_draw_rectangle(pos.x1, pos.y1, pos.x1 + pos.x2, pos.y1 + pos.y2,
                      g.color_black, 1);
    al_draw_text(g.font, g.color_white,
                 pos.x1 + text_offset_x, pos.y1 + text_offset_y, 0, name);
}

void TextButton::hoverOver(void) {
    al_draw_filled_rectangle(pos.x1, pos.y1, pos.x1 + pos.x2,
                             pos.y1 + pos.y2, g.color_grey2);
    al_draw_rectangle(pos.x1, pos.y1, pos.x1 + pos.x2, pos.y1 + pos.y2,
                      g.color_black, 1);
    al_draw_text(g.font, g.color_white,
                 pos.x1 + text_offset_x, pos.y1 + text_offset_y, 0, name);
}

struct HelpUI : public UI {
    ALLEGRO_BITMAP *background;
    TextButton *button_back;

    HelpUI();
    ~HelpUI();

    void draw(void) override;
};

void HelpUI::draw(void) {
    if(background != NULL) {
        al_draw_bitmap(background, 0, 0, 0);
    }
    UI::draw();
}

static void runMainMenu(void);

static void press_Help_back(void) {
    delete g.ui_Help;
    runMainMenu();
}

static ALLEGRO_BITMAP *little_pink_bitmap(void) {
    ALLEGRO_BITMAP *b = al_create_bitmap(32, 32);
    al_set_target_bitmap(b);
    al_clear_to_color(al_color_name("pink"));
    al_set_target_backbuffer(g.display);
    if(b == NULL)
        errorQuit("Couldn't create placeholder 32x32 bitmap");
    return b;
}

HelpUI::HelpUI() {
    button_back = new TextButton("Back", round((g.display_x - 75) / 2), 630, 85, 45);
    button_back->onMouseDown = press_Help_back;
    background = al_load_bitmap("media/backgrounds/help.png");
    if(background == NULL) {
        info("Error: Couldn't load media/backgrounds/help.png");
        background = little_pink_bitmap();
    }
    widgets.push_back(button_back);
}

HelpUI::~HelpUI() {
    delete button_back;
    al_destroy_bitmap(background);
}

static void button_Help_press(void) {
    g.color_bg = g.color_grey;
    g.ui_Help = new HelpUI;
    g.ui = g.ui_Help;
}

static void main_buttons_update(void) {
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

static void switchToMainMenu(void) {
    g.ui_MainMenu->resetFadeLevels();
    g.color_bg = g.color_black;
    g.ui = g.ui_MainMenu;
}

// these could probably be a single function
// update: let's just face it. It's not happening
static void runMainMenu(void) {
    if(g.ui == NULL ||
       g.ui == g.ui_MainMap ||
       g.ui == g.ui_MiniMap ||
       g.ui == g.ui_Help) {
        switchToMainMenu();
    } else {
        info("You can only go to the main menu from the map or minimap");
    }
}

static void button_MainMap_press(void) {
    if(g.ui != g.ui_MainMap) {
        if(g.ui == g.ui_Crafting)
            g.ui_Crafting->craftGrids->exit();
        g.ui = g.ui_MainMap;
        g.color_bg = g.color_black;
    }
    main_buttons_update();
}

static void button_Items_press(void) {
    if(g.ui != g.ui_Items) {
        if(g.ui == g.ui_Crafting)
            g.ui_Crafting->craftGrids->exit();
        g.ui_Items->gridsystem->reset();
        g.ui = g.ui_Items;
        g.color_bg = g.color_grey;
    }
    main_buttons_update();
}

static void button_Vehicle_press(void) {
    if(g.ui != g.ui_Vehicle) {
        if(g.ui == g.ui_Crafting)
            g.ui_Crafting->craftGrids->exit();
        g.ui_Vehicle->gridsystem->reset();
        g.ui = g.ui_Vehicle;
        g.color_bg = g.color_grey;
    }
    main_buttons_update();
}

static void button_MiniMap_press(void) {
    if(g.ui != g.ui_MiniMap) {
        if(g.ui == g.ui_Crafting)
            g.ui_Crafting->craftGrids->exit();
        g.ui = g.ui_MiniMap;
        g.color_bg = g.color_grey;
        g.minimap->recreate();
    }
    main_buttons_update();
}

static void button_Skills_press(void) {
    if(g.ui != g.ui_Skills) {
        if(g.ui == g.ui_Crafting)
            g.ui_Crafting->craftGrids->exit();
        g.ui_Skills->skillsGrid->reset();
        g.ui = g.ui_Skills;
        g.color_bg = g.color_grey;
    }
    main_buttons_update();
}

static void button_Condition_press(void) {
    if(g.ui != g.ui_Condition) {
        if(g.ui == g.ui_Crafting)
            g.ui_Crafting->craftGrids->exit();
        g.ui_Condition->gridsystem->reset();
        g.ui = g.ui_Condition;
        g.color_bg = g.color_grey;
    }
    main_buttons_update();
}

static void button_Camp_press(void) {
    if(g.ui != g.ui_Camp) {
        if(g.ui == g.ui_Crafting)
            g.ui_Crafting->craftGrids->exit();
        g.ui_Camp->gridsystem->reset();
        g.ui = g.ui_Camp;
        g.color_bg = g.color_grey;
    }
    main_buttons_update();
}

static void button_Scavenge_press(void) {
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

static void button_Crafting_press(void) {
    if(g.ui != g.ui_Crafting) {
        g.ui_Crafting->craftGrids->reset();
        g.ui = g.ui_Crafting;
        g.color_bg = g.color_grey;
    }
    main_buttons_update();
}

static void button_endturn_press(void) {
    g.map->player->wait();
    end_turn();
}

static void button_Sleep_press(void) {
    g.map->player->sleep();
    end_turn();
}

static void load_bitmaps(void) {
    vector<const char *> filenames;
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
    /* 45 */ filenames.push_back("media/backgrounds/body.png");
    /* 46 */ filenames.push_back("media/indicators/background.png");
    /* 47 */ filenames.push_back("media/indicators/green_bar.png");
    /* 48 */ filenames.push_back("media/buttons/sleep_up.png");
    /* 49 */ filenames.push_back("media/items/water_bottle.png");
    /* 50 */ filenames.push_back("media/items/red_shirt.png");
    /* 51 */ filenames.push_back("media/items/red_shirt_grid.png");
    /* 52 */ filenames.push_back("media/items/blue_pants.png");
    /* 53 */ filenames.push_back("media/items/blue_pants_grid.png");
    /* 54 */ filenames.push_back("media/items/head_mask.png");
    /* 55 */ filenames.push_back("media/items/head_mask_grid.png");
    /* 56 */ filenames.push_back("media/items/right_hand_glove.png");
    /* 57 */ filenames.push_back("media/items/left_hand_glove.png");
    /* 58 */ filenames.push_back("media/items/right_shoe.png");
    /* 59 */ filenames.push_back("media/items/left_shoe.png");
    /* 60 */ filenames.push_back("media/items/upper_torso_clean_rag.png");
    /* 61 */ filenames.push_back("media/items/lower_left_leg_clean_rag.png");
    /* 62 */ filenames.push_back("media/items/lower_right_leg_clean_rag.png");
    /* 63 */ filenames.push_back("media/items/upper_right_leg_clean_rag.png");
    /* 64 */ filenames.push_back("media/items/upper_left_leg_clean_rag.png");
    /* 65 */ filenames.push_back("media/items/lower_right_arm_clean_rag.png");
    /* 66 */ filenames.push_back("media/items/upper_right_arm_clean_rag.png");
    /* 67 */ filenames.push_back("media/items/lower_torso_clean_rag.png");
    /* 68 */ filenames.push_back("media/items/lower_left_arm_clean_rag.png");
    /* 69 */ filenames.push_back("media/items/upper_left_arm_clean_rag.png");
    /* 70 */ filenames.push_back("media/backgrounds/upper_torso_wound.png");
    /* 71 */ filenames.push_back("media/backgrounds/lower_left_leg_wound.png");
    /* 72 */ filenames.push_back("media/backgrounds/lower_right_leg_wound.png");
    /* 73 */ filenames.push_back("media/backgrounds/upper_right_leg_wound.png");
    /* 74 */ filenames.push_back("media/backgrounds/upper_left_leg_wound.png");
    /* 75 */ filenames.push_back("media/backgrounds/lower_right_arm_wound.png");
    /* 76 */ filenames.push_back("media/backgrounds/upper_right_arm_wound.png");
    /* 77 */ filenames.push_back("media/backgrounds/lower_torso_wound.png");
    /* 78 */ filenames.push_back("media/backgrounds/lower_left_arm_wound.png");
    /* 79 */ filenames.push_back("media/backgrounds/upper_left_arm_wound.png");
    /* 80 */ filenames.push_back("media/items/abstract/retreat.png");
    /* 81 */ filenames.push_back("media/items/abstract/advance.png");
    /* 82 */ filenames.push_back("media/items/hand_combat.png");
    /* 83 */ filenames.push_back("media/characters/unknown.png");
    /* 84 */ filenames.push_back("media/items/makeshift_wood_bow.png");
    /* 85 */ filenames.push_back("media/items/right_shoe_grid.png");
    /* 86 */ filenames.push_back("media/items/left_shoe_grid.png");
    /* 87 */ filenames.push_back("media/tile/hillygrass.png");
    /* 88 */ filenames.push_back("media/tile/dirt.png");
    /* 89 */ filenames.push_back("media/tile/crackedground.png");
    /* 90 */ filenames.push_back("media/characters/dog.png");
    /* 91 */ filenames.push_back("media/characters/char1.png");
    /* 92 */ filenames.push_back("media/backgrounds/story-field1.png");
    /* 93 */ filenames.push_back("media/backgrounds/mainmenubg.png");
    /* 94 */ filenames.push_back("media/characters/char2.png");
    /* 95 */ filenames.push_back("media/characters/char3.png");
    /* 96 */ filenames.push_back("media/items/moldy_bread.png");
    /* 97 */ filenames.push_back("media/items/water.png");
    /* 98 */ filenames.push_back("media/indicators/tile_has_storage.png");

    cout << "Loading bitmaps: ";
    for(auto& filename : filenames) {
        ALLEGRO_BITMAP *bitmap = al_load_bitmap(filename);
        if(bitmap != NULL)
            cout << '.';
        else {
            al_set_new_bitmap_flags(ALLEGRO_MEMORY_BITMAP);
            bitmap = al_load_bitmap(filename);
            al_set_new_bitmap_flags(ALLEGRO_VIDEO_BITMAP);
            if(bitmap == NULL) {
                printf("\n*** ERROR! Failed to load bitmap: %s. If the file exists then it's possible that your graphics card doesn't support textures of its size. Making a 32x32 pink texture to replace it and hope for the best\n", filename);
                bitmap = little_pink_bitmap();
            }
        }
        g.bitmaps.push_back(bitmap);
    }
    cout << " ok" << endl;
}

static void unload_bitmaps(void) {
    for(auto& bitmap : g.bitmaps)
        al_destroy_bitmap(bitmap);
}

static void init_buttons(void) {
    g.button_MainMap   = new Button ("Main map");
    g.button_MiniMap   = new Button ("Mini map");
    g.button_Skills    = new Button ("Skills");
    g.button_Crafting  = new Button ("Crafting");
    g.button_Items     = new Button ("Inventory");
    g.button_Condition = new Button ("Condition");
    g.button_Camp      = new Button ("Camp");
    g.button_Vehicle   = new Button ("Vehicle");
    g.button_endturn   = new Button ("Wait");
    g.button_scavenge  = new Button ("Scavenge");
    g.button_sleep     = new Button ("Sleep");

    int off_y = 200;
    int step = 45;
    // left
    g.button_MainMap->pos.x1 = 0;
    g.button_MainMap->pos.y1 = off_y + 1 * step;
    g.button_MainMap->pos.x2 = 75;
    g.button_MainMap->pos.y2 = 45;
    g.button_MainMap->up = g.bitmaps[0];
    g.button_MainMap->down = g.bitmaps[1];
    g.button_MainMap->onMouseDown = button_MainMap_press;

    g.button_MiniMap->pos.x1 = 0;
    g.button_MiniMap->pos.y1 = off_y + 2 * step;
    g.button_MiniMap->pos.x2 = 75;
    g.button_MiniMap->pos.y2 = 45;
    g.button_MiniMap->up = g.bitmaps[2];
    g.button_MiniMap->down = g.bitmaps[3];
    g.button_MiniMap->onMouseDown = button_MiniMap_press;

    g.button_Skills->pos.x1 = 0;
    g.button_Skills->pos.y1 = off_y + 3 * step;
    g.button_Skills->pos.x2 = 75;
    g.button_Skills->pos.y2 = 45;
    g.button_Skills->up = g.bitmaps[4];
    g.button_Skills->down = g.bitmaps[5];
    g.button_Skills->onMouseDown = button_Skills_press;

    g.button_Crafting->pos.x1 = 0;
    g.button_Crafting->pos.y1 = off_y + 4 * step;
    g.button_Crafting->pos.x2 = 75;
    g.button_Crafting->pos.y2 = 45;
    g.button_Crafting->up = g.bitmaps[6];
    g.button_Crafting->down = g.bitmaps[7];
    g.button_Crafting->onMouseDown = button_Crafting_press;

    // right
    g.button_Items->pos.x1 = 0;
    g.button_Items->pos.y1 = off_y + 5 * step;
    g.button_Items->pos.x2 = 75;
    g.button_Items->pos.y2 = 45;
    g.button_Items->up = g.bitmaps[8];
    g.button_Items->down = g.bitmaps[9];
    g.button_Items->onMouseDown = button_Items_press;

    g.button_Condition->pos.x1 = 0;
    g.button_Condition->pos.y1 = off_y + 6 * step;
    g.button_Condition->pos.x2 = 75;
    g.button_Condition->pos.y2 = 45;
    g.button_Condition->up = g.bitmaps[10];
    g.button_Condition->down = g.bitmaps[11];
    g.button_Condition->onMouseDown = button_Condition_press;

    g.button_Camp->pos.x1 = 0;
    g.button_Camp->pos.y1 = off_y + 7 * step;
    g.button_Camp->pos.x2 = 75;
    g.button_Camp->pos.y2 = 45;
    g.button_Camp->up = g.bitmaps[12];
    g.button_Camp->down = g.bitmaps[13];
    g.button_Camp->onMouseDown = button_Camp_press;

    g.button_Vehicle->pos.x1 = 0;
    g.button_Vehicle->pos.y1 = off_y + 8 * step;
    g.button_Vehicle->pos.x2 = 75;
    g.button_Vehicle->pos.y2 = 45;
    g.button_Vehicle->up = g.bitmaps[14];
    g.button_Vehicle->down = g.bitmaps[15];
    g.button_Vehicle->onMouseDown = button_Vehicle_press;

    g.button_endturn->pos.x1 = 1180;
    g.button_endturn->pos.y1 = 0;
    g.button_endturn->pos.x2 = 100;
    g.button_endturn->pos.y2 = 30;
    g.button_endturn->up = g.bitmaps[25];
    g.button_endturn->down = NULL;
    g.button_endturn->onMouseDown = button_endturn_press;

    g.button_scavenge->pos.x1 = 1180;
    g.button_scavenge->pos.y1 = 30;
    g.button_scavenge->pos.x2 = 100;
    g.button_scavenge->pos.y2 = 30;
    g.button_scavenge->up = g.bitmaps[44];
    g.button_scavenge->down = NULL;
    g.button_scavenge->onMouseDown = button_Scavenge_press;

    g.button_sleep->pos.x1 = 1180;
    g.button_sleep->pos.y1 = 60;
    g.button_sleep->pos.x2 = 100;
    g.button_sleep->pos.y2 = 30;
    g.button_sleep->up = g.bitmaps[48];
    g.button_sleep->down = NULL;
    g.button_sleep->onMouseDown = button_Sleep_press;

    g.main_buttons.push_back(g.button_MainMap);
    g.main_buttons.push_back(g.button_MiniMap);
    g.main_buttons.push_back(g.button_Skills);
    g.main_buttons.push_back(g.button_Crafting);
    g.main_buttons.push_back(g.button_Items);
    g.main_buttons.push_back(g.button_Condition);
    g.main_buttons.push_back(g.button_Camp);
    g.main_buttons.push_back(g.button_Vehicle);
}

static void init_weaponswitcher(void) {
    g.weapon_switcher = new WeaponSwitcher;
    g.weapon_switcher->pos.x1 = 910;
    g.weapon_switcher->pos.y1 = 570;
    g.weapon_switcher->pos.x2 = 270;
    g.weapon_switcher->pos.y2 = 150;
}

static void init_timedisplay(void) {
    g.time_display = new TimeDisplay;
    g.time_display->pos.x1 = 2;
    g.time_display->pos.y1 = 223;
    g.time_display->pos.x2 = 100;
    g.time_display->pos.y2 = 25;
}

__attribute__ ((unused))
static void set_indicators_target(Character *t) {
    g.health_indicator->quantity = &t->health;
    g.pain_indicator->quantity = &t->pain;
    g.temperature_indicator->quantity = &t->warmth;
    g.fatigue_indicator->quantity = &t->fatigue;
    g.hydration_indicator->quantity = &t->hydration;
    g.satiety_indicator->quantity = &t->satiety;
    g.burden_indicator->quantity = &t->burden;
}

static void init_indicators(void) {
    int off_y = 6;
    int space_y = 6;

    g.health_indicator = new BarIndicator;
    g.health_indicator->indicator_name = "Health";
    g.health_indicator->pos.x1 = 0;
    g.health_indicator->pos.y1 = off_y;
    g.health_indicator->pos.x2 = 100;
    g.health_indicator->pos.y2 = 25;
    g.health_indicator->quantity = &g.map->player->health;
    g.health_indicator->up = g.bitmaps[46];
    g.health_indicator->bars = g.bitmaps[47];

    g.pain_indicator = new BarIndicator;
    g.pain_indicator->indicator_name = "Pain";
    g.pain_indicator->pos.x1 = 0;
    g.pain_indicator->pos.y1 = off_y + 25 + space_y * 1;
    g.pain_indicator->pos.x2 = 100;
    g.pain_indicator->pos.y2 = 25;
    g.pain_indicator->quantity = &g.map->player->pain;
    g.pain_indicator->up = g.bitmaps[46];
    g.pain_indicator->bars = g.bitmaps[47];

    g.temperature_indicator = new BarIndicator;
    g.temperature_indicator->indicator_name = "Warmth";
    g.temperature_indicator->pos.x1 = 0;
    g.temperature_indicator->pos.y1 = off_y + 50 + space_y * 2;
    g.temperature_indicator->pos.x2 = 100;
    g.temperature_indicator->pos.y2 = 25;
    g.temperature_indicator->quantity = &g.map->player->warmth;
    g.temperature_indicator->up = g.bitmaps[46];
    g.temperature_indicator->bars = g.bitmaps[47];

    g.fatigue_indicator = new BarIndicator;
    g.fatigue_indicator->indicator_name = "Fatigue";
    g.fatigue_indicator->pos.x1 = 0;
    g.fatigue_indicator->pos.y1 = off_y + 75 + space_y * 3;
    g.fatigue_indicator->pos.x2 = 100;
    g.fatigue_indicator->pos.y2 = 25;
    g.fatigue_indicator->quantity = &g.map->player->fatigue;
    g.fatigue_indicator->up = g.bitmaps[46];
    g.fatigue_indicator->bars = g.bitmaps[47];

    g.hydration_indicator = new BarIndicator;
    g.hydration_indicator->indicator_name = "Hydration";
    g.hydration_indicator->pos.x1 = 0;
    g.hydration_indicator->pos.y1 = off_y + 100 + space_y * 4;
    g.hydration_indicator->pos.x2 = 100;
    g.hydration_indicator->pos.y2 = 25;
    g.hydration_indicator->quantity = &g.map->player->hydration;
    g.hydration_indicator->up = g.bitmaps[46];
    g.hydration_indicator->bars = g.bitmaps[47];

    g.satiety_indicator = new BarIndicator;
    g.satiety_indicator->indicator_name = "Satiety";
    g.satiety_indicator->pos.x1 = 0;
    g.satiety_indicator->pos.y1 = off_y + 125 + space_y * 5;
    g.satiety_indicator->pos.x2 = 100;
    g.satiety_indicator->pos.y2 = 25;
    g.satiety_indicator->quantity = &g.map->player->satiety;
    g.satiety_indicator->up = g.bitmaps[46];
    g.satiety_indicator->bars = g.bitmaps[47];

    g.burden_indicator = new BarIndicator;
    g.burden_indicator->indicator_name = "Burden";
    g.burden_indicator->pos.x1 = 0;
    g.burden_indicator->pos.y1 = off_y + 150 + space_y * 6;
    g.burden_indicator->pos.x2 = 100;
    g.burden_indicator->pos.y2 = 25;
    g.burden_indicator->quantity = &g.map->player->burden;
    g.burden_indicator->up = g.bitmaps[46];
    g.burden_indicator->bars = g.bitmaps[47];
}

static void init_messagelog(void) {
    g.log = new(MessageLog);
    g.log->pos.x1 = 100;
    g.log->pos.y1 = 570;
    g.log->pos.x2 = 1080;
    g.log->pos.y2 = 200;
    g.log->background = g.bitmaps[16];
    g.log->font = g.font;
}

enum TileKind {
    TILE_GRASS = 0,
    TILE_WOOD,
    TILE_CITY,
    TILE_SWAMP,
    TILE_HILL,
    TILE_DIRT,
    TILE_CRACKED_DIRT
};

static void init_tileinfo(void) {
    g.map = new TileMap (g.tilemap_sx, g.tilemap_sy, 16, 16);

    TileInfo i;
    // grass
    i.minimap_color = al_map_rgb(0, 255, 0);
    i.sprite = g.bitmaps[17];
    i.blocks_los = false;
    i.blocks_movement = false;
    i.has_locations = false;
    i.name = "Grassland";
    g.map->tile_info.push_back(i);
    // tree
    i.minimap_color = al_map_rgb(0, 150, 0);
    i.sprite = g.bitmaps[18];
    i.blocks_los = true;
    i.blocks_movement = false;
    i.has_locations = true;
    i.name = "Wood";
    g.map->tile_info.push_back(i);
    // city
    i.minimap_color = al_map_rgb(255, 255, 255);
    i.sprite = g.bitmaps[19];
    i.blocks_los = true;
    i.blocks_movement = false;
    i.has_locations = true;
    i.name = "City";
    g.map->tile_info.push_back(i);
    // swamp
    i.minimap_color = al_map_rgb(0, 0, 200);
    i.sprite = g.bitmaps[23];
    i.blocks_los = false;
    i.blocks_movement = false;
    i.has_locations = true;
    i.name = "Swamp";
    g.map->tile_info.push_back(i);
    // Hilly grass
    i.minimap_color = al_map_rgb(0, 150, 0);
    i.sprite = g.bitmaps[87];
    i.blocks_los = true;
    i.blocks_movement = false;
    i.has_locations = false;
    i.name = "Hill";
    g.map->tile_info.push_back(i);
    // Dirt
    i.minimap_color = al_map_rgb(150, 75, 0);
    i.sprite = g.bitmaps[88];
    i.blocks_los = false;
    i.blocks_movement = false;
    i.has_locations = true;
    i.name = "Dirt";
    g.map->tile_info.push_back(i);
    // crackedground
    i.minimap_color = al_map_rgb(125, 50, 0);
    i.sprite = g.bitmaps[89];
    i.blocks_los = false;
    i.blocks_movement = false;
    i.has_locations = true;
    i.name = "Cracked ground";
    g.map->tile_info.push_back(i);
}

static void init_tilemap(void) {
    init_tileinfo();
    g.map->generate();
}

static void init_minimap(void) {
    g.minimap = new(MiniMap);
}

MiniMapUI::MiniMapUI(void) {
    widgets.push_back(g.minimap);

    addLogAndButtons();
    addIndicatorWidgets();
}

MiniMapUI::~MiniMapUI(void) {
    // info("~MiniMapUI()");
}

static void init_map_stories(void) {
    g.map_stories.clear();
    g.map_stories.emplace(g.map->player->n + 3, "strange_building");
}

static void init_player(void) {
    Character *c = new Character;

    c->name = strdup("Player");
    c->nextMove = 9000;
    c->faction = FACTION_PLAYER;

    for(auto&& fac_rep : c->faction_reps) fac_rep = -1;
    c->faction_reps[c->faction] = 1;

    // start the player in the upper left
    c->setPos(g.map->size_x * (g.map->size_y / 5) + (g.map->size_x / 4));

    c->sprite = g.bitmaps[21];

    // start the player with clothing
    c->torso->PlaceItem(new Item ("red hoodie"));
    c->legs->PlaceItem(new Item ("blue jeans"));
    c->right_foot->PlaceItem(new Item ("right shoe"));
    c->left_foot->PlaceItem(new Item ("left shoe"));

    c->enableSkill(0);
    c->enableSkill(1);
    c->enableSkill(32);
    c->enableSkill(33);
    c->recomputeCarryWeight();
    c->recomputeWarmth();
    c->health = 1;
    c->active = true; // doesn't do anything

    g.map->player = c;

    g.map->tiles[c->n].info_index = TILE_DIRT;
    g.map->addLabel(c->n, strdup("Dead Field"));
    g.map->tiles[c->n + 3].info_index = TILE_CITY;
    g.map->addLabel(c->n + 3, strdup("Strange Building"));
}

static char *random_human_NPC_name(void) {
    vector<string> first_names =
        { "Herb", "Jepson", "Farley", "Homer", "Eustace", "Piers",
          "Sonnie", "Alycia", "Suzie", "Lilibeth", "Nat", "Keanna",
          "Jordyn", "Cary" };
    vector<string> last_names =
        { "Bert", "Parker", "Rigby", "Brooke", "Chamberlain", "Moore",
          "Aitken", "Burke", "Martinson", "Beasley", "Corra", "Lowe",
          "Auttenberg", "Lamar" };

    uniform_int_distribution<> fnd(0, first_names.size() - 1);
    uniform_int_distribution<> lnd(0, last_names.size() - 1);

    string name = first_names.at(fnd(*g.rng)) + " " + last_names.at(lnd(*g.rng));
    return strdup(name.c_str());
}

// creates the player and npcs
static void init_characters(void) {
    // must be called after init_tilemap();
    assert(g.map != NULL);

    init_player();

    const int n_start_npcs = g.map->eco.want_creatures;

    cout << "Spawning " << n_start_npcs << " NPCs" << endl;

    for(int i = 0; i < n_start_npcs; i++) {
        g.map->addRandomCharacter();
    }

    // init map stuff
    g.map->updateActiveCharacters();
    g.map->updateCharsByPos();
    g.map->player->update_visibility();
    g.map->focusOnPlayer();

    init_map_stories();
}

MainMapUI::MainMapUI() {
    widgets.push_back(g.map);
    addLogAndButtons();
    addIndicatorWidgets();
}

MainMapUI::~MainMapUI(void) {
    // info("~MainMapUI()");
}

static void init_colors(void) {
    g.color_grey = al_color_name("grey");
    g.color_grey2 = al_map_rgb(200, 200, 200);
    g.color_grey3 = al_map_rgb(220, 220, 220);
    g.color_darkgrey = al_map_rgb(100, 100, 110);
    g.color_black = al_map_rgb(0, 0, 0);
    g.color_white = al_map_rgb(255, 255, 255);
    g.color_red = al_map_rgb(200, 0, 0);
    g.color_tile_tint = al_map_rgba_f(0.5, 0.5, 0.5, 1.0);
    g.color_active_tile_tint = al_map_rgba_f(1, 1, 1, 0.2);
    g.color_bg = g.color_black;
}

static void init_misc(void) {
    g.hand_combat = new Item ("fist");
    g.ai_encounterInterrupt = -1;
}

static void allegro_init(void) {
    int ret = 0;

    al_init(); // no return value
    if(al_is_system_installed() == false)
        errorQuit("Failed to initialize core allegro library.");
    else
        info("Initialized core allegro library.");

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

    ret = al_init_ttf_addon();
    if(ret == false)
        errorQuit("Failed to initialize allegro addon: ttf.");
    else
        info("Initialized allegro addon: ttf.");

    ret = al_init_native_dialog_addon();
    if(ret == false)
        errorQuit("Failed to initialize allegro addon: native dialogs.");
    else
        info("Initialized allegro addon: native dialogs.");

    /*
      TODO: how do the first and third arguments work?
    */
    g.save_filechooser = al_create_native_file_dialog(NULL, "save game", "*.sav",
                                                      ALLEGRO_FILECHOOSER_SAVE);
    g.load_filechooser = al_create_native_file_dialog(NULL, "load game", "*.sav",
                                                      ALLEGRO_FILECHOOSER_FILE_MUST_EXIST);

    if(g.save_filechooser == NULL)
        errorQuit("Failed to initialize save game filechooser");

    if(g.load_filechooser == NULL)
        errorQuit("Failed to initialize load game filechooser");

    g.display = al_create_display(g.display_x, g.display_y);

    if(g.display == NULL)
        errorQuit("Failed to create display.");
    else
        info("Created display.");

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
}

static void load_fonts(void) {
    g.font = al_load_font(g.font_filename, g.font_height, 0);
    if(g.font == NULL)
        errorQuit("failed to load font: " + string(g.font_filename));
    else
        info("Loaded font: " + string(g.font_filename));
}

static void init_rng(int seed) {
    if(seed != -1) {
        info("Using seed: " + to_string(seed));
        g.rng = new mt19937(seed);
    }
    else {
        random_device rd;
        g.rng = new mt19937(rd());
    }

    if(g.rng == NULL)
        errorQuit("Failed to initialize random number generator");
}

static void init_args(int argc, char **argv, int *seed) {

    if(argc >= 2) {
        try {
            *seed = std::stoi(argv[1]);
        }
        catch (exception &e) {
            info("WARNING: couldn't parse seed argument");
            *seed = -1;
        }
    } else {
        *seed = -1;
    }
    if(argc >= 4) {
        try {
            g.tilemap_sx = std::stoi(argv[2]);
            g.tilemap_sy = std::stoi(argv[3]);
        }
        catch (exception &e) {
            info("WARNING: couldn't parse tilemap dimension arguments");
            g.tilemap_sx = 150;
            g.tilemap_sy = 150;
        }
    } else {
        g.tilemap_sx = 150;
        g.tilemap_sy = 150;
    }
}

struct PerfTimer {
    double start_time;
    const char *name;

    PerfTimer(const char *name) {
        this->name = name;
        start_time = al_current_time();
    }
    ~PerfTimer() {
        printf("%s elapsed time: %fs\n", name, al_current_time() - start_time);
    }
};

// delets all the things that are created by new_game and load_game
static void unload_game(void) {
    if(g.minimap == NULL)
        return;
    delete g.health_indicator;
    delete g.pain_indicator;
    delete g.temperature_indicator;
    delete g.fatigue_indicator;
    delete g.hydration_indicator;
    delete g.satiety_indicator;
    delete g.burden_indicator;
    delete g.ui_MainMap;
    delete g.ui_MiniMap;
    delete g.ui_Items;
    delete g.ui_Vehicle;
    delete g.ui_Encounter;
    delete g.ui_Crafting;
    delete g.ui_Skills;
    delete g.ui_Condition;
    delete g.ui_Camp;
    delete g.ui_Scavenge;
    delete g.ui_Interact;
    // the map may already be unloaded if the player dies
    if(g.map != NULL) {
        delete g.map;
        g.map = NULL;
    }
    delete g.log;
    delete g.minimap;
    g.minimap = NULL;
}

static void init_UIs(void) {
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
    g.ui_Interact  = new InteractUI;

    g.time_display->calculate_tod();
}

static void new_game(void) {
    PerfTimer t("New game");

    unload_game();

    init_tilemap();
    init_characters();
    init_minimap();
    init_messagelog();
    init_indicators();

    init_UIs();
}

void Disease::save(ostream &os) {
    os << duration << ' ' << vulnerability << ' ';
}

void Disease::load(istream &is) {
    is >> duration >> vulnerability;
}

void Label::save(ostream &os) {
    os << n << ' ' << x << ' ' << y << ' ' << offset_x << ' ';
    os << strlen(text) << ' ' << text << ' ';
}

void Label::load(istream &is) {
    is >> n >> x >> y >> offset_x;
    int text_len;
    is >> text_len;
    char *text = (char *)malloc(text_len + 1);
    is.ignore(1); // ignore the space
    is.read(&text[0], text_len);
    text[text_len] = '\0';
    this->text = text;
}

static int find_bitmap_index(ALLEGRO_BITMAP *searched) {
    size_t i = 0;
    for(auto&& bitmap : g.bitmaps) {
        if(bitmap == searched)
            return i;
        i++;
    }
    return -1;
}

void Item::save(ostream &os) {
    os << pos.x1 << ' ' << pos.y1 << ' ' << pos.x2 << ' ' << pos.y2 << ' ';
    os << info_index << ' ' << cur_stack << ' ' << rotated << ' ';
    os << condition << ' ';
    if(storage != NULL) {
        os << true << ' ';
        storage->save(os);
    } else {
        os << false << ' ';
    }
}

void Item::load(istream &is) {
    is >> pos.x1 >> pos.y1 >> pos.x2 >> pos.y2;
    is >> info_index >> cur_stack >> rotated;
    is >> condition;
    bool has_storage;
    is >> has_storage;
    if(has_storage == true) {
        storage = new Grid;
        storage->load(is);
    } else {
        storage = NULL;
    }
}

static int find_gridinfo_index(GridInfo *info) {
    if(info == NULL)
        return -1;
    int i = 0;
    for(auto&& grid_info : g.gridinfo_store) {
        if(info == grid_info)
            return i;
        i++;
    }
    return -1;
}

void Grid::save(ostream &os) {
    os << pos.x1 << ' ' << pos.y1 << ' ' << pos.x2 << ' ' << pos.y2 << ' ';
    os << (int)grid_size_x << ' ' << (int)grid_size_y << ' ';
    os << find_gridinfo_index(info) << ' ';
    os << (int)items.size() << ' ';
    for(auto&& item : items) {
        item->save(os);
    }
}

void Grid::load(istream &is) {
    int gx, gy;
    is >> pos.x1 >> pos.y1 >> pos.x2 >> pos.y2 >> gx >> gy;
    grid_size_x = gx;
    grid_size_y = gy;
    int index;
    is >> index;
    if(index == -1)
        info = NULL;
    else
        info = g.gridinfo_store[index];
    resetPos();
    gsb = NULL;
    if(info == NULL || info->noGrid == false)
        gsb = new GridSortButton (this);
    gsb_displayed = false;
    int n_items;
    is >> n_items;
    items.resize(n_items);
    for(auto&& item : items) {
        item = new Item;
        item->load(is);
        item->parent = this;
    }
}

void Wound::save(ostream &os) {
    os << age << ' ' << severity << ' ' << bleeding << ' ';
}
void Wound::load(istream &is) {
    is >> age >> severity >> bleeding;
}

void Character::save(ostream &os) {
    os << strlen(name) << ' ' << name << ' ' << n << ' ';
    os << find_bitmap_index(sprite) << ' ';

    os << health << ' ' << pain << ' ' << warmth << ' '
       << fatigue << ' ' << hydration << ' ' << satiety << ' '
       << burden << ' ' << maxBurden << ' ' << skills << ' '
       << (int)selected_weapon_slot << ' ' << nextMove << ' '
       << (int)faction << ' ' << active << ' ';

    for(auto&& fac_rep : faction_reps) {
        os << fac_rep << ' ';
    }

    for(auto&& hp : inventory_hardpoints) {
        hp->save(os);
    }
    vehicle->save(os);

    medical_upper_torso->save(os);
    medical_lower_torso->save(os);
    medical_left_upper_leg->save(os);
    medical_right_upper_leg->save(os);
    medical_left_lower_leg->save(os);
    medical_right_lower_leg->save(os);
    medical_left_upper_arm->save(os);
    medical_right_upper_arm->save(os);
    medical_left_lower_arm->save(os);
    medical_right_lower_arm->save(os);

    for(auto&& w : wounds)
        w.save(os);

    for(auto&& d : diseases)
        d.save(os);
}

void Character::load(istream &is) {
    int name_len;
    is >> name_len;
    char *name = (char *)malloc(name_len + 1);
    is.ignore(1); // ignore the space
    is.read(&name[0], name_len);
    name[name_len] = '\0';
    this->name = name;
    size_t sprite_index;
    is >> n >> sprite_index;
    sprite = g.bitmaps[sprite_index];

    setPos(n);

    int w_slot;
    int i_fac;

    is >> health >> pain >> warmth
       >> fatigue >> hydration >> satiety
       >> burden >> maxBurden >> skills
       >> w_slot >> nextMove >> i_fac >> active;

    selected_weapon_slot = w_slot;
    faction = (Faction)i_fac;

    for(auto&& fac_rep : faction_reps) {
        is >> fac_rep;
    }

    for(auto&& hp :inventory_hardpoints) {
        hp->load(is);
    }
    vehicle->load(is);

    medical_upper_torso->load(is);
    medical_lower_torso->load(is);
    medical_left_upper_leg->load(is);
    medical_right_upper_leg->load(is);
    medical_left_lower_leg->load(is);
    medical_right_lower_leg->load(is);
    medical_left_upper_arm->load(is);
    medical_right_upper_arm->load(is);
    medical_left_lower_arm->load(is);
    medical_right_lower_arm->load(is);

    wounds.resize(10);
    for(auto&& w : wounds)
        w.load(is);

    diseases.resize(DISEASE_MAX);
    for(auto&& d : diseases)
        d.load(is);
}

void Location::save(ostream &os) {
    os << info_index << ' ' << last_looted << ' ';
}

void Location::load(istream &is) {
    is >> info_index >> last_looted;
}

void TileMap::save(ostream &os) {
    os << cols << ' ' << rows << '\n';
    os << view_x << ' ' << view_y << ' ' << view_px << ' ' << view_py << ' ';
    os << res_px << ' ' << res_py << '\n';
    os << size_x << ' ' << size_y << ' ' << pos.x1 << ' ' << pos.y1 << ' ';
    os << pos.x2 << ' ' << pos.y2 << ' ';
    os << max_t << ' ';
    os << '\n';

    os << tiles.size() << ' ';
    int i = 0;
    for(auto&& tile : tiles) {
        os << (int)tile.info_index << ' ' << (int)tile.visible << ' ';
        if(tile.ground_items != NULL) {
            os << true << ' ';
            os << tile.ground_items->size() << ' ';
            for(auto&& g : *tile.ground_items) {
                g->save(os);
            }
        } else {
            os << false << ' ';
        }
        if(tile.locations != NULL) {
            os << true << ' ';
            os << tile.locations->size() << ' ';
            for(auto &&l : *tile.locations) {
                l->save(os);
            }
        } else {
            os << false << ' ';
        }
        if(i % size_x == 0) {
            os << '\n';
        }
        i++;
    }
    os << '\n';
    player->save(os);
    os << '\n';
    os << all_characters.size() << ' ';
    os << '\n';
    for(auto&& ch : all_characters) {
        ch->save(os);
        os << '\n';
    }
    os << labels.size() << ' ';
    os << '\n';
    for(auto&& label : labels) {
        label.save(os);
        os << '\n';
    }
}

void TileMap::load(istream &is) {
    is >> cols >> rows >> view_x >> view_y >> view_px >> view_py >> res_px >> res_py;
    is >> size_x >> size_y >> pos.x1 >> pos.y1 >> pos.x2 >> pos.y2;
    is >> max_t;
    size_t n;
    is >> n;
    tiles.resize(n);
    for(auto&& tile : tiles) {
        int ii, v;
        is >> ii >> v;
        tile.info_index = ii;
        tile.visible = v;
        bool has_ground_items;
        is >> has_ground_items;
        if(has_ground_items == true) {
            int n_pages;
            is >> n_pages;
            tile.ground_items = new vector<Grid *>;
            (*tile.ground_items).resize(n_pages);
            for(auto&& page : *tile.ground_items) {
                page = new Grid;
                page->load(is);
            }
        } else {
            tile.ground_items = NULL;
        }
        bool has_locations;
        is >> has_locations;
        if(has_locations == true) {
            int n_loc;
            is >> n_loc;
            tile.locations = new vector<Location *>;
            (*tile.locations).resize(n_loc);
            for(auto &&loc : *tile.locations) {
                loc = new Location;
                loc->load(is);
            }
        } else {
            tile.locations = NULL;
        }
    }
    player = new Character;
    player->load(is);
    is >> n;
    all_characters.resize(n);
    for(auto&& ch : all_characters) {
        ch = new Character;
        ch->load(is);
    }
    player->update_visibility();
    updateActiveCharacters();
    updateCharsByPos();
    is >> n;
    for(size_t i = 0; i < n; i++) {
        Label tmp;
        tmp.load(is);
        labels.push_back(tmp);
    }
}

static bool save_game(const char *filename) {
    PerfTimer t("Save game");

    if(g.map == NULL)
        return false;

    ofstream out(filename, ios::out);
    if(out.fail() == true) {
        info("Error: Couldn't open test.sav for writing");
        cout << strerror(errno) << endl;
        return false;
    }

    try {
        out << "project_x " << COMPILED_VERSION << '\n';
        out << g.log->visible << ' ' << '\n';
        g.map->save(out);
    } catch (exception &e) {
        out.close();
        return false;
    }

    out.close();
    return true;
}

static bool load_game(const char *filename) {
    PerfTimer t("Load game");

    ifstream in(filename, ios::in);
    if(in.fail() == true) {
        info("Error: Couldn't open test.sav for reading");
        cout << strerror(errno) << endl;
        return false;
    }

    unload_game();
    bool log_visible;

    try {
        char str[10] = { '\0' };
        in.read(&str[0], 9);
        if(strcmp(str, "project_x") != 0) {
            info("Error: Invalid save game file: wrong starting string");
            cout << "got: \"" << str << "\" expected: \"project_x\"" << endl;
            in.close();
            return false;
        }
        int loaded_version;
        in >> loaded_version;
        if(loaded_version != COMPILED_VERSION) {
            info("Error: Wrong save game file version");
            cout << "got: " << loaded_version << " expected: " << COMPILED_VERSION << endl;
            in.close();
            return false;
        }

        init_tileinfo();
        in >> log_visible;
        g.map->load(in);
    } catch (exception &e) {
        in.close();
        return false;
    }

    in.close();

    init_minimap();
    init_messagelog();
    g.log->visible = log_visible;
    init_indicators();

    init_UIs();

    return true;
}

static void logo(void) {
    cout << " ____            _           _    __  __\n";
    cout << "|  _ \\ _ __ ___ (_) ___  ___| |_  \\ \\/ /\n";
    cout << "| |_) | '__/ _ \\| |/ _ \\/ __| __|  \\  / \n";
    cout << "|  __/| | | (_) | |  __/ (__| |_   /  \\ \n";
    cout << "|_|   |_|  \\___// |\\___|\\___|\\__| /_/\\_\\\n";
    cout << "              |__/                      \n";
#ifdef VERSION
    cout << "                     Build id: " << VERSION << "\n\n";
#endif
}

int main(int argc, char **argv) {
    logo();

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

    al_start_timer(timer);

    al_register_event_source(event_queue, al_get_display_event_source(g.display));
    al_register_event_source(event_queue, al_get_timer_event_source(timer));
    al_register_event_source(event_queue, al_get_keyboard_event_source());

    {
        PerfTimer t("Initialization");

        int seed;
        init_args(argc, argv, &seed);
        load_fonts();
        load_bitmaps();
        init_rng( seed );
        init_colors();
        // init_iteminfo();
        // save_ItemInfo();
        // g.item_info.clear();
        load_ItemInfo();
        init_hardpointinfo();
        init_weaponswitcher();
        init_buttons();
        init_recipes();
        init_skills();
        init_locationdata();
        init_timedisplay();
        init_misc();
        init_interactions();

        g.ui_MainMenu  = new MainMenuUI;
        g.ui_FadeTransition = new FadeTransitionUI;
        g.ui_Notification = new NotificationUI;

        g.map = NULL; // game is over
        g.minimap = NULL; // game is fully unloaded
    }

    al_set_target_backbuffer(g.display);

    /*
      TODO: Check maximum video bitmap dimensions and
      warn user if it's less than 2048x2048
     */

    if(argc <= 1) {
        switchToMainMenu();
        notify("This is pre-alpha software. Please report bugs and give feedback!");
        fade_to_UI();
    } else {
        new_game();
        button_MainMap_press();
    }

    bool redraw = true;
    bool was_mouse_down = false;
    bool draw_hover = false;

    double frame_start;
    double frame_end;
    double frame_time = 0;
    int16_t frame_counter = 0;

    g.running = true;

    // main loop
    while(g.running) {
        frame_start = al_current_time();

        al_get_mouse_state(&g.mouse_state);
        al_get_keyboard_state(&g.keyboard_state);

        g.mouse_x = g.mouse_state.x;
        g.mouse_y = g.mouse_state.y;
        draw_hover = false;
        // 1 - RMB
        // 2 - LMB
        // 4 - wheel
        g.mouse_button = g.mouse_state.buttons;

        if (g.mouse_state.buttons != 0) {
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
            draw_hover = true;
            // hover event
        }

        al_wait_for_event(event_queue, &ev);

        if(ev.type == ALLEGRO_EVENT_KEY_DOWN) {
            g.key = ev.keyboard.keycode;
            if(g.key == ALLEGRO_KEY_ESCAPE) {
                if(g.ui == g.ui_MainMenu) {
                    if(g.map != NULL)
                        button_MainMap_press();
                } else {
                    runMainMenu();
                }
            } else {
                g.ui->keyDownEvent();
            }
        }
        else if(ev.type == ALLEGRO_EVENT_TIMER) {
            { // logic goes here
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
                if(draw_hover == true)
                    g.ui->hoverOverEvent();
            }
            al_flip_display();
            frame_counter++;
        }

        frame_end = al_current_time();
        g.dt = frame_end - frame_start;
        frame_time += g.dt;
        if(frame_time >= 10.0) {
            // printf("FPS: %f\n", frame_counter / 10.0);
            frame_time = 0;
            frame_counter = 0;
        }
    }

    unload_bitmaps();
    unload_game();

    al_destroy_display(g.display);
    al_destroy_font(g.font);

    delete g.rng;
    delete g.time_display;
    delete g.weapon_switcher;
    delete g.ui_MainMenu;
    delete g.hand_combat;

    for(auto&& button : g.main_buttons)
        delete button;
    for(auto&& skill : g.skills)
        delete skill;
    for(auto&& gridinfo : g.gridinfo_store)
        delete gridinfo;
    for(auto&& recipe : recipes)
        delete recipe;
    for(auto&& loc : g.location_info)
        delete loc.location_item;
    for(auto&& s : g.stories)
        delete s.second;
    g.stories.clear();
    g.map_stories.clear();
    for(auto&& ii : g.item_info)
        free((char*)ii.name);

    g.item_info.clear();
    g.location_info.clear();

    info("Exiting");

    return 0;
}
