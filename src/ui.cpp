#include "./ui.h"

extern int mouse_x;
extern int mouse_y;

/*
  global keys shared by all UIs

  returns true if it caught the key, false if it should
  be passed to the widgets.
 */
static bool (*global_key_callback)(void) = NULL;

void register_global_key_callback(bool (*cb)(void)) {
    global_key_callback = cb;
}

void UI::update(void) {
    for(auto& widget : widgets) {
        widget->update();
    }
}

void UI::mouseDownEvent(void) {
    for(auto& widget : widgets) {
        if(widget->pos.x1 <= mouse_x &&
           widget->pos.y1 <= mouse_y &&
           widget->pos.x1 + widget->pos.x2 >= mouse_x &&
           widget->pos.y1 + widget->pos.y2 >= mouse_y) {
            widget->mouseDown();
            if(widget->onMouseDown != nullptr)
                widget->onMouseDown();
        }
    }
}

void UI::mouseUpEvent(void) {
    for(auto& widget : widgets) {
        if(widget->pos.x1 <= mouse_x &&
           widget->pos.y1 <= mouse_y &&
           widget->pos.x1 + widget->pos.x2 >= mouse_x &&
           widget->pos.y1 + widget->pos.y2 >= mouse_y) {
            widget->mouseUp();
            if(widget->onMouseUp != nullptr)
                widget->onMouseUp();
            return;
        }
    }
}

void UI::keyDownEvent(void) {
    if(global_key_callback != nullptr) {
        if(global_key_callback() == true)
            return;
    }
    for(auto& widget : widgets) {
        if(widget->pos.x1 <= mouse_x &&
           widget->pos.y1 <= mouse_y &&
           widget->pos.x1 + widget->pos.x2 >= mouse_x &&
           widget->pos.y1 + widget->pos.y2 >= mouse_y) {
            widget->keyDown();
            if(widget->onKeyDown != nullptr)
                widget->onKeyDown();
            return;
        }
    }
}

void UI::hoverOverEvent(void) {
    for(auto& widget : widgets) {
        if(widget->pos.x1 <= mouse_x &&
           widget->pos.y1 <= mouse_y &&
           widget->pos.x1 + widget->pos.x2 >= mouse_x &&
           widget->pos.y1 + widget->pos.y2 >= mouse_y) {
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
