/****************************************************************
*                                                               *
*    Name: wrappers.h                                           *
*                                                               *
*    Creator: Balazs Tuska                                      *
*                                                               *
*    Description:                                               *
*       This file contains the wrappers. The terrain objects    *
*       are all wrappers. They handle the visibility  and       *
*       the position of the objects, roads, grass and trees.    *
*                                                               *
****************************************************************/

#ifndef __wrappers_h__
#define __wrappers_h__

#include "irrlicht.h"
// Irrlicht Namespaces
using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

#include <Newton.h>
void addToShadowNodes(irr::scene::ISceneNode*);
void removeFromShadowNodes(irr::scene::ISceneNode*);

#include "pools.h"

class BigTerrain;

struct SRoadWrapper_old
{
public:
    SRoadWrapper_old(/*ISceneNode* node*/):roadNode(0/*node*/), /*refCount(0),*/ visCount(0) {}
    
    void incVisCount()
    {
        if (visCount==0)
        {
            //addToShadowNodes(roadNode);
            roadNode->setVisible(true);
        }
        visCount++;
    }

    void decVisCount()
    {
        visCount--;
        if (visCount==0)
        {
            //removeFromShadowNodes(roadNode);
            roadNode->setVisible(false);
        }
    }
    
    ISceneNode* roadNode;
    //int refCount;
    int visCount;
};

struct SObjectWrapper
{
public:
    //SObjectWrapper() : visible(false), data(0), m_bigTerrain(0) {}
    SObjectWrapper(BigTerrain* p_bigTerrain)
        : visible(false), data(0), m_bigTerrain(p_bigTerrain)
    {}
    virtual ~SObjectWrapper();
    
    virtual void setVisible(bool pvisible);
    virtual void setFarVisible();
    virtual void setNearVisible();
    
    void setPosition(const vector3df& ppos)
    {
        pos = ppos;
    }

    vector3df& getPosition()
    {
        return pos;
    }
    
    void setPool(int ppoolId)
    {
        poolId = ppoolId;
    }
    
    int getPool()
    {
        return poolId;
    }
    
    bool getVisible()
    {
        return visible;
    }
    
    
protected:
    bool visible;
    vector3df pos;
    int poolId;
    void* data;
    BigTerrain* m_bigTerrain;
    //vector3df rot;
    //vector3df sca;
};

struct SItinerPoint : public SObjectWrapper
{
public:
    SItinerPoint(BigTerrain* p_bigTerrain)
        : SObjectWrapper(p_bigTerrain)//, poolId(ITINER_POOLID_OFFSET)
    {
        poolId = ITINER_POOLID_OFFSET;
    }
    ~SItinerPoint() {}
    
    virtual void setVisible(bool pvisible);
    //virtual void setFarVisible();
    //virtual void setNearVisible();
    
private:
};

struct SAIPoint : public SObjectWrapper
{
public:
    SAIPoint(BigTerrain* p_bigTerrain)
        : SObjectWrapper(p_bigTerrain)//, poolId(ITINER_POOLID_OFFSET)
    {
    }
    ~SAIPoint() {}

    // comment it out if you don't need the visible stuff
    //virtual void setVisible(bool pvisible) {}
    //virtual void setFarVisible() {}
    //virtual void setNearVisible(){}
    
private:
};

#endif // __wrappers_h__
