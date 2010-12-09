/****************************************************************
*                                                               *
*    Name: competitors.cpp                                      *
*                                                               *
*    Creator: Balazs Tuska                                      *
*                                                               *
*    Description:                                               *
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

#include "competitors.h"
#include "RaceEngine.h"

#ifdef __linux__
#include "linux_includes.h"
#endif

#ifdef MY_DEBUG
# define dprintf(x) x
#else
# define dprintf(x)
#endif /* debug */

core::array<SCompetitor*> competitors;

SCompetitor::SCompetitor(int p_num, const char* p_name, const char* p_coName,
                         const char* p_carName, const char* p_teamName,
                         int p_carIndex, int p_strength, bool p_ai)
    : lastTime(0), globalTime(0), lastPenalityTime(0), globalPenalityTime(0)
{
    num = p_num;
    strcpy(name, p_name);
    strcpy(coName, p_coName);
    strcpy(carName, p_carName);
    strcpy(teamName, p_teamName);
    carIndex = p_carIndex;
    strength = p_strength;
    ai = p_ai;
}

core::stringw SCompetitor::getName()
{
    core::stringw str = L" ";
    str = name;
    str.replace(L'_', L' ');
    return str;
}

core::stringw SCompetitor::getCoName()
{
    core::stringw str = L" ";
    str = coName;
    str.replace(L'_', L' ');
    return str;
}

core::stringw SCompetitor::getCarName()
{
    core::stringw str = L" ";
    str = carName;
    str.replace(L'_', L' ');
    return str;
}

core::stringw SCompetitor::getTeamName()
{
    core::stringw str = L" ";
    str = teamName;
    str.replace(L'_', L' ');
    return str;
}

void loadCompetitors(const char* fname)
{
    FILE* f;
    int ret = 1;
    int num;
    char name[256];
    char coName[256];
    char carName[256];
    char teamName[256];
    int carIndex;
    int strength;
    
    if (competitors.size()>0)
    {
        printf("Do not read competitor file, because of existing competitors: %s\n", fname);
        return;
    }
    
    dprintf(printf("open competitor file: %s\n", fname);)
    
    f = fopen(fname, "r");

    if (!f)
    {
        printf("unable to open competitor file: %s\n", fname);
        return;
    }

    while (1)
    {
        ret = fscanf(f, "%d %s %s %s %s %d %d\n", &num, name, coName, carName, teamName, &carIndex, &strength);
        if (ret < 7) break;
        SCompetitor* comp = new SCompetitor(num, name, coName, carName, teamName, carIndex, strength, true);
        competitors.push_back(comp);
    }
    fclose(f);
    
    CRaceEngine::getRaceState() = competitors;
    
    dprintf(printf("%d competitors read\n", competitors.size());)
}

void destroyCompetitors()
{
    for (int i = 0; i < competitors.size(); i++)
    {
        if (competitors[i])
        {
            delete competitors[i];
            competitors[i] = 0;
        }
    }
    competitors.clear();
}
