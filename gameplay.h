/****************************************************************
*                                                               *
*    Name: gameplay.h                                           *
*                                                               *
*    Creator: Balazs Tuska                                      *
*                                                               *
*    Description:                                               *
*       This file contains the variables and fuctions that are  *
*       connected to the gameplay. Stages (big terrains),       *
*       current stage, view, time, car, load/save game and      *
*       other stuffs connected to the player.                   *
*                                                               *
****************************************************************/

#ifndef __GAMEPLAY_H__
#define __GAMEPLAY_H__

#include "irrlicht.h"
#include "NewtonRaceCar.h"
#include "BigTerrain.h"
#include <Newton.h>
//#include "MyList.h"
#include "CQuad.h"
#include "MapReaderThread.h"
#include "VehiclePool.h"
#include "competitors.h"
#include "RaceEngine.h"

// Irrlicht Namespaces
using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

#ifdef MY_DEBUG
# define dprintf(x) x
#else
# define dprintf(x)
#endif /* debug */

#ifdef MY_PDEBUG
# define pdprintf(x) x
#else
# define pdprintf(x)
#endif /* debug */


//#define COMPETITORS_NUM 136

//#define CAMERA_SPEED 0.4f 0.04f
#define CAMERA_SPEED cameraSpeed
#define DEFAULT_FAR_VALUE farValue
#define FAR_VALUE_MULTI 1.5f

#define view_max 3
#define view_multi 4

#define RESET_PENALITY 120

#define MAX_STAGES 27
class Stages
{
public:
    c8 name[256];
    c8 info[1024];
    u32 stageTime;
    u32 gtime;
    u32 stageNum;
    //u32 stagePart;
};

extern Stages **stages;

extern NewtonRaceCar* car;
extern BigTerrain* bigTerrain;
//extern c8 carName[256];
extern int carType;
extern scene::ICameraSceneNode* camera;
extern scene::ICameraSceneNode* fix_camera;
extern scene::ICameraSceneNode* fps_camera;
extern scene::ICameraSceneNode* car_selector_camera;
extern gui::IGUIStaticText* fpsText;
extern gui::IGUIStaticText* polyText;
extern gui::IGUIStaticText* posText;
extern gui::IGUIStaticText* speedText;
extern gui::IGUIStaticText* demageText;
extern gui::IGUIStaticText* timeText;
extern gui::IGUIStaticText* compassText;
extern gui::IGUIStaticText* versionText;
extern gui::IGUIImage* bgImage;
extern gui::IGUIImage* hudImage;
extern gui::IGUIImage* hudCompassImage;
extern gui::IGUIImage* hudInfo;
extern bool showCompass;
extern scene::IAnimatedMeshSceneNode* compassArrow;
extern scene::ISceneNode* skydome;
extern scene::IBillboardSceneNode* sunSphere;
extern video::ITexture* smokeTexture;
extern bool fpsCam;
extern bool quitGame;
extern int inGame;
extern u32 lasttick;
extern u32 tick;
extern bool reinitialize;

extern int view_num;
extern int view_mask;
extern matrix4 viewdest[view_max*view_multi];
extern matrix4 viewpos[view_max*view_multi];

extern matrix4 viewdest_cur;
extern matrix4 viewpos_cur;

extern bool useDynCam;
extern bool dynCamReset;


#ifdef IRRLICHT_SDK_15
extern core::dimension2d<s32> screenSize;
#else
extern core::dimension2d<u32> screenSize;
#endif
#define MAX_SCREENRTT 3
extern video::ITexture* screenRTT[MAX_SCREENRTT];
extern int currentScreenRTT;
extern video::ITexture* depthRTT;
extern video::ITexture* bgImageTexture;
extern video::ITexture* blurmap;
extern video::ITexture* blurmapSide;
extern video::ITexture* motiondir_map[view_multi];
//extern video::ITexture* motiondir_mapSide;

#define MAX_BGIMAGE 10
extern const char* bgImagesHi[MAX_BGIMAGE+1];
extern const char* bgImagesLo[MAX_BGIMAGE+1];
extern char** bgImages;
extern video::ITexture* bgImagesTextures[MAX_BGIMAGE+1];

#define MAX_HUD 7
extern video::ITexture* hudTextures[MAX_HUD+2];

extern matrix4 cLightDest_loc;
extern matrix4 cLightPos_loc;
extern matrix4 cLightDir;
extern matrix4 cLightPos;
extern matrix4 cLightTar;

extern scene::ILightSceneNode* lnode;
extern scene::ILightSceneNode* lnode_4_shaders;
//extern scene::ILightSceneNode* clnode;
extern ICameraSceneNode* lightCam;
extern ICameraSceneNode* lightCamCar;
extern irr::core::array<irr::scene::ISceneNode*> shadowNodes;
extern irr::core::array<irr::scene::ISceneNode*> depthNodes;
extern irr::core::array<irr::scene::ISceneNode*> objectNodes;

extern int currentStage;
extern int oldStage;
extern u32 globalTime;
//extern float day_delta_multi;

extern int loading;
extern int startNewGame;

extern CMapReaderThread* mapReader;

extern CVehiclePool* vehiclePool;

extern SCompetitor* playerCompetitor;
extern CRaceEngine* raceEngine;

struct SState
{
    core::vector3df carPos;
    core::vector3df carRot;

    u32 currentTime;
    int cps;
    u32 penality;
    bool timeStarted;
    bool timeEnded;
    core::array<u32> cpTime;
    core::array<u32> cpTimed;
    
    s32 day_start_time_offset;
    
    int currentStage;
    unsigned int globalTime;
    int carType;
    int view_num;
    int savedCarDirt;
    int car_dirt_delta;
    bool useDynCam;
};

void loadGameplay(  const c8* name,
                    IrrlichtDevice* pdevice,
                    IVideoDriver* pdriver,
                    ISceneManager* psmgr,
                    IGUIEnvironment* penv,
                    NewtonWorld *pnWorld,
#ifdef USE_MY_SOUNDENGINE
                    CMySoundEngine* psoundEngine
#else
                    irrklang::ISoundEngine* psoundEngine
#endif
);

bool saveGame(const c8* name);
bool loadGame(const c8* name);
bool checkLoadGame(const c8* name);

void startGame(int stageNum, SState* state = 0);
void endGame();
void restartStage();

void pauseGame();
void resumeGame();

void addToShadowNodes(irr::scene::ISceneNode*);
void removeFromShadowNodes(irr::scene::ISceneNode*);

void addToDepthNodes(irr::scene::ISceneNode*);
void removeFromDepthNodes(irr::scene::ISceneNode*);

void addToObjectNodes(irr::scene::ISceneNode*);
void removeFromObjectNodes(irr::scene::ISceneNode*);

void saveState();

bool restoreState();

void releaseGameStuff();

//void calculate_day_delta(u32 tick);

#endif // __GAMEPLAY_H__

