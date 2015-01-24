override CXXFLAGS+= -Wall -pedantic --std=c++11 -funsigned-char

INCLUDES= `pkg-config --cflags sigc++-2.0`
LIBS= `pkg-config --libs allegro-5.0 allegro_primitives-5.0 allegro_color-5.0 allegro_image-5.0 allegro_font-5.0 sigc++-2.0`

STRIP=
ifeq (,$(findstring -g,$(CXXFLAGS)))
ifeq (,$(findstring -pg,$(CXXFLAGS)))
	STRIP=strip
endif
endif

OBJS= \
	main.o

PCH=

default: all

$(OBJS): CXXFLAGS+= $(INCLUDES)
$(PCH): %.h.gch: %.h
	$(CXX) $(CXXFLAGS) -x c++-header -o $@.tmp $<
	$(MV) $@.tmp $@

all: $(OBJS)
#ifneq (,$(STRIP))
#	$(STRIP) ../project_x
#endif
	$(CXX) $(CXXFLAGS) -o ../project_x $(OBJS) $(LIBS)

clean:
	-$(RM) $(OBJS) $(PCH) ../project_x

depend:
	makedepend -Y $(subst .o,.cpp,$(OBJS))
	makedepend -a -o.h.gch -Y -Ishared -Iengine -Irpggame $(PCH:.h.gch=.h)
