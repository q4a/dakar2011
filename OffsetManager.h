/****************************************************************
*                                                               *
*    Name: OffsetManager.h                                      *
*                                                               *
*    Creator: Balazs Tuska                                      *
*                                                               *
*    Description:                                               *
*                                                               *
****************************************************************/

#ifndef __OFFSETMANAGER_H__
#define __OFFSETMANAGER_H__

#include "irrlicht.h"
// Irrlicht Namespaces
using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

#include "Newton.h"

// user callback
typedef void (*updatePositionByOffset)(void* userData, const irr::core::vector3df& newPos);

class OffsetObject;
typedef irr::core::list<OffsetObject*> offsetObjectList_t;

class OffsetObject
{
public:
    OffsetObject()
        : node(0), body(0), pos(), iterator(), dynamic(false), userData(0), callback(0)
    {
    }
    OffsetObject(irr::scene::ISceneNode* node)
        : node(node), body(0), pos(), iterator(), dynamic(false), userData(0), callback(0)
    {
        if (node)
        {
            pos = node->getPosition();
        }
    }
    OffsetObject(irr::scene::ISceneNode* node, NewtonBody* body)
        : node(node), body(body), pos(), iterator(), dynamic(false), userData(0), callback(0)
    {
        if (node)
        {
            pos = node->getPosition();
        }
    }
    OffsetObject(irr::scene::ISceneNode* node, bool dynamic)
        : node(node), body(0), pos(), iterator(), dynamic(dynamic), userData(0), callback(0)
    {
        /*
        if (node)
        {
            pos = node->getPosition();
        }
        */
    }
    OffsetObject(irr::scene::ISceneNode* node, NewtonBody* body, bool dynamic)
        : node(node), body(body), pos(), iterator(), dynamic(dynamic), userData(0), callback(0)
    {
        /*
        if (node)
        {
            pos = node->getPosition();
        }
        */
    }
    
    void setNode(irr::scene::ISceneNode* p_node)
    {
        node = p_node;
        if (node)
        {
            pos = node->getPosition();
        }
        else
        {
            pos = irr::core::vector3df();
        }
    }
    void setBody(NewtonBody* p_body)
    {
        if (node)
        {
            body = p_body;
        }
        else
        {
            body = 0;
        }
    }
    void setUserDataAndCallback(void* p_userData, updatePositionByOffset p_callback)
    {
        userData = p_userData;
        callback = p_callback;
    }

    ~OffsetObject();
    
    void update(const irr::core::vector3df& offset, const irr::core::vector3df& loffset);
    
private:
    friend class OffsetManager;
    irr::scene::ISceneNode* node;
    NewtonBody* body;
    irr::core::vector3df pos;
    offsetObjectList_t::Iterator iterator;
    bool dynamic;
    void* userData;
    updatePositionByOffset callback;
};

class OffsetManager
{
public:
    OffsetManager();
    bool update(const irr::core::vector3df& newPos, bool force = false);
    const irr::core::vector3df& getOffset() const {return offset;}
    void addObject(OffsetObject* object);
    void removeObject(OffsetObject* object);
    bool empty() {return objects.empty();}
    void reset();
    
private:
    offsetObjectList_t objects;
    irr::core::vector3df offset;
    irr::core::vector3di last;
};

#endif // __OFFSETMANAGER_H__
