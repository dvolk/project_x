#include "./ui.h"

extern int mouse_x;
extern int mouse_y;

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
