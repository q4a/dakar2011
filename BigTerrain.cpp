/****************************************************************
*                                                               *
*    Name: BigTerrain.cpp                                       *
*                                                               *
*    Creator: Balazs Tuska                                      *
*                                                               *
*    Description:                                               *
*       This file contains the big terrain. The big terrain     *
*       contains max_x*max_y small terrains. We will display    *
*       only terrains that we are on or next to.                *
*                                                               *
****************************************************************/

#include "BigTerrain.h"
#include "Materials.h"
#include <math.h>
#include <assert.h>
#include "my_shaders.h"
#include "settings.h"
#include "message.h"
#include "pools.h"
#include "gameplay.h"
#include "competitors.h"
#include "MyRoad.h"
#include "itiner_hud.h"
#include "error.h"

#include "CHeightmap.h"

#ifdef __linux__
#include "linux_includes.h"
#endif

#define MAX_OBJECT_NUM 20000
#define DEF_OV_LIMIT 300.f // object visibility limit
#define OV_LIMIT ov_limit // object visibility limit
#define GATE_LIMIT 10.0f
/*
#ifdef MY_DEBUG
#define dprintf(x) x
#else
#define dprintf(x)
#endif
*/
extern core::vector3df terrainScale;//(30.0f, 1.0f, 30.0f);	// scale

BigTerrain::BigTerrain(const c8* name, IrrlichtDevice* p_device,ISceneManager* p_smgr,
                       IVideoDriver* p_driver, NewtonWorld* p_nWorld,
                       u32 pstageTime, u32 pgtime, scene::ISceneNode* p_skydome, video::ITexture* p_shadowMap,
                       int p_stageNum,
                       TerrainPool* p_terrainPool)
           : nWorld(p_nWorld), map(0), max_x(0), max_y(0), last_x(-1), last_y(-1),
             ov_last_x(-1), ov_last_y(-1),
             stHeightmapSize(0), tScale(0.f), /*roadMeshes(0),*/ roadWrappers_old(), /*numOfRoads(0),*/
             /*objectMeshes(0),*/ objectWrappers(),/* numOfObjects(0),*/
             ov_limit(objectVisibilityLimit),
             /*numOfGrasses(0), grassWrappers(),*/
             startGate(0), startOffsetObject(0), endGate(0), endOffsetObject(0),
             device(p_device), smgr(p_smgr), driver(p_driver),
             //startTime(0), endTime(0),
             timeStarted(false), timeEnded(false), lastTick(0), currentTime(0),
             cps(0), penality(0),
             stageTime(pstageTime), stageLength(0.f), gtime(pgtime),
             densityMap(0), objectWire(0),
             heightMap(0), textureMap(0), shadowMap(p_shadowMap),
             cpPos(), cpTime(), cpTimed(), cpGate(), cpOffsetObject(),
             objectReps(), stageNum(p_stageNum), smallTerrainsForUpdate(), smallTerrainsForUpdateLock(), mapLock(),
             roadList(), activeItinerPoints(), aiPoints(), speed(60.0f), skydome(p_skydome),
             m_terrainPool(p_terrainPool), vscale(VSCALE), waterHeight(WATER_HEIGHT),
             lastMapsQueueUpdate(0), mapsQueueVersion(0)
             /*,
             bodyl(0), collisionl(0),
             bodyr(0), collisionr(0),
             bodyu(0), collisionu(0),
             bodyd(0), collisiond(0)*/
{
//    max_x = 8;
//    max_y = 24;
//    max_x = 2;
//    max_y = 2;
    FILE* f;
    int ret;
    c8 heightMapName[256];
    c8 densityMapName[256];
    c8 textureMapName[256];
    c8 textureName[256];
    
    c8 dummyStr[256];
    float f1, f2;

    c8 roadfileName[256];
    c8 objectfileName[256];
    //c8 grassfileName[256];
    //c8 treefileName[256];
    c8 skyDomeFileName[256];
    core::stringw str;
    
    memset(textures, 0, sizeof(textures));
    
    dprintf(printf("Read bigterrain: %s\n", name));

    f = fopen(name, "r");

    if (!f)
    {
        printf("big terrain file unable to open: %s\n", name);
        return;       
    }

    // read the sizes
    ret = fscanf(f, "%u x %f\n", &stHeightmapSize, &tScale);
    if ( ret < 2 )
    {
        printf("error reading %s ret %d errno %d\n", name, ret, errno);
        fclose(f);
        return;
    }
    dprintf(printf("Small terrain size: %u, Terrain scale: %f\n", stHeightmapSize, tScale);)
    assert(fabsf(tScale-TERRAIN_SCALE)<0.01f && stHeightmapSize == SMALLTERRAIN_HEIGHTMAP_SIZE);

    // read heightmap, densitymap and texturemap
    ret = fscanf(f, "%s\n%s\n%s\n", heightMapName, densityMapName, textureMapName);
    if ( ret < 3 )
    {
        printf("error reading %s ret %d errno %d\n", name, ret, errno);
        fclose(f);
        return;
    }
    dprintf(printf("Heightmap %s\ndensitymap %s\ntexturemap: %s\n", heightMapName, densityMapName, textureMapName);)

    /*
    {
        c8 baseName[256];
        strcpy(baseName, heightmapName);
        baseName[strlen(heightmapName)-4] = '\0';
        strcpy(densityMapName, baseName);
        strcat(densityMapName, "_den.png");
    }
    */
    int sizeX = 0;
    int sizeY = 0;
    video::IImage* partImage = 0;
#ifdef USE_IMAGE_HM
    
	heightMap = driver->createImageFromFile(heightMapName);
	partImage = heightMap;
    str = L"Loading: 10%";
    MessageText::addText(str.c_str(), 1, true);
    sizeX = partImage->getDimension().Width;
    sizeY = partImage->getDimension().Height;
    for (int i = 0; i < sizeX; i++)
    {
        for (int j = 0; j<sizeY/2; j++)
        {
            SColor tmp_col = partImage->getPixel(i, j);
            partImage->setPixel(i, j, partImage->getPixel(i, sizeY-1-j));
            partImage->setPixel(i, sizeY-1-j, tmp_col);
        }
    }
#else
	heightMap = smgr->addHeightmap();
	if (heightMapName[strlen(heightMapName)-3]=='.')
	{
        dprintf(printf("read heightmap as text\n");)
        bool ret = heightMap->read(heightMapName, device);
        if (!ret)
        {
            myError(4, "unable to read text heightmap: %s", heightMapName);
        }
    }
    else
    {
        dprintf(printf("read heightmap as bin\n");)
        bool ret = heightMap->readBin(heightMapName, device);
        if (!ret)
        {
            myError(4, "unable to read binary heightmap: %s", heightMapName);
        }
        //assert(0);
    }
    str = L"Loading: 10%";
    MessageText::addText(str.c_str(), 1, true);
    sizeX = heightMap->getXSize();
    sizeY = heightMap->getYSize();
    for (int i = 0; i < sizeX; i++)
    {
        for (int j = 0; j<sizeY/2; j++)
        {
            unsigned short tmp_height = heightMap->get(i, j);
            bool tmp_road = heightMap->getRoad(i, j);
            heightMap->set(i, j, heightMap->get(i, sizeY-1-j));
            heightMap->setRoad(i, j, heightMap->getRoad(i, sizeY-1-j));
            heightMap->set(i, sizeY-1-j, tmp_height);
            heightMap->setRoad(i, sizeY-1-j, tmp_road);
        }
        if (i%100==0)
        {
            MessageText::refresh();
        }
    }
#endif
    str = L"Loading: 13%";
    MessageText::addText(str.c_str(), 1, true);
    
	densityMap = driver->createImageFromFile(densityMapName);
	partImage = densityMap;
    sizeX = partImage->getDimension().Width;
    sizeY = partImage->getDimension().Height;
    for (int i = 0; i < sizeX; i++)
    {
        for (int j = 0; j<sizeY/2; j++)
        {
            SColor tmp_col = partImage->getPixel(i, j);
            partImage->setPixel(i, j, partImage->getPixel(i, sizeY-1-j));
            partImage->setPixel(i, sizeY-1-j, tmp_col);
        }
        if (i%100==0)
        {
            MessageText::refresh();
        }
    }
    str = L"Loading: 16%";
    MessageText::addText(str.c_str(), 1, true);
    
	
	textureMap = driver->createImageFromFile(textureMapName);
	partImage = textureMap;
    sizeX = partImage->getDimension().Width;
    sizeY = partImage->getDimension().Height;
    for (int i = 0; i < sizeX; i++)
    {
        for (int j = 0; j<sizeY/2; j++)
        {
            SColor tmp_col = partImage->getPixel(i, j);
            partImage->setPixel(i, j, partImage->getPixel(i, sizeY-1-j));
            partImage->setPixel(i, sizeY-1-j, tmp_col);
        }
        if (i%100==0)
        {
            MessageText::refresh();
        }
    }
    str = L"Loading: 20%";
    MessageText::addText(str.c_str(), 1, true);
    
        
	applyRoadOnHeightMap();
	
#ifdef USE_IMAGE_HM
	max_x = heightMap->getDimension().Width / SMALLTERRAIN_HEIGHTMAP_SIZE;
	max_y = heightMap->getDimension().Height / SMALLTERRAIN_HEIGHTMAP_SIZE;
#else
	max_x = heightMap->getXSize() / SMALLTERRAIN_HEIGHTMAP_SIZE;
	max_y = heightMap->getYSize() / SMALLTERRAIN_HEIGHTMAP_SIZE;
#endif    

    if (ov_limit > SMALLTERRAIN_SIZE)
    {
        ov_limit = objectVisibilityLimit = SMALLTERRAIN_SIZE - 1;
    }
    
    map = new SmallTerrain*[max_x*max_y];
    memset(map, 0, sizeof(void*)*max_x*max_y);

    dprintf(printf("%u x %u maps found\n", max_x, max_y);)
    
    // read the terrain textures
    for (int i = 0; i < TERRAIN_TEXTURE_NUM; i++)
    {
        char baseName[256];
        char extName[256];
        char useName[256];

        ret = fscanf(f, "%s\n", textureName);

        if (ret < 1)
        {
            printf("error reading %s ret %d errno %d\n", name, ret, errno);
            fclose(f);
            return;
        }

        if (use_highres_textures)
        {
            strcpy(baseName, textureName);
            baseName[strlen(textureName)-4] = '\0';
            strcpy(extName, textureName + (strlen(textureName)-4));
            dprintf(printf("terrain texture file name base: %s, ext: %s\n", baseName, extName);)
            strcpy(useName, baseName); strcat(useName, "_512"); strcat(useName, extName);
        }
        else
        {
            strcpy(useName, textureName);
        }

        textures[i] = driver->getTexture(useName);
    }

    // read skydome texture
    ret = fscanf(f, "%s\n%s\n%s\n",
                 skyDomeFileName,
                 roadfileName, objectfileName/*, grassfileName, treefileName*/);
    if (ret < 3)
    {
        printf("error reading %s ret %d errno %d\n", name, ret, errno);
        fclose(f);
        return;
    }
    
    if (skydome)
    {
        video::ITexture* sdt = driver->getTexture(skyDomeFileName);
        if (sdt)
            skydome->setMaterialTexture(0, sdt);
    }

    // read sounds for the terrain
    ret = fscanf(f, "%s\n%s\n%s\n", groundSoundName, puffSoundName, skidSoundName);
    if (ret < 3)
    {
        printf("error reading %s ret %d errno %d\n", name, ret, errno);
        fclose(f);
        return;
    }
    dprintf(printf("%s %s %s\n", groundSoundName, puffSoundName, skidSoundName);)

    ret = fscanf(f, "speed: %f\n", &speed);
    if (ret < 1)
    {
        printf("error reading %s ret %d errno %d\n", name, ret, errno);
        fclose(f);
        return;
    }
    dprintf(printf("speed: %f\n", speed);)
    
    // read the friction multiplier
    ret = fscanf(f, "friction_multi: %f\n", &friction_multi);
    if (ret < 1)
    {
        printf("error reading %s ret %d errno %d\n", name, ret, errno);
        fclose(f);
        return;
    }
    dprintf(printf("friction_multi: %f\n", friction_multi);)

    // read the vertical scale of the terrain
    ret = fscanf(f, "vscale: %f\n", &vscale);
    if (ret < 1)
    {
        printf("error reading %s ret %d errno %d\n", name, ret, errno);
        fclose(f);
        return;
    }
    dprintf(printf("vscale: %f\n", vscale);)

    // read the water height
    ret = fscanf(f, "water_height: %f\n", &waterHeight);
    if (ret < 1)
    {
        printf("error reading %s ret %d errno %d\n", name, ret, errno);
        fclose(f);
        return;
    }
    dprintf(printf("water_height: %f\n", waterHeight);)

    // read the checkpoints
    ret = fscanf(f, "start_pos: %f %f\n", &startPos.X, &startPos.Z);
    if (ret < 2)
    {
        printf("error reading %s ret %d errno %d\n", name, ret, errno);
        fclose(f);
        return;
    }
    dprintf(printf("start_pos: %f %f\n", startPos.X, startPos.Z);)

    ret = fscanf(f, "start_offset: %f %f\n", &startOffset.X, &startOffset.Z);
    if (ret < 2)
    {
        printf("error reading %s ret %d errno %d\n", name, ret, errno);
        fclose(f);
        return;
    }
    dprintf(printf("start_offset: %f %f\n", startOffset.X, startOffset.Z);)

    ret = fscanf(f, "start_rot: %f %f %f\n", &startRot.X, &startRot.Y, &startRot.Z);
    if (ret < 3)
    {
        printf("error reading %s ret %d errno %d\n", name, ret, errno);
        fclose(f);
        return;
    }
    dprintf(printf("start_rot: %f %f %f\n", startRot.X, startRot.Y, startRot.Z);)


    while (true)
    {
        ret = fscanf(f, "%s %f %f\n", dummyStr, &f1, &f2);
        if (ret < 3)
        {
            break;
        }
        
        if (strcmp(dummyStr, "end_pos:")==0)
        {
            // end_pos
            endPos.X = f1;
            endPos.Z = f2;            
        }
        else
        {
            // cp_pos
            cpPos.push_back(vector3df(f1, 0.0, f2));
        }
    }

    fclose(f);
    
    // comment this out if you want to generate, the newton cache for all the map
    //doCache();

/*
    str = L"Loading: 0%";
    MessageText::addText(str.c_str(), 1, true);

    for (int y = 0; y < max_y;y++)
    {
        for (int x = 0; x < max_x;x++)
        {
               
            dprintf(printf("------------- x: %d, y: %d (%d) ---------------\n", x ,y, x + (y*max_x)));
            map[x + (max_x*y)] =
               new SmallTerrain(
               heightmapName, terrainTextureName, detailmapName,
               terrainTexture0Name, terrainTexture1Name, terrainTexture2Name, terrainTexture3Name,
               smgr, driver, pnWorld, x, y, max_x, max_y, stHeightmapSize, tScale, shadowMap, stageNum);
        
            str = L"Loading: ";
            str += (((1 + x + (max_x*y))*70)/(max_x*max_y));
            str += "%";
            MessageText::addText(str.c_str(), 1, true);
        }
    }
    dprintf(printf("-------------------[ end ]-------------------\n"));
*/
    objectWire = new CObjectWire((float)max_x*SMALLTERRAIN_SIZE, (float)max_y*SMALLTERRAIN_SIZE);

    str = L"Loading: 40%";
    MessageText::addText(str.c_str(), 1, true);

	//driver->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS, false);
    //loadRoads_old(roadfileName, smgr, driver, core::vector3df(0.f, 0.f, 0.f));
    CMyRoad::loadRoads(roadfileName, roadList, smgr, driver, nWorld);
	//driver->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS, true);
    
    str = L"Loading: 45%";
    MessageText::addText(str.c_str(), 1, true);
/*    
    if (use_high_poly_objects)
    {
        char baseName[256];
        char extName[10];
        strcpy(baseName, objectfileName);
        baseName[strlen(objectfileName)-4] = '\0';
        strcpy(extName, objectfileName + (strlen(objectfileName)-4));
        dprintf(printf("object name base: %s, ext: %s\n", baseName, extName));
        strcpy(objectfileName, baseName); strcat(objectfileName, "_high"); strcat(objectfileName, extName);
    }
*/
    if (!use_mipmaps)
	    driver->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS, false);
    loadObjects(objectfileName, smgr, driver);
    if (!use_mipmaps)
    	driver->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS, true);

    calculateAIPointTimes();    

    str = L"Loading: 48%";
    MessageText::addText(str.c_str(), 1, true);
  
  /* 
    if (!use_mipmaps)
    	driver->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS, false);
    loadGrasses(grassfileName, smgr, driver, core::vector3df(0.f, 0.f, 0.f));
    if (!use_mipmaps)
    	driver->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS, true);

    str = L"Loading: 85%";
    MessageText::addText(str.c_str(), 1, true);

    if (!use_mipmaps)
    	driver->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS, false);
    loadTrees(treefileName, smgr, driver, core::vector3df(0.f, 0.f, 0.f));
    if (!use_mipmaps)
    	driver->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS, true);

    str = L"Loading: 91%";
    MessageText::addText(str.c_str(), 1, true);

    
    dprintf(printf("add objects to small terrains\n"));
    for (int i = 0; i<numOfObjects;i++)
    {
        core::vector3df pos;
        pos = objectWrappers[i]->getPosition();
        objectWire->addObject(pos, objectWrappers[i]);
        //getSmallTerrain(pos.X, pos.Z)->addObject(objectWrappers[i]);
        if (i % 1000 == 0) MessageText::addText(0, 1, true);
    }
*/

    dprintf(printf("add start pos\n"));
    // handle start and end gates
    
	startGate = smgr->addVolumeLightSceneNode(0, -1,
				32,                              // Subdivisions on U axis
				32,                              // Subdivisions on V axis
				video::SColor(0, 255, 255, 255), // foot color
				video::SColor(0, 0, 0, 0));      // tail color

    dprintf(printf("add cps\n"));
    for(int i = 0; i<cpPos.size(); i++)
    {
        cpGate.push_back(smgr->addVolumeLightSceneNode(0, -1,
				32,                              // Subdivisions on U axis
				32,                              // Subdivisions on V axis
				video::SColor(0, 255, 255, 255), // foot color
				video::SColor(0, 0, 0, 0)));      // tail color
    }

    dprintf(printf("add end pos\n"));
	endGate = smgr->addVolumeLightSceneNode(0, -1,
				32,                              // Subdivisions on U axis
				32,                              // Subdivisions on V axis
				video::SColor(0, 255, 255, 255), // foot color
				video::SColor(0, 0, 0, 0));      // tail color

    float* cpDist = new float[cpPos.size()];
    float sumDist = 0;

	if (startGate && endGate)
	{
		startGate->setScale(core::vector3df(GATE_LIMIT, GATE_LIMIT, GATE_LIMIT));
		startGate->setPosition(core::vector3df(startPos.X,0.f/*getHeight(startPos.X, startPos.Z)*/,startPos.Z));
		startGate->setVisible(false);
		startOffsetObject = new OffsetObject(startGate);
		offsetManager->addObject(startOffsetObject);
        for(int i = 0; i<cpGate.size(); i++)
        {
            cpGate[i]->setScale(core::vector3df(GATE_LIMIT, GATE_LIMIT, GATE_LIMIT));
            cpGate[i]->setPosition(core::vector3df(cpPos[i].X,0.f/*getHeight(cpPos[i].X, cpPos[i].Z)*/,cpPos[i].Z));
    		cpGate[i]->setVisible(false);
    		cpOffsetObject.push_back(new OffsetObject(cpGate[i]));
    		offsetManager->addObject(cpOffsetObject[i]);
            if (i == 0)
            {
                float dist = cpPos[i].getDistanceFrom(startPos);
                sumDist = dist;
                cpDist[i] = dist;
            }
            else
            {
                float dist = cpPos[i].getDistanceFrom(cpPos[i-1]);
                sumDist += dist;
                cpDist[i] = sumDist;
            }
            
        }
		endGate->setScale(core::vector3df(GATE_LIMIT, GATE_LIMIT, GATE_LIMIT));
		endGate->setPosition(core::vector3df(endPos.X,0.f/*getHeight(endPos.X, endPos.Z)*/,endPos.Z));
		endGate->setVisible(false);
		endOffsetObject = new OffsetObject(endGate);
		offsetManager->addObject(endOffsetObject);

        float dist = 0;
        if (cpPos.size() > 0)
            endPos.getDistanceFrom(cpPos[cpPos.size()-1]);
        else
            endPos.getDistanceFrom(startPos);
        sumDist += dist;
		// load textures for animation
		core::array<video::ITexture*> ltextures;
		for (s32 g=7; g > 0; --g)
		{
			core::stringc tmp;
			tmp = "data/objects/gate/portal";
			tmp += g;
			tmp += ".bmp";
			video::ITexture* t = driver->getTexture( tmp.c_str() );
			ltextures.push_back(t);
		}
		// create texture animator
		scene::ISceneNodeAnimator* glow = smgr->createTextureAnimator(ltextures, 150);
		// add the animator
		startGate->addAnimator(glow);
        for(int i = 0; i<cpGate.size(); i++) cpGate[i]->addAnimator(glow);
		endGate->addAnimator(glow);
		// drop the animator because it was created with a create() function
		glow->drop();

        for(int i = 0; i<cpPos.size(); i++)
        {
            cpTime.push_back((u32)((float)gtime*cpDist[i]/sumDist));
            cpTimed.push_back(0);
        }
	}
	cps = cpPos.size();
	delete [] cpDist;
    
    float boxP0[3];
    float boxP1[3];
    /*
    boxP0[0] = 0.f;
    boxP0[1] = -100.f;
    boxP0[2] = 0.f;
    boxP1[0] = (float)max_x*SMALLTERRAIN_SIZE;
    boxP1[1] = 5000.f;
    boxP1[2] = (float)max_y*SMALLTERRAIN_SIZE;
    */
    boxP0[0] = -1.1f*SMALLTERRAIN_SIZE;
    boxP0[1] = -100.f;
    boxP0[2] = -1.1f*SMALLTERRAIN_SIZE;
    boxP1[0] = 2.1f*SMALLTERRAIN_SIZE;
    boxP1[1] = 7000.f;
    boxP1[2] = 2.1f*SMALLTERRAIN_SIZE;
    dprintf(printf("nWorld bbox: %f %f %f\n", boxP1[0], boxP1[1], boxP1[2]);)
    //*(int*)1 = 1;
/*
    matrix4 bounding; 
    
    collision = NewtonCreateBox(nWorld, box.X, box.Y, box.Z, treeID, NULL);
    body = NewtonCreateBody(nWorld, collision);
    NewtonBodySetMaterialGroupID(body, treeID);
    NewtonBodyGetMatrix (body, bounding.pointer());
    bounging
    NewtonBodySetMatrix (body, bounding.pointer()); 
*/   
    
    NewtonSetWorldSize (nWorld, (float*)boxP0, (float*)boxP1);

// MEMORY TEST
//#define MEMORY_TEST
#ifdef MEMORY_TEST
    const int smallMapsCnt = 10;
    SmallTerrain** smallTerrains = new SmallTerrain*[smallMapsCnt];
    u32 tm = 0;
    u32 fm = 0;
    
    myMessage(0, "Begin memory test, watch memory usage. Start create %d small terrains\nMemory: %u", smallMapsCnt, getUsedMemory()/(1024));
    
    for (int i = 0; i < smallMapsCnt; i++)
    {
        smallTerrains[i] = new SmallTerrain(
           heightMap, densityMap, textureMap, textures, this->shadowMap,
           smgr, driver, nWorld,
           this->skydome,
           i, 2, max_x, max_y,
           stHeightmapSize, tScale,                           
           this->stageNum,
           this,
           objectReps, 0, roadList, m_terrainPool);
    }
    
    myMessage(1, "%d small terrains are created. Now free them.\nMemory: %u", smallMapsCnt, getUsedMemory()/(1024));
    
    for (int i = 0; i < smallMapsCnt; i++)
    {
        delete smallTerrains[i];
    }

    myMessage(1, "Test end.\nMemory: %u", getUsedMemory()/(1024));

    delete [] smallTerrains;

#endif // MEMORY_TEST
// END MEMORY TEST
}

BigTerrain::~BigTerrain()
{
    if (map)
    {
        for (int i = 0; i < max_x*max_y;i++)
        {
            printf("delete map %d\n", i);
            if (map[i])
            {
                delete map[i];
                map[i] = 0;
            }
        }
        delete [] map;
        map = 0;
    }
    destroyRoads_old();
    destroyObjects();
    //destroyGrasses();
    
    printf("delete objectWire\n");
    if (objectWire)
    {
        delete objectWire;
        objectWire = 0;
    }
    
    if (startGate) startGate->remove();
    if (startOffsetObject)
    {
        startOffsetObject->setNode(0);
        offsetManager->removeObject(startOffsetObject);
        delete startOffsetObject;
        startOffsetObject = 0;
    }
    //for(int i = 0; i<CP_NUM; i++) {if (cpGate[i]) cpGate[i]->remove();}
    for (unsigned int i = 0; i < cpGate.size(); i++) {if (cpGate[i]) cpGate[i]->remove();}
    for (unsigned int i = 0; i < cpOffsetObject.size(); i++)
    {
        if (cpOffsetObject[i])
        {
            cpOffsetObject[i]->setNode(0);
            offsetManager->removeObject(cpOffsetObject[i]);
            delete cpOffsetObject[i];
            cpOffsetObject[i] = 0;
        }
    }
    cpOffsetObject.clear();
    if (endGate) endGate->remove();
    if (endOffsetObject)
    {
        endOffsetObject->setNode(0);
        offsetManager->removeObject(endOffsetObject);
        delete endOffsetObject;
        endOffsetObject = 0;
    }
    if (densityMap)
    {
        densityMap->drop();
        densityMap = 0;
    }
    if (heightMap)
    {
#ifdef USE_IMAGE_HM
	   heightMap->drop();
#else
	   delete heightMap;
#endif
	   heightMap = 0;
    }
	if (textureMap)
	{
	   textureMap->drop();
	   textureMap = 0;
    }
    /*
	for (int i = 0; i < TERRAIN_TEXTURE_NUM; i++)
	{
        if (textures[i])
        {
            textures[i]->drop();
            textures[i] = 0;
        }
    }
    */
/*
    if (bodyl)
    {
        NewtonDestroyBody(nWorld, bodyl);
        bodyl = 0;
    }
    if (collisionl)
    {
	   NewtonReleaseCollision(nWorld, collisionl);
	   collisionl = 0;
    }
    if (bodyr)
    {
        NewtonDestroyBody(nWorld, bodyr);
        bodyr = 0;
    }
    if (collisionr)
    {
	   NewtonReleaseCollision(nWorld, collisionr);
	   collisionr = 0;
    }
    if (bodyu)
    {
        NewtonDestroyBody(nWorld, bodyu);
        bodyu = 0;
    }
    if (collisionu)
    {
	   NewtonReleaseCollision(nWorld, collisionu);
	   collisionu = 0;
    }
    if (bodyd)
    {
        NewtonDestroyBody(nWorld, bodyd);
        bodyd = 0;
    }
    if (collisiond)
    {
	   NewtonReleaseCollision(nWorld, collisiond);
	   collisiond = 0;
    }
*/
}
                         
core::vector3df BigTerrain::updatePos(float newX, float newY, int obj_density, bool force, bool showPerc)
{
// small terrain update    
    pdprintf(printf("b 1\n"));
    int new_x = (int)(newX / SMALLTERRAIN_SIZE);
    int new_y = (int)(newY / SMALLTERRAIN_SIZE);
    
    core::vector3df ret;
    const core::vector3df me = vector3df(newX, 0.f, newY);
    float distanceMin = FLT_MAX;

    {    
        pdprintf(printf("b 1b\n"));
        int nearestInd = -1;
        float nearestDist = 50.f;
        
        for (int i = 0; i < activeItinerPoints.size(); i++)
        {
            float dist = me.getDistanceFrom(activeItinerPoints[i]->getPosition());
            if (dist < nearestDist)
            {
                nearestInd = i;
                nearestDist = dist;
            }
        }
        if (nearestInd > -1)
        {
            ItinerHud::addItiner(activeItinerPoints[nearestInd]->getPool());
        }
        pdprintf(printf("b 1c\n"));
    }

    pdprintf(printf("b 2\n"));

    if (new_x != last_x || new_y != last_y)
    {
        if (force)
        {
            updateMaps(new_x, new_y, obj_density, showPerc);
        }
        else
        {
            // non-threaded way
            //updateMaps(new_x, new_y, obj_density, true);
            // non-threaded way, but in queue
            updateMapsAddToQueue(new_x, new_y, obj_density);
            // threaded way - not working!
            //mapReader->updateMap(this, new_x, new_y, obj_density);
        }
        last_x = new_x;
        last_y = new_y;
    }

    pdprintf(printf("b 3\n"));

// object update
    new_x = (int)(newX / TILE_SIZE);
    new_y = (int)(newY / TILE_SIZE);
//    new_x = (int)(newX / (OV_LIMIT/2));
//    new_y = (int)(newY / (OV_LIMIT/2));
    
    pdprintf(printf("b 4\n"));
    if (new_x != ov_last_x || new_y != ov_last_y || force)
    {
        //printf("update objs\n");
        dprintf(printf("b 5 %p\n", objectWire);)
        objectWire->updatePos(newX, newY, OV_LIMIT, force);
        dprintf(printf("b 6\n"));
        smallTerrainsForUpdateLock.lock();
        dprintf(printf("b 6b\n"));
        for (int i = 0; i < smallTerrainsForUpdate.size(); i++)
        {
            assert(smallTerrainsForUpdate[i]);
            dprintf(printf("b 6ba %p\n", smallTerrainsForUpdate[i]));
            smallTerrainsForUpdate[i]->updatePos(newX, newY, OV_LIMIT, force);
            dprintf(printf("b 6bb %p\n", smallTerrainsForUpdate[i]));
        }
        dprintf(printf("b 6c\n"));
        smallTerrainsForUpdateLock.unlock();
        dprintf(printf("b 6d\n"));
        //if (startTime==0)
        dprintf(printf("b 7\n"));
        if (!timeStarted)
        {
            if (fabsf(newX-startPos.X)<OV_LIMIT && fabsf(newY-startPos.Z)<OV_LIMIT)
            {
                vector3df pos = startGate->getPosition();
                pos.Y = getHeight(startPos.X, startPos.Z);
                dprintf(printf("STARTGATE: Y %f\n", pos.Y);)
                startGate->setPosition(pos);
                startGate->setVisible(true);
            }
            else
                startGate->setVisible(false);
        }
        dprintf(printf("b 8\n"));
        for (int i = 0; i<cpPos.size(); i++)
        {
            if (cpTimed[i]==0)
            {
                if (fabsf(newX-cpPos[i].X)<OV_LIMIT && fabsf(newY-cpPos[i].Z)<OV_LIMIT)
                {
                    vector3df pos = cpGate[i]->getPosition();
                    pos.Y = getHeight(cpPos[i].X, cpPos[i].Z);
                    cpGate[i]->setPosition(pos);
                    cpGate[i]->setVisible(true);
                }
                else
                    cpGate[i]->setVisible(false);
            }
        }
        //if (endTime==0)
        dprintf(printf("b 9\n"));
        if (!timeEnded)
        {
            if (fabsf(newX-endPos.X)<OV_LIMIT && fabsf(newY-endPos.Z)<OV_LIMIT)
            {
                vector3df pos = endGate->getPosition();
                pos.Y = getHeight(endPos.X, endPos.Z);
                endGate->setPosition(pos);
                endGate->setVisible(true);
            }
            else
                endGate->setVisible(false);
        }
        ov_last_x = new_x;
        ov_last_y = new_y;
    }
    pdprintf(printf("b 10\n"));

//    if (startTime==0)
    if (!timeStarted)
    {
        if(fabsf(newX-startPos.X)<GATE_LIMIT && fabsf(newY-startPos.Z)<GATE_LIMIT)
        {
            dprintf(printf("startpos\n"));
            core::stringw str = L"Start point passed of stage ";
            str += stages[oldStage]->stageNum;
            /*
            if (stages[oldStage]->stagePart > 0)
            {
                str += L" part ";
                str += stages[oldStage]->stagePart;
            }
            */
            str += L"!\n\nFollow the road to find the next checkpoint!";
            MessageText::addText(str.c_str(), 5);
            //startTime = device->getTimer()->getTime();
            timeStarted = true;
            lastTick = device->getTimer()->getTime();
            startGate->setVisible(false);
            offsetManager->removeObject(startOffsetObject);
        }
        else
        {
            ret = startPos - me;
        }
    }
    pdprintf(printf("b 11\n"));
    //if (startTime!=0)
    if (timeStarted)
    {
        for (int i = 0; i<cpPos.size(); i++)
        {
            if (cpTimed[i]==0)
            {
                if (fabsf(newX-cpPos[i].X)<GATE_LIMIT*3.f && fabsf(newY-cpPos[i].Z)<GATE_LIMIT*3.f)
                {
                    core::stringw str;
                
                    cps--;
                    str = L"Checkpoint ";
                    str += (i+1);
                    str += L" passed!\n";
                    //str += cpTime[i];


                    if (cps)
                    {
                        str += cps;
                        str += L" checkpoint(s) remaining.\n";
                    } else {
                        str += L"Only the end point remaining.\n";
                    }

                    cpTimed[i] = currentTime+penality;//device->getTimer()->getTime();
                    
                    /*
                    u32 position = 1;
                    //diffTime /= 1000;
                    if (cpTimed[i]>cpTime[i])
                        position += ((cpTimed[i] - cpTime[i]) / 3);
                    if (position > competitors.size()) position = competitors.size();
                    str += position;
                    if ((position-1)%10==0 && (position-11)%100!=0)
                        str += L"st";
                    else
                    if ((position-2)%10==0 && (position-12)%100!=0)
                        str += L"nd";
                    else
                    if ((position-3)%10==0 && (position-13)%100!=0)
                        str += L"rd";
                    else
                        str += L"th";
                    str += ".";
                    */

                    MessageText::addText(str.c_str(), 5);
                    dprintf(printf("cppos[%d], remaining cps: %d\n", i, cps));
                    cpGate[i]->setVisible(false);
                    offsetManager->removeObject(cpOffsetObject[i]);
                }
                else
                {
                    core::vector3df vec = cpPos[i] - me;
                    float distance = vec.getLengthSQ();
                    if (distance < distanceMin)
                    {
                        ret = vec;
                        distanceMin = distance;
                    }
                }
            }
        }    
    }
    pdprintf(printf("b 12\n"));
    //if (startTime!=0 && endTime==0)
    if (timeStarted && !timeEnded)
    {
        if (fabsf(newX-endPos.X)<GATE_LIMIT*2.f && fabsf(newY-endPos.Z)<GATE_LIMIT*2.f)
        {
            dprintf(printf("endpos\n"));
            core::stringw str = L"You have reached the endpoint of stage ";
            str += stages[oldStage]->stageNum;
            /*
            if (stages[oldStage]->stagePart > 0)
            {
                str += L" part ";
                str += stages[oldStage]->stagePart;
            }
            */
            if (cps)
            {
                str += L", but you missed ";
                str += cps;
                str += L" checkpoint(s)\n\n";
                addPenality(cps*300);
            } else {
                str += L"!\n\n"; //  Congratulation!!!
            }

            if (penality>0)
            {
                str += L"Your penality is ";
                addTimeToStr(str, penality);
                str += L"\n\n";
            }
            else
            {
                str += L"You don't have any penality!\n\n";
            }

            //endTime = device->getTimer()->getTime() + penality;
            currentTime += penality;
            timeEnded = true;
            showCompass = false;
            hudCompassImage->setVisible(false);
            compassText->setVisible(false);
            compassArrow->setVisible(false);
            u32 diffTime = currentTime; //endTime - startTime;
            endGate->setVisible(false);
            offsetManager->removeObject(endOffsetObject);

            str += L"Your time is ";
            addTimeToStr(str, diffTime);
            if (penality>0)
                str += L" including the penalities!";

            /*
            u32 position = 1;
            //diffTime /= 1000;
            if (diffTime>stageTime)
                position += ((diffTime - stageTime) / 3);
            if (position > competitors.size()) position = competitors.size();
            if (position==1)
                str += L"\n\nYou won this stage!";
            else
            {
                str += L"\n\nYou reached ";
                str += position;
                if ((position-1)%10==0 && (position-11)%100!=0)
                    str += L"st";
                else
                if ((position-2)%10==0 && (position-12)%100!=0)
                    str += L"nd";
                else
                if ((position-3)%10==0 && (position-13)%100!=0)
                    str += L"rd";
                else
                    str += L"th";
                str += L" position in this stage!";
            }
        
            position = 1;
            //if ((globalTime/1000)+diffTime>gtime)
            //    position += ((((globalTime/1000)+diffTime) - gtime) / ((oldStage + 1)*3));
            if (globalTime+diffTime>gtime)
                position += (((globalTime+diffTime) - gtime) / ((oldStage + 1)*3));
            if (position > competitors.size()) position = competitors.size();
            if (oldStage+1 >= MAX_STAGES || stages[oldStage+1] == 0)
                str += L"\n\n\nCongratulation!!! You have successfully completed the Dakar 2010.\n\nYou finished in ";
            else
                str += L"\n\nYou are in ";
            str += position;
            if ((position-1)%10==0 && (position-11)%100!=0)
                str += L"st";
            else
            if ((position-2)%10==0 && (position-12)%100!=0)
                str += L"nd";
            else
            if ((position-3)%10==0 && (position-13)%100!=0)
                str += L"rd";
            else
                str += L"th";
            if (oldStage+1 >= MAX_STAGES || stages[oldStage+1] == 0)
                str += L" position!";
            else
                str += L" position in the Dakar rally!";
            */
            str += L"\n\nYou can drive further or check your position in the Standings.";
            
            if (oldStage+1 >= MAX_STAGES || stages[oldStage+1] == 0)
                MessageText::addText(str.c_str(), /*40*/10);
            else
                MessageText::addText(str.c_str(), 10);
            
            // new stuff
            playerCompetitor->lastTime = diffTime;
            playerCompetitor->globalTime += diffTime;
            if (raceEngine)
                raceEngine->insertIntoFinishedState(playerCompetitor);
        }
        else
        {
            core::vector3df vec = endPos - me;
            float distance = vec.getLengthSQ();
            if (distance < distanceMin)
            {
                ret = vec;
                distanceMin = distance;
            }
        }
    }
    pdprintf(printf("b 13\n"));
    
    return ret;
}

void BigTerrain::updateTime(u32 ptick)
{
    if (!timeStarted || timeEnded) return;
    const u32 ctick = ptick/1000;
    
    if (ctick != lastTick)
    {
        currentTime++;
        if (showCompass)
            addPenality(1);
        lastTick = ctick;
    }
}

float BigTerrain::getHeight(float x, float y) const
{
    int _x = (int)(x / SMALLTERRAIN_SIZE);
    int _y = (int)(y / SMALLTERRAIN_SIZE);
    if (_x > max_x || _y > max_y || !map[_x + (max_x*_y)])
       return 0.f;

    //if (!map[_x + (max_x*_y)]->terrain) map[_x + (max_x*_y)]->activate();

    return map[_x + (max_x*_y)]->terrain->getHeight(x - offsetManager->getOffset().X, y - offsetManager->getOffset().Z);
}

float BigTerrain::getDensity(float x, float y, int category) const
{
    int _x = (int)(x / TERRAIN_SCALE);
    int _y = (int)(y / TERRAIN_SCALE);
    if (!densityMap || _x < 0 || _x >= densityMap->getDimension().Width ||
                       _y < 0 || _y >= densityMap->getDimension().Height)
       return 0;

    u32 ret = 0;
    if (category == 0 || ((category & 1) == 1))
    {
        ret += densityMap->getPixel(_x, _y).getRed();
    }
    if (category == 0 || ((category & 2) == 2))
    {
        ret += densityMap->getPixel(_x, _y).getGreen();
    }
    if (category == 0 || ((category & 4) == 4))
    {
        ret += densityMap->getPixel(_x, _y).getBlue();
    }

    if (ret > 255) ret = 255;

    pdprintf(printf("%d %d ret: %d (cat: %d) (r: %u, g: %u, b: %u)\n", _x, _y, ret, category,
        densityMap->getPixel(_x, _y).getRed(), densityMap->getPixel(_x, _y).getGreen(), densityMap->getPixel(_x, _y).getBlue());)

    return ((float)(ret))/255.f;
}

scene::ISceneNode* BigTerrain::getTerrain(float x, float y) const
{
    int _x = (int)(x / SMALLTERRAIN_SIZE);
    int _y = (int)(y / SMALLTERRAIN_SIZE);
    if (_x > max_x || _y > max_y || !map[_x + (max_x*_y)])
       return 0;

    //if (!map[_x + (max_x*_y)]->terrain) map[_x + (max_x*_y)]->activate();

    return map[_x + (max_x*_y)]->terrain;
}

SmallTerrain* BigTerrain::getSmallTerrain(float x, float y) const
{
    int _x = (int)(x / SMALLTERRAIN_SIZE);
    int _y = (int)(y / SMALLTERRAIN_SIZE);
    if (_x > max_x || _y > max_y)
       return 0;

    return map[_x + (max_x*_y)];
}

float BigTerrain::getSmallTerrainSize() const
{
    return SMALLTERRAIN_SIZE;
}

float BigTerrain::getTerrainSize() const
{
    return SMALLTERRAIN_SIZE*max_x;
}

void BigTerrain::loadRoads_old(const c8* name, ISceneManager* smgr, IVideoDriver* driver,
                               const vector3df& loc)
{
    FILE* f;
    int numOfRoads = 0;
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

    dprintf(printf("Read old roads: %s\n", name));
    dprintf(printf("roads offset: %f %f %f\n", loc.X, loc.Y, loc.Z));
    
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

    dprintf(printf("%u old road(s) found\n", numOfRoads));
    
    //roadMeshes = new IAnimatedMesh*[numOfRoads];
    //roadNodes = new ISceneNode*[numOfRoads];
    //roadWrappers = new SRoadWrapper*[numOfRoads];
    roadWrappers_old.clear();
    
    for (int i = 0;i < numOfRoads;i++)
    {
	    SMeshBuffer* buffer = new SMeshBuffer();
	    SMesh* mesh = new SMesh();
	    
	    roadWrappers_old.push_back(new SRoadWrapper_old());
	    
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
            vtx.Pos.Y = getHeight(x + loc.X, z + loc.Z) + 0.05f;
            if (fabsf(vtx.Pos.Y) > 2000.f) vtx.Pos.Y = 0.0f;
            vtx.TCoords.X = tu;
            vtx.TCoords.Y = tv*0.05f;
            buffer->Vertices.push_back(vtx);
            //getSmallTerrain(x + loc.X, z + loc.Z)->addRoad_old(roadWrappers_old[i]);
            if (ind % 1000 == 0) MessageText::addText(0, 1, true);
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
	   
	   //roadMeshes[i] = animatedMesh;
       roadWrappers_old[i]->roadNode = smgr->addAnimatedMeshSceneNode(animatedMesh);
       roadWrappers_old[i]->roadNode->getMaterial(0).MaterialTypeParam = 0.1f;
////       roadWrappers[i]->roadNode->getMaterial(0).TextureLayer[0].TextureWrap = video::ETC_CLAMP;
       if (useShaders)
           roadWrappers_old[i]->roadNode->setMaterialFlag(video::EMF_LIGHTING, false);
       else
           roadWrappers_old[i]->roadNode->setMaterialFlag(video::EMF_LIGHTING, globalLight);
/*
       if (globalLight)
       {
	       roadWrappers[i]->roadNode->setMaterialFlag(video::EMF_NORMALIZE_NORMALS, true);
       }
*/
       if (useShaders && useCgShaders)
        roadWrappers_old[i]->roadNode->setMaterialType((video::E_MATERIAL_TYPE)myMaterialType_transp_road);
       else
        roadWrappers_old[i]->roadNode->setMaterialType(video::EMT_TRANSPARENT_ALPHA_CHANNEL);
//       roadWrappers[i]->roadNode->setMaterialType(video::EMT_SOLID);
//       roadWrappers[i]->roadNode->setMaterialType((video::E_MATERIAL_TYPE)myMaterialType_smoke);
	   roadWrappers_old[i]->roadNode->setMaterialTexture(0, driver->getTexture(textureName));
	   //roadWrappers[i]->roadNode->setMaterialTexture(1, driver->getTexture("data/roads/sand_road_hm.bmp"));
	   roadWrappers_old[i]->roadNode->setVisible(false);
	   roadWrappers_old[i]->roadNode->setPosition(loc);
	   //roadWrappers[i]->roadNode->setMaterialType(video::EMT_PARALLAX_MAP_SOLID );
//       roadWrappers[i]->roadNode->setMaterialType(video::EMT_PARALLAX_MAP_TRANSPARENT_VERTEX_ALPHA);
    }
    
    fclose(f);
    
}

void BigTerrain::destroyRoads_old()
{
    printf("Destroy big old roads #%d\n", roadWrappers_old.size());
    for (int i = 0; i < roadWrappers_old.size();i++)
    {
//       roadMeshes[i]->drop();
       roadWrappers_old[i]->roadNode->remove();
       delete roadWrappers_old[i];       
    }
    roadWrappers_old.clear();
}

void BigTerrain::loadObjects(const c8* name, ISceneManager* smgr, IVideoDriver* driver)
{
    FILE* f;
    int ret = 0;
    int repeat = 0;
    c8 meshName[256];
    c8 type[256];
    vector3df pos(0.f,0.f,0.f);

    dprintf(printf("Read objects: %s\n", name));
    
    f = fopen(name, "r");
    
    if (!f)
    {
        printf("objects file unable to open: %s\n", name);
        return;       
    }

    srand(1); // set seed, wand object to the same place always
    
    //objectMeshes = new IAnimatedMesh*[MAX_OBJECT_NUM];
    //objectNodes = new IAnimatedMeshSceneNode*[MAX_OBJECT_NUM];
    //memset(objectMeshes, 0, sizeof(void*)*MAX_OBJECT_NUM);
    //memset(objectNodes, 0, sizeof(void*)*MAX_OBJECT_NUM);
    while (true)
    {
        int poolId = 0;
        
        ret = fscanf(f, "%s %s", type, meshName);
        if (ret < 2)
        {
            // no more object
            break;
        }
        
        if (strcmp(type, "fix:")==0)
        {
            // fix positioned objects
            ret = fscanf(f, "%f %f", &pos.X, &pos.Z);
            if (ret < 2)
            {
                // no more object
                printf("objects file unexpectedly ended: %s\n", name);
                break;
            }
            poolId = getPoolIdFromName(meshName);
            SObjectWrapper* objectWrapper;
            if (poolId == 0)
            {
                SAIPoint* aiPoint = new SAIPoint(this);
                aiPoints.push_back(aiPoint);
                objectWrapper = aiPoint;
            }
            else
            {
                objectWrapper = new SObjectWrapper(this);
                objectWrappers.push_back(objectWrapper);
            }
            objectWrapper->setPosition(pos);
            objectWrapper->setPool(poolId);
            objectWire->addObject(pos, objectWrapper);
        }
        else
        if (strcmp(type, "iti:")==0)
        {
            // fix positioned objects
            ret = fscanf(f, "%f %f", &pos.X, &pos.Z);
            if (ret < 2)
            {
                // no more object
                printf("objects file unexpectedly ended: %s\n", name);
                break;
            }
            poolId = getItinerIdFromName(meshName);
            SItinerPoint* objectWrapper = new SItinerPoint(this);
            objectWrapper->setPosition(pos);
            objectWrapper->setPool(poolId);
            objectWrappers.push_back(objectWrapper);
            objectWire->addObject(pos, objectWrapper);
        }
        else
        {
            // fix positioned objects
            ret = fscanf(f, "%d", &repeat);
            if (ret < 1)
            {
                // no more object
                printf("objects file unexpectedly ended: %s\n", name);
                break;
            }
            // random positioned objects
            poolId = getPoolIdFromName(meshName);
            objectReps.push_back(SObjectPoolIdRepPair(poolId, repeat));
        }
        
    }
    fclose(f);
    
    dprintf(printf("%d fix object(s) and %d repeatable object(s) found\n", objectWrappers.size(), objectReps.size()));
}

void BigTerrain::destroyObjects()
{
    printf("Destroy big objects #%d\n", objectWrappers.size());
    for (int i = 0; i < objectWrappers.size();i++)
    {
//       objectMeshes[i]->drop();
       delete objectWrappers[i];
    }
    objectWrappers.clear();
    activeItinerPoints.clear();
    printf("Destroy big AI points #%d\n", aiPoints.size());
    for (int i = 0; i < aiPoints.size();i++)
    {
//       objectMeshes[i]->drop();
       delete aiPoints[i];
    }
    aiPoints.clear();
}

void BigTerrain::setOVLimit(float new_val)
{
    ov_limit = new_val;
}

float BigTerrain::getOVLimit()
{
    return ov_limit;
}

void BigTerrain::addTimeToStr(core::stringw& str, u32 diffTime)
{
    str += diffTime/3600; // hours
    str += " : ";
    if ((diffTime/60)%60==0)
        str += "00";
    else
        {
            if ((diffTime/60)%60<10)
                str += "0";
            str += (diffTime/60)%60; // minutes
        }
    str += " : ";
    if (diffTime%60==0)
        str += "00";
    else
        {
            if (diffTime%60<10)
                str += "0";
            str += diffTime%60;  // sec
        }
}

u32 BigTerrain::addPenality(u32 ap)
{
    //if (startTime && !endTime)
    if (timeStarted && !timeEnded)
    {
        penality += ap;
        return penality;
    }
    return (u32)-1;
}

void BigTerrain::saveState(SState* state)
{
    state->currentTime = currentTime;
    state->cps = cps;
    state->penality = penality;
    state->timeStarted = timeStarted;
    state->timeEnded = timeEnded;
    
    state->cpTime = cpTime;
    state->cpTimed = cpTimed;
}

void BigTerrain::restoreState(SState* state)
{
    currentTime = state->currentTime;
    cps = state->cps;
    penality = state->penality;
    timeStarted = state->timeStarted;
    timeEnded = state->timeEnded;
    
    dprintf(printf("BigTerrain::restoreState(): cpTime.size() %u state->cpTime.size() %u\n", cpTime.size(), state->cpTime.size());)
    
    assert(cpTime.size() == state->cpTime.size());
    assert(cpTimed.size() == state->cpTimed.size());
    
    for (int i = 0; i < state->cpTime.size();i++)
    {
        cpTime[i] = state->cpTime[i];
        cpTimed[i] = state->cpTimed[i];
    }
}

void BigTerrain::updateMaps(int new_x, int new_y, int obj_density, bool showPerc)
{
    int percentage = 50;
    core::stringw str;
    
    int added = 0;
    int removed = 0;
    
    dprintf(printf("-----------\nupdate maps begin\n------------\n");)
    
    smallTerrainsForUpdateLock.lock();
    smallTerrainsForUpdate.clear();
    smallTerrainsForUpdateLock.unlock();

    //printf("update small maps\n");
    for (int x = 0; x < max_x;x++)
    {
        for (int y = 0; y < max_y;y++)
        {
            if( abs(x-new_x) <= 1 && abs(y-new_y) <= 1)
            {
                if (map[x + (max_x*y)] == 0)
                {
                    added++;
                    map[x + (max_x*y)] = 
                       new SmallTerrain(
                           heightMap, densityMap, textureMap, textures, this->shadowMap,
                           smgr, driver, nWorld,
                           this->skydome,
                           x, y, max_x, max_y,
                           stHeightmapSize, tScale,                           
                           this->stageNum,
                           this,
                           objectReps, obj_density, roadList, m_terrainPool,
                           vscale, waterHeight);
                }
                smallTerrainsForUpdateLock.lock();
                smallTerrainsForUpdate.push_back(map[x + (max_x*y)]);
                smallTerrainsForUpdateLock.unlock();
                if (showPerc)
                {
                    percentage += 5;
                    str = L"Loading: ";
                    str += percentage;
                    str += L"%";
                    MessageText::addText(str.c_str(), 1, true);
                }
            }
            else
            {
                if (map[x + (max_x*y)])
                {
                    removed++;
                    delete map[x + (max_x*y)];
                    map[x + (max_x*y)] = 0;
                }
            }
        }
    }
    //myMessage(3, "Added maps: %d, removed maps: %d", added, removed);
    dprintf(printf("-----------\nupdate maps end added: %d, removed: %d\n------------\n", added, removed);)
}

void BigTerrain::updateMapsAddToQueue(int new_x, int new_y, int obj_density)
{
    dprintf(printf("-----------\nupdate maps to queue begin\n------------\n");)
    
    mapLock.lock();
    smallTerrainsForUpdateLock.lock();
    smallTerrainsForUpdate.clear();
    smallTerrainsForUpdateLock.unlock();

    mapsQueueVersion++;
    while (mapsQueue.size())
    {
        delete mapsQueue.removeFirst();
    }

    //printf("update small maps\n");
    for (int x = 0; x < max_x;x++)
    {
        for (int y = 0; y < max_y;y++)
        {
            if( abs(x-new_x) <= 1 && abs(y-new_y) <= 1)
            {
                if (map[x + (max_x*y)] == 0)
                {
                    SMapsQueueElement* sQE = new SMapsQueueElement;
                    sQE->x = x;
                    sQE->y = y;
                    sQE->obj_density = obj_density;
                    sQE->visible = true;
                    sQE->version = mapsQueueVersion;
                    mapsQueue.push_back(sQE);
                }
                else
                {
                    smallTerrainsForUpdateLock.lock();
                    smallTerrainsForUpdate.push_back(map[x + (max_x*y)]);
                    smallTerrainsForUpdateLock.unlock();
                    //for (int i = 0; i < mapsQueue.size();i++)
                    /*
                    for (CMyList<SMapsQueueElement*>::element* iter = mapsQueue.getIterator();
                          iter != mapsQueue.getEnd();
                          iter = iter->next)
                    {
                        if (iter->data->x == x && iter->data->y == y)
                        {
                            assert(iter->data->visible==false);
                            delete iter->data;
                            iter->data = 0;
                            mapsQueue.del(iter);
                            break;
                        }
                    }
                    */
                }
            }
            else
            {
                if (map[x + (max_x*y)])
                {
                    /*
                    for (CMyList<SMapsQueueElement*>::element* iter = mapsQueue.getIterator();
                          iter != mapsQueue.getEnd();
                          iter = iter->next)
                    {
                        if (iter->data->x == x && iter->data->y == y)
                        {
                            assert(iter->data->visible);
                            delete iter->data;
                            iter->data = 0;
                            mapsQueue.del(iter);
                            break;
                        }
                    }
                    */
                    delete map[x + (max_x*y)];
                    map[x + (max_x*y)] = 0;
                    /*
                    SMapsQueueElement* sQE = new SMapsQueueElement;
                    sQE->x = x;
                    sQE->y = y;
                    sQE->obj_density = obj_density;
                    sQE->visible = false;
                    mapsQueue.push_back(sQE);
                    */
                }
            }
        }
    }
    mapLock.unlock();

    dprintf(printf("-----------\nupdate maps to queue end\n------------\n");)
}

void BigTerrain::checkMapsQueue()
{

    if (mapsQueue.size() > 0)
    {
        lastMapsQueueUpdate++;
        if (lastMapsQueueUpdate%100) return;
        SMapsQueueElement* mQE = mapsQueue.removeFirst();
        if (use_threads)
        {
            // threaded version
            mapReader->updatePieceOfMap(this, mQE);
        }
        else
        {
            // non-threaded version
            checkMapsQueueThread(mQE);
        }
    }
}

void BigTerrain::checkMapsQueueThread(SMapsQueueElement* mQE)
{
    mapLock.lock();
    if (mapsQueueVersion == mQE->version)
    {
        dprintf(printf("-----------\ncheck maps queue begin\n------------\n");)
        const int x = mQE->x;
        const int y = mQE->y;
        const int obj_density = mQE->obj_density;
        if (mQE->visible)
        {
            //assert(map[x + (max_x*y)] == 0);
            if (map[x + (max_x*y)] == 0)
            {
                map[x + (max_x*y)] = 
                   new SmallTerrain(
                       heightMap, densityMap, textureMap, textures, this->shadowMap,
                       smgr, driver, nWorld,
                       this->skydome,
                       x, y, max_x, max_y,
                       stHeightmapSize, tScale,
                       this->stageNum,
                       this,
                       objectReps, obj_density, roadList, m_terrainPool,
                       vscale, waterHeight);
                dprintf(printf("new map(%d, %d): %p\n", x, y, map[x + (max_x*y)]);)
                smallTerrainsForUpdateLock.lock();
                smallTerrainsForUpdate.push_back(map[x + (max_x*y)]);
                smallTerrainsForUpdateLock.unlock();
            }
        }
        else
        {
            assert(0 && "should not happen");
            //assert(map[x + (max_x*y)] != 0);
            if (map[x + (max_x*y)] != 0)
            {
                dprintf(printf("remove map(%d, %d): %p\n", x, y, map[x + (max_x*y)]);)
                delete map[x + (max_x*y)];
                map[x + (max_x*y)] = 0;
            }
        }
    }
    delete mQE;
    dprintf(printf("-----------\ncheck maps queue end\n------------\n");)
    mapLock.unlock();
}

void BigTerrain::updateRoads()
{
    //printf("update small maps\n");
    for (int x = 0; x < max_x;x++)
    {
        for (int y = 0; y < max_y;y++)
        {
            if (map[x + (max_x*y)] != 0)
            {
                map[x + (max_x*y)]->updateRoads(roadList, 1, shadowMap);
            }
        }
    }
}

void BigTerrain::addNewRoad()
{
    if (roadList.size() == 0) return;
    
    CMyRoad* road = new CMyRoad(smgr, driver, nWorld);
    road->setSlicePoints(roadList[roadList.size()-1]->getSlicePoints());
    road->setSliceIndices(roadList[roadList.size()-1]->getSliceIndices());
    road->setTexture(roadList[roadList.size()-1]->getTexture());
    road->setTextureName(roadList[roadList.size()-1]->getTextureName());
    roadList.push_back(road);
}

void BigTerrain::loadRoads(const char* roadsName)
{
    CMyRoad::loadRoads(roadsName, roadList, smgr, driver, nWorld);
}

void BigTerrain::updateObjectWire()
{
    dprintf(printf("BT::updateObjectWire() objWrappers size %d\n", objectWrappers.size());)
    if (objectWire)
    {
        delete objectWire;
    }
    objectWire = new CObjectWire((float)max_x*SMALLTERRAIN_SIZE, (float)max_y*SMALLTERRAIN_SIZE);
    for (int i = 0; i < objectWrappers.size(); i++)
    {
        objectWire->addObject(objectWrappers[i]->getPosition(), objectWrappers[i]);
    }
}

void BigTerrain::saveObjects(const c8* name)
{
    FILE* f;
    int ret = 0;
    //int repeat = 0;
    //c8 meshName[256];
    //c8 type[256];
    //vector3df pos(0.f,0.f,0.f);

    dprintf(printf("Write objects: %s\n", name));
    
    f = fopen(name, "w");
    
    if (!f)
    {
        printf("objects file unable to open for write: %s\n", name);
        return;       
    }

    // write only itiner points
    for (int i = 0; i < objectWrappers.size(); i++)
    {
        if (objectWrappers[i]->getPool()>=ITINER_POOLID_OFFSET)
        {
            ret = fprintf(f, "iti: %s", getItinerNameFromId(objectWrappers[i]->getPool()));
            if (ret < 1)
            {
                // no more object
                break;
            }
            ret = fprintf(f, " %f %f\n", objectWrappers[i]->getPosition().X, objectWrappers[i]->getPosition().Z);
            if (ret < 2)
            {
                // no more object
                printf("objects file unexpectedly ended at write: %s\n", name);
                break;
            }
        }
    }

    // write ai points
    for (int i = 0; i < aiPoints.size(); i++)
    {
        ret = fprintf(f, "fix: %s", getPoolNameFromId(aiPoints[i]->getPool()));
        if (ret < 1)
        {
            // no more object
            break;
        }

        ret = fprintf(f, " %f %f\n", aiPoints[i]->getPosition().X, aiPoints[i]->getPosition().Z);
        if (ret < 2)
        {
            // no more object
            printf("objects file unexpectedly ended at write: %s\n", name);
            break;
        }
    }

    // write only ai points inside object wrappers
    for (int i = 0; i < objectWrappers.size(); i++)
    {
        if (objectWrappers[i]->getPool() == 0)
        {
            ret = fprintf(f, "fix: %s", getPoolNameFromId(objectWrappers[i]->getPool()));
            if (ret < 1)
            {
                // no more object
                break;
            }
            ret = fprintf(f, " %f %f\n", objectWrappers[i]->getPosition().X, objectWrappers[i]->getPosition().Z);
            if (ret < 2)
            {
                // no more object
                printf("objects file unexpectedly ended at write: %s\n", name);
                break;
            }
        }
    }

    // write only non-itiner and non-ai points
    for (int i = 0; i < objectWrappers.size(); i++)
    {
        if (objectWrappers[i]->getPool() < ITINER_POOLID_OFFSET && objectWrappers[i]->getPool() != 0)
        {
            ret = fprintf(f, "fix: %s", getPoolNameFromId(objectWrappers[i]->getPool()));
            if (ret < 1)
            {
                // no more object
                break;
            }
            ret = fprintf(f, " %f %f\n", objectWrappers[i]->getPosition().X, objectWrappers[i]->getPosition().Z);
            if (ret < 2)
            {
                // no more object
                printf("objects file unexpectedly ended at write: %s\n", name);
                break;
            }
        }
    }

    // write random objects
    for (int i = 0; i < objectReps.size(); i++)
    {
        // fix positioned objects
        ret = fprintf(f, "ran: %s %d\n", getPoolNameFromId(objectReps[i].poolId), objectReps[i].rep);
        if (ret < 2)
        {
            // no more object
            printf("objects file unexpectedly ended: %s\n", name);
            break;
        }
    }

    fclose(f);
    
    dprintf(printf("%d fix object(s) written\n", objectWrappers.size());)
}

void BigTerrain::addActiveItinerPoint(SItinerPoint* itinerPoint)
{
    activeItinerPoints.push_back(itinerPoint);
}

void BigTerrain::removeActiveItinerPoint(SItinerPoint* itinerPoint)
{
    for (int i = 0; i < activeItinerPoints.size(); i++)
    {
        if (activeItinerPoints[i] == itinerPoint)
        {
            activeItinerPoints.erase(i);
            break;
        }
    }
}

void BigTerrain::applyRoadOnHeightMap()
{
#ifdef USE_IMAGE_HM
    int sizeX = heightMap->getDimension().Width;
    int sizeY = heightMap->getDimension().Height;
#else
    int sizeX = heightMap->getXSize();
    int sizeY = heightMap->getYSize();
#endif
    int avgCol = 0;
    int percentage = 0;
    core::stringw str = L"";

    // first pass
    for (int x = 0; x < sizeX; x++)
    {
        if (x%50==0)
        {
            percentage++;
            int shownum = 20 + ((5*percentage) / (sizeX / 50));
            str = L"Loading: ";
            str += shownum;
            str += L"%";
            MessageText::addText(str.c_str(), 1, true);
            //MessageText::refresh();
        }
        for (int y = 0; y < sizeY; y++)
        {
            if (isRoad(x, y))
            {
                avgCol = getAverage(x, y);
                applyAverage(x, y, avgCol);
            }
        }
    }

    // second pass back
    for (int x = sizeX-1; x >= 0; x--)
    {
        if (x%50==0)
        {
            percentage++;
            int shownum = 20 + ((5*percentage) / (sizeX / 50));
            str = L"Loading: ";
            str += shownum;
            str += L"%";
            MessageText::addText(str.c_str(), 1, true);
            //MessageText::refresh();
        }
        for (int y = sizeY-1; y >= 0; y--)
        {
            if (isRoad(x, y))
            {
                avgCol = getAverage2(x, y);
                applyAverage2(x, y, avgCol);
            }
        }
    }

    // second pass
    for (int x = 0; x < sizeX; x++)
    {
        if (x%50==0)
        {
            percentage++;
            int shownum = 20 + ((5*percentage) / (sizeX / 50));
            str = L"Loading: ";
            str += shownum;
            str += L"%";
            MessageText::addText(str.c_str(), 1, true);
            //MessageText::refresh();
        }
        for (int y = 0; y < sizeY; y++)
        {
            if (isRoad(x, y))
            {
                avgCol = getAverage2(x, y);
                applyAverage2(x, y, avgCol);
            }
        }
    }

    // third pass
    for (int x = 0; x < sizeX; x++)
    {
        if (x%50==0)
        {
            percentage++;
            int shownum = 20 + ((5*percentage) / (sizeX / 50));
            str = L"Loading: ";
            str += shownum;
            str += L"%";
            MessageText::addText(str.c_str(), 1, true);
            //MessageText::refresh();
        }
        for (int y = 0; y < sizeY; y++)
        {
            if (!isRoad(x, y) && isNextToRoad(x, y)) // change  -> !
            {
                avgCol = getAverage3(x, y); // change 2 -> 3
                applyAverage3(x, y, avgCol);
            }
        }
    }

}

bool BigTerrain::isRoad(int x, int y)
{
#ifdef USE_IMAGE_HM
    SColor c = heightMap->getPixel(x, y);
    return c.getRed()-5 > c.getGreen();
#else
    return heightMap->getRoad(x, y);
#endif
}

bool BigTerrain::isNextToRoad(int px, int py, int dist)
{
    for (int x = px-dist; x <= px+dist; x++)
    {
        for (int y = py-dist; y <= py+dist; y++)
        {
#ifdef USE_IMAGE_HM
            if (x >= 0 && y >= 0 && x < heightMap->getDimension().Width && y < heightMap->getDimension().Height
#else
            if (x >= 0 && y >= 0 && x < heightMap->getXSize() && y < heightMap->getYSize()
#endif
                && isRoad(x, y))
            {
                return true;
            }
        }
    }
    return false;
}

int BigTerrain::getAverage(int px, int py)
{
    int avgCol = 0;
    int avgCnt = 0;
    const int dist = 1;
    //core::array<int> heights;
    
    for (int x = px-dist; x <= px+dist; x++)
    {
        for (int y = py-dist; y <= py+dist; y++)
        {
#ifdef USE_IMAGE_HM
            if (x >= 0 && y >= 0 && x < heightMap->getDimension().Width && y < heightMap->getDimension().Height
                && !isRoad(x, y)
#else
            if (x >= 0 && y >= 0 && x < heightMap->getXSize() && y < heightMap->getYSize()
#endif
                )
            {
#ifdef USE_IMAGE_HM
                avgCnt++;
                avgCol += heightMap->getPixel(x, y).getRed();
#else
                if (isRoad(x, y))
                {
                    avgCnt+=3;
                    avgCol += heightMap->get(x, y)*3;
                }
                else
                {
                    avgCnt++;
                    avgCol += heightMap->get(x, y);
                }
#endif
                //int i = 0;
                //while (i < heights.size() && heightMap->getPixel(x, y).getRed() < heights[i]) i++;
                //heights.insert(heightMap->getPixel(x, y).getRed(), i);
            }
        }
    }
    //return heights.size()>0 ? heights[0] : 0;
    //return heights.size()>0 ? heights[heights.size()/2] : 0;
    return avgCnt ? (avgCol/avgCnt) : 0;
}

void BigTerrain::applyAverage(int px, int py, int avgCol)
{
#ifdef USE_IMAGE_HM
    SColor col(0, avgCol, 0, 0);
    heightMap->setPixel(px, py, col);
#else
    heightMap->set(px, py, avgCol);
#endif
/*    
    for (int x = px-1; x <= px+1; x++)
    {
        for (int y = py-1; y <= py+1; y++)
        {
            if (x >= 0 && y >= 0 && x < heightMap->getDimension().Width && y < heightMap->getDimension().Height
                && !isRoad(x, y) && !isNextToRoad(x, y))
            {
                heightMap->setPixel(x, y, col);
            }
        }
    }
*/
}

int BigTerrain::getAverage2(int px, int py)
{
    int avgCol = 0;
    int avgCnt = 0;
    const int dist = 3;
    //core::array<int> heights;
    
    for (int x = px-dist; x <= px+dist; x++)
    {
        for (int y = py-dist; y <= py+dist; y++)
        {
#ifdef USE_IMAGE_HM
            if (x >= 0 && y >= 0 && x < heightMap->getDimension().Width && y < heightMap->getDimension().Height
#else
            if (x >= 0 && y >= 0 && x < heightMap->getXSize() && y < heightMap->getYSize()
#endif
                && isRoad(x, y))
            {
                avgCnt++;
#ifdef USE_IMAGE_HM
                avgCol += heightMap->getPixel(x, y).getRed();
#else
                avgCol += heightMap->get(x, y);
#endif
                //int i = 0;
                //while (i < heights.size() && heightMap->getPixel(x, y).getRed() < heights[i]) i++;
                //heights.insert(heightMap->getPixel(x, y).getRed(), i);
            }
        }
    }
    //return heights.size()>0 ? heights[0] : 0;
    //return heights.size()>0 ? heights[heights.size()/2] : 0;
    return avgCnt ? (avgCol/avgCnt) : 0;
}

void BigTerrain::applyAverage2(int px, int py, int avgCol)
{
#ifdef USE_IMAGE_HM
    SColor col(0, avgCol, 0, 0);
    heightMap->setPixel(px, py, col);
#else
    heightMap->set(px, py, avgCol);
#endif
/*
    const int dist = 1;
    for (int x = px-dist; x <= px+dist; x++)
    {
        for (int y = py-dist; y <= py+dist; y++)
        {
            if (x >= 0 && y >= 0 && x < heightMap->getDimension().Width && y < heightMap->getDimension().Height
                && !isRoad(x, y) && !isNextToRoad(x, y))
            {
                heightMap->setPixel(x, y, col);
            }
        }
    }
*/
}

int BigTerrain::getAverage3(int px, int py, int dist)
{
#ifdef USE_IMAGE_HM
//    return heightMap->getPixel(px, py).getRed();
#else
//    return heightMap->get(px, py);
#endif

    int avgCol = 0;
    int avgCnt = 0;
  
    //core::array<int> heights;
    
    for (int x = px-dist; x <= px+dist; x++)
    {
        for (int y = py-dist; y <= py+dist; y++)
        {
#ifdef USE_IMAGE_HM
            if (x >= 0 && y >= 0 && x < heightMap->getDimension().Width && y < heightMap->getDimension().Height
#else
            if (x >= 0 && y >= 0 && x < heightMap->getXSize() && y < heightMap->getYSize()
#endif
                && isRoad(x, y))
            {
                avgCnt++;
#ifdef USE_IMAGE_HM
                avgCol += heightMap->getPixel(x, y).getRed();
#else
                avgCol += heightMap->get(x, y);
#endif
                //int i = 0;
                //while (i < heights.size() && heightMap->getPixel(x, y).getRed() < heights[i]) i++;
                //heights.insert(heightMap->getPixel(x, y).getRed(), i);
            }
        }
    }
    //return heights.size()>0 ? heights[0] : 0;
    //return heights.size()>0 ? heights[heights.size()/2] : 0;
    return avgCnt ? (avgCol/avgCnt) : 0;

}
/*
void BigTerrain::applyAverage3(int px, int py, int avgCol)
{
#ifdef USE_IMAGE_HM
    SColor col(0, avgCol, avgCol, avgCol);
    heightMap->setPixel(px, py, col);
#else
    heightMap->set(px, py, avgCol);
    heightMap->setRoad(px, py, false);
#endif

    const int dist = 1;
    for (int x = px-dist; x <= px+dist; x++)
    {
        for (int y = py-dist; y <= py+dist; y++)
        {
#ifdef USE_IMAGE_HM
            if (x >= 0 && y >= 0 && x < heightMap->getDimension().Width && y < heightMap->getDimension().Height
#else
            if (x >= 0 && y >= 0 && x < heightMap->getXSize() && y < heightMap->getYSize()
#endif
                && !isRoad(x, y) && !isNextToRoad(x, y))
            {
#ifdef USE_IMAGE_HM
                heightMap->setPixel(x, y, col);
#else
                heightMap->set(x, y, avgCol);
#endif
            }
        }
    }
}
*/
void BigTerrain::applyAverage3(int px, int py, int avgCol)
{
#ifdef USE_IMAGE_HM
    SColor col(0, avgCol, avgCol, avgCol);
    heightMap->setPixel(px, py, col);
#else
    heightMap->set(px, py, avgCol);
//    return;
//    heightMap->setRoad(px, py, false);
#endif


/*
    const int dist = 1;
    for (int x = px-dist; x <= px+dist; x++)
    {
        for (int y = py-dist; y <= py+dist; y++)
        {
#ifdef USE_IMAGE_HM
            if (x >= 0 && y >= 0 && x < heightMap->getDimension().Width && y < heightMap->getDimension().Height
#else
            if (x >= 0 && y >= 0 && x < heightMap->getXSize() && y < heightMap->getYSize()
#endif
                && !isRoad(x, y) && !isNextToRoad(x, y))
            {
#ifdef USE_IMAGE_HM
                heightMap->setPixel(x, y, col);
#else
                heightMap->set(x, y, 300+avgCol);
#endif
            }
        }
    }
*/
}

void BigTerrain::doCache()
{
    core::stringw str = L"";
    
    dprintf(printf("-----------\ndo cache\n------------\n");)
    
    //printf("update small maps\n");
    for (int x = 0; x < max_x;x++)
    {
        for (int y = 0; y < max_y;y++)
        {
            SmallTerrain* st = new SmallTerrain(
                       heightMap, densityMap, textureMap, textures, this->shadowMap,
                       smgr, driver, nWorld,
                       this->skydome,
                       x, y, max_x, max_y,
                       stHeightmapSize, tScale,                           
                       this->stageNum,
                       this,
                       objectReps, 0, roadList, m_terrainPool,
                       vscale, waterHeight);
            delete st;

            str = L"Do caching: x: ";
            str += x;
            str += L" / ";
            str += max_x;
            str += L", ";
            str += y;
            str += L" / ";
            str += max_y;
            MessageText::addText(str.c_str(), 1, true);
        }
    }
    dprintf(printf("-----------\ndo cache end\n------------\n");)
}

void BigTerrain::calculateAIPointTimes()
{
    float distance = 0.f;
    for (int i = 1; i < aiPoints.size(); i++)
    {
        vector2df pcp(aiPoints[i-1]->getPosition().X,aiPoints[i-1]->getPosition().Z);
        vector2df cp(aiPoints[i]->getPosition().X,aiPoints[i]->getPosition().Z);
        float diff = pcp.getDistanceFrom(cp);
        
        distance += diff;
        aiPoints[i]->setDistance(distance);
    }
    for (int i = 1; i < aiPoints.size(); i++)
    {
        aiPoints[i]->setTime((u32)((float)stageTime*(aiPoints[i]->getDistance()/distance)));
    }
    stageLength = distance;
}
