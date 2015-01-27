#include <allegro5/allegro.h>
#include <allegro5/allegro_color.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>

#include <stdint.h>

#include <vector>
#include <iostream>
#include <memory>
#include <cmath>

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

// global state
struct Game {
    ALLEGRO_DISPLAY *display;
    ALLEGRO_FONT *font;

    ALLEGRO_COLOR color_white;
    ALLEGRO_COLOR color_black;
    ALLEGRO_COLOR color_grey;
    ALLEGRO_COLOR color_grey2;
    ALLEGRO_COLOR color_grey3;
    ALLEGRO_COLOR color_tile_tint;

    Button *button_MainMap;
    Button *button_MiniMap;
    Button *button_Skills;
    Button *button_Crafting;
    Button *button_Items;
    Button *button_Condition;
    Button *button_Camp;
    Button *button_Vehicle;

    TileMap *map;
    MiniMap *minimap;
    vector<Character *> characters;
    Character *player;
    Grid *player_inventory;

    vector<ALLEGRO_BITMAP *> bitmaps;

    UI *ui; // current UI

    MainMapUI *ui_MainMap;
    MiniMapUI *ui_MiniMap;
    UI *ui_Skills;
    UI *ui_Crafting;
    ItemsUI *ui_Items;
    UI *ui_Conditions;
    UI *ui_Camp;
    UI *ui_Vehicle;

    MessageLog *log;

    int mouse_x;
    int mouse_y;
    int hold_off_x;
    int hold_off_y;
    int key;

    void AddMessage(string str);
};

Game g;

struct Rect {
    float x1;
    float y1;
    float x2;
    float y2;
};

struct Item {
    Item();
    Item(int x1, int y1, int x2, int y2, ALLEGRO_BITMAP *sprite);

    Rect pos;
    string name;
    Grid *parent;
    Grid *old_parent;
    ALLEGRO_BITMAP *sprite;
    void draw(void);
};

Item::Item() {
    pos.x1 = 1;
    pos.y1 = 1;
    pos.x2 = 1;
    pos.y2 = 1;
    name = "";
    sprite = NULL;
}

Item::Item(int x1, int y1, int x2, int y2, ALLEGRO_BITMAP *img) {
    pos.x1 = x1;
    pos.y1 = y1;
    pos.x2 = x2;
    pos.y2 = y2;
    name = "";
    sprite = img;
}

struct Widget {
    Rect pos;
    signal<void> onMouseDown;
    signal<void> onMouseUp;
    signal<void> onKeyDown;

    virtual void mouseDown(void) = 0;
    virtual void mouseUp(void) = 0;
    virtual void keyDown(void) = 0;

    virtual ~Widget() { info("~Widget"); };

    virtual void draw(void) = 0;
    virtual void update() { };
};

struct Character : public Widget {
    int n; // position by offset

    ALLEGRO_BITMAP *sprite;
    vector<int> currently_seeing;

    void update_visibility(void);

    Character(int n, ALLEGRO_BITMAP *sprite) {
        this->n = n;
        this->sprite = sprite;
        currently_seeing.reserve(50);
    }

    ~Character(void) {
        info("~Character");
    }

    void mouseDown(void) {
        g.AddMessage("That tickles!");
    }
    void mouseUp(void) { }
    void keyDown(void) { }

    void draw(void);
};

int dir_transform(int n, int dir);

struct Grid {
    Rect pos;
    vector<Item *> items;

    ~Grid() {
        info("~Grid()");
        for(auto& item : items)
            delete item;
    }

    void draw(void);
    Item *get_item(int x, int y); // get item at screen position
};

struct GridSystem : public Widget {
    vector<Grid *> grids;
    Item *held;

    GridSystem(void) { }
    ~GridSystem(void) {
        info("~GridSystem()");
    }

    void mouseDown(void) { gsMouseDownEvent(); }
    void mouseUp(void) { gsMouseUpEvent(); }
    void keyDown(void) { }

    void gsMouseDownEvent(void);
    void gsMouseUpEvent(void);
    void draw(void);
};

struct TileInfo {
    int bitmap_index;
    ALLEGRO_COLOR minimap_color;
};

struct Tile {
    int8_t info_index;
    int8_t visible;
    Grid *ground_items;

    Tile() {
        ground_items = NULL;
        info_index = 0;
        visible = 0;
    }
};

struct TileMap : public Widget {
    int cols; // display dimensions
    int rows; // display dimensions
    int view_x;
    int view_y;
    vector<Tile> tiles;
    vector<TileInfo> tile_info;
    vector<ALLEGRO_BITMAP *>bitmaps;

    ~TileMap(void);

    void handleKeyDown(void);

    void mouseDown(void);
    void mouseUp(void) { }
    void keyDown(void) { handleKeyDown(); }

    // must be called after the bitmaps vector is filled
    void generate(void);
    void draw(void);
    void drawTopHalfOfTileAt(int x, int y);
    void mouseToTileXY(int &x, int &y);
    int mouseToTileN(void);
};

struct MiniMap : public Widget {
    MiniMap();
    ~MiniMap();

    ALLEGRO_BITMAP *buf;

    void mouseDown(void) { }
    void mouseUp(void) { }
    void keyDown(void) { }

    void draw(void);
    void update(void) { }
    void recreate(void);
};

MiniMap::MiniMap() {
    buf = al_create_bitmap(304, 304);
}

MiniMap::~MiniMap() {
    al_destroy_bitmap(buf);
    info("~MiniMap()");
}

void MiniMap::recreate() {
    al_set_target_bitmap(buf);

    al_clear_to_color(g.color_black);
    al_draw_rectangle(1, 1, 303, 303, g.color_white, 2);

    // draw the minimap
    for(int y = 0; y < 150; y++) {
        int y150 = 150 * y;
        for(int x = 0; x < 150; x++) {
            Tile t = g.map->tiles[y150 + x];
            if(t.visible) {
                ALLEGRO_COLOR col = g.map->tile_info[t.info_index].minimap_color;
                // four pixels per hex cell
                al_put_pixel(x * 2 + 2, y * 2 + 2, col);
                al_put_pixel(x * 2 + 3, y * 2 + 2, col);
                al_put_pixel(x * 2 + 2, y * 2 + 3, col);
                al_put_pixel(x * 2 + 3, y * 2 + 3, col);
            }
        }
    }

    ALLEGRO_COLOR red = al_map_rgb(255, 0, 0);

    // draw player location lines
    int p_x = g.player->n % 150;
    int p_y = g.player->n / 150;
    al_draw_line(p_x * 2 + 2, 2, p_x * 2 + 2, 302, red, 2);
    al_draw_line(2, p_y * 2 + 2, 302, p_y * 2 + 2, red, 2);

    // draw mainmap view rectangle
    al_draw_rectangle(g.map->view_x * 2 + 2,
                      g.map->view_y * 2 + 2,
                      (g.map->view_x + 20) * 2 + 2,
                      (g.map->view_y + 18) * 2 + 2, red, 2);

    al_set_target_backbuffer(g.display);
}

void MiniMap::draw(void) {
    al_draw_bitmap(buf, 490, 150, 0);
}

void TileMap::mouseToTileXY(int &x, int &y) {
    x = view_x + floor(g.mouse_x / 80);
    int off_y = x % 2 == 0 ? 0 : 20;
    y = view_y + floor((g.mouse_y - 40 - off_y) / 40);
}

int TileMap::mouseToTileN(void) {
    int x, y;
    mouseToTileXY(x, y);
    return 150 * y + x;
}

// check if the clicked tile is next to the player
// if so, move them there.
void TileMap::mouseDown(void) {
    int clicked_n = mouseToTileN();
    int player_n = g.player->n;

    if(clicked_n == player_n)
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
        g.player->n = clicked_nearby;
        g.player->update_visibility();
    }

    char buf[35];
    snprintf(buf, sizeof(buf), "clicked in direction dir=%d", clicked_nearby);
    g.AddMessage(buf);
}

void Character::update_visibility(void) {
    currently_seeing.clear();
    for(int dir = 0; dir <= 6; dir++) {
        int n2 = dir_transform(n, dir);
        for(int dir2 = 0; dir2 <= 6; dir2++) {
            int n3 = dir_transform(n2, dir2);
            g.map->tiles[n3].visible = true;
            currently_seeing.push_back(n3);
        }
    }
}

/*
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
            return n - 150;
        case 2:
            return n - 150 + 1;
        case 3:
            return n + 1;
        case 4:
            return n + 150;
        case 5:
            return n - 1;
        case 6:
            return n - 150 - 1;
        }
    } else {
        // odd
        switch(dir) {
        case 0:
            return n;
        case 1:
            return n - 150;
        case 2:
            return n + 1;
        case 3:
            return n + 150 + 1;
        case 4:
            return n + 150;
        case 5:
            return n + 150 - 1;
        case 6:
            return n - 1;
        }
    }
    return n;
}
void Character::draw(void) {
    int ch_x = n % 150;
    int ch_y = floor(n / 150);

    int r_x = ch_x - g.map->view_x;
    int r_y = ch_y - g.map->view_y;

    int off_x = 80 * r_x;
    int off_y = n % 2  == 0 ? 0 : 20;
    off_y += 40 * r_y;

    al_draw_bitmap(sprite, off_x+25, off_y, 0);

    if(DEBUG_VISIBILITY ||
       g.map->tiles[150 * (ch_y + 1) + ch_x].visible == true) {
        g.map->drawTopHalfOfTileAt(ch_x, ch_y + 1);
    }

    // since we're already here, set the character's
    // widget position too.
    pos.x1 = off_x + 25;
    pos.y1 = off_y;
    pos.x2 = 50;
    pos.y2 = 60;
}

void TileMap::handleKeyDown(void) {
    if(g.key == ALLEGRO_KEY_UP)
        if(view_y > 0)
            view_y--;
    if(g.key == ALLEGRO_KEY_LEFT)
        if(view_x > 0)
            view_x--;
    if(g.key == ALLEGRO_KEY_DOWN)
        if(view_y < 150 - rows)
            view_y++;
    if(g.key == ALLEGRO_KEY_RIGHT)
        if(view_x < 150 - cols)
            view_x++;

    char buf[35];
    snprintf(buf, sizeof(buf),
             "Tile map view: x = %d, y = %d", view_x, view_y);
    g.AddMessage(buf);
}

TileMap::~TileMap(void) {
    info("~TileMap");
    // for(auto& bitmap : bitmaps)
    //    al_destroy_bitmap(bitmap);
    //bitmaps.clear();
    //tiles.clear();
}

struct UI {
    vector<Widget *> widgets;

    virtual ~UI() {
        info("~UI()");
        for(auto& widget : widgets)
            delete widget;
    }

    void mouseDownEvent(void);
    void mouseUpEvent(void);
    void keyDownEvent(void);

    void update(void);
    void draw(void);
};

void UI::update(void) {
    for(auto& widget : widgets)
        widget->update();
}

void UI::mouseDownEvent(void) {
    for(auto& widget : widgets) {
        if(widget->pos.x1 <= g.mouse_x &&
           widget->pos.y1 <= g.mouse_y &&
           widget->pos.x1 + widget->pos.x2 >= g.mouse_x &&
           widget->pos.y1 + widget->pos.y2 >= g.mouse_y) {
            widget->mouseDown();
            widget->onMouseDown.emit();
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
            widget->onMouseUp.emit();
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
            widget->onKeyDown.emit();
        }
    }
}
void UI::draw(void) {
    for(auto& widget : widgets) {
        widget->draw();
    }
}

struct Button : public Widget {
    bool pressed;
    ALLEGRO_BITMAP *up;
    ALLEGRO_BITMAP *down;

    ~Button();

    void mouseDown(void) { press(); }
    void mouseUp(void) { }
    void keyDown(void) { }

    void press(void);
    void draw(void);
    void update(void);
};

struct MessageLog : public Widget {
    ALLEGRO_BITMAP *background;
    ALLEGRO_FONT *font;
    vector<string> lines;

    ~MessageLog();

    void mouseDown(void) { }
    void mouseUp(void) { }
    void keyDown(void) { }

    void draw(void);
};

void Game::AddMessage(string str) {
    if(log != NULL)
        log->lines.push_back(str);
}

void Button::press(void) {
    //    pressed = !pressed;
    //    g.AddMessage("pressed button");
}

Button::~Button(void) {
    info("~Button");
}

MessageLog::~MessageLog(void) {
    info("~MessageLog");
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
    if(up != NULL && down != NULL) {
        al_draw_bitmap(pressed ? down : up, pos.x1, pos.y1, 0);
    }
}

void Button::update() {
}

void TileMap::generate(void) {
    tiles.reserve(150*150);
    int up_to = bitmaps.size();
    for(int i = 0; i < 150*150; i++) {
        Tile t;
        t.info_index = rand() % up_to;
        t.visible = false;
        tiles[i] = t;
    }
}

void TileMap::draw(void) {
    // draw the hex grid
    int start = 150 * view_y + view_x;
    int i = 0;
    for(int y = 0; y < rows; y++) {
        for(int x = 0; x < cols; x++) {
            int t = start + (150 * y) + x;
            if(DEBUG_VISIBILITY || tiles[t].visible == true) {
            // if(true) {
                // can the player currently see the tile?
                int currently_seeing = 0;
                for(auto& cs : g.player->currently_seeing) {
                    if(t == cs) {
                        currently_seeing = 1;
                        goto draw;
                    }
                }

            draw:

                int off_x = (i % cols) * 80;
                int off_y = (i + view_x) % 2  == 0 ? 0 : 20;
                off_y = off_y + (40 * floor(i / cols));

                if(currently_seeing != 0) {
                    // if so, draw the tile at full brightness
                    al_draw_bitmap(bitmaps[tile_info[tiles[t].info_index].bitmap_index],
                                   off_x, off_y, 0);
                }
                else {
                    // otherwise draw it 50% tinted

                    al_draw_tinted_bitmap(bitmaps[tile_info[tiles[t].info_index].bitmap_index]
                                          , g.color_tile_tint
                                          , off_x, off_y, 0);
                }
            }
            i++;
        }
    }

    // characters are drawn as part of the map
    // the player is always visible
    g.player->draw();
    for(auto& character : g.characters) {
        for(auto& cs : g.player->currently_seeing) {
            if(DEBUG_VISIBILITY || character->n == cs) {
                character->draw();
                break;
            }
        }
    }
}

// redraws the top half of a tile (the part that overlaps with the
// tile above it). Used for occluding character sprites on the map
void TileMap::drawTopHalfOfTileAt(int x, int y) {
    int n = 150 * y + x;

    int r_x = x - g.map->view_x;
    int r_y = y - g.map->view_y;

    int off_x = 80 * r_x;
    int off_y = n % 2  == 0 ? 0 : 20;
    off_y += 40 * r_y;

    al_draw_bitmap_region(bitmaps[tile_info[tiles[n].info_index].bitmap_index],
                          0, 0, 100, 40, off_x, off_y, 0);
}

void GridSystem::draw(void) {
    for (auto& g : grids)
        g->draw();

    if(held != NULL)
        held->draw();
}

void GridSystem::gsMouseDownEvent() {
    // does any grid contain an item at that position?
    Item *i = NULL;
    for(auto& grid : grids) {
        i = grid->get_item(g.mouse_x, g.mouse_y);
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
}

bool rectIntersect(int a_x, int a_y, int a_width, int a_height,
                   int b_x, int b_y, int b_width, int b_height) {
    return
        !(b_x >= a_x + a_width
          || b_x + b_width <= a_x
          || b_y >= a_y + a_height
          || b_y + b_height <= a_y);
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
        int drop_x =
            ((g.mouse_x - g.hold_off_x) - grid->pos.x1) / 10;
        int drop_y =
            ((g.mouse_y - g.hold_off_y) - grid->pos.y1) / 10;

        // bounds check on grid
        if(g.mouse_x - g.hold_off_x >= grid->pos.x1 &&
           g.mouse_y - g.hold_off_y >= grid->pos.y1 &&
           g.mouse_x - g.hold_off_x + 10 * held->pos.x2
           <= grid->pos.x2 + 8 &&
           g.mouse_y - g.hold_off_y + 10 * held->pos.y2
           <= grid->pos.y2 + 8) {
            // this is the grid we want

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
                    // abort completely
                    goto blocked;
                }
            }

            // we've found the grid and there's nothing blocking
            // the placement there, so drop it
            held->parent = grid;
            held->old_parent = NULL;
            held->pos.x1 = drop_x;
            held->pos.y1 = drop_y;
            grid->items.push_back(held);

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
    held = NULL;

    char b[40];
    snprintf(b, sizeof(b), "Blocked on grid %d", i);
    g.AddMessage(b);
}

void Grid::draw(void) {
    al_draw_filled_rectangle(pos.x1, pos.y1, pos.x2, pos.y2, g.color_grey2);

    for (int x = pos.x1; x <= pos.x2; x = x + 10)
        al_draw_line(x, pos.y1, x, pos.y2, g.color_grey3, 1);
    for (int y = pos.y1; y <= pos.y2; y = y + 10)
        al_draw_line(pos.x1, y, pos.x2, y, g.color_grey3, 1);

    for (auto& i : items)
        i->draw();
}

void Item::draw(void) {
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

Item *Grid::get_item(int x, int y) {
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

struct MainMapUI : public UI {
    MainMapUI();
    ~MainMapUI();
};

struct MiniMapUI : public UI {
    MiniMapUI();
    ~MiniMapUI();
};

struct ItemsUI : public UI {
    GridSystem *gridsystem;

    ItemsUI();
    ~ItemsUI();
};

ItemsUI::ItemsUI() {
    widgets.push_back(g.log);
    widgets.push_back(g.button_MainMap);
    widgets.push_back(g.button_MiniMap);
    widgets.push_back(g.button_Skills);
    widgets.push_back(g.button_Crafting);
    widgets.push_back(g.button_Items);
    widgets.push_back(g.button_Condition);
    widgets.push_back(g.button_Camp);
    widgets.push_back(g.button_Vehicle);
    gridsystem = NULL;
}

ItemsUI::~ItemsUI() {
    info("~ItemsUI");
}

void button_MainMap_press(void) {
    if(g.ui != g.ui_MainMap) {
        g.ui = g.ui_MainMap;
        g.AddMessage("Switched to main map.");
    }
}

void create_inventory_gridsystem(GridSystem *gs);

void button_Items_press(void) {
    if(g.ui != g.ui_Items) {
        g.ui = g.ui_Items;
        GridSystem *gs = g.ui_Items->gridsystem;
        if(gs != NULL) {
            g.ui_Items->gridsystem->grids.clear();
        } else {
            gs = new(GridSystem);
            g.ui_Items->widgets.push_back(gs);
        }

        create_inventory_gridsystem(gs);
        g.ui_Items->gridsystem = gs;
        g.AddMessage("Switched to inventory.");
    }
}

void button_MiniMap_press(void) {
    if(g.ui != g.ui_MiniMap) {
        g.minimap->recreate();
        g.ui = g.ui_MiniMap;
        g.AddMessage("Switched to mini map.");
    }
}

void create_inventory_gridsystem(GridSystem *gs) {
    // add player inventory
    gs->grids.push_back(g.player_inventory);

    // get ground inventory at player position
    Grid *ground = g.map->tiles[g.player->n].ground_items;

    // create it if it doesn't exist
    if(ground == NULL) {
        ground = new(Grid);
        ground->pos.x1 = 10;
        ground->pos.y1 = 10;
        ground->pos.x2 = 210;
        ground->pos.y2 = 310;
        Item *crowbar = new Item(3, 3, 14, 2, g.bitmaps[20]);
        ground->items.push_back(crowbar);
    }

    gs->grids.push_back(ground);
    g.map->tiles[g.player->n].ground_items = ground;

    for(auto& grid : gs->grids)
        for(auto& item : grid->items)
            item->parent = grid;

    gs->pos.x1 = 0;
    gs->pos.y1 = 0;
    gs->pos.x2 = 1280;
    gs->pos.y2 = 720;

    gs->held = NULL;
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
    g.button_MainMap   = new(Button);
    g.button_MiniMap   = new(Button);
    g.button_Skills    = new(Button);
    g.button_Crafting  = new(Button);
    g.button_Items     = new(Button);
    g.button_Condition = new(Button);
    g.button_Camp      = new(Button);
    g.button_Vehicle   = new(Button);

    // left
    g.button_MainMap->pos.x1 = 0;
    g.button_MainMap->pos.y1 = 480;
    g.button_MainMap->pos.x2 = 100;
    g.button_MainMap->pos.y2 = 60;
    g.button_MainMap->up = g.bitmaps[0];
    g.button_MainMap->down = g.bitmaps[1];
    g.button_MainMap->pressed = false;
    g.button_MainMap->onMouseDown.connect(ptr_fun(button_MainMap_press));

    g.button_MiniMap->pos.x1 = 0;
    g.button_MiniMap->pos.y1 = 540;
    g.button_MiniMap->pos.x2 = 100;
    g.button_MiniMap->pos.y2 = 60;
    g.button_MiniMap->up = g.bitmaps[2];
    g.button_MiniMap->down = g.bitmaps[3];
    g.button_MiniMap->pressed = false;
    g.button_MiniMap->onMouseDown.connect(ptr_fun(button_MiniMap_press));

    g.button_Skills->pos.x1 = 0;
    g.button_Skills->pos.y1 = 600;
    g.button_Skills->pos.x2 = 100;
    g.button_Skills->pos.y2 = 60;
    g.button_Skills->up = g.bitmaps[4];
    g.button_Skills->down = g.bitmaps[5];
    g.button_Skills->pressed = false;

    g.button_Crafting->pos.x1 = 0;
    g.button_Crafting->pos.y1 = 660;
    g.button_Crafting->pos.x2 = 100;
    g.button_Crafting->pos.y2 = 60;
    g.button_Crafting->up = g.bitmaps[6];
    g.button_Crafting->down = g.bitmaps[7];
    g.button_Crafting->pressed = false;

    // right
    g.button_Items->pos.x1 = 1180;
    g.button_Items->pos.y1 = 280;
    g.button_Items->pos.x2 = 100;
    g.button_Items->pos.y2 = 60;
    g.button_Items->up = g.bitmaps[8];
    g.button_Items->down = g.bitmaps[9];
    g.button_Items->pressed = false;
    g.button_Items->onMouseDown.connect(ptr_fun(button_Items_press));

    g.button_Condition->pos.x1 = 1180;
    g.button_Condition->pos.y1 = 340;
    g.button_Condition->pos.x2 = 100;
    g.button_Condition->pos.y2 = 60;
    g.button_Condition->up = g.bitmaps[10];
    g.button_Condition->down = g.bitmaps[11];
    g.button_Condition->pressed = false;

    g.button_Camp->pos.x1 = 1180;
    g.button_Camp->pos.y1 = 400;
    g.button_Camp->pos.x2 = 100;
    g.button_Camp->pos.y2 = 60;
    g.button_Camp->up = g.bitmaps[12];
    g.button_Camp->down = g.bitmaps[13];
    g.button_Camp->pressed = false;

    g.button_Vehicle->pos.x1 = 1180;
    g.button_Vehicle->pos.y1 = 460;
    g.button_Vehicle->pos.x2 = 100;
    g.button_Vehicle->pos.y2 = 60;
    g.button_Vehicle->up = g.bitmaps[14];
    g.button_Vehicle->down = g.bitmaps[15];
    g.button_Vehicle->pressed = false;
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
    g.map = new(TileMap);

    g.map->cols = 18;
    g.map->rows = 20;
    g.map->view_x = 4;
    g.map->view_y = 4;
    g.map->pos.x1 = 0;
    g.map->pos.y1 = 0;
    g.map->pos.x2 = 1280;
    g.map->pos.y2 = 720;

    TileInfo i;
    i.minimap_color = al_map_rgb(0, 255, 0);
    i.bitmap_index = 0;
    g.map->tile_info.push_back(i);
    i.minimap_color = al_map_rgb(0, 150, 0);
    i.bitmap_index = 1;
    g.map->tile_info.push_back(i);
    i.minimap_color = al_map_rgb(255, 255, 255);
    i.bitmap_index = 2;
    g.map->tile_info.push_back(i);

    g.map->bitmaps.push_back(g.bitmaps[17]);
    g.map->bitmaps.push_back(g.bitmaps[18]);
    g.map->bitmaps.push_back(g.bitmaps[19]);

    g.map->generate();
}

void init_minimap(void) {
    g.minimap = new(MiniMap);
    g.minimap->pos.x1 = 490;
    g.minimap->pos.y1 = 210;
    g.minimap->pos.x2 = g.minimap->pos.x1 + 300;
    g.minimap->pos.y2 = g.minimap->pos.y1 + 300;
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
}

MiniMapUI::~MiniMapUI(void) {
    info("~TestUI2()");
}

// creates the player and npcs
// must be called after init_tilemap();
void init_characters(void) {
    for(int i = 0; i < 10; i++) {
        int n = (rand() % 30) * 150 + (rand() % 30);
        Character *c = new Character(n, g.bitmaps[21]);
        g.characters.push_back(c);
    }

    int n = 12 * 150 + 12;
    g.player = new Character(n, g.bitmaps[21]);

    // player's inventory
    Grid *inventory = new(Grid);
    inventory->pos.x1 = 300;
    inventory->pos.y1 = 10;
    inventory->pos.x2 = 510;
    inventory->pos.y2 = 310;

    // add starting items
    Item *first_aid_kit = new Item(3, 3, 6, 6, g.bitmaps[22]);
    inventory->items.push_back(first_aid_kit);

    g.player_inventory = inventory;
    g.player->update_visibility();
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

    g.display = al_create_display(1280, 720);
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

    load_bitmaps();
    init_tilemap();
    init_characters();
    init_buttons();
    init_messagelog();
    init_minimap();

    {
        g.ui_MainMap = new(MainMapUI);
        g.ui_MiniMap = new(MiniMapUI);
        g.ui_Items   = new(ItemsUI);

        // start on the main map
        g.ui = g.ui_MainMap;
    }

    bool redraw = true;
    bool was_mouse_down = false;

    // main loop
    while(1) {
        al_get_mouse_state(&mouse_state);
        al_get_keyboard_state(&keyboard_state);

        g.mouse_x = mouse_state.x;
        g.mouse_y = mouse_state.y;

        if (mouse_state.buttons & 1) {
            if (!was_mouse_down) {
                // mouse down event
                g.ui->mouseDownEvent();
                was_mouse_down = true;
            }
        }
        else if (was_mouse_down) {
            g.ui->mouseUpEvent();
            // mouse up event
            was_mouse_down = false;
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
            al_clear_to_color(g.color_black);

            { // drawing goes here
                g.ui->draw();
            }
            al_flip_display();
        }
    }

    unload_bitmaps();

    /*
      we can't do this since the UI's share widgets
      delete g.ui_MainMap;
      delete g.ui_MiniMap;
      delete g.ui_Items;
    */

    /*
      allegro is automatically unloaded
      al_uninstall_system();
    */

    return 0;
}
