#include <allegro5/allegro.h>
#include <allegro5/allegro_color.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>

#include <stdint.h>

#include <vector>
#include <set>
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
struct HardpointInfo;

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
    ALLEGRO_COLOR color_bg;

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

    HardpointInfo *right_hand;
    HardpointInfo *left_hand;

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

    set<Widget *> all_widgets;
    set<Button *> main_buttons;
};

Game g;

struct Rect {
    float x1;
    float y1;
    float x2;
    float y2;
};

enum ItemKind {
    Container,
    TorsoClothing,
    Weapon,
    // etc
};

struct Item {
    Rect pos;
    string name;
    Grid *parent;
    Grid *old_parent;
    Grid *storage;
    ALLEGRO_BITMAP *sprite;
    ItemKind kind;

    Item(int x1, int y1, int x2, int y2, ALLEGRO_BITMAP *sprite, ItemKind k);
    ~Item() {
        // Item doesn't own anything
        info("~Item()");
    }

    void draw(void);
};

Item::Item(int x1, int y1, int x2, int y2, ALLEGRO_BITMAP *img, ItemKind k) {
    kind = k;
    pos.x1 = x1;
    pos.y1 = y1;
    pos.x2 = x2;
    pos.y2 = y2;
    name = "";
    sprite = img;
    storage = NULL;
}

struct Widget {
    Rect pos;
    signal<void> onMouseDown;
    signal<void> onMouseUp;
    signal<void> onKeyDown;

    virtual void mouseDown(void) = 0;
    virtual void mouseUp(void) = 0;
    virtual void keyDown(void) = 0;

    Widget() {
        g.all_widgets.insert(this);
    }

    virtual ~Widget() {
        info("~Widget()");
    };

    virtual void draw(void) = 0;
    virtual void update() { };
};

struct HardpointInfo {
    ALLEGRO_BITMAP *sprite;
    bool singleItem;
};

void init_hardpointinfo(void) {
    g.right_hand = new HardpointInfo;
    g.right_hand->singleItem = true;
    g.right_hand->sprite = g.bitmaps[22];

    g.left_hand = new HardpointInfo;
    g.left_hand->singleItem = true;
    g.left_hand->sprite = g.bitmaps[22];
}

struct Grid {
    // "widget" dimensions
    Rect pos;

    // grid spacing in pixels
    int grid_px_x;
    int grid_px_y;

    // grid size in grid units
    int grid_size_x;
    int grid_size_y;

    HardpointInfo *hpinfo;
    vector<Item *> items;

    Grid(int w_pos_x, int w_pos_y, int size_x, int size_y, HardpointInfo *h) {
        hpinfo = h;
        grid_px_x = 10;
        grid_px_y = 10;
        grid_size_x = size_x;
        grid_size_y = size_y;
        pos.x1 = w_pos_x;
        pos.y1 = w_pos_y;
        pos.x2 = pos.x1 + size_x * grid_px_x;
        pos.y2 = pos.y1 + size_y * grid_px_y;
    }
    ~Grid() {
        info("~Grid()");
        // a grid owns its items
        for(auto& item : items)
            delete item;
    }

    void draw(void);
    void resetPos(void) {
        pos.x2 = pos.x1 + grid_size_x * grid_px_x;
        pos.y2 = pos.y1 + grid_size_y * grid_px_y;
    }
    Item *get_item(int x, int y); // get item at screen position
};

struct Character : public Widget {
    int n; // position by offset
    Grid *inventory;
    Grid *right_hand;
    Grid *left_hand;

    ALLEGRO_BITMAP *sprite;
    vector<int> currently_seeing;

    void update_visibility(void);

    Character(int n, ALLEGRO_BITMAP *sprite);

    ~Character() {
        delete inventory;
        info("~Character()");
    };

    void mouseDown(void) {
        g.AddMessage("That tickles!");
    }
    void mouseUp(void) { }
    void keyDown(void) { }

    void draw(void);
};

Character::Character(int n, ALLEGRO_BITMAP *sprite) {
    this->n = n;
    this->sprite = sprite;
    currently_seeing.reserve(50);
}

int dir_transform(int n, int dir);

struct GridSystem : public Widget {
    vector<Grid *> grids;
    Item *held;

    GridSystem(void) { }
    ~GridSystem(void) {
        delete held;
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

    vector<Tile> tiles;
    vector<TileInfo> tile_info;
    vector<ALLEGRO_BITMAP *>bitmaps;

    TileMap(int sx, int sy);
    ~TileMap() {
        // tilemap doesn't own anything
        info("~TileMap()");
    }

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
    void focusOnPlayer(void);
};

TileMap::TileMap(int sx, int sy) {
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

    hex_size_x = 100;
    hex_size_y = 80;

    hex_step_x = 80;
    hex_step_y = 40;

    r_off_x = 0;
    r_off_y = 0;

    if(size_x * hex_step_x < g.display_x)
        r_off_x = (g.display_x - (size_x + 0.5) * hex_step_x) / 2;
    if(size_y * hex_step_y < g.display_y)
        r_off_y = (g.display_y - size_y * hex_step_y) / 3;
}

void TileMap::focusOnPlayer(void) {
    int p_x = g.player->n % size_x;
    int p_y = g.player->n / size_x;
    view_x = max(0, p_x - cols/2 + 1);
    view_y = max(0, p_y - rows/2 + 2);
    view_x = max(0, min(view_x, g.map->size_x - cols));
    view_y = max(0, min(view_y, g.map->size_y - rows));
}

struct MiniMap : public Widget {
    MiniMap();
    ~MiniMap();

    int size_x;
    int size_y;

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
        int p_x = g.player->n % g.map->size_x;
        int p_y = g.player->n / g.map->size_x;
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
    int off_x = (g.display_x - size_x) / 2;
    int off_y = (g.display_y - size_y) / 3;

    al_draw_bitmap(buf, off_x, off_y, 0);
}

void TileMap::mouseToTileXY(int &x, int &y) {
    x = view_x + floor((g.mouse_x - r_off_x) / 80);
    int off_y = x % 2 == 0 ? 0 : 20;
    y = view_y + floor((g.mouse_y - 40 - off_y - r_off_y) / 40);
}

int TileMap::mouseToTileN(void) {
    int x, y;
    mouseToTileXY(x, y);
    return size_x * y + x;
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
    int n_max = g.map->size_x * g.map->size_y;

    for(int dir = 0; dir <= 6; dir++) {
        int n2 = dir_transform(n, dir);
        if(n2 != -1) {
            for(int dir2 = 0; dir2 <= 6; dir2++) {
                int n3 = dir_transform(n2, dir2);
                if(n3 != -1) {
                    // bounds check
                    if(n3 >= 0 && n3 < n_max) {
                        g.map->tiles[n3].visible = true;
                        currently_seeing.push_back(n3);
                    }
                }
            }
        }
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
    int ch_x = n % g.map->size_x;
    int ch_y = n / g.map->size_x;

    // is it on the screen?
    if(ch_y >= g.map->view_y + g.map->rows
       || ch_x >= g.map->view_x + g.map->cols
       || ch_y < g.map->view_y
       || ch_x < g.map->view_x) {
        return;
    }

    if(sprite == NULL) {
        info("WARNING: character doesn't have a sprite.");
        return;
    }

    int r_x = ch_x - g.map->view_x;
    int r_y = ch_y - g.map->view_y;

    int off_x = 80 * r_x;
    int off_y = n % 2  == 0 ? 0 : 20;
    off_y += 40 * r_y;

    al_draw_bitmap(sprite,
                   g.map->r_off_x + off_x+25,
                   g.map->r_off_y + off_y, 0);

    if(DEBUG_VISIBILITY ||
       g.map->tiles[g.map->size_x * (ch_y + 1) + ch_x].visible == true) {
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
        if(view_y < size_y - rows)
            view_y++;
    if(g.key == ALLEGRO_KEY_RIGHT)
        if(view_x < size_x - cols)
            view_x++;
    if(g.key == ALLEGRO_KEY_C)
        g.map->focusOnPlayer();

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

    Button() {
        pressed = false;
        up = NULL;
        down = NULL;
    }
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
    if(up != NULL && down != NULL) {
        al_draw_bitmap(pressed ? down : up, pos.x1, pos.y1, 0);
    }
}

void Button::update() {
}

void TileMap::generate(void) {
    tiles.reserve(size_x * size_y);
    int up_to = bitmaps.size();
    Tile t;
    for(int i = 0; i < size_x * size_y; i++) {
        t.info_index = rand() % up_to;
        t.visible = false;
        tiles[i] = t;
    }
    info("Finished generating map");
}

void TileMap::draw(void) {
    // draw the hex grid
    int start = size_x * view_y + view_x;
    int max_t = size_x * size_y;

    // the map might be smaller than the requested view size
    int r_cols = min(size_x, cols);
    int r_rows = min(size_y, rows);

    int i = 0;
    for(int y = 0; y < r_rows; y++) {
        for(int x = 0; x < r_cols; x++) {
            int t = start + (size_x * y) + x;

            if(t < 0 || t > max_t) {
                info("WARNING: tried to view tile that is out of bounds");
                t = 0;
            }

            if(DEBUG_VISIBILITY || tiles[t].visible == true) {
                // can the player currently see the tile?
                int currently_seeing = 0;
                for(auto& cs : g.player->currently_seeing) {
                    if(t == cs) {
                        currently_seeing = 1;
                        goto draw;
                    }
                }

            draw:

                int off_x = (i % r_cols) * 80;
                int off_y = (i + view_x) % 2  == 0 ? 0 : 20;
                off_y = off_y + (40 * floor(i / r_cols));

                if(currently_seeing != 0) {
                    // if so, draw the tile at full brightness
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
            }
            i++;
        }
    }

    // characters are drawn as part of the map
    // the player is always visible
    if(g.player == NULL)
        info("WARNING: no player");
    else
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
    int n = size_x * y + x;

    int r_x = x - g.map->view_x;
    int r_y = y - g.map->view_y;

    if(r_y >= g.map->size_y)
        return;

    int off_x = 80 * r_x;
    int off_y = n % 2  == 0 ? 0 : 20;
    off_y += 40 * r_y;

    al_draw_bitmap_region(bitmaps[tile_info[tiles[n].info_index].bitmap_index],
                          0, 0,
                          100, 40,
                          r_off_x + off_x,
                          r_off_y + off_y,
                          0);
    // TODO: check if there's a character on that tile and
    // redraw the top half of the sprite
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
                        // abort completely
                        goto blocked;
                    }
                }
            } // or a hardpoint?
            else if(grid->hpinfo->singleItem) {
                int s = grid->items.size();
                // some hardpoints only accept one item
                if(s >= 1)
                    goto blocked;
            }
                
            // we've found the grid and there's nothing blocking
            // the placement there, so drop it
            held->parent = grid;
            held->old_parent = NULL;
            held->pos.x1 = drop_x;
            held->pos.y1 = drop_y;
            grid->items.push_back(held);

            // does the item have its own grid?
            if(held->storage != NULL &&
               grid->hpinfo != NULL) {
                // ^^ we only want to display it if it's on a hardpoint
                held->storage->pos.x1 = grid->pos.x2 + 20;
                held->storage->pos.y1 = grid->pos.y1;
                held->storage->resetPos();
                    
                grids.push_back(held->storage);
            }

            held = NULL;

            char b[40];
            snprintf(b, sizeof(b), "Moved onto grid %d %d %d", i, drop_x, drop_y);
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
    if(hpinfo == NULL) {
        for (int x = pos.x1; x <= pos.x2; x = x + grid_px_x)
            al_draw_line(x, pos.y1, x, pos.y2, g.color_grey3, 1);
        for (int y = pos.y1; y <= pos.y2; y = y + grid_px_y)
            al_draw_line(pos.x1, y, pos.x2, y, g.color_grey3, 1);
    }
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

struct InventoryGridSystem;

struct ItemsUI : public UI {
    InventoryGridSystem *gridsystem;

    ItemsUI();
    ~ItemsUI();
};

struct InventoryGridSystem : public GridSystem {
    InventoryGridSystem() { };
    ~InventoryGridSystem() {
        info("InventoryGridSystem()");
    };

    void init(void);
    void draw(void) {
        al_draw_text(g.font, g.color_white, 80, 35, 0, "Ground:");
        al_draw_text(g.font, g.color_white, 760, 35, 0, "Inventory:");
        GridSystem::draw();
    }
};

void InventoryGridSystem::init(void) {
    // add player inventory
    grids.push_back(g.player->inventory);
    grids.push_back(g.player->right_hand);
    grids.push_back(g.player->left_hand);
    // get ground inventory at player position
    Grid *ground = g.map->tiles[g.player->n].ground_items;

    // create it if it doesn't exist
    if(ground == NULL) {
        ground = new Grid (20, 50, 20, 30, NULL);
        Item *crowbar = new Item(3, 3, 14, 2, g.bitmaps[20], Weapon);
        ground->items.push_back(crowbar);
    }

    grids.push_back(ground);
    g.map->tiles[g.player->n].ground_items = ground;

    for(auto& grid : grids)
        for(auto& item : grid->items)
            item->parent = grid;

    pos.x1 = 0;
    pos.y1 = 0;
    pos.x2 = 1280;
    pos.y2 = 720;

    held = NULL;
}

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
    info("~ItemsUI()");
}

void main_buttons_update(void) {
    for(auto& b : g.main_buttons)
        b->pressed = false;
    if(g.ui == g.ui_MainMap)
        g.button_MainMap->pressed = true;
    if(g.ui == g.ui_MiniMap)
        g.button_MiniMap->pressed = true;
    if(g.ui == g.ui_Items)
        g.button_Items->pressed = true;
}

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
        InventoryGridSystem *gs = g.ui_Items->gridsystem;

        if(gs != NULL) {
            gs->grids.clear();
            g.AddMessage("Using old grid system.");
        } else {
            gs = new(InventoryGridSystem);
            g.ui_Items->widgets.push_back(gs);
            g.AddMessage("Created inventory grid system.");
        }

        gs->init();
        g.ui_Items->gridsystem = gs;
        g.ui = g.ui_Items;
        g.color_bg = g.color_grey;
        g.AddMessage("Switched to inventory.");
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
    g.map = new TileMap (6, 6);

    g.map->cols = 16;
    g.map->rows = 16;
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
    i.minimap_color = al_map_rgb(0, 0, 200);
    i.bitmap_index = 3;
    g.map->tile_info.push_back(i);

    g.map->bitmaps.push_back(g.bitmaps[17]);
    g.map->bitmaps.push_back(g.bitmaps[18]);
    g.map->bitmaps.push_back(g.bitmaps[19]);
    g.map->bitmaps.push_back(g.bitmaps[23]);

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
    info("~MiniMapUI()");
}

// creates the player and npcs
// must be called after init_tilemap();
void init_characters(void) {
    for(int i = 0; i < 5; i++) {
        int mx = g.map->size_x;
        int my = g.map->size_y;
        int n = (rand() % my) * mx + (rand() % mx);

        // create an inventory
        Grid *inventory = new Grid (700, 50, 20, 30, NULL);

        // add starting items
        Item *first_aid_kit1 = new Item(3, 3, 6, 6, g.bitmaps[22], Container);
        Grid *first_aid_kit_grid1 = new Grid(0, 0, 8, 8, NULL);
        Item *first_aid_kit2 = new Item(7, 3, 6, 6, g.bitmaps[22], Container);
        Grid *first_aid_kit_grid2 = new Grid(0, 0, 8, 8, NULL);
        first_aid_kit1->storage = first_aid_kit_grid1;
        first_aid_kit2->storage = first_aid_kit_grid2;
        inventory->items.push_back(first_aid_kit1);
        inventory->items.push_back(first_aid_kit2);

        Character *c = new Character(n, g.bitmaps[21]);
        c->right_hand = new Grid (400, 200, 5, 5, g.right_hand);
        c->left_hand = new Grid (400, 320, 5, 5, g.left_hand);
        c->inventory = inventory;

        // player is character 0
        if(i == 0)
            g.player = c;
        else // everyone else is an NPC
            g.characters.push_back(c);
    }
    g.player->update_visibility();
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
    g.color_bg = g.color_black;

    load_bitmaps();
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

        // start on the main map
        g.ui = g.ui_MainMap;
        main_buttons_update();
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
            al_clear_to_color(g.color_bg);

            { // drawing goes here
                g.ui->draw();
            }
            al_flip_display();
        }
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
