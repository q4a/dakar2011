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
    if (pvisible==visible) return;

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

void SObjectWrapper::setFarVisible()
{
    assert(0 && "obsolete function setFarVisible()");
/*
    if (visible==true)
    {
        if (data)
        {
            if (getFarPoolElement(data))
            {
                setFarPoolElement(data);
            }
            else
            {
                putPoolElement(poolId, data);
                visible = false;
                data = 0;
            }
        }
        return;
    }
    data = getPoolElement(poolId, pos, true);
    if (data)
        visible = true;
*/
}

void SObjectWrapper::setNearVisible()
{
    assert(0 && "obselete function setNearVisible()");
/*
    if (visible==true)
    {
        if (data)
            setNearPoolElement(data);
        return;
    }
    visible = true;
    data = getPoolElement(poolId, pos);
*/
}

    
/*
    IAnimatedMesh *mesh = objectNode->getMesh();
    float vArray[9]; // vertex array (3*3 floats)
    int v1i, v2i, v3i;
    
    collision = NewtonCreateTreeCollision(nWorld);
    NewtonTreeCollisionBeginBuild(collision);

    for (int i=0; i<mesh->getMeshBufferCount(); i++)
    {
        IMeshBuffer* mb = mesh->getMeshBuffer(i);
        //printf("mesh grp%u\n", i);
		video::S3DVertex2TCoords* mb_vertices = (irr::video::S3DVertex2TCoords*)mb->getVertices();
		u16* mb_indices  = mb->getIndices();
        for (int j=0; j<mb->getIndexCount(); j+=3 )
        {
            v1i = mb_indices[j];
            v2i = mb_indices[j+1];
            v3i = mb_indices[j+2];

			vArray[0] = mb_vertices[v1i].Pos.X;
			vArray[1] = mb_vertices[v1i].Pos.Y;
			vArray[2] = mb_vertices[v1i].Pos.Z;
			vArray[3] = mb_vertices[v2i].Pos.X;
			vArray[4] = mb_vertices[v2i].Pos.Y;
			vArray[5] = mb_vertices[v2i].Pos.Z;
			vArray[6] = mb_vertices[v3i].Pos.X;
			vArray[7] = mb_vertices[v3i].Pos.Y;
			vArray[8] = mb_vertices[v3i].Pos.Z;

			NewtonTreeCollisionAddFace(collision, 3, (float*)vArray, 12, 1);
        }
    }
    NewtonTreeCollisionEndBuild(collision, 0);
*/

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

