/****************************************************************
*                                                               *
*    Name: BigTerrain.h                                         *
*                                                               *
*    Creator: Balazs Tuska                                      *
*                                                               *
*    Description:                                               *
*       This file contains the big terrain. The big terrain     *
*       contains max_x*max_y small terrains. We will display    *
*       only terrains that we are on or next to.                *
*                                                               *
****************************************************************/

#ifndef __BIGTERRAIN_H__
#define __BIGTERRAIN_H__


#include "irrlicht.h"
// Irrlicht Namespaces
using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

#include <Newton.h>

#include "SmallTerrain.h"
#include "CObjectWire.h"
#include "MyLock.h"
#include "MyList.h"
#include "TerrainPool.h"

//#define CP_NUM 6

class SState;
class CMapReaderThread;
class CMyRoad;

struct SMapsQueueElement
{
    int x;
    int y;
    int obj_density;
    bool visible;
};

class BigTerrain
{
public:
       BigTerrain(const c8* name, IrrlichtDevice* p_device, ISceneManager* p_smgr,
                  IVideoDriver* p_driver, NewtonWorld* p_nWorld, u32 pstageTime, u32 pgtime,
                  scene::ISceneNode* p_skydome, video::ITexture* p_shadowMap, int p_stageNum,
                  TerrainPool* p_terrainPool);
       ~BigTerrain();

       core::vector3df updatePos(float newX, float newY, int obj_density, bool force, bool showPerc = false);
       void updateMaps(int new_x, int new_y, int obj_density, bool showPerc = false);
       void updateMapsAddToQueue(int new_x, int new_y, int obj_density);
       void checkMapsQueue();
       void updateRoads();
       void addNewRoad();
       void loadRoads(const char* roadsName);
       void updateObjectWire();
       void saveObjects(const char* name);
       void addActiveItinerPoint(SItinerPoint* itinerPoint);
       void removeActiveItinerPoint(SItinerPoint* itinerPoint);
       
       float getHeight(float x, float y) const;
       float getDensity(float x, float y, int category = -1) const;
       scene::ISceneNode* getTerrain(float x, float y) const;
       SmallTerrain* getSmallTerrain(float x, float y) const;
       float getSmallTerrainSize() const;
       float getTerrainSize() const;
       void setOVLimit(float new_val);
       float getOVLimit();

       void loadRoads_old(const c8* name, ISceneManager* smgr, IVideoDriver* driver,
                      const vector3df& loc);
       void destroyRoads_old();
       
       void loadObjects(const c8* name, ISceneManager* smgr, IVideoDriver* driver);
       void destroyObjects();
       
       bool getTimeStarted() {return timeStarted;}
       bool getTimeEnded() {return timeEnded;}
       u32 getCurrentTime() {return currentTime;}
       void updateTime(u32 ptick);
       //u32 getStartTime() {return startTime;}
       //u32 getCpTime(int i) {return i<CP_NUM?cpTime[i]:0;}
       //u32 getEndTime() {return endTime;}
       u32 getPenality() {return penality;}
       u32 addPenality(u32 ap);
       static void addTimeToStr(core::stringw& str, u32 diffTime);
       
       vector3df getStartPos() {return vector3df(startPos.X+startOffset.X, getHeight(startPos.X+startOffset.X,startPos.Z+startOffset.Z)+3.f, startPos.Z+startOffset.Z);}
       vector3df getStartRot() {return startRot;}

       c8* getGroundSoundName() {return groundSoundName;}
       c8* getPuffSoundName() {return puffSoundName;}
       c8* getSkidSoundName() {return skidSoundName;}
       float& getFrictionMulti() {return friction_multi;}
       
       void saveState(SState* state);
       void restoreState(SState* state);
       
       core::array<CMyRoad*>& getRoadList() {return roadList;}
       core::array<SObjectWrapper*>& getObjectWrappers() {return objectWrappers;}
       core::array<SAIPoint*>& getAIPoints() {return aiPoints;}
       
       float getSpeed() {return speed;}
       
       u32 getStageTime() {return stageTime;}
       float getStageLength() {return stageLength;}

private:
       void applyRoadOnHeightMap();
       bool isRoad(int x, int y);
       bool isNextToRoad(int px, int py);
       int getAverage(int px, int py);
       void applyAverage(int px, int py, int avgCol);
       int getAverage2(int px, int py);
       void applyAverage2(int px, int py, int avgCol);
       int getAverage3(int px, int py);
       void applyAverage3(int px, int py, int avgCol);
       
       void doCache();
       
       void calculateAIPointTimes();
              
public:
       friend class CMapReaderThread;
       NewtonWorld* nWorld;
       IrrlichtDevice* device;
       ISceneManager* smgr;
       IVideoDriver* driver;
       
       int max_x;     // how many small terrains are here
       int max_y;
       int last_x;    // which small terrain we are on.
       int last_y;
       int ov_last_x; // object visibility last x
       int ov_last_y; // object visibility last y
       
       SmallTerrain** map;

       //unsigned int numOfRoads;
       //IAnimatedMesh** roadMeshes;
       core::array<SRoadWrapper_old*> roadWrappers_old;
       //ISceneNode** roadNodes;
       
       //unsigned int numOfObjects;
       //IAnimatedMesh** objectMeshes;
       //IAnimatedMeshSceneNode** objectNodes;
       core::array<SObjectWrapper*> objectWrappers;

       int stHeightmapSize;
       float tScale;
       float ov_limit;

       //unsigned int numOfGrasses;
       //core::array<SObjectWrapper*> grassWrappers;
       //ISceneNode** grassNodes;
       
       vector3df startPos;
       vector3df startOffset;
       vector3df startRot;
       core::array<vector3df> cpPos;
       vector3df endPos;
       scene::IVolumeLightSceneNode* startGate;
       core::array<scene::IVolumeLightSceneNode*> cpGate;
       scene::IVolumeLightSceneNode* endGate;
       
       bool timeStarted;
       bool timeEnded;
       core::array<u32> cpTime;
       core::array<u32> cpTimed;
       //u32 startTime;
       //u32 endTime;
       u32 currentTime;
       u32 lastTick;
       int cps;
       u32 penality;
       u32 stageTime;
       float stageLength;
       u32 gtime;

       c8 groundSoundName[256];
       c8 puffSoundName[256];
       c8 skidSoundName[256];
       
       float friction_multi;
       video::IImage* densityMap;
       video::IImage* heightMap;
       video::IImage* textureMap;
       video::ITexture* textures[TERRAIN_TEXTURE_NUM];
       video::ITexture* shadowMap;

       CObjectWire* objectWire;
       
       core::array<SObjectPoolIdRepPair> objectReps;
       
       int stageNum;

       core::array<SmallTerrain*> smallTerrainsForUpdate;
       CMyLock smallTerrainsForUpdateLock;
       
       core::array<CMyRoad*> roadList;
       core::array<SItinerPoint*> activeItinerPoints;
       core::array<SAIPoint*> aiPoints;
       float speed;
       CMyList<SMapsQueueElement*> mapsQueue;
       scene::ISceneNode* skydome;
       TerrainPool* m_terrainPool;
/*       
       NewtonBody* bodyl;
       NewtonCollision* collisionl;
       NewtonBody* bodyr;
       NewtonCollision* collisionr;
       NewtonBody* bodyu;
       NewtonCollision* collisionu;
       NewtonBody* bodyd;
       NewtonCollision* collisiond;
*/
};

#endif // __BIGTERRAIN_H__

