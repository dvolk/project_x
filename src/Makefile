SANITIZE=-fsanitize=address -fsanitize=leak -fsanitize=undefined
override CXXFLAGS+=-g -Wall -Wextra -pedantic --std=c++11 $(SANITIZE) -funsigned-char
#override CXX=clang++-3.5
INCLUDES=

### Linux
LIBS=-lstdc++ `pkg-config --libs allegro-5.0 allegro_primitives-5.0 allegro_color-5.0 allegro_image-5.0 allegro_font-5.0`

### Windows
# LIBS=-lallegro-5.0.10-mt -lallegro_primitives-5.0.10-mt -lallegro_color-5.0.10-mt -lallegro_image-5.0.10-mt -lallegro_font-5.0.10-mt

OBJS= \
	util.o rect.o widget.o main.o

version:
	@( VERSION_STRING=$(VERSION) ; \
            [ -e "../.git" ] && GITVERSION=$$( git describe --tags --always --dirty --match "[0-9A-Z]*.[0-9A-Z]*" ) && VERSION_STRING=$$GITVERSION ; \
            [ -e "version.h" ] && OLDVERSION=$$(grep VERSION version.h|cut -d '"' -f2) ; \
            if [ "x$$VERSION_STRING" != "x$$OLDVERSION" ]; then echo "#define VERSION \"$$VERSION_STRING\"" | tee version.h ; fi \
         )

default: all

all: version $(OBJS)
	$(CXX) $(SANITIZE) -o ../project_x $(OBJS) $(LDFLAGS) $(LIBS)

clean:
	-$(RM) $(OBJS) version.h ../project_x
