/****************************************************************
*                                                               *
*    Name: gameplay.cpp                                         *
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

#include "irrlicht.h"
// Irrlicht Namespaces
using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

#include "gameplay.h"
#include "settings.h"
#include "message.h"
#include "multiplayer.h"
#include "effects.h"
#include "my_shaders.h"
#include "CQuad.h"
#include "competitors.h"
#include "editor.h"
#include <assert.h>
#include <unistd.h>

#ifdef __linux__
#include "linux_includes.h"
#endif


// ----------------- EXTERNALS ----------------------
//c8 carName[256] = "data/vehicles/car_mitsu.txt";
int carType = 0;

BigTerrain* bigTerrain = 0;
//new BigTerrain("data/bigterrains/bigterrain1.txt", device, smgr, driver, nWorld);
NewtonRaceCar* car = 0;
//new NewtonRaceCar (smgr, driver, carName, nWorld, soundEngine,
//                                        bigTerrain->getStartPos(), bigTerrain->getStartRot());
scene::ICameraSceneNode* camera = 0;
scene::ICameraSceneNode* fix_camera = 0;
OffsetObject* fix_cameraOffsetObject = 0;
scene::ICameraSceneNode* fps_camera = 0;
OffsetObject* fps_cameraOffsetObject = 0;
scene::ICameraSceneNode* car_selector_camera = 0;
gui::IGUIStaticText* fpsText = 0;
gui::IGUIStaticText* polyText = 0;
gui::IGUIStaticText* posText = 0;
gui::IGUIStaticText* speedText = 0;
gui::IGUIStaticText* demageText = 0;
gui::IGUIStaticText* timeText = 0;
gui::IGUIStaticText* versionText = 0;
gui::IGUIImage* bgImage = 0;
gui::IGUIImage* hudImage = 0;
gui::IGUIImage* hudMap = 0;
gui::IGUIImage* hudInfo = 0;
gui::IGUIImage* crossImage = 0;
bool showMap = false;
scene::ISceneNode* skydome = 0;
scene::IBillboardSceneNode* sunSphere = 0;
scene::IBillboardSceneNode* sunSphere1 = 0;
scene::IBillboardSceneNode* sunSphere2 = 0;
scene::IBillboardSceneNode* sunSphere3 = 0;
scene::IBillboardSceneNode* sunSphere4 = 0;
video::ITexture* smokeTexture = 0;
video::ITexture* smokeWaterTexture = 0;
bool fpsCam = false;
bool quitGame = false;
int inGame = 0;
u32 lasttick = 0;
u32 tick = 0;
bool reinitialize = false;

int view_num = 0;
int view_mask = 0;
//matrix4 viewdest[view_max*view_multi];
//matrix4 viewpos[view_max*view_multi];

matrix4 viewdest_cur_helper;
matrix4 viewpos_cur_helper;

matrix4& viewdest_cur = viewdest_cur_helper;
matrix4& viewpos_cur = viewpos_cur_helper;

bool useDynCam = true;
bool dynCamReset = true;

// car light
matrix4 cLightDest_loc;
matrix4 cLightPos_loc;
matrix4 cLightDir;
matrix4 cLightPos;
matrix4 cLightTar;

// global light
scene::ILightSceneNode* lnode = 0;
scene::ILightSceneNode* lnode_4_shaders = 0;
//scene::ILightSceneNode* clnode = 0;
ICameraSceneNode* lightCam = 0;
ICameraSceneNode* lightCamCar = 0;
//irr::core::array<irr::scene::ISceneNode*> shadowNodes;
irr::core::array<irr::scene::ISceneNode*> shadowNodes;
irr::core::array<irr::scene::ISceneNode*> depthNodes;
irr::core::array<irr::scene::ISceneNode*> objectNodes;

core::dimension2d<u32> screenSize;
video::ITexture* screenRTT[MAX_SCREENRTT] = {0, 0, 0};
int currentScreenRTT = 0;
video::ITexture* bgImageTexture = 0;
video::ITexture* depthRTT = 0;
video::ITexture* blurmap = 0;
video::ITexture* blurmapSide = 0;
video::ITexture* motiondir_map[view_multi] = {0, 0, 0, 0};
core::array<video::IRenderTarget> mrtList;
//video::ITexture* motiondir_mapSide = 0;

int loading = 0;
int startNewGame = 1;

CMapReaderThread* mapReader = 0;

CVehiclePool* vehiclePool = 0;

SCompetitor* playerCompetitor = 0;
CRaceEngine* raceEngine = 0;
static CRaceEngine* loadedRaceEngine = 0;

OffsetManager* offsetManager = 0;

float car_pressure_multi = TYRE_PRESSURE_MULTI_DEFAULT;
float car_ss_multi = SUSPENSION_SPRING_MULTI_DEFAULT;
float car_sd_multi = SUSPENSION_DAMPER_MULTI_DEFAULT;
float car_sl_multi = SUSPENSION_LENGTH_MULTI_DEFAULT;

unsigned int newtonUpdateCount = 0;

const char* bgImagesHi[MAX_BGIMAGE+1] =
{
    "data/bg/dakar_bg1_pos1.jpg",
    "data/bg/dakar_bg1_pos2.jpg",
    "data/bg/dakar_bg1_pos3.jpg",
    "data/bg/dakar_bg1_pos4.jpg",
    "data/bg/dakar_bg1_pos5.jpg",
    "data/bg/dakar_bg1_pos6.jpg",
    "data/bg/dakar_bg1_pos7.jpg",
    "data/bg/dakar_bg1_pos8.jpg",
    "data/bg/dakar_bg1_pos9.jpg",
    "data/bg/dakar_bg1_pos10.jpg",
    "data/bg/dakar_bg1_kb.jpg"
};

const char* bgImagesLo[MAX_BGIMAGE+1] =
{
    "data/bg/dakar_bg1_pos1_1280.jpg",
    "data/bg/dakar_bg1_pos2_1280.jpg",
    "data/bg/dakar_bg1_pos3_1280.jpg",
    "data/bg/dakar_bg1_pos4_1280.jpg",
    "data/bg/dakar_bg1_pos5_1280.jpg",
    "data/bg/dakar_bg1_pos6_1280.jpg",
    "data/bg/dakar_bg1_pos7_1280.jpg",
    "data/bg/dakar_bg1_pos8_1280.jpg",
    "data/bg/dakar_bg1_pos9_1280.jpg",
    "data/bg/dakar_bg1_pos10_1280.jpg",
    "data/bg/dakar_bg1_kb_1280.jpg"
};

char** bgImages = (char**)bgImagesLo;

video::ITexture* bgImagesTextures[MAX_BGIMAGE+1] =
{
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0
};

video::ITexture* hudTextures[MAX_HUD+2] = {0,0,0,0,0,0,0,0,0};
//video::ITexture* hudMapTextures[HUD_MAPS] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0};
video::ITexture* hudUserOnMapTexture = 0;

#define SAVE_TMP_FILE "savegames/tmp_state.txt"

// -----------------STATICS --------------------------
static IVideoDriver* driver = 0;
static ISceneManager* smgr = 0;
static IGUIEnvironment* env = 0;
static IrrlichtDevice* device = 0;
static NewtonWorld *nWorld = 0;
#ifdef USE_MY_SOUNDENGINE
static CMySoundEngine* soundEngine = 0;
#else
static irrklang::ISoundEngine* soundEngine = 0;
#endif

static int savedCarDirt = 0;

static SState* savedState = 0;
static void saveStateInternal();
static bool restoreStateInternal(int newStartNewGame);

// ---------------- STRUCTURES -----------------------
Stages **stages;
int currentStage = 0;
int oldStage = 0;
u32 globalTime = 0;

// ---------------- FUNCTIONS ------------------------
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
)
{
    FILE* f;
    int ret;
    
    device = pdevice;
    driver = pdriver;
    smgr = psmgr;
    env = penv;
    nWorld = pnWorld;
    soundEngine = psoundEngine;
    
    stages = new Stages*[MAX_STAGES];
    memset(stages, 0, sizeof(int*)*MAX_STAGES);
    
    f = fopen(name, "r");

    if (!f)
    {
        printf("gameplay file unable to open: %s\n", name);
        return;       
    }
    
    u32 gtime = 0;
    for (int i= 0; i<MAX_STAGES; i++)
    {
        c8 bigterrain_name[256];
        c8 word[256];
        u32 stageTime = 0;
        u32 stageNum = 0;
        //u32 stagePart = 0;
        
        ret = fscanf(f, "%s\n%u\n", bigterrain_name, &stageTime);
        if ( ret < 2 ) break;
        dprintf(printf("%d. %s add to gameplay, time: %u\n", i, bigterrain_name, stageTime));
        stages[i] = new Stages();
        strcpy(stages[i]->name, bigterrain_name);
        memset(stages[i]->info, 0, sizeof(stages[i]->info));
        stages[i]->stageTime = stageTime;
        //stages[i]->stageNum = stageNum;
        //stages[i]->stagePart = stagePart;
        gtime += stageTime;
        stages[i]->gtime = gtime;
        // read info
        ret = fscanf(f, "%s", word);
        //printf("w1 '%s'\n", word);
        while (ret > 0 && strcmp(word, "EOM")!=0 )
        {
            if (strcmp(word, "NL")==0)
                strcat(stages[i]->info, "\n");
            else
            {
                strcat(stages[i]->info, word);
                strcat(stages[i]->info, " ");
            }
            ret = fscanf(f, "%s", word);
            //printf("w2 '%s'\n", word);
        }
    }    
    fclose(f);
//    "data/bigterrains/bigterrain1.txt"
}

void startGame(int stageNum, SState* state)
{
    if (stageNum >= MAX_STAGES || stages[stageNum] == 0) return;

    core::stringw str;

    device->getCursorControl()->setVisible(false);
    
    srand(tick % RAND_MAX);
    /*
    for (unsigned int i = 0; i < 20; i++)
    {
        int ranv = rand();
        printf("%d - %d\n", ranv % 100, ranv);
    }
    printf("tick: %u (%d)\n", tick, RAND_MAX);
    assert(0);
    */
    str = L"Loading: 0%";
    MessageText::addText(str.c_str(), 1, true, false);

    loading = 1;
    if (stageNum < 3)
    {
        if (!bgImagesTextures[MAX_BGIMAGE])
            bgImagesTextures[MAX_BGIMAGE] = driver->getTexture(bgImages[MAX_BGIMAGE]);
        bgImage->setImage(bgImagesTextures[MAX_BGIMAGE]);
    }
    else
    {
        if (!bgImagesTextures[stageNum%MAX_BGIMAGE])
            bgImagesTextures[stageNum%MAX_BGIMAGE] = driver->getTexture(bgImages[stageNum%MAX_BGIMAGE]);
        bgImage->setImage(bgImagesTextures[stageNum%MAX_BGIMAGE]);
    }
    bgImage->setVisible(true);

    if (stageNum == 0 && startNewGame == 1)
    {
        globalTime = 0;
    }
//    else
//        if (!startNewGame && bigTerrain && bigTerrain->getEndTime()) globalTime += bigTerrain->getEndTime() - bigTerrain->getStartTime();
    //printf("startNewGame: %u, bigTerrain: %p, TimeEnded: %u\n", startNewGame, bigTerrain, bigTerrain?bigTerrain->getCurrentTime():0);
    if (startNewGame==0 && bigTerrain && bigTerrain->getTimeEnded())
    {
        globalTime += bigTerrain->getCurrentTime();
    }
    
    endGame();
    
    bigTerrain = new BigTerrain(stages[stageNum]->name, device, smgr, driver, nWorld,
                                stages[stageNum]->stageTime, stages[stageNum]->gtime,
                                skydome, shadowMap, stageNum);

    str = L"Loading: 50%";
    MessageText::addText(str.c_str(), 1, true, false);

    if (/*stages[stageNum]->stagePart <= 1 && */startNewGame != 2)
    {
        savedCarDirt = 0;
        car_dirt = 0.f;
    }

    camera->setPosition(state ? state->carPos : bigTerrain->getStartPos());
    offsetManager->addObject(fix_cameraOffsetObject);
    offsetManager->addObject(fps_cameraOffsetObject);
    offsetManager->update(camera->getPosition(), true);
    //camera->setPosition(camera->getPosition()-offsetManager->getOffset());
    //offsetManager->update(camera->getPosition(), true);
    
    if (state)
    {
        bigTerrain->restoreState(state);
        bigTerrain->updatePos(state->carPos.X, state->carPos.Z, density_objects, true, true);
    }
    else
    {
        // TODO, update somwhere if load
        bigTerrain->updatePos(bigTerrain->getStartPos().X, bigTerrain->getStartPos().Z, density_objects, true, true);
    }
    
    str = L"Loading: 95%";
    MessageText::addText(str.c_str(), 1, true, false);
    
    car = vehiclePool->getVehicle(carType);
    assert(car);
    car->activate(state ? state->carPos : bigTerrain->getStartPos(),
                  state ? state->carRot : bigTerrain->getStartRot(),
                  bigTerrain->getGroundSoundName(),
                  bigTerrain->getPuffSoundName(),
                  bigTerrain->getSkidSoundName(),
                  bigTerrain->getFrictionMulti(),
                  skydome,
                  shadowMap,
                  bigTerrain->getWaterHeight(),
                  true,
                  savedCarDirt,
                  car_pressure_multi,
                  car_ss_multi,
                  car_sd_multi,
                  car_sl_multi
                  );
    car->setAutoGear(gear_type=='a');

    //NewtonUpdate(nWorld, 0.015f);
    str = L"Loading: 99%";
    dynCamReset = true;
    
    playerCompetitor->carIndex = carType;
//#ifndef USE_EDITOR
    if (!state)
    {
        if (stageNum == 0)
        {
            for (int i = 0; i < competitors.size(); i++)
            {
                competitors[i]->lastTime = 0;
                competitors[i]->globalTime = 0;
                competitors[i]->lastPenalityTime = 0;
                competitors[i]->globalPenalityTime = 0;
            }
            CRaceEngine::clearStates();
            CRaceEngine::getRaceState() = competitors;
#ifdef USE_EDITOR
            CRaceEngine::getRaceState().push_front(playerCompetitor);
#else
            CRaceEngine::getRaceState().push_back(playerCompetitor);
#endif
            CRaceEngine::getStageState() = CRaceEngine::getRaceState();
        }
        else
        {
            for (int i = 0; i < competitors.size(); i++)
            {
                competitors[i]->lastTime = 0;
                competitors[i]->lastPenalityTime = 0;
            }
        }
        playerCompetitor->lastTime = 0;
        playerCompetitor->lastPenalityTime = 0;
#ifdef USE_EDITOR
        if (!editorMode)
        {
#endif
            raceEngine = new CRaceEngine(smgr, env, bigTerrain);
            while (raceEngine && raceEngine->update(0, vector3df(), playerCompetitor, device, CRaceEngine::AtStart));
#ifdef USE_EDITOR
        }
#endif
    }
    else
    {
        raceEngine = loadedRaceEngine;
        assert(raceEngine);
        raceEngine->refreshBigTerrain(bigTerrain);
    }
    loadedRaceEngine = 0;
//#endif

    bgImage->setVisible(false);
    versionText->setVisible(false);
#ifndef VIDEO_MODE
    fpsText->setVisible(display_extra_info);
    polyText->setVisible(display_extra_info);
    timeText->setVisible(true);
    speedText->setVisible(true);
    demageText->setVisible(true);
    hudImage->setVisible(draw_hud && !useCgShaders);
    hudInfo->setVisible(info_bg);
#else
    draw_hud = false; // remove
#endif
    showMap = false;
    crossImage->setVisible(false);
#ifdef USE_EDITOR
    posText->setVisible(display_extra_info);
    if (editorMode)
    {
        editorSetVisible(display_extra_info);
    }
#endif
    sunSphere->setVisible(useAdvCgShaders);
    sunSphere1->setVisible(useAdvCgShaders);
    //sunSphere2->setVisible(useAdvCgShaders);
    //sunSphere3->setVisible(useAdvCgShaders);
    //sunSphere4->setVisible(useAdvCgShaders);
    projMat.buildProjectionMatrixPerspectiveFovLH(
        lightCam->getFOV(),
        lightCam->getAspectRatio(),
        lightCam->getNearValue(),
        lightCam->getFarValue()
    );
    
    car_dirt_last_tick = lasttick = device->getTimer()->getTime();
    if (!state)
    {
        day_start_time = lasttick;
    }
    else
    {
        day_start_time = (u32)(lasttick - state->day_start_time_offset);
    }
    //day_delta_time = 0;
    if (/*stages[stageNum]->stagePart <= 1 &&*/ startNewGame != 1)
    {
        car_dirt = 0.f;
        car_dirt_delta = 0;
    }

    oldStage = stageNum;
    startNewGame = 0;

    if (!state)
    {
        str = L"Stage ";
        str += stageNum+1;
        /*
        str += stages[stageNum]->stageNum;
        if (stages[stageNum]->stagePart > 0)
        {
            str += L" part ";
            str += stages[stageNum]->stagePart;
        }
        */
        str += L" started!\n\n";
        str += stages[stageNum]->info;
        
        MessageText::addText(str.c_str(), 15, true);
    }
    loading = 0;
}

void endGame()
{
    printf("delete race engine\n");
    if (raceEngine)
    {
        printf("race engine: enter into update loop\n");
        while (raceEngine->update(0, vector3df(), playerCompetitor, device, CRaceEngine::AtTheEnd));
        printf("race engine: leave update loop\n");
        // todo update global states here
        CRaceEngine::refreshStates(raceEngine);
        delete raceEngine;
        raceEngine = 0;
    }

    printf("delete bigWorld\n");
    if (bigTerrain)
    {
        delete bigTerrain;
        bigTerrain = 0;
    }
    
    printf("delete racecar %p\n", car);
    if (car)
    {
        if (fpsCam)
        {
            fpsCam = !fpsCam;
            camera = fpsCam ? fps_camera : fix_camera;
            smgr->setActiveCamera(camera);
    
            matrix4 campos = car->getMatrix() * viewpos_cur;
            matrix4 camtar = car->getMatrix() * viewdest_cur;
            camera->setPosition(core::vector3df(campos[12],campos[13],campos[14]));
            camera->setTarget(core::vector3df(camtar[12],camtar[13],camtar[14]));
            camera->setFarValue(DEFAULT_FAR_VALUE);
            camera->setNearValue(nearValue);
        }
        if (startNewGame == 0)
            savedCarDirt = car->getDirt();
        vehiclePool->putVehicle(car);
        car = 0;
    }
    printf("delete racecar end\n");
    
    if (nWorld)
        NewtonInvalidateCache(nWorld);

    fpsText->setVisible(false);
    polyText->setVisible(false);
    timeText->setVisible(false);
    speedText->setVisible(false);
    demageText->setVisible(false);
    hudImage->setVisible(false);
    hudInfo->setVisible(false);
    crossImage->setVisible(false);
    showMap = false;
#ifdef USE_EDITOR
    posText->setVisible(false);
    if (editorMode)
    {
        editorSetVisible(false);
    }
#endif
    sunSphere->setVisible(false);
    sunSphere1->setVisible(false);
    sunSphere2->setVisible(false);
    sunSphere3->setVisible(false);
    sunSphere4->setVisible(false);
    
#ifdef USE_MULTIPLAYER
    if (isMultiplayer)
        leaveStageToServer();
#endif // USE_MULTIPLAYER

    dprintf(printf("offset stuff\n");)
    offsetManager->removeObject(fix_cameraOffsetObject);
    offsetManager->removeObject(fps_cameraOffsetObject);
    assert(offsetManager->empty() && "offsetManager is not empty");
    offsetManager->reset();
}

void pauseGame()
{
//    if (car)
//        car->pause();
    if (vehiclePool)
    {
        vehiclePool->pauseActiveVehicles();
    }
    if (fpsCam)
    {
        core::vector3df pos = camera->getPosition();
        core::vector3df tar = camera->getTarget();
        camera = fix_camera;
        smgr->setActiveCamera(camera);
        camera->setPosition(pos);
        camera->setTarget(tar);
        camera->setFarValue(DEFAULT_FAR_VALUE);
        camera->setNearValue(nearValue);
    }
}

void resumeGame()
{
//    if (car)
//        car->resume();
    if (vehiclePool)
    {
        vehiclePool->resumeActiveVehicles();
    }
    if (fpsCam)
    {
        core::vector3df pos = camera->getPosition();
        core::vector3df tar = camera->getTarget();
        camera = fps_camera;
        smgr->setActiveCamera(camera);
        camera->setPosition(pos);
        camera->setTarget(tar);
        camera->setFarValue(DEFAULT_FAR_VALUE);
        camera->setNearValue(nearValue);
    }
    device->getCursorControl()->setVisible(false);
}

bool saveGame(const c8* name)
{
    FILE* f;
    int ret;
    
    if (!car || !bigTerrain)
    {
        printf("There is nothing to save into the save game.\n");
        return true;
    }
    
    f = fopen(name, "w+");

    if (!f)
    {
        printf("savegame file unable to open for save: %s\n", name);
        return false;
    }

    dprintf(printf("save game 1 savedState: %p\n", savedState);)

    saveStateInternal();
    dprintf(printf("save game 2 savedState: %p\n", savedState);)
    
    ret = fprintf(f, "carpos: %f %f %f\ncarrot: %f %f %f\ncurrent_time: %u\ncps: %d\npenality: %u\n" \
                     "started: %u\nended: %u\ncpsize: %u\ntime_offset: %d\n" \
                     "current_stage: %d\nglobal_time: %u\ncar_type: %d\nview_num: %d\n" \
                     "car_dirt: %d\ncar_dirt_delta: %d\nuse_dyn_cam: %d\n" \
                     "car_pressure_multi: %f\ncar_ss_multi: %f\n" \
                     "car_sd_multi: %f\ncar_sl_multi: %f\n",
                offsetManager->getOffset().X+savedState->carPos.X,
                savedState->carPos.Y+1.2f,
                offsetManager->getOffset().Z+savedState->carPos.Z,
                savedState->carRot.X,
                savedState->carRot.Y,
                savedState->carRot.Z,
                savedState->currentTime,
                savedState->cps,
                savedState->penality,
                savedState->timeStarted,
                savedState->timeEnded,
                savedState->cpTime.size(),
                savedState->day_start_time_offset,
                savedState->currentStage,
                savedState->globalTime,
                savedState->carType,
                savedState->view_num,
                savedState->savedCarDirt,
                savedState->car_dirt_delta,
                savedState->useDynCam,
                savedState->car_pressure_multi,
                savedState->car_ss_multi,
                savedState->car_sd_multi,
                savedState->car_sl_multi
            );
    if ( ret < 24 )
    {
        printf("error writing %s ret %d errno %d\n", name, ret, errno);
        fclose(f);
        return false;
    }

    for (int i = 0; i < savedState->cpTime.size(); i++)
    {
        ret = fprintf(f, "cptimes[%d]: %d %d\n",
                i,
                savedState->cpTime[i],
                savedState->cpTimed[i]
            );
        if ( ret < 3 )
        {
            printf("error writing %s ret %d errno %d\n", name, ret, errno);
            fclose(f);
            return false;
        }
    }
    
    if (raceEngine)
    {
        ret = fprintf(f, "race_engine: 1\n");
        if ( ret < 0 )
        {
            printf("error writing %s ret %d errno %d\n", name, ret, errno);
            fclose(f);
            return false;
        }
        if (!raceEngine->save(f))
        {
            printf("error writing %s ret %d errno %d\n", name, ret, errno);
            fclose(f);
            return false;
        }
    }
    else
    {
        ret = fprintf(f, "race_engine: 0\n");
        if ( ret < 0 )
        {
            printf("error writing %s ret %d errno %d\n", name, ret, errno);
            fclose(f);
            return false;
        }
    }
        
    fclose(f);
   
    return true;
    
}
    
bool loadGame(const c8* name)
{
    FILE* f;
    int ret;
    int cpsize = 0;
    int useRaceEngine = 0;
    
    dprintf(printf("load game: %s\n", name);)

    f = fopen(name, "r");

    if (!f)
    {
        printf("savegame file unable to open: %s\n", name);
        return false;
    }

    dprintf(printf("load game savedState: %p\n", savedState);)
    if (savedState!=0)
    {
        delete savedState;
    }
    savedState = new SState;

    dprintf(printf("load game - load state: %p\n", savedState);)
    
    ret = fscanf(f, "carpos: %f %f %f\ncarrot: %f %f %f\ncurrent_time: %u\ncps: %d\npenality: %u\n" \
                     "started: %u\nended: %u\ncpsize: %u\ntime_offset: %d\n" \
                     "current_stage: %d\nglobal_time: %u\ncar_type: %d\nview_num: %d\n" \
                     "car_dirt: %d\ncar_dirt_delta: %d\nuse_dyn_cam: %d\n" \
                     "car_pressure_multi: %f\ncar_ss_multi: %f\n" \
                     "car_sd_multi: %f\ncar_sl_multi: %f\n",
                &savedState->carPos.X,
                &savedState->carPos.Y,
                &savedState->carPos.Z,
                &savedState->carRot.X,
                &savedState->carRot.Y,
                &savedState->carRot.Z,
                &savedState->currentTime,
                &savedState->cps,
                &savedState->penality,
                &savedState->timeStarted,
                &savedState->timeEnded,
                &cpsize,
                &savedState->day_start_time_offset,
                &savedState->currentStage,
                &savedState->globalTime,
                &savedState->carType,
                &savedState->view_num,
                &savedState->savedCarDirt,
                &savedState->car_dirt_delta,
                &savedState->useDynCam,
                &savedState->car_pressure_multi,
                &savedState->car_ss_multi,
                &savedState->car_sd_multi,
                &savedState->car_sl_multi
            );
    if ( ret < 24 )
    {
        printf("error reading %s ret %d errno %d\n", name, ret, errno);
        fclose(f);
        return false;
    }

    dprintf(printf("load game - load cps: %d\n", cpsize);)

    savedState->cpTime.clear();
    savedState->cpTimed.clear();
    dprintf(printf("cp times cleared\n");)
    for (int i = 0; i < cpsize; i++)
    {
        int dummy = 0;
        int cpTime = 0;
        int cpTimed = 0;
        ret = fscanf(f, "cptimes[%d]: %d %d\n",
                &dummy,
                &cpTime,
                &cpTimed
            );
        if ( ret < 3 )
        {
            printf("error reading %s ret %d errno %d\n", name, ret, errno);
            fclose(f);
            return false;
        }
        savedState->cpTime.push_back(cpTime);
        savedState->cpTimed.push_back(cpTimed);
    }
    
    ret = fscanf(f, "race_engine: %d\n", &useRaceEngine);
    if ( ret < 1 )
    {
        printf("error reading %s ret %d errno %d\n", name, ret, errno);
        fclose(f);
        return false;
    }
    if (raceEngine)
    {
        dprintf(printf("delete race engine %p\n", raceEngine);)
        delete raceEngine;
        raceEngine = 0;
    }
    dprintf(printf("clear states\n");)
    CRaceEngine::clearStates();
    dprintf(printf("create new race engine\n");)
    loadedRaceEngine = new CRaceEngine(smgr, env, bigTerrain);
    CRaceEngine::getRaceState() = competitors;
    CRaceEngine::getRaceState().push_back(playerCompetitor);
    CRaceEngine::getStageState() = CRaceEngine::getRaceState();

    dprintf(printf("load game - load raceEngine: %d\n", useRaceEngine);)
    
    if (!useRaceEngine || !loadedRaceEngine->load(f, smgr, env))
    {
        printf("error reading %s ret %d errno %d\n", name, ret, errno);
        fclose(f);
        delete savedState;
        savedState = 0;
        delete loadedRaceEngine;
        loadedRaceEngine = 0;
        return false;
    }
        
    fclose(f);

    dprintf(printf("load game end, restore state\n");)
    
    restoreStateInternal(2);
    
    //startNewGame = 2; // TODO: what is it for?
    
    return true;
}

void restartStage()
{
    if (car)
        savedCarDirt = car->getDirt();
    else
        savedCarDirt = 0;
    startNewGame = 1;
    currentStage = oldStage;
    startGame(currentStage);
}

bool checkLoadGame(const c8* name)
{
    FILE* f;
    
    f = fopen(name, "r");

    if (!f)
    {
        return false;
    }
    
    fclose(f);
    
    return true;
}

void addToShadowNodes(irr::scene::ISceneNode* node)
{
    shadowNodes.push_back(node);
}

void removeFromShadowNodes(irr::scene::ISceneNode* node)
{
    int del = shadowNodes.binary_search(node);
    if (del>=0)
        shadowNodes.erase(del);
}

void addToDepthNodes(irr::scene::ISceneNode* node)
{
    depthNodes.push_back(node);
}

void removeFromDepthNodes(irr::scene::ISceneNode* node)
{
    int del = depthNodes.binary_search(node);
    if (del>=0)
        depthNodes.erase(del);
}

void addToObjectNodes(irr::scene::ISceneNode* node)
{
    objectNodes.push_back(node);
}

void removeFromObjectNodes(irr::scene::ISceneNode* node)
{
    int del = objectNodes.binary_search(node);
    if (del>=0)
        objectNodes.erase(del);
}

void saveState()
{
    saveGame(SAVE_TMP_FILE);
}

void saveStateInternal()
{
    if (car && bigTerrain)
    {
        if (savedState==0)
            savedState = new SState;
        
        savedState->carPos = car->getMatrix().getTranslation();
        savedState->carRot = vector3df(0.f, car->getAngle(), 0.f);//car->getMatrix().getRotationDegrees();
        bigTerrain->saveState(savedState);

        savedState->day_start_time_offset = lasttick - day_start_time;
        savedState->currentStage = oldStage;
        savedState->globalTime = globalTime;
        savedState->carType = carType;
        savedState->view_num = view_num;
        savedState->savedCarDirt = savedCarDirt = car->getDirt();
        savedState->car_dirt_delta = car_dirt_delta;
        savedState->useDynCam = useDynCam;
        savedState->car_pressure_multi = car_pressure_multi;
        savedState->car_ss_multi = car_ss_multi;
        savedState->car_sd_multi = car_sd_multi;
        savedState->car_sl_multi = car_sl_multi;
    }
}

bool restoreState()
{
    return loadGame(SAVE_TMP_FILE);
}

bool restoreStateInternal(int newStartNewGame)
{
    if (savedState)
    {
        startNewGame = newStartNewGame;
        currentStage = oldStage = savedState->currentStage;
        globalTime = savedState->globalTime;
        carType = savedState->carType;
        view_num = savedState->view_num;
        savedCarDirt = savedState->savedCarDirt;
        car_dirt_delta = savedState->car_dirt_delta;
        useDynCam = savedState->useDynCam;
        car_pressure_multi = savedState->car_pressure_multi;
        car_ss_multi = savedState->car_ss_multi;
        car_sd_multi = savedState->car_sd_multi;
        car_sl_multi = savedState->car_sl_multi;
        
        //viewpos_cur = viewpos[view_num];
        //viewdest_cur = viewdest[view_num];
        car_dirt = (float)savedCarDirt/(float)(MAX_CAR_DIRT-1); //tcd;

        startGame(oldStage, savedState);
        delete savedState;
        savedState = 0;
        return true;
    }
    return false;
}

void removeState()
{
    unlink(SAVE_TMP_FILE);
}

void releaseGameStuff()
{
    if (stages)
    {
        for (int i = 0; i < MAX_STAGES; i++)
        {
            printf("remove %i. (%p) stage\n", i, stages[i]);
            if (stages[i])
            {
                delete stages[i];
                stages[i] = 0;
            }
        }
        delete [] stages;
        stages = 0;
    }
    if (savedState!=0)
    {
        delete savedState;
        savedState = 0;
    }
}
