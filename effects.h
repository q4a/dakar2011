/****************************************************************
*                                                               *
*    Name: effects.h                                            *
*                                                               *
*    Creator: Balazs Tuska                                      *
*                                                               *
*    Description:                                               *
*       This file contains the variables and fuctions that are  *
*       connected to the effects during the game. Most effects  *
*       are only available when the user use the Cg shaders.    *
*                                                               *
****************************************************************/

#ifndef __EFFECTS_H__
#define __EFFECTS_H__

#include "irrlicht.h"

// Irrlicht Namespaces
using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

extern u32 day_start_time;
extern float day_delta_multi;

extern bool update_collision_effect;
extern float coll_tick;
extern u32 coll_start_time;

extern float car_engine_effect;

extern float etick;

#define MAX_CAR_DIRT 15
extern const char* car_dirt_array[MAX_CAR_DIRT];
extern ITexture* car_dirttexture_array[MAX_CAR_DIRT];
extern float car_dirt;
extern u32 car_dirt_last_tick;
extern u32 car_dirt_delta;

void calculate_day_delta(u32 ptick);
void updateEffects(u32 ptick);

#endif // __EFFECTS_H__

