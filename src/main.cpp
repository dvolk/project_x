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

int mouse_x;
int mouse_y;
int hold_off_x;
int hold_off_y;
int key;

ALLEGRO_COLOR color_grey;
ALLEGRO_COLOR color_grey2;
ALLEGRO_COLOR color_grey3;
ALLEGRO_COLOR color_black;

ALLEGRO_FONT *font;

MessageLog *g_log;
UI *g_ui;

UI *MainUI;
UI *LootUI;

void errorQuit(string str) {
    cout << "Error: " << str << endl;
    exit(1);
}

void info(string str) {
    cout << "Info: " << str << endl;
}

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

    virtual ~Widget() { info("~Widget"); };

    virtual void draw(void) = 0;
    virtual void update() { };
};

struct Grid {
    Rect pos;
    vector<Item *> items;

    ~Grid() {
        info("~Grid()");
        for(auto item : items)
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
        for(auto grid : grids)
            delete grid;
    }

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
    int cols;
    int view_x;
    int view_y;
    vector<Tile> tiles;
    vector<ALLEGRO_BITMAP *>bitmaps;

    ~TileMap(void);

    // must be called after the bitmaps vector is filled
    void generate(void);
    void draw(void);
    void tmKeyDownEvent(void);
};

TileMap::~TileMap(void) {
    info("~TileMap");
    for(auto& bitmap : bitmaps)
        al_destroy_bitmap(bitmap);
    //bitmaps.clear();
    //tiles.clear();
}

struct UI {
    vector<Widget *> widgets;

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
        if(widget->pos.x1 <= mouse_x &&
           widget->pos.y1 <= mouse_y &&
           widget->pos.x1 + widget->pos.x2 >= mouse_x &&
           widget->pos.y1 + widget->pos.y2 >= mouse_y) {
            widget->onMouseDown.emit();
        }
    }
}

void UI::mouseUpEvent(void) {
    for(auto& widget : widgets) {
        if(widget->pos.x1 <= mouse_x &&
           widget->pos.y1 <= mouse_y &&
           widget->pos.x1 + widget->pos.x2 >= mouse_x &&
           widget->pos.y1 + widget->pos.y2 >= mouse_y) {
            widget->onMouseUp.emit();
        }
    }
}

void UI::keyDownEvent(void) {
    for(auto& widget : widgets) {
        if(widget->pos.x1 <= mouse_x &&
           widget->pos.y1 <= mouse_y &&
           widget->pos.x1 + widget->pos.x2 >= mouse_x &&
           widget->pos.y1 + widget->pos.y2 >= mouse_y) {
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

    void press(void);
    void draw(void);
    void update(void);
};

struct MessageLog : public Widget {
    ALLEGRO_BITMAP *background;
    ALLEGRO_FONT *font;
    vector<string> lines;

    ~MessageLog();

    void draw(void);
};

void switch_ui(bool p);
static bool blah = 1;

void Button::press(void) {
    pressed = !pressed;
    g_log->lines.push_back("pressed button");
    blah = !blah;
    switch_ui(blah);
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
        al_draw_text(font, color_black, pos.x1 + 8, pos.y1 + off_y, 0, lines[i].c_str());
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
}

void TileMap::draw(void) {
    int start = cols * view_y + view_x;
    int rows = 20;
    for(int i = start; i < start + cols * rows; i++) {
        int idx = i - start;
        int off_x = idx % cols * 80;
        int off_y = (idx + view_x) % 2  == 0 ? 0 : 20;
        off_y = off_y + (40 * floor(idx / cols));
        int dst_x = off_x - 80;
        int dst_y = off_y - 60;
        al_draw_bitmap(bitmaps[tiles[i].bitmap_index], dst_x, dst_y, 0);
    }
}

void TileMap::tmKeyDownEvent(void) {
    if(key == ALLEGRO_KEY_DOWN)
        if(view_y < 150)
            view_y++;
    if(key == ALLEGRO_KEY_UP)
        if(view_y > 0)
            view_y--;
    if(key == ALLEGRO_KEY_LEFT)
        if(view_x > 0)
            view_x--;
    if(key == ALLEGRO_KEY_RIGHT)
        if(view_x < 150)
            view_x++;
}

void GridSystem::draw(void) {
    for (auto& g : grids)
        g->draw();

    if(held != NULL)
        held->draw();
}

void GridSystem::gsMouseDownEvent() {
    Item *i = grids[0]->get_item(mouse_x, mouse_y);
    if(i != NULL) {
        held = i;
        hold_off_x =
            mouse_x - (i->parent->pos.x1 + i->pos.x1 * 10);
        hold_off_y =
            mouse_y - (i->parent->pos.y1 + i->pos.y1 * 10);
        held->parent = NULL;
    }
}

void GridSystem::gsMouseUpEvent() {
    if(held != NULL) {
        // mouse is holding an item
        int drop_x =
            ((mouse_x - hold_off_x) - grids[0]->pos.x1) / 10;
        int drop_y =
            ((mouse_y - hold_off_y) - grids[0]->pos.y1) / 10;

        held->parent = grids[0];
        held->pos.x1 = drop_x;
        held->pos.y1 = drop_y;
        grids[0]->items.push_back(held);
        //delete held;
        held = NULL;
        g_log->lines.push_back("You moved the block! Congratulations!");
    }
}

void Grid::draw(void) {
    al_draw_filled_rectangle(pos.x1, pos.y1, pos.x2, pos.y2, color_grey2);

    for (int x = pos.x1; x <= pos.x2; x = x + 10)
        al_draw_line(x, pos.y1, x, pos.y2, color_grey3, 1);
    for (int y = pos.y1; y <= pos.y2; y = y + 10)
        al_draw_line(pos.x1, y, pos.x2, y, color_grey3, 1);

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
        al_draw_filled_rectangle(mouse_x - hold_off_x,
                                 mouse_y - hold_off_y,
                                 mouse_x - hold_off_x + pos.x2 * 10,
                                 mouse_y - hold_off_y + pos.y2 * 10,
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

struct TestUI : public UI {
    TestUI();
    ~TestUI();
};

struct TestUI2 : public UI {
    TestUI2();
    ~TestUI2();
};

void switch_ui(bool p) {
    cout << p << endl;
    if(!p) {
        g_ui = LootUI;
    } else {
        g_ui = MainUI;
    }
}

TestUI2::TestUI2(void) {
    ALLEGRO_BITMAP *button_up = al_load_bitmap("media/buttons/button_up.png");
    ALLEGRO_BITMAP *button_down = al_load_bitmap("media/buttons/button_down.png");
    Button *test_button = new(Button);
    test_button->pos.x1 = 400;
    test_button->pos.y1 = 400;
    test_button->pos.x2 = 100;
    test_button->pos.y2 = 100;
    test_button->up = button_up;
    test_button->down = button_down;
    test_button->pressed = false;
    test_button->onMouseDown.connect(mem_fun(test_button, &Button::press));

    widgets.push_back(test_button);
}

TestUI2::~TestUI2() {
    info("~TestUI2()");
    for(auto& widget : widgets)
        delete widget;
    //widgets.clear();
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

TestUI::TestUI() {
    ALLEGRO_BITMAP *button_up = al_load_bitmap("media/buttons/button_up.png");
    ALLEGRO_BITMAP *button_down = al_load_bitmap("media/buttons/button_down.png");
    ALLEGRO_BITMAP *messagelogbg = al_load_bitmap("media/backgrounds/messagelogbg.png");

    Button *test_button = new(Button);
    test_button->pos.x1 = 400;
    test_button->pos.y1 = 400;
    test_button->pos.x2 = 100;
    test_button->pos.y2 = 100;
    test_button->up = button_up;
    test_button->down = button_down;
    test_button->pressed = false;
    test_button->onMouseDown.connect(mem_fun(test_button, &Button::press));

    MessageLog *log = new(MessageLog);
    log->pos.x1 = 100;
    log->pos.y1 = 520;
    log->pos.x2 = 1080;
    log->pos.y2 = 200;
    log->background = messagelogbg;
    log->font = font;
    log->lines.push_back("Player is dying.");
    log->lines.push_back("Player is dead. Blegrgrgrvddgd...");

    TestGridSystem *tgs = new(TestGridSystem);
    {
        tgs->onMouseUp.connect(mem_fun(tgs, &TestGridSystem::gsMouseUpEvent));
        tgs->onMouseDown.connect(mem_fun(tgs, &TestGridSystem::gsMouseDownEvent));
    }

    g_log = log;

    TileMap *tm = new(TileMap);
    {
        tm->cols = 18;
        tm->view_x = 4;
        tm->view_y = 4;
        tm->pos.x1 = 0;
        tm->pos.y1 = 0;
        tm->pos.x2 = 1280;
        tm->pos.y2 = 720;

        ALLEGRO_BITMAP *tile_grass = al_load_bitmap("media/tile/grass.png");
        ALLEGRO_BITMAP *tile_tree = al_load_bitmap("media/tile/tree.png");
        ALLEGRO_BITMAP *tile_city = al_load_bitmap("media/tile/city.png");

        tm->bitmaps.push_back(tile_grass);
        tm->bitmaps.push_back(tile_tree);
        tm->bitmaps.push_back(tile_city);

        tm->generate();
        tm->onKeyDown.connect(mem_fun(tm, &TileMap::tmKeyDownEvent));
    }

    widgets.push_back(tm);
    widgets.push_back(log);
    widgets.push_back(test_button);
    widgets.push_back(tgs);
}

TestUI::~TestUI(void) {
    info("~TestUI()");
    for(auto& widget : widgets)
        delete widget;
    //widgets.clear();
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

    font = al_create_builtin_font();
    if(font == NULL)
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

    color_grey = al_color_name("grey");
    color_grey2 = al_map_rgb(200, 200, 200);
    color_grey3 = al_map_rgb(240, 240, 240);
    color_black = al_map_rgb(0, 0, 0);

    bool redraw = true;
    bool was_mouse_down = false;

    {
        UI *ui = new(TestUI);
        MainUI = ui;
        g_ui = ui;
        ui = new(TestUI2);
        LootUI = ui;
    }

    while(1) {
        al_get_mouse_state(&mouse_state);
        al_get_keyboard_state(&keyboard_state);

        mouse_x = mouse_state.x;
        mouse_y = mouse_state.y;

        if (mouse_state.buttons & 1) {
            if (!was_mouse_down) {
                // mouse down event
                g_ui->mouseDownEvent();
                was_mouse_down = true;
            }
        }
        else if (was_mouse_down) {
            g_ui->mouseUpEvent();
            // mouse up event
            was_mouse_down = false;
        }

        al_wait_for_event(event_queue, &ev);

        if(ev.type == ALLEGRO_EVENT_KEY_DOWN) {
            key = ev.keyboard.keycode;
            if(key == ALLEGRO_KEY_ESCAPE)
                break;
            else
                g_ui->keyDownEvent();
        }
        else if(ev.type == ALLEGRO_EVENT_TIMER) {
            { // logic goes here
                g_ui->update();
            }
            redraw = true;
        }
        else if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            break;
        }

        if(redraw && al_is_event_queue_empty(event_queue)) {
            redraw = false;
            al_clear_to_color(color_grey);

            { // drawing goes here
                g_ui->draw();
            }
            al_flip_display();
        }
    }

    // don't really need to deallocate if we're quitting...
    al_destroy_display(display);
    delete g_ui;

    return 0;
}
