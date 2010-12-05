
# name of the binary - only valid for targets which set SYSTEM
DESTNAME = Dakar2011.bin

SRCS = $(shell ls *.cpp)

OBJS = $(SRCS:.cpp=.o)

IRRLICHT_DIR = ../irrlicht-svn-3473-patch4
IRRKLANG_DIR = ../irrKlang-1.3.0
NEWTON_DIR = ../newtonSDK/sdk
OPENAL_DIR = ../openal-soft-1.10.622
ALUT_DIR = ../freealut-1.1.0-src
CG_DIR = ../Cg30

# general compiler settings
CPPFLAGS = -I. -I$(IRRLICHT_DIR)/include -I$(IRRLICHT_DIR)/source/Irrlicht -I/usr/X11R6/include -I$(IRRKLANG_DIR)/include -I$(NEWTON_DIR) -I$(CG_DIR)/include -I$(OPENAL_DIR)/include  -I$(ALUT_DIR)/include
CXXFLAGS = -O3 -ffast-math
#  -DMY_DEBUG -DUSE_EDITOR -DUSE_MY_SOUNDENGINE
#CXXFLAGS = -g -Wall

#default target is Linux
all: all_linux

#ifeq ($(HOSTTYPE), x86_64)
#LIBSELECT=64
#endif

all_linux: LDFLAGS = -L/usr/X11R6/lib -L$(NEWTON_DIR) -L$(OPENAL_DIR)/build -L$(ALUT_DIR)/build/lib -L$(CG_DIR)/lib -L$(IRRKLANG_DIR)/bin/linux-gcc -lGL -lopenal -lalut -lXxf86vm -lXext -lX11 -lXcursor -lpthread -lIrrKlang -lNewton -lCg -lCgGL $(IRRKLANG_DIR)/bin/linux-gcc/ikpMP3.so $(IRRKLANG_DIR)/bin/linux-gcc/ikpFlac.so $(IRRLICHT_DIR)/lib/Linux/libIrrlicht.so.1.7.0-SVN 
all_linux clean_linux: SYSTEM=Linux


SUFFIXES : .o .cpp
.cpp.o :
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(CG_DEF) -c -o $@ $<
#	clang $(CPPFLAGS) $(CXXFLAGS) $(CG_DEF) -c -o $@ $<

all_linux: $(OBJS)
	$(CXX) -o $(DESTNAME) $(OBJS) $(LDFLAGS)
#	clang -o $(DESTNAME) $(OBJS) $(LDFLAGS)
#	$(CXX) -D__my_server__ -o server/own_server_udp server/own_server_udp.cpp
#	clang -D__my_server__ -o server/own_server_udp server/own_server_udp.cpp
#$(warning Building...)
#$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(srcs) -o $(DESTNAME) $(LDFLAGS)

clean: clean_linux
	$(warning Cleaning...)

clean_linux:
	@$(RM) $(OBJS)
	@$(RM) $(DESTNAME)
#	@$(RM) server/own_server_udp

.PHONY: all clean clean_linux
