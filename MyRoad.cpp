/****************************************************************
*                                                               *
*    Name: MyRoad.cpp                                           *
*                                                               *
*    Creator: Balazs Tuska                                      *
*                                                               *
*    Description:                                               *
*                                                               *
****************************************************************/

#include "MyRoad.h"
#include "settings.h"
#include "my_shaders.h"
#include "Materials.h"
#include <assert.h>
#include "BigTerrain.h"
#include "SmallTerrain.h"
#include "gameplay.h"
#include "message.h"

#include "linux_includes.h"

#ifdef MY_DEBUG
# define dprintf(x) x
#else
# define dprintf(x)
#endif /* debug */

#ifdef MY_PDEBUG
# define pdprintf(x) x
#else
# define pdprintf(x)
#endif /* debug */

#define NEW_ROAD_OFFSET 1

core::array<CRoadType*> CRoadType::roadTypes;

CRoadType::CRoadType(IVideoDriver* p_driver)
    : driver(p_driver),
      slicePoints(),
      sliceIndices(),
      texture(0),
      friction_multi(0.9f),
      tRate(0.f)
{
}

CRoadType::~CRoadType()
{
    slicePoints.clear();
    sliceIndices.clear();
}
/*
void CRoadType::setTextureName(const char* newTextureName)
{
    strcpy(textureName, newTextureName);
}
*/
bool CRoadType::load(FILE* f)
{
    char textureName[256];
    float f1, f2;
    unsigned int num;
    int ret = 0;
    int ind;

    slicePoints.clear();
    sliceIndices.clear();
    tRate = 0.f;
    memset(textureName, 0, sizeof(textureName));

    ret = fscanf(f, "texture: %s\n", textureName);
    if (ret < 1)
    {
        printf("error reading texture name from road type file\n");
        return false;
    }
    driver->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS, false);
    texture = driver->getTexture(textureName);
    driver->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS, true);
    
    ret = fscanf(f, "friction_multi: %f\n", &friction_multi);
    if (ret < 1)
    {
        printf("error reading firction multiplier from road type file\n");
        return false;
    }
    
    ret = fscanf(f, "num_of_slice_points: %u\n", &num);
    if (ret < 1)
    {
        printf("error reading num of slice points from road type file\n");
        return false;
    }

    for (int i = 0; i < num; i++)
    {
        ret = fscanf(f, "%f %f\n", &f1, &f2);
        if (ret < 2 )
        {
            printf("error reading %d. slice point from road type file\n", i);
            return false;
        }
        slicePoints.push_back(vector2df(f1, f2));
        if (0 < i) tRate += (slicePoints[i] - slicePoints[i-1]).getLength();
    }
    
    ret = fscanf(f, "num_of_slice_indices: %u\n", &num);
    if (ret < 1)
    {
        printf("error reading num of slice indices from road type file\n");
        return false;
    }
    
    for (int i = 0; i < num; i++)
    {
        ret = fscanf(f, "%d\n", &ind);
        if (ret < 1 )
        {
            printf("error reading %d. slice index from road type file\n", i);
            return false;
        }
        sliceIndices.push_back(ind);
    }
    return true;
}
/*static*/
bool CRoadType::loadRoadTypes(const char* name, IVideoDriver* p_driver)
{
    FILE* f;
    int ret = 0;
    int numOfRoads = 0;

    dprintf(printf("Read road types: %s\n", name));

    f = fopen(name, "r");

    if (!f)
    {
        printf("road type file unable to open: %s\n", name);
        return false;
    }
    
    while (true)
    {
        CRoadType* roadType = new CRoadType(p_driver);
        if (roadType->load(f))
        {
            roadTypes.push_back(roadType);
            // road->generateRoadNode();
        }
        else
        {
            printf("error reading road type from file: %s \n", name);
            delete roadType;
            fclose(f);
            return false;
        }
        MessageText::refresh();
    }
    
    fclose(f);
    
    return true;
}


CMyRoad::CMyRoad(ISceneManager* p_smgr, IVideoDriver* p_driver, NewtonWorld *p_nWorld) : 
    basePoints(),
    roadNode(0),
    newtonBody(0),
    collision(0),
    smgr(p_smgr),
    driver(p_driver),
    nWorld(p_nWorld),
    offsetObject(0),
    type(0),
    roadType(0),
    parent(-1)
{
}

CMyRoad::~CMyRoad()
{
    if (roadNode)
    {
        //roadNode->drop();
        roadNode->remove();
    }
    basePoints.clear();
    if (newtonBody)
    {
        NewtonBodySetUserData(newtonBody, 0);
        NewtonDestroyBody(nWorld, newtonBody);
    }
    newtonBody = 0;
    if (collision)
    {
        NewtonWorldCriticalSectionLock(nWorld);
        NewtonReleaseCollision(nWorld, collision);
        NewtonWorldCriticalSectionUnlock(nWorld);
    }
    collision = 0;
    if (offsetObject)
    {
        offsetObject->setNode(0);
        offsetObject->setBody(0);
        offsetManager->removeObject(offsetObject);
        delete offsetObject;
        offsetObject = 0;
    }
    type = 0;
    roadType = 0;
}

void CMyRoad::addBasePoint(const core::vector3df& newPoint, BigTerrain* p_bigTerrain,
                           bool addToDensityMap, bool addToRoadMap, bool addToTextureMap)
{
    //assert(0);
    if (basePoints.size() > 0)
    {
        vector3df bp = basePoints[basePoints.size() - 1];
        vector3df dir = newPoint - bp;
        float dist = dir.getLength();
        float cur = 0.f;
        while (cur + 8.f < dist)
        {
            cur += 5.f;
            vector3df ap = bp + dir*(cur/dist);
            basePoints.push_back(ap);
            if (p_bigTerrain)
            {
                if (addToDensityMap)
                {
                    p_bigTerrain->zeroDensity(ap.X, ap.Z);
                }
                if (addToRoadMap)
                {
                    p_bigTerrain->setRoadOnHeightMap(ap.X, ap.Z);
                }
                if (addToTextureMap)
                {
                    p_bigTerrain->setRoadOnTextureMap(ap.X, ap.Z);
                }
            }
        }
    }
    basePoints.push_back(newPoint);
    //printf("add newPoint\n");
    if (p_bigTerrain)
    {
        if (addToDensityMap)
        {
            p_bigTerrain->zeroDensity(newPoint.X, newPoint.Z);
        }
        if (addToRoadMap)
        {
            p_bigTerrain->setRoadOnHeightMap(newPoint.X, newPoint.Z);
        }
        if (addToTextureMap)
        {
            p_bigTerrain->setRoadOnTextureMap(newPoint.X, newPoint.Z);
        }
    }
}

void CMyRoad::setBasePoints(const core::array<core::vector3df> &newBasePoints)
{
    basePoints = newBasePoints;
}
/*
void CMyRoad::setSlicePoints(const core::array<core::vector2df> &newSlicePoints)
{
    slicePoints = newSlicePoints;
    tRate = 0.f;
    for (int j = 0; j < slicePoints.size() - 1; j++)
        tRate += (slicePoints[j+1] - slicePoints[j]).getLength();
}

void CMyRoad::setSliceIndices(const core::array<int> &newSliceIndices)
{
    sliceIndices = newSliceIndices;
}
*/

void CMyRoad::setType(unsigned int newType)
{
    if (newType >= CRoadType::roadTypes.size()) return;
    type = newType;
    roadType = CRoadType::roadTypes[newType];
}

ISceneNode* CMyRoad::generateRoadNode(SmallTerrain* p_smallTerrain, unsigned int regenerate, video::ITexture* p_shadowMap)
{
    if (roadNode)
    {
        roadNode->drop();
    }
    roadNode = 0;

    if (newtonBody)
    {
        NewtonDestroyBody(nWorld, newtonBody);
    }
    newtonBody = 0;
    if (collision)
    {
        NewtonWorldCriticalSectionLock(nWorld);
        NewtonReleaseCollision(nWorld, collision);
        NewtonWorldCriticalSectionUnlock(nWorld);
    }
    collision = 0;

    if (offsetObject)
    {
        offsetObject->setBody(0);
        offsetObject->setNode(0);
        offsetManager->removeObject(offsetObject);
    }
    
    if (basePoints.size() < 2 || !roadType || roadType->slicePoints.size() < 2 || roadType->sliceIndices.size()%3 != 0)
    {
        return roadNode;
    }
    
    SMeshBuffer* buffer = new SMeshBuffer();
    SMesh* mesh = new SMesh();

    float prevHeight = 0.f;
    float prevPrevHeight = 0.f;

#ifdef NEW_ROAD_OFFSET
    regenerate = 1 - regenerate;
    for (int i = 0; i < basePoints.size(); i++)
    {
        basePoints[i].X -= (offsetManager->getOffset().X/**(float)regenerate*/);
        basePoints[i].Z -= (offsetManager->getOffset().Z/**(float)regenerate*/);
    }
#endif

    float ty = 0.f;
    int vertexCount = 0;
    for (int i = 0; i < basePoints.size(); i++)
    {
        vector2df normal;
        if (i == 0)
        {
            normal = vector2df(basePoints[i+1].X - basePoints[i].X, basePoints[i+1].Z - basePoints[i].Z);
        }
        else if (i == basePoints.size()-1)
        {
            normal = vector2df(basePoints[i].X - basePoints[i-1].X, basePoints[i].Z - basePoints[i-1].Z);
        }
        else
        {
            normal = vector2df(basePoints[i+1].X - basePoints[i-1].X, basePoints[i+1].Z - basePoints[i-1].Z);
        }
        normal.normalize();
        normal.rotateBy(-90.f);
        
        float addHeight = 0.f;
#ifndef NEW_ROAD_OFFSET
        float h0 = p_smallTerrain->terrain->getHeight((roadType->slicePoints[0].X*normal.X)+basePoints[i].X-(offsetManager->getOffset().X*(float)regenerate), (roadType->slicePoints[0].X*normal.Y)+basePoints[i].Z-(offsetManager->getOffset().Z*(float)regenerate));
        float hb = p_smallTerrain->terrain->getHeight(basePoints[i].X-(offsetManager->getOffset().X*(float)regenerate), basePoints[i].Z-(offsetManager->getOffset().Z*(float)regenerate));
        float hl = p_smallTerrain->terrain->getHeight((roadType->slicePoints[roadType->slicePoints.size()-1].X*normal.X)+basePoints[i].X-(offsetManager->getOffset().X*(float)regenerate), (roadType->slicePoints[roadType->slicePoints.size()-1].X*normal.Y)+basePoints[i].Z-(offsetManager->getOffset().Z*(float)regenerate));
#else
        float h0 = p_smallTerrain->terrain->getHeight((roadType->slicePoints[0].X*normal.X)+basePoints[i].X+(offsetManager->getOffset().X*(float)regenerate), (roadType->slicePoints[0].X*normal.Y)+basePoints[i].Z+(offsetManager->getOffset().Z*(float)regenerate));
        float hb = p_smallTerrain->terrain->getHeight(basePoints[i].X+(offsetManager->getOffset().X*(float)regenerate), basePoints[i].Z+(offsetManager->getOffset().Z*(float)regenerate));
        float hl = p_smallTerrain->terrain->getHeight((roadType->slicePoints[roadType->slicePoints.size()-1].X*normal.X)+basePoints[i].X+(offsetManager->getOffset().X*(float)regenerate), (roadType->slicePoints[roadType->slicePoints.size()-1].X*normal.Y)+basePoints[i].Z+(offsetManager->getOffset().Z*(float)regenerate));
#endif
        addHeight = hb;
        if (h0 > hb && h0 > hl) addHeight = h0;
        if (hl > h0 && hl > hb) addHeight = hl;
        if (addHeight < 0.01f)
        {
            //printf("%u. h0 %f hb %f hl %f ph %f pph %f ah %f nah %f\n", i, h0, hb, hl, prevHeight, prevPrevHeight, addHeight, prevHeight - (prevPrevHeight - prevHeight));
            addHeight = prevHeight - (prevPrevHeight - prevHeight);
            //assert(0);
        }

        prevPrevHeight = prevHeight;
        prevHeight = addHeight;
        
        float tx = 0.f;
        for (int j = 0; j < roadType->slicePoints.size(); j++)
        {
            video::S3DVertex vtx;
            if (j == 0 && /*roadType->slicePoints.size() > 3*/roadType->friction_multi > 0.01f)
            {
                vtx.Pos = vector3df(((roadType->slicePoints[j+1].X-((roadType->slicePoints[j+1].X-roadType->slicePoints[j].X)*10.f))*normal.X)+basePoints[i].X,
                                    (roadType->slicePoints[j].Y*10.f)+basePoints[i].Y+addHeight,
                                    ((roadType->slicePoints[j+1].X-((roadType->slicePoints[j+1].X-roadType->slicePoints[j].X)*10.f))*normal.Y)+basePoints[i].Z);
            }
            else
            if (j == roadType->slicePoints.size()-1 && /*roadType->slicePoints.size() > 3*/roadType->friction_multi > 0.01f)
            {
                vtx.Pos = vector3df(((roadType->slicePoints[j-1].X+((roadType->slicePoints[j].X-roadType->slicePoints[j-1].X)*10.f))*normal.X)+basePoints[i].X,
                                    (roadType->slicePoints[j].Y*10.f)+basePoints[i].Y+addHeight,
                                    ((roadType->slicePoints[j-1].X+((roadType->slicePoints[j].X-roadType->slicePoints[j-1].X)*10.f))*normal.Y)+basePoints[i].Z);
            }
            else
            {
                vtx.Pos = vector3df((roadType->slicePoints[j].X*normal.X)+basePoints[i].X,
                                    roadType->slicePoints[j].Y+basePoints[i].Y+addHeight,
                                    (roadType->slicePoints[j].X*normal.Y)+basePoints[i].Z);
                if (roadType->slicePoints.size() == 2)
                {
                    if (j == 0)
                    {
                        if (h0 > 0.01f)
                            vtx.Pos.Y = roadType->slicePoints[j].Y+basePoints[i].Y+h0;
                    }
                    else
                    {
                        if (hl > 0.01f)
                            vtx.Pos.Y = roadType->slicePoints[j].Y+basePoints[i].Y+hl;
                    }
                }
                else
                {
#ifndef NEW_ROAD_OFFSET
                    float hc = p_smallTerrain->terrain->getHeight(
                        (roadType->slicePoints[j].X*normal.X)+basePoints[i].X-(offsetManager->getOffset().X*(float)regenerate),
                        (roadType->slicePoints[j].X*normal.Y)+basePoints[i].Z-(offsetManager->getOffset().Z*(float)regenerate));
#else
                    float hc = p_smallTerrain->terrain->getHeight(
                        (roadType->slicePoints[j].X*normal.X)+basePoints[i].X+(offsetManager->getOffset().X*(float)regenerate),
                        (roadType->slicePoints[j].X*normal.Y)+basePoints[i].Z+(offsetManager->getOffset().Z*(float)regenerate));
#endif
                    if (hc > 0.01f)
                        vtx.Pos.Y = roadType->slicePoints[j].Y+basePoints[i].Y+hc;
                }
            }
            vtx.TCoords = vector2df(tx, ty);
            
            if (j < roadType->slicePoints.size() - 1) tx += (roadType->slicePoints[j+1] - roadType->slicePoints[j]).getLength()/roadType->tRate;

            // normal calculation
            
            //vector2df fornormal = vector2df(roadType->slicePoints[j].X*normal.X, roadType->slicePoints[j].X*normal.Y);
            //fornormal.normalize();
            //vtx.Normal.X = fornormal.X;
            //vtx.Normal.Z = fornormal.Y;
            /*
            if (j == 0)
            {
                vector2df cucc = roadType->slicePoints[j+1] - roadType->slicePoints[j];
                cucc.normalize();
                vtx.Normal.Y = -cucc.Y;
            }
            else if (j == roadType->slicePoints.size()-1)
            {
                //vtx.Normal.Y = vector2df(roadType->slicePoints[j] - roadType->slicePoints[j-1]).Y;
                vector2df cucc = roadType->slicePoints[j] - roadType->slicePoints[j-1];
                cucc.normalize();
                vtx.Normal.Y = -cucc.Y;
            }
            else
            {
                //vtx.Normal.Y = vector2df(roadType->slicePoints[j+1] - roadType->slicePoints[j-1]).Y;
                vector2df cucc = roadType->slicePoints[j+1] - roadType->slicePoints[j-1];
                cucc.normalize();
                vtx.Normal.Y = -cucc.Y;
            }
            */
            /*
            if (j == 0)
            {
                vector2df cucc = roadType->slicePoints[j+1] - roadType->slicePoints[j];
                cucc.normalize();
                vtx.Normal.Y = fabsf(cucc.X);
            }
            else if (j == roadType->slicePoints.size()-1)
            {
                //vtx.Normal.Y = vector2df(roadType->slicePoints[j] - roadType->slicePoints[j-1]).Y;
                vector2df cucc = roadType->slicePoints[j] - roadType->slicePoints[j-1];
                cucc.normalize();
                vtx.Normal.Y = fabsf(cucc.X);
            }
            else
            {
                //vtx.Normal.Y = vector2df(roadType->slicePoints[j+1] - roadType->slicePoints[j-1]).Y;
                vector2df vector1 = roadType->slicePoints[j+1] - roadType->slicePoints[j];
                vector2df vector2 = roadType->slicePoints[j] - roadType->slicePoints[j-1];
                vector2df cucc = vector2 - vector1;
                cucc.normalize();
                vtx.Normal.Y = fabsf(cucc.Y);
            }
            */
            //vtx.Normal.normalize();
//            printf("%f %f %f\n", vtx.Normal.X, vtx.Normal.Y, vtx.Normal.Z);
            buffer->Vertices.push_back(vtx);
        }
        if (i < basePoints.size() - 1)
        {
            ty += (basePoints[i+1] - basePoints[i]).getLength()/roadType->tRate;
            for (int k = 0; k < roadType->sliceIndices.size(); k++)
            {
                buffer->Indices.push_back(vertexCount+roadType->sliceIndices[k]);
            }
            /*
            for (int j = 0; j < roadType->slicePoints.size(); j++)
            {
                if (j == 0)
                {
            	    core::plane3d<f32> p(
        		                   buffer->Vertices[vertexCount+j].Pos,
        		                   buffer->Vertices[vertexCount+j+roadType->slicePoints.size()].Pos,
        		                   buffer->Vertices[vertexCount+j+1].Pos);
                    p.Normal.normalize();
                    buffer->Vertices[vertexCount+j].Normal = p.Normal;
                }
                else if (j == roadType->slicePoints.size()-1)
                {
            	    core::plane3d<f32> p(
        		                   buffer->Vertices[vertexCount+j-1].Pos,
        		                   buffer->Vertices[vertexCount+j+roadType->slicePoints.size()].Pos,
        		                   buffer->Vertices[vertexCount+j].Pos);
                    p.Normal.normalize();
                    buffer->Vertices[vertexCount+j].Normal = p.Normal;
                }
                else
                {
            	    core::plane3d<f32> p(
        		                   buffer->Vertices[vertexCount+j-1].Pos,
        		                   buffer->Vertices[vertexCount+j+roadType->slicePoints.size()].Pos,
        		                   buffer->Vertices[vertexCount+j+1].Pos);
                    p.Normal.normalize();
                    buffer->Vertices[vertexCount+j].Normal = p.Normal;
                }
                printf("%f %f %f\n", buffer->Vertices[vertexCount+j].Normal.X,
                    buffer->Vertices[vertexCount+j].Normal.Y,
                    buffer->Vertices[vertexCount+j].Normal.Z);
                
            }
            */
            vertexCount += roadType->slicePoints.size();
        }
    }

    for (s32 ind=0; ind<(s32)buffer->Indices.size(); ind+=3)
    {
	    core::plane3d<f32> p(
		                   buffer->Vertices[buffer->Indices[ind+0]].Pos,
		                   buffer->Vertices[buffer->Indices[ind+1]].Pos,
		                   buffer->Vertices[buffer->Indices[ind+2]].Pos);
        p.Normal.normalize();
        //printf("%f %f %f\n", p.Normal.X, p.Normal.Y, p.Normal.Z);
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
   
    roadNode = smgr->addAnimatedMeshSceneNode(animatedMesh);
    if (useShaders)
       roadNode->setMaterialFlag(video::EMF_LIGHTING, false);
    else
       roadNode->setMaterialFlag(video::EMF_LIGHTING, globalLight);
    
    roadNode->setMaterialTexture(0, roadType->texture);
    if (useShaders && useCgShaders)
    {
        roadNode->setMaterialTexture(1, p_shadowMap);
    }
    if (roadType->friction_multi > 0.01f)
    {
        roadNode->setMaterialType((video::E_MATERIAL_TYPE)myMaterialType_road);
    }
    else
    {
        roadNode->setMaterialType((video::E_MATERIAL_TYPE)myMaterialType_transp_road);
    }
//    }
//    else
//    {
//        if (texture->hasAlpha())
//        {
//            //assert(0 && "trans");
//            roadNode->setMaterialType(/*video::EMT_SOLID*/video::EMT_TRANSPARENT_ALPHA_CHANNEL);
//        }
//        else
//        {
//            //assert(0 && "solid");
//            roadNode->setMaterialType(video::EMT_SOLID/*video::EMT_TRANSPARENT_ALPHA_CHANNEL*/);
//        }
//    }

//////////////////
#if 1
    if (roadType->friction_multi > 0.01f)
    {
        dprintf(printf("road build begin %d\n", animatedMesh->getMeshBufferCount()));
        NewtonWorldCriticalSectionLock(nWorld);
        collision = NewtonCreateTreeCollision(nWorld, roadID/*, NULL*/);
        NewtonTreeCollisionBeginBuild(collision);
        {
    	    int j;
            int v1i, v2i, v3i;
    //        IMeshBuffer *mb;
            float vArray[9]; // vertex array (3*3 floats)
            float maxes[3] = {0.f,0.f,0.f};
            float mines[3] = {100000.f,100000.f,100000.f};
            int pols = 0;
            int step = 1;
            
            scene::IMeshBuffer* mb = animatedMesh->getMeshBuffer(0);
            //assert(mb->getIndexType()==EIT_32BIT);
    
    		video::S3DVertex/*2TCoords*/* mb_vertices = (irr::video::S3DVertex2TCoords*)mb->getVertices();
    		
    		u16* mb_indices  = mb->getIndices();
            
            //float* my_vertices = new float[3*mb->getIndexCount()];
            
            dprintf(printf("index count: %d vertex count %d\n", mb->getIndexCount(), mb->getVertexCount()));
            if (mb->getIndexType()==EIT_32BIT/*mb->getVertexCount()>= 256*256*/)
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
    	
    			vArray[0] = mb_vertices[v1i].Pos.X;
    			vArray[1] = mb_vertices[v1i].Pos.Y;
    			vArray[2] = mb_vertices[v1i].Pos.Z;
    			vArray[3] = mb_vertices[v2i].Pos.X;
    			vArray[4] = mb_vertices[v2i].Pos.Y;
    			vArray[5] = mb_vertices[v2i].Pos.Z;
    			vArray[6] = mb_vertices[v3i].Pos.X;
    			vArray[7] = mb_vertices[v3i].Pos.Y;
    			vArray[8] = mb_vertices[v3i].Pos.Z;
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
    			NewtonTreeCollisionAddFace(collision, 3, (float*)vArray, 12, 1);
                //dprintf(printf("v1i %d: %f %f %f\n", v1i, vArray[0], vArray[1], vArray[2]);)
                //dprintf(printf("v2i %d: %f %f %f\n", v2i, vArray[3], vArray[4], vArray[5]);)
                //dprintf(printf("v3i %d: %f %f %f\n", v3i, vArray[6], vArray[7], vArray[8]);)
    		}
            dprintf(printf("road pols: %d mines: %f %f %f\n", pols, mines[0], mines[1], mines[2]));
            dprintf(printf("road pols: %d maxes: %f %f %f\n", pols, maxes[0], maxes[1], maxes[2]));
    		//mb->drop();
        }
        dprintf(printf("collisionendbuild\n"));
        //NewtonWorldCriticalSectionLock(nWorld);
        NewtonTreeCollisionEndBuild(collision, 0);
        dprintf(printf("collisionendbuild 1\n"));
        NewtonWorldCriticalSectionUnlock(nWorld);
        dprintf(printf("road build end\n"));
    
        newtonBody = NewtonCreateBody(nWorld, collision);
        dprintf(printf("road newton body %p\n", newtonBody);)
        NewtonBodySetMaterialGroupID(newtonBody, roadID);
    	NewtonBodySetUserData(newtonBody, this);
    }
#endif // 0 or 1
////////////////////////
#ifdef NEW_ROAD_OFFSET
    vector3df rpos = roadNode->getPosition();
    rpos.X += (offsetManager->getOffset().X/**(float)regenerate*/);
    rpos.Z += (offsetManager->getOffset().Z/**(float)regenerate*/);
    roadNode->setPosition(rpos);
#endif
    if (offsetObject)
    {
        offsetObject->setNode(roadNode);
        offsetObject->setBody(newtonBody);
        offsetManager->addObject(offsetObject);
    }
    else
    {
        offsetObject = new OffsetObject(roadNode, newtonBody);
        offsetManager->addObject(offsetObject);
    }

    return roadNode;
}

bool CMyRoad::loadRoads(const char* name, core::array<CMyRoad*> &roadList,
                        ISceneManager* p_smgr, IVideoDriver* p_driver,
                        NewtonWorld *p_nWorld, BigTerrain* p_bigTerrain)
{
    FILE* f;
    int ret = 0;
    int numOfRoads = 0;

    dprintf(printf("Read roads: %s\n", name));

    f = fopen(name, "r");

    if (!f)
    {
        printf("road file unable to open: %s\n", name);
        return false;
    }

    ret = fscanf(f, "num_of_roads: %u\n", &numOfRoads);
    if (ret < 1)
    {
        printf("error reading %s ret %d errno %d\n", name, ret, errno);
        fclose(f);
        return false;
    }
    
    for (int i = 0; i < numOfRoads; i++)
    {
        CMyRoad* road = new CMyRoad(p_smgr, p_driver, p_nWorld);
        if (road->load(f, p_bigTerrain))
        {
            roadList.push_back(road);
            // road->generateRoadNode();
        }
        else
        {
            printf("error reading road from file: %s \n", name);
            fclose(f);
            return false;
        }
    }
    
    fclose(f);
    
    return true;
}

bool CMyRoad::saveRoads(const char* name, core::array<CMyRoad*> &roadList)
{
    FILE* f;
    int ret = 0;

    dprintf(printf("Write roads: %s\n", name));

    f = fopen(name, "w");
    
    if (!f)
    {
        printf("road file unable to open: %s\n", name);
        return false;
    }

    ret = fprintf(f, "num_of_roads: %u\n", roadList.size());
    if (ret < 1)
    {
        printf("error writing %s ret %d errno %d\n", name, ret, errno);
        fclose(f);
        return false;
    }
    
    for (int i = 0; i < roadList.size(); i++)
    {
        if (!roadList[i]->save(f))
        {
            printf("error writing road to file: %s \n", name);
            fclose(f);
            return false;
        }
    }

    fclose(f);
    
    return false;
}
    
bool CMyRoad::load(FILE* f, BigTerrain* p_bigTerrain)
{
    int ret = 0;
    unsigned int num;
    int ind;
    float f1, f2, f3;
    
    ret = fscanf(f, "type: %u\n", &num);
    if (ret < 1)
    {
        printf("error reading type from road file\n");
        return false;
    }
    setType(num);
    basePoints.clear();
    
    ret = fscanf(f, "num_of_base_points: %u\n", &num);
    if (ret < 1)
    {
        printf("error reading num of base points from road file\n");
        return false;
    }
    
    for (int i = 0; i < num; i++)
    {
        ret = fscanf(f, "%f %f\n", &f1, &f2);
        if (ret < 2)
        {
            printf("error reading %d. base point from road file\n", i);
            return false;
        }
        /*basePoints.push_back*/
        addBasePoint(vector3df(f1, 0.f, f2), p_bigTerrain, true, false, false);
    }
    
    return true;
}

bool CMyRoad::save(FILE* f)
{
    int ret = 0;

    ret = fprintf(f, "type: %u\n", type);
    if (ret < 1)
    {
        printf("error writing type to road file\n");
        return false;
    }

    ret = fprintf(f, "num_of_base_points: %u\n", basePoints.size());
    if (ret < 1)
    {
        printf("error writing num of base points to road file\n");
        return false;
    }
    
    for (int i = 0; i < basePoints.size(); i++)
    {
        ret = fprintf(f, "%f %f\n", basePoints[i].X, basePoints[i].Z);
        if (ret < 2)
        {
            printf("error writing %d. base point to road file\n", i);
            return false;
        }
    }

    return true;
}
