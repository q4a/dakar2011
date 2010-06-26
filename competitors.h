/****************************************************************
*                                                               *
*    Name: competitors.h                                        *
*                                                               *
*    Creator: Balazs Tuska                                      *
*                                                               *
*    Description:                                               *
*                                                               *
****************************************************************/

#ifndef __COMPETITORS_H__
#define __COMPETITORS_H__

#include "irrlicht.h"
#include "NewtonRaceCar.h"
#include "BigTerrain.h"
#include <Newton.h>
#include "MyList.h"
#include "CQuad.h"

// Irrlicht Namespaces
using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

class SCompetitor
{
public:
    SCompetitor() {}
    SCompetitor(int p_num, const char* p_name, const char* p_coName, const char* p_teamName,
                int p_carIndex, int p_strength, bool p_ai);
    
    core::stringw getName();
    core::stringw getCoName();
    core::stringw getTeamName();
/*
    void setName(const core::stringw& newName);
    void setCoName(const core::stringw& newCoName);
    void setTeamName(const core::stringw& newTeamName);
*/
    int num;    
    char name[256];
    char coName[256];
    char teamName[256];
    int carIndex;
    int strength;
    bool ai;
    unsigned int lastTime;
    unsigned int globalTime;
    unsigned int lastPenalityTime;
    unsigned int globalPenalityTime;
};

extern core::array<SCompetitor*> competitors;

void loadCompetitors(const char* name);
void destroyCompetitors();

#endif // __COMPETITORS_H__
