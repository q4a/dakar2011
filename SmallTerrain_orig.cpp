/****************************************************************
*                                                               *
*    Name: SmallTerrains.cpp                                    *
*                                                               *
*    Creator: Balazs Tuska                                      *
*                                                               *
*    Description:                                               *
*       This file contains the real terrain code. The map       *
*       consist of many small terrain to be fast.               *
*                                                               *
****************************************************************/

#include "SmallTerrain.h"
#include "Materials.h"
#include <math.h>
#include "my_shaders.h"
#include "settings.h"
#include "gameplay.h"

#ifdef __linux__
#include "linux_includes.h"
#endif

#define MAX_OBJECT_NUM 10000
#define MAX_GRASS_NUM 15000
#define MAX_ROAD_NUM 50
#define MAX_LOD 5

SmallTerrain::SmallTerrain(const c8* heightmapName,
                           const c8* terrainTextureName,
                           const c8* detailmapName,
                           const c8* roadfileName,
                           const c8* objectfileName,
                           ISceneManager* smgr, IVideoDriver* driver, NewtonWorld* pnWorld,
                           const int x, const int y,
                           const int max_x, const int max_y,
                           int pstHeightmapSize, float ptScale)
           : nWorld(pnWorld), terrain(0), body(0),
             visible(false), collision(0), stHeightmapSize(pstHeightmapSize), tScale(ptScale),
             /*roadMeshes(0),*/ roadWrappers(0), numOfRoads(0),
             /*objectMeshes(0),*/ objectWrappers(0), numOfObjects(0),
             grassWrappers(0), numOfGrasses(0), wasCalculated(true)
{
    core::vector3df loc((float)x*SMALLTERRAIN_SIZE, 0.f, (float)y*SMALLTERRAIN_SIZE);
    core::vector3df terrainScale(TERRAIN_SCALE, 1.0f, TERRAIN_SCALE);	// scale
    collision = NewtonCreateTreeCollision(nWorld, levelID/*, NULL*/);
    NewtonTreeCollisionBeginBuild(collision);

///////////////// BEGIN SMALL ////////////////////    
	// add terrain scene node
	terrain = smgr->addTerrainSceneNode(
		heightmapName,
		0,					// parent node
		-1,					// node id
//		core::vector3df(0.f, 0.f, 0.f),		// position
		loc,		// position
		core::vector3df(0.f, 0.f, 0.f),		// rotation
//		core::vector3df(1.f, 1.f, 1.f),	// scale
		terrainScale,	// scale
//		core::vector3df(TERRAIN_SCALE, 1.f, TERRAIN_SCALE),	// scale
		video::SColor ( 255, 255, 255, 255 ),	// vertexColor
		MAX_LOD,					// maxLOD
		scene::ETPS_17,				// patchSize
		0,/*8*/					// smoothFactor
		false,                    // addAtFault
		x*SMALLTERRAIN_HEIGHTMAP_SIZE, // x offset
		y*SMALLTERRAIN_HEIGHTMAP_SIZE, // y offset
		SMALLTERRAIN_HEIGHTMAP_SIZE+1    // small size
		);
    
	terrain->setMaterialFlag(video::EMF_LIGHTING, false);
/*	terrain->setMaterialFlag(video::EMF_LIGHTING, globalLight);
	if (globalLight)
	{
	   terrain->setMaterialFlag(video::EMF_NORMALIZE_NORMALS, true);
    }
*/
    // calculate image part from the texture
    
    IImage* fullImage = driver->createImageFromFile(terrainTextureName);
    int sizeX = fullImage->getDimension().Width / max_x;
    int sizeY = fullImage->getDimension().Height / max_y;
    IImage* partImage = //fullImage;
                        driver->createImage(fullImage,
                                            core::position2d<s32>((/*max_x-1-*/x)*sizeX, (/*max_y-1-*/y)*sizeY),
#ifdef IRRLICHT_SDK_15
                                            core::dimension2d<s32>(sizeX,sizeY));
#else
                                            core::dimension2d<u32>(sizeX,sizeY));
#endif
    for (int i = 0; i < sizeX/2; i++)
        for (int j = 0; j<sizeY; j++)
        {
            SColor tmp_col = partImage->getPixel(i, j);
            partImage->setPixel(i, j, partImage->getPixel(sizeX-1-i, j));
            partImage->setPixel(sizeX-1-i, j, tmp_col);
        }

    sprintf(terrainPartTextureName, "%s_%d_%d", terrainTextureName, x, y);
    printf("terrainPartTextureName: %s sizeX: %d, sizeY: %d, pi: %p\n", terrainPartTextureName, sizeX, sizeY, partImage);

	terrain->setMaterialTexture(0,
//    		driver->getTexture(terrainTextureName)
			driver->addTexture(terrainPartTextureName, partImage)
            );
			// data/terrain-texture.jpg data/sand.bmp
	terrain->setMaterialTexture(1,
//			driver->getTexture("data/detailmap3.jpg"));
			driver->getTexture(detailmapName));
	//terrain->setMaterialType(video::EMT_DETAIL_MAP);
	if (useShaders)
	   terrain->setMaterialType((video::E_MATERIAL_TYPE)myMaterialType_light_2tex);
	else
    	terrain->setMaterialType(video::EMT_DETAIL_MAP);
	terrain->scaleTexture(1.0f, SMALLTERRAIN_HEIGHTMAP_SIZE/*(float)2*/);
	terrain->setVisible(false);
	for (int i = 0; i<MAX_LOD; i++)
	{
        terrain->overrideLODDistance(i, LOD_distance*TERRAIN_SCALE*(1 + i + (i/2)));
    }
	
	//terrain->setDebugDataVisible ( true );
	// create triangle selector for the terrain	
/*
	scene::ITriangleSelector* selector
		= smgr->createTerrainTriangleSelector(terrain, 0);
	terrain->setTriangleSelector(selector);

	// create collision response animator and attach it to the camera
	scene::ISceneNodeAnimator* anim = smgr->createCollisionResponseAnimator(
		selector, camera, core::vector3df(60,100,60),
		core::vector3df(0,0,0),
		core::vector3df(0,50,0));
	selector->drop();
	camera->addAnimator(anim);
	anim->drop();
*///return;
    float maxes[3] = {0.f,0.f,0.f};
//////////////////
    printf("map build begin %d\n", terrain->getMesh()->getMeshBufferCount());
    {
	    int j;
        int v1i, v2i, v3i;
//        IMeshBuffer *mb;
        float vArray[9]; // vertex array (3*3 floats)
        int pols = 0;
        int step = 1;
	
        scene::CDynamicMeshBuffer* mb = new scene::CDynamicMeshBuffer(video::EVT_2TCOORDS, video::EIT_16BIT);
        terrain->getMeshBufferForLOD(*mb, 0);

		video::S3DVertex2TCoords* mb_vertices = (irr::video::S3DVertex2TCoords*)mb->getVertices();
		u16* mb_indices  = mb->getIndices();

        printf("index count: %d vertex count %d\n", mb->getIndexCount(), mb->getVertexCount());
        if (mb->getVertexCount()>= 256*256)
        {
            step = 2;
        }
        printf("using step = %u\n", step);
		// add each triangle from the mesh
		for (j=0; j<mb->getIndexCount()*step; j+=3*step)
		{
//            printf("pol: %d\n", j);
            pols++;
            if (step == 1)
            {
                v1i = mb_indices[j];
                v2i = mb_indices[j+1];
                v3i = mb_indices[j+2];
            }
            else
            {
                v1i = *((int*)&mb_indices[j]);
                v2i = *((int*)&mb_indices[j+2]);
                v3i = *((int*)&mb_indices[j+4]);
            }
	
			vArray[0] = loc.X + mb_vertices[v1i].Pos.X*terrainScale.X;
			vArray[1] = loc.Y + mb_vertices[v1i].Pos.Y*terrainScale.Y;
			vArray[2] = loc.Z + mb_vertices[v1i].Pos.Z*terrainScale.Z;
			vArray[3] = loc.X + mb_vertices[v2i].Pos.X*terrainScale.X;
			vArray[4] = loc.Y + mb_vertices[v2i].Pos.Y*terrainScale.Y;
			vArray[5] = loc.Z + mb_vertices[v2i].Pos.Z*terrainScale.Z;
			vArray[6] = loc.X + mb_vertices[v3i].Pos.X*terrainScale.X;
			vArray[7] = loc.Y + mb_vertices[v3i].Pos.Y*terrainScale.Y;
			vArray[8] = loc.Z + mb_vertices[v3i].Pos.Z*terrainScale.Z;
			if (vArray[0]>maxes[0]) maxes[0] = vArray[0];
			if (vArray[1]>maxes[1]) maxes[1] = vArray[1];
			if (vArray[2]>maxes[2]) maxes[2] = vArray[2];
			if (vArray[3]>maxes[0]) maxes[0] = vArray[3];
			if (vArray[4]>maxes[1]) maxes[1] = vArray[4];
			if (vArray[5]>maxes[2]) maxes[2] = vArray[5];
			if (vArray[6]>maxes[0]) maxes[0] = vArray[6];
			if (vArray[7]>maxes[1]) maxes[1] = vArray[7];
			if (vArray[8]>maxes[2]) maxes[2] = vArray[8];
//            printf("v1i %d: %f %f %f\n", v1i, vArray[0], vArray[1], vArray[2]);
//            printf("v2i %d: %f %f %f\n", v2i, vArray[3], vArray[4], vArray[5]);
//            printf("v3i %d: %f %f %f\n", v3i, vArray[6], vArray[7], vArray[8]);

			NewtonTreeCollisionAddFace(collision, 3, (float*)vArray, 12, 1);
		}
//        }
        printf("pols: %d maxes: %f %f %f\n", pols, maxes[0], maxes[1], maxes[2]);
		mb->drop();
    }
///////////////// END SMALL ////////////////////    

    printf("collisionendbuild\n");
    NewtonTreeCollisionEndBuild(collision, 0);
    printf("map build end\n");
////////////////////////
    // add a notification call back for when the car leave the world

//    loadRoads(roadfileName, smgr, driver, loc);
//    loadObjects(objectfileName, smgr, driver, loc);
                                      
}

SmallTerrain::~SmallTerrain()
{
    if (terrain)
    {
        removeFromDepthNodes(terrain);
        terrain->remove();
        terrain = 0;
    }
    if (body)
    {
        NewtonDestroyBody(nWorld, body);
        body = 0;
    }
    if (collision)
    {
	   NewtonReleaseCollision(nWorld, collision);
	   collision = 0;
    }
    //destroyRoads();
    //destroyObjects();
}

void SmallTerrain::setActive(bool pvisible)
{
    if (visible == pvisible) return;
    
    visible = pvisible;
    
    if (visible)
    {
        //printf("createbody\n");
        body = NewtonCreateBody(nWorld, collision);

        NewtonBodySetMaterialGroupID(body, levelID);

        // set the newton world size based on the bsp size
        float boxP0[3]; 
        float boxP1[3]; 
        float matrix[4][4]; 
        NewtonBodyGetMatrix (body, &matrix[0][0]); 
        //printf("calculateAABB\n");
        NewtonCollisionCalculateAABB (collision, &matrix[0][0],  &boxP0[0], &boxP1[0]); 
        // you can pad the box here if you wish
        boxP1[1] = 10000;
        //printf("P0\n");
        /*
        for (int my_i = 0; my_i < 3; my_i++)
            printf("%f ", boxP0[my_i]);
        printf("\nP1\n");
        for (int my_i = 0; my_i < 3; my_i++)
            printf("%f ", boxP1[my_i]);
        printf("\n");
        */
        if (terrain)        
            addToDepthNodes(terrain);
        //boxP0.y -= 10.f; 
        //boxP1.y += somevaluef; 
        //NewtonSetWorldSize (nWorld, (float*)boxP0, (float*)boxP1);
    }
    else
    {
        if (terrain)        
            removeFromDepthNodes(terrain);
        if (body)
        {
            NewtonDestroyBody(nWorld, body);
            body = 0;
        }
    }
    if (terrain)
    {
        terrain->setVisible(visible);
    }
    for (int i = 0; i < numOfRoads;i++)
    {
        if (visible)
            roadWrappers[i]->incVisCount();
        else
            roadWrappers[i]->decVisCount();
    }
    if (!visible)
    {
        for (int i = 0; i < numOfObjects;i++)
        {
           objectWrappers[i]->setVisible(visible);
        }
        for (int i = 0; i < numOfGrasses;i++)
        {
           grassWrappers[i]->setVisible(visible);
        }
        wasCalculated = false;
    }
}

void SmallTerrain::updatePos(float newX, float newY, float limit, int obj_density, int grass_density, bool force)
{
    const core::vector3df c0(newX - (limit+1.f), terrain->getPosition().Y, newY - (limit+1.f));
    const core::vector3df c1(newX + (limit+1.f), terrain->getPosition().Y, newY - (limit+1.f));
    const core::vector3df c2(newX + (limit+1.f), terrain->getPosition().Y, newY + (limit+1.f));
    const core::vector3df c3(newX - (limit+1.f), terrain->getPosition().Y, newY + (limit+1.f));
    if (
        ((terrain->getPosition()<=c0 && c0<=core::vector3df(terrain->getPosition().X+SMALLTERRAIN_SIZE, terrain->getPosition().Y, terrain->getPosition().Z+SMALLTERRAIN_SIZE)) ||
        (terrain->getPosition()<=c1 && c1<=core::vector3df(terrain->getPosition().X+SMALLTERRAIN_SIZE, terrain->getPosition().Y, terrain->getPosition().Z+SMALLTERRAIN_SIZE)) ||
        (terrain->getPosition()<=c2 && c2<=core::vector3df(terrain->getPosition().X+SMALLTERRAIN_SIZE, terrain->getPosition().Y, terrain->getPosition().Z+SMALLTERRAIN_SIZE)) ||
        (terrain->getPosition()<=c3 && c3<=core::vector3df(terrain->getPosition().X+SMALLTERRAIN_SIZE, terrain->getPosition().Y, terrain->getPosition().Z+SMALLTERRAIN_SIZE))) ||
        force || limit > SMALLTERRAIN_SIZE
        )
    {    
        int numObj = numOfObjects;
        int numGra = numOfGrasses;
        int numObjLim = (numObj * obj_density) / 100;
        int numGraLim = (numGra * grass_density) / 100;
	
        if (!force)
        {
            numObj = (numObj * obj_density) / 100;
            numGra = (numGra * grass_density) / 100;
        }
        //printf("update\n");    
        for (int i = 0; i < numObj; i++)
        {
            if( fabsf(objectWrappers[i]->getPosition().X-newX) <= limit &&
                fabsf(objectWrappers[i]->getPosition().Z-newY) <= limit &&
		i < numObjLim)
                objectWrappers[i]->setVisible(true);
            else
                objectWrappers[i]->setVisible(false);
        }
        for (int i = 0; i < numGra; i++)
        {
            if( fabsf(grassWrappers[i]->getPosition().X-newX) <= limit &&
                fabsf(grassWrappers[i]->getPosition().Z-newY) <= limit &&
                i < numGraLim)
                grassWrappers[i]->setVisible(true);
            else
                grassWrappers[i]->setVisible(false);
        }
        wasCalculated = true;
    }
    else
    {
        if (wasCalculated)
        {
            wasCalculated = false;
            for (int i = 0; i < numOfObjects; i++)
            {
                objectWrappers[i]->setVisible(false);
            }
            for (int i = 0; i < numOfGrasses; i++)
            {
                grassWrappers[i]->setVisible(false);
            }
        }
    }
}

void SmallTerrain::addRoad(SRoadWrapper* roadWrapper)
{
    if (numOfRoads==0) // add the first road
    {
        roadWrappers = new SRoadWrapper*[MAX_ROAD_NUM];
    } else
    {
        if (roadWrappers[numOfRoads-1] == roadWrapper) // we add a road only once
        {
            return;
        }
    }
    
    if (numOfRoads >= MAX_ROAD_NUM)
    {
        return;
    }
    
    roadWrappers[numOfRoads] = roadWrapper;
    numOfRoads++;
}

void SmallTerrain::addObject(SObjectWrapper* objectWrapper)
{
    if (numOfObjects==0) // add the first obj
    {
        objectWrappers = new SObjectWrapper*[MAX_OBJECT_NUM];
    } else
    {
        if (objectWrappers[numOfObjects-1] == objectWrapper) // we add an obj only once
        {
            return;
        }
    }
    
    if (numOfObjects >= MAX_OBJECT_NUM)
    {
        return;
    }
    
    objectWrappers[numOfObjects] = objectWrapper;
    numOfObjects++;
}

void SmallTerrain::addGrass(SObjectWrapper* grassWrapper)
{
    if (numOfGrasses==0) // add the first obj
    {
        grassWrappers = new SObjectWrapper*[MAX_GRASS_NUM];
    } else
    {
        if (grassWrappers[numOfGrasses-1] == grassWrapper) // we add an obj only once
        {
            return;
        }
    }
    
    if (numOfGrasses >= MAX_GRASS_NUM)
    {
        return;
    }
    
    grassWrappers[numOfGrasses] = grassWrapper;
    numOfGrasses++;
}

/*
void SmallTerrain::loadRoads(const c8* name, ISceneManager* smgr, IVideoDriver* driver,
                             const vector3df& loc)
{
    FILE* f;
    numOfRoads = 0;
    c8 meshName[256];
    c8 textureName[256];
    float dummy1, dummy2, dummy3;
    unsigned int numOfVertices, numOfPols;
    float x,y,z,tu,tv;
    int ret;
    s32 verInd;
    video::S3DVertex vtx;
    vtx.Color.set(255,255,255,255);
    vtx.Normal.set(0,1,0);

    printf("Read roads: %s\n", name);
    printf("roads offset: %f %f %f\n", loc.X, loc.Y, loc.Z);
    
    f = fopen(name, "r");
    
    if (!f)
    {
        printf("road file unable to open: %s\n", name);
        return;       
    }
    
    ret = fscanf(f, "%u\n", &numOfRoads);
    
    if (!numOfRoads || ret <=0 )
    {
        printf("error reading %s ret %d errno %d\n", name, ret, errno);
        fclose(f);
        return;
    }

    printf("%u road(s) found\n", numOfRoads);
    
    roadMeshes = new IAnimatedMesh*[numOfRoads];
    roadNodes = new ISceneNode*[numOfRoads];
    
    for (int i = 0;i < numOfRoads;i++)
    {
	    SMeshBuffer* buffer = new SMeshBuffer();
	    SMesh* mesh = new SMesh();
	    
        ret = fscanf(f, "%s\n%s\n%f\n%u\n", meshName, textureName, &dummy1, &numOfVertices);
        if (ret <= 0)
        {
           printf("error reading %s ret %d errno %d\n", name, ret, errno);
           fclose(f);
           return;
        }

        for (int ind = 0; ind < numOfVertices; ind++)
        {
            ret = fscanf(f, "%f %f %f %f %f %f %f %f\n", &x, &y, &z, &tu, &tv, &dummy1, &dummy2, &dummy3);
            if (ret <= 0)
            {
               printf("error reading %s ret %d errno %d\n", name, ret, errno);
               fclose(f);
               return;
            }
            vtx.Pos.X = x;            
            vtx.Pos.Z = z;
            vtx.Pos.Y = terrain->getHeight(x + loc.X, z + loc.Z) + 0.05f;
            if (fabsf(vtx.Pos.Y) > 2000.f) vtx.Pos.Y = 0.0f;
            vtx.TCoords.X = tu;
            vtx.TCoords.Y = tv;
            buffer->Vertices.push_back(vtx);
        }
        ret = fscanf(f, "%u\n", &numOfPols);
        if (ret <= 0)
        {
           printf("error reading %s ret %d errno %d\n", name, ret, errno);
           fclose(f);
           return;
        }
        for (int ind = 0; ind < numOfPols; ind++)
        {
            ret = fscanf(f, "%u\n", &verInd);
            if (ret <= 0)
            {
               printf("error reading %s ret %d errno %d\n", name, ret, errno);
               fclose(f);
               return;
            }
            if (verInd > numOfVertices)
            {
               printf("!!!!! verInd > numOfVertices: %d > %u\n", verInd, numOfVertices);
            }
            buffer->Indices.push_back(verInd);
        }

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
       
       buffer->recalculateBoundingBox();

	   SAnimatedMesh* animatedMesh = new SAnimatedMesh();
	   mesh->addMeshBuffer(buffer);
	   mesh->recalculateBoundingBox();
	   animatedMesh->addMesh(mesh);
	   animatedMesh->recalculateBoundingBox();

	   mesh->drop();
	   buffer->drop();
	   
	   roadMeshes[i] = animatedMesh;
       roadNodes[i] = smgr->addAnimatedMeshSceneNode(roadMeshes[i]);
       roadNodes[i]->setMaterialFlag(video::EMF_LIGHTING, false);
//       roadNodes[i]->setMaterialFlag(video::EMF_LIGHTING, globalLight);
//	   if (globalLight)
//	   {
//	       roadNodes[i]->setMaterialFlag(video::EMF_NORMALIZE_NORMALS, true);
//       }
	   roadNodes[i]->setMaterialTexture(0, driver->getTexture(textureName));
	   roadNodes[i]->setVisible(false);
	   roadNodes[i]->setPosition(loc);
//       roadNodes[i]->setMaterialType(video::EMT_DETAIL_MAP);
    }
    
    fclose(f);
    
}

void SmallTerrain::destroyRoads()
{
    printf("Destroy small roads #%d\n", numOfRoads);
    for (int i = 0; i < numOfRoads;i++)
    {
//       roadMeshes[i]->drop();
       roadNodes[i]->remove();
    }
    if (numOfRoads)
    {
        delete roadMeshes;
        delete roadNodes;
    }
    numOfRoads = 0;
}

void SmallTerrain::loadObjects(const c8* name, ISceneManager* smgr, IVideoDriver* driver,
                             const vector3df& loc)
{
    FILE* f;
    int ret = 0;
    int repeat = 0;
    c8 meshName[256];
    c8 textureName[256];
    vector3df pos(0.f,0.f,0.f);
    vector3df rot(0.f,0.f,0.f);
    vector3df sca(1.f,1.f,1.f);

    numOfObjects = 0;

    printf("Read objects: %s\n", name);
    printf("objects offset: %f %f %f\n", loc.X, loc.Y, loc.Z);
    
    f = fopen(name, "r");
    
    if (!f)
    {
        printf("objects file unable to open: %s\n", name);
        return;       
    }

    srand(1); // set seed, wand object to the same place always
    
    objectMeshes = new IAnimatedMesh*[MAX_OBJECT_NUM];
    objectNodes = new IAnimatedMeshSceneNode*[MAX_OBJECT_NUM];
    memset(objectMeshes, 0, sizeof(void*)*MAX_OBJECT_NUM);
    memset(objectNodes, 0, sizeof(void*)*MAX_OBJECT_NUM);
    
    for (int i = 0;numOfObjects < MAX_OBJECT_NUM;i++)
    {
        ret = fscanf(f, "%s\n%s\n%f, %f, %f\n%f, %f, %f\n%f, %f, %f\n%d\n", meshName, textureName,
                &pos.X, &pos.Y, &pos.Z,
                &rot.X, &rot.Y, &rot.Z,
                &sca.X, &sca.Y, &sca.Z,
                &repeat);
        if (ret < 12)
        {
            // no more object
            break;
        }
        //assert(i==numOfObjects);
        for (int j = 0; j < repeat && numOfObjects < MAX_OBJECT_NUM; j++)
        {
            objectMeshes[numOfObjects] = smgr->getMesh(meshName);
            objectNodes[numOfObjects] = smgr->addAnimatedMeshSceneNode(objectMeshes[numOfObjects]);
            objectNodes[numOfObjects]->setMaterialFlag(video::EMF_LIGHTING, globalLight);
            if (globalLight)
            {
                objectNodes[numOfObjects]->addShadowVolumeSceneNode();
                objectNodes[numOfObjects]->setMaterialFlag(video::EMF_NORMALIZE_NORMALS, true);
            }
            if (strcmp(textureName, "null"))
                objectNodes[numOfObjects]->setMaterialTexture(0, driver->getTexture(textureName));
            if (j != 0)
            {
                pos.X = (float)(rand() % ((int)SMALLTERRAIN_SIZE));
                pos.Z = (float)(rand() % ((int)SMALLTERRAIN_SIZE));
            }
            pos.Y = terrain->getHeight(loc.X+pos.X, loc.Z+pos.Z);
            objectNodes[numOfObjects]->setPosition(loc+pos);
            objectNodes[numOfObjects]->setRotation(rot);
            objectNodes[numOfObjects]->setScale(sca);
            objectNodes[numOfObjects]->setVisible(false);
            numOfObjects++;
        }
    }
    
    fclose(f);
    
    printf("%d object(s) found\n", numOfObjects);
}

void SmallTerrain::destroyObjects()
{
    printf("Destroy small objects #%d\n", numOfObjects);
    for (int i = 0; i < numOfObjects;i++)
    {
//        printf("delete mesh\n");
//        objectMeshes[i]->drop();
//        printf("delete node\n");
        objectNodes[i]->remove();
    }
    printf("delete small mesh array\n");
    if (objectMeshes)
    {
        delete objectMeshes;
    }
    printf("delete small nodes array\n");
    if (objectNodes)
    {
        delete objectNodes;
    }
    numOfObjects = 0;
    printf("Destroy small objects end\n");
}
*/

