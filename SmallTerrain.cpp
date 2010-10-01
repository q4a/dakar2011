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

#include "CHeightmap.h"

#ifdef __linux__
#include "linux_includes.h"
#endif

#ifdef MY_DEBUG
#define dprintf(x) x
#else
#define dprintf(x)
#endif

static int write_error = 0;

SmallTerrain::SmallTerrain(
#ifdef USE_IMAGE_HM
                           video::IImage* heightMap,
#else
                           CHeightmap* heightMap,
#endif
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
                           core::array<CMyRoad*> &bigRoadList,
                           float p_vscale,
                           float p_waterHeight)
           : nWorld(pnWorld), terrain(0), body(0),
             visible(false), collision(0), stHeightmapSize(pstHeightmapSize), tScale(ptScale),
             objectWrappers(),
             wasCalculated(true),
             smgr(psmgr), driver(pdriver), x(px), y(py), max_x(pmax_x), max_y(pmax_y),
             m_bigTerrain(p_bigTerrain), objectWire(0), roadList(), oceanNode(0), partTexture(0),
             vscale(p_vscale), offsetObject(0), offsetObjectOcean(0)
{
    core::vector3df loc((float)x*SMALLTERRAIN_SIZE, 0.f, (float)y*SMALLTERRAIN_SIZE);
    core::vector3df terrainScale(TERRAIN_SCALE, vscale, TERRAIN_SCALE);	// scale
    
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
		false,
        x*SMALLTERRAIN_HEIGHTMAP_SIZE, // x offset
        y*SMALLTERRAIN_HEIGHTMAP_SIZE, // y offset
        SMALLTERRAIN_HEIGHTMAP_SIZE+1    // small size
	);

    offsetObject = new OffsetObject(terrain);

    if (useShaders)
        terrain->setMaterialFlag(video::EMF_LIGHTING, false);
    else    
        terrain->setMaterialFlag(video::EMF_LIGHTING, globalLight);

// remove me
//    terrain->setMaterialFlag(video::EMF_WIREFRAME, true);
// use this if shader wrap is working correctly
//    if (useCgShaders)
//    {
//        terrain->setMaterialFlag(video::EMF_TEXTURE_WRAP, true);
//    }
//    else
//    {
        terrain->scaleTexture(1.0f, (float)SMALLTERRAIN_HEIGHTMAP_SIZE /* 3.0f*/);
//    }

	for (int j = 0; j<MAX_LOD; j++)
	{
        terrain->overrideLODDistance(j, LOD_distance*TERRAIN_SCALE*(1 + j + (j/2)));
    }
    // calculate image part from the texture
    const int sizeX = SMALLTERRAIN_HEIGHTMAP_SIZE;
    const int sizeY = SMALLTERRAIN_HEIGHTMAP_SIZE;
    IImage* partImage = driver->createImage(textureMap->getColorFormat(),
                                            core::dimension2d<u32>(sizeX,sizeY));

    for (int i = 0; i < sizeX; i++)
        for (int j = 0; j<sizeY; j++)
        {
            SColor tmp_col = textureMap->getPixel((x*sizeX)+i, (y*sizeY)+j);
            partImage->setPixel(sizeX-1-i, j, tmp_col);
        }

    c8 textureMapPartName[256];
    sprintf(textureMapPartName, "textureMapPart_%d_%d", x, y);
    partTexture = driver->addTexture(textureMapPartName, partImage);
    terrain->setMaterialTexture(0, partTexture);
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

///////////////// BEGIN SMALL ////////////////////    
	// add terrain scene node
    // add a notification call back for when the car leave the world
    updateObjects(objectReps, obj_density);

    bool fixOcean = false;
#ifdef USE_IMAGE_HM
    if ((float)heightMap->getPixel(x*SMALLTERRAIN_HEIGHTMAP_SIZE, y*SMALLTERRAIN_HEIGHTMAP_SIZE).getAverage()*vscale > p_waterHeight &&
        (float)heightMap->getPixel(x*SMALLTERRAIN_HEIGHTMAP_SIZE, (y+1)*SMALLTERRAIN_HEIGHTMAP_SIZE).getAverage()*vscale > p_waterHeight &&
        (float)heightMap->getPixel((x+1)*SMALLTERRAIN_HEIGHTMAP_SIZE, y*SMALLTERRAIN_HEIGHTMAP_SIZE).getAverage()*vscale > p_waterHeight &&
        (float)heightMap->getPixel((x+1)*SMALLTERRAIN_HEIGHTMAP_SIZE, (y+1)*SMALLTERRAIN_HEIGHTMAP_SIZE).getAverage()*vscale > p_waterHeight
#else // USE_IMAGE_HM
    if ((float)heightMap->get(x*SMALLTERRAIN_HEIGHTMAP_SIZE, y*SMALLTERRAIN_HEIGHTMAP_SIZE)*vscale > p_waterHeight &&
        (float)heightMap->get(x*SMALLTERRAIN_HEIGHTMAP_SIZE, (y+1)*SMALLTERRAIN_HEIGHTMAP_SIZE)*vscale > p_waterHeight &&
        (float)heightMap->get((x+1)*SMALLTERRAIN_HEIGHTMAP_SIZE, y*SMALLTERRAIN_HEIGHTMAP_SIZE)*vscale > p_waterHeight &&
        (float)heightMap->get((x+1)*SMALLTERRAIN_HEIGHTMAP_SIZE, (y+1)*SMALLTERRAIN_HEIGHTMAP_SIZE)*vscale > p_waterHeight
#endif // USE_IMAGE_HM
       )
    {
        fixOcean = true;
    }
    activate(heightMap);
    updateRoads(bigRoadList, 0, shadowMap);

    // ocean
    oceanNode = loadMySimpleObject("data/bigterrains/ocean/ocean_surface.mso");
    if (oceanNode)
    {
        oceanNode->setPosition(vector3df(-0.25f, p_waterHeight, -0.25f) + terrain->getPosition());
        oceanNode->setScale(vector3df(SMALLTERRAIN_SIZE+0.5f, 1.0f, SMALLTERRAIN_SIZE+0.5f));
        if (useCgShaders)
        {
            oceanNode->setMaterialTexture(0, driver->getTexture("data/bigterrains/ocean/normal2.png"));
    
            if (skydome && skydome->getMaterial(0).getTexture(0))
                oceanNode->setMaterialTexture(1, skydome->getMaterial(0).getTexture(0));
        }
        else
        {
            oceanNode->setMaterialTexture(0, driver->getTexture("data/bigterrains/ocean/water.jpg"));
        }

        if (fixOcean /*|| true*/)
        {
            oceanNode->setMaterialType(/*video::EMT_REFLECTION_2_LAYER*/(video::E_MATERIAL_TYPE)myMaterialType_ocean_fix);
        }
        else
        {
            oceanNode->setMaterialType(/*video::EMT_REFLECTION_2_LAYER*/(video::E_MATERIAL_TYPE)myMaterialType_ocean);
        }

        offsetObjectOcean = new OffsetObject(oceanNode);
    }
    setActive(true);
}

SmallTerrain::~SmallTerrain()
{
    if (terrain)
    {
        //removeFromDepthNodes(terrain);
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
        NewtonWorldCriticalSectionLock(nWorld);
        NewtonReleaseCollision(nWorld, collision);
        NewtonWorldCriticalSectionUnlock(nWorld);
        collision = 0;
    }
    if (offsetObject)
    {
        offsetManager->removeObject(offsetObject);
        delete offsetObject;
        offsetObject = 0;
    }
    if (offsetObjectOcean)
    {
        offsetManager->removeObject(offsetObjectOcean);
        delete offsetObjectOcean;
        offsetObjectOcean = 0;
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
    
    if (partTexture)
    {
        //partTexture->drop();
        partTexture = 0;
    }

    for (int i = 0; i < roadList.size(); i++)
    {
        if (roadList[i])
        {
            delete roadList[i];
            roadList[i] = 0;
        }
    }
    roadList.clear();
}

#ifdef USE_IMAGE_HM
void SmallTerrain::activate(video::IImage* heightMap)
#else
void SmallTerrain::activate(CHeightmap* heightMap)
#endif
{
    if (collision) return;

#if 1
    unsigned short* elevationMap = new unsigned short[(SMALLTERRAIN_HEIGHTMAP_SIZE+1)*(SMALLTERRAIN_HEIGHTMAP_SIZE+1)];
    char* attributeMap = new char[(SMALLTERRAIN_HEIGHTMAP_SIZE+1)*(SMALLTERRAIN_HEIGHTMAP_SIZE+1)];
    
    memset(attributeMap, (char)levelID, sizeof(char)*(SMALLTERRAIN_HEIGHTMAP_SIZE+1)*(SMALLTERRAIN_HEIGHTMAP_SIZE+1));
    
    for (int i = 0; i < (SMALLTERRAIN_HEIGHTMAP_SIZE+1); i++)
        for (int j = 0; j < (SMALLTERRAIN_HEIGHTMAP_SIZE+1); j++)
        {
#ifdef USE_IMAGE_HM
            elevationMap[i + ((SMALLTERRAIN_HEIGHTMAP_SIZE+1)*j)] =
                    (unsigned short)heightMap->getPixel((SMALLTERRAIN_HEIGHTMAP_SIZE*x)+i,
                                                        (SMALLTERRAIN_HEIGHTMAP_SIZE*y)+j).getAverage();
#else // USE_IMAGE_HM
            elevationMap[i + ((SMALLTERRAIN_HEIGHTMAP_SIZE+1)*j)] =
                    (unsigned short)heightMap->get((SMALLTERRAIN_HEIGHTMAP_SIZE*x)+i,
                                                        (SMALLTERRAIN_HEIGHTMAP_SIZE*y)+j);
#endif // USE_IMAGE_HM
        }

    NewtonWorldCriticalSectionLock(nWorld);
    collision = NewtonCreateHeightFieldCollision(nWorld, SMALLTERRAIN_HEIGHTMAP_SIZE+1, SMALLTERRAIN_HEIGHTMAP_SIZE+1,
                    0, elevationMap, attributeMap, TERRAIN_SCALE, vscale, levelID);
    NewtonWorldCriticalSectionUnlock(nWorld);

    delete [] elevationMap;
    delete [] attributeMap;
#endif // 1
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
        
        if (offsetObject)
        {
            offsetObject->setBody(body);
            offsetManager->addObject(offsetObject);
        }
        if (offsetObjectOcean)
        {
            offsetManager->addObject(offsetObjectOcean);
        }

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
        if (offsetObject)
        {
            offsetObject->setBody(0);
            offsetManager->removeObject(offsetObject);
        }
        if (offsetObjectOcean)
        {
            offsetManager->removeObject(offsetObjectOcean);
        }
    }
    if (terrain)
    {
        terrain->setVisible(visible);
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

void SmallTerrain::updateRoads(core::array<CMyRoad*> &bigRoadList, unsigned int regenerate, video::ITexture* p_shadowMap, int roadToUpdate)
{
    dprintf(printf("ST::updateRoads() old size %d\n", roadList.size());)
    for (int i = 0; i < roadList.size(); i++)
    {
        if (roadList[i] && (roadToUpdate == -1 || roadList[i]->getParent() == roadToUpdate))
        {
            delete roadList[i];
            roadList[i] = 0;
        }
    }
    if (roadToUpdate == -1)
    {
        roadList.clear();
    }

    core::vector2df down((float)x*SMALLTERRAIN_SIZE, (float)y*SMALLTERRAIN_SIZE);
    core::vector2df up((float)(x+1)*SMALLTERRAIN_SIZE, (float)(y+1)*SMALLTERRAIN_SIZE);

    dprintf(printf("ST::updateRoads() BT roads size %d\n", bigRoadList.size());)
    for (int i = 0; i < bigRoadList.size(); i++)
    {
        if (roadToUpdate!=-1 && roadToUpdate!=i) continue;
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
                        road->setType(bigRoadList[i]->getType());
                        road->setParent(i);
                        /*
                        road->setSlicePoints(bigRoadList[i]->getSlicePoints());
                        road->setSliceIndices(bigRoadList[i]->getSliceIndices());
                        road->setTexture(bigRoadList[i]->getTexture());
                        road->setTextureName(bigRoadList[i]->getTextureName());
                        */
                        if (j > 0)
                        {
                            float height = 0.0f;
                            /*
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
                            */
                            //printf("height: %f\n", height);
                            //road->addBasePoint(vector3df(basePoints[j-1].X,height,basePoints[j-1].Z));
                            //printf("add bp 1, new size: %d\n", road->getBasePoints().size());
                        }
                    }
                    if (j==0 || j==basePoints.size()-1)
                    {
                        //road->addBasePoint(vector3df(basePoints[j].X,terrain->getHeight(basePoints[j].X, basePoints[j].Z)-0.6f,basePoints[j].Z));
                        road->addBasePoint(vector3df(basePoints[j].X, -0.6f,basePoints[j].Z), 0, false, false, false);
                        //printf("add bp 2, new size: %d\n", road->getBasePoints().size());
                    }
                    else
                    {
                        //road->addBasePoint(vector3df(basePoints[j].X,terrain->getHeight(basePoints[j].X, basePoints[j].Z),basePoints[j].Z));
                        road->addBasePoint(vector3df(basePoints[j].X, 0.f,basePoints[j].Z), 0, false, false, false);
                        //printf("add bp 3, new size: %d\n", road->getBasePoints().size());
                    }
                }
                else
                {
                    if (road)
                    {
                        if (j < basePoints.size() - 1)
                        {
                            road->addBasePoint(vector3df(basePoints[j+1].X,0.f,basePoints[j+1].Z), 0, false, false, false);
                        }
                        dprintf(printf("ST::updateRoads() end road found BT road num %d basePoint %d, new road size %d\n", i, j, road->getBasePoints().size());)
                        //road->addBasePoint(vector3df(basePoints[j].X,terrain->getHeight(basePoints[j-1].X, basePoints[j-1].Z),basePoints[j].Z));
                        road->generateRoadNode(this, regenerate, p_shadowMap);
                        roadList.push_back(road);
                        road = 0;
                    }
                }
            }
            if (road)
            {
                dprintf(printf("ST::updateRoads() end road found BT 2 road num %d, new road size %d\n", i, road->getBasePoints().size());)
                road->generateRoadNode(this, regenerate, p_shadowMap);
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

void SmallTerrain::updateObjects(core::array<SObjectPoolIdRepPair> &objectReps, int obj_density)
{
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
    
    objectWire = new CObjectWire(SMALLTERRAIN_SIZE, SMALLTERRAIN_SIZE, (float)x*SMALLTERRAIN_SIZE, (float)y*SMALLTERRAIN_SIZE);
    vector3df maxs;
    int addSuccCnt = 0;
    int addFailCnt = 0;
    for (int i = 0; i < objectReps.size(); i++)
    {
        for (int j = 0; j < (objectReps[i].rep*obj_density)/100;j++)
        {
            vector3df pos;
            float val = ((float)(rand()%32768))/32768.f;
            pos = vector3df((float)x*SMALLTERRAIN_SIZE+(((float)(rand()%32768))/32768.f)*SMALLTERRAIN_SIZE,
                            0.0f,
                            (float)y*SMALLTERRAIN_SIZE+(((float)(rand()%32768))/32768.f)*SMALLTERRAIN_SIZE);
            float tval = m_bigTerrain->getDensity(pos.X, pos.Z, getPoolCategoryFromId(objectReps[i].poolId));
            if (tval > 0.05f && val < tval)

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
}
