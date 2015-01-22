#include <allegro5/allegro.h>
#include <allegro5/allegro_color.h>
#include <allegro5/allegro_primitives.h>

#include <vector>
#include <iostream>
#include <memory>

int mouse_x;
int mouse_y;
int hold_off_x;
int hold_off_y;

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

void GridSystem::draw(void) {
    for (auto g : grids)
        g.draw();
}

void Grid::draw(void) {
    ALLEGRO_COLOR col = al_map_rgb(200, 200, 200);
    ALLEGRO_COLOR l = al_map_rgb(240, 240, 240);
    
    al_draw_filled_rectangle(pos.x1, pos.y1, pos.x2, pos.y2, col);

    for (int x = pos.x1; x <= pos.x2; x = x + 10)
        al_draw_line(x, pos.y1, x, pos.y2, l, 1);
    for (int y = pos.y1; y <= pos.y2; y = y + 10)
        al_draw_line(pos.x1, y, pos.x2, y, l, 1);
    
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
            cout << c << "\n";
            return a;
        }
        c++;
    }
    //printf("No item here\n");
    return NULL;
}



int main(void) {
    al_init();
    al_init_primitives_addon();
    al_install_keyboard();
    al_install_mouse();
    
    ALLEGRO_DISPLAY *display = al_create_display(1280, 720);
    ALLEGRO_COLOR color_blue = al_color_name("steelblue");
    ALLEGRO_COLOR color_grey = al_color_name("grey");
    ALLEGRO_EVENT_QUEUE *event_queue = al_create_event_queue();
    ALLEGRO_TIMER *timer = al_create_timer(1.0 / 60);
    ALLEGRO_EVENT ev;
    //ALLEGRO_TIMEOUT timeout;
    ALLEGRO_MOUSE_STATE state;
    al_start_timer(timer);    

    al_register_event_source(event_queue, al_get_display_event_source(display));
    al_register_event_source(event_queue, al_get_timer_event_source(timer));

    GridSystem gs;
    Grid g1;
    g1.pos.x1 = 10;
    g1.pos.y1 = 10;
    g1.pos.x2 = 200;
    g1.pos.y2 = 300;
    Item i1;
    i1.pos.x1 = 3;
    i1.pos.y1 = 3;
    i1.pos.x2 = 5;
    i1.pos.y2 = 2;
    i1.parent = &g1;
    i1.name = "item 1";
    Item i2;
    i2.pos.x1 = 3;
    i2.pos.y1 = 10;
    i2.pos.x2 = 2;
    i2.pos.y2 = 5;
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
    
    int x = 20;
    int y = 20;
    bool redraw = true;
    bool was_mouse_down = false;
    Item *held = NULL;

    while(1) {
        al_get_mouse_state(&state);
        
        mouse_x = state.x;
        mouse_y = state.y;
        if (state.buttons & 1) {
            if (!was_mouse_down) {
                // mouse down event
                Item *i = gs.grids[0].get_item(mouse_x, mouse_y);
                if(i != NULL) {
                    cout << "Clicked on item: " << i->name << "\n";
                    held = i;
                    hold_off_x =
                        mouse_x - (i->parent->pos.x1 + i->pos.x1 * 10);
                    hold_off_y =
                        mouse_y - (i->parent->pos.y1 + i->pos.y1 * 10);
                    held->parent = NULL;
                }
                cout << "pressed mouse key\n";
                was_mouse_down = true;
                cout << g1.items.size() << "\n";
            }
            
            //printf("Mouse position: (%d, %d)\n", state.x, state.y);
        }
        else if (was_mouse_down) {
            // mouse up event
            cout << "released mouse key\n";
            was_mouse_down = false;
            
            if(held != NULL) {
                int drop_x =
                    ((mouse_x - hold_off_x) - gs.grids[0].pos.x1) / 10;
                int drop_y =
                    ((mouse_y - hold_off_y) - gs.grids[0].pos.y1) / 10;
                cout << "dropping at " << drop_x << " " << drop_y << "\n";
                held->parent = &gs.grids[0];
                held->pos.x1 = drop_x;
                held->pos.y1 = drop_y;
                gs.grids[0].items.push_back(*held);
                delete held;
                held = NULL;
            }
            cout << g1.items.size() << "\n";
            
        }
 
        al_wait_for_event(event_queue, &ev);


            if(ev.type == ALLEGRO_EVENT_TIMER) {
                // logic goes here
                x++;
                y++;
                redraw = true;
            }
            else if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
                break;
            }

        
        if(redraw && al_is_event_queue_empty(event_queue)) {
            // drawing goes here
            redraw = false;
            al_clear_to_color(color_grey);
            al_draw_line(10, 10, x, y, color_blue, 10);
            gs.draw();
            if(held != NULL) {
                printf("drawing held %d %d %d %d\n", mouse_x, mouse_y, hold_off_x, hold_off_y);
                held->draw();
            }
            al_flip_display();
        }
    }
    
    al_destroy_display(display);
    return 0;
}
