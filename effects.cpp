/****************************************************************
*                                                               *
*    Name: effects.cpp                                          *
*                                                               *
*    Creator: Balazs Tuska                                      *
*                                                               *
*    Description:                                               *
*       This file contains the variables and fuctions that are  *
*       connected to the effects during the game. Most effects  *
*       are only available when the user use the Cg shaders.    *
*                                                               *
****************************************************************/

#include "effects.h"
#include "gameplay.h"
#include "settings.h"

#include <irrlicht.h>

using namespace irr;

const char* car_dirt_array[MAX_CAR_DIRT] =
{
    "data/vehicles/texture/dirt0.png",
    "data/vehicles/texture/dirt1.png",
    "data/vehicles/texture/dirt2.png",
    "data/vehicles/texture/dirt3.png",
    "data/vehicles/texture/dirt4.png",
    "data/vehicles/texture/dirt5.png",
    "data/vehicles/texture/dirt6.png",
    "data/vehicles/texture/dirt7.png",
    "data/vehicles/texture/dirt8.png",
    "data/vehicles/texture/dirt9.png",
    "data/vehicles/texture/dirt10.png",
    "data/vehicles/texture/dirt11.png",
    "data/vehicles/texture/dirt12.png",
    "data/vehicles/texture/dirt13.png",
    "data/vehicles/texture/dirt14.png"
};

ITexture* car_dirttexture_array[MAX_CAR_DIRT] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// day time vars
u32 day_start_time = 0;
//u32 day_delta_time = 0;
float day_delta_multi = 0.0f;


// car collision vars
bool update_collision_effect = false;
float coll_tick = 0.f;
u32 coll_start_time = 0;

// car engine effect
float car_engine_effect = -1.f;

// general
float etick;

// car dirt: 0.0 - most dirt, 1.0 - no dirt
float car_dirt = 0.0f;
u32 car_dirt_last_tick = 0;
u32 car_dirt_delta = 0;

//static u32 day_last_tick = 0;
//static u32 last_day_tick = 0;
void calculate_day_delta(u32 ptick)
{
    u32 day_delta_time = ptick - day_start_time;
    const u32 shour = 7; // 7
    /*
    if (stages[oldStage]->stagePart > 1)
    {
        shour = 14;
    }
    */
#define DAY_START (8)
#define DAY_END (19)
#define NIGHT_START (23)
#define NIGHT_END (4)
#define CALC_HOUR (60*1000*4)
#define CALC_HOUR_F (60.f*1000.f*4.f)
#define CALC_DAY (CALC_HOUR*24)
    u32 hour = (day_delta_time/CALC_HOUR + shour)%24; // stage start part 1 at 6, part 2 at 14
    if (DAY_START <= hour && hour < DAY_END) day_delta_multi = 1.0f;
    else
    if (NIGHT_START <= hour || hour < NIGHT_END) day_delta_multi = 0.1f;
    else
    if (hour < DAY_START) 
    {
        u32 delta = (day_delta_time+shour*CALC_HOUR)%CALC_DAY;
        u32 delta10 = (DAY_START*CALC_HOUR) - delta;
        //printf ("delta: %u delta10: %u\n", delta, delta10);
    
        day_delta_multi = 1.f - ((float)delta10 / ((4.f/0.9f)*CALC_HOUR_F));
    }
    else
    if (hour >= DAY_END) 
    {
        u32 delta = (day_delta_time+shour*CALC_HOUR)%CALC_DAY;
        u32 delta15 = delta - (DAY_END*CALC_HOUR);
    
        day_delta_multi = 1.f - ((float)delta15 / ((4.f/0.9f)*CALC_HOUR_F));
    }
    else
        day_delta_multi = 1.0f;

    if (useAdvCgShaders)
    {
        float rotateDeg = (360.f * (float)((day_delta_time+shour*CALC_HOUR)%CALC_DAY)) / (float)CALC_DAY;
        core::vector2df lightpos = core::vector2df(0.f, -150.f);
        lightpos.rotateBy(rotateDeg);
        
        lnode_4_shaders->setPosition(core::vector3df(lightpos.X, lightpos.Y, 20.f));
        //sunSphere->setPosition(lnode_4_shaders->getPosition());
    }

/*        
    if (!useShaders)
    {
        float lightColor = day_delta_multi - 0.3f * day_delta_multi;
        lnode->getLightData().DiffuseColor = video::SColorf(day_delta_multi,day_delta_multi,day_delta_multi);
        lnode->getLightData().AmbientColor = video::SColorf(lightColor,lightColor,lightColor);
        lnode->getLightData().SpecularColor = video::SColorf(lightColor,lightColor,lightColor);
    }
*/        
    //printf("hour: %u dm: %f ptick %u start %u\n", hour, day_delta_multi, ptick, day_start_time);
}

void calculate_dirt(u32 ptick)
{
#define DIRT_TIME 170000
#define MAX_DIRT_TIME (MAX_CAR_DIRT * DIRT_TIME)
    u32 delta_time = ptick - car_dirt_last_tick;
    car_dirt_last_tick = ptick;
    

    if (!car || fabsf(car->getSpeed())<10.f || car_dirt_delta > MAX_DIRT_TIME) return;
    
    car_dirt_delta += delta_time;

// 10 sec to full dirt = 600000.0f
/*
    if (car_dirt_delta > DIRT_TIME * 5)
        car_dirt = 1.0f;
    else
    {
        car_dirt = 0.5f + (float)car_dirt_delta / (float)(DIRT_TIME * 5);
        if (car_dirt > 1.f) car_dirt = 1.f;
    }
*/
    if (car_dirt_delta > DIRT_TIME * (car->getDirt()+1))
    {
        car->updateDirt();
        car_dirt = (float)car->getDirt()/(float)(MAX_CAR_DIRT-1);
    }
}

void updateEffects(u32 ptick)
{
    //calculate_day_delta(ptick);
    
    // collision effect
    if (update_collision_effect)
    {
        if (tick - coll_start_time < 500)
        {
           coll_tick = (float)(tick - coll_start_time) / 10.f;
           //printf("update collision effect %f\n", coll_tick);
        }
        else
           update_collision_effect = false;
    }

    // engine effect    
    if (car && car->getEngineIsGoing())
        car_engine_effect = (float)tick / 10.f;
    else
        car_engine_effect = -1.0f;
    
    // general effect
    etick = (float)tick / 1000.f;
    
    // dirt effect
    calculate_dirt(ptick);
}
