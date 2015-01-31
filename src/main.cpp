#include <allegro5/allegro.h>
#include <allegro5/allegro_color.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>

#include <stdint.h>

#include <unordered_map>
#include <vector>
#include <set>
#include <iostream>
#include <cmath>
#include <algorithm>

#include <sigc++/sigc++.h>

#include "util.h"

#define DEBUG_VISIBILITY false

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
struct HardpointInfo;
struct ItemInfo;

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

    TileMap *map;
    MiniMap *minimap;
    vector<ItemInfo> item_info;

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

    vector<ALLEGRO_BITMAP *> bitmaps;

    UI *ui; // current UI

    MainMapUI *ui_MainMap;
    MiniMapUI *ui_MiniMap;
    UI *ui_Skills;         // not implemented
    UI *ui_Crafting;       // not implemented
    ItemsUI *ui_Items;
    UI *ui_Conditions;     // not implemented
    UI *ui_Camp;           // not implemented
    VehicleUI *ui_Vehicle;

    MessageLog *log;

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
};

struct ItemInfo {
    string name;
    int maxStack;
    int weight; // [g]
    bool isVehicle;
    float condition;
    bool isContainer;
    int container_size_x;
    int container_size_y;
    ALLEGRO_BITMAP *sprite;
};

void init_iteminfo(void) {
    ItemInfo tmp;
    /* 00 */
    tmp.name = "";
    tmp.maxStack = 1;
    tmp.sprite = NULL;
    tmp.isVehicle = false;
    tmp.isContainer = false;
    tmp.container_size_x = 0;
    tmp.container_size_y = 0;
    g.item_info.push_back(tmp);

    /* 01 */
    tmp.name = "Backpack";
    tmp.maxStack = 1;
    tmp.weight = 1000;
    tmp.sprite = g.bitmaps[24];
    tmp.isVehicle = false;
    tmp.isContainer = true;
    tmp.container_size_x = 8;
    tmp.container_size_y = 8;
    g.item_info.push_back(tmp);

    /* 02 */
    tmp.name = "First aid kit";
    tmp.maxStack = 1;
    tmp.weight = 750;
    tmp.sprite = g.bitmaps[22];
    tmp.isVehicle = false;
    tmp.isContainer = true;
    tmp.container_size_x = 5;
    tmp.container_size_y = 5;
    g.item_info.push_back(tmp);

    /* 03 */
    tmp.name = "Crowbar";
    tmp.maxStack = 1;
    tmp.weight = 2500;
    tmp.sprite = g.bitmaps[20];
    tmp.isVehicle = false;
    tmp.isContainer = false;
    tmp.container_size_x = 0;
    tmp.container_size_y = 0;
    g.item_info.push_back(tmp);

    /* 04 */
    tmp.name = "Shopping trolley";
    tmp.maxStack = 1;
    tmp.weight = 5000;
    tmp.sprite = g.bitmaps[26];
    tmp.isVehicle = true;
    tmp.isContainer = true;
    tmp.container_size_x = 20;
    tmp.container_size_y = 20;
    g.item_info.push_back(tmp);

    /* 05 */
    tmp.name = "Pill Bottle";
    tmp.maxStack = 1;
    tmp.weight = 50;
    tmp.sprite = g.bitmaps[27];
    tmp.isVehicle = false;
    tmp.isContainer = true;
    tmp.container_size_x = 1;
    tmp.container_size_y = 2;
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
    // the grid that the item belongs to. set to NULL
    // when the item is held by the player
    Grid *parent;
    // when the item is held, this is set to the parent
    // so that the item can be returned if it can't
    // be placed
    Grid *old_parent;
    // items can have their own storage space e.g.
    // a backpack. NULL if they don't
    Grid *storage;
    // item's condition. Once it goes negative the item should be
    // destroyed. Item conditions should be decreased a bit on
    // each turn, and seperately when the item is used, so all items
    // should be checked at the end of the turn, and individual items
    // when they're used float condition;

    Item(int x1, int y1, int x2, int y2, int info_index);
    ~Item();

    void resetHardpointPos(void);

    void draw(void);

    bool isVehicle(void) { return g.item_info[info_index].isVehicle; }
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
        info("~Widget()");
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

    HardpointInfo() {
        vehiclepoint = false;
        maxItems = 1;
        sprite = NULL;
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
    // "widget" dimensions
    Rect pos;

    // grid spacing in pixels
    int grid_px_x;
    int grid_px_y;

    // grid size in grid units
    int grid_size_x;
    int grid_size_y;

    vector<Item *> items;

    HardpointInfo *hpinfo;
    GridSortButton *gsb;
    bool gsb_displayed;

    Grid(int w_pos_x, int w_pos_y, int size_x, int size_y, HardpointInfo *h) {
        hpinfo = h;
        grid_px_x = 10;
        grid_px_y = 10;
        grid_size_x = size_x;
        grid_size_y = size_y;
        pos.x1 = w_pos_x;
        pos.y1 = w_pos_y;
        resetPos();
        gsb = new GridSortButton (this);
        gsb_displayed = false;
    }

    ~Grid() {
        info("~Grid()");
        // a grid owns its items
        for(auto& item : items)
            delete item;
    }

    void draw(void);
    void drawAt(int x, int y);

    void AddItem(Item *item);
    void RemoveItemFromGrid(Item *item);
    void PlaceItemOnGrid(Item *to_place);

    void resetPos(void) {
        pos.x2 = pos.x1 + grid_size_x * grid_px_x;
        pos.y2 = pos.y1 + grid_size_y * grid_px_y;
    }
    Item *grab_item(int x, int y); // get item at screen position

    bool item_compatible(Item *i);
};

bool BiggerItemsFirst(Item *l, Item *r) {
    return l->pos.x2 * l->pos.y2 > r->pos.x2 * r->pos.y2;
}

bool SmallerItemsFirst(Item *l, Item *r) {
    return l->pos.x2 * l->pos.y2 < r->pos.x2 * r->pos.y2;
}

void sortGrid(Grid *to_sort, bool (*comp)(Item *l, Item *r)) {
    int num_items = to_sort->items.size();

    for(auto& item : to_sort->items) {
        item->pos.x1 = -999;
        item->pos.y1 = -999;
    }

    sort(to_sort->items.begin(), to_sort->items.end(), comp);

    to_sort->items.reserve(2 * num_items);

    for(auto& item : to_sort->items) {
        to_sort->PlaceItemOnGrid(item);
    }

    to_sort->items.erase(to_sort->items.begin(),
                         to_sort->items.begin() + num_items);

    assert((int)to_sort->items.size() == num_items);
}

Item::Item(int x1, int y1, int x2, int y2, int info_index) {
    pos.x1 = x1;
    pos.y1 = y1;
    pos.x2 = x2;
    pos.y2 = y2;
    parent = NULL;
    old_parent = NULL;
    storage = NULL;
    this->info_index = info_index;
    if(g.item_info[info_index].isContainer == true)
        storage = new Grid(0,
                           0,
                           g.item_info[info_index].container_size_x,
                           g.item_info[info_index].container_size_y,
                           NULL);
}

void Grid::AddItem(Item *item) {
    item->parent = this;
    items.push_back(item);
}

void Grid::RemoveItemFromGrid(Item *to_remove) {
    bool found = false;
    int c = 0;
    for(auto& item: items) {
        if(item == to_remove) {
            break;
        }
        c++;
    }
    assert(found == true);
    items.erase(items.begin() + c);
}

bool rectIntersect(int a_x, int a_y, int a_width, int a_height,
                   int b_x, int b_y, int b_width, int b_height);

// automatically find a place to place the item on
void Grid::PlaceItemOnGrid(Item *to_place) {
    if(hpinfo != NULL) {
        // if we're on a hard point just check how many item there
        // are there already
        if((int)items.size() < hpinfo->maxItems) {
            to_place->pos.x1 = 0;
            to_place->pos.y1 = 0;
            AddItem(to_place);
            return;
        }
    }

    // if it's a grid we try placing it on every grid square
    bool found = false;
    int drop_x = 0;
    int drop_y = 0;
    for(drop_y = 0; drop_y < grid_size_y; drop_y++) {
        for(drop_x = 0; drop_x < grid_size_x; drop_x++) {
            int collides_with = 0;
            // check how many items it collides with
            for(auto& item : items) {
                if(rectIntersect(item->pos.x1 * 10,
                                 item->pos.y1 * 10,
                                 item->pos.x2 * 10,
                                 item->pos.y2 * 10,
                                 drop_x * 10,
                                 drop_y * 10,
                                 to_place->pos.x2 * 10,
                                 to_place->pos.y2 * 10)) {
                    collides_with += 1;
                }
            }
            if(collides_with == 0) {
                // it doesn't collide with any items, so
                // check the right and bottom bounds of the grid
                if(pos.x1 + (drop_x + to_place->pos.x2) * 10 <= pos.x2 &&
                   pos.y1 + (drop_y + to_place->pos.y2) * 10 <= pos.y2) {
                    found = true;
                    goto done;
                }
            }
        }
    }
 done:
    if(found == true) {
        to_place->pos.x1 = drop_x;
        to_place->pos.y1 = drop_y;

        // clear the grid sort button if the new grid isn't a hardpoint
        if(to_place->storage != NULL &&
           this->hpinfo == NULL) {
            to_place->storage->gsb_displayed = false;
        }
        AddItem(to_place);
    }
    else {
        // couldn't place it
        assert(to_place->parent != NULL);
        to_place->parent->AddItem(to_place);
    }
}

bool Grid::item_compatible(Item *i) {
    // you can place anything in a grid
    if(hpinfo == NULL)
        return true;

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
    info("~Item()");
}

void Item::resetHardpointPos(void) {
    if(parent != NULL) {
        storage->pos.x1 = parent->pos.x1 + pos.x2 * 10 + 10;
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
    vector<Grid *> inventory_hardpoints;

    ALLEGRO_BITMAP *sprite;
    vector<int> currently_seeing;
    // 0 - sees only tile they're on
    // 1 - sees immediate neighbours
    // etc
    int current_los_distance;

    void update_visibility(void);

    Character(int n, ALLEGRO_BITMAP *sprite);

    ~Character() {
        for(auto& hardpoint : inventory_hardpoints) {
            delete hardpoint;
        }
        delete vehicle;
        info("~Character()");
    };

    void draw(void);
    void drawOffset(int offset_x, int offset_y);

    void addInventoryHardpoints(GridSystem *gs);
    void addVehicleHardpoint(GridSystem *gs);

    void do_AI_map_turn(void);
    void die(void);
};

// do stuff on the map
void Character::do_AI_map_turn(void) {

}

// drop held items on the ground
void Character::die(void) {

}

Character::Character(int n, ALLEGRO_BITMAP *sprite) {
    this->n = n;
    this->sprite = sprite;

    int off_x = 350;
    int off_y = 100;
    neck = new Grid (off_x, off_y - 50, 4, 4, g.neck);
    head = new Grid (off_x, off_y, 4, 4, g.head);
    right_shoulder = new Grid (off_x - 50, off_y, 4, 4, g.right_shoulder);
    left_shoulder = new Grid (off_x + 50, off_y, 4, 4, g.left_shoulder);
    torso = new Grid (off_x - 15, off_y + 50, 7, 8, g.torso);
    right_hand = new Grid (off_x - 65, off_y + 100, 4, 4, g.right_hand_hold);
    left_hand = new Grid (off_x + 65, off_y + 100, 4, 4, g.left_hand_hold);
    legs = new Grid (off_x - 15, off_y + 140, 7, 10, g.legs);
    right_foot = new Grid (off_x - 25, off_y + 250, 4, 4, g.right_foot);
    left_foot = new Grid (off_x + 25, off_y + 250, 4, 4, g.left_foot);
    right_hand_hold = new Grid (530, 175, 4, 4, g.right_hand_hold);
    left_hand_hold = new Grid (530, 300, 4, 4, g.left_hand_hold);
    vehicle = new Grid(400, 150, 4, 4, g.vehicle);
    back = new Grid(530, 50, 4, 4, g.back);

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
}

int dir_transform(int n, int dir);

struct TileInfo {
    int bitmap_index;
    ALLEGRO_COLOR minimap_color;
    bool blocks_los;
};

struct Tile {
    int8_t info_index;
    int8_t visible;
    Grid *ground_items;

    Tile() {
        info("Tile()");
        ground_items = NULL;
        info_index = 0;
        visible = 0;
    }

    ~Tile() {
        info("~Tile()");
        delete ground_items;
    }
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
    int hex_size_x;
    int hex_size_y;
    // size of the hex render steps
    int hex_step_x;
    int hex_step_y;

    // stuff that's constant between TileMap::draw
    // and TileMap::drawTile
    int max_t;
    int start;
    int mouse_n;

    // map from positions to characters
    unordered_multimap<int, Character *> charsByPos;
    
    vector<Character *> characters;
    Character *player;

    // array of size_x * size_y tiles, allocated in constructor
    // TODO: would a 2D array be better?
    vector <Tile> tiles;

    vector<TileInfo> tile_info;
    vector<ALLEGRO_BITMAP *>bitmaps;

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
    Character *characterAt(int n);
    bool playerSees(int n);

    void updateCharsByPos(void);
};

void TileMap::updateCharsByPos(void) {
    charsByPos.clear();
    charsByPos.emplace(player->n, player);

    for(auto& npc : characters) {
        charsByPos.emplace(npc->n, npc);
    }
}

Button::Button() {
    pressed = false;
    up = NULL;
    down = NULL;
}

struct GridSystem : public Widget {
    vector<Grid *> grids;

    Item *held;

    GridSystem(void) {
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

    void mouseDown(void) { gsMouseDownEvent(); }
    void mouseUp(void) { gsMouseUpEvent(); }
    void keyDown(void) { }
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
    void AutoMoveAllItems(Grid *from, Grid *to);
};

void GridSystem::AutoMoveItem(Item *item, Grid *from, Grid *to) {
    from->RemoveItemFromGrid(item);
    to->PlaceItemOnGrid(item);
}

void GridSystem::AutoMoveAllItems(Grid *from, Grid *to) {
    for(auto& item : from->items)
        AutoMoveItem(item, from, to);
}

Grid *ground_at_player(void);

// moves items under mouse cursor to the ground, if possible
// this is always for the player
void GridSystem::MouseAutoMoveItemToGround() {
    Grid *ground = ground_at_player();

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
    assert(ground != NULL);
    assert(from != NULL);

    ground->PlaceItemOnGrid(item);
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
    for(auto& tile : tiles) {
        delete tile.ground_items;
    }
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
    
    hex_size_x = 100;
    hex_size_y = 80;

    hex_step_x = 80;
    hex_step_y = 40;

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
        g.AddMessage("player moved");
        player->n = clicked_nearby;
        player->update_visibility();
    }

    char buf[100];
    snprintf(buf, sizeof(buf), "clicked on n=%d %d %d %f %f %f %f",
             clicked_nearby, g.mouse_x, g.mouse_y, pos.x1, pos.y1, pos.x2, pos.y2);
    g.AddMessage(buf);
}

bool good_index(int n) {
    return n >= 0 && n <= g.map->max_t;
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
    void draw(void);

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
        sortGrid(parent, BiggerItemsFirst);
    else if(g.mouse_button == 2)
        sortGrid(parent, SmallerItemsFirst);
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
    int start = max(0, lines_n - 23);
    for(int i = start; i < lines_n; i++) {
        al_draw_text(font, g.color_grey3, pos.x1 + 8, pos.y1 + off_y, 0, lines[i].c_str());
        off_y = off_y + 8;
    }
}

void Button::draw(void) {
    if(pressed == true && down != NULL)
        al_draw_bitmap(down, pos.x1, pos.y1, 0);
    else
        al_draw_bitmap(up, pos.x1, pos.y1, 0);
}

void Button::update() {
}

void TileMap::generate(void) {
    tiles.reserve(size_x * size_y);
    int up_to = bitmaps.size();
    for(int i = 0; i <= max_t; i++) {
        tiles[i].info_index = rand() % up_to;
        tiles[i].visible = false;
        tiles[i].ground_items = NULL;
    }
    info("Finished generating map");
    cout << (int)tiles.size() << endl;
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
    updateCharsByPos();
    int j = 0;
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
    cout << j << endl;

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
            if(item->parent->pos.x1 + item->pos.x1 * 10 <= g.mouse_x &&
               item->parent->pos.y1 + item->pos.y1 * 10 <= g.mouse_y &&
               item->parent->pos.x1 + (item->pos.x1 + item->pos.x2) * 10 >= g.mouse_x &&
               item->parent->pos.y1 + (item->pos.y1 + item->pos.y2) * 10 >= g.mouse_y) {

                al_draw_filled_rectangle(g.mouse_x + 16, g.mouse_y,
                                         g.mouse_x + 150, g.mouse_y + 48, g.color_black);
                al_draw_text(g.font, g.color_grey3, g.mouse_x + 24, g.mouse_y + 8,
                             0, g.item_info[item->info_index].name.c_str());
                al_draw_textf(g.font, g.color_grey3, g.mouse_x + 24, g.mouse_y + 24,
                              0, "%d g", g.item_info[item->info_index].weight);

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
        MouseAutoMoveItemToGround();
    }
    // check if we're clicking the sort buttons
    for(auto& grid : grids) {
        if(g.mouse_x > grid->gsb->pos.x1 && g.mouse_y > grid->gsb->pos.y1 &&
           g.mouse_y < grid->gsb->pos.x2 && g.mouse_y < grid->gsb->pos.y2) {
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
        i = grid->grab_item(g.mouse_x, g.mouse_y);
        if(i != NULL)
            goto got_it;
    }
    // no
    return;

 got_it:
    held = i;
    g.hold_off_x =
        g.mouse_x - (i->parent->pos.x1 + i->pos.x1 * 10);
    g.hold_off_y =
        g.mouse_y - (i->parent->pos.y1 + i->pos.y1 * 10);
    held->old_parent = held->parent;
    held->parent = NULL;

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

bool rectIntersect(int a_x, int a_y, int a_width, int a_height,
                   int b_x, int b_y, int b_width, int b_height) {
    return
        !(b_x >= a_x + a_width
          || b_x + b_width <= a_x
          || b_y >= a_y + a_height
          || b_y + b_height <= a_y);
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

void GridSystem::gsMouseUpEvent() {
    // are we holding an item?
    if(held == NULL)
        return;

    // find a grid to drop it on
    int i = 0;
    for(auto& grid : grids) {
        // proposed position to drop it into relative to the
        // grid currently being examined
        int drop_x = 0;
        int drop_y = 0;

        // the bounds check depends if the grid is a real grid
        // or a hardpoint
        bool in_bounds = false;
        if(grid->hpinfo == NULL) {
            drop_x = ((g.mouse_x - g.hold_off_x) - grid->pos.x1) / 10;
            drop_y = ((g.mouse_y - g.hold_off_y) - grid->pos.y1) / 10;
            in_bounds =
                g.mouse_x - g.hold_off_x >= grid->pos.x1 &&
                g.mouse_y - g.hold_off_y >= grid->pos.y1 &&
                g.mouse_x - g.hold_off_x + 10 * held->pos.x2
                <= grid->pos.x2 + 8 &&
                g.mouse_y - g.hold_off_y + 10 * held->pos.y2
                <= grid->pos.y2 + 8;
        }
        else {
            in_bounds =
                g.mouse_x >= grid->pos.x1 &&
                g.mouse_y >= grid->pos.y1 &&
                g.mouse_x <= grid->pos.x2 + 8 &&
                g.mouse_y <= grid->pos.y2 + 8;
        }

        if(in_bounds) {
            // is this a real grid?
            if(grid->hpinfo == NULL) {
                // bounds check items in grid
                for(auto& item : grid->items) {
                    if(rectIntersect(item->pos.x1 * 10,
                                     item->pos.y1 * 10,
                                     item->pos.x2 * 10,
                                     item->pos.y2 * 10,
                                     drop_x * 10,
                                     drop_y * 10,
                                     held->pos.x2 * 10,
                                     held->pos.y2 * 10)) {
                        // there's an item there already:
                        // abort
                        goto blocked;
                    }
                }
            } // or a hardpoint?
            else if(grid->hpinfo->maxItems <= (int)grid->items.size()) {
                // too many items on the hardpoint
                goto blocked;
            }

            // is this item compatible with the grid?
            if(grid->item_compatible(held) == false)
               goto blocked;

            // we've found the grid and there's nothing blocking
            // the placement there, so drop it
            held->parent = grid;
            held->old_parent = NULL;
            held->pos.x1 = drop_x;
            held->pos.y1 = drop_y;
            grid->items.push_back(held);

            addStorageGrid();

            held = NULL;

            char b[40];
            snprintf(b, sizeof(b), "Moved onto grid %d", i);
            g.AddMessage(b);
            // the item is placed. we're done
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
}

void Grid::draw(void) {
    al_draw_filled_rectangle(pos.x1, pos.y1, pos.x2, pos.y2, g.color_grey2);
    if(hpinfo == NULL) {
        for (int x = pos.x1; x <= pos.x2; x = x + grid_px_x)
            al_draw_line(x, pos.y1, x, pos.y2, g.color_grey3, 1);
        for (int y = pos.y1; y <= pos.y2; y = y + grid_px_y)
            al_draw_line(pos.x1, y, pos.x2, y, g.color_grey3, 1);
    }
    if(gsb_displayed == true)
        gsb->draw();

    for (auto& i : items)
        i->draw();
}

void Grid::drawAt(int x, int y) {
    // well...
    int old_x1 = pos.x1;
    int old_y1 = pos.y1;
    int old_x2 = pos.x2;
    int old_y2 = pos.y2;
    pos.x1 = x;
    pos.y1 = y;
    pos.x2 = x + grid_size_x * grid_px_x;
    pos.y2 = y + grid_size_y * grid_px_y;
    draw();
    pos.x1 = old_x1;
    pos.y1 = old_y1;
    pos.x2 = old_x2;
    pos.y2 = old_y2;
}

void Item::draw(void) {
    ALLEGRO_BITMAP *sprite = g.item_info[info_index].sprite;

    if(parent != NULL) {
        // we're on a grid
        int x1 = parent->pos.x1 + pos.x1 * 10;
        int y1 = parent->pos.y1 + pos.y1 * 10;
        int x2 = parent->pos.x1 + (pos.x1 + pos.x2) * 10;
        int y2 = parent->pos.y1 + (pos.y1 + pos.y2) * 10;

        if(sprite == NULL) {
            al_draw_filled_rectangle(x1, y1, x2, y2, g.color_grey3);
            al_draw_rectangle(x1, y1, x2, y2, g.color_black, 1);
        }
        else
            al_draw_bitmap(sprite, x1, y1, 0);
    } else {
        // we're held by the mouse
        int x1 = g.mouse_x - g.hold_off_x;
        int y1 = g.mouse_y - g.hold_off_y;
        int x2 = g.mouse_x - g.hold_off_x + pos.x2 * 10;
        int y2 = g.mouse_y - g.hold_off_y + pos.y2 * 10;

        if(sprite == NULL) {
            al_draw_filled_rectangle(x1, y1, x2, y2, g.color_grey3);
            al_draw_rectangle(x1, y1, x2, y2, g.color_black, 1);
        }
        else
            al_draw_bitmap(sprite, x1, y1, 0);
    }
}

Item *Grid::grab_item(int x, int y) {
    int c = 0;
    for (auto& i : items) {
        if(i->parent->pos.x1 + i->pos.x1 * 10 <= x &&
           i->parent->pos.y1 + i->pos.y1 * 10 <= y &&
           i->parent->pos.x1 + (i->pos.x1 + i->pos.x2) * 10 >= x &&
           i->parent->pos.y1 + (i->pos.y1 + i->pos.y2) * 10 >= y) {

            Item *a = items[c];
            items.erase(items.begin() + c);
            return a;
        }
        c++;
    }
    return NULL;
}

void end_turn() {
    g.AddMessage("Turn ended");
}

struct MainMapUI : public UI {
    MainMapUI();
    ~MainMapUI();
};

struct MiniMapUI : public UI {
    MiniMapUI();
    ~MiniMapUI();
};

struct InventoryGridSystem;
struct VehicleGridSystem;

struct ItemsUI : public UI {
    InventoryGridSystem *gridsystem;

    ItemsUI();
    ~ItemsUI();
};

struct VehicleUI : public UI {
    VehicleGridSystem *gridsystem;

    VehicleUI();
    ~VehicleUI();
};

struct VehicleGridSystem : public GridSystem {
    VehicleGridSystem() { };
    ~VehicleGridSystem() {
        info("~VehicleGridSystem()");
    };

    void reset(void);
    void draw(void) {
        al_draw_text(g.font, g.color_white, 80, 35, 0, "Ground:");
        al_draw_text(g.font, g.color_white, 390, 135, 0, "Vehicle:");
        GridSystem::draw();
    }
};

struct InventoryGridSystem : public GridSystem {
    InventoryGridSystem() {
        reset();
    }
    ~InventoryGridSystem() {
        info("~InventoryGridSystem()");
    };

    void reset(void);
    void draw(void) {
        al_draw_text(g.font, g.color_white, 80, 35, 0, "Ground:");
        al_draw_text(g.font, g.color_white, 350, 35, 0, "You:");

        GridSystem::draw();
    }
};

VehicleUI::VehicleUI() {
    gridsystem = new VehicleGridSystem;

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
}

VehicleUI::~VehicleUI() {
    info("~VehicleUI()");
}

ItemsUI::ItemsUI() {
    gridsystem  = new InventoryGridSystem;

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
}

ItemsUI::~ItemsUI() {
    info("~ItemsUI()");
}

/*
  TODO: valgrind reports a loss here:

  448 (80 direct, 368 indirect) bytes in 1 blocks are definitely lost in loss record 617 of 702
  at 0x4C2B100: operator new(unsigned long) (in /usr/lib/valgrind/vgpreload_memcheck-amd64-linux.so)
  by 0x409DC2: ground_at_player() (main.cpp:1980)
*/
Grid *ground_at_player(void) {
    // get ground inventory at player position
    Grid *ground = g.map->tiles[g.map->player->n].ground_items;

    // create it if it doesn't exist
    if(ground == NULL) {
        cout << "creating new ground" << endl;
        ground = new Grid (20, 50, 20, 30, NULL);
        Item *crowbar = new Item(0, 0, 14, 2, 3);
        Item *shopping_trolley = new Item(0, 0, 16, 16, 4);
        Item *pill_bottle = new Item(0, 0, 1, 2, 5);
        ground->PlaceItemOnGrid(crowbar);
        ground->PlaceItemOnGrid(shopping_trolley);
        ground->PlaceItemOnGrid(pill_bottle);
        g.map->tiles[g.map->player->n].ground_items = ground;
    }
    return ground;
}

void VehicleGridSystem::reset(void) {
    grids.clear();
    grids.push_back(g.map->player->vehicle);

    g.map->player->addVehicleHardpoint(this);

    Grid *ground = ground_at_player();
    ground->gsb_displayed = true;
    grids.push_back(ground);
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

    Grid *ground = ground_at_player();
    ground->gsb_displayed = true;
    grids.push_back(ground);
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
}

// these could probably be a single function
void button_MainMap_press(void) {
    if(g.ui != g.ui_MainMap) {
        g.ui = g.ui_MainMap;
        g.color_bg = g.color_black;
        g.AddMessage("Switched to main map.");
    }
    main_buttons_update();
}

void button_Items_press(void) {
    if(g.ui != g.ui_Items) {
        g.ui_Items->gridsystem->reset();
        g.ui = g.ui_Items;
        g.AddMessage("Switched to inventory.");
        g.color_bg = g.color_grey;
    }
    main_buttons_update();
}

void button_Vehicle_press(void) {
    if(g.ui != g.ui_Vehicle) {
        g.ui_Vehicle->gridsystem->reset();
        g.ui = g.ui_Vehicle;
        g.color_bg = g.color_grey;
        g.AddMessage("Switched to vehicle.");
    }
    main_buttons_update();
}

void button_MiniMap_press(void) {
    if(g.ui != g.ui_MiniMap) {
        g.minimap->recreate();
        g.ui = g.ui_MiniMap;
        g.color_bg = g.color_grey;
        g.AddMessage("Switched to mini map.");
    }
    main_buttons_update();
}

void button_endturn_press(void) {
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

    g.button_Crafting->pos.x1 = 0;
    g.button_Crafting->pos.y1 = 660;
    g.button_Crafting->pos.x2 = 100;
    g.button_Crafting->pos.y2 = 60;
    g.button_Crafting->up = g.bitmaps[6];
    g.button_Crafting->down = g.bitmaps[7];

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

    g.button_Camp->pos.x1 = 1180;
    g.button_Camp->pos.y1 = 400;
    g.button_Camp->pos.x2 = 100;
    g.button_Camp->pos.y2 = 60;
    g.button_Camp->up = g.bitmaps[12];
    g.button_Camp->down = g.bitmaps[13];

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
    g.log->pos.y1 = 520;
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
    g.map->tile_info.push_back(i);
    // tree
    i.minimap_color = al_map_rgb(0, 150, 0);
    i.bitmap_index = 1;
    i.blocks_los = false;
    g.map->tile_info.push_back(i);
    // city
    i.minimap_color = al_map_rgb(255, 255, 255);
    i.bitmap_index = 2;
    i.blocks_los = true;
    g.map->tile_info.push_back(i);
    // lake
    i.minimap_color = al_map_rgb(0, 0, 200);
    i.bitmap_index = 3;
    i.blocks_los = false;
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
}

MiniMapUI::~MiniMapUI(void) {
    info("~MiniMapUI()");
}

// creates the player and npcs
// must be called after init_tilemap();
void init_characters(void) {
    for(int i = 0; i < 5; i++) {
        int n = rand() % g.map->max_t;

        Character *c = new Character(55, g.bitmaps[21]);

        // add starting items
        Item *backpack = new Item(0, 0, 7, 7, 1);
        c->back->PlaceItemOnGrid(backpack);
        Item *first_aid_kit1 = new Item(0, 0, 6, 6, 2);
        backpack->storage->PlaceItemOnGrid(first_aid_kit1);
        Item *first_aid_kit2 = new Item(0, 0, 6, 6, 2);
        c->left_hand_hold->PlaceItemOnGrid(first_aid_kit2);
        Item *pill_bottle2 = new Item(0, 0, 1, 2, 5);
        first_aid_kit1->storage->PlaceItemOnGrid(pill_bottle2);
        Item *pill_bottle3 = new Item(0, 0, 1, 2, 5);
        first_aid_kit2->storage->PlaceItemOnGrid(pill_bottle3);

        // player is character 0
        if(i == 0) {
            g.map->player = c;
        }
        else // everyone else is an NPC
            g.map->characters.push_back(c);
    }
    g.map->player->update_visibility();
    g.map->focusOnPlayer();
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
    widgets.push_back(g.button_endturn);
}

MainMapUI::~MainMapUI(void) {
    info("~MainMapUI()");
}

void allegro_init(void) {
    int ret = 0;

    al_init(); // no return value
    info("Probably initialized core allegro library.");

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
}

int main(void) {
    allegro_init();

    g.display_x = 1280;
    g.display_y = 720;
    g.display = al_create_display(g.display_x, g.display_y);
    if(g.display == NULL)
        errorQuit("Failed to create display.");
    else
        info("Created display.");

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

    g.font = al_create_builtin_font();
    if(g.font == NULL)
        errorQuit("failed to load builtin font.");
    else
        info("Loaded builtin font.");

    ALLEGRO_EVENT ev;
    ALLEGRO_MOUSE_STATE mouse_state;
    ALLEGRO_KEYBOARD_STATE keyboard_state;

    al_start_timer(timer);
    al_set_target_backbuffer(g.display);

    al_register_event_source(event_queue, al_get_display_event_source(g.display));
    al_register_event_source(event_queue, al_get_timer_event_source(timer));
    al_register_event_source(event_queue, al_get_keyboard_event_source());

    g.color_grey = al_color_name("grey");
    g.color_grey2 = al_map_rgb(200, 200, 200);
    g.color_grey3 = al_map_rgb(220, 220, 220);
    g.color_black = al_map_rgb(0, 0, 0);
    g.color_white = al_map_rgb(255, 255, 255);
    g.color_tile_tint = al_map_rgba_f(0.5, 0.5, 0.5, 1.0);
    g.color_active_tile_tint = al_map_rgba_f(1, 1, 1, 0.2);
    g.color_bg = g.color_black;

    load_bitmaps();
    init_iteminfo();
    init_tilemap();
    init_hardpointinfo();
    init_characters();
    init_buttons();
    init_messagelog();
    init_minimap();

    {
        g.ui_MainMap = new(MainMapUI);
        g.ui_MiniMap = new(MiniMapUI);
        g.ui_Items   = new(ItemsUI);
        g.ui_Vehicle = new(VehicleUI);

        // start on the main map
        g.ui = g.ui_MainMap;
        main_buttons_update();
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
                break;
            else
                g.ui->keyDownEvent();
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

    for(auto& widget : g.all_widgets)
        delete widget;

    info("Exiting");

    return 0;
}
