/****************************************************************
*                                                               *
*    Name: my_shaders.h                                         *
*                                                               *
*    Creator: Balazs Tuska                                      *
*                                                               *
*    Description:                                               *
*       This file contains the shader handling.                 *
*                                                               *
****************************************************************/

#ifndef __MY_SHADERS_H__
#define __MY_SHADERS_H__

#include "irrlicht.h"
// Irrlicht Namespaces
using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

// from normal shaders
extern s32 myMaterialType_light_tex;
extern s32 myMaterialType_light_tex_wfar;
extern s32 myMaterialType_light_tex_s;
extern s32 myMaterialType_light_tex_s_car;
extern s32 myMaterialType_light_tex_s_car_tyre;
extern s32 myMaterialType_light_notex;
extern s32 myMaterialType_light_notex_wfar;
extern s32 myMaterialType_light_notex_car;
extern s32 myMaterialType_light_2tex;
extern s32 myMaterialType_smoke;

// from Cg shaders
extern s32 myMaterialType_transp;
extern s32 myMaterialType_transp_road;
extern s32 myMaterialType_transp_stat;
extern s32 myMaterialType_tex;
extern s32 myMaterialType_shadow;
extern s32 myMaterialType_depth;
extern s32 myMaterialType_depthSun;
extern s32 myMaterialType_screenRTT;
extern s32 myMaterialType_light_2tex_2;
extern s32 myMaterialType_ocean;

extern video::ITexture* shadowMap;
extern video::ITexture* shadowMapGame;
extern video::ITexture* shadowMapMenu;
extern video::ITexture* shadowMapCar;
extern float renderToShadowMap;
extern core::matrix4 projMat;
extern core::matrix4 viewMat;
extern float max_shadow;


void setupShaders (IrrlichtDevice* device,
                   IVideoDriver* driver,
                   video::E_DRIVER_TYPE driverType,
                   ISceneManager* smgr,
                   scene::ICameraSceneNode* camera,
                   bool usehls,
                   scene::ILightSceneNode* lnode);

void setupShaders2 (IrrlichtDevice* device,
                   IVideoDriver* driver,
                   video::E_DRIVER_TYPE driverType,
                   ISceneManager* smgr,
                   scene::ICameraSceneNode* camera,
                   bool usehls,
                   scene::ILightSceneNode* lnode);

void deleteCgShaders(IVideoDriver* driver);

void recreateRTTs(IVideoDriver* driver);

#endif // __MY_SHADERS_H__
