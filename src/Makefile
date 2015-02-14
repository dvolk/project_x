SANITIZE=-fsanitize=address -fsanitize=leak -fsanitize=undefined
override CXXFLAGS+=-g -Wall -Wextra -pedantic --std=c++11 $(SANITIZE) -funsigned-char

INCLUDES=

### Linux
LIBS=`pkg-config --libs allegro-5.0 allegro_primitives-5.0 allegro_color-5.0 allegro_image-5.0 allegro_font-5.0`

### Windows
# LIBS=-lallegro-5.0.10-mt -lallegro_primitives-5.0.10-mt -lallegro_color-5.0.10-mt -lallegro_image-5.0.10-mt -lallegro_font-5.0.10-mt

OBJS= \
	main.o util.o

default: all

all: $(OBJS)
	$(CXX) $(CXXFLAGS) -o ../project_x $(OBJS) $(LDFLAGS) $(LIBS)

clean:
	-$(RM) $(OBJS) ../project_x
