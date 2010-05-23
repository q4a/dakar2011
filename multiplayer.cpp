/****************************************************************
*                                                               *
*    Name: multiplayer.cpp                                      *
*                                                               *
*    Creator: Balazs Tuska                                      *
*                                                               *
*    Description:                                               *
*       This file contains the code of the multiplayer part of  *
*       the game.                                               *
*                                                               *
****************************************************************/

#include <stdlib.h>
#include <unistd.h>
//#include <string.h>
#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#ifdef __linux__
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#define ERRNO errno
#define NOWAIT MSG_DONTWAIT
#else
#include <winsock2.h>
#define ERRNO WSAGetLastError()
#define NOWAIT 0
#endif
#include "multiplayer.h"
#include "settings.h"
#include "gameplay.h"
#include "message.h"
#include "my_shaders.h"

volatile SCarDataWrapper* carDataWrappers[CAR_NUM] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
volatile int multi_num = -1;
volatile int initial_message_sent = 0;

volatile int connected = 0; // connected to server 1 , 2 if recved initial data
volatile int sock = 0;
volatile struct sockaddr_in server_sa;

bool isMultiplayer = false;

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

int tryConnect(char* addr, int port)
{
    if (connected) return 1;

#ifndef __linux__
    WSADATA wsaData; 
    //WSAData wsaData;

    if (WSAStartup(MAKEWORD(1, 1), &wsaData) != 0)
    {
        printf("WSAStartup failed.\n");
        return 0;
    }
#endif
    sock = socket(AF_INET,SOCK_DGRAM,0);

    if(sock<0)
    {
        printf("+++++++++ socket failed: ERRNO: %d \n", ERRNO);
        return 0;
    }

    memset((void*)&server_sa,0,sizeof(struct sockaddr_in));

    server_sa.sin_addr.s_addr = inet_addr(addr);
    server_sa.sin_family = AF_INET;
    server_sa.sin_port = htons(port);

    if (connect(sock, (struct sockaddr *)&server_sa,sizeof(struct sockaddr)) < 0) {
        printf("+++++++++ ds_connect failed: ERRNO: %d\n", ERRNO);
        return 0;
    }

#ifdef __linux__
    unsigned long flags = 1;
    if (-1 == (flags = fcntl(sock, F_GETFL, 0)))
        flags = 0;
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);
#else           
    unsigned long flags = 1;
    ioctlsocket(sock, FIONBIO, &flags);
#endif
    connected = 1;

    printf("+++++++++ connect connected to %s:%d\n", addr, port);
    return 1;
}

int disconnect()
{
#ifdef __linux__
    close(sock);
#else
    closesocket(sock);
    WSACleanup();
#endif
    connected = 0;
    sock = 0;
    initial_message_sent = 0;
}

int sendMyData()
{
    char data[DATA_LENGTH+sizeof(int)];
    SCarData* my_carData = (SCarData*)(data+1+sizeof(int));
    int sent;
    if (multi_num < 0 || multi_num >= CAR_NUM || connected != 2) return 0;


    data[0] = 'u';
    memcpy(data+1, (char*)&oldStage, sizeof(int));
    my_carData->brake=car->getBrake();
    my_carData->num=multi_num;
    my_carData->steer=car->getSteer();
    my_carData->torque=car->getTorqueReal();
    memcpy(&my_carData->matrix, car->getMatrix().pointer(), 16*sizeof(float));
    my_carData->type=car->getCarType();
/*
// uf
    float force[3];
	float Ixx;
	float Iyy;
	float Izz;
	float mass;

    NewtonBodyGetForce(car->GetRigidBody(), force);
    NewtonBodyGetMassMatrix(car->GetRigidBody(), &mass, &Ixx, &Iyy, &Izz);
    force[1] -= (mass * GRAVITY);
    memcpy(&my_carData->force, force, 3*sizeof(float));
*/    
    //memcpy(data+1+sizeof(int), (char*)&my_carData, sizeof(SCarData));
    sent = send(sock,data,DATA_LENGTH+sizeof(int),NOWAIT);
    if( sent < (int)(DATA_LENGTH+sizeof(int)) )
    {
        printf("+++++++++ sending failed, tried %d  sent bytes %d, ERRNO: %d\n", DATA_LENGTH+sizeof(int), sent, ERRNO);
        return 0;
    }
    //printf("send update date okay\n");
    return 1;
}

void updateOneCarData(int i);

int recvOtherDatas()
{
    char data[DATA_LENGTH];
    int sent;

    if (connected != 2) return 0;

    data[0] = 'u';
    //sent = recv(sock,data,DATA_LENGTH,NOWAIT);
    while( (sent = recv(sock,data,DATA_LENGTH,NOWAIT)) > 0 )
    {
        if (sent != (int)DATA_LENGTH)
        {
            if (sent==3) // remove message
            {
                if (data[0]=='d' && data[1]=='d' && data[2]=='d')
                {
                    connected = 1;
                    initial_message_sent = 0; // resend init
                    multi_num = -1;
                    return 0;
                }
            }
            else
            if (sent==5) // re new discard it
            {
                continue;
            }

            printf("revd other datas okay but size mismatch %d != %d\n", sent, DATA_LENGTH);
            //sent = recv(sock,data,DATA_LENGTH,NOWAIT);
            continue;
        }
        if( data[0]!='u')
        {
            printf("+++ recv other datas size okay but char not u != %c", data[0]);
            //sent = recv(sock,data,DATA_LENGTH,NOWAIT);
            continue;
        }
        SCarData* carData = (SCarData*)(data + 1);
        if (carData->num < 0 || carData->num >= CAR_NUM || carData->type < 0)
        {
            printf("+++ recv other datas size okay num, or type is not okay", carData->num, carData->type);
            //sent = recv(sock,data,DATA_LENGTH,NOWAIT);
            continue;
        }
        if (carDataWrappers[carData->num]==0)
        {
                                            
            carDataWrappers[carData->num] = new SCarDataWrapper;
            
            if (carData->type>=vehiclePool->getVehicleTypesSize() || carData->type<0) carData->type=0;
            
            matrix4 mat;
            mat.setM(carData->matrix);
            core::vector3df pos = mat.getTranslation();
            core::vector3df rot = mat.getRotationDegrees();
            /*
            char carFileName[256];
            strcpy(carFileName, "data/vehicles/");
            strcat(carFileName, carList[carData->type]->carFileName);
            */
            carDataWrappers[carData->num]->car = vehiclePool->getVehicle(carData->type);
            if (!carDataWrappers[carData->num]->car)
            {
                printf("no more free car in the pool for the multiplayer car\n");
                delete carDataWrappers[carData->num];
                carDataWrappers[carData->num] = 0;
                continue;
            }
            carDataWrappers[carData->num]->car->activate(
                 pos,
                 rot,
                 bigTerrain->getGroundSoundName(), bigTerrain->getPuffSoundName(),
                 bigTerrain->getSkidSoundName(),
                 bigTerrain->getFrictionMulti(),
                 skydome,
                 shadowMap,
                 bigTerrain->getWaterHeight());
            printf("add new car %d type %d\n", carData->num, carData->type);
            
            if (trace_net)
            {
                core::stringw str = L"";
                str += carData->num+1;
                str += L". car has joined to the game. (You are ";
                str += multi_num+1;
                str += L".)";
                MessageText::addText(str.c_str(), 5);
            }
        }
        memcpy((char*)&carDataWrappers[carData->num]->carData, carData, sizeof(SCarData));
        carDataWrappers[carData->num]->time = getSec();
        //uf NewtonBodySetForce(carDataWrappers[carData->num]->car->GetRigidBody(), (float*)carDataWrappers[carData->num]->carData.force);
        updateOneCarData(carData->num);
        //sent = recv(sock,data,DATA_LENGTH,NOWAIT);
    }
    return 1;
}

void updateOneCarData(int i)
{
    if (i<0 || i>CAR_NUM || carDataWrappers[i] == 0 || carDataWrappers[i]->car == 0) return;
    
    matrix4 mat;
    mat.setM((float*)(carDataWrappers[i]->carData.matrix));
    //core::vector3df pos = mat.getTranslation();
    //core::vector3df rot = mat.getRotationDegrees();
    core::vector3df rpos = mat.getTranslation();
    core::vector3df cpos = carDataWrappers[i]->car->getMatrix().getTranslation();

    //if (rpos.getDistanceFrom(cpos) > 0.6f)
        carDataWrappers[i]->car->setMatrixWithNB(mat);
        
    carDataWrappers[i]->car->setSteering(carDataWrappers[i]->carData.steer);
    carDataWrappers[i]->car->setTireTorque(carDataWrappers[i]->carData.torque);
    carDataWrappers[i]->car->setHandBrakes(carDataWrappers[i]->carData.brake);
    
    /*
    // it is done by vehiclePool->activeVehicles
    if (carDataWrappers[i]->car->getBrake()>0.f)
      carDataWrappers[i]->car->applyHandBrakes(carDataWrappers[i]->car->getBrake());
    carDataWrappers[i]->car->applyTireTorque(carDataWrappers[i]->car->getTorqueReal());
    carDataWrappers[i]->car->applySteering(carDataWrappers[i]->car->getSteer());
    */
//    NewtonBodySetForce(carDataWrappers[i]->car->GetRigidBody(), carDataWrappers[i]->carData.force);
}

void updateOtherDatas()
{
    if (!isMultiplayer) return;

    int i;
    time_t sec = getSec();

    for (i=0;i<CAR_NUM;i++)
    {
        if (carDataWrappers[i])
        {
            if (sec - carDataWrappers[i]->time > TIMEOUT)
            {
                printf("delete car %d TIMEOUT\n", i);
                
                if (trace_net)
                {
                    core::stringw str = L"";
                    str += i+1;
                    str += L". car has left the game. (You are ";
                    str += multi_num+1;
                    str += L".)";
                    MessageText::addText(str.c_str(), 5);
                }
                
                vehiclePool->putVehicle(carDataWrappers[i]->car);
                delete carDataWrappers[i];
                carDataWrappers[i] = 0;
            }
            else // yeah, we have a car update it
            {
                updateOneCarData(i);
            }
        }
    }
}

int sendInitialMessage()
{
    char data[INITIAL_DATA_LENGTH];
    int sent;
    if (connected != 1) return 0;
    if (initial_message_sent) return 1;

    data[0] = 'n';
    memcpy(data+1, (char*)&oldStage, sizeof(int));
    sent = send(sock,data,INITIAL_DATA_LENGTH,0);
    if( sent < (int)INITIAL_DATA_LENGTH )
    {
        printf("+++++++++ sending failed initial message, tried %d  sent bytes %d, ERRNO: %d\n", INITIAL_DATA_LENGTH, sent, ERRNO);
        return 0;
    }
    initial_message_sent = 1;
    printf("send initial message okay\n");
    return 1;
}

int recvInitialAck()
{
    char data[INITIAL_DATA_LENGTH];
    volatile int sent;

    if (connected != 1) return 0;
    if (initial_message_sent!=1) return 0;
    sent = recv(sock,data,INITIAL_DATA_LENGTH,NOWAIT);
    if( sent < (int)INITIAL_DATA_LENGTH )
    {
        printf("+++ recv initial message ack, tried %d  sent bytes %d, ERRNO: %d\n", INITIAL_DATA_LENGTH, sent, ERRNO);
        initial_message_sent = 0;
        return 0;
    }
    if (data[0] != 'n')
    {
        printf("+++ recv initial message ack ok, but nor n != %c\n", data[0]);
        initial_message_sent = 0;
        return 0;
    }

    memcpy((char*)&multi_num, data+1, sizeof(int));
    connected = 2;
    printf("recv initial message ack okay car %d recved %d\n", multi_num, sent);
    if (trace_net)
    {
        core::stringw str = L"Server accepted the connection. You are ";
        str += multi_num+1;
        str += L".";
        MessageText::addText(str.c_str(), 5);
    }
    return 1;
}

int sendEndMessage()
{
    char data[INITIAL_DATA_LENGTH+sizeof(int)];
    int sent;

    if (multi_num < 0 || multi_num >= CAR_NUM || connected == 0 || initial_message_sent == 0) return 0;

    data[0] = 'd';
    memcpy(data+1, (char*)&oldStage, sizeof(int));
    memcpy(data+1+sizeof(int), (char*)&multi_num, sizeof(int));

    multi_num = -1;
    initial_message_sent = 0;
    connected = 1;
    sent = send(sock,data,INITIAL_DATA_LENGTH+sizeof(int),0);
    if( sent < (int)(INITIAL_DATA_LENGTH+sizeof(int)) )
    {
        printf("+++++++++ sending failed initial message, tried %d  sent bytes %d, ERRNO: %d\n", INITIAL_DATA_LENGTH+sizeof(int), sent, ERRNO);
        return 0;
    }
    printf("send initial message okay\n");
    return 1;
}

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
)
{
    device = pdevice;
    driver = pdriver;
    smgr = psmgr;
    env = penv;
    nWorld = pnWorld;
    soundEngine = psoundEngine;
    isMultiplayer = true;
}

void updateConnection()
{
    int ret;
    
    if (!isMultiplayer) return;
    tryConnect(server_name, server_port);
    if (connected && car)
    {
        if (connected==1)
        {
            if (initial_message_sent == 0) sendInitialMessage();
            recvInitialAck();
        }
        if (connected==2)
        {
            ret = sendMyData();
            if (!ret)
                printf("send my data %d\n", ret);
            ret = recvOtherDatas();
            if (!ret)
                printf("recv other datas %d\n", ret);
            updateOtherDatas();
        }
    }
}

void disconnectFromServer(bool putPool)
{
    if (!isMultiplayer) return;
    if (connected)
    {
        sendEndMessage();
        disconnect();
        for (int i=0;i<CAR_NUM;i++)
        {
            if (carDataWrappers[i])
            {
                printf("delete car %d leave stage\n", i);
                if (putPool)
                    vehiclePool->putVehicle(carDataWrappers[i]->car);
                delete carDataWrappers[i];
                carDataWrappers[i] = 0;
            }
        }
    }
    isMultiplayer = false;
}

void leaveStageToServer()
{
    if (connected)
    {
        int i;
        sendEndMessage();
    
        for (i=0;i<CAR_NUM;i++)
        {
            if (carDataWrappers[i])
            {
                printf("delete car %d leave stage\n", i);
                //delete carDataWrappers[i]->car;
                delete carDataWrappers[i];
                carDataWrappers[i] = 0;
            }
        }
    }
}

u32 getSec()
{
    return device->getTimer()->getTime() / 1000;
}
