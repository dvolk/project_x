#include <allegro5/allegro.h>
#include <allegro5/allegro_color.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>

#include <stdint.h>

#include <vector>
#include <iostream>
#include <memory>
#include <cmath>

int mouse_x;
int mouse_y;
int hold_off_x;
int hold_off_y;
ALLEGRO_COLOR color_blue;
ALLEGRO_COLOR color_grey;
ALLEGRO_COLOR color_grey2;
ALLEGRO_COLOR color_grey3;

using namespace std;

struct Rect {
    float x1;
    float y1;
    float x2;
    float y2;
};

struct Grid;

struct Item {
    Rect pos;
    string name;
    Grid *parent;
    void draw(void);
};

struct Grid {
    Rect pos;
    vector<Item> items;

    void draw(void);
    Item *get_item(int x, int y); // get item at screen position
};

struct GridSystem {
    vector<Grid> grids;

    void draw(void);
};

struct Tile {
    int16_t bitmap_index;
};

struct TileMap {
    int cols;
    int view_x;
    int view_y;
    vector<Tile> tiles;
    vector<ALLEGRO_BITMAP *>bitmaps;

    void generate(void);
    void draw(void);
};

void TileMap::generate(void) {
    for(int i = 0; i < 150*150; i++) {
        Tile t;
        t.bitmap_index = rand() % 6;
        tiles.push_back(t);
    }
}

void TileMap::draw(void) {
    int start = cols * view_y + view_x;
    int rows = 14;
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

void GridSystem::draw(void) {
    for (auto g : grids)
        g.draw();
}

void Grid::draw(void) {
    al_draw_filled_rectangle(pos.x1, pos.y1, pos.x2, pos.y2, color_grey2);

    for (int x = pos.x1; x <= pos.x2; x = x + 10)
        al_draw_line(x, pos.y1, x, pos.y2, color_grey3, 1);
    for (int y = pos.y1; y <= pos.y2; y = y + 10)
        al_draw_line(pos.x1, y, pos.x2, y, color_grey3, 1);
    
    for (auto i : items)
        i.draw();
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
    for (auto i : items) {
        if(i.parent->pos.x1 + i.pos.x1 * 10 <= x &&
           i.parent->pos.y1 + i.pos.y1 * 10 <= y &&
           i.parent->pos.x1 + (i.pos.x1 + i.pos.x2) * 10 >= x &&
           i.parent->pos.y1 + (i.pos.y1 + i.pos.y2) * 10 >= y) {

            // humm
            Item *a = new(Item);
            *a = items[c];
            items.erase(items.begin() + c);
            return a;
        }
        c++;
    }
    return NULL;
}



int main(void) {
    al_init();
    al_init_primitives_addon();
    al_init_image_addon();
    al_install_keyboard();
    al_install_mouse();
    
    ALLEGRO_DISPLAY *display = al_create_display(1280, 720);
    ALLEGRO_EVENT_QUEUE *event_queue = al_create_event_queue();
    ALLEGRO_TIMER *timer = al_create_timer(1.0 / 60);
    ALLEGRO_EVENT ev;
    ALLEGRO_MOUSE_STATE mouse_state;
    ALLEGRO_KEYBOARD_STATE keyboard_state;
    al_start_timer(timer);
    al_set_target_backbuffer(display);

    color_blue = al_color_name("steelblue");
    color_grey = al_color_name("grey");
    color_grey2 = al_map_rgb(200, 200, 200);
    color_grey3 = al_map_rgb(240, 240, 240);

    ALLEGRO_BITMAP *tile_grass = al_load_bitmap("tile_grass.png");
    ALLEGRO_BITMAP *tile_tree = al_load_bitmap("tile_tree.png");
    ALLEGRO_BITMAP *tile_city = al_load_bitmap("tile_city.png");

    TileMap tm;
    tm.cols = 10;
    tm.generate();
    tm.view_x = 4;
    tm.view_y = 4;

    tm.bitmaps.push_back(tile_grass);
    tm.bitmaps.push_back(tile_grass);
    tm.bitmaps.push_back(tile_grass);
    tm.bitmaps.push_back(tile_tree);
    tm.bitmaps.push_back(tile_tree);
    tm.bitmaps.push_back(tile_city);

    al_register_event_source(event_queue, al_get_display_event_source(display));
    al_register_event_source(event_queue, al_get_timer_event_source(timer));
    al_register_event_source(event_queue, al_get_keyboard_event_source());

    GridSystem gs;
    {
        Grid g1;
        g1.pos.x1 = 10;
        g1.pos.y1 = 10;
        g1.pos.x2 = 200;
        g1.pos.y2 = 300;
        Item i1;
        i1.pos.x1 = 3;
        i1.pos.y1 = 3;
        i1.pos.x2 = 7;
        i1.pos.y2 = 1;
        i1.parent = &g1;
        i1.name = "item 1";
        Item i2;
        i2.pos.x1 = 3;
        i2.pos.y1 = 10;
        i2.pos.x2 = 4;
        i2.pos.y2 = 4;
        i2.parent = &g1;
        i2.name = "item 2";
        g1.items.push_back(i1);
        g1.items.push_back(i2);
        Grid g2;
        g2.pos.x1 = 250;
        g2.pos.y1 = 10;
        g2.pos.x2 = 450;
        g2.pos.y2 = 200;
        gs.grids.push_back(g1);
        gs.grids.push_back(g2);
    }
    
    bool redraw = true;
    bool was_mouse_down = false;
    Item *held = NULL;

    while(1) {
        al_get_mouse_state(&mouse_state);
        al_get_keyboard_state(&keyboard_state);
        
        mouse_x = mouse_state.x;
        mouse_y = mouse_state.y;
        if (mouse_state.buttons & 1) {
            if (!was_mouse_down) {
                // mouse down event
                Item *i = gs.grids[0].get_item(mouse_x, mouse_y);
                if(i != NULL) {
                    held = i;
                    hold_off_x =
                        mouse_x - (i->parent->pos.x1 + i->pos.x1 * 10);
                    hold_off_y =
                        mouse_y - (i->parent->pos.y1 + i->pos.y1 * 10);
                    held->parent = NULL;
                }
                was_mouse_down = true;
            }
        } else if (was_mouse_down) {
            // mouse up event
            was_mouse_down = false;
            
            if(held != NULL) {
                // mouse is holding an item
                int drop_x =
                    ((mouse_x - hold_off_x) - gs.grids[0].pos.x1) / 10;
                int drop_y =
                    ((mouse_y - hold_off_y) - gs.grids[0].pos.y1) / 10;

                held->parent = &gs.grids[0];
                held->pos.x1 = drop_x;
                held->pos.y1 = drop_y;
                gs.grids[0].items.push_back(*held);
                delete held;
                held = NULL;
            }
        }
 
        al_wait_for_event(event_queue, &ev);

        if(ev.type == ALLEGRO_EVENT_KEY_DOWN) {
            if(ev.keyboard.keycode == ALLEGRO_KEY_DOWN)
                if(tm.view_y < 150)
                    tm.view_y++;
            if(ev.keyboard.keycode == ALLEGRO_KEY_UP)
                if(tm.view_y > 0)
                    tm.view_y--;
            if(ev.keyboard.keycode == ALLEGRO_KEY_LEFT)
                if(tm.view_x > 0)
                    tm.view_x--;
            if(ev.keyboard.keycode == ALLEGRO_KEY_RIGHT)
                if(tm.view_x < 150)
                    tm.view_x++;
            if(ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
                break;
        }

        if(ev.type == ALLEGRO_EVENT_TIMER) {
            // logic goes here
            redraw = true;
        }
        else if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            break;
        }

        
        if(redraw && al_is_event_queue_empty(event_queue)) {
            redraw = false;
            al_clear_to_color(color_grey);

            { // drawing goes here
                tm.draw();
                gs.draw();
                if(held != NULL) {
                    held->draw();
                }
            }
            al_flip_display();
        }
    }
    
    al_destroy_display(display);
    return 0;
}
