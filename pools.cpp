/****************************************************************
*                                                               *
*    Name: pools.cpp                                            *
*                                                               *
*    Creator: Balazs Tuska                                      *
*                                                               *
*    Description:                                               *
*       This file contains the pool of the objects, grass and   *
*       the trees.                                              *
*                                                               *
****************************************************************/

#include <assert.h>
#include <string.h>
#include "pools.h"
#include "MyList.h"
#include "settings.h"
#include "my_shaders.h"
#include "Materials.h"
#include "gameplay.h"

#include "CTreeGenerator.h"
#include "CBillboardGroupSceneNode.h"
#include "CTreeSceneNode.h"

#ifdef __linux__
#include "linux_includes.h"
#endif

#ifdef MY_DEBUG
#define dprintf(x) x
#else
#define dprintf(x)
#endif

#define USE_GRASS_BILLBOARDS 0
#ifndef USE_GRASS_BILLBOARDS
#define USE_GRASS_MSO 0
#endif

struct STreeDesign
{
    CTreeGenerator* Generator;
    video::ITexture* TreeTexture;
    video::ITexture* LeafTexture;
    video::ITexture* BillTexture;
};

core::array<STreeDesign*> treeDesigns;

//int treetypes_num = 0;

struct ObjectWrapper
{
public:
    ObjectWrapper(NewtonWorld* pnWorld)
        :objectNode(0), farObjectNode(0), visible(true), fvisible(true), nWorld(pnWorld), body(0),
         collision(0), addShadow(false), shadowNode(0), type(NONE)
     {}
    ~ObjectWrapper()
    {
        objectNode->remove();
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
        if (farObjectNode)
            farObjectNode->remove();
    }
    
    void setVisible(bool pvisible)
    {
        if (pvisible==visible) return;
        visible = pvisible;
        objectNode->setVisible(visible);
        if (visible)
        {
            if (collision)
            {
                //printf("createbody\n");
                body = NewtonCreateBody(nWorld, collision);
        
                NewtonBodySetMaterialGroupID(body, treeID);
        
                // set the newton world size based on the bsp size
                
                //float boxP0[3]; 
                //float boxP1[3]; 
                float matrix[16]; 
                NewtonBodyGetMatrix (body, &matrix[0]); 
                matrix[12] = objectNode->getPosition().X; //+ ofs.X - box.X * 0.5f;
                matrix[13] = objectNode->getPosition().Y; //+ ofs.Y;
                matrix[14] = objectNode->getPosition().Z; //+ ofs.Z - box.Z * 0.5f;
                NewtonBodySetMatrix (body, &matrix[0]); 
                //printf("calculateAABB\n");
                //NewtonCollisionCalculateAABB (collision, &matrix[0],  &boxP0[0], &boxP1[0]); 
                
                // you can pad the box here if you wish
                /*
                boxP1[1] = 10000;
                printf("P0\n");
                for (int my_i = 0; my_i < 3; my_i++)
                    printf("%f ", boxP0[my_i]);
                printf("\nP1\n");
                for (int my_i = 0; my_i < 3; my_i++)
                    printf("%f ", boxP1[my_i]);
                printf("\n");
                //boxP0.y -= 10.f; 
                //boxP1.y += somevaluef; 
                //NewtonSetWorldSize (nWorld, (float*)boxP0, (float*)boxP1);
                */
                /*
                if (globalLight)
                {
                    //objectNode->addShadowVolumeSceneNode();
                    objectNode->setMaterialFlag(video::EMF_NORMALIZE_NORMALS, true);
                }
                */
            }
            if (addShadow)
            {
                if (shadowNode)
                    addToShadowNodes(shadowNode);
                else
                    addToShadowNodes(objectNode);
            }
            addToObjectNodes(objectNode);
        }
        else
        {
            if (addShadow)
            {
                if (shadowNode)
                    removeFromShadowNodes(shadowNode);
                else
                    removeFromShadowNodes(objectNode);
            }
            removeFromObjectNodes(objectNode);
            if (body)
            {
                NewtonDestroyBody(nWorld, body);
                body = 0;
            }
        }
    }
    
    void setFarVisible(bool pvisible)
    {
        if (pvisible==fvisible) return;
        fvisible = pvisible;
        if (farObjectNode)
        {
            farObjectNode->setVisible(fvisible);
            if (fvisible)
            {
                addToObjectNodes(farObjectNode);
            }
            else
            {
                removeFromObjectNodes(farObjectNode);
            }
        }
    }
    
    void calculateCollision(const core::vector3df& pbox, const core::vector3df& pofs)
    {
        if (pbox == vector3df(0.0f, 0.0f, 0.0f)) return;
        box = pbox;
        ofs = pofs;
        collision = NewtonCreateBox(nWorld, box.X, box.Y, box.Z, treeID, NULL);
    }

    void calculateCollision(const IAnimatedMesh* mesh, const core::vector3df& pbox,
                            const vector3df& scale,
                            const char* name, const int num)
    {
        if (pbox == vector3df(0.0f, 0.0f, 0.0f)) return;
        //if (!num)
        //printf("ojject %s mbc %u\n", name, mesh->getMeshBufferCount());
    
        int sizeOfBuffers = 0;
        for (int i = 0; i < mesh->getMeshBufferCount(); i++)
        {
            if (mesh->getMeshBuffer(i)->getVertexType() != EVT_STANDARD)
            {
                if (!num)
                printf("ojject %u type missmatch %u\n", i, mesh->getMeshBuffer(i)->getVertexType());
                
                return;
            }
            
            sizeOfBuffers += mesh->getMeshBuffer(i)->getVertexCount();
        }
        
        float* my_vertices = new float[sizeOfBuffers*3];
        int cursor = 0;
        
        for (int i = 0; i < mesh->getMeshBufferCount();i++)
        {
            IMeshBuffer* mb = mesh->getMeshBuffer(i);
            video::S3DVertex* mb_vertices = (video::S3DVertex*)mb->getVertices();
            for (int j = 0; j < mb->getVertexCount(); j++)
            {
                my_vertices[(cursor+j)*3] = mb_vertices[j].Pos.X*scale.X;
                my_vertices[(cursor+j)*3+1] = mb_vertices[j].Pos.Y*scale.Y;
                my_vertices[(cursor+j)*3+2] = mb_vertices[j].Pos.Z*scale.Z;
            }
            cursor += mb->getVertexCount();
        }
    
        collision = NewtonCreateConvexHull(nWorld, sizeOfBuffers,
                                my_vertices, 3 * sizeof(float), 0.1f, vehicleID,
                                0);
        delete [] my_vertices;
    }
    
    //IAnimatedMeshSceneNode* objectNode;
    ISceneNode* objectNode;
    ISceneNode* farObjectNode;
    IShadowVolumeSceneNode* shadowNode;
    bool addShadow;
    PoolObjectType type;
private:
    bool visible;
    bool fvisible;
    NewtonBody* body;
    NewtonWorld* nWorld;
    NewtonCollision* collision;
    core::vector3df box;
    core::vector3df ofs;
};

struct nameNum
{
    c8 name[256];
    CMyList<ObjectWrapper*> objectPool;
    IAnimatedMesh* objectMesh;
    IAnimatedMesh* farObjectMesh;
};
static CMyList<nameNum*> objectPools;

struct SItinerNameTexture
{
    char name[256];
    video::ITexture* texture;
};
static core::array<SItinerNameTexture*> itinerTypes;

int createObjectPool(const c8* name,
                    const c8* fname,
                    ISceneManager* smgr, IVideoDriver* driver, NewtonWorld* nWorld,
                    int num, PoolObjectType type,
                    const c8* textureName,
                    const vector3df& rot,
                    const vector3df& sca,
                    const vector3df& box,
                    const vector3df& ofs
)
{
    int ind = 0;
    //printf("create pool: %s, t: %s\n", name, textureName);
    for (ind = 0; ind < objectPools.length();ind++)
    {
        if (strcmp(objectPools[ind]->name, name)==0)
        {
            //printf("pool exists, skip: %s, t: %s\n", name, textureName);
            break; // we found the pool return the id
        }
    }
    
    if (ind == objectPools.length()) // not found pool create new one
    {
        nameNum* newElement = new nameNum;
        strcpy(newElement->name, name);
        newElement->objectMesh = newElement->farObjectMesh = 0;
        objectPools.addLast(newElement);
        generateElementsToPool(smgr, driver, nWorld,
                            ind, num, type,
                            textureName, fname,
                            rot, sca, box, ofs);
    }

    //printf("pool created: %s, t: %s, ind: %d\n", name, textureName, ind);
    
    return ind;
}

void generateElementsToPool(ISceneManager* smgr, IVideoDriver* driver, NewtonWorld* nWorld,
                            int poolId, int num, PoolObjectType type,
                            const c8* textureName,
                            const c8* fname,
                            const vector3df& rot,
                            const vector3df& sca,
                            const vector3df& box,
                            const vector3df& ofs
)
{
    c8* name;
    
    if (poolId >= objectPools.length()) return;
    
    name = objectPools[poolId]->name;
    CMyList<ObjectWrapper*> &objectPool = objectPools[poolId]->objectPool;

    IAnimatedMesh** poolMesh = &objectPools[poolId]->objectMesh;
    IAnimatedMesh** fPoolMesh = &objectPools[poolId]->farObjectMesh;
    
    if (type==NORMAL) // normal object
    {
        for (int j = 0; j < num /*&& numOfObjects < MAX_OBJECT_NUM*/; j++)
        {
            IAnimatedMesh* objectMesh = *poolMesh;
            if(objectMesh == 0)
            {
                if(!strcmp(name+strlen(name)-3, "mso"))
                    objectMesh = readMySimpleObject(name);
                else
                    objectMesh = smgr->getMesh(name);
                *poolMesh = objectMesh;
            }
            
            IAnimatedMeshSceneNode* objectNode = smgr->addAnimatedMeshSceneNode(objectMesh);
            IAnimatedMeshSceneNode* farObjectNode = 0;

            //printf("check fname: %s\n", fname);
            if(useObjectLods && strcmp(fname, "null")) // we have a far object
            {
                //printf("read fname: %s\n", fname);
                IAnimatedMesh* farObjectMesh = *fPoolMesh;
                if (farObjectMesh==0)
                {
                    farObjectMesh = readMySimpleObject(fname);
                    *fPoolMesh = farObjectMesh;
                }
                farObjectNode = smgr->addAnimatedMeshSceneNode(farObjectMesh);
            }
            //printf("check fname: %s done obj: %p\n", fname, farObjectNode);

            ObjectWrapper* objectWrapper = new ObjectWrapper(nWorld);
            objectWrapper->objectNode = objectNode;
            objectWrapper->farObjectNode = farObjectNode;
            if (useShaders)
            {
                objectNode->setMaterialFlag(video::EMF_LIGHTING, false);
                if (farObjectNode)
                    farObjectNode->setMaterialFlag(video::EMF_LIGHTING, false);
            }
            else
            {
                objectNode->setMaterialFlag(video::EMF_LIGHTING, globalLight);
                if (farObjectNode)
                   farObjectNode->setMaterialFlag(video::EMF_LIGHTING, globalLight);
            }
//            objectNodes[numOfObjects]->setMaterialFlag(video::EMF_LIGHTING, true);
            if (globalLight)
            {
                //if (stencil_shadows)
                    objectWrapper->shadowNode = objectNode->addShadowVolumeSceneNode();
                objectNode->setMaterialFlag(video::EMF_NORMALIZE_NORMALS, true);
                if (farObjectNode)
                    farObjectNode->setMaterialFlag(video::EMF_NORMALIZE_NORMALS, true);
            }
            if (strcmp(textureName, "null"))
            {
                objectNode->setMaterialTexture(0, driver->getTexture(textureName));
                if (farObjectNode)
                {
                    objectNode->setMaterialType((video::E_MATERIAL_TYPE)myMaterialType_light_tex_wfar);
                    //farObjectNode->setMaterialTexture(0, driver->getTexture(textureName));
                    farObjectNode->setMaterialType((video::E_MATERIAL_TYPE)myMaterialType_light_notex_wfar);
                }
                else
                    objectNode->setMaterialType((video::E_MATERIAL_TYPE)myMaterialType_light_tex);
            }
            else
            {
                if (farObjectNode)
                {
                    objectNode->setMaterialType((video::E_MATERIAL_TYPE)myMaterialType_light_notex_wfar);
                    farObjectNode->setMaterialType((video::E_MATERIAL_TYPE)myMaterialType_light_notex_wfar);
                }
                else
                    objectNode->setMaterialType((video::E_MATERIAL_TYPE)myMaterialType_light_notex);
            }
            objectNode->setRotation(rot);
            objectNode->setScale(sca);
            
            //objectWrapper->calculateCollision(box, ofs);
            objectWrapper->calculateCollision(objectMesh, box, sca, name, j);
            objectWrapper->setVisible(false);
            objectWrapper->setFarVisible(false);
            objectWrapper->addShadow = !stencil_shadows;
            objectWrapper->type = NORMAL;
            objectPool.addLast(objectWrapper);
        }
    }
    else 
    if (type==GRASS)// grass
    {
        unsigned int numOfVertices, numOfPols;
        float x,y,z,tu,tv;
        s32 verInd;
        video::S3DVertex vtx;
        vtx.Color.set(255,255,255,255);
        vtx.Normal.set(0,1,0);
        for (int i = 0;i < num;i++)
        {
            ObjectWrapper* objectWrapper = new ObjectWrapper(nWorld);
            ISceneNode* objectNode;
            IAnimatedMesh* objectMesh = *poolMesh;
            switch (grass_type)
            {
            case GRASS_GENERATED:
            {
                if (objectMesh==0)
                {
            	    SMeshBuffer* buffer = new SMeshBuffer();
            	    SMesh* mesh = new SMesh();
            	    const float width = 3.f;//((float)(rand() % 10) + 1.f) / 5.f;
            	    const float height = 2.f;//((float)(rand() % 20) + 2.f) / 6.f;
            
                    //printf("debug %d %f %f\n", i, traX, traZ);
            	    
                    for (int j = 0; j < 7; j++)
                    {
                	    float offx = ((float)(rand() % GENERATED_GRASS_SIZE - (GENERATED_GRASS_SIZE / 2)));
                	    float offz = ((float)(rand() % GENERATED_GRASS_SIZE - (GENERATED_GRASS_SIZE / 2)));
                        
                        vtx.Pos.X = offx-width/2.f;
                        vtx.Pos.Z = offz+0.f;
                        vtx.Pos.Y = 0.f;
                        vtx.TCoords.X = 0.f;
                        vtx.TCoords.Y = 1.f;
                        buffer->Vertices.push_back(vtx);
                
                        vtx.Pos.X = offx+width/2.f;
                        vtx.Pos.Z = offz+0.f;
                        vtx.Pos.Y = 0.f;
                        vtx.TCoords.X = 1.f/*width/3.f*/;
                        vtx.TCoords.Y = 1.f;
                        buffer->Vertices.push_back(vtx);
                
                        vtx.Pos.X = offx+width/2.f;
                        vtx.Pos.Z = offz+0.f;
                        vtx.Pos.Y = height;
                        vtx.TCoords.X = 1.f/*width/3.f*/;
                        vtx.TCoords.Y = 0.f;
                        buffer->Vertices.push_back(vtx);
                
                        vtx.Pos.X = offx-width/2.f;
                        vtx.Pos.Z = offz+0.f;
                        vtx.Pos.Y = height;
                        vtx.TCoords.X = 0.f;
                        vtx.TCoords.Y = 0.f;
                        buffer->Vertices.push_back(vtx);
                
                // other dir
                        vtx.Pos.X = offx+0.f;
                        vtx.Pos.Z = offz-width/2.f;
                        vtx.Pos.Y = 0.f;
                        vtx.TCoords.X = 0.f;
                        vtx.TCoords.Y = 1.f;
                        buffer->Vertices.push_back(vtx);
                
                        vtx.Pos.X = offx+0.f;
                        vtx.Pos.Z = offz+width/2.f;
                        vtx.Pos.Y = 0.f;
                        vtx.TCoords.X = 1.f/*width/3.f*/;
                        vtx.TCoords.Y = 1.f;
                        buffer->Vertices.push_back(vtx);
                
                        vtx.Pos.X = offx+0.f;
                        vtx.Pos.Z = offz+width/2.f;
                        vtx.Pos.Y = height;
                        vtx.TCoords.X = 1.f/*width/3.f*/;
                        vtx.TCoords.Y = 0.f;
                        buffer->Vertices.push_back(vtx);
                
                        vtx.Pos.X = offx+0.f;
                        vtx.Pos.Z = offz-width/2.f;
                        vtx.Pos.Y = height;
                        vtx.TCoords.X = 0.f;
                        vtx.TCoords.Y = 0.f;
                        buffer->Vertices.push_back(vtx);
                
                        buffer->Indices.push_back((j*8)+0);
                        buffer->Indices.push_back((j*8)+1);
                        buffer->Indices.push_back((j*8)+2);
                
                        buffer->Indices.push_back((j*8)+0);
                        buffer->Indices.push_back((j*8)+2);
                        buffer->Indices.push_back((j*8)+3);
                
                        buffer->Indices.push_back((j*8)+0);
                        buffer->Indices.push_back((j*8)+2);
                        buffer->Indices.push_back((j*8)+1);
                
                        buffer->Indices.push_back((j*8)+0);
                        buffer->Indices.push_back((j*8)+3);
                        buffer->Indices.push_back((j*8)+2);
                // other dir
                        buffer->Indices.push_back((j*8)+4);
                        buffer->Indices.push_back((j*8)+5);
                        buffer->Indices.push_back((j*8)+6);
                
                        buffer->Indices.push_back((j*8)+4);
                        buffer->Indices.push_back((j*8)+6);
                        buffer->Indices.push_back((j*8)+7);
                
                        buffer->Indices.push_back((j*8)+4);
                        buffer->Indices.push_back((j*8)+6);
                        buffer->Indices.push_back((j*8)+5);
                
                        buffer->Indices.push_back((j*8)+4);
                        buffer->Indices.push_back((j*8)+7);
                        buffer->Indices.push_back((j*8)+6);
                    }
                    //printf("debug %d norm start\n", i);
                    
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
            
                    //printf("debug %d norm end\n", i);
                   
                    buffer->recalculateBoundingBox();
            
            	    SAnimatedMesh* animatedMesh = new SAnimatedMesh();
            	    mesh->addMeshBuffer(buffer);
            	    mesh->recalculateBoundingBox();
            	    animatedMesh->addMesh(mesh);
            	    animatedMesh->recalculateBoundingBox();
            
            	    mesh->drop();
            	    buffer->drop();
                    
                    objectMesh = animatedMesh;
                    *poolMesh = objectMesh;
                }
                objectNode = smgr->addAnimatedMeshSceneNode(objectMesh);
                break;
            }
            case GRASS_BILLBOARD:
            {
                objectNode = smgr->addBillboardSceneNode(0, core::dimension2d<f32>(2.5f, 3.f));
                break;
            }
            case GRASS_BILLBOARD_GROUP:
            {
                objectNode = new CBillboardGroupSceneNode(0, smgr);
                for (int j = 0; j < 7; j++)
                {
            	    float offx = ((float)(rand() % GENERATED_GRASS_SIZE - (GENERATED_GRASS_SIZE / 2)));
            	    float offz = ((float)(rand() % GENERATED_GRASS_SIZE - (GENERATED_GRASS_SIZE / 2)));
            	    
            	    ((CBillboardGroupSceneNode*)objectNode)->addBillboard(vector3df(offx, 0.f, offz),
            	       core::dimension2d<f32>(2.5f, 3.f));
                }
                break;
            }
            case GRASS_OBJECT:
            {
                //printf("read grass %s\n", name);
                if (objectMesh==0)
                {
                    if(!strcmp(name+strlen(name)-3, "mso"))
                        objectMesh = readMySimpleObject(name);
                    else
                        objectMesh = smgr->getMesh(name);
                    *poolMesh = objectMesh;
                }
                objectNode = (ISceneNode*)smgr->addAnimatedMeshSceneNode(objectMesh);
                break;
            }
            } // switch

            objectWrapper->objectNode = objectNode;
            //objectNode->setMaterialFlag(video::EMF_LIGHTING, false);
            if (useShaders)
                objectNode->setMaterialFlag(video::EMF_LIGHTING, false);
            else
                objectNode->setMaterialFlag(video::EMF_LIGHTING, globalLight);
            if (globalLight)
            {
                //if (stencil_shadows)
                //    objectWrapper->shadowNode = objectNode->addShadowVolumeSceneNode();
                objectNode->setMaterialFlag(video::EMF_NORMALIZE_NORMALS, true);
            }
            if (useShaders && useCgShaders)
            {
                //if (grass_type == GRASS_BILLBOARD_GROUP)
                //    objectNode->setMaterialType((video::E_MATERIAL_TYPE)myMaterialType_transp_stat);
                //else
                    objectNode->setMaterialType((video::E_MATERIAL_TYPE)myMaterialType_transp);
            }
            else
                objectNode->setMaterialType(video::EMT_TRANSPARENT_ALPHA_CHANNEL);
            //objectNode->getMaterial(0).TextureLayer[0].TextureWrap = video::ETC_CLAMP;
            objectNode->getMaterial(0).MaterialTypeParam = 0.5f;
    	    objectNode->setMaterialTexture(0, driver->getTexture(textureName));
    	    objectWrapper->setVisible(false);
            objectWrapper->type = GRASS;

            objectPool.addLast(objectWrapper);
    	    //objectNode->setRotation(vector3df(0.f,(float)(rand()%180),0.f));
            //getSmallTerrain(loc.X+traX, loc.Z+traZ)->addGrass(grassNodes[i]);
        }
    }
    else
    if (type==TREE) // trees
    {
        int type = (int) textureName;
        s32 seed = 0;
        if (type>=treeDesigns.size())
        {
            printf("no such tree type %d, we have types to %d\n", type, treeDesigns.size());
            return;
        }        
        if (treeDesigns[type]->Generator==0)
        {
            printf("no such tree type generator %d\n", type);
            return;
        }
        dprintf(printf("tree name %s, type %d rep %d\n", name, type, num));
        for (int j = 0; j < num /*&& numOfObjects < MAX_OBJECT_NUM*/; j++)
        {
            
            CTreeSceneNode* objectNode = new CTreeSceneNode(smgr->getRootSceneNode(), smgr,
                        -1, core::vector3df(0.f,0.f,0.f), rot, sca);
            ObjectWrapper* objectWrapper = new ObjectWrapper(nWorld);
            objectWrapper->objectNode = objectNode;

            objectNode->setup( treeDesigns[type]->Generator, seed, treeDesigns[type]->BillTexture );
            
            if (objectNode->getLeafNode())
            {
            	objectNode->getLeafNode()->getMaterial(0).TextureLayer[0].AnisotropicFilter = true;
                objectNode->getLeafNode()->getMaterial(0).TextureLayer[0].BilinearFilter = false;
                
                objectNode->getLeafNode()->setMaterialTexture( 0, treeDesigns[type]->LeafTexture );
                if (useShaders && useCgShaders)
                    objectNode->getLeafNode()->setMaterialType((video::E_MATERIAL_TYPE)myMaterialType_transp_stat);
                else
                    objectNode->getLeafNode()->setMaterialType( video::EMT_TRANSPARENT_ALPHA_CHANNEL );
            }        	

            objectNode->setMaterialTexture( 0, treeDesigns[type]->TreeTexture );

            if (useShaders)
                objectNode->setMaterialFlag(video::EMF_LIGHTING, false);
            else
                objectNode->setMaterialFlag(video::EMF_LIGHTING, globalLight);
//            objectNodes[numOfObjects]->setMaterialFlag(video::EMF_LIGHTING, true);
            if (globalLight)
            {
                //objectNode->addShadowVolumeSceneNode();
                objectNode->setMaterialFlag(video::EMF_NORMALIZE_NORMALS, true);
            }
            objectNode->setRotation(rot);
            objectNode->setScale(sca);
            
            //if ( lightsEnabled )
            //    objectNode->getLeafNode()->applyVertexShadows( lightDir, 1.0f, 0.25f );
            
            //objectNode->getLeafNode()->setMaterialType( leafMaterialType );

            objectNode->setMaterialType((video::E_MATERIAL_TYPE)myMaterialType_light_tex);
            
            objectWrapper->calculateCollision(box, ofs);
            objectWrapper->setVisible(false);
            objectWrapper->addShadow = true;
            objectWrapper->type = TREE;
            objectPool.addLast(objectWrapper);
        }
    }
}

void* getPoolElement(int poolId, const vector3df& pos, bool farObj)
{
    if (poolId >= objectPools.length()) return 0;
    
    CMyList<ObjectWrapper*> &objectPool = objectPools[poolId]->objectPool;
    
    ObjectWrapper* objectWrapper = objectPool.removeFirst();

    if (objectWrapper)
    {
        
        if (objectWrapper->farObjectNode)
        {
            objectWrapper->farObjectNode->setPosition(pos);
        }
        else
        {
            if (farObj)
            {
                objectPool.addFirst(objectWrapper);
                return 0;
            }
        }

        if (objectWrapper->type==GRASS && (grass_type == GRASS_BILLBOARD || grass_type == GRASS_BILLBOARD_GROUP))
        {
            objectWrapper->objectNode->setPosition(vector3df(pos.X, pos.Y+1.5f, pos.Z));
            /*
            switch (grass_type)
            {
                case GRASS_GENERATED:
                    objectWrapper->objectNode->setPosition(vector3df(pos.X, pos.Y-0.3f, pos.Z));
                    break;
                case GRASS_BILLBOARD:
                    objectWrapper->objectNode->setPosition(vector3df(pos.X, pos.Y+1.5f, pos.Z));
                    break;
                default:
                    objectWrapper->objectNode->setPosition(pos);
                    break;
            }
            */
        }
        else
            objectWrapper->objectNode->setPosition(pos);
        //objectWrapper->objectNode->updateAbsolutePosition();
        //printf("obj: %f %f %f\n", objectWrapper->objectNode->getAbsolutePosition().X, 
        //                          objectWrapper->objectNode->getAbsolutePosition().Y,
        //                          objectWrapper->objectNode->getAbsolutePosition().Z);
        objectWrapper->setVisible(!farObj);
        objectWrapper->setFarVisible(farObj);
    }
    
    return (void*)objectWrapper;
    
    
}

void putPoolElement(int poolId, void* arg)
{
    if (poolId >= objectPools.length()) return;
    
    CMyList<ObjectWrapper*> &objectPool = objectPools[poolId]->objectPool;
    
    ObjectWrapper* objectWrapper = (ObjectWrapper*)arg;
    
    if (objectWrapper)
    {
        objectWrapper->setVisible(false);
        objectWrapper->setFarVisible(false);
        objectPool.addFirst(objectWrapper);
    }
}

void setFarPoolElement(void* arg)
{
    ObjectWrapper* objectWrapper = (ObjectWrapper*)arg;
    
    if (objectWrapper)
    {
        objectWrapper->setVisible(false);
        objectWrapper->setFarVisible(true);
    }
}

bool getFarPoolElement(void* arg)
{
    ObjectWrapper* objectWrapper = (ObjectWrapper*)arg;
    
    if (objectWrapper && objectWrapper->farObjectNode)
    {
        return true;
    }
    
    return false;
}

void setNearPoolElement(void* arg)
{
    ObjectWrapper* objectWrapper = (ObjectWrapper*)arg;
    
    if (objectWrapper)
    {
        objectWrapper->setVisible(true);
        objectWrapper->setFarVisible(false);
    }
}

void printPoolStat()
{
    printf("Object pool size: %d, grass pool size: %d\n", object_pool_size, grass_pool_size);
    for (int ind = 0; ind < objectPools.length();ind++)
    {
        printf("%d. Poolname: '%s', free objects: %d\n", ind, objectPools[ind]->name, objectPools[ind]->objectPool.length());
    }
}

//#define MSO_DEBUG
SAnimatedMesh* readMySimpleObject(const char* name)
{

    FILE* f;
    unsigned int numOfVertices, numOfPols;
    float x,y,z,tu,tv;
    u32 r,g,b;
    int ret, index;
    s32 verInd;
    video::S3DVertex vtx;
    vtx.Color.set(255,255,255,255);
    vtx.Normal.set(0,1,0);

#ifdef MSO_DEBUG
    printf("Read my simple object: %s\n", name);
#endif
    
    f = fopen(name, "r");
    
    if (!f)
    {
        printf("my simple object file unable to open: %s\n", name);
        return 0;
    }
    
    SMeshBuffer* buffer = new SMeshBuffer();
    SMesh* mesh = new SMesh();
    
#ifdef MSO_DEBUG
    printf("read vertices\n");
#endif    
    ret = fscanf(f, "vertices\n%u\n", &numOfVertices);
    if (ret <= 0)
    {
       printf("error reading %s ret %d errno %d\n", name, ret, errno);
       fclose(f);
       return 0;
    }

#ifdef MSO_DEBUG
    printf("vertices: %u\n", numOfVertices);
#endif    

    for (int ind = 0; ind < numOfVertices; ind++)
    {
#ifdef MSO_DEBUG
    printf("read a vertex\n");
#endif    
        ret = fscanf(f, "%d %f %f %f %f %f %u %u %u\n", &index, &x, &y, &z, &tu, &tv, &r, &g, &b);
        if (ret <= 0)
        {
           printf("error reading %s ret %d errno %d\n", name, ret, errno);
           fclose(f);
           return 0;
        }
#ifdef MSO_DEBUG
    printf("vertex read done\n");
#endif    
        vtx.Pos.X = x;            
        vtx.Pos.Z = z;
        vtx.Pos.Y = y; 
        vtx.TCoords.X = tu;
        vtx.TCoords.Y = tv;
        vtx.Color.set(255,r,g,b);
        buffer->Vertices.push_back(vtx);
    }
#ifdef MSO_DEBUG
    printf("read polygons number\n");
#endif    
    ret = fscanf(f, "polygons\n%u\n", &numOfPols);
    if (ret <= 0)
    {
       printf("error reading %s ret %d errno %d\n", name, ret, errno);
       fclose(f);
       return 0;
    }
#ifdef MSO_DEBUG
    printf("polygons: %u\n", numOfPols);
#endif    
    for (int ind = 0; ind < numOfPols*3; ind++)
    {
#ifdef MSO_DEBUG
    printf("read a poly part\n");
#endif    
        ret = fscanf(f, "%u\n", &verInd);
        if (ret <= 0)
        {
           printf("error reading %s ret %d errno %d\n", name, ret, errno);
           fclose(f);
           return 0;
        }
#ifdef MSO_DEBUG
    printf("read a poly part done\n");
#endif    
        if (verInd >= numOfVertices)
        {
           printf("!!!!! verInd >= numOfVertices: %d > %u\n", verInd, numOfVertices);
        }
        buffer->Indices.push_back(verInd);
    }

#ifdef MSO_DEBUG
    printf("renormalize\n");
#endif    
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
#ifdef MSO_DEBUG
    printf("renormalize done\n");
#endif    
   
    buffer->recalculateBoundingBox();

    SAnimatedMesh* animatedMesh = new SAnimatedMesh();
    mesh->addMeshBuffer(buffer);
    mesh->recalculateBoundingBox();
    animatedMesh->addMesh(mesh);
    animatedMesh->recalculateBoundingBox();

    mesh->drop();
    buffer->drop();

    fclose(f);
#ifdef MSO_DEBUG
    printf("read done return %p\n", animatedMesh);
#endif    

    return animatedMesh;    
}

void releasePools()
{
    for (int i = 0; i < objectPools.length();i++)
    {
        nameNum* delThis = objectPools[i];
        
        for (int j = 0; j < delThis->objectPool.length(); j++)
        {
            ObjectWrapper* obj = delThis->objectPool[j];
            delete obj;
        }
        delThis->objectPool.delList();
        if (delThis->objectMesh)
            delThis->objectMesh->drop();
        if (delThis->farObjectMesh)
            delThis->farObjectMesh->drop();
        delete delThis;
    }
    objectPools.delList();

    for (int i = 0; i < treeDesigns.size(); i++)
    {
        if (treeDesigns[i])
        {
            if (treeDesigns[i]->Generator)
            {
                delete treeDesigns[i]->Generator;
                treeDesigns[i]->Generator = 0;
            }
            delete treeDesigns[i];
            treeDesigns[i] = 0;
        }
    }
    treeDesigns.clear();
    
    for (int i = 0; i < itinerTypes.size(); i++)
    {
        if (itinerTypes[i])
        {
            delete itinerTypes[i];
            itinerTypes[i] = 0;
        }
    }
    itinerTypes.clear();
}

void loadObjectTypes(const c8* name, ISceneManager* smgr, IVideoDriver* driver, NewtonWorld* nWorld)
{
    FILE* f;
    int ret = 0;
    c8 meshName[256];
    c8 farMeshName[256];
    c8 textureName[256];
    vector3df rot(0.f,0.f,0.f);
    vector3df sca(1.f,1.f,1.f);
    vector3df box(0.f,0.f,0.f);
    vector3df ofs(0.f,0.f,0.f);

    dprintf(printf("Read object types: %s\n", name));
    
    f = fopen(name, "r");
    
    if (!f)
    {
        printf("object type file unable to open: %s\n", name);
        return;       
    }

    srand(1); // set seed, wand object to the same place always
    
    for (int i = 0;/*numOfObjects < MAX_OBJECT_NUM*/;i++)
    {
        SObjectWrapper** objectWrappers_tmp;
        int poolId = 0;
        
        ret = fscanf(f, "nam: %s\n" \
                        "tex: %s\n" \
                        "far: %s\n" \
                        "rot: %f, %f, %f\n" \
                        "sca: %f, %f, %f\n" \
                        "box: %f, %f, %f\n" \
                        "ofs: %f, %f, %f\n",
                meshName, textureName,
                farMeshName,
                &rot.X, &rot.Y, &rot.Z,
                &sca.X, &sca.Y, &sca.Z,
                &box.X, &box.Y, &box.Z,
                &ofs.X, &ofs.Y, &ofs.Z
                );
        if (ret < 15)
        {
            // no more object
            //printf("|%s| |%s|\n", meshName, textureName);
            break;
        }
        
        if (i==0)
        {
            createObjectPool(meshName, farMeshName,
                              smgr, driver, nWorld,
                              object_pool_size*100, NORMAL,
                              textureName,
                              rot, sca, box, ofs);
        }
        else
        {
            createObjectPool(meshName, farMeshName,
                              smgr, driver, nWorld,
                              object_pool_size, NORMAL,
                              textureName,
                              rot, sca, box, ofs);
        }
    }
    fclose(f);
}

void loadGrassTypes(const c8* name, ISceneManager* smgr, IVideoDriver* driver, NewtonWorld* nWorld)
{
    FILE* f;
    int ret;

    //video::ITexture *tex[10];// = driver->getTexture("data/objects/grass.png");
    c8 objectName[256];
    c8 textureName[256];
    vector3df rot(0.f,0.f,0.f);
    vector3df sca(1.f,1.f,1.f);
    vector3df box(0.f,0.f,0.f);
    vector3df ofs(0.f,0.f,0.f);

    dprintf(printf("Generate grass types: %s\n", name));

    f = fopen(name, "r");
    
    if (!f)
    {
        printf("grass type file unable to open: %s\n", name);
        return;       
    }

    while (true)
    {
        ret = fscanf(f, "%s\n%s\n", objectName, textureName);
        createObjectPool(objectName, "null",
                         smgr, driver, nWorld,
                         grass_pool_size, GRASS,
                         textureName,
                         rot, sca, box, ofs);
        if (ret < 2)
        {
            break;
        }
    }
    
    fclose(f);
}

void loadTreeTypes(const c8* treetype,
                   ISceneManager* smgr,
                   IVideoDriver* driver,
                   IrrlichtDevice* device,
                   NewtonWorld* nWorld
                   )
{
    FILE* f;
    int ret = 0;
    c8 meshName[256];
    c8 treetexture[256];
    c8 leaftexture[256];
    c8 billtexture[256];

    int type;
    vector3df rot(0.f,0.f,0.f);
    vector3df sca(1.f,1.f,1.f);
    vector3df box(0.f,0.f,0.f);
    vector3df ofs(0.f,0.f,0.f);
    
    dprintf(printf("Read tree types: %s\n", treetype));

    if (treeDesigns.size() > 0)
    {
        dprintf(printf("tree types has already been readed %d types, return.\n", treeDesigns.size()));
        return;
    }

    f = fopen(treetype, "r");
    
    if (!f)
    {
        printf("treetypes file unable to open: %s\n", treetype);
        return;       
    }

    //treeDesigns = new STreeDesign[treetypes_num];
    //memset(treeDesigns, 0, treetypes_num * sizeof(STreeDesign));
    //
    // Load tree designs
    //
    for ( s32 i=0; ; i++ )
    {
        ret = fscanf(f, "%s\n%s\n%s\n%s\n"
                        "rot: %f, %f, %f\n" \
                        "sca: %f, %f, %f\n" \
                        "box: %f, %f, %f\n" \
                        "ofs: %f, %f, %f\n",
                        meshName, treetexture, leaftexture, billtexture,
                        &rot.X, &rot.Y, &rot.Z,
                        &sca.X, &sca.Y, &sca.Z,
                        &box.X, &box.Y, &box.Z,
                        &ofs.X, &ofs.Y, &ofs.Z);
    
        if (ret < 16)
        {
            //printf("treetypes file unable to read elements: %s\n", treetype);
            //fclose(f);
            //return;
            break;
        }
        
        STreeDesign* treeDesign = new STreeDesign;
        treeDesign->Generator = new CTreeGenerator( smgr );
        io::IXMLReader* xml = device->getFileSystem()->createXMLReader( meshName );
        treeDesign->Generator->loadFromXML( xml );
        xml->drop();
        treeDesign->TreeTexture = driver->getTexture( treetexture );
        treeDesign->LeafTexture = driver->getTexture( leaftexture );
        treeDesign->BillTexture = driver->getTexture( billtexture );

        treeDesigns.push_back(treeDesign);

        assert(treeDesigns.size()==i+1);
        
        createObjectPool(meshName, "null",
                          smgr, driver, nWorld,
                          object_pool_size, TREE,
                          (char*)i,
                          rot, sca, box, ofs);
    }
    
    fclose(f);
}

void loadItinerTypes(const c8* name, ISceneManager* smgr, IVideoDriver* driver, NewtonWorld* nWorld)
{
    FILE* f;
    int ret = 0;
    c8 textureName[256];

    dprintf(printf("Read itiner types: %s\n", name));
    
    f = fopen(name, "r");
    
    if (!f)
    {
        printf("itiner type file unable to open: %s\n", name);
        return;       
    }

    for (int i = 0;/*numOfObjects < MAX_OBJECT_NUM*/;i++)
    {
        SObjectWrapper** objectWrappers_tmp;
        int poolId = 0;
        
        ret = fscanf(f, "%s\n", textureName);
        if (ret < 1)
        {
            // no more object
            //printf("|%s| |%s|\n", meshName, textureName);
            break;
        }
        SItinerNameTexture* itinerNT = new SItinerNameTexture;
        strcpy(itinerNT->name, textureName);
        itinerNT->texture = driver->getTexture(textureName);
        itinerTypes.push_back(itinerNT);
    }
    fclose(f);
}

int getPoolIdFromName(const char* name)
{
    int ind = 0;
    for (ind = 0; ind < objectPools.length();ind++)
    {
        if (strstr(objectPools[ind]->name, name))
        {
            //printf("pool exists, skip: %s, t: %s\n", name, textureName);
            break; // we found the pool return the id
        }
    }
    if (ind == objectPools.length()) ind = -1;
    return ind;
}

const char* getPoolNameFromId(int ind)
{
    if (ind < 0 || ind >=objectPools.length() ) return "null";
    return objectPools[ind]->name;
}

int getPoolsSize()
{
    return objectPools.length();
}

int getItinerIdFromName(const char* name)
{
    int ind = 0;
    for (ind = 0; ind < itinerTypes.size(); ind++)
    {
        if (strstr(itinerTypes[ind]->name, name))
        {
            //printf("pool exists, skip: %s, t: %s\n", name, textureName);
            break; // we found the pool return the id
        }
    }
    if (ind == itinerTypes.size()) ind = -1 - ITINER_POOLID_OFFSET;
    return ITINER_POOLID_OFFSET+ind;
}

const char* getItinerNameFromId(int ind)
{
    ind -= ITINER_POOLID_OFFSET;
    if (ind < 0 || ind >=itinerTypes.size() ) return "null";
    return itinerTypes[ind]->name;
}

int getItinerTypesSize()
{
    return itinerTypes.size();
}

video::ITexture* getItinerTextureFromId(int ind)
{
    ind -= ITINER_POOLID_OFFSET;
    if (ind < 0 || ind >=itinerTypes.size() ) return 0;
    return itinerTypes[ind]->texture;
}

void releaseItinerTypes()
{
    for (int i =  0; i < itinerTypes.size(); i++)
    {
        if (itinerTypes[i])
        {
            delete itinerTypes[i];
            itinerTypes[i] = 0;
        }
    }
    itinerTypes.clear();
}
