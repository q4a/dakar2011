/****************************************************************
*                                                               *
*    Name: SmallTerrains.h                                      *
*                                                               *
*    Creator: Balazs Tuska                                      *
*                                                               *
*    Description:                                               *
*       This file contains the real terrain code. The map       *
*       consist of many small terrain to be fast.               *
*                                                               *
****************************************************************/

#ifndef __SMALLTERRAIN_H__
#define __SMALLTERRAIN_H__

#include "irrlicht.h"
// Irrlicht Namespaces
using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

#include <Newton.h>

//3840.f
//#define SMALLTERRAIN_SIZE 3360.f
//#define SMALLTERRAIN_SIZE 7200.f
#define TERRAIN_TEXTURE_NUM 5

#include "wrappers.h"
#include "TerrainPool.h"

class BigTerrain;
class CObjectWire;
class CMyRoad;
class OffsetObject;

struct SObjectPoolIdRepPair
{
    SObjectPoolIdRepPair(int p_poolId, int p_rep) :  poolId(p_poolId), rep(p_rep) {}
    int poolId;
    int rep;
};

class SmallTerrain
{
public:
       SmallTerrain(
#ifdef USE_IMAGE_HM
                    video::IImage* heightMap,
#else
                    CHeightmap* heightMap,
#endif
                    video::IImage* densityMap,
                    video::IImage* textureMap,
                    video::ITexture** textures,
                    video::ITexture* shadowMap,
                    ISceneManager* psmgr, IVideoDriver* pdriver, NewtonWorld* pnWorld,
                    scene::ISceneNode* skydome,
                    const int px, const int py,
                    const int pmax_x, const int pmax_y,
                    int pstHeightmapSize, float ptScale,
                    int stageNum,
                    BigTerrain* p_bigTerrain,
                    core::array<SObjectPoolIdRepPair> &objectReps, int obj_density,
                    core::array<CMyRoad*> &bigRoadList,
                    float p_vscale,
                    float p_waterHeight);
       ~SmallTerrain();
       
       void setActive(bool pvisible);
       
       void updatePos(float newX, float newY, float limit, bool force);
       void updateRoads(core::array<CMyRoad*> &bigRoadList, unsigned int regenerate, video::ITexture* p_shadowMap, int roadToUpdate = -1);
       
       /*static */IAnimatedMeshSceneNode* loadMySimpleObject(const char* name);

//private:
#ifdef USE_IMAGE_HM
       void activate(video::IImage* heightMap);
#else
       void activate(CHeightmap* heightMap);
#endif
       void deactivate();

public:
       scene::ITerrainSceneNode* terrain;
       
private:
       NewtonBody* body;
       NewtonWorld* nWorld;
       NewtonCollision* collision;
       ISceneManager* smgr;
       IVideoDriver* driver;
       bool visible;
       
       int stHeightmapSize;
       float tScale;

       core::array<SObjectWrapper*> objectWrappers;

       bool wasCalculated;
       
       int x;
       int y;
       int max_x;
       int max_y;
       BigTerrain* m_bigTerrain;
       CObjectWire* objectWire;
       
       core::array<CMyRoad*> roadList;
       
       // ocean
       IAnimatedMeshSceneNode* oceanNode;
       ITexture* partTexture;
       float vscale;
       OffsetObject* offsetObject;
       OffsetObject* offsetObjectOcean;
};

#endif // __SMALLTERRAIN_H__

