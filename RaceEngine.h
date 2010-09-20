/****************************************************************
*                                                               *
*    Name: RaceEngine.h                                         *
*                                                               *
*    Creator: Balazs Tuska                                      *
*                                                               *
*    Description:                                               *
*                                                               *
****************************************************************/

#ifndef __RACEENGINE_H__
#define __RACEENGINE_H__

#include "irrlicht.h"
#include "NewtonRaceCar.h"
#include "BigTerrain.h"
#include <Newton.h>
#include "MyList.h"
#include "CQuad.h"
#include "competitors.h"

//#define SPEED_BASE_AI 1


// Irrlicht Namespaces
using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

class CRaceEngine;
class OffsetObject;

class SStarter
{
public:
    SStarter(ISceneManager* smgr, IGUIEnvironment* env, BigTerrain* p_bigTerrain, CRaceEngine* p_raceEngine, SCompetitor* p_competitor, u32 p_startingCD);
    ~SStarter();
    
    bool update(u32 currentTime, const vector3df& p_me, bool camActive);
    void switchToVisible();
    void switchToNotVisible();
    void goToNextPoint(u32 currentTime);
    void calculateTo(vector3df &p_nextPointPos);

    BigTerrain* m_bigTerrain;
    SCompetitor* competitor;
    u32 startingCD;
    u32 startTime;
    u32 finishTime;
    int nextPoint;
    vector2df currentPos;
    bool crashed;
    bool visible;
    NewtonRaceCar* vehicle;
    CRaceEngine* m_raceEngine;
    unsigned int forResetCnt;
    unsigned int forBigResetCnt;
    unsigned int forNonResetCnt;
    float currentRand;
    ITextSceneNode* nameText;
    OffsetObject* nameTextOffsetObject;
    vector2df dir;
#ifdef SPEED_BASE_AI
    float nextPointCD;
    //float nextPointDist;
#else
    float passedDistance;
    float distanceStep;
#endif
    float stageRand;
};

class CRaceEngine
{
public:
    enum UpdateWhen
    {
        AtStart = 0,
        InTheMiddle,
        AtTheEnd
    };
    
    CRaceEngine(ISceneManager* smgr, IGUIEnvironment* env, BigTerrain* p_bigTerrain /*, core::array<SCompetitor*> &p_raceState*/);
    ~CRaceEngine();
    bool update(u32 p_tick, const vector3df& me, SCompetitor* p_playerCompetitor, irr::IrrlichtDevice* device, UpdateWhen when = InTheMiddle);
    void updateShowNames();
    void addUpdater(SStarter* starter);
    void removeUpdater(SStarter* starter);
    core::array<SCompetitor*> &getFinishedState() {return finishedState;}
    core::array<SStarter*> &getStarters() {return starters;}
    int insertIntoFinishedState(SCompetitor* competitor);
    
    bool save(FILE* f/*, SCompetitor* p_playerCompetitor*/);
    bool load(FILE* f, ISceneManager* smgr, IGUIEnvironment* env/*, SCompetitor* p_playerCompetitor*/);
    void refreshBigTerrain(BigTerrain* p_bigTerrain);
    
    bool isRaceFinished() {return raceFinished;}
    
    static void refreshRaceState(CRaceEngine* stageState);
    static core::array<SCompetitor*> &getRaceState() {return raceState;}

private:
    BigTerrain* m_bigTerrain;
    core::array<SStarter*> starters;
    core::array<SStarter*> cupdaters;
    core::array<SCompetitor*> finishedState;
    u32 lastMTick;
    u32 lastCTick;
    u32 currentTime;
    bool raceFinished;

    static core::array<SCompetitor*> raceState;
};

#endif // __RACEENGINE_H__
