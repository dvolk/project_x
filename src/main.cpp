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

void errorQuit(string str) {
    cout << "Error: " << str << endl;
    exit(1);
}

void info(string str) {
    cout << "Info: " << str << endl;
}

// global state
struct Game {
    ALLEGRO_DISPLAY *display;
    ALLEGRO_FONT *font;

    ALLEGRO_COLOR color_white;
    ALLEGRO_COLOR color_black;
    ALLEGRO_COLOR color_grey;
    ALLEGRO_COLOR color_grey2;
    ALLEGRO_COLOR color_grey3;

    Button *button_MainMap;
    Button *button_MiniMap;
    Button *button_Skills;
    Button *button_Crafting;
    Button *button_Items;
    Button *button_Condition;
    Button *button_Camp;
    Button *button_Vehicle;

    TileMap *map;
    vector<Character *> characters;

    UI *ui; // current UI

    UI *ui_MainMap;
    UI *ui_MiniMap;
    UI *ui_Skills;
    UI *ui_Crafting;
    UI *ui_Items;
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
    Rect pos;
    string name;
    Grid *parent;
    void draw(void);
};

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

    ~Character(void) {
        info("~Character");
        al_destroy_bitmap(sprite);
    }

    void mouseDown(void) {
        g.AddMessage("That tickles!");
    }
    void mouseUp(void) { }
    void keyDown(void) { }

    void draw(void);
};

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
        for(auto& grid : grids)
            delete grid;
    }

    void mouseDown(void) { gsMouseDownEvent(); }
    void mouseUp(void) { gsMouseUpEvent(); }
    void keyDown(void) { }

    void gsMouseDownEvent(void);
    void gsMouseUpEvent(void);
    void draw(void);
};

struct TestGridSystem : public GridSystem {
    TestGridSystem();
    ~TestGridSystem();
};

struct Tile {
    int16_t bitmap_index;
};

struct TileMap : public Widget {
    int cols; // display dimensions
    int rows; // display dimensions
    int view_x;
    int view_y;
    vector<Tile> tiles;
    vector<ALLEGRO_BITMAP *>bitmaps;

    ~TileMap(void);

    void handleKeyDown(void);

    void mouseDown(void) { }
    void mouseUp(void) { }
    void keyDown(void) { handleKeyDown(); }

    // must be called after the bitmaps vector is filled
    void generate(void);
    void draw(void);
    void drawTopHalfOfTileAt(int x, int y);
};

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

    g.map->drawTopHalfOfTileAt(ch_x, ch_y + 1);

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
    for(auto& bitmap : bitmaps)
        al_destroy_bitmap(bitmap);
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
    pressed = !pressed;
    g.AddMessage("pressed button");
}

Button::~Button(void) {
    info("~Button");
    al_destroy_bitmap(up);
    al_destroy_bitmap(down);
}

MessageLog::~MessageLog(void) {
    info("~MessageLog");
    al_destroy_bitmap(background);
    //free(font);
    //lines.clear();
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
        t.bitmap_index = rand() % up_to;
        tiles[i] = t;
    }
    Tile t;
    t.bitmap_index = 2;
    tiles[10*150+60] = t;
    tiles[10*150+61] = t;
    tiles[11*150+61] = t;
}

void TileMap::draw(void) {
    // draw the hex grid
    int start = 150 * view_y + view_x;
    int i = 0;
    for(int y = 0; y < rows; y++) {
        for(int x = 0; x < cols; x++) {
            int t = start + (150 * y) + x;
            int off_x = (i % cols) * 80;
            int off_y = (i + view_x) % 2  == 0 ? 0 : 20;
            off_y = off_y + (40 * floor(i / cols));

            al_draw_bitmap(bitmaps[tiles[t].bitmap_index], off_x, off_y, 0);
            i++;
        }
    }
}

void TileMap::drawTopHalfOfTileAt(int x, int y) {
    int n = 150 * y + x;

    int r_x = x - g.map->view_x;
    int r_y = y - g.map->view_y;

    int off_x = 80 * r_x;
    int off_y = n % 2  == 0 ? 0 : 20;
    off_y += 40 * r_y;

    al_draw_bitmap_region(bitmaps[tiles[n].bitmap_index],
                          0, 0, 100, 40, off_x, off_y, 0);
}

void GridSystem::draw(void) {
    for (auto& g : grids)
        g->draw();

    if(held != NULL)
        held->draw();
}

void GridSystem::gsMouseDownEvent() {
    Item *i = grids[0]->get_item(g.mouse_x, g.mouse_y);
    if(i != NULL) {
        held = i;
        g.hold_off_x =
            g.mouse_x - (i->parent->pos.x1 + i->pos.x1 * 10);
        g.hold_off_y =
            g.mouse_y - (i->parent->pos.y1 + i->pos.y1 * 10);
        held->parent = NULL;
    }
}

void GridSystem::gsMouseUpEvent() {
    if(held != NULL) {
        // mouse is holding an item
        int drop_x =
            ((g.mouse_x - g.hold_off_x) - grids[0]->pos.x1) / 10;
        int drop_y =
            ((g.mouse_y - g.hold_off_y) - grids[0]->pos.y1) / 10;

        held->parent = grids[0];
        held->pos.x1 = drop_x;
        held->pos.y1 = drop_y;
        grids[0]->items.push_back(held);
        //delete held;
        held = NULL;
        g.AddMessage("You moved the block! Congratulations!");
    }
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
    ALLEGRO_COLOR col = al_map_rgb(100, 0, 100);

    if(parent != NULL) {
        // we're on a grid
        al_draw_filled_rectangle(parent->pos.x1 + pos.x1 * 10,
                                 parent->pos.y1 + pos.y1 * 10,
                                 parent->pos.x1 + (pos.x1 + pos.x2) * 10,
                                 parent->pos.y1 + (pos.y1 + pos.y2) * 10,
                                 col);
    } else {
        // we're held by the mouse
        al_draw_filled_rectangle(g.mouse_x - g.hold_off_x,
                                 g.mouse_y - g.hold_off_y,
                                 g.mouse_x - g.hold_off_x + pos.x2 * 10,
                                 g.mouse_y - g.hold_off_y + pos.y2 * 10,
                                 col);
    }

}

Item *Grid::get_item(int x, int y) {
    int c = 0;
    for (auto& i : items) {
        if(i->parent->pos.x1 + i->pos.x1 * 10 <= x &&
           i->parent->pos.y1 + i->pos.y1 * 10 <= y &&
           i->parent->pos.x1 + (i->pos.x1 + i->pos.x2) * 10 >= x &&
           i->parent->pos.y1 + (i->pos.y1 + i->pos.y2) * 10 >= y) {

            // humm
            Item *a = new(Item);
            *a = *items[c];
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

void switch_ui(void) {
    if(g.ui == g.ui_MainMap) {
        g.ui = g.ui_MiniMap;
    } else {
        g.ui = g.ui_MainMap;
    }
}

void button_MainMap_press(void) {
    for(auto& character : g.characters) {
        character->n = dir_transform(character->n, rand() % 6);
    }
    g.AddMessage("Turn ended!");
}

void init_buttons(void) {
    // left
    ALLEGRO_BITMAP *button_mainmap_up = al_load_bitmap("media/buttons/mainmap_up.png");
    ALLEGRO_BITMAP *button_mainmap_down = al_load_bitmap("media/buttons/mainmap_down.png");
    ALLEGRO_BITMAP *button_minimap_up = al_load_bitmap("media/buttons/minimap_up.png");
    ALLEGRO_BITMAP *button_minimap_down = al_load_bitmap("media/buttons/minimap_down.png");
    ALLEGRO_BITMAP *button_skills_up = al_load_bitmap("media/buttons/skills_up.png");
    ALLEGRO_BITMAP *button_skills_down = al_load_bitmap("media/buttons/skills_down.png");
    ALLEGRO_BITMAP *button_crafting_up = al_load_bitmap("media/buttons/crafting_up.png");
    ALLEGRO_BITMAP *button_crafting_down = al_load_bitmap("media/buttons/crafting_down.png");

    // right
    ALLEGRO_BITMAP *button_items_up = al_load_bitmap("media/buttons/items_up.png");
    ALLEGRO_BITMAP *button_items_down = al_load_bitmap("media/buttons/items_down.png");
    ALLEGRO_BITMAP *button_condition_up = al_load_bitmap("media/buttons/condition_up.png");
    ALLEGRO_BITMAP *button_condition_down = al_load_bitmap("media/buttons/condition_down.png");
    ALLEGRO_BITMAP *button_camp_up = al_load_bitmap("media/buttons/camp_up.png");
    ALLEGRO_BITMAP *button_camp_down = al_load_bitmap("media/buttons/camp_down.png");
    ALLEGRO_BITMAP *button_vehicle_up = al_load_bitmap("media/buttons/vehicle_up.png");
    ALLEGRO_BITMAP *button_vehicle_down = al_load_bitmap("media/buttons/vehicle_down.png");

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
    g.button_MainMap->up = button_mainmap_up;
    g.button_MainMap->down = button_mainmap_down;
    g.button_MainMap->pressed = false;
    g.button_MainMap->onMouseDown.connect(ptr_fun(button_MainMap_press));

    g.button_MiniMap->pos.x1 = 0;
    g.button_MiniMap->pos.y1 = 540;
    g.button_MiniMap->pos.x2 = 100;
    g.button_MiniMap->pos.y2 = 60;
    g.button_MiniMap->up = button_minimap_up;
    g.button_MiniMap->down = button_minimap_down;
    g.button_MiniMap->pressed = false;

    g.button_Skills->pos.x1 = 0;
    g.button_Skills->pos.y1 = 600;
    g.button_Skills->pos.x2 = 100;
    g.button_Skills->pos.y2 = 60;
    g.button_Skills->up = button_skills_up;
    g.button_Skills->down = button_skills_down;
    g.button_Skills->pressed = false;

    g.button_Crafting->pos.x1 = 0;
    g.button_Crafting->pos.y1 = 660;
    g.button_Crafting->pos.x2 = 100;
    g.button_Crafting->pos.y2 = 60;
    g.button_Crafting->up = button_crafting_up;
    g.button_Crafting->down = button_crafting_down;
    g.button_Crafting->pressed = false;

    // right
    g.button_Items->pos.x1 = 1180;
    g.button_Items->pos.y1 = 280;
    g.button_Items->pos.x2 = 100;
    g.button_Items->pos.y2 = 60;
    g.button_Items->up = button_items_up;
    g.button_Items->down = button_items_down;
    g.button_Items->pressed = false;

    g.button_Condition->pos.x1 = 1180;
    g.button_Condition->pos.y1 = 340;
    g.button_Condition->pos.x2 = 100;
    g.button_Condition->pos.y2 = 60;
    g.button_Condition->up = button_condition_up;
    g.button_Condition->down = button_condition_down;
    g.button_Condition->pressed = false;

    g.button_Camp->pos.x1 = 1180;
    g.button_Camp->pos.y1 = 400;
    g.button_Camp->pos.x2 = 100;
    g.button_Camp->pos.y2 = 60;
    g.button_Camp->up = button_camp_up;
    g.button_Camp->down = button_camp_down;
    g.button_Camp->pressed = false;

    g.button_Vehicle->pos.x1 = 1180;
    g.button_Vehicle->pos.y1 = 460;
    g.button_Vehicle->pos.x2 = 100;
    g.button_Vehicle->pos.y2 = 60;
    g.button_Vehicle->up = button_vehicle_up;
    g.button_Vehicle->down = button_vehicle_down;
    g.button_Vehicle->pressed = false;
}

void init_messagelog(void) {
    ALLEGRO_BITMAP *messagelogbg = al_load_bitmap("media/backgrounds/messagelogbg.png");

    g.log = new(MessageLog);
    g.log->pos.x1 = 100;
    g.log->pos.y1 = 520;
    g.log->pos.x2 = 1080;
    g.log->pos.y2 = 200;
    g.log->background = messagelogbg;
    g.log->font = g.font;
    g.log->lines.push_back("Player is dying.");
    g.log->lines.push_back("Player is dead. Blegrgrgrvddgd...");
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

    ALLEGRO_BITMAP *tile_grass = al_load_bitmap("media/tile/grass.png");
    ALLEGRO_BITMAP *tile_tree = al_load_bitmap("media/tile/tree.png");
    ALLEGRO_BITMAP *tile_city = al_load_bitmap("media/tile/city.png");

    g.map->bitmaps.push_back(tile_grass);
    g.map->bitmaps.push_back(tile_tree);
    g.map->bitmaps.push_back(tile_city);

    g.map->generate();
}

MiniMapUI::MiniMapUI(void) {
}

MiniMapUI::~MiniMapUI(void) {
    info("~TestUI2()");
}

TestGridSystem::TestGridSystem() {
    Grid *g1 = new(Grid);
    g1->pos.x1 = 10;
    g1->pos.y1 = 10;
    g1->pos.x2 = 200;
    g1->pos.y2 = 300;
    Grid *g2 = new(Grid);
    g2->pos.x1 = 250;
    g2->pos.y1 = 10;
    g2->pos.x2 = 450;
    g2->pos.y2 = 200;
    Item *i1 = new(Item);
    i1->pos.x1 = 3;
    i1->pos.y1 = 3;
    i1->pos.x2 = 7;
    i1->pos.y2 = 1;
    i1->name = "item 1";
    Item *i2 = new(Item);
    i2->pos.x1 = 3;
    i2->pos.y1 = 10;
    i2->pos.x2 = 4;
    i2->pos.y2 = 4;
    i2->name = "item 2";

    g1->items.push_back(i1);
    g1->items.push_back(i2);
    grids.push_back(g1);
    grids.push_back(g2);

    // REALLY set item parents
    int i = 0;
    for(auto& grid : grids) {
        int j = 0;
        for(auto& item : grid->items) {
            grids[i]->items[j]->parent = grids[i];
            j++;
        }
        i++;
    }

    pos.x1 = 0;
    pos.y1 = 0;
    pos.x2 = 1280;
    pos.y2 = 720;

    held = NULL;
}

TestGridSystem::~TestGridSystem() {
    info("~TestGridSystem()");
    //grids.clear();
}

MainMapUI::MainMapUI() {
    ALLEGRO_BITMAP *ch_sprite = al_load_bitmap("media/characters/test_character.png");
    Character *c = new(Character);
    c->n = 10 * 150 + 10;
    c->sprite = ch_sprite;
    Character *c1 = new(Character);
    c1->n = 10 * 150 + 11;
    c1->sprite = ch_sprite;
    Character *c2 = new(Character);
    c2->n = 10 * 150 + 12;
    c2->sprite = ch_sprite;

    widgets.push_back(g.map);

    g.characters.push_back(c);
    widgets.push_back(c);
    g.characters.push_back(c1);
    widgets.push_back(c1);
    g.characters.push_back(c2);
    widgets.push_back(c2);

    widgets.push_back(g.log);
    widgets.push_back(g.button_MainMap);
    widgets.push_back(g.button_MiniMap);
    widgets.push_back(g.button_Skills);
    widgets.push_back(g.button_Crafting);
    widgets.push_back(g.button_Items);
    widgets.push_back(g.button_Condition);
    widgets.push_back(g.button_Camp);
    widgets.push_back(g.button_Vehicle);

    TestGridSystem *tgs = new(TestGridSystem);

    widgets.push_back(tgs);
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

    ALLEGRO_DISPLAY *display = al_create_display(1280, 720);
    if(display == NULL)
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
    al_set_target_backbuffer(display);

    al_register_event_source(event_queue, al_get_display_event_source(display));
    al_register_event_source(event_queue, al_get_timer_event_source(timer));
    al_register_event_source(event_queue, al_get_keyboard_event_source());

    g.color_grey = al_color_name("grey");
    g.color_grey2 = al_map_rgb(200, 200, 200);
    g.color_grey3 = al_map_rgb(240, 240, 240);
    g.color_black = al_map_rgb(0, 0, 0);
    g.color_white = al_map_rgb(255, 255, 255);

    bool redraw = true;
    bool was_mouse_down = false;

    init_buttons();
    init_messagelog();
    init_tilemap();

    {
        g.ui_MainMap = new(MainMapUI);
        g.ui = g.ui_MainMap;
    }

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
            al_clear_to_color(g.color_grey);

            { // drawing goes here
                g.ui->draw();
            }
            al_flip_display();
        }
    }

    // don't really need to deallocate if we're quitting...
    al_destroy_display(display);
    delete g.ui;

    return 0;
}
