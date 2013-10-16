PROG_NAME = reword
SOURCES = reword.cpp \
		audio.cpp \
		button.cpp \
		control.cpp \
		controls.cpp \
		easing.cpp \
		error.cpp \
		fontttf.cpp \
		framerate.cpp \
		game.cpp \
		gamedata.cpp \
		helpers.cpp \
		image.cpp \
		imageanim.cpp \
		input.cpp \
		locator.cpp \
		play.cpp \
		playdiff.cpp \
		playgame.cpp \
		playgamedict.cpp \
		playgamepopup.cpp \
		playhigh.cpp \
		playinst.cpp \
		playmainmenu.cpp \
		playmenu.cpp \
		playmodemenu.cpp \
		playoptions.cpp\
		random.cpp \
		resource.cpp \
		roundels.cpp \
		score.cpp \
		screen.cpp \
		singleton.cpp \
		sprite.cpp \
		spritemgr.cpp \
		surface.cpp \
		utils.cpp \
		waiting.cpp \
		words.cpp

OBJECTS = $(SOURCES:.cpp=.o)
BIN = $(PROG_NAME)

ifdef pc
CXX = g++
# LDFLAGS = -lm -lg -lpthread -lSDL -lSDL_image -lSDL_mixer -lSDL_ttf
# CXX = g++
# CXXFLAGS = -c -Wall `sdl-config --cflags` -ggdb -D_DEBUG -I/usr/include/SDL
#LDFLAGS = -lm -lg -lpthread -lSDL -lSDL_mixer -lSDL_image -lSDL_ttf `sdl-config --static-libs`
#LDFLAGS = -L/lib -DGP2X -lSDL_image  -lSDL_gfx  -lSDL_ttf  -ljpeg -lpng12 -lz -lSDL_mixer -lvorbisidec -lmikmod -lmad -lSDL -lgcc -lm -lc -lexpat -lpthread -msoft-float
#-lfreetype
LDFLAGS = -Wl,-rpath,/usr/local/lib -L/usr/local/lib lSDL2 -lSDL2_mixer -lSDL2_image -lSDL2_ttf -lpthread
#=====BUILD DEBUG
#CXXFLAGS = -c -Wall `sdl-config --cflags` -ggdb -D_DEBUG -I/usr/include/SDL
CXXFLAGS = -g -O2 -D_REENTRANT -I/usr/local/include/SDL2 -std=c++11
STRIP = ls -l
#=====BUILD RELEASE
#CXXFLAGS = -c -Wall `sdl-config --cflags` -I/usr/include/SDL
#STRIP = strip
endif

ifdef gp2x
BIN = $(BIN).gpe
CROSS_PATH = /opt/open2x/gcc-4.1.1-glibc-2.3.6/
LDFLAGS = -L$(CROSS_PATH)/lib -static-lm -lg -lpthread -lSDL -lSDL_mixer -lSDL_image -lSDL_ttf `$(CROSS_PATH)/bin/sdl-config --static-libs`
CXX = $(CROSS_PATH)/bin/arm-open2x-linux-g++
CXXFLAGS = -DGP2X -c -Wall `$(CROSS_PATH)/bin/sdl-config --cflags` -I$(CROSS_PATH)/include -I$(CROSS_PATH)/include/SDL
STRIP = $(CROSS_PATH)/bin/arm-open2x-linux-strip
endif

ifdef pan
##CROSS_PATH = /usr/local/pandora/arm-2009q3
##LD_LIBRARY_PATH=$(CROSS_PATH)/usr/lib
##LD_RUN_PATH=$(CROSS_PATH)/bin
##LD_AOUT_LIBRARY_PATH=$(LD_RUN_PATH)
##SDL_CONFIG = $(CROSS_PATH)/usr/bin/sdl-config
###LDFLAGS = -L$(CROSS_PATH)/usr/lib -L$(CROSS_PATH)/lib -static -DPANDORA -lm -lg -lpthread -lSDL -lSDL_mixer -lSDL_image -lSDL_ttf `$(SDL_CONFIG) --static-libs`
##LDFLAGS = -L$(CROSS_PATH)/lib -L$(CROSS_PATH)/usr/lib
##LDFLAGS	+= -lm -lg -lpthread -lts -lfreetype -lSDL -lSDL_mixer -lSDL_image -lSDL_ttf
###CC = $(CROSS_PATH)/bin/arm-none-linux-gnueabi-gcc
###CXX = $(CROSS_PATH)/bin/arm-none-linux-gnueabi-g++ -v -shared
##CXX = $(CROSS_PATH)/bin/arm-none-linux-gnueabi-g++
##LD = $(CROSS_PATH)/bin/arm-none-linux-gnueabi-ld
##CXXFLAGS = -DPANDORA -c -Wall `$(SDL_CONFIG) --cflags` -I$(CROSS_PATH)/usr/include -I$(CROSS_PATH)/usr/include/SDL
##STRIP = $(CROSS_PATH)/bin/arm-none-linux-gnueabi-strip
CROSS_PATH = /usr/local/angstrom/arm/arm-angstrom-linux-gnueabi
LD_LIBRARY_PATH=$(CROSS_PATH)/usr/lib
LD_RUN_PATH=$(CROSS_PATH)/bin
LD_AOUT_LIBRARY_PATH=$(LD_RUN_PATH)
SDL_CONFIG = $(CROSS_PATH)/usr/bin/sdl-config
LDFLAGS = -L$(CROSS_PATH)/lib -L$(CROSS_PATH)/usr/lib
LDFLAGS	+= -lm -lg -lpthread -lts -lfreetype -lSDL -lSDL_mixer -lSDL_image -lSDL_ttf -lboost_filesystem-mt -lboost_system-mt
CXXFLAGS = -DPANDORA -c -Wall `$(SDL_CONFIG) --cflags` -I$(CROSS_PATH)/usr/include -I$(CROSS_PATH)/usr/include/SDL
CXX = $(CROSS_PATH)/bin/g++
LD = $(CROSS_PATH)/bin/ld
STRIP = $(CROSS_PATH)/bin/strip
endif

all:
	make pcversion pc=1
#to make both versions at once include the following
#	make cleanobjs
#	make gpversion arm=1

pc:
	make pcversion pc=1

gp2x:
	make gpversion gp2x=1

pan:
	make panversion pan=1

pcversion: $(SOURCES) $(BIN)

gpversion: $(SOURCES) $(BIN)

panversion: $(SOURCES) $(BIN)

$(BIN): $(OBJECTS)
	$(CXX) $(LDFLAGS) $(OBJECTS) -o $@
	$(STRIP) $(BIN)

.cpp.o:
	$(CXX) $(CXXFLAGS) $< -o $@

cleanobjs:
	rm -f $(OBJECTS)

clean:
	rm -f $(BIN) *~ *.o *.bak




