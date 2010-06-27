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
s32 myMaterialType_light_tex_wfar = video::EMT_SOLID;
s32 myMaterialType_light_notex = video::EMT_SOLID;
s32 myMaterialType_light_notex_wfar = video::EMT_SOLID;
s32 myMaterialType_light_notex_car = video::EMT_SOLID;
s32 myMaterialType_light_2tex = video::EMT_SOLID;
s32 myMaterialType_light_2tex_2 = /*video::EMT_SOLID*/video::EMT_DETAIL_MAP;
s32 myMaterialType_ocean = video::EMT_SOLID;
s32 myMaterialType_ocean_fix = video::EMT_SOLID;
s32 myMaterialType_smoke = video::EMT_TRANSPARENT_ALPHA_CHANNEL;
s32 myMaterialType_light_tex_s = video::EMT_SOLID;
s32 myMaterialType_light_tex_s_car = video::EMT_SOLID;
s32 myMaterialType_light_tex_s_car_tyre = video::EMT_SOLID;
s32 myMaterialType_transp = video::EMT_TRANSPARENT_ALPHA_CHANNEL;
s32 myMaterialType_transp_road = video::EMT_TRANSPARENT_ALPHA_CHANNEL;
s32 myMaterialType_transp_stat = video::EMT_TRANSPARENT_ALPHA_CHANNEL;
s32 myMaterialType_tex = video::EMT_SOLID;
s32 myMaterialType_shadow = video::EMT_SOLID;
s32 myMaterialType_depth = video::EMT_SOLID;
s32 myMaterialType_depthSun = video::EMT_SOLID;
s32 myMaterialType_screenRTT = video::EMT_SOLID;

// shadow map
video::ITexture* shadowMap = 0;
video::ITexture* shadowMapMenu = 0;
video::ITexture* shadowMapGame = 0;
video::ITexture* shadowMapCar = 0;
float renderToShadowMap = 0.f;
core::matrix4 projMat;
core::matrix4 viewMat;
float max_shadow = 0.f;

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
                if(driverType == video::EDT_OPENGL) \
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

class MyShaderCallBack2_light_2tex : public ICgShaderConstantSetCallBack
{
public:
    CGparameter	WorldViewProjection, InvWorld, TraWorld, World,
                lightColor, lightPosition,
                eyePosition, shininess,
                tex0, tex1,
                day_multi,
                clightPos, clightDir, clight,
                eyePositionF,
                pshadowMap, pshadowParam, pshadowRes,
                ptextureMatrix
                ;
public:
    MyShaderCallBack2_light_2tex(IrrlichtDevice* pdevice,
                     video::IVideoDriver* pdriver,
                     scene::ILightSceneNode* plnode) :
        device(pdevice), driver(pdriver), m_lnode(plnode)
    {
    }

#ifdef IRR_CG_8
    virtual void OnSetConstants(ICgServices* services,const CGeffect& Effect,const CGpass& Pass,const CGprogram& Vertex,const CGprogram& Pixel,const irr::video::SMaterial& Material)
#else
	virtual void OnSetConstants(ICgServices* services,CGprogram Vertex,CGprogram Pixel,const SMaterial& Material)
#endif
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

#ifdef IRR_CG_8
    virtual void OnSetConstants(ICgServices* services,const CGeffect& Effect,const CGpass& Pass,const CGprogram& Vertex,const CGprogram& Pixel,const irr::video::SMaterial& Material)
#else
	virtual void OnSetConstants(ICgServices* services,CGprogram Vertex,CGprogram Pixel,const SMaterial& Material)
#endif
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

#ifdef IRR_CG_8
    virtual void OnSetConstants(ICgServices* services,const CGeffect& Effect,const CGpass& Pass,const CGprogram& Vertex,const CGprogram& Pixel,const irr::video::SMaterial& Material)
#else
	virtual void OnSetConstants(ICgServices* services,CGprogram Vertex,CGprogram Pixel,const SMaterial& Material)
#endif
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

#ifdef IRR_CG_8
    virtual void OnSetConstants(ICgServices* services,const CGeffect& Effect,const CGpass& Pass,const CGprogram& Vertex,const CGprogram& Pixel,const irr::video::SMaterial& Material)
#else
	virtual void OnSetConstants(ICgServices* services,CGprogram Vertex,CGprogram Pixel,const SMaterial& Material)
#endif
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

#ifdef IRR_CG_8
    virtual void OnSetConstants(ICgServices* services,const CGeffect& Effect,const CGpass& Pass,const CGprogram& Vertex,const CGprogram& Pixel,const irr::video::SMaterial& Material)
#else
	virtual void OnSetConstants(ICgServices* services,CGprogram Vertex,CGprogram Pixel,const SMaterial& Material)
#endif
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

#ifdef IRR_CG_8
    virtual void OnSetConstants(ICgServices* services,const CGeffect& Effect,const CGpass& Pass,const CGprogram& Vertex,const CGprogram& Pixel,const irr::video::SMaterial& Material)
#else
	virtual void OnSetConstants(ICgServices* services,CGprogram Vertex,CGprogram Pixel,const SMaterial& Material)
#endif
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

#ifdef IRR_CG_8
    virtual void OnSetConstants(ICgServices* services,const CGeffect& Effect,const CGpass& Pass,const CGprogram& Vertex,const CGprogram& Pixel,const irr::video::SMaterial& Material)
#else
	virtual void OnSetConstants(ICgServices* services,CGprogram Vertex,CGprogram Pixel,const SMaterial& Material)
#endif
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

#ifdef IRR_CG_8
    virtual void OnSetConstants(ICgServices* services,const CGeffect& Effect,const CGpass& Pass,const CGprogram& Vertex,const CGprogram& Pixel,const irr::video::SMaterial& Material)
#else
	virtual void OnSetConstants(ICgServices* services,CGprogram Vertex,CGprogram Pixel,const SMaterial& Material)
#endif
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
    CGparameter	WorldViewProjection, tex0, tex1, day_multi, prtsm;
public:
    MyShaderCallBack2_tr_light(IrrlichtDevice* pdevice,
                     video::IVideoDriver* pdriver,
                     scene::ILightSceneNode* plnode) :
        device(pdevice), driver(pdriver), m_lnode(plnode) {}

#ifdef IRR_CG_8
    virtual void OnSetConstants(ICgServices* services,const CGeffect& Effect,const CGpass& Pass,const CGprogram& Vertex,const CGprogram& Pixel,const irr::video::SMaterial& Material)
#else
	virtual void OnSetConstants(ICgServices* services,CGprogram Vertex,CGprogram Pixel,const SMaterial& Material)
#endif
	{
	    // Vertex Shader
	    ADD_WORLD_VIEW_PROJ_V

	    //ADD_RENDER_TO_SHADOW_MAP_V

        // Pixel Shader
        ADD_TEXTURE0

        ADD_DAY_MULTI
        
        //ADD_RENDER_TO_SHADOW_MAP_F
	}

public:
	IrrlichtDevice* device;
	scene::ILightSceneNode* m_lnode;
    video::IVideoDriver* driver;
	const video::SMaterial *UsedMaterial;
};

// part of scene: sky
class MyShaderCallBack2_tex : public ICgShaderConstantSetCallBack
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
    MyShaderCallBack2_tex(IrrlichtDevice* pdevice,
                     video::IVideoDriver* pdriver,
                     scene::ILightSceneNode* plnode) :
        device(pdevice), driver(pdriver), m_lnode(plnode) {}

#ifdef IRR_CG_8
    virtual void OnSetConstants(ICgServices* services,const CGeffect& Effect,const CGpass& Pass,const CGprogram& Vertex,const CGprogram& Pixel,const irr::video::SMaterial& Material)
#else
	virtual void OnSetConstants(ICgServices* services,CGprogram Vertex,CGprogram Pixel,const SMaterial& Material)
#endif
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

// part of scene: grass, road
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

#ifdef IRR_CG_8
    virtual void OnSetConstants(ICgServices* services,const CGeffect& Effect,const CGpass& Pass,const CGprogram& Vertex,const CGprogram& Pixel,const irr::video::SMaterial& Material)
#else
	virtual void OnSetConstants(ICgServices* services,CGprogram Vertex,CGprogram Pixel,const SMaterial& Material)
#endif
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

#ifdef IRR_CG_8
    virtual void OnSetConstants(ICgServices* services,const CGeffect& Effect,const CGpass& Pass,const CGprogram& Vertex,const CGprogram& Pixel,const irr::video::SMaterial& Material)
#else
	virtual void OnSetConstants(ICgServices* services,CGprogram Vertex,CGprogram Pixel,const SMaterial& Material)
#endif
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

#ifdef IRR_CG_8
    virtual void OnSetConstants(ICgServices* services,const CGeffect& Effect,const CGpass& Pass,const CGprogram& Vertex,const CGprogram& Pixel,const irr::video::SMaterial& Material)
#else
	virtual void OnSetConstants(ICgServices* services,CGprogram Vertex,CGprogram Pixel,const SMaterial& Material)
#endif
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

#ifdef IRR_CG_8
    virtual void OnSetConstants(ICgServices* services,const CGeffect& Effect,const CGpass& Pass,const CGprogram& Vertex,const CGprogram& Pixel,const irr::video::SMaterial& Material)
#else
	virtual void OnSetConstants(ICgServices* services,CGprogram Vertex,CGprogram Pixel,const SMaterial& Material)
#endif
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

#ifdef IRR_CG_8
    virtual void OnSetConstants(ICgServices* services,const CGeffect& Effect,const CGpass& Pass,const CGprogram& Vertex,const CGprogram& Pixel,const irr::video::SMaterial& Material)
#else
	virtual void OnSetConstants(ICgServices* services,CGprogram Vertex,CGprogram Pixel,const SMaterial& Material)
#endif
	{
        /*
        if (Material.getTexture(0))
        {
            tex0 = cgGetNamedParameter(Pixel, "tex0");
            if (tex0)
                services->EnableTexture(tex0, Material.getTexture(0));
        }
        if (Material.getTexture(1))
        {
            tex1 = cgGetNamedParameter(Pixel, "tex1");
            if (tex1)
                services->EnableTexture(tex1, Material.getTexture(1));
        }
        if (Material.getTexture(2))
        {
            tex2 = cgGetNamedParameter(Pixel, "tex2");
            if (tex2)
                services->EnableTexture(tex2, Material.getTexture(2));
        }
        printf("s %p\n", Material.getTexture(2));
        */
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

// ----------------------------- USED SHADER END --------------------------------------

#endif /* DISABLE_CG_SHADERS */
void setupShaders2 (IrrlichtDevice* device,
                   IVideoDriver* driver,
                   video::E_DRIVER_TYPE driverType,
                   ISceneManager* smgr,
                   scene::ICameraSceneNode* camera,
                   bool usehls,
                   scene::ILightSceneNode* plnode
                   )
{
#ifndef DISABLE_CG_SHADERS 

//#ifdef IRRLICHT_SDK_15
	const c8* light_tex_vsFileName; // filename for the vertex shader
	const c8* light_tex_psFileName; // filename for the pixel shader
	const c8* light_tex_s_fileName; // filename for the vertex shader
	const c8* light_tex_s_car_fileName; // filename for the vertex shader
	const c8* light_tex_s_car_tyre_fileName; // filename for the vertex shader

	const c8* light_tex_wfar_vsFileName; // filename for the vertex shader
	const c8* light_tex_wfar_psFileName; // filename for the pixel shader

	const c8* light_notex_vsFileName; // filename for the vertex shader
	const c8* light_notex_psFileName; // filename for the pixel shader

	const c8* light_notex_wfar_vsFileName; // filename for the vertex shader
	const c8* light_notex_wfar_psFileName; // filename for the pixel shader

	const c8* light_notex_car_vsFileName; // filename for the vertex shader
	const c8* light_notex_car_psFileName; // filename for the pixel shader

	const c8* light_2tex_vsFileName; // filename for the vertex shader
	const c8* light_2tex_psFileName; // filename for the pixel shader

	const c8* light_2tex_2_vsFileName; // filename for the vertex shader
	const c8* light_2tex_2_psFileName; // filename for the pixel shader

	const c8* ocean_vsFileName; // filename for the vertex shader
	const c8* ocean_psFileName; // filename for the pixel shader
	const c8* ocean_fix_vsFileName; // filename for the vertex shader
	const c8* ocean_fix_psFileName; // filename for the pixel shader

    const c8* smoke_fileName; // filename for the pixel shader
    const c8* transp_fileName;
    const c8* transp_road_fileName;
    const c8* transp_stat_fileName;
    const c8* light_transp_fileName;
    const c8* tex_fileName;
    const c8* shadow_fileName;
    const c8* depth_fileName;
    const c8* depthSun_fileName;
    const c8* screenRTT_fileName;
    const c8* vs_version;
    const c8* ps_version;
/*
#define NULLSTRING 0    
#else
	core::string<c16> light_tex_vsFileName; // filename for the vertex shader
	core::string<c16> light_tex_psFileName; // filename for the pixel shader

	core::string<c16> light_notex_vsFileName; // filename for the vertex shader
	core::string<c16> light_notex_psFileName; // filename for the pixel shader

	core::string<c16> light_2tex_vsFileName; // filename for the vertex shader
	core::string<c16> light_2tex_psFileName; // filename for the pixel shader

    core::string<c16> smoke_psFileName; // filename for the pixel shader
#define NULLSTRING ""
#endif
*/
    dprintf(printf("T&L: %d\n", driver->queryFeature(video::EVDF_HARDWARE_TL)));
    dprintf(printf("Multitexturing: %d\n", driver->queryFeature(video::EVDF_MULTITEXTURE)));
    dprintf(printf("Stencil buffer: %d\n", driver->queryFeature(video::EVDF_STENCIL_BUFFER)));
    dprintf(printf("Vertex shader 1.1: %d\n", driver->queryFeature(video::EVDF_VERTEX_SHADER_1_1)));
    dprintf(printf("Vertex shader 2.0: %d\n", driver->queryFeature(video::EVDF_VERTEX_SHADER_2_0)));
    dprintf(printf("Vertex shader 3.0: %d\n", driver->queryFeature(video::EVDF_VERTEX_SHADER_3_0)));
    dprintf(printf("Pixel shader 1.1: %d\n", driver->queryFeature(video::EVDF_PIXEL_SHADER_1_1)));
    dprintf(printf("Pixel shader 1.2: %d\n", driver->queryFeature(video::EVDF_PIXEL_SHADER_1_2)));
    dprintf(printf("Pixel shader 1.3: %d\n", driver->queryFeature(video::EVDF_PIXEL_SHADER_1_3)));
    dprintf(printf("Pixel shader 1.4: %d\n", driver->queryFeature(video::EVDF_PIXEL_SHADER_1_4)));
    dprintf(printf("Pixel shader 2.0: %d\n", driver->queryFeature(video::EVDF_PIXEL_SHADER_2_0)));
    dprintf(printf("Pixel shader 3.0: %d\n", driver->queryFeature(video::EVDF_PIXEL_SHADER_3_0)));
    dprintf(printf("ARB vertex program: %d\n", driver->queryFeature(video::EVDF_ARB_VERTEX_PROGRAM_1)));
    dprintf(printf("ARB fragment program: %d\n", driver->queryFeature(video::EVDF_ARB_FRAGMENT_PROGRAM_1)));
    dprintf(printf("GLSL: %d\n", driver->queryFeature(video::EVDF_ARB_GLSL)));
    dprintf(printf("HLSL: %d\n", driver->queryFeature(video::EVDF_HLSL)));

	if (((driverType == video::EDT_DIRECT3D9 && driver->queryFeature(video::EVDF_HLSL)) ||
		 (driverType == video::EDT_OPENGL && driver->queryFeature(video::EVDF_ARB_GLSL))) &&
         (driver->queryFeature(video::EVDF_PIXEL_SHADER_2_0) || driver->queryFeature(video::EVDF_ARB_FRAGMENT_PROGRAM_1)) && 
         (driver->queryFeature(video::EVDF_VERTEX_SHADER_2_0) || driver->queryFeature(video::EVDF_ARB_VERTEX_PROGRAM_1))  
         && usehls)
	{
		dprintf(printf("use high level cg shaders.\n"));
	}
	else
	{
		dprintf(printf("Not use high level shaders, because of missing driver/hardware support.\n"));
        useShaders = useCgShaders = false;
        return;
    }
    
	light_tex_psFileName = "data/shaders/cg/light_tex.cg";
	light_tex_vsFileName = light_tex_psFileName; // both shaders are in the same file
	light_tex_s_fileName = "data/shaders/cg/light_tex_s.cg";
	light_tex_s_car_fileName = "data/shaders/cg/light_tex_s_car.cg";
	light_tex_s_car_tyre_fileName = "data/shaders/cg/light_tex_s_car_tyre.cg";
	light_tex_wfar_psFileName = "data/shaders/cg/light_tex_wfar.cg";
	light_tex_wfar_vsFileName = light_tex_psFileName; // both shaders are in the same file
	light_notex_psFileName = "data/shaders/cg/light_notex.cg";
	light_notex_vsFileName = light_notex_psFileName;
	light_notex_wfar_psFileName = "data/shaders/cg/light_notex_wfar.cg";
	light_notex_wfar_vsFileName = light_notex_psFileName;
	light_notex_car_psFileName = "data/shaders/cg/light_notex_car.cg";
	light_notex_car_vsFileName = light_notex_psFileName;
	smoke_fileName = "data/shaders/cg/smoke.cg";
	light_2tex_psFileName = "data/shaders/cg/light_2tex.cg";
	light_2tex_vsFileName = light_2tex_psFileName;
	light_2tex_2_psFileName = "data/shaders/cg/light_2tex_2.cg";
	light_2tex_2_vsFileName = light_2tex_2_psFileName;
	ocean_psFileName = "data/shaders/cg/ocean.cg";
	ocean_vsFileName = ocean_psFileName;
	ocean_fix_psFileName = "data/shaders/cg/ocean_fix.cg";
	ocean_fix_vsFileName = ocean_fix_psFileName;
	transp_fileName = "data/shaders/cg/transp_obj.cg";
	transp_road_fileName = "data/shaders/cg/transp_road.cg";
	transp_stat_fileName = "data/shaders/cg/transp_stat.cg";
	light_transp_fileName = "data/shaders/cg/light_transp.cg";
	tex_fileName = "data/shaders/cg/tex.cg";
	shadow_fileName = "data/shaders/cg/shadow.cg";
	depth_fileName = "data/shaders/cg/depth.cg";
	depthSun_fileName = "data/shaders/cg/depth_sun.cg";
	screenRTT_fileName = "data/shaders/cg/screen_rtt.cg";
	vs_version = "vs_2_0";
	ps_version = "ps_2_0";

	if (!driver->queryFeature(video::EVDF_PIXEL_SHADER_2_0) &&
		!driver->queryFeature(video::EVDF_ARB_FRAGMENT_PROGRAM_1))
	{
		dprintf(printf("WARNING: Pixel shaders disabled "\
			"because of missing driver/hardware support.\n"));
        useShaders = useCgShaders = useAdvCgShaders = false;
	}
	
	if (!driver->queryFeature(video::EVDF_VERTEX_SHADER_2_0) &&
		!driver->queryFeature(video::EVDF_ARB_VERTEX_PROGRAM_1))
	{
		dprintf(printf("WARNING: Vertex shaders disabled "\
			"because of missing driver/hardware support.\n"));
        useShaders = useCgShaders = useAdvCgShaders = false;
	}
	
	if (0 && useAdvCgShaders)
	{
        if (driver->queryFeature(video::EVDF_VERTEX_SHADER_3_0) &&
            driver->queryFeature(video::EVDF_PIXEL_SHADER_3_0))
        {
            vs_version = "vs_3_0";
            ps_version = "ps_3_0";

    	    light_2tex_2_psFileName = "data/shaders/cg/light_2tex_2_adv.cg";
            light_2tex_2_vsFileName = light_2tex_2_psFileName;
            myMessage(12, "Your system does support Vertex or Pixel shader 3.0");
        }
        else
        {
            myMessage(12, "Your system does not support Vertex or Pixel shader 3.0, use 2.0");
            useAdvCgShaders = false;
        }
    }

    if (driver->queryFeature(video::EVDF_RENDER_TO_TARGET))
    {
        dprintf(printf("shadow map is supported\n"));

        bool tempTexFlagMipMaps = driver->getTextureCreationFlag(ETCF_CREATE_MIP_MAPS);
        bool tempTexFlag32 = driver->getTextureCreationFlag(ETCF_ALWAYS_32_BIT);

//        driver->setTextureCreationFlag(ETCF_CREATE_MIP_MAPS, false);
//        driver->setTextureCreationFlag(ETCF_ALWAYS_32_BIT, tempTexFlag32);

#ifdef IRRLICHT_SDK_15
        shadowMap = shadowMapGame = shadows ? driver->addRenderTargetTexture(core::dimension2d<s32>(shadow_map_size,shadow_map_size), "RTT1")
                             : 0;
        shadowMapCar = shadows ? driver->addRenderTargetTexture(core::dimension2d<s32>(shadow_map_size,shadow_map_size), "RTT1")
                             : 0;
#else
        shadowMap = shadowMapGame = shadows ? driver->addRenderTargetTexture(core::dimension2d<u32>(shadow_map_size,shadow_map_size), "RTT1")
                             : 0;
        shadowMapCar = shadows ? driver->addRenderTargetTexture(core::dimension2d<u32>(shadow_map_size,shadow_map_size), "RTT1")
                             : 0;
#endif                             
        dprintf(printf("shadow has mip-maps: %s\n", (shadowMap && shadowMap->hasMipMaps())?"yes":"no"));

//        driver->setTextureCreationFlag(ETCF_CREATE_MIP_MAPS, false);
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

    if (!useShaders) return;
    dprintf(printf("create new gpu\n"));
    gpu = new ICgProgrammingServices(device/*, false, true*/);
    dprintf(printf("create new gpu end\n"));

	if (gpu)
	{
		MyShaderCallBack2_light_2tex* mc_light_2tex = new MyShaderCallBack2_light_2tex(device, driver, plnode);

		MyShaderCallBack2_light_2tex_2* mc_light_2tex_2 = new MyShaderCallBack2_light_2tex_2(device, driver, plnode);

		MyShaderCallBack2_ocean* mc_ocean = new MyShaderCallBack2_ocean(device, driver, plnode);

		MyShaderCallBack2_light_tex* mc_light_tex = new MyShaderCallBack2_light_tex(device, driver, plnode);
		MyShaderCallBack2_light_notex* mc_light_notex = new MyShaderCallBack2_light_notex(device, driver, plnode);
		MyShaderCallBack2_light_tex_s* mc_light_tex_s = new MyShaderCallBack2_light_tex_s(device, driver, plnode);
		MyShaderCallBack2_light_tex_s_car* mc_light_tex_s_car = new MyShaderCallBack2_light_tex_s_car(device, driver, plnode);
		MyShaderCallBack2_light_tex_s_car_tyre* mc_light_tex_s_car_tyre = new MyShaderCallBack2_light_tex_s_car_tyre(device, driver, plnode);
		
		MyShaderCallBack2_tr_light* mc_tr_light = new MyShaderCallBack2_tr_light(device, driver, plnode);
		MyShaderCallBack2_transp* mc_tr = new MyShaderCallBack2_transp(device, driver, plnode);
		MyShaderCallBack2_tex* mc_tex = new MyShaderCallBack2_tex(device, driver, plnode);
		MyShaderCallBack2_shadow* mc_shadow = new MyShaderCallBack2_shadow(device, driver, plnode);
		MyShaderCallBack2_screenRTT* mc_screenRTT = new MyShaderCallBack2_screenRTT(device, driver, plnode);
		MyShaderCallBack2_depth* mc_depth = new MyShaderCallBack2_depth(device, driver, plnode);
		MyShaderCallBack2_depthSun* mc_depthSun = new MyShaderCallBack2_depthSun(device, driver, plnode);

		// create the shaders depending on if the user wanted high level
		// or low level shaders:

			// create material from high level shaders (hlsl or glsl)

		myMaterialType_light_tex = gpu->addCgShaderMaterialFromFiles(CG_SOURCE,
			light_tex_vsFileName, "main_v", "arbvp1", vs_version,
			light_tex_psFileName, "main_f", "arbfp1", ps_version,
			mc_light_tex, /*video::EMT_SOLID*/video::EMT_TRANSPARENT_ALPHA_CHANNEL);
//		printf("myMaterialType_light_tex %u\n", myMaterialType_light_tex );

		myMaterialType_light_tex_wfar = gpu->addCgShaderMaterialFromFiles(CG_SOURCE,
			light_tex_wfar_vsFileName, "main_v", "arbvp1", vs_version,
			light_tex_wfar_psFileName, "main_f", "arbfp1", ps_version,
			mc_light_tex, video::EMT_SOLID/*video::EMT_TRANSPARENT_ALPHA_CHANNEL*/);

		myMaterialType_light_tex_s = gpu->addCgShaderMaterialFromFiles(CG_SOURCE,
			light_tex_s_fileName, "main_v", "arbvp1", vs_version,
			light_tex_s_fileName, "main_f", "arbfp1", ps_version,
			mc_light_tex_s, video::EMT_SOLID);

		myMaterialType_light_tex_s_car = gpu->addCgShaderMaterialFromFiles(CG_SOURCE,
			light_tex_s_car_fileName, "main_v", "arbvp1", vs_version,
			light_tex_s_car_fileName, "main_f", "arbfp1", ps_version,
			mc_light_tex_s_car, video::EMT_SOLID);

		myMaterialType_light_tex_s_car_tyre = gpu->addCgShaderMaterialFromFiles(CG_SOURCE,
			light_tex_s_car_tyre_fileName, "main_v", "arbvp1", vs_version,
			light_tex_s_car_tyre_fileName, "main_f", "arbfp1", ps_version,
			mc_light_tex_s_car_tyre, video::EMT_SOLID);

		myMaterialType_light_notex = gpu->addCgShaderMaterialFromFiles(CG_SOURCE,
			light_notex_vsFileName, "main_v", "arbvp1", vs_version,
			light_notex_psFileName, "main_f", "arbfp1", ps_version,
			mc_light_notex, /*video::EMT_SOLID*/video::EMT_TRANSPARENT_ALPHA_CHANNEL);

		myMaterialType_light_notex_wfar = gpu->addCgShaderMaterialFromFiles(CG_SOURCE,
			light_notex_wfar_vsFileName, "main_v", "arbvp1", vs_version,
			light_notex_wfar_psFileName, "main_f", "arbfp1", ps_version,
			mc_light_notex, video::EMT_SOLID/*video::EMT_TRANSPARENT_ALPHA_CHANNEL*/);

		myMaterialType_light_notex_car = gpu->addCgShaderMaterialFromFiles(CG_SOURCE,
			light_notex_car_vsFileName, "main_v", "arbvp1", vs_version,
			light_notex_car_psFileName, "main_f", "arbfp1", ps_version,
			mc_light_notex, video::EMT_SOLID/*video::EMT_TRANSPARENT_ALPHA_CHANNEL*/);

		myMaterialType_light_2tex = gpu->addCgShaderMaterialFromFiles(CG_SOURCE,
			light_2tex_vsFileName, "main_v", "arbvp1", vs_version,
			light_2tex_psFileName, "main_f", "arbfp1", ps_version,
			mc_light_2tex, video::EMT_SOLID/*video::EMT_TRANSPARENT_ALPHA_CHANNEL*/);

		myMaterialType_light_2tex_2 = gpu->addCgShaderMaterialFromFiles(CG_SOURCE,
			light_2tex_2_vsFileName, "main_v", "arbvp1", vs_version,
			light_2tex_2_psFileName, "main_f", "arbfp1", ps_version,
			mc_light_2tex_2, video::EMT_SOLID/*video::EMT_TRANSPARENT_ALPHA_CHANNEL*/);

		myMaterialType_ocean = gpu->addCgShaderMaterialFromFiles(CG_SOURCE,
			ocean_vsFileName, "main_v", "arbvp1", vs_version,
			ocean_psFileName, "main_f", "arbfp1", ps_version,
			mc_ocean, /*video::EMT_SOLID*/video::EMT_TRANSPARENT_ALPHA_CHANNEL);

		myMaterialType_ocean_fix = gpu->addCgShaderMaterialFromFiles(CG_SOURCE,
			ocean_fix_vsFileName, "main_v", "arbvp1", vs_version,
			ocean_fix_psFileName, "main_f", "arbfp1", ps_version,
			mc_ocean, /*video::EMT_SOLID*/video::EMT_TRANSPARENT_ALPHA_CHANNEL);

		myMaterialType_smoke = gpu->addCgShaderMaterialFromFiles(CG_SOURCE,
			smoke_fileName, "main_v", "arbvp1", vs_version,
			//0, 0, 0, 0,
			smoke_fileName, "main_f", "arbfp1", ps_version,
			mc_tr_light, video::EMT_TRANSPARENT_ALPHA_CHANNEL);

		myMaterialType_transp = gpu->addCgShaderMaterialFromFiles(CG_SOURCE,
			transp_fileName, "main_v", "arbvp1", vs_version,
			//0, 0, 0, 0,
			transp_fileName, "main_f", "arbfp1", ps_version,
			mc_tr, video::EMT_TRANSPARENT_ALPHA_CHANNEL);

		myMaterialType_transp_road = gpu->addCgShaderMaterialFromFiles(CG_SOURCE,
			transp_road_fileName, "main_v", "arbvp1", vs_version,
			//0, 0, 0, 0,
			transp_road_fileName, "main_f", "arbfp1", ps_version,
			mc_tr, video::EMT_TRANSPARENT_ALPHA_CHANNEL);

		myMaterialType_transp_stat = gpu->addCgShaderMaterialFromFiles(CG_SOURCE,
			transp_stat_fileName, "main_v", "arbvp1", vs_version,
			//0, 0, 0, 0,
			transp_stat_fileName, "main_f", "arbfp1", ps_version,
			mc_tr, video::EMT_TRANSPARENT_ALPHA_CHANNEL);

		myMaterialType_tex = gpu->addCgShaderMaterialFromFiles(CG_SOURCE,
			tex_fileName, "main_v", "arbvp1", vs_version,
			//0, 0, 0, 0,
			tex_fileName, "main_f", "arbfp1", ps_version,
			mc_tex, video::EMT_SOLID);

		myMaterialType_shadow = gpu->addCgShaderMaterialFromFiles(CG_SOURCE,
			shadow_fileName, "main_v", "arbvp1", vs_version,
			//0, 0, 0, 0,
			shadow_fileName, "main_f", "arbfp1", ps_version,
			mc_shadow, video::EMT_SOLID);

		myMaterialType_screenRTT = gpu->addCgShaderMaterialFromFiles(CG_SOURCE,
			screenRTT_fileName, "main_v", "arbvp1", /*"vs_2_0"*/vs_version,
			screenRTT_fileName, "main_f", "arbfp1", /*"ps_2_0"*/ps_version,
			mc_screenRTT, video::EMT_SOLID);

		myMaterialType_depth = gpu->addCgShaderMaterialFromFiles(CG_SOURCE,
			depth_fileName, "main_v", "arbvp1", vs_version,
			depth_fileName, "main_f", "arbfp1", ps_version,
			mc_depth, video::EMT_SOLID);

		myMaterialType_depthSun = gpu->addCgShaderMaterialFromFiles(CG_SOURCE,
			depthSun_fileName, "main_v", "arbvp1", vs_version,
			depthSun_fileName, "main_f", "arbfp1", ps_version,
			mc_depthSun, video::EMT_SOLID);

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
	}
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
    if (!driver->queryFeature(video::EVDF_RENDER_TO_TARGET)/* || !useCgShaders*/)
    {
        printf("Render to texture is not supported: automatically turn off RTT and shadows\n");
        useScreenRTT = false;
        shadows = false;
        depth_effect = false;
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
    
    if (shitATI)
    {
        while (atiRes < screenSize.Width) atiRes *= 2;
        dprintf(printf("Resolution for ATI: %u\n", atiRes));
    }
    if (useScreenRTT)
    {
    	bool tempTexFlagMipMaps = driver->getTextureCreationFlag(ETCF_CREATE_MIP_MAPS);
	    bool tempTexFlag32 = driver->getTextureCreationFlag(ETCF_ALWAYS_32_BIT);
	    for (int i = 0; i<MAX_SCREENRTT; i++)
	    {
            screenRTT[i] = driver->addRenderTargetTexture(
              //!driver->getVendorInfo().equals_ignore_case("NVIDIA Corporation") ? dimension2du(512, 512) :
#ifdef IRRLICHT_SDK_15
              shitATI ? dimension2d<s32>(atiRes, atiRes) : screenSize);
#else
              shitATI ? dimension2d<u32>(atiRes, atiRes) : screenSize);
#endif
        }
	    driver->setTextureCreationFlag(ETCF_CREATE_MIP_MAPS, tempTexFlagMipMaps);
	    driver->setTextureCreationFlag(ETCF_ALWAYS_32_BIT, tempTexFlag32);
	    depth_effect = true;
    }
    else
    {
        for (int i = 0; i < MAX_SCREENRTT; i++) screenRTT[i] = 0;
	    depth_effect = false;
    }

    if (depth_effect)
    {
    	bool tempTexFlagMipMaps = driver->getTextureCreationFlag(ETCF_CREATE_MIP_MAPS);
	    bool tempTexFlag32 = driver->getTextureCreationFlag(ETCF_ALWAYS_32_BIT);
        depthRTT = driver->addRenderTargetTexture(
              //!driver->getVendorInfo().equals_ignore_case("NVIDIA Corporation") ? dimension2du(512, 512) :
#ifdef IRRLICHT_SDK_15
              /*shitATI ? dimension2d<s32>(atiRes, atiRes) : screenSize*/dimension2d<s32>(512, 512));
#else
              /*shitATI ? dimension2d<u32>(atiRes, atiRes) : screenSize*/dimension2d<u32>(512, 512));
#endif
	    driver->setTextureCreationFlag(ETCF_CREATE_MIP_MAPS, tempTexFlagMipMaps);
	    driver->setTextureCreationFlag(ETCF_ALWAYS_32_BIT, tempTexFlag32);
    }
    else
        depthRTT = 0;
    if (!useScreenRTT || !depth_effect)
    {
        for (int i = 0; i<depthNodes.size();i++)
        {
            depthNodes[i]->setVisible(true);
        }
    }
}
