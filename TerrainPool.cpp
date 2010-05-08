
#include "TerrainPool.h"
#include "settings.h"

TerrainPool::TerrainPool(int numOfTerrains, ISceneManager* smgr, IVideoDriver* driver)
    : terrains()
{
    core::vector3df terrainScale_t(TERRAIN_SCALE_T, VSCALE, TERRAIN_SCALE_T);	// scale
    //core::vector3df terrainScale_t((20.f/(float)terrain_tesselation), 1.5f, (20.f/(float)terrain_tesselation));	// scale
    IImage* tmpHM = driver->createImage(ECF_R8G8B8, core::dimension2d<u32>(SMALLTERRAIN_HEIGHTMAP_SIZE_T+1, SMALLTERRAIN_HEIGHTMAP_SIZE_T+1));
    for (int i = 0; i < numOfTerrains; i++)
    {
        scene::ITerrainSceneNode* terrain = smgr->addTerrainSceneNode(
    		tmpHM,
    		0,					// parent node
    		-1,					// node id
    		core::vector3df(0.f, 0.f, 0.f),		// position
    		core::vector3df(0.f, 0.f, 0.f),		// rotation
    		terrainScale_t,	// scale
    		video::SColor ( 255, 255, 255, 255 ),	// vertexColor
    		MAX_LOD,					// maxLOD
    		scene::ETPS_17,				// patchSize
    		TESSELATION-1,					// smoothFactor
    		false
		);
		if (terrain)
		{
            terrain->setVisible(false);
            terrain->scaleTexture(1.0f, (float)SMALLTERRAIN_HEIGHTMAP_SIZE /* 3.0f*/);
    	
        	for (int j = 0; j<MAX_LOD; j++)
        	{
                terrain->overrideLODDistance(j, LOD_distance*TERRAIN_SCALE_T*(1 + j + (j/2)));
            }
            
            terrains.push_back(terrain);
        }
    }
    tmpHM->drop();
}

TerrainPool::~TerrainPool()
{
}

scene::ITerrainSceneNode* TerrainPool::getTerrain()
{
    return terrains.removeFirst();
}

void TerrainPool::putTerrain(scene::ITerrainSceneNode* terrain)
{
    terrain->setVisible(false);
    terrains.push_back(terrain);
}
