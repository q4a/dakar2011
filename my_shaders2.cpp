/****************************************************************
*                                                               *
*    Name: my_shaders2.cpp                                      *
*                                                               *
*    Creator: Balazs Tuska                                      *
*                                                               *
*    Description:                                               *
*       This file contains the Cg shader handling               *
*                                                               *
****************************************************************/
#include "my_shaders.h"
#ifndef DISABLE_CG_SHADERS

#include "error.h"
#include <Cg/cg.h>

// IrrCg
#include "IrrCg.h"

// Chernobyl Game Engine namespaces
using namespace IrrCg;

#ifdef __linux__
#include "linux_includes.h"
#endif

#include "settings.h"
#include "gameplay.h"
#include "effects.h"
#include <assert.h>

ICgProgrammingServices* gpu = 0;

s32 myMaterialType_light_tex = /*video::EMT_SOLID*/video::EMT_TRANSPARENT_ALPHA_CHANNEL;
s32 myMaterialType_light_notex = video::EMT_SOLID;
s32 myMaterialType_light_notex_car = video::EMT_SOLID;
s32 myMaterialType_light_2tex_2 = /*video::EMT_SOLID*/video::EMT_DETAIL_MAP;
s32 myMaterialType_ocean = video::EMT_SOLID/*video::EMT_NORMAL_MAP_SOLID video::EMT_SPHERE_MAP video::EMT_REFLECTION_2_LAYER*/;
s32 myMaterialType_ocean_fix = video::EMT_SOLID/*video::EMT_NORMAL_MAP_SOLID video::EMT_SPHERE_MAP video::EMT_REFLECTION_2_LAYER*/;
s32 myMaterialType_smoke = video::EMT_TRANSPARENT_ALPHA_CHANNEL;
s32 myMaterialType_light_tex_s = /*video::EMT_SOLID*/video::EMT_TRANSPARENT_ALPHA_CHANNEL;
s32 myMaterialType_light_tex_s_car = /*video::EMT_SOLID video::EMT_SPHERE_MAP*/video::EMT_REFLECTION_2_LAYER;
s32 myMaterialType_light_tex_s_car_tyre = video::EMT_SOLID;
s32 myMaterialType_transp = video::EMT_TRANSPARENT_ALPHA_CHANNEL;
s32 myMaterialType_road = video::EMT_SOLID;
s32 myMaterialType_transp_road = /*video::EMT_SOLID*/video::EMT_TRANSPARENT_ALPHA_CHANNEL;
s32 myMaterialType_transp_stat = video::EMT_TRANSPARENT_ALPHA_CHANNEL;
s32 myMaterialType_sky = video::EMT_SOLID;
s32 myMaterialType_shadow = video::EMT_SOLID;
s32 myMaterialType_depth = video::EMT_SOLID;
s32 myMaterialType_depthSun = video::EMT_SOLID;
s32 myMaterialType_screenRTT = video::EMT_SOLID;
s32 myMaterialType_palca = video::EMT_TRANSPARENT_ALPHA_CHANNEL;
s32 myMaterialType_sun = video::EMT_TRANSPARENT_ALPHA_CHANNEL;

// shadow map
video::ITexture* shadowMap = 0;
video::ITexture* shadowMapMenu = 0;
video::ITexture* shadowMapGame = 0;
video::ITexture* shadowMapCar = 0;
float renderToShadowMap = 0.f;
core::matrix4 projMat;
core::matrix4 viewMat;
float max_shadow = 0.f;
bool ableToUseShaders = true;

#define ADD_TEXTURE0 
/*
\
        if (Material.getTexture(0)) \
        { \
            tex0 = cgGetNamedParameter(Pixel, "tex0"); \
            if (tex0) \
                services->EnableTexture( tex0,Material.getTexture(0)); \
        }
*/
#define ADD_TEXTURE1 
/*
\
        if (Material.getTexture(1)) \
        { \
            tex1 = cgGetNamedParameter(Pixel, "tex1"); \
            if (tex1) \
                services->EnableTexture( tex1,Material.getTexture(1)); \
        }
*/

#define ADD_TEXTURE2
#define ADD_TEXTURE3 


#define ADD_EYEPOSITION \
        eyePosition = cgGetNamedParameter(Vertex, "eyePosition"); \
        if (eyePosition) \
        { \
            vector3df CameraPosition = device->getSceneManager()->getActiveCamera()->getPosition(); \
            services->setParameter3f(eyePosition, CameraPosition.X, CameraPosition.Y, CameraPosition.Z); \
        }

#define ADD_EYEPOSITION_SHINE \
        eyePosition = cgGetNamedParameter(Vertex, "eyePosition"); \
        if (eyePosition) \
        { \
            vector3df CameraPosition = device->getSceneManager()->getActiveCamera()->getPosition(); \
            services->setParameter3f(eyePosition, CameraPosition.X, CameraPosition.Y, CameraPosition.Z); \
            shininess = cgGetNamedParameter(Vertex, "shininess"); \
            if (shininess) \
                services->setParameter1f(shininess, 0.7f); \
        }

#define ADD_EYEPOSITIONF \
        eyePositionF = cgGetNamedParameter(Pixel, "eyePositionF"); \
        if (eyePositionF) \
        { \
            vector3df CameraPosition = device->getSceneManager()->getActiveCamera()->getPosition(); \
            services->setParameter3f(eyePositionF, CameraPosition.X, CameraPosition.Y, CameraPosition.Z); \
        }


// sunSphere->getScale().X; 
#define ADD_SUNPOSITIONF \
        sunPositionF = cgGetNamedParameter(Pixel, "sunPositionF"); \
        sunScaleF = cgGetNamedParameter(Pixel, "sunScaleF"); \
        if (sunPositionF && sunSphere) \
        { \
            vector3df sunPosition = sunSphere->getPosition(); \
            float sunScale = sunSphere->getSize().Width; \
            vector3df sunDir = (device->getSceneManager()->getActiveCamera()->getPosition() - sunPosition).normalize() * sunScale ; \
            sunPosition += sunDir; \
            services->setParameter3f(sunPositionF, sunPosition.X, sunPosition.Y, sunPosition.Z); \
            services->setParameter1f(sunScaleF, sunScale); \
        }

#define ADD_LIGHTPOS_F \
        plightPos = cgGetNamedParameter(Pixel, "lightPos"); \
        if (plightPos) \
        { \
            vector3df lp = lightCam->getPosition(); \
            services->setParameter3f(plightPos, lp.X, lp.Y, lp.Z); \
        }

#define ADD_PARAM \
        param = cgGetNamedParameter(Pixel, "param"); \
        if (param) \
           services->setParameter1f(param, Material.MaterialTypeParam);

#define ADD_PARAM2 \
        param2 = cgGetNamedParameter(Pixel, "param2"); \
        if (param2) \
           services->setParameter1f(param2, Material.MaterialTypeParam2);

#define ADD_CAR_ENGINE_EFFECT_V \
        pcar_engine = cgGetNamedParameter(Vertex, "car_engine"); \
        if (pcar_engine) \
            services->setParameter1f(pcar_engine, car_engine_effect);

#define ADD_CAR_SPEED_F \
        if (car) \
        { \
            pcar_speed = cgGetNamedParameter(Pixel, "car_speed"); \
            if (pcar_speed) \
                services->setParameter1f(pcar_speed, car->getSpeed()/120.f); \
        }

#define ADD_CAR_ENGINE_EFFECT_XY_V \
        pcar_engine = cgGetNamedParameter(Vertex, "car_engine_x"); \
        if (pcar_engine) \
            services->setParameter1f(pcar_engine, (sin(car_engine_effect)*0.002)); \
        pcar_engine = cgGetNamedParameter(Vertex, "car_engine_y"); \
        if (pcar_engine) \
            services->setParameter1f(pcar_engine, (cos(car_engine_effect)*0.002));

#define ADD_COLL_TICK_V \
        pcoll_tick = cgGetNamedParameter(Vertex, "coll_tick"); \
        if (pcoll_tick) \
            services->setParameter1f(pcoll_tick, update_collision_effect?coll_tick:-1.f);

#define ADD_COLL_TICK_XY_V \
        pcoll_tick = cgGetNamedParameter(Vertex, "coll_tick_x"); \
        if (pcoll_tick) \
            services->setParameter1f(pcoll_tick, update_collision_effect?(sin(coll_tick)*0.02):0.f); \
        pcoll_tick = cgGetNamedParameter(Vertex, "coll_tick_y"); \
        if (pcoll_tick) \
            services->setParameter1f(pcoll_tick, update_collision_effect?(cos(coll_tick)*0.02):0.f);

#define ADD_COLL_TICK_F \
        pcoll_tick = cgGetNamedParameter(Pixel, "coll_tick"); \
        if (pcoll_tick) \
            services->setParameter1f(pcoll_tick, update_collision_effect?coll_tick:-1.f);

#define ADD_TICK_V \
        ptick = cgGetNamedParameter(Vertex, "tick"); \
        if (ptick) \
            services->setParameter1f(ptick, (float)tick);

#define ADD_TICK \
        ptick = cgGetNamedParameter(Pixel, "tick"); \
        if (ptick) \
            services->setParameter1i(ptick, tick);

#define ADD_ETICK_F \
        petick = cgGetNamedParameter(Pixel, "etick"); \
        if (petick) \
            services->setParameter1f(petick, etick);

#define ADD_DAY_MULTI \
        day_multi = cgGetNamedParameter(Pixel, "day_multi"); \
        if (day_multi) \
            services->setParameter1f(day_multi, day_delta_multi); 

#define ADD_CAR_DIRT_F \
        pcar_dirt = cgGetNamedParameter(Pixel, "car_dirt"); \
        if (pcar_dirt) \
            services->setParameter1f(pcar_dirt, car_dirt); 

#define ADD_CAR_LIGHT \
        if (car && car->getLight()) \
        { \
            clightPos = cgGetNamedParameter(Pixel, "clightPos"); \
            if (clightPos) \
            { \
                services->setParameter3f(clightPos, cLightPos[12], cLightPos[13], cLightPos[14]); \
                clightDir = cgGetNamedParameter(Pixel, "clightDir"); \
                if (clightDir) \
                { \
                    services->setParameter3f(clightDir, cLightDir[12], cLightDir[13], cLightDir[14]); \
                } \
            } \
            clight = cgGetNamedParameter(Pixel, "clight"); \
            if (clight) \
                services->setParameter1f(clight, 1.f); \
        } \
        else \
        { \
            clight = cgGetNamedParameter(Pixel, "clight"); \
            if (clight) \
                services->setParameter1f(clight, 0.f); \
        }

#define ADD_OV_LIMIT_F \
        pov_limit = cgGetNamedParameter(Pixel, "ov_limit"); \
        if (pov_limit) \
            services->setParameter1f(pov_limit, objectVisibilityLimit); 
                    //printf ("%f %f %f\n", cLightDir[12], cLightDir[13], cLightDir[14]); 

#define ADD_RENDER_TO_SHADOW_MAP_V \
        prtsm = cgGetNamedParameter(Vertex, "rtsm"); \
        if (prtsm) \
            services->setParameter1f(prtsm, renderToShadowMap);

#define ADD_RENDER_TO_SHADOW_MAP_F \
        prtsm = cgGetNamedParameter(Pixel, "rtsm"); \
        if (prtsm) \
            services->setParameter1f(prtsm, renderToShadowMap);

#define ADD_MAX_SHADOW_F \
        pmax_shadow = cgGetNamedParameter(Pixel, "max_shadow"); \
        if (pmax_shadow) \
            services->setParameter1f(pmax_shadow, max_shadow);

//cgGetNamedParameter(Pixel, "shadowMap")
#define ADD_SHADOWMAP_F \
        if (shadows && shadowMap) \
        { \
            pshadowMap = 0; \
            if (pshadowMap) \
                services->EnableTextureSM(pshadowMap, shadowMap); \
            pshadowParam = cgGetNamedParameter(Pixel, "shadowParam"); \
            if (pshadowParam) \
                if(driverType == video::EDT_OPENGL /*|| driverType == video::EDT_OPENGL3*/) \
                    services->setParameter1f(pshadowParam, -1.f); \
                else \
                    services->setParameter1f(pshadowParam, 1.f); \
            pshadowRes = cgGetNamedParameter(Pixel, "shadowRes"); \
            if (pshadowRes) \
               services->setParameter1f(pshadowRes, (float)shadow_map_size); \
        } \
        else \
        { \
            pshadowParam = cgGetNamedParameter(Pixel, "shadowParam"); \
            if (pshadowParam) \
               services->setParameter1f(pshadowParam, 0.f); \
        }

#define ADD_FAR_VALUE_F \
        pfar_value = cgGetNamedParameter(Pixel, "far_value"); \
        if (pfar_value) \
            services->setParameter1f(pfar_value, farValue); 

#define ADD_DEPTH_F \
        pdepth = cgGetNamedParameter(Pixel, "depth"); \
        if (pdepth) \
            services->setParameter1f(pdepth, depth_effect?1.0f:0.0f); 
            
#define ADD_WORLD_VIEW_PROJ_V \
	    WorldViewProjection = cgGetNamedParameter(Vertex, "mWorldViewProj"); \
        core::matrix4 cworldViewProj; \
		cworldViewProj = driver->getTransform(video::ETS_PROJECTION); \
		cworldViewProj *= driver->getTransform(video::ETS_VIEW); \
		cworldViewProj *= driver->getTransform(video::ETS_WORLD); \
		services->setMatrix(WorldViewProjection,ICGT_MATRIX_IDENTITY,cworldViewProj);

#define ADD_WORLD_VIEW_PROJ_F \
	    WorldViewProjectionF = cgGetNamedParameter(Pixel, "mWorldViewProjF"); \
		services->setMatrix(WorldViewProjectionF,ICGT_MATRIX_IDENTITY,cworldViewProj);

#define ADD_WORLD_V \
	    World = cgGetNamedParameter(Vertex, "mWorld"); \
        core::matrix4 cworld; \
		cworld = driver->getTransform(video::ETS_WORLD); \
		services->setMatrix(World,ICGT_MATRIX_IDENTITY,cworld); 

#define ADD_WORLD_F \
	    WorldF = cgGetNamedParameter(Pixel, "mWorldF"); \
		services->setMatrix(WorldF,ICGT_MATRIX_IDENTITY,cworld); 

#define ADD_INV_WORLD_V \
	    InvWorld = cgGetNamedParameter(Vertex, "mInvWorld"); \
		core::matrix4 cinvWorld = driver->getTransform(video::ETS_WORLD); \
		cinvWorld.makeInverse(); \
		services->setMatrix(InvWorld,ICGT_MATRIX_IDENTITY,cinvWorld);

#define ADD_INV_WORLD_F \
	    InvWorldF = cgGetNamedParameter(Pixel, "mInvWorldF"); \
		services->setMatrix(InvWorldF,ICGT_MATRIX_IDENTITY,cinvWorld);

#define ADD_TRA_WORLD_V 
/*
	    TraWorld = cgGetNamedParameter(Vertex, "mTransWorld"); \
		core::matrix4 ctraWorld = driver->getTransform(video::ETS_WORLD); \
		ctraWorld = ctraWorld.getTransposed(); \
		services->setMatrix(TraWorld,ICGT_MATRIX_IDENTITY, ctraWorld); 
*/
#define ADD_TEXTURE_MATRIX_V \
	    ptextureMatrix = cgGetNamedParameter(Vertex, "mTextureMatrix"); \
	    if (ptextureMatrix) \
	    { \
            core::matrix4 textureMatrix; \
            textureMatrix = lightCam->getProjectionMatrix(); \
            textureMatrix *= lightCam->getViewMatrix(); \
            textureMatrix *= driver->getTransform(video::ETS_WORLD); \
        	services->setMatrix(ptextureMatrix,ICGT_MATRIX_IDENTITY,textureMatrix); \
        }

#define ADD_LIGHT_COL_V \
        lightColor = cgGetNamedParameter(Vertex, "mLightColor"); \
		video::SColorf col = m_lnode->getLightData().DiffuseColor; \
        services->setParameter3f(lightColor, col.r, col.g, col.b);

#define ADD_LIGHT_POS_V \
        lightPosition = cgGetNamedParameter(Vertex, "mLightPos"); \
		core::vector3df pos = m_lnode->getLightData().Position; \
        services->setParameter3f(lightPosition, pos.X, pos.Y, pos.Z);

#define ADD_ENVMAP_P 
/*
\
        if (skydome && skydome->getMaterial(0).getTexture(0)) \
        { \
            envMap = cgGetNamedParameter(Pixel, "envMap"); \
            if (envMap) \
            { \
                services->EnableTextureSM( envMap, skydome->getMaterial(0).getTexture(0)); \
            } \
        }
*/

/*
core::matrix4 worldViewProj;
core::matrix4 world;
core::matrix4 invWorld;
core::matrix4 traWorld;
core::matrix4 textureMatrix;
#define ADD_WORLD_VIEW_PROJ_V \
	    WorldViewProjection = cgGetNamedParameter(Vertex, "mWorldViewProj"); \
		services->setMatrix(WorldViewProjection,ICGT_MATRIX_IDENTITY,worldViewProj);

#define ADD_WORLD_V \
	    World = cgGetNamedParameter(Vertex, "mWorld"); \
		services->setMatrix(World,ICGT_MATRIX_IDENTITY,world); 

#define ADD_INV_WORLD_V \
	    InvWorld = cgGetNamedParameter(Vertex, "mInvWorld"); \
		services->setMatrix(InvWorld,ICGT_MATRIX_IDENTITY,invWorld);

#define ADD_TRA_WORLD_V \
	    TraWorld = cgGetNamedParameter(Vertex, "mTransWorld"); \
		services->setMatrix(TraWorld,ICGT_MATRIX_IDENTITY, traWorld); 
*/

/*
static float get_day_multi()
{
    return day_delta_multi;
}
*/
// part of scene: no transparent texture with light, and the terrain, and the car

class MyShaderCallBack2_light_2tex_2 : public ICgShaderConstantSetCallBack
{
public:
    CGparameter	WorldViewProjection, InvWorld, TraWorld, World,
                lightColor, lightPosition,
                eyePosition, shininess,
                tex0, tex1,
                tex2, tex3,
                day_multi,
                clightPos, clightDir, clight,
                eyePositionF,
                pshadowMap, pshadowParam, pshadowRes,
                ptextureMatrix
                ;
public:
    MyShaderCallBack2_light_2tex_2(IrrlichtDevice* pdevice,
                     video::IVideoDriver* pdriver,
                     scene::ILightSceneNode* plnode) :
        device(pdevice), driver(pdriver), m_lnode(plnode)
    {
    }

    virtual void OnSetConstants(ICgServices* services,const CGeffect& Effect,const CGpass& Pass,const CGprogram& Vertex,const CGprogram& Pixel,const irr::video::SMaterial& Material)
	{
	    // Vertex Shader
        ADD_WORLD_VIEW_PROJ_V
        ADD_WORLD_V
        ADD_INV_WORLD_V
        ADD_TRA_WORLD_V
        ADD_TEXTURE_MATRIX_V
        ADD_LIGHT_COL_V
        ADD_LIGHT_POS_V
        ADD_EYEPOSITION_SHINE
        // Pixel Shader
        ADD_TEXTURE0
        ADD_TEXTURE1
        ADD_TEXTURE2
        ADD_TEXTURE3
        ADD_CAR_LIGHT
        ADD_DAY_MULTI
        ADD_EYEPOSITIONF
        ADD_SHADOWMAP_F
	}

public:
	IrrlichtDevice* device;
	scene::ILightSceneNode* m_lnode;
    video::IVideoDriver* driver;
	const video::SMaterial *UsedMaterial;
};

class MyShaderCallBack2_ocean : public ICgShaderConstantSetCallBack
{
public:
    CGparameter	WorldViewProjection, InvWorld, TraWorld, World,
                ptick,
                lightColor, lightPosition,
                eyePosition, shininess,
                tex0, tex1,
                tex2, tex3,
                day_multi,
                clightPos, clightDir, clight,
                eyePositionF,
                pshadowMap, pshadowParam, pshadowRes,
                ptextureMatrix
                ;
public:
    MyShaderCallBack2_ocean(IrrlichtDevice* pdevice,
                     video::IVideoDriver* pdriver,
                     scene::ILightSceneNode* plnode) :
        device(pdevice), driver(pdriver), m_lnode(plnode)
    {
    }

    virtual void OnSetConstants(ICgServices* services,const CGeffect& Effect,const CGpass& Pass,const CGprogram& Vertex,const CGprogram& Pixel,const irr::video::SMaterial& Material)
	{
	    // Vertex Shader
        ADD_WORLD_VIEW_PROJ_V
//        ADD_WORLD_V
//        ADD_INV_WORLD_V
        //ADD_TRA_WORLD_V
//        ADD_TEXTURE_MATRIX_V
//        ADD_LIGHT_COL_V
//        ADD_LIGHT_POS_V
        ADD_EYEPOSITION
        ADD_TICK_V
        // Pixel Shader
        ADD_TEXTURE0
        ADD_TEXTURE1
        ADD_TEXTURE2
//        ADD_CAR_LIGHT
        ADD_DAY_MULTI
        //ADD_EYEPOSITIONF
//        ADD_SHADOWMAP_F
	}

public:
	IrrlichtDevice* device;
	scene::ILightSceneNode* m_lnode;
    video::IVideoDriver* driver;
	const video::SMaterial *UsedMaterial;
};

class MyShaderCallBack2_light_tex : public ICgShaderConstantSetCallBack
{
public:
    CGparameter	WorldViewProjection, InvWorld, TraWorld, World,
                lightColor, lightPosition,
                tex0,
                day_multi,
                clightPos, clightDir, clight,
                eyePositionF,
                pov_limit,
                petick
                ;
public:
    MyShaderCallBack2_light_tex(IrrlichtDevice* pdevice,
                     video::IVideoDriver* pdriver,
                     scene::ILightSceneNode* plnode) :
        device(pdevice), driver(pdriver), m_lnode(plnode)
    {
    }

    virtual void OnSetConstants(ICgServices* services,const CGeffect& Effect,const CGpass& Pass,const CGprogram& Vertex,const CGprogram& Pixel,const irr::video::SMaterial& Material)
	{
	    // Vertex Shader
        ADD_WORLD_VIEW_PROJ_V
        ADD_WORLD_V
        ADD_INV_WORLD_V
        ADD_TRA_WORLD_V
        ADD_LIGHT_COL_V
        ADD_LIGHT_POS_V
        // Pixel Shader
        ADD_TEXTURE0
        ADD_CAR_LIGHT
        ADD_DAY_MULTI
        ADD_EYEPOSITIONF
        ADD_OV_LIMIT_F
        ADD_ETICK_F
	}

public:
	IrrlichtDevice* device;
	scene::ILightSceneNode* m_lnode;
    video::IVideoDriver* driver;
	const video::SMaterial *UsedMaterial;
};

class MyShaderCallBack2_light_notex : public ICgShaderConstantSetCallBack
{
public:
    CGparameter	WorldViewProjection, InvWorld, TraWorld, World,
                lightColor, lightPosition,
                day_multi,
                clightPos, clightDir, clight,
                eyePositionF,
                pov_limit,
                petick,
                envMap, eyePosition // envmap
                ;
public:
    MyShaderCallBack2_light_notex(IrrlichtDevice* pdevice,
                     video::IVideoDriver* pdriver,
                     scene::ILightSceneNode* plnode) :
        device(pdevice), driver(pdriver), m_lnode(plnode)
    {
    }

    virtual void OnSetConstants(ICgServices* services,const CGeffect& Effect,const CGpass& Pass,const CGprogram& Vertex,const CGprogram& Pixel,const irr::video::SMaterial& Material)
	{
	    // Vertex Shader
        ADD_WORLD_VIEW_PROJ_V
        ADD_WORLD_V
        ADD_INV_WORLD_V
        ADD_TRA_WORLD_V
        ADD_LIGHT_COL_V
        ADD_LIGHT_POS_V
        ADD_EYEPOSITION
        // Pixel Shader
        ADD_ENVMAP_P // envMap
        ADD_CAR_LIGHT
        ADD_DAY_MULTI
        ADD_EYEPOSITIONF
        ADD_OV_LIMIT_F
        ADD_ETICK_F
	}

public:
	IrrlichtDevice* device;
	scene::ILightSceneNode* m_lnode;
    video::IVideoDriver* driver;
	const video::SMaterial *UsedMaterial;
};

class MyShaderCallBack2_light_tex_s : public ICgShaderConstantSetCallBack
{
public:
    CGparameter	WorldViewProjection, InvWorld, TraWorld, World,
                lightColor, lightPosition,
                eyePosition, shininess,
                tex0,
                day_multi,
                clightPos, clightDir, clight,
                eyePositionF,
                pov_limit,
                petick
                ;
public:
    MyShaderCallBack2_light_tex_s(IrrlichtDevice* pdevice,
                     video::IVideoDriver* pdriver,
                     scene::ILightSceneNode* plnode) :
        device(pdevice), driver(pdriver), m_lnode(plnode)
    {
    }

    virtual void OnSetConstants(ICgServices* services,const CGeffect& Effect,const CGpass& Pass,const CGprogram& Vertex,const CGprogram& Pixel,const irr::video::SMaterial& Material)
	{
	    // Vertex Shader
        ADD_WORLD_VIEW_PROJ_V
        ADD_WORLD_V
        ADD_INV_WORLD_V
        ADD_TRA_WORLD_V
        ADD_LIGHT_COL_V
        ADD_LIGHT_POS_V
        ADD_EYEPOSITION_SHINE
        // Pixel Shader
        ADD_TEXTURE0
        ADD_CAR_LIGHT
        ADD_DAY_MULTI
        ADD_EYEPOSITIONF
        ADD_OV_LIMIT_F
        ADD_ETICK_F
	}

public:
	IrrlichtDevice* device;
	scene::ILightSceneNode* m_lnode;
    video::IVideoDriver* driver;
	const video::SMaterial *UsedMaterial;
};

class MyShaderCallBack2_light_tex_s_car : public ICgShaderConstantSetCallBack
{
public:
    CGparameter	WorldViewProjection, InvWorld, TraWorld, World,
                lightColor, lightPosition,
                eyePosition, shininess,
                tex0, tex1,
                day_multi,
                eyePositionF,
                pcoll_tick,
                pcar_engine,
                pshadowMap, pshadowParam, pshadowRes,
                ptextureMatrix,
                plightPos, pfar_value,
                pcar_dirt,
                envMap
                ;
public:
    MyShaderCallBack2_light_tex_s_car(IrrlichtDevice* pdevice,
                     video::IVideoDriver* pdriver,
                     scene::ILightSceneNode* plnode) :
        device(pdevice), driver(pdriver), m_lnode(plnode)
    {
    }

    virtual void OnSetConstants(ICgServices* services,const CGeffect& Effect,const CGpass& Pass,const CGprogram& Vertex,const CGprogram& Pixel,const irr::video::SMaterial& Material)
	{
	    // Vertex Shader
        ADD_WORLD_VIEW_PROJ_V
        ADD_WORLD_V
        ADD_INV_WORLD_V
        ADD_TRA_WORLD_V
        ADD_TEXTURE_MATRIX_V
        ADD_LIGHT_COL_V
        ADD_LIGHT_POS_V
        ADD_EYEPOSITION_SHINE
        ADD_COLL_TICK_XY_V
        ADD_CAR_ENGINE_EFFECT_XY_V
        // Pixel Shader
        ADD_TEXTURE0
        ADD_TEXTURE1
        ADD_DAY_MULTI
        ADD_EYEPOSITIONF
        ADD_LIGHTPOS_F
        ADD_SHADOWMAP_F
        ADD_FAR_VALUE_F
        ADD_CAR_DIRT_F
        ADD_ENVMAP_P
	}

public:
	IrrlichtDevice* device;
	scene::ILightSceneNode* m_lnode;
    video::IVideoDriver* driver;
	const video::SMaterial *UsedMaterial;
};

class MyShaderCallBack2_light_tex_s_car_tyre : public ICgShaderConstantSetCallBack
{
public:
    CGparameter	WorldViewProjection, InvWorld, TraWorld, World,
                lightColor, lightPosition,
                eyePosition, shininess,
                tex0, tex1,
                day_multi,
                eyePositionF,
                pcoll_tick,
                pcar_engine,
                pcar_dirt
                ;
public:
    MyShaderCallBack2_light_tex_s_car_tyre(IrrlichtDevice* pdevice,
                     video::IVideoDriver* pdriver,
                     scene::ILightSceneNode* plnode) :
        device(pdevice), driver(pdriver), m_lnode(plnode)
    {
    }

    virtual void OnSetConstants(ICgServices* services,const CGeffect& Effect,const CGpass& Pass,const CGprogram& Vertex,const CGprogram& Pixel,const irr::video::SMaterial& Material)
	{
	    // Vertex Shader
        ADD_WORLD_VIEW_PROJ_V
        ADD_WORLD_V
        ADD_INV_WORLD_V
        ADD_TRA_WORLD_V
        ADD_LIGHT_COL_V
        ADD_LIGHT_POS_V
        ADD_EYEPOSITION_SHINE
        ADD_COLL_TICK_XY_V
        //ADD_CAR_ENGINE_EFFECT_V
        // Pixel Shader
        ADD_TEXTURE0
        ADD_TEXTURE1
        ADD_DAY_MULTI
        ADD_CAR_DIRT_F
	}

public:
	IrrlichtDevice* device;
	scene::ILightSceneNode* m_lnode;
    video::IVideoDriver* driver;
	const video::SMaterial *UsedMaterial;
};

// part of scene: smoke
class MyShaderCallBack2_tr_light : public ICgShaderConstantSetCallBack
{
public:
    CGparameter	WorldViewProjection, tex0, tex1, day_multi, prtsm, param2;
public:
    MyShaderCallBack2_tr_light(IrrlichtDevice* pdevice,
                     video::IVideoDriver* pdriver,
                     scene::ILightSceneNode* plnode) :
        device(pdevice), driver(pdriver), m_lnode(plnode) {}

    virtual void OnSetConstants(ICgServices* services,const CGeffect& Effect,const CGpass& Pass,const CGprogram& Vertex,const CGprogram& Pixel,const irr::video::SMaterial& Material)
	{
	    // Vertex Shader
	    ADD_WORLD_VIEW_PROJ_V

	    //ADD_RENDER_TO_SHADOW_MAP_V

        // Pixel Shader
        ADD_TEXTURE0

        ADD_DAY_MULTI
        
        ADD_PARAM2
        //ADD_RENDER_TO_SHADOW_MAP_F
	}

public:
	IrrlichtDevice* device;
	scene::ILightSceneNode* m_lnode;
    video::IVideoDriver* driver;
	const video::SMaterial *UsedMaterial;
};

// part of scene: sky
class MyShaderCallBack2_sky : public ICgShaderConstantSetCallBack
{
public:
    CGparameter	WorldViewProjection,
                tex0, tex1,
                day_multi,
                eyePosition,
                eyePositionF,
                ptick,
                prtsm;
public:
    MyShaderCallBack2_sky(IrrlichtDevice* pdevice,
                     video::IVideoDriver* pdriver,
                     scene::ILightSceneNode* plnode) :
        device(pdevice), driver(pdriver), m_lnode(plnode) {}

    virtual void OnSetConstants(ICgServices* services,const CGeffect& Effect,const CGpass& Pass,const CGprogram& Vertex,const CGprogram& Pixel,const irr::video::SMaterial& Material)
	{
	    // Vertex Shader
	    ADD_WORLD_VIEW_PROJ_V
	    
        //ADD_EYEPOSITION
        
        //ADD_RENDER_TO_SHADOW_MAP_V

        // Pixel Shader
        ADD_TEXTURE0
        ADD_TEXTURE1

        ADD_DAY_MULTI

        ADD_EYEPOSITIONF
        
        //ADD_RENDER_TO_SHADOW_MAP_F
        
        //ADD_TICK
        
	}

public:
	IrrlichtDevice* device;
	scene::ILightSceneNode* m_lnode;
    video::IVideoDriver* driver;
	const video::SMaterial *UsedMaterial;
};

// part of scene: road (non-transparent)
class MyShaderCallBack2_road : public ICgShaderConstantSetCallBack
{
public:
    CGparameter	WorldViewProjection, InvWorld, TraWorld, World,
                lightColor, lightPosition,
                eyePosition, shininess,
                tex0,
                day_multi,
                clightPos, clightDir, clight,
                eyePositionF,
                pshadowMap, pshadowParam, pshadowRes,
                ptextureMatrix
                ;
public:
    MyShaderCallBack2_road(IrrlichtDevice* pdevice,
                     video::IVideoDriver* pdriver,
                     scene::ILightSceneNode* plnode) :
        device(pdevice), driver(pdriver), m_lnode(plnode)
    {
    }

    virtual void OnSetConstants(ICgServices* services,const CGeffect& Effect,const CGpass& Pass,const CGprogram& Vertex,const CGprogram& Pixel,const irr::video::SMaterial& Material)
	{
	    // Vertex Shader
        ADD_WORLD_VIEW_PROJ_V
        ADD_WORLD_V
        ADD_INV_WORLD_V
        ADD_TRA_WORLD_V
        ADD_TEXTURE_MATRIX_V
        ADD_LIGHT_COL_V
        ADD_LIGHT_POS_V
        ADD_EYEPOSITION_SHINE
        // Pixel Shader
        ADD_TEXTURE0
        ADD_CAR_LIGHT
        ADD_DAY_MULTI
        ADD_EYEPOSITIONF
        ADD_SHADOWMAP_F
	}

public:
	IrrlichtDevice* device;
	scene::ILightSceneNode* m_lnode;
    video::IVideoDriver* driver;
	const video::SMaterial *UsedMaterial;
};

// part of scene: grass, road (transparent)
class MyShaderCallBack2_transp : public ICgShaderConstantSetCallBack
{
public:
    CGparameter	WorldViewProjection, World,
                tex0, param, day_multi,
                clightPos, clightDir, clight,
                ptick,
                eyePosition,
                eyePositionF,
                pov_limit,
                petick,
                prtsm,
                pshadowMap, pshadowParam, pshadowRes,
                ptextureMatrix
                ;
public:
    MyShaderCallBack2_transp(IrrlichtDevice* pdevice,
                     video::IVideoDriver* pdriver,
                     scene::ILightSceneNode* plnode) :
        device(pdevice), driver(pdriver), m_lnode(plnode) {}

    virtual void OnSetConstants(ICgServices* services,const CGeffect& Effect,const CGpass& Pass,const CGprogram& Vertex,const CGprogram& Pixel,const irr::video::SMaterial& Material)
	{
	    // Vertex Shader
	    ADD_WORLD_VIEW_PROJ_V
        
	    ADD_WORLD_V

        ADD_TEXTURE_MATRIX_V
	    
        // Pixel Shader
        ADD_TEXTURE0
        
        ADD_PARAM
        
        ADD_CAR_LIGHT

        ADD_DAY_MULTI

        ADD_EYEPOSITIONF
        
        ADD_OV_LIMIT_F
        //ADD_TICK
        ADD_ETICK_F

        ADD_SHADOWMAP_F
	}

public:
	IrrlichtDevice* device;
	scene::ILightSceneNode* m_lnode;
    video::IVideoDriver* driver;
	const video::SMaterial *UsedMaterial;
};

// shadow
class MyShaderCallBack2_shadow : public ICgShaderConstantSetCallBack
{
public:
    CGparameter	WorldViewProjection, World,
                plightPos,
                pfar_value, pov_limit,
                pmax_shadow
                ;
public:
    MyShaderCallBack2_shadow(IrrlichtDevice* pdevice,
                     video::IVideoDriver* pdriver,
                     scene::ILightSceneNode* plnode) :
        device(pdevice), driver(pdriver), m_lnode(plnode) {}

    virtual void OnSetConstants(ICgServices* services,const CGeffect& Effect,const CGpass& Pass,const CGprogram& Vertex,const CGprogram& Pixel,const irr::video::SMaterial& Material)
	{
	    // Vertex Shader
	    ADD_WORLD_VIEW_PROJ_V
        
	    ADD_WORLD_V

        // Pixel Shader
        ADD_LIGHTPOS_F
        
        ADD_FAR_VALUE_F
        ADD_OV_LIMIT_F
        ADD_MAX_SHADOW_F
	}

public:
	IrrlichtDevice* device;
	scene::ILightSceneNode* m_lnode;
    video::IVideoDriver* driver;
//	const video::SMaterial *UsedMaterial;
};

// depth
class MyShaderCallBack2_depth : public ICgShaderConstantSetCallBack
{
public:
    CGparameter	WorldViewProjection, World,
                eyePositionF,
                pfar_value, pov_limit,
                pcar_speed
                ;
public:
    MyShaderCallBack2_depth(IrrlichtDevice* pdevice,
                     video::IVideoDriver* pdriver,
                     scene::ILightSceneNode* plnode) :
        device(pdevice), driver(pdriver), m_lnode(plnode) {}

    virtual void OnSetConstants(ICgServices* services,const CGeffect& Effect,const CGpass& Pass,const CGprogram& Vertex,const CGprogram& Pixel,const irr::video::SMaterial& Material)
	{
	    // Vertex Shader
	    ADD_WORLD_VIEW_PROJ_V
        
	    ADD_WORLD_V

        // Pixel Shader
//        ADD_EYEPOSITIONF
        
//        ADD_FAR_VALUE_F
//        ADD_OV_LIMIT_F
//        ADD_CAR_SPEED_F
	}

public:
	IrrlichtDevice* device;
	scene::ILightSceneNode* m_lnode;
    video::IVideoDriver* driver;
//	const video::SMaterial *UsedMaterial;
};

class MyShaderCallBack2_depthSun : public ICgShaderConstantSetCallBack
{
public:
    CGparameter	WorldViewProjection, World, WorldViewProjectionF, WorldF,
                eyePositionF, sunPositionF, sunScaleF,
                pfar_value, pov_limit,
                pcar_speed
                ;
public:
    MyShaderCallBack2_depthSun(IrrlichtDevice* pdevice,
                     video::IVideoDriver* pdriver,
                     scene::ILightSceneNode* plnode) :
        device(pdevice), driver(pdriver), m_lnode(plnode) {}

    virtual void OnSetConstants(ICgServices* services,const CGeffect& Effect,const CGpass& Pass,const CGprogram& Vertex,const CGprogram& Pixel,const irr::video::SMaterial& Material)
	{
	    // Vertex Shader
	    ADD_WORLD_VIEW_PROJ_V
        
	    ADD_WORLD_V

        // Pixel Shader
//        ADD_EYEPOSITIONF

        ADD_SUNPOSITIONF

//	    ADD_WORLD_F
        
//        ADD_CAR_SPEED_F
	}

public:
	IrrlichtDevice* device;
	scene::ILightSceneNode* m_lnode;
    video::IVideoDriver* driver;
//	const video::SMaterial *UsedMaterial;
};

// screenRTT
class MyShaderCallBack2_screenRTT : public ICgShaderConstantSetCallBack
{
public:
    CGparameter	tex0, tex1, tex2,
                pcoll_tick,
                pdepth,
                petick,
                pcar_speed
                ;
public:
    MyShaderCallBack2_screenRTT(IrrlichtDevice* pdevice,
                     video::IVideoDriver* pdriver,
                     scene::ILightSceneNode* plnode) :
        device(pdevice), driver(pdriver), m_lnode(plnode) {}

    virtual void OnSetConstants(ICgServices* services,const CGeffect& Effect,const CGpass& Pass,const CGprogram& Vertex,const CGprogram& Pixel,const irr::video::SMaterial& Material)
	{
        ADD_TEXTURE0
        ADD_TEXTURE1
        ADD_TEXTURE2
        ADD_TEXTURE3
        ADD_COLL_TICK_F
        ADD_DEPTH_F
        ADD_ETICK_F
        ADD_CAR_SPEED_F
	}

public:
	IrrlichtDevice* device;
	scene::ILightSceneNode* m_lnode;
    video::IVideoDriver* driver;
//	const video::SMaterial *UsedMaterial;
};

// palca
class MyShaderCallBack2_palca : public ICgShaderConstantSetCallBack
{
public:
    CGparameter	tex0;
public:
    MyShaderCallBack2_palca(IrrlichtDevice* pdevice,
                     video::IVideoDriver* pdriver,
                     scene::ILightSceneNode* plnode) :
        device(pdevice), driver(pdriver), m_lnode(plnode) {}

    virtual void OnSetConstants(ICgServices* services,const CGeffect& Effect,const CGpass& Pass,const CGprogram& Vertex,const CGprogram& Pixel,const irr::video::SMaterial& Material)
	{
        ADD_TEXTURE0
	}

public:
	IrrlichtDevice* device;
	scene::ILightSceneNode* m_lnode;
    video::IVideoDriver* driver;
//	const video::SMaterial *UsedMaterial;
};

// ----------------------------- USED SHADER END --------------------------------------

#endif /* DISABLE_CG_SHADERS */
void setupShaders2 (IrrlichtDevice* device,
                   IVideoDriver* driver,
                   video::E_DRIVER_TYPE driverType,
                   ISceneManager* smgr,
                   scene::ICameraSceneNode* camera,
                   scene::ILightSceneNode* plnode
                   )
{
#ifndef DISABLE_CG_SHADERS 

	core::stringc light_tex_fileName;
	core::stringc light_tex_s_fileName;
	core::stringc light_tex_s_car_fileName;
	core::stringc light_tex_s_car_tyre_fileName;

	core::stringc light_notex_fileName;

	core::stringc light_notex_car_fileName;

	core::stringc light_2tex_2_fileName;

	core::stringc ocean_fileName; // filename for the vertex shader
	core::stringc ocean_fix_fileName; // filename for the vertex shader

    core::stringc smoke_fileName; // filename for the pixel shader
    core::stringc transp_fileName;
    core::stringc road_fileName;
    core::stringc transp_road_fileName;
    core::stringc transp_stat_fileName;
    core::stringc sky_fileName;
    core::stringc shadow_fileName;
    core::stringc depth_fileName;
    core::stringc depthSun_fileName;
    core::stringc screenRTT_fileName;
    core::stringc palca_fileName;
    const c8* ogl_vs_version;
    const c8* ogl_ps_version;
    const c8* d3d_vs_version;
    const c8* d3d_ps_version;

	ogl_vs_version = "arbvp1";
	ogl_ps_version = "arbfp1";
	d3d_vs_version = "vs_2_0";
	d3d_ps_version = "ps_2_0";

    core::stringc cg_dir = "data/shaders/cg/";

    
	if ((driverType == video::EDT_DIRECT3D9 && driver->queryFeature(video::EVDF_HLSL) &&
         driver->queryFeature(video::EVDF_PIXEL_SHADER_2_0) && driver->queryFeature(video::EVDF_VERTEX_SHADER_2_0)
        ) ||
		((driverType == video::EDT_OPENGL /*|| driverType == video::EDT_OPENGL3*/) && driver->queryFeature(video::EVDF_ARB_GLSL) &&
         driver->queryFeature(video::EVDF_ARB_FRAGMENT_PROGRAM_1) && driver->queryFeature(video::EVDF_ARB_VERTEX_PROGRAM_1)
        )
       )
	{
		dprintf(printf("use high level cg shaders.\n"));
	}
	else
	{
		dprintf(printf("Not use high level shaders, because of missing driver/hardware support.\n"));
        ableToUseShaders = useShaders = useCgShaders = useAdvCgShaders = false;
        return;
    }
    if (!useCgShaders) return;

    dprintf(printf("create new gpu\n"));
    gpu = new ICgProgrammingServices(device/*, false, true*/);
    dprintf(printf("create new gpu end\n"));
    ICgServices* cgServices = gpu->getCgServices();

    printf("CG support:\n");
    printf("arbfp1:    %d\n", cgServices->isProfileSupported("arbfp1"));
    printf("arbvp1:    %d\n", cgServices->isProfileSupported("arbvp1"));
    printf("fp20:      %d\n", cgServices->isProfileSupported("fp20"));
    printf("fp30:      %d\n", cgServices->isProfileSupported("fp30"));
    printf("fp40:      %d\n", cgServices->isProfileSupported("fp40"));
    printf("glslf:     %d\n", cgServices->isProfileSupported("glslf"));
    printf("glslg:     %d\n", cgServices->isProfileSupported("glslg"));
    printf("glslv:     %d\n", cgServices->isProfileSupported("glslv"));
    printf("gp4:       %d\n", cgServices->isProfileSupported("gp4"));
    printf("gp4fp:     %d\n", cgServices->isProfileSupported("gp4fp"));
    printf("gp4gp:     %d\n", cgServices->isProfileSupported("gp4gp"));
    printf("gp4vp:     %d\n", cgServices->isProfileSupported("gp4vp"));
    printf("hlslf:     %d\n", cgServices->isProfileSupported("hlslf"));
    printf("hlslv:     %d\n", cgServices->isProfileSupported("hlslv"));
    printf("ps_1_1:    %d\n", cgServices->isProfileSupported("ps_1_1"));
    printf("ps_1_2:    %d\n", cgServices->isProfileSupported("ps_1_2"));
    printf("ps_1_3:    %d\n", cgServices->isProfileSupported("ps_1_3"));
    printf("ps_2_0:    %d\n", cgServices->isProfileSupported("ps_2_0"));
    printf("ps_2_x:    %d\n", cgServices->isProfileSupported("ps_2_x"));
    printf("ps_3_0:    %d\n", cgServices->isProfileSupported("ps_3_0"));
    printf("ps_4_0:    %d\n", cgServices->isProfileSupported("ps_4_0"));
    printf("vp20:      %d\n", cgServices->isProfileSupported("vp20"));
    printf("vp30:      %d\n", cgServices->isProfileSupported("vp30"));
    printf("vp40:      %d\n", cgServices->isProfileSupported("vp40"));
    printf("vs_4_0:    %d\n", cgServices->isProfileSupported("vs_4_0"));
    printf("gs_4_0:    %d\n", cgServices->isProfileSupported("gs_4_0"));


    if (driver->queryFeature(video::EVDF_RENDER_TO_TARGET))
    {
        dprintf(printf("shadow map is supported\n"));

        bool tempTexFlagMipMaps = driver->getTextureCreationFlag(ETCF_CREATE_MIP_MAPS);
        bool tempTexFlag32 = driver->getTextureCreationFlag(ETCF_ALWAYS_32_BIT);

        //driver->setTextureCreationFlag(ETCF_CREATE_MIP_MAPS, false);
        //driver->setTextureCreationFlag(ETCF_ALWAYS_32_BIT, tempTexFlag32);

        shadowMap = shadowMapGame = shadows ? driver->addRenderTargetTexture(core::dimension2d<u32>(shadow_map_size,shadow_map_size), "RTT1")
                             : 0;
        shadowMapCar = shadows ? driver->addRenderTargetTexture(core::dimension2d<u32>(shadow_map_size,shadow_map_size), "RTT1")
                             : 0;
        dprintf(printf("shadow has mip-maps: %s\n", (shadowMap && shadowMap->hasMipMaps())?"yes":"no"));

        //driver->setTextureCreationFlag(ETCF_CREATE_MIP_MAPS, false);
        shadowMapMenu = driver->getTexture("data/menu_textures/menu_shadow.png");
        
        driver->setTextureCreationFlag(ETCF_CREATE_MIP_MAPS, tempTexFlagMipMaps);
        driver->setTextureCreationFlag(ETCF_ALWAYS_32_BIT, tempTexFlag32);

        //hudImage->setImage(shadowMap);
    }
    else
    {
        dprintf(printf("shadow map is not supported, because of lack of the RTT support\n"));
        shadowMap = shadowMapGame = shadowMapMenu = 0;
    }

    // possible to use shaders, let see if possible to use vs and ps 3.0
    if (useAdvCgShaders)
    {
        //if (driverType == video::EDT_DIRECT3D9) // FIXME: irrlicht 1.7.1 has a MRT bug in D3D9 driver
        //{
        //    dprintf(printf("MRT bug adv shader -> disable.\n"));
        //    useAdvCgShaders = false;
        //}
        //else
        if (!driver->queryFeature(video::EVDF_MULTIPLE_RENDER_TARGETS))
        {
            dprintf(printf("MRT is not supported by the driver.\n"));
            useAdvCgShaders = false;
        }
        else
        {
            if (driverType == video::EDT_DIRECT3D9 && !cgServices->isProfileSupported("ps_3_0"))
            {
		        dprintf(printf("3.0 shaders are not supported by this D3D9 driver.\n"));
                useAdvCgShaders = false;
            }
            //else
            //if (driverType == video::EDT_OPENGL && !cgServices->isProfileSupported("") // !cgServices->isProfileSupported("fp40")
            //{
		    //    dprintf(printf("3.0 shaders are not supported by this OpenGL driver.\n"));
            //    useAdvCgShaders = false;
            //}
        }
        if (!useAdvCgShaders)
        {
    		dprintf(printf("3.0 shaders are not supported disable it.\n"));
        }
        else
        {
            cg_dir = "data/shaders/cg_mrt/";
        	d3d_vs_version = "vs_3_0";
        	d3d_ps_version = "ps_3_0";
            useScreenRTT = depth_effect = true;
            setupShaders3(device, driver, gpu, plnode);
            return;
        }
    }
    

	light_tex_fileName = cg_dir + "light_tex.cg";
	light_tex_s_fileName = cg_dir + "light_tex_s.cg";
	light_tex_s_car_fileName = cg_dir + "light_tex_s_car.cg";
	light_tex_s_car_tyre_fileName = cg_dir + "light_tex_s_car_tyre.cg";
	light_notex_fileName = cg_dir + "light_notex.cg";
	light_notex_car_fileName = cg_dir + "light_notex_car.cg";
	smoke_fileName = cg_dir + "smoke.cg";
	light_2tex_2_fileName = cg_dir + "light_2tex_2.cg";
	ocean_fileName = cg_dir + "ocean.cg";
	ocean_fix_fileName = cg_dir + "ocean_fix.cg";
	transp_fileName = cg_dir + "transp_obj.cg";
	road_fileName = cg_dir + "road.cg";
	transp_road_fileName = cg_dir + "transp_road.cg";
	transp_stat_fileName = cg_dir + "transp_stat.cg";
	sky_fileName = cg_dir + "sky.cg";
	shadow_fileName = cg_dir + "shadow.cg";
	depth_fileName = cg_dir + "depth.cg";
	depthSun_fileName = cg_dir + "depth_sun.cg";
	screenRTT_fileName = cg_dir + "screen_rtt.cg";
	palca_fileName = cg_dir + "palca.cg";

	MyShaderCallBack2_light_2tex_2* mc_light_2tex_2 = new MyShaderCallBack2_light_2tex_2(device, driver, plnode);

	MyShaderCallBack2_ocean* mc_ocean = new MyShaderCallBack2_ocean(device, driver, plnode);

	MyShaderCallBack2_light_tex* mc_light_tex = new MyShaderCallBack2_light_tex(device, driver, plnode);
	MyShaderCallBack2_light_notex* mc_light_notex = new MyShaderCallBack2_light_notex(device, driver, plnode);
	MyShaderCallBack2_light_tex_s* mc_light_tex_s = new MyShaderCallBack2_light_tex_s(device, driver, plnode);
	MyShaderCallBack2_light_tex_s_car* mc_light_tex_s_car = new MyShaderCallBack2_light_tex_s_car(device, driver, plnode);
	MyShaderCallBack2_light_tex_s_car_tyre* mc_light_tex_s_car_tyre = new MyShaderCallBack2_light_tex_s_car_tyre(device, driver, plnode);
	
	MyShaderCallBack2_tr_light* mc_tr_light = new MyShaderCallBack2_tr_light(device, driver, plnode);
	MyShaderCallBack2_transp* mc_tr = new MyShaderCallBack2_transp(device, driver, plnode);
	MyShaderCallBack2_sky* mc_sky = new MyShaderCallBack2_sky(device, driver, plnode);
	MyShaderCallBack2_shadow* mc_shadow = new MyShaderCallBack2_shadow(device, driver, plnode);
	MyShaderCallBack2_screenRTT* mc_screenRTT = new MyShaderCallBack2_screenRTT(device, driver, plnode);
	MyShaderCallBack2_depth* mc_depth = new MyShaderCallBack2_depth(device, driver, plnode);
	MyShaderCallBack2_depthSun* mc_depthSun = new MyShaderCallBack2_depthSun(device, driver, plnode);
	MyShaderCallBack2_palca* mc_palca = new MyShaderCallBack2_palca(device, driver, plnode);
	MyShaderCallBack2_road* mc_road = new MyShaderCallBack2_road(device, driver, plnode);

	// create the shaders depending on if the user wanted high level
	// or low level shaders:

		// create material from high level shaders (hlsl or glsl)

    dprintf(printf("reading shader file: %s\n", light_tex_fileName.c_str());)
	myMaterialType_light_tex = gpu->addCgShaderMaterialFromFiles(CG_SOURCE,
		light_tex_fileName.c_str(), "main_v", ogl_vs_version, d3d_vs_version,
		light_tex_fileName.c_str(), "main_f", ogl_ps_version, d3d_ps_version,
		mc_light_tex, /*video::EMT_SOLID*/video::EMT_TRANSPARENT_ALPHA_CHANNEL);
	//printf("myMaterialType_light_tex %u\n", myMaterialType_light_tex );

    dprintf(printf("reading shader file: %s\n", light_tex_s_fileName.c_str());)
	myMaterialType_light_tex_s = gpu->addCgShaderMaterialFromFiles(CG_SOURCE,
		light_tex_s_fileName.c_str(), "main_v", ogl_vs_version, d3d_vs_version,
		light_tex_s_fileName.c_str(), "main_f", ogl_ps_version, d3d_ps_version,
		mc_light_tex_s, /*video::EMT_SOLID*/video::EMT_TRANSPARENT_ALPHA_CHANNEL);

    dprintf(printf("reading shader file: %s\n", light_tex_s_car_fileName.c_str());)
	myMaterialType_light_tex_s_car = gpu->addCgShaderMaterialFromFiles(CG_SOURCE,
		light_tex_s_car_fileName.c_str(), "main_v", ogl_vs_version, d3d_vs_version,
		light_tex_s_car_fileName.c_str(), "main_f", ogl_ps_version, d3d_ps_version,
		mc_light_tex_s_car, video::EMT_SOLID);

    dprintf(printf("reading shader file: %s\n", light_tex_s_car_tyre_fileName.c_str());)
	myMaterialType_light_tex_s_car_tyre = gpu->addCgShaderMaterialFromFiles(CG_SOURCE,
		light_tex_s_car_tyre_fileName.c_str(), "main_v", ogl_vs_version, d3d_vs_version,
		light_tex_s_car_tyre_fileName.c_str(), "main_f", ogl_ps_version, d3d_ps_version,
		mc_light_tex_s_car_tyre, video::EMT_SOLID);

    dprintf(printf("reading shader file: %s\n", light_notex_fileName.c_str());)
	myMaterialType_light_notex = gpu->addCgShaderMaterialFromFiles(CG_SOURCE,
		light_notex_fileName.c_str(), "main_v", ogl_vs_version, d3d_vs_version,
		light_notex_fileName.c_str(), "main_f", ogl_ps_version, d3d_ps_version,
		mc_light_notex, /*video::EMT_SOLID*/video::EMT_TRANSPARENT_ALPHA_CHANNEL);

    dprintf(printf("reading shader file: %s\n", light_notex_car_fileName.c_str());)
	myMaterialType_light_notex_car = gpu->addCgShaderMaterialFromFiles(CG_SOURCE,
		light_notex_car_fileName.c_str(), "main_v", ogl_vs_version, d3d_vs_version,
		light_notex_car_fileName.c_str(), "main_f", ogl_ps_version, d3d_ps_version,
		mc_light_notex, video::EMT_SOLID/*video::EMT_TRANSPARENT_ALPHA_CHANNEL*/);

    dprintf(printf("reading shader file: %s\n", light_2tex_2_fileName.c_str());)
	myMaterialType_light_2tex_2 = gpu->addCgShaderMaterialFromFiles(CG_SOURCE,
		light_2tex_2_fileName.c_str(), "main_v", ogl_vs_version, d3d_vs_version,
		light_2tex_2_fileName.c_str(), "main_f", ogl_ps_version, d3d_ps_version,
		mc_light_2tex_2, video::EMT_SOLID/*video::EMT_TRANSPARENT_ALPHA_CHANNEL*/);

    dprintf(printf("reading shader file: %s\n", ocean_fileName.c_str());)
	myMaterialType_ocean = gpu->addCgShaderMaterialFromFiles(CG_SOURCE,
		ocean_fileName.c_str(), "main_v", ogl_vs_version, d3d_vs_version,
		ocean_fileName.c_str(), "main_f", ogl_ps_version, d3d_ps_version,
		mc_ocean, /*video::EMT_SOLID*/video::EMT_TRANSPARENT_ALPHA_CHANNEL);

    dprintf(printf("reading shader file: %s\n", ocean_fix_fileName.c_str());)
	myMaterialType_ocean_fix = gpu->addCgShaderMaterialFromFiles(CG_SOURCE,
		ocean_fix_fileName.c_str(), "main_v", ogl_vs_version, d3d_vs_version,
		ocean_fix_fileName.c_str(), "main_f", ogl_ps_version, d3d_ps_version,
		mc_ocean, /*video::EMT_SOLID*/video::EMT_TRANSPARENT_ALPHA_CHANNEL);

    dprintf(printf("reading shader file: %s\n", smoke_fileName.c_str());)
	myMaterialType_smoke = gpu->addCgShaderMaterialFromFiles(CG_SOURCE,
		smoke_fileName.c_str(), "main_v", ogl_vs_version, d3d_vs_version,
		//0, 0, 0, 0,
		smoke_fileName.c_str(), "main_f", ogl_ps_version, d3d_ps_version,
		mc_tr_light, video::EMT_TRANSPARENT_ALPHA_CHANNEL);

    dprintf(printf("reading shader file: %s\n", transp_fileName.c_str());)
	myMaterialType_transp = gpu->addCgShaderMaterialFromFiles(CG_SOURCE,
		transp_fileName.c_str(), "main_v", ogl_vs_version, d3d_vs_version,
		//0, 0, 0, 0,
		transp_fileName.c_str(), "main_f", ogl_ps_version, d3d_ps_version,
		mc_tr, video::EMT_TRANSPARENT_ALPHA_CHANNEL);

    dprintf(printf("reading shader file: %s\n", road_fileName.c_str());)
	myMaterialType_road = gpu->addCgShaderMaterialFromFiles(CG_SOURCE,
		road_fileName.c_str(), "main_v", ogl_vs_version, d3d_vs_version,
		//0, 0, 0, 0,
		road_fileName.c_str(), "main_f", ogl_ps_version, d3d_ps_version,
		mc_road, video::EMT_SOLID);

    dprintf(printf("reading shader file: %s\n", transp_road_fileName.c_str());)
	myMaterialType_transp_road = gpu->addCgShaderMaterialFromFiles(CG_SOURCE,
		transp_road_fileName.c_str(), "main_v", ogl_vs_version, d3d_vs_version,
		//0, 0, 0, 0,
		transp_road_fileName.c_str(), "main_f", ogl_ps_version, d3d_ps_version,
		mc_tr, video::EMT_TRANSPARENT_ALPHA_CHANNEL/*video::EMT_SOLID*/);

    dprintf(printf("reading shader file: %s\n", transp_stat_fileName.c_str());)
	myMaterialType_transp_stat = gpu->addCgShaderMaterialFromFiles(CG_SOURCE,
		transp_stat_fileName.c_str(), "main_v", ogl_vs_version, d3d_vs_version,
		//0, 0, 0, 0,
		transp_stat_fileName.c_str(), "main_f", ogl_ps_version, d3d_ps_version,
		mc_tr, video::EMT_TRANSPARENT_ALPHA_CHANNEL);

    dprintf(printf("reading shader file: %s\n", sky_fileName.c_str());)
	myMaterialType_sky = gpu->addCgShaderMaterialFromFiles(CG_SOURCE,
		sky_fileName.c_str(), "main_v", ogl_vs_version, d3d_vs_version,
		//0, 0, 0, 0,
		sky_fileName.c_str(), "main_f", ogl_ps_version, d3d_ps_version,
		mc_sky, video::EMT_SOLID);

    dprintf(printf("reading shader file: %s\n", shadow_fileName.c_str());)
	myMaterialType_shadow = gpu->addCgShaderMaterialFromFiles(CG_SOURCE,
		shadow_fileName.c_str(), "main_v", ogl_vs_version, d3d_vs_version,
		//0, 0, 0, 0,
		shadow_fileName.c_str(), "main_f", ogl_ps_version, d3d_ps_version,
		mc_shadow, video::EMT_SOLID);

    dprintf(printf("reading shader file: %s\n", screenRTT_fileName.c_str());)
	myMaterialType_screenRTT = gpu->addCgShaderMaterialFromFiles(CG_SOURCE,
		screenRTT_fileName.c_str(), "main_v", ogl_vs_version, d3d_vs_version,
		screenRTT_fileName.c_str(), "main_f", ogl_ps_version, d3d_ps_version,
		mc_screenRTT, video::EMT_SOLID);

    dprintf(printf("reading shader file: %s\n", depth_fileName.c_str());)
	myMaterialType_depth = gpu->addCgShaderMaterialFromFiles(CG_SOURCE,
		depth_fileName.c_str(), "main_v", ogl_vs_version, d3d_vs_version,
		depth_fileName.c_str(), "main_f", ogl_ps_version, d3d_ps_version,
		mc_depth, video::EMT_SOLID);

    dprintf(printf("reading shader file: %s\n", depthSun_fileName.c_str());)
	myMaterialType_depthSun = gpu->addCgShaderMaterialFromFiles(CG_SOURCE,
		depthSun_fileName.c_str(), "main_v", ogl_vs_version, d3d_vs_version,
		depthSun_fileName.c_str(), "main_f", ogl_ps_version, d3d_ps_version,
		mc_depthSun, video::EMT_SOLID);

    dprintf(printf("reading shader file: %s\n", palca_fileName.c_str());)
	myMaterialType_palca = gpu->addCgShaderMaterialFromFiles(CG_SOURCE,
		palca_fileName.c_str(), "main_v", ogl_vs_version, d3d_vs_version,
		palca_fileName.c_str(), "main_f", ogl_ps_version, d3d_ps_version,
		mc_palca, video::EMT_TRANSPARENT_ALPHA_CHANNEL);


//		mc->drop();
/*
		delete mc_light_2tex;
		delete mc_light_2tex_2;
		delete mc_light_tex;
		delete mc_light_notex;
		delete mc_light_tex_s;
		delete mc_light_tex_s_car;
		delete mc_light_tex_s_car_tyre;
		delete mc_tr_light;
		delete mc_tr;
		delete mc_tex;
		delete mc_shadow;
		delete mc_screenRTT;
		delete mc_depth;
*/
#endif /* DISABLE_CG_SHADERS */
}

void deleteCgShaders(IVideoDriver* driver)
{
#ifndef DISABLE_CG_SHADERS 
    if (gpu)
    {
        printf("release material renderes from %d to %d\n", myMaterialType_light_tex, myMaterialType_depth);
        for (int i = myMaterialType_light_tex; i < myMaterialType_depth; i++)
        {
            delete driver->getMaterialRenderer(i);
        }

        delete gpu;
        gpu = 0;
    }
#endif /* DISABLE_CG_SHADERS */
}

void recreateRTTs(IVideoDriver* driver)
{
    if (!driver->queryFeature(video::EVDF_RENDER_TO_TARGET)/* || !useCgShaders*/
    	// we are run out of d3d9 instructions, so turn off the post effects with ps 2.0
       )
    {
        printf("Render to texture is not supported: automatically turn off RTT and shadows\n");
        useScreenRTT = false;
        depth_effect = false;
        if (!driver->queryFeature(video::EVDF_RENDER_TO_TARGET))
        {
            shadows = false;
        }
        return;
    }

    u32 atiRes = 256;
    
    for (int i = 0; i < MAX_SCREENRTT; i++)
    {
        if (screenRTT[i])
        {
            driver->removeTexture(screenRTT[i]);
            screenRTT[i] = 0;
        }
    }
    currentScreenRTT = 0;
    if (depthRTT)
    {
        driver->removeTexture(depthRTT);
        depthRTT = 0;
    }
    
    // TODO: how can i remove each IRenderTarget? Now we only clear the list.
    mrtList.clear();
    
    /*
    if (shadowMap)
    {
        driver->removeTexture(shadowMap);
        shadowMap = shadowMapGame = 0;
    }
    
    if (shadowMapCar)
    {
        driver->removeTexture(shadowMapCar);
        shadowMapCar = 0;
    }
    */
    if (shitATI)
    {
        while (atiRes < screenSize.Width) atiRes *= 2;
        dprintf(printf("Resolution for ATI: %u\n", atiRes));
    }

    if (useScreenRTT)
    {
    	bool tempTexFlagMipMaps = driver->getTextureCreationFlag(ETCF_CREATE_MIP_MAPS);
	    bool tempTexFlag32 = driver->getTextureCreationFlag(ETCF_ALWAYS_32_BIT);
#if 0
	    for (int i = 0; i<MAX_SCREENRTT; i++)
	    {
            screenRTT[i] = driver->addRenderTargetTexture(shitATI ? dimension2d<u32>(atiRes, atiRes) : screenSize);
        }
#endif
        assert(MAX_SCREENRTT==3);

        screenRTT[0] = driver->addRenderTargetTexture(shitATI ? dimension2d<u32>(atiRes, atiRes) : screenSize);
        if (screenSize.Width > 1280)
        {
            screenRTT[1] = driver->addRenderTargetTexture(dimension2d<u32>(512, 512));
        }
        else
        {
            screenRTT[1] = driver->addRenderTargetTexture(dimension2d<u32>(256, 256));
        }
        screenRTT[2] = driver->addRenderTargetTexture(shitATI ? dimension2d<u32>(atiRes, atiRes) : screenSize);

        if (useAdvCgShaders)
            depthRTT = driver->addRenderTargetTexture(shitATI ? dimension2d<u32>(atiRes, atiRes) : screenSize);
        else
            depthRTT = driver->addRenderTargetTexture(dimension2d<u32>(512, 512));

	    driver->setTextureCreationFlag(ETCF_CREATE_MIP_MAPS, tempTexFlagMipMaps);
	    driver->setTextureCreationFlag(ETCF_ALWAYS_32_BIT, tempTexFlag32);
	    
        if (useAdvCgShaders)
        {
            mrtList.push_back(video::IRenderTarget(screenRTT[0]));
            mrtList.push_back(video::IRenderTarget(depthRTT));
            mrtList.push_back(video::IRenderTarget(screenRTT[2]));
        }
	    
	    depth_effect = true;
    }
    else
    {
        for (int i = 0; i < MAX_SCREENRTT; i++) screenRTT[i] = 0;
        depthRTT = 0;
	    depth_effect = false;
    }

    if (!useScreenRTT || !depth_effect)
    {
        for (int i = 0; i<depthNodes.size();i++)
        {
            depthNodes[i]->setVisible(true);
        }
    }
    /*
    if (shadows)
    {
        shadowMap = shadowMapGame = shadows ? driver->addRenderTargetTexture(core::dimension2d<u32>(shadow_map_size,shadow_map_size), "RTT1")
                             : 0;
        shadowMapCar = shadows ? driver->addRenderTargetTexture(core::dimension2d<u32>(shadow_map_size,shadow_map_size), "RTT1")
                                 : 0;
        shadowMapMenu = driver->getTexture("data/menu_textures/menu_shadow.png");
    }
    else
    {
        shadowMap = shadowMapGame = shadowMapMenu = 0;
    }
    */
}
