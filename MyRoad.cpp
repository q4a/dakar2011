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

CMyRoad::CMyRoad(ISceneManager* p_smgr, IVideoDriver* p_driver, NewtonWorld *p_nWorld) : 
    basePoints(),
    slicePoints(),
    sliceIndices(),
    roadNode(0),
    newtonBody(0),
    collision(0),
    tRate(0.f),
    smgr(p_smgr),
    driver(p_driver),
    nWorld(p_nWorld),
    texture(0),
    friction_multi(0.9f),
    offsetObject(0)
{
    memset(textureName, 0, sizeof(textureName));
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
        NewtonReleaseCollision(nWorld, collision);
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
}

void CMyRoad::addBasePoint(const core::vector3df& newPoint)
{
    if (basePoints.size() > 0)
    {
        vector3df bp = basePoints[basePoints.size() - 1];
        vector3df dir = newPoint - bp;
        float dist = dir.getLength();
        float cur = 0.f;
        while (cur + 8.f < dist)
        {
            cur += 5.f;
            basePoints.push_back(bp + dir*(cur/dist));
        }
    }
    basePoints.push_back(newPoint);
}

void CMyRoad::setBasePoints(const core::array<core::vector3df> &newBasePoints)
{
    basePoints = newBasePoints;
}

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

void CMyRoad::setTextureName(const char* newTextureName)
{
    strcpy(textureName, newTextureName);
}

ISceneNode* CMyRoad::generateRoadNode(SmallTerrain* p_smallTerrain, unsigned int regenerate)
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
        NewtonReleaseCollision(nWorld, collision);
    }
    collision = 0;

    if (offsetObject)
    {
        offsetObject->setBody(0);
        offsetObject->setNode(0);
        offsetManager->removeObject(offsetObject);
    }
    
    if (basePoints.size() < 2 || slicePoints.size() < 2 || sliceIndices.size()%3 != 0)
    {
        return roadNode;
    }
    
    SMeshBuffer* buffer = new SMeshBuffer();
    SMesh* mesh = new SMesh();

    float prevHeight = 0.f;
    float prevPrevHeight = 0.f;

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
        {
            float h0 = p_smallTerrain->terrain->getHeight((slicePoints[0].X*normal.X)+basePoints[i].X-(offsetManager->getOffset().X*(float)regenerate), (slicePoints[0].X*normal.Y)+basePoints[i].Z-(offsetManager->getOffset().Z*(float)regenerate));
            float hb = p_smallTerrain->terrain->getHeight(basePoints[i].X-(offsetManager->getOffset().X*(float)regenerate), basePoints[i].Z-(offsetManager->getOffset().Z*(float)regenerate));
            float hl = p_smallTerrain->terrain->getHeight((slicePoints[slicePoints.size()-1].X*normal.X)+basePoints[i].X-(offsetManager->getOffset().X*(float)regenerate), (slicePoints[slicePoints.size()-1].X*normal.Y)+basePoints[i].Z-(offsetManager->getOffset().Z*(float)regenerate));
            addHeight = hb;
            if (h0 > hb && h0 > hl) addHeight = h0;
            if (hl > h0 && hl > hb) addHeight = hl;
            if (addHeight < 0.01f)
            {
                //printf("%u. h0 %f hb %f hl %f ph %f pph %f ah %f nah %f\n", i, h0, hb, hl, prevHeight, prevPrevHeight, addHeight, prevHeight - (prevPrevHeight - prevHeight));
                addHeight = prevHeight - (prevPrevHeight - prevHeight);
                //assert(0);
            }
        }
        prevPrevHeight = prevHeight;
        prevHeight = addHeight;
        
        float tx = 0.f;
        for (int j = 0; j < slicePoints.size(); j++)
        {
            video::S3DVertex vtx;
            if (j == 0)
            {
                vtx.Pos = vector3df(((slicePoints[j+1].X-((slicePoints[j+1].X-slicePoints[j].X)*10.f))*normal.X)+basePoints[i].X,
                                    (slicePoints[j].Y*10.f)+basePoints[i].Y+addHeight,
                                    ((slicePoints[j+1].X-((slicePoints[j+1].X-slicePoints[j].X)*10.f))*normal.Y)+basePoints[i].Z);
            }
            else
            if (j == slicePoints.size()-1)
            {
                vtx.Pos = vector3df(((slicePoints[j-1].X+((slicePoints[j].X-slicePoints[j-1].X)*10.f))*normal.X)+basePoints[i].X,
                                    (slicePoints[j].Y*10.f)+basePoints[i].Y+addHeight,
                                    ((slicePoints[j-1].X+((slicePoints[j].X-slicePoints[j-1].X)*10.f))*normal.Y)+basePoints[i].Z);
            }
            else
            {
                vtx.Pos = vector3df((slicePoints[j].X*normal.X)+basePoints[i].X,
                                    slicePoints[j].Y+basePoints[i].Y+addHeight,
                                    (slicePoints[j].X*normal.Y)+basePoints[i].Z);
            }
            vtx.TCoords = vector2df(tx, ty);
            
            if (j < slicePoints.size() - 1) tx += (slicePoints[j+1] - slicePoints[j]).getLength()/tRate;
            
            buffer->Vertices.push_back(vtx);
        }
        if (i < basePoints.size() - 1)
        {
            ty += (basePoints[i+1] - basePoints[i]).getLength()/tRate;
            for (int k = 0; k < sliceIndices.size(); k++)
            {
                buffer->Indices.push_back(vertexCount+sliceIndices[k]);
            }
            vertexCount += slicePoints.size();
        }
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
   
    roadNode = smgr->addAnimatedMeshSceneNode(animatedMesh);
    if (useShaders)
       roadNode->setMaterialFlag(video::EMF_LIGHTING, false);
    else
       roadNode->setMaterialFlag(video::EMF_LIGHTING, globalLight);
    
    roadNode->setMaterialTexture(0, texture);
    if (useShaders && useCgShaders)
        roadNode->setMaterialType((video::E_MATERIAL_TYPE)myMaterialType_transp_road);
    else
        roadNode->setMaterialType(video::EMT_TRANSPARENT_ALPHA_CHANNEL);

//////////////////
#if 1
    dprintf(printf("road build begin %d\n", animatedMesh->getMeshBufferCount()));
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
    NewtonWorldCriticalSectionLock(nWorld);
    NewtonTreeCollisionEndBuild(collision, 0);
    NewtonWorldCriticalSectionUnlock(nWorld);
    dprintf(printf("road build end\n"));

    newtonBody = NewtonCreateBody(nWorld, collision);
    dprintf(printf("road newton body %p\n", newtonBody);)
    NewtonBodySetMaterialGroupID(newtonBody, roadID);
	NewtonBodySetUserData(newtonBody, this);
#endif // 0 or 1
////////////////////////
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
                        ISceneManager* p_smgr, IVideoDriver* p_driver, NewtonWorld *p_nWorld)
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
        if (road->load(f))
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
    
bool CMyRoad::load(FILE* f)
{
    int ret = 0;
    int num;
    int ind;
    float f1, f2, f3;
    
    ret = fscanf(f, "texture: %s\n", textureName);
    if (ret < 1)
    {
        printf("error reading texture name from road file\n");
        return false;
    }
    texture = driver->getTexture(textureName);
    
    ret = fscanf(f, "num_of_slice_points: %u\n", &num);
    if (ret < 1)
    {
        printf("error reading num of slice points from road file\n");
        return false;
    }
    
    for (int i = 0; i < num; i++)
    {
        ret = fscanf(f, "%f %f\n", &f1, &f2);
        if (ret < 2 )
        {
            printf("error reading %d. slice point from road file\n", i);
            return false;
        }
        slicePoints.push_back(vector2df(f1, f2));
    }
    
    ret = fscanf(f, "num_of_slice_indices: %u\n", &num);
    if (ret < 1)
    {
        printf("error reading num of slice indices from road file\n");
        return false;
    }
    
    for (int i = 0; i < num; i++)
    {
        ret = fscanf(f, "%d\n", &ind);
        if (ret < 1 )
        {
            printf("error reading %d. slice index from road file\n", i);
            return false;
        }
        sliceIndices.push_back(ind);
    }
    
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
        addBasePoint(vector3df(f1, 0.f, f2));
    }
    
    return true;
}

bool CMyRoad::save(FILE* f)
{
    int ret = 0;

    ret = fprintf(f, "texture: %s\n", textureName);
    if (ret < 1)
    {
        printf("error writing texture name to road file\n");
        return false;
    }
    
    ret = fprintf(f, "num_of_slice_points: %u\n", slicePoints.size());
    if (ret < 1)
    {
        printf("error writing num of slice points to road file\n");
        return false;
    }
    
    for (int i = 0; i < slicePoints.size(); i++)
    {
        ret = fprintf(f, "%f %f\n", slicePoints[i].X, slicePoints[i].Y);
        if (ret < 2)
        {
            printf("error writing %d. slice point to road file\n", i);
            return false;
        }
    }

    ret = fprintf(f, "num_of_slice_indices: %u\n", sliceIndices.size());
    if (ret < 1)
    {
        printf("error writing num of slice indices to road file\n");
        return false;
    }
    
    for (int i = 0; i < sliceIndices.size(); i++)
    {
        ret = fprintf(f, "%d\n", sliceIndices[i]);
        if (ret < 1)
        {
            printf("error writing %d. slice index to road file\n", i);
            return false;
        }
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
