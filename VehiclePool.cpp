/****************************************************************
*                                                               *
*    Name: VehiclePool.cpp                                      *
*                                                               *
*    Creator: Balazs Tuska                                      *
*                                                               *
*    Description:                                               *
*                                                               *
****************************************************************/

#include "VehiclePool.h"
#include "NewtonRaceCar.h"
#include "message.h"

#ifdef MY_DEBUG
#define dprintf(x) x
#else
#define dprintf(x)
#endif

#ifdef MY_PDEBUG
#define pdprintf(x) x
#else
#define pdprintf(x)
#endif

//#define pdprintf(x) x

#define PREGENERATED_VEHICLE_NUM 2

CVehicleType::CVehicleType(
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
                 int rep)
    : device(p_device), smgr(p_smgr), driver(p_driver), nWorld(p_nWorld),
      soundEngine(p_soundEngine),
      vehicleList(), vehicleCollision(0), vehicleCollisionBox(0),
      omb(0)
{
    strcpy(vehicleName, p_name);
    strcpy(vehicleFileName, p_fileName);
    
    for (int i = 0; i < rep; i++)
    {
        NewtonRaceCar* vehicle = new NewtonRaceCar(device, smgr, driver, nWorld, soundEngine,
                                                   vehicleCollision, vehicleCollisionBox, omb,
                                                   type, vehicleFileName);
        vehicle->pause();
        vehicleList.push_back(vehicle);
        MessageText::refresh();
    }
}

CVehicleType::~CVehicleType()
{
    for (int i = 0; i < vehicleList.size(); i++)
    {
        if (vehicleList[i])
        {
            delete vehicleList[i];
            vehicleList[i] = 0;
        }
    }
    vehicleList.clear();
    if (vehicleCollision)
    {
        NewtonReleaseCollision (nWorld, vehicleCollision);
        vehicleCollision = 0;
    }
    if (vehicleCollisionBox)
    {
        NewtonReleaseCollision (nWorld, vehicleCollisionBox);
        vehicleCollisionBox = 0;
    }
    if (omb)
    {
        delete [] omb;
        omb = 0;
    }
}



CVehiclePool::CVehiclePool(IrrlichtDevice* p_device,
                           ISceneManager* p_smgr,
                           IVideoDriver* p_driver,
                           NewtonWorld* p_nWorld,
#ifdef USE_MY_SOUNDENGINE
                           CMySoundEngine* p_soundEngine,
#else
                           irrklang::ISoundEngine* p_soundEngine,
#endif
                           const char *p_name)
    : device(p_device), smgr(p_smgr), driver(p_driver), nWorld(p_nWorld),
      soundEngine(p_soundEngine),
      vehicleTypes(), activeVehicles()
{
	do {
        FILE* f;
        int ret = 1;
        char vname[256];
        char vfname[256];
        int rep = PREGENERATED_VEHICLE_NUM;
        
        f = fopen(p_name, "r");

        if (!f)
        {
            break;
        }

        while (1)
        {
            ret = fscanf(f, "%s\n%s\n%d\n", vname, vfname, &rep);
            if (ret <= 0) break;
            if (rep < 2 ) rep = 2;
            CVehicleType* vt = new CVehicleType(device, smgr, driver, nWorld, soundEngine,
                                                vname, vfname, vehicleTypes.size(), rep);
            
            vehicleTypes.push_back(vt);
            MessageText::refresh();
        }
        fclose(f);
    } while (0);
}

CVehiclePool::~CVehiclePool()
{
    for (int i = 0; i < vehicleTypes.size(); i++)
    {
        if (vehicleTypes[i])
        {
            delete vehicleTypes[i];
            vehicleTypes[i] = 0;
        }
    }
    vehicleTypes.clear();

    for (int i = 0; i < activeVehicles.size(); i++)
    {
        if (activeVehicles[i])
        {
            delete activeVehicles[i];
            activeVehicles[i] = 0;
        }
    }
    activeVehicles.clear();
}

NewtonRaceCar* CVehiclePool::getVehicle(const int type)
{
    dprintf(printf("getVehicle() type %d\n", type);)
    if (type < 0 || type >= vehicleTypes.size() || vehicleTypes[type]==0 ||
        vehicleTypes[type]->vehicleList.size() <= 0) return 0;
    
    dprintf(printf("getVehicle() size %d\n", vehicleTypes[type]->vehicleList.size());)
    for (int i = 0; i < vehicleTypes[type]->vehicleList.size(); i++)
    {
        dprintf(printf("%d. %p\n", i, vehicleTypes[type]->vehicleList[i]);)
    }
    int ind = vehicleTypes[type]->vehicleList.size() - 1;
    NewtonRaceCar* vehicle = vehicleTypes[type]->vehicleList[ind];
    dprintf(printf("getVehicle() vehicle 1 %p\n", vehicle);)
    
    vehicleTypes[type]->vehicleList[ind] = 0;
    vehicleTypes[type]->vehicleList.erase(ind);

    dprintf(printf("getVehicle() vehicle 2 %p\n", vehicle);)
    
    activeVehicles.push_back(vehicle);

    dprintf(printf("getVehicle() vehicle 3 %p\n", vehicle);)
    
    return vehicle;
}

void CVehiclePool::putVehicle(NewtonRaceCar* vehicle)
{
    if (!vehicle) return;
    
    for (int i = 0; i < activeVehicles.size(); i++)
    {
        if (activeVehicles[i]==vehicle)
        {
            activeVehicles.erase(i);
            break;
        }
    }
    vehicle->deactivate();
    const int type = vehicle->getCarType();
    if (type >= 0 && type < vehicleTypes.size())
    {
        vehicleTypes[type]->vehicleList.push_back(vehicle);
    }
}

void CVehiclePool::updateActiveVehicles()
{
    pdprintf(printf("update actives: %d\n", activeVehicles.size());)
    for (int i = 0; i < activeVehicles.size(); i++)
    {
        pdprintf(printf("update actives %d. %p\n", i, activeVehicles[i]);)
        if (activeVehicles[i])
        {
            pdprintf(printf("update actives %d. brake\n", i);)
            if (activeVehicles[i]->getBrake()>0.001f)
                activeVehicles[i]->applyHandBrakes(activeVehicles[i]->getBrake());
            pdprintf(printf("update actives %d. torque\n", i);)
            activeVehicles[i]->applyTireTorque(activeVehicles[i]->getTorqueReal());
            pdprintf(printf("update actives %d. steer\n", i);)
            //if (fabsf(activeVehicles[i]->getSteer())>0.001f)
            activeVehicles[i]->applySteering(activeVehicles[i]->getSteer());
            pdprintf(printf("update actives %d. gear\n", i);)
            if (activeVehicles[i]->getAutoGear())
                activeVehicles[i]->updateGear();
        }
    }
    pdprintf(printf("update actives end: %d\n", activeVehicles.size());)
}

void CVehiclePool::pauseActiveVehicles()
{
    for (int i = 0; i < activeVehicles.size(); i++)
    {
        if (activeVehicles[i])
        {
            activeVehicles[i]->pause();
        }
    }
}

void CVehiclePool::resumeActiveVehicles()
{
    for (int i = 0; i < activeVehicles.size(); i++)
    {
        if (activeVehicles[i])
        {
            activeVehicles[i]->resume();
        }
    }
}

void CVehiclePool::flushActiveVehicles()
{
    for (int i = 0; i < activeVehicles.size(); i++)
    {
        if (activeVehicles[i])
        {
            const int type = activeVehicles[i]->getCarType();
            if (type >= 0 && type < vehicleTypes.size())
            {
                vehicleTypes[type]->vehicleList.push_back(activeVehicles[i]);
            }
            activeVehicles[i]->deactivate();
            activeVehicles[i] = 0;
        }
    }
    activeVehicles.clear();
}

core::stringw CVehiclePool::getName(const int type)
{
    core::stringw str = L"";
    if (type < 0 || type >= vehicleTypes.size() || vehicleTypes[type]==0) return str;

    str += vehicleTypes[type]->vehicleName;
    str.replace(L'_', L' ');
    return str;
}
