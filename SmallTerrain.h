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

#define TERRAIN_SCALE tScale
#define SMALLTERRAIN_HEIGHTMAP_SIZE stHeightmapSize
#define SMALLTERRAIN_SIZE ((float)SMALLTERRAIN_HEIGHTMAP_SIZE*TERRAIN_SCALE)
//3840.f
//#define SMALLTERRAIN_SIZE 3360.f
//#define SMALLTERRAIN_SIZE 7200.f
#define TERRAIN_TEXTURE_NUM 6

#include "wrappers.h"

class BigTerrain;
class CObjectWire;
class CMyRoad;

struct SObjectPoolIdRepPair
{
    SObjectPoolIdRepPair(int p_poolId, int p_rep) :  poolId(p_poolId), rep(p_rep) {}
    int poolId;
    int rep;
};

class SmallTerrain
{
public:
       SmallTerrain(video::IImage* heightMap,
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
                    core::array<CMyRoad*> &bigRoadList);
       ~SmallTerrain();
       
       void setActive(bool pvisible);

       void addRoad_old(SRoadWrapper_old* roadWrapper_ord);
       //void addObject(SObjectWrapper* objectWrapper);
       //void addGrass(SObjectWrapper* grassWrapper);
       
       void updatePos(float newX, float newY, float limit, bool force);
       void updateRoads(core::array<CMyRoad*> &bigRoadList);
       
       /*static */IAnimatedMeshSceneNode* loadMySimpleObject(const char* name);

//private:
       void activate(video::IImage* heightMap);
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
       /*
       c8 heightmapName[256];
       c8 terrainTextureName[256];
       c8 detailmapName[256];
       c8 terrainTexture0Name[256];
       c8 terrainTexture1Name[256];
       c8 terrainTexture2Name[256];
       c8 terrainTexture3Name[256];
       */
       
       int stHeightmapSize;
       float tScale;

       //unsigned int numOfRoads;
       //IAnimatedMesh** roadMeshes;
       //ISceneNode** roadNodes;
       core::array<SRoadWrapper_old*> roadWrappers_old;

       //unsigned int numOfObjects;
       //IAnimatedMesh** objectMeshes;
       //IAnimatedMeshSceneNode** objectNodes;
       core::array<SObjectWrapper*> objectWrappers;

       //unsigned int numOfGrasses;
       //core::array<SObjectWrapper*> grassWrappers;
       //ISceneNode** grassNodes;
       
       bool wasCalculated;
       
       //c8 terrainPartTextureName[256];
       //c8 terrainPartTexture0Name[256];
       int x;
       int y;
       int max_x;
       int max_y;
       c8 heightMapPartName[256];
       BigTerrain* m_bigTerrain;
       CObjectWire* objectWire;
       
       core::array<CMyRoad*> roadList;
       
       // ocean
       IAnimatedMeshSceneNode* oceanNode;
};

#endif // __SMALLTERRAIN_H__

