/****************************************************************
*                                                               *
*    Name: objectWire.h                                         *
*                                                               *
*    Creator: Balazs Tuska                                      *
*                                                               *
*    Description:                                               *
*                                                               *
****************************************************************/

#ifndef __objectwire_h__
#define __objectwire_h__

#include "irrlicht.h"
// Irrlicht Namespaces
using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

#include <Newton.h>

#include "MyList.h"
#include "wrappers.h"
#include "settings.h"

#define TILE_SIZE obj_wire_size

class CObjectTile;

class CObjectWire
{
public:
    CObjectWire(const float p_sizeX, const float p_sizeY, const float p_offsetX = 0.f, const float p_offsetY = 0.f);
    ~CObjectWire();

    bool addObject(const core::vector3df &pos, SObjectWrapper* objectWrapper);
    //void addGrass(const core::vector3df &pos, SObjectWrapper* grassWrapper);
    
    void updatePos(const float posX, const float posY, float limit, bool force);
                     
private:
    //core::vector3df oldPos;
    int oldPosX;
    int oldPosY;
    int sizeX;
    int sizeY;
    int size;
    float offsetX;
    float offsetY;
    CObjectTile* tiles;
};

class CObjectTile
{
public:
    CObjectTile();
    ~CObjectTile();

    void addObject(SObjectWrapper* objectWrapper);
    //void addGrass(SObjectWrapper* grassWrapper);
    
    void setVisible(bool newVisible, bool force, bool &visible, core::array<SObjectWrapper*> &wrappers);
    void setVisibleObj(bool newVisible, bool force);
    //void setVisibleGra(bool newVisible, int gra_density, bool force);

private:
    core::array<SObjectWrapper*> objectWrappers;
    //CMyList<SObjectWrapper*> grassWrappers;
    
    bool objVisible;
    //bool graVisible;
};

#endif // __objectwire_h__
