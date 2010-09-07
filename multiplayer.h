/****************************************************************
*                                                               *
*    Name: multiplayer.h                                        *
*                                                               *
*    Creator: Balazs Tuska                                      *
*                                                               *
*    Description:                                               *
*       This file contains the code of the multiplayer part of  *
*       the game.                                               *
*                                                               *
****************************************************************/
#ifdef USE_MULTIPLAYER
#ifndef __MULTIPLAYER_H__
#define __MULTIPLAYER_H__

#include <sys/time.h>
#include <time.h>
#include <sys/types.h>

#ifdef __linux__
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#else
#include <winsock2.h>
#endif

#define CAR_NUM 10
#define DATA_LENGTH (1 + sizeof(SCarData))
#define INITIAL_DATA_LENGTH (1 + sizeof(int))
#define TIMEOUT 5
#define MYPORT 22010

struct SCarData
{
    int num;
    int type;
    float matrix[16];
    float force[3];
    float torque;
    float steer;
    float brake;
};

#ifndef __my_server__
#include "irrlicht.h"
// Irrlicht Namespaces
using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;
#include "NewtonRaceCar.h"

u32 getSec();

extern bool isMultiplayer;

void connectToServer(irr::IrrlichtDevice* pdevice,
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
void updateConnection();
void updateOtherDatas();
void disconnectFromServer(bool putPool);
void leaveStageToServer();

struct SCarDataWrapper
{
    int time;
    NewtonRaceCar* car;    
    struct SCarData carData;
};

#else
struct SCarDataWrapper
{
    int time;
    struct sockaddr_in their_addr;
    struct SCarData carData;
};

time_t getSec()
{
   timeval tv;
   gettimeofday(&tv, 0);
   return tv.tv_sec;
//   return ((unsigned long long)tv.tv_sec * 1000000) | ((unsigned long long)tv.tv_usec);
}
#endif

#endif // __MULTIPLAYER_H__
#endif // USE_MULTIPLAYER
