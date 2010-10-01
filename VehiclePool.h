/****************************************************************
*                                                               *
*    Name: VehiclePool.h                                        *
*                                                               *
*    Creator: Balazs Tuska                                      *
*                                                               *
*    Description:                                               *
*                                                               *
****************************************************************/

#ifndef __VEHICLEPOOL_H__
#define __VEHICLEPOOL_H__

#include "irrlicht.h"
#include <Newton.h>

// Irrlicht Namespaces
using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

#ifdef USE_MY_SOUNDENGINE
 #include "mySound.h"
#else
 #include <irrKlang.h>
 using namespace irrklang;
#endif

class NewtonRaceCar;

class CVehicleType
{
public:
    CVehicleType(
                 IrrlichtDevice* p_device,
                 ISceneManager* p_smgr,
                 IVideoDriver* p_driver,
                 NewtonWorld* p_nWorld,
#ifdef USE_MY_SOUNDENGINE
                 CMySoundEngine* p_soundEngine,
#else
                 irrklang::ISoundEngine* p_soundEngine,
#endif
                 const char *p_name,
                 const char *p_fileName,
                 int type,
                 int rep);
    ~CVehicleType();
    
    char vehicleName[256];
    char vehicleFileName[256];
    core::array<NewtonRaceCar*> vehicleList;

	IrrlichtDevice* device;
    ISceneManager* smgr;
    IVideoDriver* driver;
	NewtonWorld* nWorld;
#ifdef USE_MY_SOUNDENGINE
    CMySoundEngine* soundEngine;
#else
    irrklang::ISoundEngine* soundEngine;
#endif
	NewtonCollision* vehicleCollision;
	NewtonCollision* vehicleCollisionBox;
    char* omb;
};



class CVehiclePool
{
public:
    CVehiclePool(IrrlichtDevice* p_device,
                 ISceneManager* p_smgr,
                 IVideoDriver* p_driver,
                 NewtonWorld* p_nWorld,
#ifdef USE_MY_SOUNDENGINE
                 CMySoundEngine* p_soundEngine,
#else
                 irrklang::ISoundEngine* p_soundEngine,
#endif
                 const char *p_name);
    ~CVehiclePool();
    
    int getVehicleTypesSize() {return vehicleTypes.size();}
    
    NewtonRaceCar* getVehicle(const int type);
    void putVehicle(NewtonRaceCar* vehicle);
    void updateActiveVehicles();
    void pauseActiveVehicles();
    void resumeActiveVehicles();
    void flushActiveVehicles();
    core::stringw getName(const int type);
    
private:
	IrrlichtDevice* device;
    ISceneManager* smgr;
    IVideoDriver* driver;
	NewtonWorld* nWorld;
#ifdef USE_MY_SOUNDENGINE
    CMySoundEngine* soundEngine;
#else
    irrklang::ISoundEngine* soundEngine;
#endif
	
	core::array<CVehicleType*> vehicleTypes;
	core::array<NewtonRaceCar*> activeVehicles;
};

#endif // __VEHICLEPOOL_H__
