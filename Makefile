# Makefile for Irrlicht Examples
# It's usually sufficient to change just the target name and source file list
# and be sure that CXX is set to a valid compiler
SRCS = $(shell ls *.cpp)

# name of the binary - only valid for targets which set SYSTEM
DESTNAME = Dakar2011.bin
#DESTPATH = ./objs/

OBJS = $(SRCS:.cpp=.o)

IRRLICHT_SDK_VER = 17
#IRRLICHT_DIR = ../irrlicht-1.5.1
IRRLICHT_DIR = ../irrlicht-1.7.1
#IRRLICHT_DIR = ../irrlicht-svn/irrlicht-20090825
#IRRLICHT_DIR = ../irrlicht-svn/irrlicht
IRRKLANG_DIR = ../irrKlang-1.3.0
NEWTON_DIR = ../newtonSDK/sdk

MY_LIBS = ./lib
CG_DIR = ../Cg
CG_FLAGS = -lCg -lCgGL
CG_DEF = 
#CG_BINS = 
#CG_DEF = -DDISABLE_CG_SHADERS

# general compiler settings
CPPFLAGS = -I. -I$(IRRLICHT_DIR)/include -I$(IRRLICHT_DIR)/source/Irrlicht -I/usr/X11R6/include -I$(IRRKLANG_DIR)/include -I$(NEWTON_DIR) -I$(CG_DIR)/include
CXXFLAGS = -O3 -ffast-math -DIRRLICHT_SDK_$(IRRLICHT_SDK_VER) -DUSE_EDITOR -DIRR_CG_8 -DUSE_MY_SOUNDENGINE
# -DMY_DEBUG
#CXXFLAGS = -g -Wall

#default target is Linux
all: all_linux

ifeq ($(HOSTTYPE), x86_64)
LIBSELECT=64
endif

# target specific settings   -lGLEW
#all_linux: LDFLAGS = -L/usr/X11R6/lib$(LIBSELECT) -L$(IRRLICHT_DIR)/lib/Linux -L../newtonSDK/sdk -L$(MY_LIBS)-lGL -lopenal -lalut -lXxf86vm -lXext -lX11 -lpthread ./libs/libIrrlicht.so.1 ./libs/libIrrKlang.so ./libs/ikpMP3.so ./libs/ikpFlac.so $(CG_BINS) ./libs/libNewton.so
all_linux: LDFLAGS = -L/usr/X11R6/lib$(LIBSELECT) -L$(NEWTON_DIR) -L$(MY_LIBS) -lGL -lopenal -lalut -lXxf86vm -lXext -lX11 -lpthread -lIrrKlang -lNewton $(CG_FLAGS) $(MY_LIBS)/ikpMP3.so $(MY_LIBS)/ikpFlac.so $(IRRLICHT_DIR)/lib/Linux/libIrrlicht.so.1.7.0-SVN
# -L$(IRRLICHT_DIR)/lib/Linux  -lIrrlicht
all_linux clean_linux: SYSTEM=Linux
all_win32: LDFLAGS = -L../../lib/Win32-gcc -lIrrlicht -lopengl32 -lm
all_win32 clean_win32: SYSTEM=Win32-gcc
all_win32 clean_win32: SUF=.exe


SUFFIXES : .o .cpp
.cpp.o :
#	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(CG_DEF) -c -o $@ $<
	clang $(CPPFLAGS) $(CXXFLAGS) $(CG_DEF) -c -o $@ $<

all_linux all_win32: $(OBJS)
#	$(CXX) -o $(DESTNAME) $(OBJS) $(LDFLAGS)
	clang -o $(DESTNAME) $(OBJS) $(LDFLAGS)
	$(CXX) -D__my_server__ -o server/own_server_udp server/own_server_udp.cpp
#	clang -D__my_server__ -o server/own_server_udp server/own_server_udp.cpp
#$(warning Building...)
#$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(srcs) -o $(DESTNAME) $(LDFLAGS)

clean: clean_linux clean_win32
	$(warning Cleaning...)

clean_linux clean_win32:
	@$(RM) $(OBJS)
	@$(RM) $(DESTNAME)
	@$(RM) server/own_server_udp

.PHONY: all all_win32 clean clean_linux clean_win32
