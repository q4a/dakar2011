/****************************************************************
*                                                               *
*    Name: pools.h                                              *
*                                                               *
*    Creator: Balazs Tuska                                      *
*                                                               *
*    Description:                                               *
*       This file contains the pool of the objects, grass and   *
*       the trees.                                              *
*                                                               *
****************************************************************/

#ifndef __pools_h__
#define __pools_h__

#include "irrlicht.h"
// Irrlicht Namespaces
using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

#include <Newton.h>

enum PoolObjectType
{
    NORMAL = 0,
    GRASS,
    TREE,
    MY_TREE,
    NONE,
};

enum GrassType
{
    GRASS_GENERATED = 0,
    GRASS_BILLBOARD,
    GRASS_OBJECT,
    GRASS_BILLBOARD_GROUP
};

#define GENERATED_GRASS_SIZE 10
#define ITINER_POOLID_OFFSET 1000

int createObjectPool(const c8* name,
                    ISceneManager* smgr, IVideoDriver* driver, NewtonWorld* nWorld,
                    int num, PoolObjectType type, // 0 - obj, 1 - grass, 3 - tree
                    const c8* textureName,
                    const vector3df& rot,
                    const vector3df& sca,
                    const vector3df& box,
                    const vector3df& ofs,
                    int category,
                    bool textureWrap = false
);

void generateElementsToPool(ISceneManager* smgr, IVideoDriver* driver, NewtonWorld* nWorld,
                            int poolId, int num, PoolObjectType type,
                            const c8* textureName,
                            const vector3df& rot,
                            const vector3df& sca,
                            const vector3df& box,
                            const vector3df& ofs,
                            bool textureWrap = false
);

//void generateTreeTypes(const c8* treetype,
//                       ISceneManager* smgr,
//                       IVideoDriver* driver,
//                       IrrlichtDevice* device);

void putPoolElement(int poolId, void* arg);

void* getPoolElement(int poolId, const vector3df& pos);

void printPoolStat();

void releasePools();

void loadObjectTypes(const c8* name, ISceneManager* smgr, IVideoDriver* driver, NewtonWorld* nWorld);
void loadGrassTypes(const c8* name, ISceneManager* smgr, IVideoDriver* driver, NewtonWorld* nWorld);
void loadTreeTypes(const c8* treetype,
                   ISceneManager* smgr,
                   IVideoDriver* driver,
                   IrrlichtDevice* device,
                   NewtonWorld* nWorld);
void loadMyTreeTypes(const c8* treetype,
                   ISceneManager* smgr,
                   IVideoDriver* driver,
                   IrrlichtDevice* device,
                   NewtonWorld* nWorld);
void loadItinerTypes(const c8* name, ISceneManager* smgr, IVideoDriver* driver, NewtonWorld* nWorld);

int getPoolIdFromName(const char* name);
const char* getPoolNameFromId(int ind);
const int getPoolCategoryFromId(int ind);
int getPoolsSize();

int getItinerIdFromName(const char* name);
const char* getItinerNameFromId(int ind);
int getItinerTypesSize();
video::ITexture* getItinerTextureFromId(int ind);
void releaseItinerTypes();

SAnimatedMesh* readMySimpleObject(const char* name);

#ifdef USE_MESH_COMBINER
void resetCombinedObjects();
void finishCombinedObjects(ISceneManager* smgr);
#endif

#endif // __pools_h__

