#include "./widget.h"

Widget::Widget() {
    visible = true;
    onMouseDown = nullptr;
    onMouseUp = nullptr;
    onKeyDown = nullptr;
}

Widget::~Widget() {
    // info("~Widget()");
}
