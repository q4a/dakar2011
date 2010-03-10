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
#include "MyRoad.h"
#include <assert.h>

#ifdef __linux__
#include "linux_includes.h"
#endif

#ifdef MY_DEBUG
#define dprintf(x) x
#else
#define dprintf(x)
#endif

//#define MAX_OBJECT_NUM 10000
//#define MAX_GRASS_NUM 40000
#define MAX_ROAD_NUM_OLD 50
#define MAX_LOD 5

#define VSCALE 1.5f

//static const char* MAGIC_NUMBER = "serialize data";
static int write_error = 0;
static void SerializeFile (void* serializeHandle, const void* buffer, int size)
{
    //printf("wb\n");
    int ret = 0;
    ret = fwrite (buffer, size, 1, (FILE*) serializeHandle);
    if (ret != size && ferror((FILE*) serializeHandle))
    {
        printf("write error %d\n", ferror((FILE*) serializeHandle));
        write_error = 1;
    }
    //printf("we\n");
}

static void DeSerializeFile (void* serializeHandle, void* buffer, int size)
{
    //printf("rb\n");
    int ret = 0;
    ret = fread (buffer, size, 1, (FILE*) serializeHandle);
    if (ret != size && feof((FILE*) serializeHandle))
    {
        printf("read error: eof %d error %d\n", feof((FILE*) serializeHandle), ferror((FILE*) serializeHandle));
    }
    //printf("re\n");
}

SmallTerrain::SmallTerrain(video::IImage* heightMap,
                           video::IImage* densityMap,
                           video::IImage* textureMap,
                           video::ITexture** textures,
                           video::ITexture* shadowMap,
                           ISceneManager* psmgr, IVideoDriver* pdriver, NewtonWorld* pnWorld,
                           scene::ISceneNode* skydome,
                           const int px, const int py,
                           const int pmax_x, const int pmax_y,
                           int pstHeightmapSize, float ptScale,
                           int stageNum, BigTerrain* p_bigTerrain,
                           core::array<SObjectPoolIdRepPair> &objectReps, int obj_density,
                           core::array<CMyRoad*> &bigRoadList)
           : nWorld(pnWorld), terrain(0), body(0),
             visible(false), collision(0), stHeightmapSize(pstHeightmapSize), tScale(ptScale),
             /*roadMeshes(0),*/ roadWrappers_old(),/* numOfRoads(0),*/
             /*objectMeshes(0),*/ objectWrappers(),/* numOfObjects(0),*/
             /*grassWrappers(0), numOfGrasses(0),*/ wasCalculated(true),
             smgr(psmgr), driver(pdriver), x(px), y(py), max_x(pmax_x), max_y(pmax_y),
             m_bigTerrain(p_bigTerrain), roadList(), oceanNode(0)
{
    core::vector3df loc((float)x*SMALLTERRAIN_SIZE, 0.f, (float)y*SMALLTERRAIN_SIZE);
    core::vector3df terrainScale(TERRAIN_SCALE, VSCALE, TERRAIN_SCALE);	// scale

	terrain = smgr->addTerrainSceneNode(
		heightMap,
		0,					// parent node
		-1,					// node id
		loc,		// position
		core::vector3df(0.f, 0.f, 0.f),		// rotation
		terrainScale,	// scale
		video::SColor ( 255, 255, 255, 255 ),	// vertexColor
		MAX_LOD,					// maxLOD
		scene::ETPS_17,				// patchSize
		0,					// smoothFactor
		false
        ,                    // addAtFault
		x*SMALLTERRAIN_HEIGHTMAP_SIZE, // x offset
		y*SMALLTERRAIN_HEIGHTMAP_SIZE, // y offset
		SMALLTERRAIN_HEIGHTMAP_SIZE+1    // small size
		
		);
    
    if (useShaders)
        terrain->setMaterialFlag(video::EMF_LIGHTING, false);
    else    
        terrain->setMaterialFlag(video::EMF_LIGHTING, globalLight);

// remove me
//    terrain->setMaterialFlag(video::EMF_WIREFRAME, true);
 
    // calculate image part from the texture
    int sizeX = textureMap->getDimension().Width / max_x;
    int sizeY = textureMap->getDimension().Height / max_y;
    IImage* partImage = //fullImage;
                        driver->createImage(textureMap,
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
/*
    char* backslash = strrchr(heightmapName, '/');
    if (!backslash) backslash = heightmapName;
    sprintf(heightMapPartName, "%s%s_%d_%d", serialized_file_path,backslash+1, x, y);
    dprintf(printf("hmpn: %s\n", heightMapPartName));
    sprintf(terrainPartTextureName, "%s_%d_%d", terrainTextureName, x, y);
    dprintf(printf("terrainPartTextureName: %s sizeX: %d, sizeY: %d, pi: %p\n", terrainPartTextureName, sizeX, sizeY, partImage));
*/
    sprintf(heightMapPartName, "%s%d_%d_%d", serialized_file_path, stageNum, x, y);

    c8 textureMapPartName[256];
    sprintf(textureMapPartName, "textureMapPart_%d_%d", x, y);
    terrain->setMaterialTexture(0, driver->addTexture(textureMapPartName, partImage));
    partImage->drop();
    
    for (int i = 0; i < TERRAIN_TEXTURE_NUM; i++)
    {
        terrain->setMaterialTexture(i+1, textures[i]);
    }
    
    if (shadowMap)
    {
        terrain->setMaterialTexture(7, shadowMap);
    }
    
    terrain->setMaterialType((video::E_MATERIAL_TYPE)myMaterialType_light_2tex_2);
	//terrain->scaleTexture(1.0f, (float)SMALLTERRAIN_HEIGHTMAP_SIZE * (1024.f / (float)dett->getSize().Width) /*(float)2*/);
	terrain->scaleTexture(1.0f, (float)SMALLTERRAIN_HEIGHTMAP_SIZE /* 3.0f*/);
    	
	//terrain->setVisible(false);
	for (int i = 0; i<MAX_LOD; i++)
	{
        terrain->overrideLODDistance(i, LOD_distance*TERRAIN_SCALE*(1 + i + (i/2)));
    }

///////////////// BEGIN SMALL ////////////////////    
	// add terrain scene node
    // add a notification call back for when the car leave the world

//    loadRoads(roadfileName, smgr, driver, loc);
//    loadObjects(objectfileName, smgr, driver, loc);

    objectWire = new CObjectWire(SMALLTERRAIN_SIZE, SMALLTERRAIN_SIZE, terrain->getPosition().X, terrain->getPosition().Z);
    vector3df maxs;
    int addSuccCnt = 0;
    int addFailCnt = 0;
    for (int i = 0; i < objectReps.size(); i++)
    {
        for (int j = 0; j < (objectReps[i].rep*obj_density)/100;j++)
        {
            vector3df pos;
            int tries = 10;
            do
            {
                pos = vector3df(terrain->getPosition().X+(((float)rand())/32768.f)*SMALLTERRAIN_SIZE,
                                0.0f,
                                terrain->getPosition().Z+(((float)rand())/32768.f)*SMALLTERRAIN_SIZE);
                tries--;
            } while (!skip_densitymap && tries > 0 && m_bigTerrain->getDensity(pos.X, pos.Z) < 0.1f);
            if (tries > 0)
            {
                SObjectWrapper* objectWrapper = new SObjectWrapper(m_bigTerrain);
                objectWrapper->setPosition(pos);
                objectWrapper->setPool(objectReps[i].poolId);
                objectWrappers.push_back(objectWrapper);
                if (objectWire->addObject(pos, objectWrapper))
                    addSuccCnt++;
                else
                    addFailCnt++;
                if (pos.X > maxs.X) maxs.X = pos.X;
                if (pos.Z > maxs.Z) maxs.Z = pos.Z;
            }
        }
        dprintf(printf("num: %d/(%d*%d) rep objects maxes: %f %f, terrain size %f, addSucc: %d addFail %d\n",
                (objectReps[i].rep*obj_density)/100, objectReps[i].rep, obj_density,
                maxs.X, maxs.Z, SMALLTERRAIN_SIZE, addSuccCnt, addFailCnt);)
    }
    activate(heightMap);
    updateRoads(bigRoadList);
    setActive(true);
    
    // ocean
    oceanNode = loadMySimpleObject("data/bigterrains/ocean/ocean_surface.mso");
/*
    scene::IAnimatedMesh* mesh = smgr->addHillPlaneMesh( "myHill",
        core::dimension2d<f32>(SMALLTERRAIN_SIZE / (float)200, SMALLTERRAIN_SIZE / (float)200),
        core::dimension2d<u32>(200,200), 0, 0,
        core::dimension2d<f32>(0,0),
        core::dimension2d<f32>(100,100));
    oceanNode = smgr->addWaterSurfaceSceneNode(mesh->getMesh(0));
*/
    if (oceanNode)
    {
        oceanNode->setPosition(vector3df(-0.25f, 30.f, -0.25f) + terrain->getPosition());
        oceanNode->setScale(vector3df(SMALLTERRAIN_SIZE+0.5f, 1.0f, SMALLTERRAIN_SIZE+0.5f));
        //oceanNode->setMaterialTexture(0, driver->getTexture("data/bigterrains/ocean/noise.png"));
        oceanNode->setMaterialTexture(0, driver->getTexture("data/bigterrains/ocean/normal2.png"));
//        oceanNode->setMaterialTexture(0, driver->getTexture("data/bigterrains/ocean/water02.jpg"));

        if (skydome && skydome->getMaterial(0).getTexture(0))
            oceanNode->setMaterialTexture(1, skydome->getMaterial(0).getTexture(0));

        //oceanNode->setMaterialTexture(1, driver->getTexture("data/bigterrains/ocean/water.png"));

        if (shadowMap)
        {
//            oceanNode->setMaterialTexture(2, shadowMap);
        }

        oceanNode->setMaterialType(/*video::EMT_REFLECTION_2_LAYER*/(video::E_MATERIAL_TYPE)myMaterialType_ocean);
        //oceanNode->scaleTexture(1.0f, (float)SMALLTERRAIN_HEIGHTMAP_SIZE /* 3.0f*/);
        //oceanNode->setMaterialFlag(video::EMF_WIREFRAME, true);
    }
    else
    {
        //assert(0 && "No ocean node");
    }
}

SmallTerrain::~SmallTerrain()
{
    if (terrain)
    {
        removeFromDepthNodes(terrain);
        terrain->remove();
        terrain = 0;
    }
    if (oceanNode)
    {
        oceanNode->remove();
        oceanNode = 0;
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
/*    if (roadWrappers)
    {
        delete [] roadWrappers;
        roadWrappers = 0;
        numOfRoads = 0;
    }
    if (objectWrappers)
    {
        delete [] objectWrappers;
        objectWrappers = 0;
        numOfObjects = 0;
    }
*/
    for (int i = 0; i < objectWrappers.size();i++)
    {
       delete objectWrappers[i];
    }
    objectWrappers.clear();

    if (objectWire)
    {
        delete objectWire;
        objectWire = 0;
    }

    //destroyRoads();
    //destroyObjects();
}

void SmallTerrain::activate(video::IImage* heightMap)
{
    if (collision) return;

#if 1
/*
    int sizeX = heightMap->getDimension().Width / max_x;
    int sizeY = heightMap->getDimension().Height / max_y;
    IImage* partImage = //fullImage;
                        driver->createImage(heightMap,
                                            core::position2d<s32>((x)*sizeX, (y)*sizeY),
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
*/
    unsigned short* elevationMap = new unsigned short[(SMALLTERRAIN_HEIGHTMAP_SIZE+1)*(SMALLTERRAIN_HEIGHTMAP_SIZE+1)];
    char* attributeMap = new char[(SMALLTERRAIN_HEIGHTMAP_SIZE+1)*(SMALLTERRAIN_HEIGHTMAP_SIZE+1)];
    
    memset(attributeMap, (char)levelID, sizeof(char)*(SMALLTERRAIN_HEIGHTMAP_SIZE+1)*(SMALLTERRAIN_HEIGHTMAP_SIZE+1));
    
    for (int i = 0; i < (SMALLTERRAIN_HEIGHTMAP_SIZE+1); i++)
        for (int j = 0; j < (SMALLTERRAIN_HEIGHTMAP_SIZE+1); j++)
        {
            elevationMap[i + ((SMALLTERRAIN_HEIGHTMAP_SIZE+1)*j)] =
                    (unsigned short)heightMap->getPixel((SMALLTERRAIN_HEIGHTMAP_SIZE*x)+i,
                                                        (SMALLTERRAIN_HEIGHTMAP_SIZE*y)+j).getAverage();
        }

    collision = NewtonCreateHeightFieldCollision(nWorld, SMALLTERRAIN_HEIGHTMAP_SIZE+1, SMALLTERRAIN_HEIGHTMAP_SIZE+1,
                    0, elevationMap, attributeMap, TERRAIN_SCALE, VSCALE, levelID);

    delete [] elevationMap;
    delete [] attributeMap;
#endif
#if 0
    FILE* file;
    NewtonCollision* new_collision = 0;

    // load the collision file
    if (use_serialized_files)
    {
        file = fopen (heightMapPartName, "rb");
        if (file)
        {
            char magicNumber[256]; 
            memset(magicNumber, 0, sizeof(magicNumber));
            DeSerializeFile (file, magicNumber, int (strlen(heightMapPartName) + 1));
            if (!strcmp (magicNumber, heightMapPartName))
            {
                //if (collision) NewtonReleaseCollision (nWorld, collision);
                //printf("1\n");
                NewtonWorldCriticalSectionLock(nWorld);
                collision = NewtonCreateCollisionFromSerialization (nWorld, DeSerializeFile, file);
                NewtonWorldCriticalSectionUnlock(nWorld);
                //printf("2\n");
                
                if (collision)
                {
                    NewtonCollisionInfoRecord collisionInfo;
                    NewtonCollisionGetInfo (collision, &collisionInfo);
                    NewtonCollisionGetInfo (collision, &collisionInfo);
                    fclose (file);
                    return;
                }
                else
                {
                    printf("selialized file found, but content is not ok, build collision again: %s\n", heightMapPartName);
                }
            }
            else
            {
                printf("selialized file found, but header is not ok, build collision again: %s != %s\n", heightMapPartName, magicNumber);
            }
            fclose (file);
        }
        else
        {
            dprintf(printf("selialized file not found, build collision again: %s\n", heightMapPartName));
        }
    }
    
    core::vector3df loc((float)x*SMALLTERRAIN_SIZE, 0.f, (float)y*SMALLTERRAIN_SIZE);
    core::vector3df terrainScale(TERRAIN_SCALE, VSCALE, TERRAIN_SCALE);	// scale

    float maxes[3] = {0.f,0.f,0.f};
    float mines[3] = {100000.f,100000.f,100000.f};
//////////////////
    dprintf(printf("map build begin %d\n", terrain->getMesh()->getMeshBufferCount()));
    collision = NewtonCreateTreeCollision(nWorld, levelID/*, NULL*/);
    NewtonTreeCollisionBeginBuild(collision);
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
        
        //float* my_vertices = new float[3*mb->getIndexCount()];
        
        dprintf(printf("index count: %d vertex count %d\n", mb->getIndexCount(), mb->getVertexCount()));
        if (mb->getVertexCount()>= 256*256)
        {
            step = 2;
        }
        dprintf(printf("using step = %u index type %u\n", step, mb->getIndexType()));
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

			if (vArray[0]<mines[0]) mines[0] = vArray[0];
			if (vArray[1]<mines[1]) mines[1] = vArray[1];
			if (vArray[2]<mines[2]) mines[2] = vArray[2];
			if (vArray[3]<mines[0]) mines[0] = vArray[3];
			if (vArray[4]<mines[1]) mines[1] = vArray[4];
			if (vArray[5]<mines[2]) mines[2] = vArray[5];
			if (vArray[6]<mines[0]) mines[0] = vArray[6];
			if (vArray[7]<mines[1]) mines[1] = vArray[7];
			if (vArray[8]<mines[2]) mines[2] = vArray[8];
//            printf("v1i %d: %f %f %f\n", v1i, vArray[0], vArray[1], vArray[2]);
//            printf("v2i %d: %f %f %f\n", v2i, vArray[3], vArray[4], vArray[5]);
//            printf("v3i %d: %f %f %f\n", v3i, vArray[6], vArray[7], vArray[8]);

			NewtonTreeCollisionAddFace(collision, 3, (float*)vArray, 12, 1);
		}
//        }
        dprintf(printf("pols: %d maxes: %f %f %f\n", pols, maxes[0], maxes[1], maxes[2]));
		mb->drop();
        
        if (x == 0)
        {
			vArray[0] = mines[0];
			vArray[1] = mines[1];
			vArray[2] = mines[2];
			vArray[3] = mines[0];
			vArray[4] = maxes[1]+100.f;
			vArray[5] = maxes[2];
			vArray[6] = mines[0];
			vArray[7] = mines[1];
			vArray[8] = maxes[2];
			NewtonTreeCollisionAddFace(collision, 3, (float*)vArray, 12, 1);
			vArray[0] = mines[0];
			vArray[1] = mines[1];
			vArray[2] = mines[2];
			vArray[3] = mines[0];
			vArray[4] = maxes[1]+100.f;
			vArray[5] = mines[2];
			vArray[6] = mines[0];
			vArray[7] = maxes[1]+100.f;
			vArray[8] = maxes[2];
			NewtonTreeCollisionAddFace(collision, 3, (float*)vArray, 12, 1);
        }
        if (x == max_x - 1)
        {
			vArray[0] = maxes[0];
			vArray[1] = mines[1];
			vArray[2] = mines[2];
			vArray[3] = maxes[0];
			vArray[4] = mines[1];
			vArray[5] = maxes[2];
			vArray[6] = maxes[0];
			vArray[7] = maxes[1]+100.f;
			vArray[8] = maxes[2];
			NewtonTreeCollisionAddFace(collision, 3, (float*)vArray, 12, 1);
			vArray[0] = maxes[0];
			vArray[1] = mines[1];
			vArray[2] = mines[2];
			vArray[3] = maxes[0];
			vArray[4] = maxes[1]+100.f;
			vArray[5] = maxes[2];
			vArray[6] = maxes[0];
			vArray[7] = maxes[1]+100.f;
			vArray[8] = mines[2];
			NewtonTreeCollisionAddFace(collision, 3, (float*)vArray, 12, 1);
        }
        if (y == 0)
        {
			vArray[0] = mines[0];
			vArray[1] = mines[1];
			vArray[2] = mines[2];
			vArray[3] = maxes[0];
			vArray[4] = mines[1];
			vArray[5] = mines[2];
			vArray[6] = maxes[0];
			vArray[7] = maxes[1]+100.f;
			vArray[8] = mines[2];
			NewtonTreeCollisionAddFace(collision, 3, (float*)vArray, 12, 1);
			vArray[0] = mines[0];
			vArray[1] = mines[1];
			vArray[2] = mines[2];
			vArray[3] = maxes[0];
			vArray[4] = maxes[1]+100.f;
			vArray[5] = mines[2];
			vArray[6] = mines[0];
			vArray[7] = maxes[1]+100.f;
			vArray[8] = mines[2];
			NewtonTreeCollisionAddFace(collision, 3, (float*)vArray, 12, 1);
        }
        if (y == max_y - 1)
        {
			vArray[0] = mines[0];
			vArray[1] = mines[1];
			vArray[2] = maxes[2];
			vArray[3] = maxes[0];
			vArray[4] = maxes[1]+100.f;
			vArray[5] = maxes[2];
			vArray[6] = maxes[0];
			vArray[7] = mines[1];
			vArray[8] = maxes[2];
			NewtonTreeCollisionAddFace(collision, 3, (float*)vArray, 12, 1);
			vArray[0] = mines[0];
			vArray[1] = mines[1];
			vArray[2] = maxes[2];
			vArray[3] = mines[0];
			vArray[4] = maxes[1]+100.f;
			vArray[5] = maxes[2];
			vArray[6] = maxes[0];
			vArray[7] = maxes[1]+100.f;
			vArray[8] = maxes[2];
			NewtonTreeCollisionAddFace(collision, 3, (float*)vArray, 12, 1);
        }
    }
///////////////// END SMALL ////////////////////    

    dprintf(printf("collisionendbuild\n"));
    NewtonWorldCriticalSectionLock(nWorld);
    NewtonTreeCollisionEndBuild(collision, 0);
    NewtonWorldCriticalSectionUnlock(nWorld);
    dprintf(printf("map build end\n"));
////////////////////////

    if (use_serialized_files)
    {
        // save the collision file
        file = fopen (heightMapPartName, "wb");
        if (file)
        {
            write_error = 0;
            SerializeFile(file, heightMapPartName, int (strlen(heightMapPartName) + 1));
            if (!write_error)
            {
                //printf("3\n");
                NewtonWorldCriticalSectionLock(nWorld);
                NewtonCollisionSerialize(nWorld, collision, SerializeFile, file);
                NewtonWorldCriticalSectionUnlock(nWorld);
                //printf("4\n");
            }
            fclose (file);
            if (write_error)
            {
                write_error = 0;
                remove(heightMapPartName);
                printf("(2) unable to write selialized file, check the serialized_file_path if writable, or maybe the disk is full: path: %s, fn: %s\n", serialized_file_path, heightMapPartName);
            }
        
            // load the collision file
            file = fopen (heightMapPartName, "rb");
            if (file)
            {
                char magicNumber[256]; 
                memset(magicNumber, 0, sizeof(magicNumber));
                DeSerializeFile (file, magicNumber, int (strlen(heightMapPartName) + 1));
                if (!strcmp (magicNumber, heightMapPartName))
                {
                    //printf("5\n");
                    NewtonWorldCriticalSectionLock(nWorld);
                    new_collision = NewtonCreateCollisionFromSerialization (nWorld, DeSerializeFile, file);
                    NewtonWorldCriticalSectionUnlock(nWorld);
                    //printf("6\n");
                    if (new_collision)
                    {
                        if (collision)
                        {
                            NewtonWorldCriticalSectionLock(nWorld);
                            NewtonReleaseCollision (nWorld, collision);
                            NewtonWorldCriticalSectionUnlock(nWorld);
                        }
                        collision = new_collision;
                        NewtonCollisionInfoRecord collisionInfo;
                        NewtonCollisionGetInfo (collision, &collisionInfo);
                        NewtonCollisionGetInfo (collision, &collisionInfo);
                    }
                    else
                    {
                        printf("newly build selialized file found, but content is not ok, using build collision: %s\n", heightMapPartName);
                    }
                }
                else
                {
                    printf("newly build selialized file found, but header is not ok, using build collision: %s != %s\n", heightMapPartName, magicNumber);
                }
                fclose (file);
            }
            else
            {
                printf("newly build selialized file not found, using build collision: %s\n", heightMapPartName);
            }
        }
        else
        {
            printf("(1) unable to write selialized file, check the serialized_file_path if writable, or maybe the disk is full: path: %s, fn: %s\n", serialized_file_path, heightMapPartName);
        }
    }
    //body = NewtonCreateBody(nWorld, collision);
    //NewtonBodySetMaterialGroupID(body, levelID);
#endif
}

void SmallTerrain::deactivate()
{
/*    if (terrain)
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
*/    if (collision)
    {
	   NewtonReleaseCollision(nWorld, collision);
	   collision = 0;
    }
}

void SmallTerrain::setActive(bool pvisible)
{
    if (visible == pvisible) return;
    
    visible = pvisible;
    
    if (visible)
    {
        //printf("createbody\n");
        
        //activate();
        matrix4 matrix;
        vector3df pos(terrain->getPosition());
//        pos.X -= TERRAIN_SCALE * 0.5f;
//        pos.Z -= TERRAIN_SCALE * 0.5f;
        matrix.setTranslation(pos);
        
        body = NewtonCreateBody(nWorld, collision);
        NewtonBodySetMaterialGroupID(body, levelID);
        NewtonBodySetMatrix(body, matrix.pointer());

        // set the newton world size based on the bsp size
        ///float boxP0[3]; 
        ///float boxP1[3]; 
        ///float matrix[4][4]; 
        ///NewtonBodyGetMatrix (body, &matrix[0][0]); 
        //printf("calculateAABB\n");
        ///NewtonCollisionCalculateAABB (collision, &matrix[0][0],  &boxP0[0], &boxP1[0]); 
        // you can pad the box here if you wish
        ///boxP1[1] = 10000;
        //printf("P0\n");
        /*
        for (int my_i = 0; my_i < 3; my_i++)
            printf("%f ", boxP0[my_i]);
        printf("\nP1\n");
        for (int my_i = 0; my_i < 3; my_i++)
            printf("%f ", boxP1[my_i]);
        printf("\n");
        */
        /* // removed by the new depth
        if (terrain)        
            addToDepthNodes(terrain);
        */
        //boxP0.y -= 10.f; 
        //boxP1.y += somevaluef; 
        //NewtonSetWorldSize (nWorld, (float*)boxP0, (float*)boxP1);
    }
    else
    {
//        deactivate();
        /* // removed by the new depth
        if (terrain)        
            removeFromDepthNodes(terrain);
        */
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
    for (int i = 0; i < roadWrappers_old.size();i++)
    {
        if (visible)
            roadWrappers_old[i]->incVisCount();
        else
            roadWrappers_old[i]->decVisCount();
    }
    if (!visible)
    {
        for (int i = 0; i < objectWrappers.size();i++)
        {
           objectWrappers[i]->setVisible(visible);
        }
        /*
        for (int i = 0; i < numOfGrasses;i++)
        {
           grassWrappers[i]->setVisible(visible);
        }
        */
        wasCalculated = false;
    }
}

void SmallTerrain::updatePos(float newX, float newY, float limit, bool force)
{
    objectWire->updatePos(newX, newY, limit, force);
}

void SmallTerrain::addRoad_old(SRoadWrapper_old* roadWrapper_old)
{
    if (roadWrappers_old.size()==0) // add the first road
    {
    } else
    {
        if (roadWrappers_old[roadWrappers_old.size()-1] == roadWrapper_old) // we add a road only once
        {
            return;
        }
    }
    
    roadWrappers_old.push_back(roadWrapper_old);
}

void SmallTerrain::updateRoads(core::array<CMyRoad*> &bigRoadList)
{
    dprintf(printf("ST::updateRoads() old size %d\n", roadList.size());)
    for (int i = 0; i < roadList.size(); i++)
    {
        delete roadList[i];
    }
    roadList.clear();

    core::vector2df down((float)x*SMALLTERRAIN_SIZE, (float)y*SMALLTERRAIN_SIZE);
    core::vector2df up((float)(x+1)*SMALLTERRAIN_SIZE, (float)(y+1)*SMALLTERRAIN_SIZE);

    dprintf(printf("ST::updateRoads() BT roads size %d\n", bigRoadList.size());)
    for (int i = 0; i < bigRoadList.size(); i++)
    {
        core::array<vector3df>& basePoints = bigRoadList[i]->getBasePoints();
        CMyRoad* road = 0;
        dprintf(printf("ST::updateRoads() BT %d. road bp size %d\n", i, basePoints.size());)
        if (basePoints.size()>1)
        {
            for (int j = 0; j < basePoints.size(); j++)
            {
                if (down.X < basePoints[j].X && down.Y < basePoints[j].Z &&
                    basePoints[j].X < up.X && basePoints[j].Z < up.Y)
                {
                    // the point is in the map
                    if (!road)
                    {
                        dprintf(printf("ST::updateRoads() new road found BT road num %d basePoint %d\n", i, j);)
                        road = new CMyRoad(smgr, driver, nWorld);
                        //dprintf(printf("ST::updateRoads() new road found BT road num %d basePoint %d, bpnum: %d\n", i, j, road->getBasePoints().size());)
                        road->setSlicePoints(bigRoadList[i]->getSlicePoints());
                        road->setSliceIndices(bigRoadList[i]->getSliceIndices());
                        road->setTexture(bigRoadList[i]->getTexture());
                        road->setTextureName(bigRoadList[i]->getTextureName());
                        if (j > 0)
                        {
                            float height = 0.0f;
                            float cheight = terrain->getHeight(basePoints[j].X, basePoints[j].Z);
                            if (j < basePoints.size()-1 && down.X < basePoints[j+1].X && down.Y < basePoints[j+1].Z &&
                                basePoints[j+1].X < up.X && basePoints[j+1].Z < up.Y)
                            {
                                height = cheight - (terrain->getHeight(basePoints[j+1].X, basePoints[j+1].Z) - cheight);
                            }
                            else
                            {
                                height = cheight;
                            }
                            //printf("height: %f\n", height);
                            road->addBasePoint(vector3df(basePoints[j-1].X,height,basePoints[j-1].Z));
                            //printf("add bp 1, new size: %d\n", road->getBasePoints().size());
                        }
                    }
                    if (j==0 || j==basePoints.size()-1)
                    {
                        road->addBasePoint(vector3df(basePoints[j].X,terrain->getHeight(basePoints[j].X, basePoints[j].Z)-0.6f,basePoints[j].Z));
                        //printf("add bp 2, new size: %d\n", road->getBasePoints().size());
                    }
                    else
                    {
                        road->addBasePoint(vector3df(basePoints[j].X,terrain->getHeight(basePoints[j].X, basePoints[j].Z),basePoints[j].Z));
                        //printf("add bp 3, new size: %d\n", road->getBasePoints().size());
                    }
                }
                else
                {
                    if (road)
                    {
                        dprintf(printf("ST::updateRoads() end road found BT road num %d basePoint %d, new road size %d\n", i, j, road->getBasePoints().size());)
                        //road->addBasePoint(vector3df(basePoints[j].X,terrain->getHeight(basePoints[j-1].X, basePoints[j-1].Z),basePoints[j].Z));
                        road->generateRoadNode();
                        roadList.push_back(road);
                        road = 0;
                    }
                }
            }
            if (road)
            {
                dprintf(printf("ST::updateRoads() end road found BT 2 road num %d, new road size %d\n", i, road->getBasePoints().size());)
                road->generateRoadNode();
                roadList.push_back(road);
                road = 0;
            }
        }
    }
    dprintf(printf("ST::updateRoads() return, %d new road(s) found\n", roadList.size());)
}

IAnimatedMeshSceneNode* SmallTerrain::loadMySimpleObject(const char* name)
{
    IAnimatedMesh* objectMesh;
    objectMesh = readMySimpleObject(name);
    IAnimatedMeshSceneNode* objectNode = smgr->addAnimatedMeshSceneNode(objectMesh);
    return objectNode;
}

/*
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
*/
