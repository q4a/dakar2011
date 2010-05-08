/****************************************************************
*                                                               *
*    Name: error.h                                              *
*                                                               *
*    Creator: Balazs Tuska                                      *
*                                                               *
*    Description:                                               *
*       This file contains some error handling functions.       *
*       Messages towards the user.                              *
*                                                               *
****************************************************************/

#ifndef __TERRAINPOOL_H__
#define __TERRAINPOOL_H__

#include "irrlicht.h"
// Irrlicht Namespaces
using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

#include "MyList.h"

#define MAX_LOD 5

#define VSCALE 1.5f

#define TERRAIN_SCALE 20.f // tScale
#define SMALLTERRAIN_HEIGHTMAP_SIZE 128 //stHeightmapSize
#define SMALLTERRAIN_SIZE ((float)SMALLTERRAIN_HEIGHTMAP_SIZE*TERRAIN_SCALE)

#define TESSELATION terrain_tesselation
#define SMALLTERRAIN_HEIGHTMAP_SIZE_T (SMALLTERRAIN_HEIGHTMAP_SIZE*TESSELATION)
#define TERRAIN_SCALE_T (TERRAIN_SCALE/(float)TESSELATION)


class TerrainPool
{
public:
    TerrainPool(int numOfTerrains, ISceneManager* smgr, IVideoDriver* driver);
    ~TerrainPool();
    
    scene::ITerrainSceneNode* getTerrain();
    void putTerrain(scene::ITerrainSceneNode* terrain);
    
private:
    CMyList<scene::ITerrainSceneNode*> terrains;
};

#endif // __TERRAINPOOL_H__
