# Makefile for Irrlicht Examples
# It's usually sufficient to change just the target name and source file list
# and be sure that CXX is set to a valid compiler
srcs =    Dakar2010.cpp \
          BigTerrain.cpp \
          gameplay.cpp \
          Materials.cpp \
          message.cpp \
          my_shaders.cpp \
          my_shaders2.cpp \
          NewtonRaceCar.cpp \
          settings.cpp \
          SmallTerrain.cpp \
          pools.cpp \
          wrappers.cpp \
          multiplayer.cpp \
          eventreceiver_game.cpp \
          eventreceiver_menu.cpp \
          effects.cpp \
          CObjectWire.cpp \
          mySound.cpp \
          CustomDGRayCastCar.cpp \
          NewtonCustomJoint.cpp \
          dMathDefines.cpp \
          dMatrix.cpp \
          dQuaternion.cpp \
          CBillboardGroupSceneNode.cpp \
          CTreeGenerator.cpp \
          CTreeSceneNode.cpp \
          CGUITexturedSkin.cpp \
          CConfigMap.cpp \
          CConfigReader.cpp \
          CImageGUISkin.cpp \
          SkinLoader.cpp \
          IrrCg.cpp \
          IrrCgMaterial.cpp

# name of the binary - only valid for targets which set SYSTEM
DESTNAME = Dakar2010.bin
#DESTPATH = ./objs/

objs = $(srcs:.cpp=.o)

IRRLICHT_SDK_VER = 16
#IRRLICHT_DIR = ../irrlicht-1.5.1
#IRRLICHT_DIR = ../irrlicht-1.6
IRRLICHT_DIR = ../irrlicht-svn/irrlicht-20090825
#IRRLICHT_DIR = ../irrlicht-svn/irrlicht

CG_BINS = ./libs/libCg.so ./libs/libCgGL.so
CG_DEF = 
#CG_BINS = 
#CG_DEF = -DDISABLE_CG_SHADERS

# general compiler settings
CPPFLAGS = -I. -I$(IRRLICHT_DIR)/include -I$(IRRLICHT_DIR)/source/Irrlicht -I/usr/X11R6/include -I../irrKlang-1.2.0/include -I../newtonSDK/sdk -I../Cg/include
CXXFLAGS = -O3 -ffast-math -DUSE_MY_SOUNDENGINE
#CXXFLAGS = -g -Wall

#default target is Linux
all: all_linux

ifeq ($(HOSTTYPE), x86_64)
LIBSELECT=64
endif

# target specific settings   -lGLEW
all_linux: LDFLAGS = -L/usr/X11R6/lib$(LIBSELECT) -L$(IRRLICHT_DIR)/lib/Linux -L../newtonSDK/sdk -lGL -lopenal -lalut -lXxf86vm -lXext -lX11 -lpthread ./libs/libIrrlicht.so.1 ./libs/libIrrKlang.so ./libs/ikpMP3.so ./libs/ikpFlac.so $(CG_BINS) ./libs/libNewton.so
all_linux clean_linux: SYSTEM=Linux
all_win32: LDFLAGS = -L../../lib/Win32-gcc -lIrrlicht -lopengl32 -lm
all_win32 clean_win32: SYSTEM=Win32-gcc
all_win32 clean_win32: SUF=.exe


SUFFIXES : .o .cpp
.cpp.o :
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -DIRRLICHT_SDK_$(IRRLICHT_SDK_VER) $(CG_DEF) -c -o $@ $<

all_linux all_win32: $(objs)
	$(CXX) -o $(DESTNAME) $(objs) $(LDFLAGS)
	$(CXX) -D__my_server__ -o own_server_udp own_server_udp.cpp
#$(warning Building...)
#$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(srcs) -o $(DESTNAME) $(LDFLAGS)

clean: clean_linux clean_win32
	$(warning Cleaning...)

clean_linux clean_win32:
	@$(RM) $(objs)
	@$(RM) $(DESTNAME)

.PHONY: all all_win32 clean clean_linux clean_win32
