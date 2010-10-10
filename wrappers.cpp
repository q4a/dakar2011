/****************************************************************
*                                                               *
*    Name: wrappers.cpp                                         *
*                                                               *
*    Creator: Balazs Tuska                                      *
*                                                               *
*    Description:                                               *
*       This file contains the wrappers. The terrain objects    *
*       are all wrappers. They handle the visibility  and       *
*       the position of the objects, roads, grass and trees.    *
*                                                               *
****************************************************************/

#include "wrappers.h"
#include "pools.h"
#include "BigTerrain.h"

#include <assert.h>

SObjectWrapper::~SObjectWrapper()
{
    if (data)
        putPoolElement(poolId, data);
    data = 0;
}

void SObjectWrapper::setVisible(bool pvisible)
{
    if (pvisible==visible
#ifdef USE_MESH_COMBINER
        && pvisible == false // with mesh combiner the visible object can be visible more times on update
#endif
       ) return;

    visible = pvisible;
    if (visible)
    {
        pos.Y = m_bigTerrain->getHeight(pos.X, pos.Z);
        data = getPoolElement(poolId, pos);
    }
    else
    {
        if (data)
            putPoolElement(poolId, data);
        data = 0;
    }
}

void SItinerPoint::setVisible(bool pvisible)
{
    if (pvisible==visible) return;

    visible = pvisible;
    if (visible)
    {
        m_bigTerrain->addActiveItinerPoint(this);
    }
    else
    {
        m_bigTerrain->removeActiveItinerPoint(this);
    }
}
