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
                       int p_stageNum)
           : nWorld(p_nWorld), map(0), max_x(0), max_y(0), last_x(-1), last_y(-1),
             ov_last_x(-1), ov_last_y(-1),
             stHeightmapSize(0), tScale(0.f),
             objectWrappers(),
             ov_limit(objectVisibilityLimit),
             startGate(0), startOffsetObject(0), endGate(0), endOffsetObject(0),
             device(p_device), smgr(p_smgr), driver(p_driver),
             timeStarted(false), timeEnded(false), lastTick(0), currentTime(0),
             cps(0), penality(0),
             stageTime(pstageTime), stageLength(0.f), gtime(pgtime),
             friction_multi(0.8f),
             densityMap(0), objectWire(0),
             heightMap(0), textureMap(0), shadowMap(p_shadowMap),
             cpPos(), cpTime(), cpTimed(), cpGate(), cpOffsetObject(),
             objectReps(), stageNum(p_stageNum), smallTerrainsForUpdate(), smallTerrainsForUpdateLock(), mapLock(),
             roadList(), activeItinerPoints(), aiPoints(), speed(60.0f), skydome(p_skydome),
             vscale(VSCALE), waterHeight(WATER_HEIGHT),
             lastMapsQueueUpdate(0), mapsQueueVersion(0),
             mapScaleX(0.f), mapScaleY(0.f), mapTexture(0)
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
    c8 mapTextureName[256];
    
    c8 dummyStr[256];
    float f1, f2;

    c8 roadfileName[256];
    c8 objectfileName[256];
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
    MessageText::addText(str.c_str(), 1, true, false);
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
    MessageText::addText(str.c_str(), 1, true, false);
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
    MessageText::addText(str.c_str(), 1, true, false);
    
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
    MessageText::addText(str.c_str(), 1, true, false);
    
    if (!useCgShaders)	
    {
        char baseName[256];
        char extName[256];

        strcpy(baseName, textureMapName);
        baseName[strlen(textureMapName)-4] = '\0';
        strcpy(extName, textureMapName + (strlen(textureMapName)-4));
        dprintf(printf("terrain texture file name base: %s, ext: %s\n", baseName, extName);)
        strcpy(textureMapName, baseName); strcat(textureMapName, "_ns"); strcat(textureMapName, extName);
    }
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
    MessageText::addText(str.c_str(), 1, true, false);
    
        
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

        if (useCgShaders)
        {
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
        else
        {
            if (i == 0)
                textures[i] = driver->getTexture("data/bigterrains/textures/detailmap.jpg");
            else
                textures[i] = 0;
        }
    }

    // read skydome texture
    ret = fscanf(f, "%s\n%s\n%s\n",
                 skyDomeFileName,
                 roadfileName, objectfileName);
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
            break;
        }
        else
        {
            // cp_pos
            cpPos.push_back(vector3df(f1, 0.0, f2));
        }
    }

    ret = fscanf(f, "map_up: %d %d\nmap_down: %d %d\nmap_texture: %s\n", &mapUp.X, &mapUp.Y, &mapDown.X, &mapDown.Y, mapTextureName);
    if ( ret < 5 )
    {
        printf("error reading %s ret %d errno %d\n", name, ret, errno);
        fclose(f);
        return;
    }
    mapSize = mapDown - mapUp;
#ifdef USE_IMAGE_HM
    mapScaleX = (float)mapSize.Width/(float)getHeightMap()->getDimension().Width;
    mapScaleY = (float)mapSize.Height/(float)getHeightMap()->getDimension().Height;
#else
    mapScaleX = (float)mapSize.Width/(float)getHeightMap()->getXSize();
    mapScaleY = (float)mapSize.Height/(float)getHeightMap()->getYSize();
#endif
    dprintf(printf("mapUp: %d, %d, mapDown: %d, %d, mapScale: %f, %f, mapTextureName: %s\n", mapUp.X, mapUp.Y, mapDown.X, mapDown.Y, mapScaleX, mapScaleY, mapTextureName);)
    mapTexture = driver->getTexture(mapTextureName);
    //assert(0);

    fclose(f);
    
    objectWire = new CObjectWire((float)max_x*SMALLTERRAIN_SIZE, (float)max_y*SMALLTERRAIN_SIZE);

    str = L"Loading: 40%";
    MessageText::addText(str.c_str(), 1, true, false);

    //driver->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS, false);
    CMyRoad::loadRoads(roadfileName, roadList, smgr, driver, nWorld, this);
    //driver->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS, true);
    
    str = L"Loading: 45%";
    MessageText::addText(str.c_str(), 1, true, false);

    if (!use_mipmaps)
	    driver->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS, false);
    loadObjects(objectfileName, smgr, driver);
    if (!use_mipmaps)
    	driver->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS, true);

    calculateAIPointTimes();    

    str = L"Loading: 48%";
    MessageText::addText(str.c_str(), 1, true, false);

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
    boxP0[0] = -1.1f*SMALLTERRAIN_SIZE;
    boxP0[1] = -100.f;
    boxP0[2] = -1.1f*SMALLTERRAIN_SIZE;
    boxP1[0] = 2.1f*SMALLTERRAIN_SIZE;
    boxP1[1] = 7000.f;
    boxP1[2] = 2.1f*SMALLTERRAIN_SIZE;
    dprintf(printf("nWorld bbox: %f %f %f\n", boxP1[0], boxP1[1], boxP1[2]);)
    
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
           objectReps, 0, roadList);
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
    destroyObjects();
    
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
#ifdef USE_MESH_COMBINER
        dprintf(printf("b 4b %p\n", objectWire);)
        resetCombinedObjects();
#endif
        dprintf(printf("b 5 %p\n", objectWire);)
        objectWire->updatePos(newX, newY, OV_LIMIT, force);
        dprintf(printf("b 6\n"));
        updateSmallTerrainsObjects(newX, newY, force);
#ifdef USE_MESH_COMBINER
        dprintf(printf("b 6b\n"));
        finishCombinedObjects(smgr);
#endif
        //if (startTime==0)
        dprintf(printf("b 7\n"));
        if (!timeStarted)
        {
            if (fabsf(newX-startPos.X)<OV_LIMIT && fabsf(newY-startPos.Z)<OV_LIMIT)
            {
                vector3df pos = startGate->getPosition();
                pos.Y = getHeight(startPos.X, startPos.Z);
                startOffsetObject->getPos().Y = pos.Y;
                //pos.Y = getHeight(pos.X+offsetManager->getOffset().X, pos.Z+offsetManager->getOffset().Z);
                //assert(pos.Y > 1.0f);
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
                    cpOffsetObject[i]->getPos().Y = pos.Y;
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
                endOffsetObject->getPos().Y = pos.Y;
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
            str += oldStage+1;
            /*
            str += stages[oldStage]->stageNum;
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


                    if (cps>0)
                    {
                        str += cps;
                        str += L" checkpoint(s) remaining.\n";
                    } else {
                        str += L"Only the end point remaining.\n";
                    }

                    if (currentTime+penality>0)
                    {
                        cpTimed[i] = currentTime+penality;//device->getTimer()->getTime();
                    }
                    else
                    {
                        cpTimed[i] = 1;
                    }
                    
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
            core::stringw str = L"";
            
            if (oldStage+1 >= MAX_STAGES || stages[oldStage+1] == 0)
                str += L"Congratulation!!! You have successfully completed the Dakar 2011.\n\n\n";

            str += L"You have reached the endpoint of stage ";
            str += oldStage+1;

            if (cps>0)
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
            u32 diffTime = currentTime; //endTime - startTime;
            endGate->setVisible(false);
            offsetManager->removeObject(endOffsetObject);

            str += L"Your time is ";
            addTimeToStr(str, diffTime);
            if (penality>0)
                str += L" (including the penalities)";

            u32 position = 1;
            // new stuff
            playerCompetitor->lastTime = diffTime;
            playerCompetitor->globalTime += diffTime;
            //playerCompetitor->finishTime = diffTime - penality;
            if (raceEngine)
                position = raceEngine->insertIntoFinishedState(playerCompetitor);

            str += L". Your current position is ";
            str += position;

            str += L"\n\nYou can drive further or check the positions in the Standings.";
            
            if (oldStage+1 >= MAX_STAGES || stages[oldStage+1] == 0)
                MessageText::addText(str.c_str(), 40);
            else
                MessageText::addText(str.c_str(), 10);
            
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

void BigTerrain::zeroDensity(float x, float y)
{
    int _x = (int)(x / TERRAIN_SCALE);
    int _y = (int)(y / TERRAIN_SCALE);
    if (!densityMap || _x < 0 || _x >= densityMap->getDimension().Width ||
                       _y < 0 || _y >= densityMap->getDimension().Height)
    {
        printf("unable to zero DM\n");
        return;
    }

    for (int __x = _x - 1; __x <= _x + 1; __x++)
        for (int __y = _y - 1; __y <= _y + 1; __y++)
        {
            if (__x >= 0 && __x < densityMap->getDimension().Width &&
                __y >= 0 && __y < densityMap->getDimension().Height)
            {
                densityMap->setPixel(__x, __y, SColor(255, 0, 0, 0));
            }
        }
}

void BigTerrain::setRoadOnTextureMap(float x, float y)
{
    int _x = (int)(x / TERRAIN_SCALE);
    int _y = (int)(y / TERRAIN_SCALE);
    if (!textureMap || _x < 0 || _x >= textureMap->getDimension().Width ||
                       _y < 0 || _y >= textureMap->getDimension().Height)
    {
        printf("unable to set road on TM\n");
        return;
    }

    for (int __x = _x - 1; __x <= _x + 1; __x++)
        for (int __y = _y - 1; __y <= _y + 1; __y++)
        {
            if (__x >= 0 && __x < textureMap->getDimension().Width &&
                __y >= 0 && __y < textureMap->getDimension().Height)
            {
                textureMap->setPixel(__x, __y, SColor(255, 255, 0, 0));
            }
        }
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
            if (poolId < 0) continue;
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
            // fix positioned objects: itinerary
            ret = fscanf(f, "%f %f", &pos.X, &pos.Z);
            if (ret < 2)
            {
                // no more object
                printf("objects file unexpectedly ended: %s\n", name);
                break;
            }
            poolId = getItinerIdFromName(meshName);
            if (poolId < 0) continue;
            SItinerPoint* objectWrapper = new SItinerPoint(this);
            objectWrapper->setPosition(pos);
            objectWrapper->setPool(poolId);
            objectWrappers.push_back(objectWrapper);
            objectWire->addObject(pos, objectWrapper);
        }
        else
        {
            // frandom positioned objects
            ret = fscanf(f, "%d", &repeat);
            if (ret < 1)
            {
                // no more object
                printf("objects file unexpectedly ended: %s\n", name);
                break;
            }
            // random positioned objects
            poolId = getPoolIdFromName(meshName);
            if (poolId < 0) continue;
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
    if (diffTime/3600 > 0)
    {
        str += diffTime/3600; // hours
        str += ":";
    }
    if ((diffTime/60)%60==0)
        str += "00";
    else
        {
            if ((diffTime/60)%60<10)
                str += "0";
            str += (diffTime/60)%60; // minutes
        }
    str += ":";
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
                           objectReps, obj_density, roadList,
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
                    MessageText::addText(str.c_str(), 1, true, false);
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
                       objectReps, obj_density, roadList,
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

void BigTerrain::updateRoads(int roadToUpdate)
{
    //printf("update small maps\n");
    for (int x = 0; x < max_x;x++)
    {
        for (int y = 0; y < max_y;y++)
        {
            if (map[x + (max_x*y)] != 0)
            {
                map[x + (max_x*y)]->updateRoads(roadList, 1, shadowMap, roadToUpdate);
            }
        }
    }
}

void BigTerrain::addNewRoad(unsigned int type)
{
    if (roadList.size() == 0) return;
    
    CMyRoad* road = new CMyRoad(smgr, driver, nWorld);
    road->setType(type);
    /*
    road->setSlicePoints(roadList[roadList.size()-1]->getSlicePoints());
    road->setSliceIndices(roadList[roadList.size()-1]->getSliceIndices());
    road->setTexture(roadList[roadList.size()-1]->getTexture());
    road->setTextureName(roadList[roadList.size()-1]->getTextureName());
    */
    roadList.push_back(road);
}

void BigTerrain::loadRoads(const char* roadsName)
{
    CMyRoad::loadRoads(roadsName, roadList, smgr, driver, nWorld, this);
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
            MessageText::addText(str.c_str(), 1, true, false);
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
            MessageText::addText(str.c_str(), 1, true, false);
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
            MessageText::addText(str.c_str(), 1, true, false);
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
            MessageText::addText(str.c_str(), 1, true, false);
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
                       objectReps, 0, roadList,
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
            MessageText::addText(str.c_str(), 1, true, false);
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

void BigTerrain::saveHeightMap(const char* hmname, const char *pngname)
{
    const unsigned int sizeX = heightMap->getXSize();
    const unsigned int sizeY = heightMap->getYSize();
	CHeightmap* inv_heightMap = smgr->addHeightmap(sizeX, sizeY);
    dprintf(printf("write heightmap as bin %s\n", hmname);)

    for (int i = 0; i < sizeX; i++)
    {
        for (int j = 0; j<sizeY; j++)
        {
            inv_heightMap->set(i, j, heightMap->get(i, sizeY-1-j));
            inv_heightMap->setRoad(i, j, heightMap->getRoad(i, sizeY-1-j));
        }
        if (i%100==0)
        {
            MessageText::refresh();
        }
    }
    dprintf(printf("invert heightmap as bin %s done\nstart write\n", hmname);)

    bool ret = inv_heightMap->writeBin(hmname);
    if (!ret)
    {
        printf("unable to write binary heightmap: %s", hmname);
    }
    dprintf(printf("write heightmap as bin %s done\nstart write as PNG %s\n", hmname, pngname);)
    ret = inv_heightMap->writeToPNG(pngname, device, driver);
    dprintf(printf("write heightmap as PNG %s done\n", pngname);)
    
    delete inv_heightMap;
}

void BigTerrain::saveTextureMap(const char* tmname)
{
    unsigned int sizeX = textureMap->getDimension().Width;
    unsigned int sizeY = textureMap->getDimension().Height;

    irr::video::IImage* inv_textureMap = driver->createImage(irr::video::ECF_R8G8B8, irr::core::dimension2d<irr::u32>(sizeX, sizeY));
    irr::video::IImageWriter* writer = 0;
    irr::io::path path = tmname;
    
    for (unsigned int i = 0; i < driver->getImageWriterCount(); i++)
    {
        irr::video::IImageWriter* tmpwriter = driver->getImageWriter(i);
        if (tmpwriter->isAWriteableFileExtension(path))
        {
            writer = tmpwriter;
            break;
        }
    }
    
    if (writer == 0)
    {
        printf("There is no writer for [%s]\n", tmname);
        return;
    }

    dprintf(printf("calculate TM %s\n", tmname);)
    for (int i = 0; i < sizeX; i++)
    {
        for (int j = 0; j<sizeY; j++)
        {
            inv_textureMap->setPixel(i, j, textureMap->getPixel(i, sizeY-1-j));
        }
        if (i%100==0)
        {
            MessageText::refresh();
        }
    }
    dprintf(printf("start write TM %s\n", tmname);)

    bool ret = driver->writeImageToFile(inv_textureMap, device->getFileSystem()->createAndWriteFile(path));
    dprintf(printf("start write TM %s done\n", tmname);)
    inv_textureMap->drop();
    return;
}

void BigTerrain::setRoadOnHeightMap(float x, float y)
{
    const unsigned int sizeX = heightMap->getXSize();
    const unsigned int sizeY = heightMap->getYSize();
    int _x = (int)(x / TERRAIN_SCALE);
    int _y = (int)(y / TERRAIN_SCALE);
    if (!textureMap || _x < 0 || _x >= sizeX ||
                       _y < 0 || _y >= sizeY)
    {
        printf("unable to set road on HM\n");
        return;
    }
    heightMap->setRoad(_x, _y, true);
}

void BigTerrain::updateSmallTerrainsObjects(float newX, float newY, bool force)
{
    dprintf(printf("su 1\n"));
    smallTerrainsForUpdateLock.lock();
    dprintf(printf("su 2\n"));
    for (int i = 0; i < smallTerrainsForUpdate.size(); i++)
    {
        assert(smallTerrainsForUpdate[i]);
        dprintf(printf("su 3 %p\n", smallTerrainsForUpdate[i]));
        smallTerrainsForUpdate[i]->updatePos(newX, newY, OV_LIMIT, force);
        dprintf(printf("su 4 %p\n", smallTerrainsForUpdate[i]));
    }
    dprintf(printf("su 5\n"));
    smallTerrainsForUpdateLock.unlock();
    dprintf(printf("su 6\n"));
}

void BigTerrain::updateObjectVisibilityLimit(int obj_density)
{
    dprintf(printf("ovl 1\n"));
    smallTerrainsForUpdateLock.lock();
    dprintf(printf("ovl 2\n"));
    for (int i = 0; i < smallTerrainsForUpdate.size(); i++)
    {
        assert(smallTerrainsForUpdate[i]);
        dprintf(printf("ovl 3 %p\n", smallTerrainsForUpdate[i]));
        smallTerrainsForUpdate[i]->updateObjects(objectReps, obj_density);
        dprintf(printf("ovl 4 %p\n", smallTerrainsForUpdate[i]));
        smallTerrainsForUpdate[i]->updatePos((float)ov_last_x * TILE_SIZE, (float)ov_last_y * TILE_SIZE, OV_LIMIT, true);
        dprintf(printf("ovl 5 %p\n", smallTerrainsForUpdate[i]));
    }
    dprintf(printf("ovl 6\n"));
    smallTerrainsForUpdateLock.unlock();
    dprintf(printf("ovl 7\n"));
}
