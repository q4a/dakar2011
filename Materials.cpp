/****************************************************************
*                                                               *
*    Name: Materials.cpp                                        *
*                                                               *
*    Creator: Balazs Tuska                                      *
*                                                               *
*    Description:                                               *
*       This file contains the materials. Play sound when the   *
*       car contact to some object.                             *
*                                                               *
****************************************************************/

#include "Materials.h"
#include "dVector.h"
#include "gameplay.h"
#include "effects.h"
#include "MyList.h"
#include <assert.h>

#include "linux_includes.h"

int levelID = 1;
int vehicleID = 2;
int treeID = 3;
int roadID = 4;
int tireID = 5;
//irrklang::ISoundEngine* se = 0;

#ifdef USE_MY_SOUNDENGINE
CMySound* puffVTSound = 0;
#else
using namespace irrklang;
irrklang::ISound* puffVTSound = 0;
#endif
/*
struct collIdPair
{
    int id1;
    int id2;
    
};
CMyList<collIdPair*> collIdPairs;
*/
void vehicleContactProcess (const NewtonJoint* contactJoint, dFloat timestep, int threadIndex)
{
    int i = 0;
    dVector pos;
    NewtonBody* body0;
    NewtonBody* body1;
	dFloat max_speed = 0.5f;
	vector3df point;
	vector3df normal;
	

    body0 = NewtonJointGetBody0(contactJoint);
    body1 = NewtonJointGetBody1(contactJoint);

	//dprintf(printf("vehicleContactProcess() cb %p b0 %p b1 %p\n", car->GetRigidBody(), body0, body1);)
	//assert(0);
    
	for (void* contact = NewtonContactJointGetFirstContact (contactJoint); contact; contact = NewtonContactJointGetNextContact (contactJoint, contact),i++)
    {
		dFloat speed;
		NewtonMaterial* material;

		material = NewtonContactGetMaterial (contact);
		speed = fabsf(NewtonMaterialGetContactNormalSpeed(material));
		
		if (speed > max_speed)
		{
            max_speed = speed;
    		NewtonMaterialGetContactPositionAndNormal (material, &point.X, &normal.X);
        }
	}
	if (max_speed > 1.0f)
	{
        float soundVolume;
        if (max_speed<80.f)
        {
            soundVolume = max_speed/80.f;
        } else
        {
            soundVolume = 1.0f;
        }
        if (puffVTSound)
#ifdef USE_MY_SOUNDENGINE
           puffVTSound->setVolume(soundVolume);
#else
           puffVTSound->getSoundSource()->setDefaultVolume(soundVolume);
#endif
        car->playSound(puffVTSound, point);

        if (max_speed > 5.f)
        {
            NewtonRaceCar* collcar = 0;
            
            if (NewtonBodyGetMaterialGroupID(body0)==vehicleID)
            {
            	//dVector veloc;
        	    //NewtonBodyGetVelocity(body0, &veloc[0]);
                dprintf(printf("collision 0\n"));
                collcar = (NewtonRaceCar*)NewtonBodyGetUserData(body0);
/*
                collcar->collide(vector3df(point.m_x,point.m_y,point.m_z),
//                                 vector3df(dir0.m_x,dir0.m_y,dir0.m_z),
                                 vector3df(normal.m_x,normal.m_y,normal.m_z),
//                                 vector3df(veloc.m_x,veloc.m_y,veloc.m_z),
                                 max_speed);
*/
                collcar->collide(point,
                                 normal,
//                                 vector3df(veloc.m_x,veloc.m_y,veloc.m_z),
                                 max_speed);
                collcar = 0;
            }

            if (NewtonBodyGetMaterialGroupID(body1)==vehicleID)
            {
            	//dVector veloc;
        	    //NewtonBodyGetVelocity(body1, &veloc[0]);
                dprintf(printf("collision 1\n"));
                normal.invert();
                collcar = (NewtonRaceCar*)NewtonBodyGetUserData(body1);
                collcar->collide(point,
                                 normal,
//                                 vector3df(veloc.m_x,veloc.m_y,veloc.m_z),
                                 max_speed);

                collcar = 0;
            }
            
            if (max_speed > 20.f && (car->GetRigidBody()==body0 || car->GetRigidBody()==body1))
            {
                //printf("update collision effect true\n");
                coll_start_time = tick;
                update_collision_effect = true;
            }
        }
    }
	//printf("CONTACT  %d\n", i);
}

void vehicleLevelContactProcess (const NewtonJoint* contactJoint, dFloat timestep, int threadIndex)
{
    int i = 0;
    dVector pos;
    NewtonBody* body0;
    NewtonBody* body1;
	dFloat max_speed = 0.5f;
	vector3df point;
	vector3df normal;

	//dprintf(printf("vehicleLevelContactProcess()\n");)
	//assert(0);

    body0 = NewtonJointGetBody0(contactJoint);
    body1 = NewtonJointGetBody1(contactJoint);
    
	for (void* contact = NewtonContactJointGetFirstContact (contactJoint); contact; contact = NewtonContactJointGetNextContact (contactJoint, contact),i++)
    {
		dFloat speed;
		NewtonMaterial* material;

		material = NewtonContactGetMaterial (contact);
		speed = fabsf(NewtonMaterialGetContactNormalSpeed(material));
		
		if (speed > max_speed)
		{
            max_speed = speed;
    		NewtonMaterialGetContactPositionAndNormal (material, &point.X, &normal.X);
        }
	}
	if (max_speed > 10.0f)
	{
/*
        float soundVolume;
        if (max_speed<80)
        {
            soundVolume = max_speed/80;
        } else
        {
            soundVolume = 1.0f;
        }
        if (puffVTSound)
           puffVTSound->getSoundSource()->setDefaultVolume(soundVolume);
*/
//        if (max_speed > 15.f)
//        {
            NewtonRaceCar* collcar = 0;
            
            if (NewtonBodyGetMaterialGroupID(body0)==vehicleID)
            {
            	//dVector veloc;
        	    //NewtonBodyGetVelocity(body0, &veloc[0]);
                collcar = (NewtonRaceCar*)NewtonBodyGetUserData(body0);
                collcar->collide(point,
                                 normal,
//                                 vector3df(veloc.m_x,veloc.m_y,veloc.m_z),
                                 max_speed, 0.05f, 0.3f);
/*
                collcar->collide(vector3df(point.m_x,point.m_y,point.m_z),
//                                 vector3df(dir0.m_x,dir0.m_y,dir0.m_z),
                                 vector3df(normal.m_x,normal.m_y,normal.m_z),
//                                 vector3df(veloc.m_x,veloc.m_y,veloc.m_z),
                                 max_speed);
*/
                collcar = 0;
            }

            if (NewtonBodyGetMaterialGroupID(body1)==vehicleID)
            {
            	//dVector veloc;
        	    //NewtonBodyGetVelocity(body1, &veloc[0]);
                normal.invert();
                collcar = (NewtonRaceCar*)NewtonBodyGetUserData(body1);
                collcar->collide(point,
                                 normal,
//                                 vector3df(veloc.m_x,veloc.m_y,veloc.m_z),
                                 max_speed, 0.05f, 0.3f);
                collcar = 0;
            }
            
//        }
    }
//    car->playSound(puffVTSound, core::vector3df(point.m_x,point.m_y,point.m_z));
	//printf("CONTACT  %d\n", i);
}

void vehicleRoadContactProcess(const NewtonJoint* contactJoint, dFloat timestep, int threadIndex)
{
    int i = 0;
    dVector pos;
    NewtonBody* body0;
    NewtonBody* body1;
	dFloat max_speed = 0.5f;
	vector3df point;
	vector3df normal;

	dprintf(printf("vehicleRoadContactProcess()\n");)
	//assert(0);
}

// initialize all material and material interactions
void SetupMaterials (NewtonWorld* nWorld,
#ifdef USE_MY_SOUNDENGINE
                     CMySoundEngine* soundEngine
#else
                     irrklang::ISoundEngine* soundEngine
#endif
                     )
{
	int defaultID;

	// get the default material ID
	defaultID = NewtonMaterialGetDefaultGroupID (nWorld);

	// set default material properties
	NewtonMaterialSetDefaultSoftness (nWorld, defaultID, defaultID, 0.05f);
	NewtonMaterialSetDefaultElasticity (nWorld, defaultID, defaultID, 0.4f);
	NewtonMaterialSetDefaultCollidable (nWorld, defaultID, defaultID, 1);
	NewtonMaterialSetDefaultFriction (nWorld, defaultID, defaultID, 1.0f, 0.5f);
	NewtonMaterialSetCollisionCallback (nWorld, defaultID, defaultID, 0, 0, 0 /*GenericContactProcess/*, NULL*/);
	
#ifdef USE_MY_SOUNDENGINE
    puffVTSound = soundEngine->play3D("data/materials/sound/puffvt.wav", vector3df(0.0f,0.0f,0.0f), false, true, true);
#else
    puffVTSound = soundEngine->play3D("data/materials/sound/puffvt.wav", vec3df(0.0f,0.0f,0.0f), false, true, true);
#endif
    if (puffVTSound)
    {
#ifdef USE_MY_SOUNDENGINE
        puffVTSound->setVolume(0.2f);
        puffVTSound->setMinDistance(3.0f);
#else
        puffVTSound->getSoundSource()->setDefaultVolume(0.2f);
        puffVTSound->getSoundSource()->setDefaultMinDistance(3.0f);
#endif
    }

	// create all materials ID
	levelID = NewtonMaterialCreateGroupID(nWorld);
	vehicleID = NewtonMaterialCreateGroupID(nWorld);
	treeID = NewtonMaterialCreateGroupID(nWorld);
	roadID = NewtonMaterialCreateGroupID(nWorld);

	// set the material properties for vehicle on level
	NewtonMaterialSetDefaultElasticity (nWorld, levelID, vehicleID, 0.1f);
    NewtonMaterialSetDefaultFriction (nWorld, levelID, vehicleID, 0.3f, 0.3f);
//	NewtonMaterialSetDefaultFriction (nWorld, levelID, vehicleID, 0.9f, 0.7f);
	NewtonMaterialSetCollisionCallback (nWorld, levelID, vehicleID, 
      0, 0, vehicleLevelContactProcess/*NULL, NULL*/);
	NewtonMaterialSetCollisionCallback (nWorld, treeID, vehicleID, 
      0, 0, vehicleContactProcess/*NULL, NULL*/);
	NewtonMaterialSetCollisionCallback (nWorld, vehicleID, vehicleID, 
      0, 0, vehicleContactProcess/*NULL, NULL*/);
//     &vehicle_level, GenericContactBegin, VehicleContactProcess, GenericContactEnd); 
//    NewtonMaterialSetDefaultFriction (nWorld, defaultID, levelID, 1.2f, 1.2f);
//    NewtonMaterialSetCollisionCallback (nWorld, defaultID, levelID, 0, 0, GenericContactProcess);
// no need yet
//    NewtonMaterialSetCollisionCallback (nWorld, defaultID, roadID, 0, 0, vehicleRoadContactProcess);
}



// destroy all material resources
void CleanUpMaterials (NewtonWorld* nWorld)
{
    if (puffVTSound)
    {
#ifdef USE_MY_SOUNDENGINE
        delete puffVTSound;
#else
        puffVTSound->drop();
#endif
        puffVTSound = 0;
    }
}
