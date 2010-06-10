/****************************************************************
*                                                               *
*    Name: OffsetManager.cpp                                    *
*                                                               *
*    Creator: Balazs Tuska                                      *
*                                                               *
*    Description:                                               *
*                                                               *
****************************************************************/

#include "OffsetManager.h"
#include "TerrainPool.h"
#include "gameplay.h"
#include <stdio.h>

OffsetObject::~OffsetObject()
{
    if (iterator != offsetObjectList_t::Iterator())
    {
        printf("offsetObject is in list but under deletion\n");
        *iterator = 0;
    }
}

void OffsetObject::update(const irr::core::vector3df& offset, const irr::core::vector3df& loffset)
{
    pdprintf(printf("update node %p: dyn %u (%f, %f)\n", node, dynamic, pos.X, pos.Z);)
    if (node)
    {
        //dprintf(printf("update %p: dyn %u (%f, %f)\n", node, dynamic, pos.X, pos.Z);)
        if (dynamic)
        {
            pos = node->getPosition();
            node->setPosition(pos-loffset);
        }
        else
        {
            node->setPosition(pos-offset);
        }
    }
    pdprintf(printf("update body %p: dyn %u (%f, %f)\n", body, dynamic, pos.X, pos.Z);)
    if (body)
    {
        float matrix[16]; 
        NewtonBodyGetMatrix (body, &matrix[0]); 
        if (dynamic)
        {
            matrix[12] = pos.X-loffset.X;
            matrix[13] = pos.Y-loffset.Y;
            matrix[14] = pos.Z-loffset.Z; //+ ofs.Z - box.Z * 0.5f;
        }
        else
        {
            matrix[12] = pos.X-offset.X;
            matrix[13] = pos.Y-offset.Y;
            matrix[14] = pos.Z-offset.Z; //+ ofs.Z - box.Z * 0.5f;
        }
        NewtonBodySetMatrix (body, &matrix[0]); 
    }
    pdprintf(printf("update callback %p userData %p\n", callback, userData);)
    if (callback)
    {
        callback(userData, node->getPosition());
    }
    pdprintf(printf("update end\n");)
}

OffsetManager::OffsetManager()
    : objects(), offset(), last()
{
}

void OffsetManager::addObject(OffsetObject* object)
{
    objects.push_front(object);
    object->iterator = objects.begin();
    object->update(offset, offset);
}

void OffsetManager::removeObject(OffsetObject* object)
{
    if (object->iterator != offsetObjectList_t::Iterator())
    {
        objects.erase(object->iterator);
        object->iterator = offsetObjectList_t::Iterator();
    }
}

bool OffsetManager::update(const irr::core::vector3df& newPos, bool force)
{
    const irr::core::vector3di new_((s32)(newPos.X/SMALLTERRAIN_SIZE), 0, (s32)(newPos.Z/SMALLTERRAIN_SIZE));
    
    if (last.X != new_.X || last.Z != new_.Z || force)
    {
        dprintf(printf("OffsetManager::update() last(%u, %u) new(%u, %u) force %u (%f, %f) -> (%f, %f) camera (%f, %f)\n",
            last.X, last.Z, new_.X, new_.Z, force, offset.X, offset.Z, newPos.X, newPos.Z,
            camera->getPosition().X, camera->getPosition().Z);)
        irr::core::vector3df loffset((float)(new_.X-last.X)*SMALLTERRAIN_SIZE, 0.f, (float)(new_.Z-last.Z)*SMALLTERRAIN_SIZE);
        last.X = new_.X;
        last.Z = new_.Z;
        offset.X = (float)last.X * SMALLTERRAIN_SIZE;
        offset.Z = (float)last.Z * SMALLTERRAIN_SIZE;
        pdprintf(printf("OffsetManager::update() new offset(%f, %f)\n", offset.X, offset.Z);)
        unsigned int updated = 0;
        for (offsetObjectList_t::Iterator it = objects.begin(); it != objects.end(); it++)
        {
            pdprintf(printf("%u - node %p\n", updated, (*it)->node);)
            (*it)->update(offset, loffset);
            updated++;
        }
        dprintf(printf("OffsetManager::update() return %u updated last(%u, %u) new(%u, %u) force %u (%f, %f) camera (%f, %f)\n",
            updated, last.X, last.Z, new_.X, new_.Z, force, offset.X, offset.Z,
            camera->getPosition().X, camera->getPosition().Z);)
        return true;
    }
    else
    {
        return false;
    }
}

void OffsetManager::reset()
{
    dprintf(printf("OffsetManager::reset() called\n");)
    unsigned int reseted = 0;
    for (offsetObjectList_t::Iterator it = objects.begin(); it != objects.end();)
    {
        delete (*it);
        it = objects.erase(it);
        reseted++;
    }
    dprintf(printf("OffsetManager::reset() %u was reseted, but it should be 0\n", reseted);)
    last = irr::core::vector3di();
    offset = irr::core::vector3df();
}

