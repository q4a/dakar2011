/****************************************************************
*                                                               *
*    Name: RaceEngine.cpp                                       *
*                                                               *
*    Creator: Balazs Tuska                                      *
*                                                               *
*    Description:                                               *
*                                                               *
****************************************************************/

#include "RaceEngine.h"
#include "settings.h"

#include "gameplay.h"
//extern scene::ISceneNode* skydome;
#include "my_shaders.h"
//extern video::ITexture* shadowMap;
#include "message.h"
#include "fonts.h"

#include "linux_includes.h"

#define FAR_VALUE ((farValue / 3.f)+400.f)
#define NEAR_VALUE (farValue / 3.f)
#ifdef MY_DEBUG
# define START_SECS 10
#else
# define START_SECS 40
#endif
#define REACHED_POINT_DIST 15.f
#define REACHED_POINT_DIST_NEAR 20.f
#define ANGLE_LIMIT 30.f
#define AI_STEP (1.15f) // orig: (1.05f)

core::array<SCompetitor*> CRaceEngine::raceState;

//#define pdprintf(x) x

// normalize angle between 0 and 360
float normalizeAngle(float &angle)
{
    while (angle > 360.f) angle -= 360.f;
    while (angle < 0.f) angle += 360.f;
    return angle;
}

float normalizeAngle180(float &angle)
{
    while (angle > 180.f) angle -= 360.f;
    while (angle < -180.f) angle += 360.f;
    return angle;
}

SStarter::SStarter(ISceneManager* smgr, IGUIEnvironment* env,
                   BigTerrain* p_bigTerrain, CRaceEngine* p_raceEngine,
                   SCompetitor* p_competitor, u32 p_startingCD)
    : m_bigTerrain(p_bigTerrain), m_raceEngine(p_raceEngine),
      competitor(p_competitor), startingCD(p_startingCD), startTime(0),
      finishTime(0), nextPoint(0), currentPos(),
      crashed(false), visible(false), vehicle(0), forResetCnt(0),
      forBigResetCnt(0), forNonResetCnt(500),
      currentRand(0.f), nameText(0), nameTextOffsetObject(0),
#ifdef SPEED_BASE_AI
      nextPointCD(0.f), dir()//, nextPointDist(0.f)
#else
      passedDistance(0.f), distanceStep(0.f)
#endif
{
    nameText = smgr->addTextSceneNode(/*env->getBuiltInFont()*/ fonts[FONT_SPECIAL14], competitor->getName().c_str(), video::SColor(255, 255, 255, 0));
    //nameText->setScale(vector3df(3.0f, 3.0f, 3.0f));
    nameText->setVisible(false);
}

SStarter::~SStarter()
{
    if (vehicle)
    {
        vehiclePool->putVehicle(vehicle);
        vehicle = 0;
    }
    if (nameText)
    {
        nameText->remove();
        nameText = 0;
    }
    if (nameTextOffsetObject)
    {
        nameTextOffsetObject->setNode(0);
        offsetManager->removeObject(nameTextOffsetObject);
        delete nameTextOffsetObject;
        nameTextOffsetObject = 0;
    }
}

bool SStarter::update(u32 currentTime, const vector3df& p_me, bool camActive)
{
    if (finishTime) return false;
    float distFromMe = currentPos.getDistanceFrom(vector2df(p_me.X, p_me.Z));
    
    if (visible && vehicle)
    {
        if (distFromMe > FAR_VALUE || !camActive)
        {
            switchToNotVisible();
            return true;
        }
        // base calculation
        vector3df cp(offsetManager->getOffset()+vehicle->getMatrix().getTranslation());
        currentPos = vector2df(cp.X, cp.Z);
        vector2df nextPointPos(m_bigTerrain->getAIPoints()[nextPoint]->getPosition().X,
                               m_bigTerrain->getAIPoints()[nextPoint]->getPosition().Z);
        float distToNextPoint = currentPos.getDistanceFrom(nextPointPos);
        if (distToNextPoint < REACHED_POINT_DIST)
        {
            goToNextPoint(currentTime);
            if (finishTime)
            {
                switchToNotVisible();
                return true;
            }
            nextPointPos = vector2df(m_bigTerrain->getAIPoints()[nextPoint]->getPosition().X,
                                     m_bigTerrain->getAIPoints()[nextPoint]->getPosition().Z);
            distToNextPoint = currentPos.getDistanceFrom(nextPointPos);
        }
        
        
        float torque = 1.0f;
        float brake = 0.0f;
        float steer = 0.0f;
        float vehicleAngle = vehicle->getAngle(); normalizeAngle(vehicleAngle);
        const float speed = vehicle->getSpeed()*1.6f;
        const float speedLimitLow = (((m_bigTerrain->getSpeed()-30.f) * (float)competitor->strength) / 100.f) * (1.f + currentRand);
        const float speedLimitDist = 40.f;
        const float speedLimitHigh = speedLimitLow + speedLimitDist;
        const float angleLimit = ANGLE_LIMIT;
        const float angleLimitMax = 180.f;
        const float brakeSpeedLimit = 30.f;
        const float brakeSpeedLimitMax = 5.f;
        
        if (fabsf(speed) < 0.5f)
        {
            forResetCnt++;
            if (forResetCnt >= 20)
            {
                dprintf(printf("-------------\nreset AI car %d\nnrc: %u\nbrc: %u\n------------\n", competitor->num, forNonResetCnt, forBigResetCnt);)
                if (forNonResetCnt >= 500)
                {
                    competitor->lastPenalityTime += RESET_PENALITY/2;
                    cp += vector3df(3.f, 3.f, 3.f); // we don't know why stop: tree or felt over
                    forBigResetCnt++;
                    if (forBigResetCnt >= 2)
                    {
                        forNonResetCnt = 0;
                        forBigResetCnt = 0;
                    }
                }
                else
                {
                    competitor->lastPenalityTime += RESET_PENALITY;
                    if (0 < nextPoint)
                    {
                        cp = vector3df(m_bigTerrain->getAIPoints()[nextPoint-1]->getPosition().X,
                                       m_bigTerrain->getHeight(m_bigTerrain->getAIPoints()[nextPoint-1]->getPosition().X,
                                                               m_bigTerrain->getAIPoints()[nextPoint-1]->getPosition().Z)+3.0f,
                                       m_bigTerrain->getAIPoints()[nextPoint-1]->getPosition().Z);
                    }
                    else
                    {
                        cp = vector3df(m_bigTerrain->getAIPoints()[0]->getPosition().X,
                                       m_bigTerrain->getHeight(m_bigTerrain->getAIPoints()[0]->getPosition().X,
                                                               m_bigTerrain->getAIPoints()[0]->getPosition().Z)+3.0f,
                                       m_bigTerrain->getAIPoints()[0]->getPosition().Z);
                    }
                }
                vehicle->reset(cp-offsetManager->getOffset());
                forResetCnt=0;
            }
        }
        else
        {
            forNonResetCnt++;
            if (forNonResetCnt >= 100000)
            {
                forNonResetCnt = 500;
            }
        }
        
        if (vehicle->getDemagePer() > 40.f)
        {
            float demage = vehicle->getDemagePer();
            int penality = (int)(3.f*demage);
            for (int i = 0; i < 4; i++) 
            {
                if (!vehicle->isTyreConnected(i))
                {
                    penality += 15;
                }
            }
            dprintf(printf("------------\nrepair AI car %d, penality: %d\n------------\n", competitor->num, penality);)
            competitor->lastPenalityTime += penality;
            vehicle->repair();
        }
        
        float nextPointAngle = (float)(nextPointPos - currentPos).getAngle(/*With(vector2df(-1.0f, 0.0f)*/)/*-180.f*/;
        normalizeAngle(nextPointAngle);
        float nextNextPointAngle = nextPointAngle;
        if (nextPoint < m_bigTerrain->getAIPoints().size()-1)
        {
            vector2df nextNextPointPos(m_bigTerrain->getAIPoints()[nextPoint+1]->getPosition().X,
                                       m_bigTerrain->getAIPoints()[nextPoint+1]->getPosition().Z);
            nextNextPointAngle = (float)(nextNextPointPos - nextPointPos).getAngle(/*With(vector2df(-1.0f, 0.0f)*/)/*-180.f*/;
            normalizeAngle(nextNextPointAngle);
        }
        float angleToNext = nextPointAngle - vehicleAngle; normalizeAngle180(angleToNext);
        float angleToNextNext = nextNextPointAngle - vehicleAngle; normalizeAngle180(angleToNextNext);
        
        pdprintf(printf("nextPoint: %d, va: %f, nToA %f\n", nextPoint, vehicleAngle, angleToNext);)
        pdprintf(printf("speed: %f, dist: %f, nA %f\n", speed, distToNextPoint, nextPointAngle);)
        
        // calculate torque base
        if (speed < speedLimitLow)
        {
            torque = 1.0f;
        }
        else if (speed > speedLimitHigh)
        {
            torque = 0.0f;
        }
        else
        {
            torque = 0.75f + ((speedLimitHigh - speed)/speedLimitDist) * 0.25f;
        }
        
        // calculate torque more, and brake
        if (angleToNext > angleLimit)
        {
            // big difference
            if (speed > brakeSpeedLimit)
            {
                brake = 1.0f;
            }
            steer = 1.0f;
        }
        else
        if (angleToNext < -angleLimit)
        {
            // big difference
            if (speed > brakeSpeedLimit)
            {
                brake = 1.0f;
            }
            steer = -1.0f;
        }
        else
        {
            // small difference
            steer = angleToNext / (angleLimit*2.0f);
        }
        
        // if we are near to the nextPoint calculate some to the next-next point
        if (distToNextPoint<(REACHED_POINT_DIST_NEAR+(speed*0.5f)) && fabsf(angleToNextNext) > (angleLimit*2.f)/3.f)
        {
            float brakeMulti = 0.0f;
            if (speed > brakeSpeedLimit*4)
            {
                brake = 1.0f;
            }
            else
            if (speed > brakeSpeedLimit)
            {
                brake = (speed - brakeSpeedLimit) / (brakeSpeedLimit*3);
            }
            
            if (fabsf(angleToNextNext) > (angleLimit*2.f))
            {
                brakeMulti = 1.0f;
            }
            else
            if (fabsf(angleToNextNext) > (angleLimit*2.f)/3.f)
            {
                brakeMulti = (fabsf(angleToNextNext) - (angleLimit*2.f)/3.f) / (angleLimit+(angleLimit/3.f));
            }
            brake = brake * brakeMulti;
        }
        vehicle->setHandBrakes(brake);
        vehicle->setSteering(steer);
        vehicle->setTireTorque(torque);
#ifndef SPEED_BASE_AI
        passedDistance += distanceStep/10.f;
#endif
    }
    else // not visible or there is no free pool vehicle
    {
        if (distFromMe < NEAR_VALUE && camActive)
        {
            switchToVisible();
            if (visible)
                return true;
        }
        
        vector2df oldPos(currentPos);
        
#ifdef SPEED_BASE_AI
        vector2df cp(m_bigTerrain->getAIPoints()[nextPoint]->getPosition().X,
                     m_bigTerrain->getAIPoints()[nextPoint]->getPosition().Z);
        float ndist = 0.f;

        if (nextPoint > 0 && nextPoint < m_bigTerrain->getAIPoints().size()-1)
        {
            vector2df ncp(m_bigTerrain->getAIPoints()[nextPoint+1]->getPosition().X,
                          m_bigTerrain->getAIPoints()[nextPoint+1]->getPosition().Z);
            ndist = cp.getDistanceFrom(ncp);
        }

        if (dir.getLength() < ndist)
        {
            currentPos += dir;
        }

        //vector2df nextPos(currentPos+dir);
        
        nextPointCD -= (AI_STEP+currentRand/*angleMulti*/);
        if (nextPointCD<=0.f /*|| 
            (nextPoint%2==0 && cp.isBetweenPoints(oldPos, currentPos)) ||
            (nextPoint%2==1 && cp.isBetweenPoints(currentPos, nextPos))*/
           )
        {
            goToNextPoint(currentTime);
        }
#else // SPEED_BASE_AI
        passedDistance += distanceStep;
        if (passedDistance > m_bigTerrain->getAIPoints()[nextPoint]->getDistance()-0.0001f)
        {
            goToNextPoint(currentTime);
        }
        if (!finishTime && nextPoint > 0 && nextPoint < m_bigTerrain->getAIPoints().size())
        {
            vector2df cp(m_bigTerrain->getAIPoints()[nextPoint-1]->getPosition().X,
                         m_bigTerrain->getAIPoints()[nextPoint-1]->getPosition().Z);
            vector2df ncp(m_bigTerrain->getAIPoints()[nextPoint]->getPosition().X,
                          m_bigTerrain->getAIPoints()[nextPoint]->getPosition().Z);
            float dist = m_bigTerrain->getAIPoints()[nextPoint-1]->getDistance();
            float ndist = m_bigTerrain->getAIPoints()[nextPoint]->getDistance();
            vector2df dir = (ncp - cp) * ((passedDistance-dist)/(ndist-dist));
            currentPos = cp + dir;
            // remove me
//            if (competitor->num==300)
//            {
//                printf("num 300 nextPoint %d, pos: x %f z %f passed dist %f dist %f ndist %f\n", nextPoint, currentPos.X, currentPos.Y, passedDistance,
//                        dist, ndist);
//            }
        }
#endif // SPEED_BASE_AI
    }
    return true;
}

void SStarter::switchToVisible()
{
    dprintf(printf("%d became visible\n", competitor->num);)
    vehicle = vehiclePool->getVehicle(competitor->carIndex);
    if (vehicle)
    {
        visible = true;
        vector3df pos(currentPos.X, m_bigTerrain->getHeight(currentPos.X, currentPos.Y)+1.2f, currentPos.Y);
#ifdef SPEED_BASE_AI
        vector3df rot(0.f, (float)dir.getAngle(/*With(vector2df(-1.0f, 0.0f)*/)/*-180.f*/, 0.f);
#else
        vector3df rot;
        if (nextPoint > 0 && nextPoint < m_bigTerrain->getAIPoints().size())
        {
            vector2df cp(m_bigTerrain->getAIPoints()[nextPoint]->getPosition().X,
                         m_bigTerrain->getAIPoints()[nextPoint]->getPosition().Z);
            vector2df pcp(m_bigTerrain->getAIPoints()[nextPoint-1]->getPosition().X,
                          m_bigTerrain->getAIPoints()[nextPoint-1]->getPosition().Z);
            rot = vector3df(0.f, (float)(cp-pcp).getAngle(/*With(vector2df(-1.0f, 0.0f)*/)/*-180.f*/, 0.f);
        }
#endif
        vehicle->activate(pos,
                      rot,
                      ""/*m_bigTerrain->getGroundSoundName()*/,
                      ""/*m_bigTerrain->getPuffSoundName()*/,
                      ""/*m_bigTerrain->getSkidSoundName()*/,
                      m_bigTerrain->getFrictionMulti(),
                      skydome,
                      shadowMap,
                      m_bigTerrain->getWaterHeight(),
                      true,
                      0);
        vehicle->setNameText(nameText);
        m_raceEngine->addUpdater(this);
        if (/*settings*/show_names)
            nameText->setVisible(true);
        nameTextOffsetObject = new OffsetObject(nameText, true);
        offsetManager->addObject(nameTextOffsetObject);
    }
    dprintf(printf("%d became visible end\n", competitor->num);)
}

void SStarter::switchToNotVisible()
{
    dprintf(printf("%d became not visible\n", competitor->num);)
    visible = false;
    if (vehicle)
    {
        vector3df cp(offsetManager->getOffset()+vehicle->getMatrix().getTranslation());
        vehicle->setNameText(0);
        vehiclePool->putVehicle(vehicle);
        vehicle = 0;
        currentPos = vector2df(cp.X, cp.Z);
        calculateTo(m_bigTerrain->getAIPoints()[nextPoint]->getPosition());
        nameText->setVisible(false);
        if (nameTextOffsetObject)
        {
            nameTextOffsetObject->setNode(0);
            offsetManager->removeObject(nameTextOffsetObject);
            delete nameTextOffsetObject;
            nameTextOffsetObject = 0;
        }
    }
    m_raceEngine->removeUpdater(this);
}

void SStarter::goToNextPoint(u32 currentTime)
{
    if (nextPoint < m_bigTerrain->getAIPoints().size())
    {
#ifdef SPEED_BASE_AI
        vector3df cp(m_bigTerrain->getAIPoints()[nextPoint]->getPosition());
        currentPos = vector2df(cp.X, cp.Z);
        nextPoint++;
#else
        if (visible)
        {
            passedDistance = m_bigTerrain->getAIPoints()[nextPoint]->getDistance();
            nextPoint++;
        }
        else
        {
            while (nextPoint < m_bigTerrain->getAIPoints().size() &&
                   passedDistance > m_bigTerrain->getAIPoints()[nextPoint]->getDistance())
            {
                nextPoint++;
            }
        }
#endif
    }
//    if (competitor->num==300)
//        printf("num: %d, nextPoint: %d/%d\n", competitor->num, nextPoint, m_bigTerrain->getAIPoints().size());
//    else
    if (nextPoint >= m_bigTerrain->getAIPoints().size())
    {
        // no more point finish the stage
        finishTime = currentTime - startTime;
        competitor->lastTime = finishTime + competitor->lastPenalityTime;
        competitor->globalTime += competitor->lastTime;
        competitor->globalPenalityTime += competitor->lastPenalityTime;
        
        int position = m_raceEngine->insertIntoFinishedState(competitor);
        core::stringw str = L"";
        
        str += competitor->num;
        str += L" ";
        str += competitor->getName();
        str += L" finished the stage, time: ";
        m_bigTerrain->addTimeToStr(str, competitor->lastTime);
        str += L",  position: ";
        str += position;
        MessageText::addText(str.c_str(), 2);
    }
    else
    {
#ifndef SPEED_BASE_AI
        if (nextPoint > 0)
        {
            vector2df cp(m_bigTerrain->getAIPoints()[nextPoint-1]->getPosition().X,
                         m_bigTerrain->getAIPoints()[nextPoint-1]->getPosition().Z);
            vector2df ncp(m_bigTerrain->getAIPoints()[nextPoint]->getPosition().X,
                          m_bigTerrain->getAIPoints()[nextPoint]->getPosition().Z);
            float dist = m_bigTerrain->getAIPoints()[nextPoint-1]->getDistance();
            float ndist = m_bigTerrain->getAIPoints()[nextPoint]->getDistance();
            dir = (ncp - cp) * ((passedDistance-dist)/(ndist-dist));
            currentPos = cp;// + dir;
        }
#endif
        currentRand = (((float)(rand() % 100) - 50.f) / 1000.f) + ((float)competitor->strength / 50000.f);
        calculateTo(m_bigTerrain->getAIPoints()[nextPoint]->getPosition());
    }
}

void SStarter::calculateTo(vector3df &p_nextPointPos)
{
#ifdef SPEED_BASE_AI
    vector2df nextPointPos(p_nextPointPos.X, p_nextPointPos.Z);
    
    float speed = ((m_bigTerrain->getSpeed() / 3.6f) * (float)competitor->strength) / 100.f;
    dir = nextPointPos - currentPos;
    float dist = dir.getLength();
    
    if (speed < 1.0f) speed = 1.f;
    if (dist < 1.0f) dist = 1.f;
    
    nextPointCD += dist / speed;
    //if (nextPointCD==0) nextPointCD = 1;
    //dir = dir / (dist / speed);
    dir = dir.normalize();
    dir = dir * (dist / nextPointCD);
#else
    distanceStep = (m_bigTerrain->getStageLength() / (float)m_bigTerrain->getStageTime()) *
                   ((float)competitor->strength / 100.f) *
                   (1.0f+currentRand);
#endif
}

CRaceEngine::CRaceEngine(ISceneManager* smgr, IGUIEnvironment* env,
                         BigTerrain* p_bigTerrain/*, core::array<SCompetitor*> &p_raceState*/)
    : m_bigTerrain(p_bigTerrain), starters(), cupdaters(), lastMTick(0),
      lastCTick(0), currentTime(0), raceFinished(false)
{
    for (int i = 0; i < raceState.size(); i++)
    {
        //if (raceState[i]->ai)
        //{
            SStarter* starter = new SStarter(smgr, env, m_bigTerrain, this, raceState[i], START_SECS);
            starters.push_back(starter);
        //}
    }
}

CRaceEngine::~CRaceEngine()
{
    for (int i = 0; i < starters.size(); i++)
    {
        delete starters[i];
        starters[i] = 0;
    }
    starters.clear();
    cupdaters.clear();
}

bool CRaceEngine::update(u32 p_tick, const vector3df& me, SCompetitor* p_playerCompetitor, irr::IrrlichtDevice* device, UpdateWhen when)
{
    const u32 mtick = p_tick/1000;
    const u32 ctick = p_tick/100;
    int updates = 0;
    
    if (raceFinished) return false;
    
    if (mtick != lastMTick || when != InTheMiddle)
    {
        pdprintf(printf("race engine update\n");)
        currentTime++;
        
        for (int i = 0; i < starters.size(); i++)
        {
            pdprintf(printf("race engine %d. sCD %d, nextPoint %d/%d, pd %f, finishTime %d\n",
                i, starters[i]->startingCD, starters[i]->nextPoint, m_bigTerrain->getAIPoints().size(),
                starters[i]->passedDistance, starters[i]->finishTime);)
/*
            if (m_bigTerrain->getAIPoints().size()!=starters[i]->nextPoint && when == AtTheEnd)
            {
                printf("race engine %d. sCD %d, nextPoint %d/%d, passedfinishTime %d\n",
                    i, starters[i]->startingCD, starters[i]->nextPoint, m_bigTerrain->getAIPoints().size(), starters[i]->finishTime);
            }
*/
//            if (when == AtTheEnd) device->sleep(100);
            if (starters[i]->startingCD > 0)
            {
                updates++;
                starters[i]->startingCD--;
                if (starters[i]->startingCD==0)
                {
                    starters[i]->startTime = currentTime;
                    if (starters[i]->competitor==p_playerCompetitor)
                    {
                        return false;
                    }
                    starters[i]->goToNextPoint(currentTime);
                }
                else
                    break;
            }
            else
            if (starters[i]->finishTime==0 && starters[i]->competitor->ai)
            {
                if (starters[i]->update(currentTime, me, when == InTheMiddle))
                {
                    updates++;
                }
            }
        }
        
        pdprintf(printf("race engine update end, updates: %d\n", updates);)
        lastMTick = mtick;
    }
    else
    if (ctick != lastCTick)
    {
        for (int i = 0; i < cupdaters.size(); i++)
        {
            if (cupdaters[i]->visible)
            {
                cupdaters[i]->update(currentTime, me, when == InTheMiddle);
            }
        }
        lastCTick = ctick;
    }
    
    if (updates==0 && when == AtTheEnd)
    {
        raceFinished = true;
    }
    return updates!=0;
}

void CRaceEngine::addUpdater(SStarter* starter)
{
    cupdaters.push_back(starter);
}

void CRaceEngine::removeUpdater(SStarter* starter)
{
    for (int i = 0; i < cupdaters.size(); i++)
    {
        if (starter==cupdaters[i])
        {
            cupdaters.erase(i);
            break;
        }
    }
}

int CRaceEngine::insertIntoFinishedState(SCompetitor* competitor)
{
    int i = 0;
    while (i < finishedState.size() && competitor->lastTime >= finishedState[i]->lastTime)
    {
        i++;
    }
    finishedState.insert(competitor, i);
    return i;
}

void CRaceEngine::refreshRaceState(CRaceEngine* stageState)
{
    raceState.clear();
/*
    for (int i = 0; i < stageState->finishedState.size(); i++)
    {
        int j = 0;
        while (j < raceState.size() && stageState->finishedState[i]->lastTime >= raceState[j]->lastTime)
        {
            j++;
        }
        raceState.insert(stageState->finishedState[i], j);
    }
*/
    for (int i = 0; i < stageState->starters.size(); i++)
    {
        int j = 0;
        //if (stageState->starters[i]->competitor->lastTime == 0) continue;
        while (j < raceState.size() &&
               (
                (stageState->starters[i]->competitor->lastTime==0 && raceState[j]->lastTime!=0) ||
                (stageState->starters[i]->competitor->globalTime >= raceState[j]->globalTime)
               )
              )
        {
            j++;
        }
        raceState.insert(stageState->starters[i]->competitor, j);
    }
}

bool CRaceEngine::save(FILE* f/*, SCompetitor* p_playerCompetitor*/)
{
#ifdef SPEED_BASE_AI
# error "Speed base AI is not supported any more"
#endif
    int ret;
    
    ret = fprintf(f, "current_time: %u\n", currentTime);
    if (ret < 1)
    {
        printf("error writing save file ret %d errno %d\n", ret, errno);
        return false;
    }
    
    ret = fprintf(f, "starters: %d\n", starters.size());
    if (ret < 1)
    {
        printf("error writing save file ret %d errno %d\n", ret, errno);
        return false;
    }
    
    for (int i = 0; i < starters.size(); i++)
    {
/*
#ifdef SPEED_BASE_AI
        ret = fprintf(f, "starter[%d]: %d %u %u %u %u %u %u %u %d %f %f %f %f %f\n",
                      i,
                      starters[i]->competitor->num,
                      starters[i]->competitor->lastTime,
                      starters[i]->competitor->lastPenalityTime,
                      starters[i]->competitor->globalTime,
                      starters[i]->competitor->globalPenalityTime,
                      starters[i]->startingCD,
                      starters[i]->startTime,
                      starters[i]->finishTime,
                      starters[i]->nextPoint,
                      starters[i]->nextPointCD,
                      starters[i]->currentPos.X,
                      starters[i]->currentPos.Y,
                      starters[i]->dir.X,
                      starters[i]->dir.Y
                      //, starters[i]->nextPointDist
                      );
        if (ret < 15)
        {
            printf("error writing save file ret %d errno %d\n", ret, errno);
            return false;
        }
#else // SPEED_BASE_AI
*/
        ret = fprintf(f, "starter[%d]: %d %u %u %u %u %u %u %u %d %f %f %f\n",
                      i,
                      starters[i]->competitor->num,
                      starters[i]->competitor->lastTime,
                      starters[i]->competitor->lastPenalityTime,
                      starters[i]->competitor->globalTime,
                      starters[i]->competitor->globalPenalityTime,
                      starters[i]->startingCD,
                      starters[i]->startTime,
                      starters[i]->finishTime,
                      starters[i]->nextPoint,
                      starters[i]->currentPos.X,
                      starters[i]->currentPos.Y,
                      starters[i]->passedDistance
                      );
        if (ret < 13)
        {
            printf("error writing save file ret %d errno %d\n", ret, errno);
            return false;
        }
/*
#endif // SPEED_BASE_AI
*/
    }
    return true;
}

bool CRaceEngine::load(FILE* f, ISceneManager* smgr, IGUIEnvironment* env/*, SCompetitor* p_playerCompetitor*/)
{
#ifdef SPEED_BASE_AI
# error "Speed base AI is not supported any more"
#endif
    int ret;
    int startersSize = 0;
    int tmpi;
    int compnum;

    ret = fscanf(f, "current_time: %u\n", &currentTime);
    if (ret < 1)
    {
        printf("error reading save file ret %d errno %d\n", ret, errno);
        return false;
    }
    
    ret = fscanf(f, "starters: %d\n", &startersSize);
    if (ret < 1)
    {
        printf("error reading save file ret %d errno %d\n", ret, errno);
        return false;
    }
    
    for (int i = 0; i < starters.size();i++) delete starters[i];
    starters.clear();
    
    for (int i = 0; i < startersSize; i++)
    {
        SStarter* starter = 0;
        int j = 0;
        
        ret = fscanf(f, "starter[%d]: %d",
                      &tmpi,
                      &compnum);
        if (ret < 2)
        {
            printf("error reading save file ret %d errno %d\n", ret, errno);
            return false;
        }
        //dprintf(printf("read stater[%d], num %d, raceState.size() = %d\n", tmpi, compnum, raceState.size());)
        
        for (j = 0; j < raceState.size(); j++)
        {
            if (raceState[j]->num == compnum) break;
        }
        //printf("read stater[%d], num %d, j %d, raceState.size() = %d\n", tmpi, compnum, j, raceState.size());
        if (j < raceState.size())
        {
            starter = new SStarter(smgr, env, m_bigTerrain, this, raceState[j], START_SECS);
            //dprintf(printf("found competitor, create starter %p\n", starter);)
        }
        else
        {
            printf("error reading save file num %d not found in raceState\n", compnum);
            return false;
        }
/*
#ifdef SPEED_BASE_AI
        ret = fscanf(f, "%u %u %u %u %u %u %u %d %f %f %f %f %f\n",
                      &starter->competitor->lastTime,
                      &starter->competitor->lastPenalityTime,
                      &starter->competitor->globalTime,
                      &starter->competitor->globalPenalityTime,
                      &starter->startingCD,
                      &starter->startTime,
                      &starter->finishTime,
                      &starter->nextPoint,
                      &starter->nextPointCD,
                      &starter->currentPos.X,
                      &starter->currentPos.Y,
                      &starter->dir.X,
                      &starter->dir.Y
                      //, &starter->nextPointDist
                      );
        if (ret < 13)
        {
            printf("error writing save file ret %d errno %d\n", ret, errno);
            return false;
        }
#else // SPEED_BASE_AI
*/
        ret = fscanf(f, "%u %u %u %u %u %u %u %d %f %f %f\n",
                      &starter->competitor->lastTime,
                      &starter->competitor->lastPenalityTime,
                      &starter->competitor->globalTime,
                      &starter->competitor->globalPenalityTime,
                      &starter->startingCD,
                      &starter->startTime,
                      &starter->finishTime,
                      &starter->nextPoint,
                      &starter->currentPos.X,
                      &starter->currentPos.Y,
                      &starter->passedDistance
                      );
        if (ret < 11)
        {
            printf("error writing save file ret %d errno %d\n", ret, errno);
            return false;
        }
/*
#endif // SPEED_BASE_AI
*/
        starters.push_back(starter);
        //printf("starter->finishTime: %u\n", starter->finishTime);
        //if (starter->finishTime!=0) insertIntoFinishedState(starter->competitor);
        if (starter->competitor->lastTime!=0) insertIntoFinishedState(starter->competitor);
    }
    return true;
}

void CRaceEngine::refreshBigTerrain(BigTerrain* p_bigTerrain)
{
    m_bigTerrain = p_bigTerrain;
    for (int i = 0; i < starters.size(); i++)
    {
        starters[i]->m_bigTerrain = m_bigTerrain;
        starters[i]->calculateTo(m_bigTerrain->getAIPoints()[starters[i]->nextPoint]->getPosition());
    }
}

void CRaceEngine::updateShowNames()
{
    for (int i = 0; i < starters.size(); i++)
    {
        if (starters[i]->visible)
        {
            starters[i]->nameText->setVisible(/*settings*/show_names);
        }
    }
}
