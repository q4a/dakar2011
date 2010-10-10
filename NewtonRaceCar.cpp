/****************************************************************
*                                                               *
*    Name: NewtonRaceCar.cpp                                    *
*                                                               *
*    Creator: Balazs Tuska                                      *
*                                                               *
*    Description:                                               *
*       This file contains the code of the car handling.        *
*       The car can be the user car or a multiplayer car.       *
*                                                               *
****************************************************************/

//#if 0
// NewtonRaceCar.cpp: implementation of the RaceCar class.
//
//////////////////////////////////////////////////////////////////////
//#include <stdafx.h>
#include "NewtonRaceCar.h"
#include "dVector.h"
#include "Materials.h"
#include <math.h>
#include <float.h>
#include "my_shaders.h"
#include "settings.h"
#include "gameplay.h"
#include "effects.h"
#include <assert.h>
#include "MyRoad.h"

#ifdef __linux__
#include "linux_includes.h"
#endif
/*
#ifdef MY_DEBUG
#define dprintf(x) x
#else
#define dprintf(x)
#endif
*/
//#define pdprintf(x) x

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#define MAX_STEER_ANGLE (30.0f * 3.1416f / 180.0f)
/*
#define JEEP_MASS					(900.0f)
#define JEEP_TIRE_MASS				(30.0f)
#define JEEP_CENTRE_OF_MASS_OFFSET	(0.4f)
#define JEEP_ENGINE_CENTRE_OF_MASS_OFFSET  (0.25f)
#define JEEP_SUSPENSION_LENGTH		(0.5f)
//#define JEEP_SUSPENSION_LENGTH		(0.2f)
#define JEEP_SUSPENSION_SPRING		(150.0f)
//#define JEEP_SUSPENSION_SPRING		(175.0f)
#define JEEP_SUSPENSION_DAMPER		(6.0f)
#define JEEP_FRICTION				1.5f
*/
#define CUSTOM_VEHICLE_JOINT_ID		0xF4ED

//#define GRAVITY	   -10.0f

#define SOUND_UPDATE_INTERVAL 20

//int vId = 0; // jeep
// vId = 1; // monstertruck  
// vId = 2; // f1
static IAnimatedMeshSceneNode* createBB(ISceneManager* smgr, const core::aabbox3d<f32> &bbox)
{
    IAnimatedMeshSceneNode* ret;
    SMeshBuffer* buffer = new SMeshBuffer();
    SMesh* mesh = new SMesh();
    s32 verInd;
    video::S3DVertex vtx;
    SColor color = SColor(255, 255,255,255);

    //printf("debug %d %f %f\n", i, traX, traZ);
    dprintf(printf("create bb min: %f %f %f\n", bbox.MinEdge.X, bbox.MinEdge.Y, bbox.MinEdge.Z));
    dprintf(printf("create bb max: %f %f %f\n", bbox.MaxEdge.X, bbox.MaxEdge.Y, bbox.MaxEdge.Z));
    dprintf(printf("create bb size: %f %f %f\n", bbox.MaxEdge.X - bbox.MinEdge.X,
                           bbox.MaxEdge.Y - bbox.MinEdge.Y, bbox.MaxEdge.Z - bbox.MinEdge.Z));

// 0
    vtx.Pos.X = bbox.MinEdge.X;
    vtx.Pos.Y = bbox.MinEdge.Y;
    vtx.Pos.Z = bbox.MinEdge.Z;
    vtx.Color = color;
    buffer->Vertices.push_back(vtx);
// 1
    vtx.Pos.X = bbox.MaxEdge.X;
    vtx.Pos.Y = bbox.MinEdge.Y;
    vtx.Pos.Z = bbox.MinEdge.Z;
    vtx.Color = color;
    buffer->Vertices.push_back(vtx);
// 2
    vtx.Pos.X = bbox.MaxEdge.X;
    vtx.Pos.Y = bbox.MaxEdge.Y;
    vtx.Pos.Z = bbox.MinEdge.Z;
    vtx.Color = color;
    buffer->Vertices.push_back(vtx);
// 3
    vtx.Pos.X = bbox.MinEdge.X;
    vtx.Pos.Y = bbox.MaxEdge.Y;
    vtx.Pos.Z = bbox.MinEdge.Z;
    vtx.Color = color;
    buffer->Vertices.push_back(vtx);
// 4
    vtx.Pos.X = bbox.MinEdge.X;
    vtx.Pos.Y = bbox.MinEdge.Y;
    vtx.Pos.Z = bbox.MaxEdge.Z;
    vtx.Color = color;
    buffer->Vertices.push_back(vtx);
// 5
    vtx.Pos.X = bbox.MaxEdge.X;
    vtx.Pos.Y = bbox.MinEdge.Y;
    vtx.Pos.Z = bbox.MaxEdge.Z;
    vtx.Color = color;
    buffer->Vertices.push_back(vtx);
// 6
    vtx.Pos.X = bbox.MaxEdge.X;
    vtx.Pos.Y = bbox.MaxEdge.Y;
    vtx.Pos.Z = bbox.MaxEdge.Z;
    vtx.Color = color;
    buffer->Vertices.push_back(vtx);
// 7
    vtx.Pos.X = bbox.MinEdge.X;
    vtx.Pos.Y = bbox.MaxEdge.Y;
    vtx.Pos.Z = bbox.MaxEdge.Z;
    vtx.Color = color;
    buffer->Vertices.push_back(vtx);

// left
    buffer->Indices.push_back(0);
    buffer->Indices.push_back(1);
    buffer->Indices.push_back(2);

    buffer->Indices.push_back(0);
    buffer->Indices.push_back(2);
    buffer->Indices.push_back(1);

// back
    buffer->Indices.push_back(1);
    buffer->Indices.push_back(5);
    buffer->Indices.push_back(6);

    buffer->Indices.push_back(1);
    buffer->Indices.push_back(6);
    buffer->Indices.push_back(2);

// right
    buffer->Indices.push_back(5);
    buffer->Indices.push_back(4);
    buffer->Indices.push_back(7);

    buffer->Indices.push_back(5);
    buffer->Indices.push_back(7);
    buffer->Indices.push_back(6);

// front
    buffer->Indices.push_back(4);
    buffer->Indices.push_back(0);
    buffer->Indices.push_back(3);

    buffer->Indices.push_back(4);
    buffer->Indices.push_back(3);
    buffer->Indices.push_back(7);

// bottom
    buffer->Indices.push_back(0);
    buffer->Indices.push_back(1);
    buffer->Indices.push_back(5);

    buffer->Indices.push_back(0);
    buffer->Indices.push_back(5);
    buffer->Indices.push_back(4);

// top
    buffer->Indices.push_back(3);
    buffer->Indices.push_back(2);
    buffer->Indices.push_back(6);

    buffer->Indices.push_back(3);
    buffer->Indices.push_back(6);
    buffer->Indices.push_back(7);

    //printf("debug %d norm start\n", i);
/*    
    for (s32 ind=0; ind<(s32)buffer->Indices.size(); ind+=3)
    {
	    core::plane3d<f32> p(
		                   buffer->Vertices[buffer->Indices[ind+0]].Pos,
		                   buffer->Vertices[buffer->Indices[ind+1]].Pos,
		                   buffer->Vertices[buffer->Indices[ind+2]].Pos);
        p.Normal.normalize();

	    buffer->Vertices[buffer->Indices[ind+0]].Normal = p.Normal;
	    buffer->Vertices[buffer->Indices[ind+1]].Normal = p.Normal;
	    buffer->Vertices[buffer->Indices[ind+2]].Normal = p.Normal;
    }
*/
    //printf("debug %d norm end\n", i);
   
    buffer->recalculateBoundingBox();

    SAnimatedMesh* animatedMesh = new SAnimatedMesh();
    mesh->addMeshBuffer(buffer);
    mesh->recalculateBoundingBox();
    animatedMesh->addMesh(mesh);
    animatedMesh->recalculateBoundingBox();

    mesh->drop();
    buffer->drop();
   
    ret = smgr->addAnimatedMeshSceneNode(animatedMesh);
    ret->setMaterialType(video::EMT_SOLID);
    ret->setMaterialFlag(video::EMF_LIGHTING, false);
    ret->setMaterialFlag(video::EMF_WIREFRAME, true);
    ret->setMaterialFlag(video::EMF_BACK_FACE_CULLING , false);
    ret->setMaterialFlag(video::EMF_FRONT_FACE_CULLING, false);
    ret->setVisible(false);
    
    return ret;
}

NewtonRaceCar::NewtonRaceCar(
    IrrlichtDevice* pdevice,
    ISceneManager* psmgr,
    IVideoDriver* pdriver,
	//const c8* fileName, 
	NewtonWorld* pnWorld, 
#ifdef USE_MY_SOUNDENGINE
    CMySoundEngine* psoundEngine,
#else
    irrklang::ISoundEngine* psoundEngine,
#endif
    NewtonCollision* &p_vehicleCollision,
    NewtonCollision* &p_vehicleCollisionBox,
    char* &p_omb,
	const int pcarType,
	const char* fileName,
	bool p_autoGear
    )
	:device(pdevice), smgr(psmgr), driver(pdriver),
     nWorld(pnWorld), soundEngine(psoundEngine),
     
     engineSound(0), groundSound(0), puffSound(0),
     skidSound(0),
     
     lastOnGroundTime(0), lastNotOnGroundTime(0), setTransformCount(0), lastUpdateGearTime(0),
     changeGearTime(10),
     lastSoundUpdateTime(0), lastOnGround(false),
     m_vehicleBody(0), m_vehicleJoint(0), m_node(0),
     m_steer(0.f), m_steer_kb(0.f), m_steer_rate(0.f),
     m_torque(0.f), m_torqueReal(0.f), m_brakes(0.f), m_debug(0), m_clutch(0),
     smokes(0), torque_multi(1.0f),/*, vId(0)*/
     friction_multi(0.8f), m_carType(pcarType),
     gear(1), engineGoing(false), engineRotate(0.f),
     cLight(0), current_car_dirt(0),
     carTex(0), clnode(0),
     demage(1.f), omb(p_omb), shadowNode(0), bb_node(0), autoGear(p_autoGear),
     vehicleCollision(p_vehicleCollision), vehicleCollisionBox(p_vehicleCollisionBox),
     num_of_tires(4), m_tires(), steer_tires(), torque_tires(), smoke_tires(),
     hb_tires(),
     mass(0.f), engine_center_of_mass(0.f), center_of_mass(0.f),
     chassis_rotation_limit(0.f), chassis_rotation_rate(0.f), max_brake_force(0.f), max_steer_angle(0.f),
     max_steer_rate(0.f), engine_steer_div(0.f), nameText(0), waterHeight(WATER_HEIGHT),
     offsetObject(0)
{
    FILE* f;
    int ret;
    float d1, d2, d3;
    c8 meshfileName[256];
    c8 texturefileName[256];
//    c8 texture2fileName[256];
    c8 soundfileName[256];
    //float d1, d2, d3;
    //int di;

	IAnimatedMesh *mesh;
	//SMesh *mesh;
	IAnimatedMeshSceneNode* bodyPart;	

    //char fileName[256];
    //if (m_carType >= carList.length() || m_carType < 0) m_carType = 0;
    //strcpy(fileName, "data/vehicles/");
    //strcat(fileName, carList[m_carType]->carFileName);
	
    dprintf(printf("Read car: %s\n", fileName));

    f = fopen(fileName, "r");
    if (!f)
    {
        printf("car file unable to open: %s\n", fileName);
        return;       
    }

//    ret = fscanf(f, "mesh: %s\ntexture: %s\ntexture2: %s\nenginesound: %s\n", meshfileName, texturefileName, texture2fileName, soundfileName);
    ret = fscanf(f, "mesh: %s\ntexture: %s\nenginesound: %s\n", meshfileName, texturefileName, soundfileName);
    if ( ret <=0 )
    {
        printf("error reading %s ret %d errno %d\n", fileName, ret, errno);
        fclose(f);
        return;
    }
// car1
    mesh = smgr->getMesh(meshfileName);//smgr->getMeshManipulator()->createMeshCopy(omesh);
    //printf("%p\n", mesh);
    
	bodyPart = smgr->addAnimatedMeshSceneNode(mesh);
    dprintf(printf("car frame count: %u\n", bodyPart->getMesh()->getFrameCount()));
    dprintf(printf("car frame amesh: %p\n", bodyPart->getMesh()));
    //dprintf(printf("car frame omb: %p\n", omb));
    dprintf(printf("car frame mesh: %p\n", mesh));
    dprintf(printf("car frame body mesh: %p\n", bodyPart->getMesh()?bodyPart->getMesh()->getMesh(0):(irr::scene::IMesh*)1));
    if (useShaders)
        bodyPart->setMaterialFlag(video::EMF_LIGHTING, false);
    else    
        bodyPart->setMaterialFlag(video::EMF_LIGHTING, globalLight);
	if (globalLight)
	{
        //if (stencil_shadows)
            shadowNode = bodyPart->addShadowVolumeSceneNode();
        //printf("add shadowNode %p\n", shadowNode);
        bodyPart->setMaterialFlag(video::EMF_NORMALIZE_NORMALS, true);
    }
    if (strcmp(texturefileName,"null")!=0)
    {
        char baseName[256];
        char extName[256];
        char brakeName[256];
        char lightName[256];
        char lightBrakeName[256];
        
        //sscanf(texturefileName, "%s.%s", baseName, extName);
        strcpy(baseName, texturefileName);
        baseName[strlen(texturefileName)-4] = '\0';
        strcpy(extName, texturefileName + (strlen(texturefileName)-4));
        dprintf(printf("car file name base: %s, ext: %s\n", baseName, extName));
        
        strcpy(brakeName, baseName);
        strcat(brakeName, "_b");
        strcat(brakeName, extName);

        carTexs[0] = driver->getTexture(texturefileName);
        carTexs[1] = driver->getTexture(brakeName);
        
        //if (useShaders)
        //{
            bodyPart->setMaterialTexture(0, carTexs[0]);

            bodyPart->setMaterialType((video::E_MATERIAL_TYPE)myMaterialType_light_tex_s_car);
        //}
    }
    else
    {
        carTexs[0] = 0;
        carTexs[1] = 0;
        bodyPart->setMaterialType((video::E_MATERIAL_TYPE)myMaterialType_light_notex_car);
    }
    bodyPart->setVisible(false);

    if (strcmp(soundfileName, ""))
    {
#ifdef USE_MY_SOUNDENGINE
        engineSound = soundEngine->play3D(soundfileName, vector3df(), true, true, true);
#else
        engineSound = soundEngine->play3D(soundfileName, vector3df(), true, false, true);
#endif
    }
    if (engineSound)
    {
#ifdef USE_MY_SOUNDENGINE
        engineSound->setMinDistance(4.0f);
#else
        engineSound->setMinDistance(4.0f);
#endif
        engineSound->setIsPaused(true);
    }

    ret = fscanf(f, "scale: %f, %f, %f\n", &d1, &d2, &d3);
    if ( ret <=0 )
    {
        printf("error reading %s ret %d errno %d\n", fileName, ret, errno);
        fclose(f);
        return;
    }
    bodyPart->setScale(vector3df(d1,d2,d3));

    ret = fscanf(f, "rotation: %f, %f, %f\n", &d1, &d2, &d3);
    if ( ret <=0 )
    {
        printf("error reading %s ret %d errno %d\n", fileName, ret, errno);
        fclose(f);
        return;
    }
    m_trafo.makeIdentity();
    m_trafo.setRotationDegrees(vector3df(d1,d2,d3));
    
	m_node = bodyPart;
	offsetObject = new OffsetObject(m_node, true);
	offsetObject->setUserDataAndCallback((void*)this, offsetObjectCallback);
    if (useCgShaders)
        m_node->getMaterial(0).setFlag(EMF_BILINEAR_FILTER, false);

    //printf("+++++++++\n %f %f\n +++++++++", groundSound->getVolume(), engineSound->getVolume());
    // simplifiy the collision with a box
    core::aabbox3d<f32> bbox = bodyPart->getBoundingBox();
//    bbox.MinEdge.X *= 0.9f;
//    bbox.MaxEdge.X *= 0.9f;
//    bbox.MinEdge.Y = 0.25f;
//    bbox.MinEdge.Z *= 0.1f;
//    bbox.MaxEdge.Z *= 0.1f;
    //bb_node = createBB(smgr, bbox);
    if (vehicleCollision == 0 || omb == 0)
    {
        assert(vehicleCollision==0 && omb==0 && vehicleCollisionBox==0);
        
        int sizeOfBuffers = 0;
        for (int i = 0; i < mesh->getMeshBufferCount(); i++)
        {
            //printf("%d %d %d\n", i, mesh->getMeshBuffer(i)->getVertexType(), mesh->getMeshBuffer(i)->getVertexCount());
            if (mesh->getMeshBuffer(i)->getVertexType() != EVT_STANDARD) {fclose(f);return;}
            
            sizeOfBuffers += mesh->getMeshBuffer(i)->getVertexCount();
        }
        
        //printf("size of omb %d\n", sizeOfBuffers);
        
        omb = new char[sizeOfBuffers*sizeof(video::S3DVertex)];
        
        char* cur = (char*)omb;
        
        for (int i = 0; i < mesh->getMeshBufferCount() && cur; i++)
        {
            int copySize = mesh->getMeshBuffer(i)->getVertexCount()*sizeof(video::S3DVertex);
            memcpy(cur, mesh->getMeshBuffer(i)->getVertices(), copySize);
            cur = cur + copySize;
        }

        vehicleCollisionBox = NewtonCreateBox(nWorld,
                           (bbox.MaxEdge.X - bbox.MinEdge.X)*bodyPart->getScale().X,
                           (bbox.MaxEdge.Y - bbox.MinEdge.Y)*bodyPart->getScale().Y,
                           (bbox.MaxEdge.Z - bbox.MinEdge.Z)*bodyPart->getScale().Z,
                           vehicleID, NULL);

        //printf("car mbc %u\n", mesh->getMeshBufferCount());
    
        float* my_vertices = new float[sizeOfBuffers*3];
        int cursor = 0;
    
        //printf("1 addr %p size %u\n", my_vertices, sizeOfBuffers*3);
        
        for (int i = 0; i < mesh->getMeshBufferCount();i++)
        {
            IMeshBuffer* mb = mesh->getMeshBuffer(i);
            video::S3DVertex* mb_vertices = (video::S3DVertex*)mb->getVertices();
            //printf("%d. mb->getVertexCount() %u cursor %d ... ", i, mb->getVertexCount(), cursor);
            for (int j = cursor; j < cursor + mb->getVertexCount(); j++)
            {
                //printf("%d. %p\n", j, &my_vertices[j*3]);
                my_vertices[j*3] = mb_vertices[j-cursor].Pos.X*bodyPart->getScale().X;
                my_vertices[j*3+1] = mb_vertices[j-cursor].Pos.Y*bodyPart->getScale().Y;
                my_vertices[j*3+2] = mb_vertices[j-cursor].Pos.Z*bodyPart->getScale().Z;
            }
            cursor += mb->getVertexCount();
            //printf("done\n");
        }
    
        vehicleCollision = NewtonCreateConvexHull(nWorld, sizeOfBuffers,
                                my_vertices, 3 * sizeof(float), 0.1f, vehicleID,
                                0);
        if (!vehicleCollision)
        {
            vehicleCollision = NewtonCreateBox(nWorld,
                                   (bbox.MaxEdge.X - bbox.MinEdge.X)*bodyPart->getScale().X,
                                   (bbox.MaxEdge.Y - bbox.MinEdge.Y)*bodyPart->getScale().Y,
                                   (bbox.MaxEdge.Z - bbox.MinEdge.Z)*bodyPart->getScale().Z,
                                   vehicleID, NULL);
            printf("vehicle collision zero, create box instead %p\n", vehicleCollision);
        }
        //printf("vehicle collision %p\n", vehicleCollision);
        delete [] my_vertices;
        
        p_vehicleCollision = vehicleCollision;
        p_vehicleCollisionBox = vehicleCollisionBox;
        p_omb = omb;
    }
    dprintf(printf("Vehicle min: %f %f %f\n", bbox.MinEdge.X, bbox.MinEdge.Y, bbox.MinEdge.Z));
    dprintf(printf("Vehicle max: %f %f %f\n", bbox.MaxEdge.X, bbox.MaxEdge.Y, bbox.MaxEdge.Z));
    dprintf(printf("Vehicle size: %f %f %f\n", bbox.MaxEdge.X - bbox.MinEdge.X,
                           bbox.MaxEdge.Y - bbox.MinEdge.Y, bbox.MaxEdge.Z - bbox.MinEdge.Z));
//	for (int i = 0; i< 16; i++)
//	    printf("%f \n", mat[i]);

    ret = fscanf(f, "mass: %f\n", &mass);
    if ( ret <=0 )
    {
        printf("error reading %s ret %d errno %d\n", fileName, ret, errno);
        fclose(f);
        return;
    }

    ret = fscanf(f, "engine_centre_of_mass: %f\n", &engine_center_of_mass);
    if ( ret <=0 )
    {
        printf("error reading %s ret %d errno %d\n", fileName, ret, errno);
        fclose(f);
        return;
    }
    
    ret = fscanf(f, "centre_of_mass: %f\n", &center_of_mass);
    if ( ret <=0 )
    {
        printf("error reading %s ret %d errno %d\n", fileName, ret, errno);
        fclose(f);
        return;
    }

    ret = fscanf(f, "num_of_tyres: %d\n", &num_of_tires);
    if ( ret <=0 )
    {
        printf("error reading %s ret %d errno %d\n", fileName, ret, errno);
        fclose(f);
        return;
    }

    ret = fscanf(f, "chassis_rotation_limit: %f\n", &chassis_rotation_limit);
    if ( ret <=0 )
    {
        printf("error reading %s ret %d errno %d\n", fileName, ret, errno);
        fclose(f);
        return;
    }
	
    ret = fscanf(f, "chassis_rotation_rate: %f\n", &chassis_rotation_rate);
    if ( ret <=0 )
    {
        printf("error reading %s ret %d errno %d\n", fileName, ret, errno);
        fclose(f);
        return;
    }
	
    ret = fscanf(f, "max_brake_force: %f\n", &max_brake_force);
    if ( ret <=0 )
    {
        printf("error reading %s ret %d errno %d\n", fileName, ret, errno);
        fclose(f);
        return;
    }

    struct gl glo;
    float prevLimit = 0.f;
    #define SPEED_MULTI 10.f
    #define MAX_TORQUE_MULTI 0.7f
    int gearNum;

    ret = fscanf(f, "max_gears: %d\n", &gearNum);
    if ( ret <=0 )
    {
        printf("error reading %s ret %d errno %d\n", fileName, ret, errno);
        fclose(f);
        return;
    }
    
    for (int i = 0; i<gearNum; i++)
    {
        ret = fscanf(f, "gear: %f %f\n", &glo.max_torque, &glo.max_torque_rate);
        if ( ret <=0 )
        {
            printf("error reading %s ret %d errno %d\n", fileName, ret, errno);
            fclose(f);
            return;
        }

        if (prevLimit==0.f)
//            glo.low = -((glo.max_torque) * (float)(i+1) * SPEED_MULTI / mass)-5.f;
            glo.low = -((glo.max_torque/(float)gearNum) * (float)(i+1) * SPEED_MULTI / mass)-5.f;
        else
            glo.low = prevLimit - 5;
//        prevLimit = (glo.max_torque) * (float)(i+1) * SPEED_MULTI / mass; 
        prevLimit = (glo.max_torque/(float)(gearNum+1)) * (float)(i+1) * SPEED_MULTI / mass;
        glo.high = prevLimit + 5;
        glo.max_torque /= (float)(i+1);
        //glo.max_torque *= ((float)(gearNum - gearLimits.length())*MAX_TORQUE_MULTI);
        dprintf(printf("add gear (%d): %f .. %f %f\n", i+1, glo.low, glo.high, glo.max_torque));
        gearLimits.addLast(glo);
    }

    ret = fscanf(f, "change_gear_time: %d\n", &changeGearTime);
    if ( ret <=0 )
    {
        printf("error reading %s ret %d errno %d\n", fileName, ret, errno);
        fclose(f);
        return;
    }

    ret = fscanf(f, "max_steer_angle: %f\n", &max_steer_angle);
    if ( ret <=0 )
    {
        printf("error reading %s ret %d errno %d\n", fileName, ret, errno);
        fclose(f);
        return;
    }

    ret = fscanf(f, "max_steer_rate: %f\n", &max_steer_rate);
    if ( ret <=0 )
    {
        printf("error reading %s ret %d errno %d\n", fileName, ret, errno);
        fclose(f);
        return;
    }
	m_steer_rate = max_steer_rate * 0.25f;

    ret = fscanf(f, "engine_steer_div: %f\n", &engine_steer_div);
    if ( ret <=0 )
    {
        printf("error reading %s ret %d errno %d\n", fileName, ret, errno);
        fclose(f);
        return;
    }

    ret = fscanf(f, "torque_multi: %f\n", &torque_multi);
    if ( ret <=0 )
    {
        printf("error reading %s ret %d errno %d\n", fileName, ret, errno);
        fclose(f);
        return;
    }
    
    for (int i = 0; i < view_max*view_multi; i++)
    {
        char viewName[256];
        viewpos[i] = matrix4();
        ret = fscanf(f, "%s %f %f %f\n", viewName, &viewpos[i][12], &viewpos[i][13], &viewpos[i][14]);
        if ( ret < 4 )
        {
            printf("error reading %s ret %d errno %d\n", fileName, ret, errno);
            fclose(f);
            return;
        }
        viewdest[i] = matrix4();
        ret = fscanf(f, "%s %f %f %f\n", viewName, &viewdest[i][12], &viewdest[i][13], &viewdest[i][14]);
        if ( ret < 4 )
        {
            printf("error reading %s ret %d errno %d\n", fileName, ret, errno);
            fclose(f);
            return;
        }
    }
    //matrix4 viewdest[view_max*view_multi];
    //matrix4 viewpos[view_max*view_multi];
    

	// Add tires;
	for (int i = 0; i < num_of_tires; i++)
	{
        RaceCarTire* tire = new RaceCarTire(smgr, driver, this, i, f);
        m_tires.push_back(tire);
    }
	
	fclose(f);

	clnode = smgr->addLightSceneNode(0//,
//        core::vector3df(0.f,-10.f,-1.f),
//        core::vector3df(5300.f,10000.f,10000.f),
//		video::SColorf(lightColor, lightColor, lightColor), 50000.f
            );
    clnode->getLightData().Type=ELT_SPOT; 
    clnode->setRotation(core::vector3df(90.f, 0.f, 0.f));
    float lightColor = 1.0f;
    float lightStrength = 1.0f;
    float lightStrengthS = 1.0f;
    clnode->getLightData().DiffuseColor = video::SColorf(lightColor,lightColor,lightColor);
    clnode->getLightData().AmbientColor = video::SColorf(lightStrength,lightStrength,lightStrength);
    clnode->getLightData().SpecularColor = video::SColorf(lightStrengthS,lightStrengthS,lightStrengthS);
    clnode->getLightData().InnerCone = 10.f;
    clnode->getLightData().OuterCone = 25.f;
	
	smokes = new Smoke*[MAX_SMOKES];
	memset(smokes, 0, MAX_SMOKES*sizeof(Smoke*));

    if (torque_tires.size() > 0)
    {
        int divSpeed = 4 / torque_tires.size();

        if (divSpeed > 1)
        {
            for (int i = 0; i < gearLimits.size(); i++)
            {
                //gearLimits[i].low /= 2;
                //gearLimits[i].high /= 2;
                gearLimits[i].max_torque *= divSpeed+1;
                gearLimits[i].max_torque_rate *= divSpeed+1;
            }
        }
    }
    //NewtonBodySetFreezeState(m_vehicleBody, 1);
    //reset(core::vector3df(-20000.f,-20000.f,-20000.f));

}

void NewtonRaceCar::activate(
                        const vector3df& loc, const vector3df& rot,
                        const c8* groundSoundName,
                        const c8* puffSoundName,
                        const c8* skidSoundName,
                        const float pfriction_multi,
                        scene::ISceneNode* skydome,
                        video::ITexture* shadowMap,
                        const float p_waterHeight,
                        bool p_useOffset,
                        const int savedCarDirt,
                        float pressure_multi,
                        float ss_multi,
                        float sd_multi,
                        float sl_multi
                        )
{
    friction_multi = pfriction_multi;
    waterHeight = p_waterHeight;
// set the dirt level of the car
    current_car_dirt = savedCarDirt;
    if (current_car_dirt >= MAX_CAR_DIRT ) current_car_dirt = MAX_CAR_DIRT - 1;

// sets the current sky and shadowMap
    if (useCgShaders)
    {
        if (m_node->getMaterial(0).getTexture(0))
        {
            m_node->setMaterialTexture(1, car_dirttexture_array[current_car_dirt]);
            if (skydome && skydome->getMaterial(0).getTexture(0))
                m_node->setMaterialTexture(2, skydome->getMaterial(0).getTexture(0));
            if (shadowMap)
                m_node->setMaterialTexture(3, shadowMap);
        }
        else
        {
            if (skydome && skydome->getMaterial(0).getTexture(0))
            {
                m_node->setMaterialTexture(0, skydome->getMaterial(0).getTexture(0));
            }
        }
    }
    else
    {
        if (skydome && skydome->getMaterial(0).getTexture(0))
        {
            //m_node->setMaterialTexture(1, m_node->getMaterial(0).getTexture(0));
            m_node->setMaterialTexture(1, skydome->getMaterial(0).getTexture(0));
        }
    }

// sound setup
    if (strcmp(groundSoundName, ""))
    {
        groundSound = soundEngine->play3D(groundSoundName, loc, true, true, true);
        if (groundSound)
        {
    #ifdef USE_MY_SOUNDENGINE
            groundSound->setMinDistance(3.0f);
            groundSound->setVolume(0.1f);
    #else
            groundSound->setMinDistance(3.0f);
            groundSound->setVolume(0.3f);
    #endif
        }
    }
    if (strcmp(puffSoundName, ""))
    {
        puffSound = soundEngine->play3D(puffSoundName, m_matrix.getTranslation(), false, true, true);
        if (puffSound)
        {
    #ifdef USE_MY_SOUNDENGINE
            puffSound->setVolume(0.1f);
            puffSound->setMinDistance(3.0f);
    #else
            puffSound->getSoundSource()->setDefaultVolume(0.15f);
            puffSound->getSoundSource()->setDefaultMinDistance(3.0f);
    #endif
        }
    }
    if (strcmp(skidSoundName, ""))
    {
        skidSound = soundEngine->play3D(skidSoundName, loc, true, true, true);
        if (skidSound)
        {
    #ifdef USE_MY_SOUNDENGINE
            skidSound->setMinDistance(3.0f);
    #else
            skidSound->setMinDistance(2.0f);
    #endif
            //skidSound->setVolume(0.3f);
        }
    }
    lastOnGround = false;
    
    if (shadowNode)
    {
        //printf("shadowNode\n");
        addToShadowNodes(shadowNode);
    }
    else
    {
        //printf("bodyPart\n");
        addToShadowNodes(m_node);
    }
    addToDepthNodes(m_node); // new depth


    m_node->setVisible(true);

// physic engine stuff
	//create the rigid body
	m_vehicleBody = NewtonCreateBody (nWorld, vehicleCollision);
	dprintf(printf("create car body %p\n", m_vehicleBody);)

	// save the pointer to the graphic object with the body.
	NewtonBodySetUserData (m_vehicleBody, this);

	// set the material group id for vehicle
	NewtonBodySetMaterialGroupID (m_vehicleBody, vehicleID);

	float Ixx;
	float Iyy;
	float Izz;
	dVector origin;
	dVector inertia;

	// calculate the moment of inertia and the relative center of mass of the solid
	NewtonConvexCollisionCalculateInertialMatrix (vehicleCollisionBox, &inertia[0], &origin[0]);	

	Ixx = mass * inertia[0];
	Iyy = mass * inertia[1];
	Izz = mass * inertia[2];

	// set the mass matrix
	NewtonBodySetMassMatrix (m_vehicleBody, mass, Ixx, Iyy, Izz);

	// Set the vehicle Center of mass
	// the rear spoilers race the center of mass by a lot for a race car
	// we need to lower some more for the geometrical value of the y axis
	//origin.m_y *= 0.5f;
    origin.m_x += engine_center_of_mass; //JEEP_ENGINE_CENTRE_OF_MASS_OFFSET;
    
    origin.m_y += center_of_mass; //JEEP_CENTRE_OF_MASS_OFFSET; 

	NewtonBodySetCentreOfMass (m_vehicleBody, &origin[0]);

	// set the matrix for both the rigid body and the graphic body
	//NewtonBodySetMatrix (m_vehicleBody, &matrix[0][0]);
    // mycode
	//NewtonBodySetMatrix(m_vehicleBody, mat.pointer());

	// ////////////////////////////////////////////////////////////////////////////////////////////////
	//
	//  second we need to add a vehicle joint to the body
	//
	// ////////////////////////////////////////////////////////////////////////////////////////////////
	dMatrix chassisMatrix;
	chassisMatrix.m_front = dVector(1.f, 0.f, 0.f, 0.f);				// this is the vehicle direction of travel
	chassisMatrix.m_up	  = dVector(0.f, 1.f, 0.f, 0.f);			// this is the downward vehicle direction
	chassisMatrix.m_right = dVector(0.f, 0.f, 1.f, 0.f);
    //chassisMatrix.m_front * chassisMatrix.m_up;	// this is in the side vehicle direction (the plane of the wheels)
	chassisMatrix.m_posit = dVector(0.f, 0.f,0.f,1.f);
//	chassisMatrix.m_posit = dVector(loc.X, loc.Y,loc.Z,1.f);

	m_vehicleJoint = new CustomDGRayCastCar(m_tires.size(), chassisMatrix, m_vehicleBody) ;
    dprintf(printf("m_vehicleJoint: %p %p\n", m_vehicleJoint, m_vehicleJoint->GetJoint()));
	
    m_vehicleJoint->SetJointID (CUSTOM_VEHICLE_JOINT_ID);
#ifdef OLD_CDGRCC
	m_vehicleJoint->SetVarChassisRotationLimit(chassis_rotation_limit);

	m_vehicleJoint->SetVarChassisRotationRate(chassis_rotation_rate);
	
	m_vehicleJoint->SetVarMaxBrakeForce(max_brake_force);

	m_vehicleJoint->SetVarMaxTorque(gearLimits[0].max_torque);
	m_vehicleJoint->SetVarMaxTorqueRate(gearLimits[0].max_torque_rate);

	m_vehicleJoint->SetVarMaxSteerAngle(max_steer_angle);

	//m_steer_rate = max_steer_rate * 0.25f;
	m_vehicleJoint->SetVarMaxSteerRate(0.1f);

	m_vehicleJoint->SetVarEngineSteerDiv(engine_steer_div);
#endif
	matrix4 mat;
	mat.setTranslation(loc);
	mat.setRotationDegrees(rot);
	setMatrixWithNB(mat);
    dprintf(printf("end phys\n");)

    if (p_useOffset && offsetObject)
    {
        offsetObject->setBody(m_vehicleBody);
    	offsetManager->addObject(offsetObject);
    }

// end phys
    for (int i = 0; i < m_tires.size(); i++)
    {
        // tire position will be get from the m_node's position, so
        // don't ned to set it manually
        m_tires[i]->activate(p_useOffset);
    }

	// set a destructor for this rigid body
	NewtonBodySetDestructorCallback (m_vehicleBody, DestroyVehicle);

	// set the transform call back function
	NewtonBodySetTransformCallback (m_vehicleBody, SetTransform);

	// set the force and torque call back function
	NewtonBodySetForceAndTorqueCallback (m_vehicleBody, ApplyGravityForce);

    NewtonBodySetAutoSleep(m_vehicleBody, 0);
    //NewtonBodySetFreezeState(m_vehicleBody, 0);
    setPressure(pressure_multi);
    setSuspensionSpring(ss_multi);
    setSuspensionDamper(sd_multi);
    setSuspensionLength(sl_multi);
    
    dprintf(printf("end activate %p\n", this);)
}

void NewtonRaceCar::deactivate()
{
    //NewtonBodySetFreezeState(m_vehicleBody, 1);
    reset(core::vector3df(-20000.f,-20000.f,-20000.f));
    repair();
    delete m_vehicleJoint;
    m_vehicleJoint = 0;
    NewtonBodySetDestructorCallback (m_vehicleBody, 0);
    NewtonBodySetTransformCallback (m_vehicleBody, 0);
    NewtonBodySetForceAndTorqueCallback (m_vehicleBody, 0);
	NewtonBodySetUserData (m_vehicleBody, 0);
    NewtonDestroyBody(nWorld, m_vehicleBody);
    m_vehicleBody = 0;

    if (offsetObject)
    {
        offsetObject->setBody(m_vehicleBody);
    	offsetManager->removeObject(offsetObject);
    }

    pause();
    stopEngine();
    if (groundSound)
    {
#ifdef USE_MY_SOUNDENGINE
        delete groundSound;
#else
        groundSound->drop();
#endif
        groundSound = 0;
    }
    if (skidSound)
    {
#ifdef USE_MY_SOUNDENGINE
        delete skidSound;
#else
        skidSound->drop();
#endif
        skidSound = 0;
    }
    if (puffSound)
    {
#ifdef USE_MY_SOUNDENGINE
        delete puffSound;
#else
        puffSound->drop();
#endif
        puffSound = 0;
    }

    if (smokes)
    {
        for(int ind = 0;ind<MAX_SMOKES;ind++)
            if (smokes[ind])
            {
                //printf("delete smoke\n");
                //smokes[ind]->node->remove();
                //delete smokes[ind];
                //smokes[ind] = 0;
                smokes[ind]->animePhase = -1;
                smokes[ind]->node->setVisible(false);
            }
    }
    m_node->setVisible(false);
    for (int i = 0; i < m_tires.size(); i++)
    {
        m_tires[i]->deactivate();
    }

    if (shadowNode)
        removeFromShadowNodes(shadowNode);
    else
        removeFromShadowNodes(m_node);
    removeFromDepthNodes(m_node); // new depth
}


NewtonRaceCar::~NewtonRaceCar()
{
    
    printf("release racecar reset\n");
    //reset(core::vector3df(-20000.f,-20000.f,-20000.f));
    //NewtonInvalidateCache(nWorld);
    printf("release racecar repair\n");
    repair();
    if (m_vehicleJoint)
    {
        printf("release racecar joint\n");
        //NewtonDestroyJoint(nWorld, m_vehicleJoint);
        delete m_vehicleJoint;
        printf("release racecar joint end %p\n", m_vehicleJoint);
        m_vehicleJoint = 0;
    }
    printf("release racecar body %p\n", m_vehicleBody);
    if (m_vehicleBody)
    {
        NewtonBodySetDestructorCallback (m_vehicleBody, 0);
        NewtonBodySetTransformCallback (m_vehicleBody, 0);
        NewtonBodySetForceAndTorqueCallback (m_vehicleBody, 0);
    	NewtonBodySetUserData (m_vehicleBody, 0);
        NewtonDestroyBody(nWorld, m_vehicleBody);
        m_vehicleBody = 0;
    }
    printf("release racecar body end %p\n", m_vehicleBody);
    printf("delete engine sound\n");
	// release the collision 
	vehicleCollision = 0;
	vehicleCollisionBox = 0;

    pause();

    if (offsetObject)
    {
        offsetObject->setBody(0);
        offsetObject->setNode(0);
    	offsetManager->removeObject(offsetObject);
    	delete offsetObject;
    	offsetObject = 0;
    }

    if (engineSound)
    {
#ifdef USE_MY_SOUNDENGINE
        delete engineSound;
#else
        engineSound->drop();
#endif
        engineSound = 0;
    }
    if (groundSound)
    {
#ifdef USE_MY_SOUNDENGINE
        delete groundSound;
#else
        groundSound->drop();
#endif
        groundSound = 0;
    }
    if (skidSound)
    {
#ifdef USE_MY_SOUNDENGINE
        delete skidSound;
#else
        skidSound->drop();
#endif
        skidSound = 0;
    }
    if (puffSound)
    {
#ifdef USE_MY_SOUNDENGINE
        delete puffSound;
#else
        puffSound->drop();
#endif
        puffSound = 0;
    }
    printf("NewtonVehicle drop\n");
    if (m_node)
    {
        if (shadowNode)
            removeFromShadowNodes(shadowNode);
        else
            removeFromShadowNodes(m_node);
        removeFromDepthNodes(m_node); // new depth
        m_node->remove();
        m_node = 0;
    }
    printf("delete smokes\n");
    if (smokes)
    {
        for(int ind = 0;ind<MAX_SMOKES;ind++)
            if (smokes[ind])
            {
                //printf("delete smoke\n");
                smokes[ind]->node->remove();
                delete smokes[ind];
                smokes[ind] = 0;
            }
        delete [] smokes;
    }
    
    printf("delete omb\n");
    omb = 0;

    for (int i = 0; i < m_tires.size(); i++)
    {
        delete m_tires[i];
        m_tires[i] = 0;
    }
    m_tires.clear();
    steer_tires.clear();
    torque_tires.clear();
    smoke_tires.clear();
    hb_tires.clear();
    //NewtonDestroyBody(nWorld, m_vehicleBody);
    //NewtonInvalidateCache(nWorld);
}

NewtonRaceCar::RaceCarTire::~RaceCarTire()
{
   printf("tire drop %d node %p body %p\n", tire_num, m_tireNode, m_newtonBody);
   
   if (m_newtonBody)
   {
       NewtonBodySetDestructorCallback (m_newtonBody, 0);
       NewtonBodySetTransformCallback (m_newtonBody, 0);
       NewtonBodySetForceAndTorqueCallback (m_newtonBody, 0);
   }
   if(m_tireNode)
   {
      if (shadowNode)
          removeFromShadowNodes(shadowNode);
      else
          removeFromShadowNodes(m_tireNode);
      removeFromDepthNodes(m_tireNode); // new depth
      m_tireNode->remove();
      m_tireNode = 0;
   }
    if (offsetObject)
    {
        offsetObject->setBody(0);
        offsetObject->setNode(0);
    	offsetManager->removeObject(offsetObject);
    	delete offsetObject;
    	offsetObject = 0;
    }

   printf("tire drop %d end\n", tire_num);
}


void NewtonRaceCar::DestroyVehicle (const NewtonBody* body)
{
	NewtonRaceCar* vehicle;

    printf("NewtonRaceCar::DestroyVehicle\n");

	// get the graphic object form the rigid body
	vehicle = (NewtonRaceCar*) NewtonBodyGetUserData (body);

    if (vehicle)
    {
	    // destroy the graphic object
	    delete vehicle;
    }
}

void NewtonRaceCar::RaceCarTire::DestroyTire (const NewtonBody* body)
{
	NewtonRaceCar::RaceCarTire* tire;

    printf("NewtonRaceCar::DestroyTire\n");

	// get the graphic object form the rigid body
	tire = (NewtonRaceCar::RaceCarTire*) NewtonBodyGetUserData (body);

	// destroy the graphic object
	//delete tire;
}



// set the transformation of a rigid body
void NewtonRaceCar::ApplyGravityForce (const NewtonBody* body, float timestep, int threadIndex)
{
	float Ixx;
	float Iyy;
	float Izz;
	float mass;
	float speed;
	NewtonRaceCar* vehicle;

//	vehicle = (NewtonRaceCar*) NewtonBodyGetUserData(body);
/*
    if (vehicle->m_debug)
    {
        printf("NewtonRaceCar::ApplyGravityForce\n");
    }
*/
    pdprintf(printf("NewtonRaceCar::ApplyGravityForce body %p\n", body));
	NewtonBodyGetMassMatrix (body, &mass, &Ixx, &Iyy, &Izz);

	if (NewtonBodyGetMaterialGroupID(body) == vehicleID)
	{
    	
    	vehicle = (NewtonRaceCar*)NewtonBodyGetUserData(body);
    /*	
    	speed = fabsf (car->getSpeed());
    	mass *= (1.0f + speed / 20.0f);
    */
    //mass = 0.0f;
        if (vehicle->m_node->getPosition().Y > vehicle->waterHeight)
        {
            dVector force (0.0f, mass * GRAVITY, 0.0f);
            NewtonBodySetForce (body, &force.m_x);
        }
        else
        {
            dVector force (0.0f, mass * GRAVITY * 0.05f, 0.0f);
            NewtonBodySetForce (body, &force.m_x);
        }
    }
    else
    {
        dVector force (0.0f, mass * GRAVITY, 0.0f);
        NewtonBodySetForce (body, &force.m_x);
    }
//NewtonBodySetVelocity(body, &force.m_x);
/*
	static int xxx;
	xxx ++;
	if (xxx == 2000){
		NewtonDestroyBody (xxxx, body);
	}
*/
    pdprintf(printf("NewtonRaceCar::ApplyGravityForce end body %p\n", body));

}

// Set the vehicle matrix and all tire matrices
void NewtonRaceCar::SetTransform (const NewtonBody* body, const float* matrixPtr, int threadIndex)
{
    if (newtonUpdateCount != 1) return;
    
	void* tyreId;
	float sign;
	float angle;
	float brakePosition;
	
	RaceCarTire* tireRecord;
	NewtonRaceCar* vehicle;
	NewtonJoint* joint;


    pdprintf(printf("NewtonRaceCar::SetTransform body %p\n", body));
	// get the graphic object form the rigid body

	// set the transformation matrix for this rigid body
	matrix4 matrix;
	memcpy(matrix.pointer(), matrixPtr, sizeof(float)*16);
//    matrix.setM(matrixPtr);

	vehicle = (NewtonRaceCar*) NewtonBodyGetUserData (body);
 /*
    if (vehicle->m_debug)
    {
        printf("NewtonRaceCar::SetTransform\n");
    }
*/
    if (vehicle->m_vehicleJoint->GetJointID() != CUSTOM_VEHICLE_JOINT_ID)
    {
        printf("settransform on a non-vehicle\n");
	}
    //u32 time = vehicle->device->getTimer()->getTime();
    vehicle->setTransformCount++;
    vehicle->setMatrix(matrix);
    if (vehicle->nameText)
    {
        vector3df vehPos = matrix.getTranslation();
        vehicle->nameText->setPosition(vector3df(vehPos.X, vehPos.Y + 3.0f, vehPos.Z));
    }
    //float angularVelocityHelper = fabsf((vehicle->m_vehicleJoint->GetTire(0).m_angularVelocity));
    float speed = vehicle->getSpeed();
    float angularVelocity = //angularVelocityHelper>180.f?angularVelocityHelper/100:angularVelocityHelper/2.f;
                            fabsf(speed);
    float soundSpeed = angularVelocity/80.f+1.0f;
    
    float soundSpeed2 = 0.f;
    float soundSpeed3 = 0.f;
    
    if ((vehicle->gear==0 /*|| (joy_axis_clutch!=-1 && vehicle->m_clutch>0.4f)*/)/* &&
        vehicle->m_torqueReal>0.f*/)
        if ((vehicle->m_torqueReal<-0.001f && speed>0.101f) ||
            (vehicle->m_torqueReal>0.001f && speed<-0.101f))
        {
            soundSpeed2 = 0.0f;
        }
        else
        {
            soundSpeed2 = fabsf(vehicle->m_torque)*1.4f + 0.0f;
        }
    else
    if (joy_axis_clutch!=-1)
    {
        if ((vehicle->m_torqueReal<-0.001f && speed>0.101f) ||
            (vehicle->m_torqueReal>0.001f && speed<-0.101f))
        {
            soundSpeed2 = 0.0f;
        }
        else
        {
            soundSpeed2 = fabsf(vehicle->m_torque)*1.4f + 0.0f;
            if (joy_axis_clutch!=-1)
            {
                soundSpeed3 = angularVelocity/(vehicle->gearLimits[vehicle->gear-1].high)+0.0f;
                soundSpeed2 = soundSpeed3 * (1.f - vehicle->m_clutch) + 
                              soundSpeed2 * vehicle->m_clutch; 
            }
        }
    }
    else
    {
        soundSpeed2 = angularVelocity/(vehicle->gearLimits[vehicle->gear-1].high)+0.0f;
    }

    if (vehicle->engineGoing)
    {
        vehicle->engineRotate = soundSpeed2+1.0f;
        if (vehicle->engineRotate > 2.4f) vehicle->engineRotate = 2.4f;
    }
    else
        vehicle->engineRotate = 0.f;

    //soundSpeed2 = 1.0f - (soundSpeed2);
    //soundSpeed2 *= soundSpeed2*soundSpeed2;
    //soundSpeed2 = (1.0f - soundSpeed2);
    //printf("%f\n", soundSpeed2);
    /*
    printf("%f %f/%f\n",
        soundSpeed2,
        vehicle->m_vehicleJoint->GetTire(0).m_localLongitudinalSpeed,
        vehicle->m_vehicleJoint->GetTire(0).m_currentSlipVeloc);
    */
    soundSpeed2 += 1.0f;
    
    if (soundSpeed2>2.4f) soundSpeed2 = 2.4f;
    
    if (vehicle->engineSound && (tick - vehicle->lastSoundUpdateTime > SOUND_UPDATE_INTERVAL))
    {
        vehicle->engineSound->setPosition(matrix.getTranslation());
        vehicle->engineSound->setPlaybackSpeed(soundSpeed2);
    }
    vehicle->updateSmoke();
    
//    if (vehicle->m_debug)
//    {
//        printf("brakes %f torque %f steer %f thread %d\n", vehicle->m_brakes,
//               vehicle->m_torque, vehicle->m_steer, threadIndex);
//    }
    
    //Set the global matrix for each tire
	matrix4 invMatrix;
    vehicle->m_matrix.getInverse(invMatrix);
	matrix4 tireMatrix;

    int tireCount = vehicle->m_vehicleJoint->GetTiresCount();
    for (int i = 0; i < tireCount; i ++) {
        const CustomDGRayCastCar::Tire& tire = vehicle->m_vehicleJoint->GetTire(i);
        tireRecord = (RaceCarTire*)tire.m_userData;
        if (tireRecord->connected)
        {
            dMatrix tmatrix(vehicle->m_vehicleJoint->CalculateTireMatrix(i));
            tireMatrix.setM((float*)(&tmatrix));
	        //memcpy(tireMatrix.pointer(), (float*)(&tmatrix), sizeof(float)*16);
    		// calculate the local matrix 
		    tireRecord->setMatrix (tireMatrix);
        }
	}
	//printf("tire torque %f\n", vehicle->m_vehicleJoint->GetTire(vehicle->m_rearRighTire.tire_num).m_angularVelocity);
    bool onGround = false;
    for (int i = 0; i < vehicle->m_tires.size(); i++)
    {
#ifdef OLD_CDGRCC
        if (!vehicle->m_vehicleJoint->GetTireOnAir(vehicle->m_tires[i]->tire_num) &&
#endif
            vehicle->m_tires[i]->connected) // it is on the ground
        {
#ifdef OLD_CDGRCC
            CustomDGRayCastCar::Tire* tire = &vehicle->m_vehicleJoint->GetTire(vehicle->m_tires[i]->tire_num);
            NewtonBody* hitBody = tire->m_HitBody;
            float newFriction = 0.f;
            if (hitBody && NewtonBodyGetMaterialGroupID(hitBody) == roadID)
            {
                // here
                CMyRoad* road = (CMyRoad*)NewtonBodyGetUserData(hitBody);
                newFriction = vehicle->m_tires[i]->friction * road->getFrictionMulti();
                vehicle->m_tires[i]->hitRoad = true;
            }
            else
            {
                newFriction = vehicle->m_tires[i]->friction * vehicle->friction_multi;
                vehicle->m_tires[i]->hitRoad = false;
            }
            tire->m_groundFriction = newFriction;
#endif
            onGround = true;
            //break;
        }
    }

	dVector force;
	NewtonBodyGetForce(vehicle->GetRigidBody(), &force.m_x);
	dVector veloc;
	NewtonBodyGetVelocity(vehicle->GetRigidBody(), &veloc.m_x);
	//float veloct = veloc.m_x ? (veloc.m_z / veloc.m_x) : 0.0f;
	//float forcet = force.m_x ? (force.m_z / force.m_x) : 0.0f;
	float szam = ((veloc.m_x * force.m_x) + (veloc.m_z * force.m_z)) / 1000.0f;
	float szamy = fabsf((veloc.m_y * force.m_y) / 1000.0f);
	//dVector front(m_matrix[0], m_matrix[1], m_matrix[2], m_matrix[3]);
/*
    printf("%f %f %f %f %f\n", 
            szam,
            veloc.m_x,
            veloc.m_z,
            force.m_x,
            force.m_z
//            vehicle->m_vehicleJoint->GetTire(2).m_sideHit
            );
*/    
    if (onGround)
    {
        if (vehicle->setTransformCount - vehicle->lastOnGroundTime > 10)
        {
            vehicle->playSound(vehicle->puffSound, vehicle->m_matrix.getTranslation());
        }
        if (vehicle->groundSound)
        {
            if (!vehicle->lastOnGround || (tick - vehicle->lastSoundUpdateTime > SOUND_UPDATE_INTERVAL))
            {
                vehicle->groundSound->setPlaybackSpeed(soundSpeed);
                vehicle->groundSound->setPosition(matrix.getTranslation());
            }
            vehicle->groundSound->setIsPaused(false);
        }
        if (szam<-1.0f && szamy < 100.f &&
            vehicle->setTransformCount - vehicle->lastNotOnGroundTime > 10)
        {
            //if (szamy>100.0f)
            //    printf("%f %f\n", szam, szamy);
            if (vehicle->skidSound)
            {
                float vol = -szam;
                if (vol > 900)
                    vol = 1.0f;
                else
                    vol = vol / 900.f;

                if (tick - vehicle->lastSoundUpdateTime > SOUND_UPDATE_INTERVAL)
                {
#ifdef USE_MY_SOUNDENGINE
                    vehicle->skidSound->setVolume(vol*0.6f);
#else
                    vehicle->skidSound->setVolume(vol);
#endif
                    vehicle->skidSound->setPosition(matrix.getTranslation());
                    vehicle->skidSound->setIsPaused(false);
                }
            }
        }
        else
        {
            if (vehicle->skidSound)
            {
                vehicle->skidSound->setIsPaused(true);
            }
        }
        vehicle->lastOnGroundTime = vehicle->setTransformCount;
    }
    else
    {
        if (vehicle->groundSound)
        {
            vehicle->groundSound->setIsPaused(true);
        }
        if (vehicle->skidSound)
        {
            vehicle->skidSound->setIsPaused(true);
        }
        vehicle->lastNotOnGroundTime = vehicle->setTransformCount;
    }
    
	if (angularVelocity>2.f)
	{
        //if ((vehicle->setTransformCount%2) == 0)
        {
            for (int i = 0; i < vehicle->smoke_tires.size(); i++)
            {
#ifdef OLD_CDGRCC
                if (!vehicle->m_vehicleJoint->GetTireOnAir(vehicle->smoke_tires[i]->tire_num) &&
                    vehicle->smoke_tires[i]->connected && !vehicle->smoke_tires[i]->hitRoad) // it is on the ground
#endif
                {
                    if (useSmokes)
                    {
                        //vector3df tpos(vehicle->smoke_tires[i]->m_matrix.getTranslation());
                        //tpos.Y -= vehicle->smoke_tires[i]->m_radius;
                        vehicle->addSmoke(vehicle->getSpeed(), vehicle->smoke_tires[i]->m_matrix.getTranslation(), vehicle->smoke_tires[i]->m_radius);
                    }
                }
            }
        }
    }
    else
    {
        if (vehicle->groundSound)
        {
            vehicle->groundSound->setIsPaused(true);
        }
    }
    
    vehicle->lastOnGround = onGround;
    if (tick - vehicle->lastSoundUpdateTime > SOUND_UPDATE_INTERVAL)
    {
        vehicle->lastSoundUpdateTime = tick;
    }
/*
    if (vehicle->m_debug)
    {
        printf("NewtonRaceCar::SetTransform end\n");
    }
*/
}

void NewtonRaceCar::RaceCarTire::SetTransformTire (const NewtonBody* body, const float* matrixPtr, int threadIndex)
{
    if (newtonUpdateCount != 1) return;
	
	NewtonRaceCar::RaceCarTire* tire;

    pdprintf(printf("NewtonRaceCar::SetTransformTire\n"));
	// get the graphic object form the rigid body

	// set the transformation matrix for this rigid body
	matrix4 matrix;
	memcpy(matrix.pointer(), matrixPtr, sizeof(float)*16);
//    matrix.setM(matrixPtr);

	tire = (NewtonRaceCar::RaceCarTire*) NewtonBodyGetUserData (body);
    tire->setMatrix (matrix);
}

void NewtonRaceCar::setSteering(float value)
{
    m_steer = value;
}

void NewtonRaceCar::setSteeringKb(float value)
{
    //printf("skb: %f\n", value);
    m_steer_kb = value;
}

void NewtonRaceCar::applySteering(float value)
{
	dFloat vEngineSteerAngle = m_vehicleJoint->GenerateTiresSteerAngle(value) /* fabsf(value)*/;
	dFloat vEngineSteerForce = m_vehicleJoint->GenerateTiresSteerForce(vEngineSteerAngle) * fabsf(value);

	for (int i = 0; i < steer_tires.size(); i++)
	{
#ifdef OLD_CDGRCC
    	m_vehicleJoint->SetCustomTireSteerAngleForce
            (steer_tires[i]->tire_num, vEngineSteerAngle * steer_tires[i]->connectionStrength,
                                       vEngineSteerForce * steer_tires[i]->connectionStrength);
#endif
    }
}

void NewtonRaceCar::setTireTorque(float value)
{
    m_torqueReal = value;
    m_torque = value * torque_multi;
}

void NewtonRaceCar::applyTireTorque(float rvalue)
{
// init part
    float speed = getSpeed();
    float aspeed = fabsf(speed);
    float value = rvalue * torque_multi;
    float avalue = fabsf(rvalue);
    
    bool skip = false;
    bool brake = false;
    bool engine = true;

// 1. part: calculation

    if ((rvalue<-0.001f && speed>0.01f) || (rvalue>0.001f && speed<-0.01f))
    {
        brake = true;
    }
    
    if ( gear<=0 ||
         (gearLimits[gear-1].high+10.f < speed && gearLimits.length()!=gear) ||
         (speed < gearLimits[0].low && gear == 1)
       )
    {
        // empty gear or over run
        skip = true;
    }
    else
    {
        if ((speed < gearLimits[gear-1].low && gear != 1)) // to low RPM for higher gears
        {
            if (!autoGear)
            {
                if (joy_axis_clutch == -1) // no clucth her
                {
                    if (gearLimits[gear-1].low > speed+15.f) // do some auto clutch
                    {
                        engine = false;
                    }
                }
                else
                {
                    if (gearLimits[gear-1].low > speed+(20.f*(float)(gear-2))+(35.f*m_clutch))
                    {
                        if (m_clutch<0.7f)
                            engine = false;
                    }
                }
            }
            // no need else
        }
        if ((joy_axis_clutch != -1) && gear == 1 && !autoGear) // can engine turn off at low speed if clutch
        {
            //bool turnBrakeOff = false;
            if (5.f > aspeed + (10.f*m_clutch))
            {
                if (m_clutch<0.7f)
                {
                    engine = false;
                }
                else
                {
                    if (brake && engine)
                        brake = false; // turn off brakes to be able to start at low speed.
                }
            }
            //if (engine && brake && turnBrakeOff)
            //{
            //    brake = false; // turn off brakes to be able to start at low speed.
            //}
        }
        // else is the normal run
    }

// 2. part: analysis
    if (engine)
    {
        startEngine();
    }
    else
    {
        stopEngine();
    }
    
    if (brake)
    {
#ifdef OLD_CDGRCC
        dFloat vEngineBrake = m_vehicleJoint->GenerateTiresBrake(fabsf(rvalue));
#endif
        for (int i = 0; i < steer_tires.size(); i++)
        {
#ifdef OLD_CDGRCC
            m_vehicleJoint->SetCustomTireBrake
#endif
                (steer_tires[i]->tire_num, vEngineBrake);
        }
        switchBrake(true);
        return;
    }
    else
    {
        switchBrake(false);
    }
    
    if (!engine || skip) return;

// 3. part apply torque
    if (lastUpdateGearTime + changeGearTime > setTransformCount) value *= 0.2f;
//    printf("%f %d\n", value, joy_axis_clutch);

    if ((joy_axis_clutch != -1)) value *= (1.0f - m_clutch*m_clutch);
    
//    if (value/torque_multi<0.f && gear!=1 && getSpeed()<0.f) return;
#ifdef OLD_CDGRCC
	dFloat vEngineTorque = m_vehicleJoint->GenerateTiresTorque
#endif
        (value) * fabsf(value) * demage;
	//printf("%f\n", vEngineTorque);
	// Set the generate torque value
	for (int i = 0; i < torque_tires.size(); i++)
	{
#ifdef OLD_CDGRCC
        m_vehicleJoint->SetCustomTireTorque
#endif
            (torque_tires[i]->tire_num, vEngineTorque * torque_tires[i]->connectionStrength);
    }
}

void NewtonRaceCar::setHandBrakes (float value)
{
    m_brakes = value;
}

void NewtonRaceCar::applyHandBrakes(float value)
{
#ifdef OLD_CDGRCC
	dFloat vEngineBrake = m_vehicleJoint->GenerateTiresBrake(value);
#else
	dFloat vEngineBrake = value*5000.f;
#endif
	// Set the generate brake value
//	if (m_debug)
//	{
//        printf("engine brake %f\n", vEngineBrake);
//    }
	for (int i = 0; i < hb_tires.size(); i++)
	{
#ifdef OLD_CDGRCC
    	m_vehicleJoint->SetCustomTireBrake
#endif
            (hb_tires[i]->tire_num, vEngineBrake);
    }
}

NewtonRaceCar::RaceCarTire::RaceCarTire(ISceneManager* smgr, IVideoDriver* driver, NewtonRaceCar *p_root, int witchTire, FILE* f)
    : m_tireNode(0),
      shadowNode(0),
      connected(false),
      connectionStrength(0.f),
      m_newtonBody(0),
      root(p_root),
      hitRoad(false),
      offsetObject(0)
{
	int i;
	int ret;
	c8 meshfileName[256];
	c8 texturefileName[256];
	float d1, d2, d3;

	// find the geometry mode;
	IAnimatedMesh *mesh;

    ret = fscanf(f, "%s\n", meshfileName); // it is not really the mesh name it is a dummy
    if ( ret <=0 )
    {
        printf("error reading car file ret %d errno %d 1\n", ret, errno);
        fclose(f);
        return;
    }
    
    dprintf(printf("read tyre (%d): %s\n", witchTire, meshfileName));

    ret = fscanf(f, "tyre_mesh: %s\n", meshfileName);
    if ( ret <=0 )
    {
        printf("error reading car file ret %d errno %d 2\n", ret, errno);
        fclose(f);
        return;
    }
	mesh = smgr->getMesh(meshfileName);
	
	m_tireNode = smgr->addAnimatedMeshSceneNode(mesh);
	//m_tireNode->setMaterialFlag(video::EMF_LIGHTING, false);
	//m_tireNode->setMaterialFlag(video::EMF_LIGHTING, globalLight);
    if (useCgShaders)
        m_tireNode->setMaterialFlag(video::EMF_LIGHTING, false);
    else    
        m_tireNode->setMaterialFlag(video::EMF_LIGHTING, globalLight);
	if (globalLight)
	{
        //if (stencil_shadows)
            shadowNode = m_tireNode->addShadowVolumeSceneNode();
        m_tireNode->setMaterialFlag(video::EMF_NORMALIZE_NORMALS, true);
    }
    m_tireNode->setVisible(false);
    
    offsetObject = new OffsetObject(m_tireNode, true);

    ret = fscanf(f, "tyre_texture: %s\n", texturefileName);
    if ( ret <=0 )
    {
        printf("error reading car file ret %d errno %d 3\n", ret, errno);
        fclose(f);
        return;
    }
    if (useCgShaders)
        m_tireNode->setMaterialType((video::E_MATERIAL_TYPE)myMaterialType_light_tex_s_car_tyre);
//    else
//        m_tireNode->setMaterialType((video::E_MATERIAL_TYPE)myMaterialType_light_tex_s_car);
	m_tireNode->setMaterialTexture(0, driver->getTexture(texturefileName));
    m_tireNode->setMaterialTexture(1, car_dirttexture_array[root->current_car_dirt]);

    ret = fscanf(f, "tyre_scale: %f, %f, %f\n", &d1, &d2, &d3);
    if ( ret <=0 )
    {
        printf("error reading car file ret %d errno %d 4\n", ret, errno);
        fclose(f);
        return;
    }
    m_tireNode->setScale(vector3df(d1,d2,d3));

    ret = fscanf(f, "tyre_rotation: %f, %f, %f\n", &d1, &d2, &d3);
    if ( ret <=0 )
    {
        printf("error reading car file ret %d errno %d 7\n", ret, errno);
        fclose(f);
        return;
    }
    m_trafo.makeIdentity();
    m_trafo.setRotationDegrees(vector3df(d1,d2,d3));

    tire_num = witchTire;
	// find the radius and with of the tire
	m_width = 0.0f;
	m_radius = 0.0f;

	// use the main body part to create the the main collision geometry
    core::aabbox3d<f32> bbox = m_tireNode->getBoundingBox();
	m_width = (bbox.MaxEdge.Z-bbox.MinEdge.Z) * m_tireNode->getScale().Z;
	m_radius = (bbox.MaxEdge.Y-bbox.MinEdge.Y) * m_tireNode->getScale().Y * 0.5f;
    
    dprintf(printf("tyre widht %f, radius %f\n", m_width, m_radius));

    ret = fscanf(f, "tyre_local_position: %f, %f, %f\n", &tirePosition.X, &tirePosition.Y, &tirePosition.Z);
    if ( ret <=0 )
    {
        printf("error reading car file ret %d errno %d 5\n", ret, errno);
        fclose(f);
        return;
    }

    ret = fscanf(f, "tyre_mass: %f\nfriction: %f\nsuspension_length: %f\nsuspension_spring: %f\nsuspension_damper: %f\n", 
                     &tyre_mass, &friction_orig, &suspension_length_orig, &suspension_spring, &suspension_damper);
    if ( ret <=0 )
    {
        printf("error reading car file ret %d errno %d 6\n", ret, errno);
        fclose(f);
        return;
    }

    friction = friction_orig;
    suspension_length = suspension_length_orig;
    
	tirePosition.Y -= suspension_length;

/*    
    printf("A tire:\n   m_vidth %f m_radius %f \n", m_width, m_radius);
    printf("   tireSuspShock %f tireSuspSpring %f \n", tireSuspesionShock, tireSuspesionSpring);
    printf("   tireMass %f\n", tireMass);
    printf("   tirePos %f, %f, %f, %f\n", tirePosition[0], tirePosition[1], tirePosition[2], tirePosition[3]);
    printf("           %f, %f, %f, %f\n", tirePosition[4], tirePosition[5], tirePosition[6], tirePosition[8]);
    printf("           %f, %f, %f, %f\n", tirePosition[8], tirePosition[9], tirePosition[10], tirePosition[11]);
    printf("           %f, %f, %f, %f\n", tirePosition[12], tirePosition[13], tirePosition[14], tirePosition[15]);
*/

    ret = fscanf(f, "steer_tyres: %d\n", &i);
    if ( ret <=0 )
    {
        printf("error reading car file ret %d errno %d 5\n", ret, errno);
        fclose(f);
        return;
    }
    if (i > 0)
    {
        root->steer_tires.push_back(this);
    }
    
    ret = fscanf(f, "torque_tyres: %d\n", &i);
    if ( ret <=0 )
    {
        printf("error reading car file ret %d errno %d 5\n", ret, errno);
        fclose(f);
        return;
    }
    if (i > 0)
    {
        root->torque_tires.push_back(this);
    }

    ret = fscanf(f, "smoke_tyres: %d\n", &i);
    if ( ret <=0 )
    {
        printf("error reading car file ret %d errno %d 5\n", ret, errno);
        fclose(f);
        return;
    }
    if (i > 0)
    {
        root->smoke_tires.push_back(this);
    }

    ret = fscanf(f, "hb_tyres: %d\n", &i);
    if ( ret <=0 )
    {
        printf("error reading car file ret %d errno %d 5\n", ret, errno);
        fclose(f);
        return;
    }
    if (i > 0)
    {
        root->hb_tires.push_back(this);
    }
}

void NewtonRaceCar::RaceCarTire::activate(bool p_useOffset)
{
    m_tireNode->setVisible(true);
    if (shadowNode)
        addToShadowNodes(shadowNode);
    else
        addToShadowNodes(m_tireNode);
    addToDepthNodes(m_tireNode); // new depth

    float newFriction = friction * root->friction_multi;
	// add the tire and set this as the user data
    root->GetJoint()->AddSingleSuspensionTire (this,
        dVector(tirePosition.X,tirePosition.Y,tirePosition.Z),
        tyre_mass,
        m_radius, m_width,
        newFriction,
        suspension_length, suspension_spring, suspension_damper,
        1
        );

    {
        matrix4 tirePositionM;
        tirePositionM.setTranslation(tirePosition);
        matrix4 tireMatrix = root->m_matrix * tirePositionM * m_trafo;
        setMatrix(tireMatrix);
    }
    
    connected = true;
    connectionStrength = 1.f;
    if (p_useOffset && offsetObject)
    {
        offsetManager->addObject(offsetObject);
    }
}

void NewtonRaceCar::RaceCarTire::deactivate()
{
    m_tireNode->setVisible(false);
    if (shadowNode)
        removeFromShadowNodes(shadowNode);
    else
        removeFromShadowNodes(m_tireNode);
    removeFromDepthNodes(m_tireNode); // new depth
    if (offsetObject)
    {
        offsetObject->setBody(0);
        offsetManager->removeObject(offsetObject);
    }

}

void NewtonRaceCar::RaceCarTire::setMatrix(matrix4& newMatrix)
{
    pdprintf(printf("NewtonRaceCar::RaceCarTire::setMatrix\n"));
    m_matrix = newMatrix*m_trafo;
	m_tireNode->setPosition(m_matrix.getTranslation());		// set position
    m_tireNode->setRotation(m_matrix.getRotationDegrees());	// and rotation
}

void NewtonRaceCar::RaceCarTire::setMatrixWithNB(matrix4& newMatrix)
{
    pdprintf(printf("NewtonRaceCar::RaceCarTire::setMatrixWithNB\n"));
    NewtonBodySetMatrix(m_newtonBody, newMatrix.pointer());
    m_matrix = newMatrix*m_trafo;
	m_tireNode->setPosition(m_matrix.getTranslation());		// set position
    m_tireNode->setRotation(m_matrix.getRotationDegrees());	// and rotation
}

void NewtonRaceCar::RaceCarTire::disconnect(const vector3df& force, NewtonWorld* nWorld)
{
    dprintf(printf("NewtonRaceCar::RaceCarTire::disconnect, connected: %d\n", connected));
    
    if (!connected) return;
    NewtonCollision* tyreCollision;
    
    tyreCollision = NewtonCreateCylinder(nWorld, m_radius * 0.9f, m_width, treeID, NULL);
    
	//create the rigid body
	m_newtonBody = NewtonCreateBody (nWorld, tyreCollision);
	//printf("create tyre body %p\n", m_newtonBody);

	// save the pointer to the graphic object with the body.
	NewtonBodySetUserData (m_newtonBody, this);

	// set the material group id for vehicle
	NewtonBodySetMaterialGroupID (m_newtonBody, treeID);

	// set a destructor for this rigid body
	NewtonBodySetDestructorCallback (m_newtonBody, DestroyTire);

	// set the transform call back function
	NewtonBodySetTransformCallback (m_newtonBody, SetTransformTire);

	// set the force and torque call back function
	NewtonBodySetForceAndTorqueCallback (m_newtonBody, ApplyGravityForce);
	
	dVector origin;
	dVector inertia;

	float Ixx;
	float Iyy;
	float Izz;

	// calculate the moment of inertia and the relative center of mass of the solid
	NewtonConvexCollisionCalculateInertialMatrix (tyreCollision, &inertia[0], &origin[0]);	

	Ixx = tyre_mass * inertia[0];
	Iyy = tyre_mass * inertia[1];
	Izz = tyre_mass * inertia[2];
    
    NewtonBodySetMassMatrix (m_newtonBody, tyre_mass, Ixx, Iyy, Izz);
    
    NewtonBodySetCentreOfMass (m_newtonBody, &origin[0]);
    
    //setMatrixWithNB(mat);
    NewtonBodySetMatrix(m_newtonBody, m_matrix.pointer());

    NewtonBodySetAutoSleep (m_newtonBody, 0);
    
    NewtonReleaseCollision (nWorld, tyreCollision);
    connected = false;
    
    connectionStrength = 0.f;

//    NewtonBodySetForce (m_newtonBody, &force.X);
    NewtonBodySetVelocity(m_newtonBody, &force.X);
    dprintf(printf("NewtonRaceCar::RaceCarTire::disconnect end\n"));

    if (offsetObject)
    {
        offsetObject->setBody(m_newtonBody);
    }
}

CustomDGRayCastCar* NewtonRaceCar::GetJoint() const
{
	return m_vehicleJoint;
}

NewtonBody* NewtonRaceCar::GetRigidBody() const
{
	return m_vehicleBody;
}

float NewtonRaceCar::getSpeed() const
{
	dVector veloc;
	NewtonBodyGetVelocity(m_vehicleBody, &veloc[0]);
	
	dVector front(m_matrix[0], m_matrix[1], m_matrix[2], m_matrix[3]);

//veloc[2] = 0.1f;
//NewtonBodySetVelocity(m_vehicleBody, &veloc[0]);
 	return (front % veloc);//*2.f/*/1.5f/*veloc[0] + veloc[1] + veloc[2]*/;
}

void NewtonRaceCar::setMatrix(matrix4& newMatrix)
{
    m_matrix = newMatrix*m_trafo;
    m_node->setPosition(m_matrix.getTranslation());		// set position
    m_node->setRotation((m_matrix).getRotationDegrees());	// and rotation
    if (m_debug && bb_node)
    {
        bb_node->setPosition(m_matrix.getTranslation());		// set position
        bb_node->setRotation((m_matrix).getRotationDegrees());	// and rotation
    }
}

void NewtonRaceCar::setMatrixWithNB(matrix4& newMatrix)
{
	 NewtonBodySetMatrix(m_vehicleBody, newMatrix.pointer());
     m_matrix = newMatrix*m_trafo;
  	 m_node->setPosition(m_matrix.getTranslation());		// set position
     m_node->setRotation((m_matrix).getRotationDegrees());	// and rotation
    if (m_debug && bb_node)
    {
        bb_node->setPosition(m_matrix.getTranslation());		// set position
        bb_node->setRotation((m_matrix).getRotationDegrees());	// and rotation
    }
}

void NewtonRaceCar::setMatrixWithNB2(vector3df& newPos, vector3df& newRot)
{
    matrix4 mat;
    mat.setTranslation(newPos);
    mat.setRotationDegrees(newRot);
    setMatrixWithNB(mat);
}

matrix4& NewtonRaceCar::getMatrix()
{
//    m_matrix
//    m_matrix.setTranslation(m_node->getPosition());
//    m_matrix.setRotationDegrees(m_node->getRotation());
    return m_matrix;
}

void NewtonRaceCar::setControl()
{
    m_debug = 1 - m_debug;
    if (bb_node)
        bb_node->setVisible(m_debug);
    dprintf(printf("set debug: %d\n", m_debug));
//    NewtonInvalidateCache(nWorld);
//	NewtonBodySetTransformCallback (m_vehicleBody, SetTransform);
}

void NewtonRaceCar::addSmoke(const float speed, const vector3df &pos, float offset)
{
    int ind = 0;
    
    if (!smokes) return;
    //printf("add smoke\n");
    for(;ind<MAX_SMOKES;ind++)
        if (smokes[ind]==0 || smokes[ind]->animePhase==-1) break;
    
    if (ind==MAX_SMOKES)
    {
        //printf("add smoke end 1\n");
        return;
    }
    
    if (smokes[ind]==0)
        smokes[ind] = new Smoke(smgr, driver, speed, pos, offset, waterHeight);
    else
        smokes[ind]->renew(smgr, driver, speed, pos, offset, waterHeight);
    //printf("add smoke end 2\n");
}

void NewtonRaceCar::updateSmoke()
{
    int ind = 0;

    if (!smokes) return;
    //printf("update smokes\n");
    
    for(;ind<MAX_SMOKES;ind++)
        if (smokes[ind])
        {
            if (smokes[ind]->animePhase < MAX_SMOKES/3)
            {
                //printf("update smoke\n");
                core::dimension2d<f32> size(1.f,1.f);
                float scale = (smokes[ind]->speed/100.f);

                //size = smokes[ind]->node->getSize();
                size *= smokes[ind]->animePhase * scale;
                smokes[ind]->animePhase++;
                smokes[ind]->node->setSize(size);
                smokes[ind]->node->getMaterial(0).MaterialTypeParam2 = 1.0f - ((float)smokes[ind]->animePhase/(float)(MAX_SMOKES/3));
            }
            else
            {
                //printf("delete smoke\n");
                //smokes[ind]->node->remove();
                //delete smokes[ind];
                //smokes[ind] = 0;
                smokes[ind]->animePhase = -1;
                smokes[ind]->node->setVisible(false);
            }
        }
    //printf("update smokes end\n");
}

NewtonRaceCar::Smoke::Smoke(ISceneManager* smgr, IVideoDriver* driver,
                     const float pspeed, const vector3df &pos, float offset, float p_waterHeight) :
    speed(fabsf(pspeed)), animePhase(0), offsetObject(0)
{
    //printf("add smoke\n");
    float addrX = (float)((rand()%20) - 10) / 20.0f;
    float addrZ = (float)((rand()%20) - 10) / 20.0f;

    node = smgr->addBillboardSceneNode(0, core::dimension2d<f32>(0.2, 0.2), vector3df(pos.X+addrX, pos.Y, pos.Z+addrZ));
	//node->setMaterialFlag(video::EMF_LIGHTING, false);
	node->setMaterialType((video::E_MATERIAL_TYPE)myMaterialType_smoke/*video::EMT_TRANSPARENT_ALPHA_CHANNEL*/);
    //node->getMaterial(0).MaterialTypeParam = 0.5f;
    if (useShaders)
        node->setMaterialFlag(video::EMF_LIGHTING, false);
    else
        node->setMaterialFlag(video::EMF_LIGHTING, globalLight);
    
    //node->getMaterial(0).TextureLayer[0].AnisotropicFilter = true;
    //node->getMaterial(0).TextureLayer[0].BilinearFilter = false;
    //if (driverType == video::EDT_OPENGL)
    //{
    //    node->getMaterial(0).setFlag(EMF_BILINEAR_FILTER, false);
    //}
    //if (globalLight)
    //{
    //    node->setMaterialFlag(video::EMF_NORMALIZE_NORMALS, true);
    //}
    //node->getMaterial(0).TextureLayer[0].TextureWrap = video::ETC_CLAMP;
    //node->getMaterial(0).MaterialTypeParam = 0.9f;
    node->getMaterial(0).MaterialTypeParam2 = 1.0f;
    if (pos.Y - offset > p_waterHeight)
    {
	   node->setMaterialTexture(0, smokeTexture);
    }
    else
    {
	   node->setMaterialTexture(0, smokeWaterTexture);
    }
}

void NewtonRaceCar::Smoke::renew(ISceneManager* smgr, IVideoDriver* driver,
                     const float pspeed, const vector3df &pos, float offset, float p_waterHeight)
{
    //printf("add smoke\n");
    float addrX = (float)((rand()%20) - 10) / 20.0f;
    float addrZ = (float)((rand()%20) - 10) / 20.0f;
    
    speed = fabsf(pspeed);
    animePhase = 0;

    node->getMaterial(0).MaterialTypeParam2 = 1.0f;
    node->setSize(core::dimension2d<f32>(0.2, 0.2));
    node->setPosition(vector3df(pos.X+addrX, pos.Y, pos.Z+addrZ));
    node->setVisible(true);
    if (pos.Y - offset > p_waterHeight)
    {
	   node->setMaterialTexture(0, smokeTexture);
    }
    else
    {
	   node->setMaterialTexture(0, smokeWaterTexture);
    }
}

#ifdef USE_MY_SOUNDENGINE
void NewtonRaceCar::playSound(CMySound* sound, const vector3df& pos)
{
    if (sound)
    {
        sound->setPosition(pos);
        sound->play();
    }
}
#else
void NewtonRaceCar::playSound(irrklang::ISound* sound, const vector3df& pos)
{
    if (sound)
        soundEngine->play3D(sound->getSoundSource(), pos);
}
#endif

void NewtonRaceCar::pause()
{
    if (engineSound)
        engineSound->setPlaybackSpeed(1.f);
    if (engineSound && engineGoing)
        engineSound->setIsPaused(true);

    if (groundSound)
        groundSound->setPlaybackSpeed(1.f);
    if (groundSound)
        groundSound->setIsPaused(true);

    if (skidSound)
        skidSound->setIsPaused(true);
}

void NewtonRaceCar::resume()
{
    if (engineSound && engineGoing)
        engineSound->setIsPaused(false);
    if (groundSound)
        groundSound->setIsPaused(false);
}

void NewtonRaceCar::startEngine()
{
    if (!engineGoing)
    {
        if (engineSound)
        {
            engineSound->setIsPaused(false);
        }
        engineGoing = true;
    }
}
 
void NewtonRaceCar::stopEngine()
{
    if (engineGoing)
    {
        if (engineSound)
            engineSound->setIsPaused(true);
        engineGoing = false;
    }
}

void NewtonRaceCar::reset(const core::vector3df& newpos)
{
    matrix4 mat = m_matrix;
    core::vector3df rot = mat.getRotationDegrees() - m_trafo.getRotationDegrees();
    dprintf(printf("reset car: orig rot: %f %f %f\n", rot.X, rot.Y, rot.Z));
    if (fabsf(rot.Z-180.f) < 90.f)
    {
        if (rot.Y < 90.f)
            rot.Y = 180.f - rot.Y;
        if (rot.Y > 270.f)
            rot.Y = 540.f - rot.Y;
    }
    rot.Z = rot.X = 0.f;
    dprintf(printf("reset car:  mod rot: %f %f %f\n", rot.X, rot.Y, rot.Z));
    mat.setTranslation(newpos);
    // vector3df(0.f, rot.Y, 0.f)
    mat.setRotationDegrees(rot);
    setMatrixWithNB(mat);
}

float NewtonRaceCar::getAngle()
{
    core::vector3df rot = m_matrix.getRotationDegrees() - m_trafo.getRotationDegrees();
    //dprintf(printf("reset car: orig rot: %f %f %f\n", rot.X, rot.Y, rot.Z));
    if (fabsf(rot.Z-180.f) < 90.f)
    {
        if (rot.Y < 90.f)
            rot.Y = 180.f - rot.Y;
        if (rot.Y > 270.f)
            rot.Y = 540.f - rot.Y;
    }
    rot.Z = rot.X = 0.f;
    //dprintf(printf("reset car:  mod rot: %f %f %f\n", rot.X, rot.Y, rot.Z));
    return rot.Y;
}

void NewtonRaceCar::updateGear(int pgear, bool needClutch)
{
    if (pgear==-2 && autoGear)
    {
        int realGear = gear - 1;
        
        if (realGear < 0 || realGear >= gearLimits.length()) 
        {
            realGear=0;
            gear = 1;
        }
        
        float speed = getSpeed();
        
        if (gearLimits[realGear].low <= speed && speed <= gearLimits[realGear].high) return;
        
        if (gearLimits[realGear].low > speed)
        {
            if (realGear > 0)
            {
                gear--;
                realGear--;
            }
            else
                return;
        }
        else
        if (gearLimits[realGear].high < speed)
        {
            if (realGear < gearLimits.length()-1)
            {
                gear++;
                realGear++;
            }
            else
                return;
        }
        //printf("ch gear %d: %f .. %f, %f %f\n", gear,
        //        gearLimits[realGear].low, gearLimits[realGear].high,
        //        gearLimits[realGear].max_torque, gearLimits[realGear].max_torque_rate);    
        lastUpdateGearTime = setTransformCount;
#ifdef OLD_CDGRCC
    	m_vehicleJoint->SetVarMaxTorque(gearLimits[realGear].max_torque);
    	m_vehicleJoint->SetVarMaxTorqueRate(gearLimits[realGear].max_torque_rate);
#endif
    }
    else
    {
        if (pgear == gear) return;
        int realGear = pgear - 1;
        
        if (realGear < 0)
        {
            gear=0;
            return;
        }
        if (realGear >= gearLimits.length())
        {
            return;
        }

        if (needClutch && joy_axis_clutch!=-1 && m_clutch<0.4f)
        {
            gear=0;
            return;
        }

        if (!needClutch)
            lastUpdateGearTime = setTransformCount;

        gear = pgear;
#ifdef OLD_CDGRCC
    	m_vehicleJoint->SetVarMaxTorque(gearLimits[realGear].max_torque);
    	m_vehicleJoint->SetVarMaxTorqueRate(gearLimits[realGear].max_torque_rate);
#endif
    }
}

void NewtonRaceCar::updateDirt()
{
    if (current_car_dirt < MAX_CAR_DIRT - 1)
    {
        current_car_dirt++;
        m_node->setMaterialTexture(1, car_dirttexture_array[current_car_dirt]);
        for (int i = 0; i<m_tires.size();i++)
            m_tires[i]->m_tireNode->setMaterialTexture(1, car_dirttexture_array[current_car_dirt]);
    }
}

void NewtonRaceCar::switchLight()
{
    cLight = 1-cLight;
}

void NewtonRaceCar::switchBrake(bool brake)
{
    if (brake == prevBrake) return;
    
    prevBrake = brake;
    
    if (brake)
    {
        carTex |= 1;
    }
    else
    {
        carTex &= ~1;
    }

    if (carTexs[carTex])
        m_node->setMaterialTexture(0, carTexs[carTex]);
}

void NewtonRaceCar::collide(const vector3df &point, const vector3df &direction,
                            float speed, float demageMultip, float amortMultip)
{
    if (!use_demage) return;
    vector3df dir = direction;
    dir = dir.normalize() * (0.2f + (speed * 0.008f));
    
    vector3df force;
    NewtonBodyGetForce(m_vehicleBody, &force.X);
    
    float len = dir.getLength();
    float len2 = direction.getLength();
    vector3df pos = point  - m_node->getPosition();
//    vector3df rot = m_node->getRotation();
    matrix4 mat = m_matrix;
    core::vector3df rot = mat.getRotationDegrees() - m_trafo.getRotationDegrees();
    if (fabsf(rot.Z-180.f) < 90.f)
    {
        rot.Z = rot.X = 180.f;
    }
    else
    {
        rot.Z = rot.X = 0.f;
    }

    vector3df rotpos;
    rot.invert();
    vector3df postmp = point;
//    core::matrix4 mat = m_node->getAbsoluteTransformation();
    core::matrix4 matrot;
    core::matrix4 matpos;
    core::matrix4 matdir;
    core::matrix4 matrotpos;
    core::matrix4 matrotdir;
    core::matrix4 matposrot;
    core::matrix4 matposrotin1;

    matpos.setTranslation(pos);
    matrot.setRotationDegrees(rot);

    matrotpos = matrot*matpos;

    matposrot = matpos*matrot;
    matposrotin1.setTranslation(pos);
    matposrotin1.setRotationDegrees(rot);
    
//    core::matrix4 pos;
//    pos.setTranslation(point);
    
    IMesh* mesh;
    if (((IAnimatedMeshSceneNode*)m_node)->getMesh()->getFrameCount())
        mesh = (IMesh*)((IAnimatedMeshSceneNode*)m_node)->getMesh()->getMesh(0);
    else
        mesh = (IMesh*)((IAnimatedMeshSceneNode*)m_node)->getMesh();
        
    dprintf(printf("collision mesh: %p (bc %u)\n", mesh, mesh->getMeshBufferCount()));
    dprintf(printf("collision speed: %f\n", speed));
    dprintf(printf("collision len: %f\n", len));
    dprintf(printf("collision len2: %f\n", len2));
    dprintf(printf("collision force %f %f %f len: %f\n", force.X, force.Y, force.Z, force.getLength()));
    dprintf(printf("collision point: %f %f %f\n", point.X, point.Y, point.Z));
    dprintf(printf("collision pos: %f %f %f\n", pos.X, pos.Y, pos.Z));
    rotpos = matrotpos.getTranslation();
    rotpos.Y = fabsf(rotpos.Y);
    dprintf(printf("collision rotpos: %f %f %f\n", rotpos.X, rotpos.Y, rotpos.Z));
    dprintf(printf("collision dir: %f %f %f\n", dir.X, dir.Y, dir.Z));
    matdir.setTranslation(dir);
    matrotdir = matrot*matdir;
    dir = matrotdir.getTranslation();
    dprintf(printf("collision rotdir: %f %f %f\n", dir.X, dir.Y, dir.Z));
    
    for (int i = 0; i < mesh->getMeshBufferCount();i++)
    {
        IMeshBuffer* mb = mesh->getMeshBuffer(i);
        //printf("collision %u. mb vertex count: %u type: %u\n", i, mb->getVertexCount(), mb->getVertexType());
        if (EVT_STANDARD != mb->getVertexType()) continue;
        for (int j = 0; j < mb->getVertexCount(); j++)
        {
            float dist = mb->getPosition(j).getDistanceFrom(rotpos);
            if (mb->getPosition(j).getDistanceFrom(rotpos) < len /*10.f*/)
            {
                vector3df amort = dir * ((len - dist) * 1.5f * amortMultip/** (len - dist)*/);
                if (amort.getLength() > 0.25f)
                {
                    amort.setLength(0.25f);
                }
                //printf("collision amort: %f %f %f\n", amort.X, amort.Y, amort.Z);
                //printf("collision %u. vert: %f %f %f  dist: %f\n", j, mb->getPosition(j).X, mb->getPosition(j).Y, mb->getPosition(j).Z, dist);
                video::S3DVertex* vertex = (video::S3DVertex*)((char*)mb->getVertices()+j*sizeof(video::S3DVertex));
                vertex->Pos += amort;
            }
            
        }
        mb->setDirty(EBT_VERTEX);
    }

    demage -= (speed / 500.f) * demageMultip;
    if (demage < 0.f) demage = 0.f;
    
    for (int i = 0; i < m_tires.size(); i++)
    {
        float dist = m_tires[i]->m_tireNode->getPosition().getDistanceFrom(point); // compare to the non transformed point
        if (m_tires[i]->connected && dist < len * 2)
        {
            demage -= (dist / 500.f) * demageMultip;
            if (demage < 0.f) demage = 0.f;
            m_tires[i]->connectionStrength -= (dist / 3.f) * demageMultip;
            if (demage < 0.f) demage = 0.f;
            
            if (m_tires[i]->connectionStrength < 0.f)
            {
                loseTyre(i);
            }
            
            dprintf(printf("collision %d tire demage: connectionStrength %f\n", i, m_tires[i]->connectionStrength));
            
        }
    }

    dprintf(printf("collision demage: %f\n", 100.f - (demage * 100.f)));
    mesh->setDirty(EBT_VERTEX);
}

void NewtonRaceCar::repair()
{
    IMesh* mesh;
    if (((IAnimatedMeshSceneNode*)m_node)->getMesh()->getFrameCount())
        mesh = (IMesh*)((IAnimatedMeshSceneNode*)m_node)->getMesh()->getMesh(0);
    else
        mesh = (IMesh*)((IAnimatedMeshSceneNode*)m_node)->getMesh();

    char* cur = (char*)omb;
    
    for (int i = 0; i < mesh->getMeshBufferCount() && cur; i++)
    {
        int copySize = mesh->getMeshBuffer(i)->getVertexCount()*sizeof(video::S3DVertex);
        memcpy(mesh->getMeshBuffer(i)->getVertices(), cur, copySize);
        cur = cur + copySize;
    }

    mesh->setDirty(EBT_VERTEX);
    
    for (int i = 0; i < m_tires.size(); i++)
    {
        if (!m_tires[i]->connected && m_tires[i]->m_newtonBody)
        {
            //delete m_tires[i]->m_newtonBody;
            NewtonDestroyBody(nWorld, m_tires[i]->m_newtonBody);
            m_tires[i]->m_newtonBody = 0;
            if (m_tires[i]->offsetObject)
            {
                m_tires[i]->offsetObject->setBody(0);
            }

        }
        m_tires[i]->connected = true;
        m_tires[i]->connectionStrength = 1.f;
        if (m_vehicleJoint)
        {
            m_vehicleJoint->GetTire(m_tires[i]->tire_num).m_suspensionLenght = m_tires[i]->suspension_length;
            //m_vehicleJoint->GetTire(m_tires[i]->tire_num).m_mass = m_tires[i]->tyre_mass;
            m_vehicleJoint->SetTireMassAndRadius(m_tires[i]->tire_num, m_tires[i]->tyre_mass, m_vehicleJoint->GetTire(m_tires[i]->tire_num).m_radius);
        }
    }

    demage = 1.f;
    
    dprintf(printf("\nrepair\n\n"));
}

void NewtonRaceCar::loseTyre(int num)
{
    if (num < 0 || num >= m_tires.size()) return;
    
    vector3df veloc;
	NewtonBodyGetVelocity(m_vehicleBody, &veloc.X);
    
    m_tires[num]->disconnect(veloc, nWorld);
    m_vehicleJoint->GetTire(m_tires[num]->tire_num).m_suspensionLenght = m_tires[num]->suspension_length * 0.1f;
    m_vehicleJoint->SetTireMassAndRadius(m_tires[num]->tire_num, m_tires[num]->tyre_mass * 0.1f, m_vehicleJoint->GetTire(m_tires[num]->tire_num).m_radius);
    //m_vehicleJoint->GetTire(m_tires[num]->tire_num).m_mass = m_tires[num]->tyre_mass * 0.1f;
}

void NewtonRaceCar::OnRegisterSceneNode()
{
    m_node->OnRegisterSceneNode();
    for (int i = 0; i < m_tires.size(); i++)
        m_tires[i]->m_tireNode->OnRegisterSceneNode();
}

void NewtonRaceCar::render()
{
    m_node->render();
    for (int i = 0; i < m_tires.size(); i++)
        m_tires[i]->m_tireNode->render();
}

void NewtonRaceCar::offsetObjectCallback(void* userData, const irr::core::vector3df& newPos)
{
    NewtonRaceCar* vehicle = (NewtonRaceCar*)userData;
    vehicle->m_matrix.setTranslation(newPos);
}

void NewtonRaceCar::setPressure(float pressure_multi)
{
    float lmass_multi = TYRE_MASS_GET_FROM_MULTI(pressure_multi);
    float lradius_multi = TYRE_RADIUS_GET_FROM_MULTI(pressure_multi);
    float lfriction_multi = TYRE_FRICTION_GET_FROM_MULTI(1.f-pressure_multi);
    for (unsigned int i = 0; i < m_tires.size(); i++)
    {
        m_vehicleJoint->SetTireMassAndRadius(m_tires[i]->tire_num, m_tires[i]->tyre_mass * lmass_multi, m_tires[i]->m_radius*lradius_multi);
        m_tires[i]->friction = m_tires[i]->friction_orig * lfriction_multi;
    }
}

void NewtonRaceCar::setSuspensionSpring(float ss_multi)
{
    for (unsigned int i = 0; i < m_tires.size(); i++)
    {
        m_vehicleJoint->GetTire(m_tires[i]->tire_num).m_springConst = m_tires[i]->suspension_spring * SUSPENSION_SPRING_GET_FROM_MULTI(ss_multi);
    }
}

void NewtonRaceCar::setSuspensionDamper(float sd_multi)
{
    for (unsigned int i = 0; i < m_tires.size(); i++)
    {
        m_vehicleJoint->GetTire(m_tires[i]->tire_num).m_springDamper = m_tires[i]->suspension_damper * SUSPENSION_DAMPER_GET_FROM_MULTI(sd_multi);
    }
}

void NewtonRaceCar::setSuspensionLength(float sl_multi)
{
    for (unsigned int i = 0; i < m_tires.size(); i++)
    {
        m_tires[i]->suspension_length = m_tires[i]->suspension_length_orig * SUSPENSION_LENGTH_GET_FROM_MULTI(sl_multi);
        if (m_tires[i]->connected)
        {
            m_vehicleJoint->GetTire(m_tires[i]->tire_num).m_suspensionLenght = m_tires[i]->suspension_length;
        }
    }
}

//#endif
