/****************************************************************
*                                                               *
*    Name: MyRoad.h                                             *
*                                                               *
*    Creator: Balazs Tuska                                      *
*                                                               *
*    Description:                                               *
*                                                               *
****************************************************************/

#ifndef __MYROAD_H__
#define __MYROAD_H__

#include "irrlicht.h"
// Irrlicht Namespaces
using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

#include "Newton.h"

class CMyRoad
{
public:
    CMyRoad(ISceneManager* p_smgr, IVideoDriver* p_driver, NewtonWorld *p_nWorld);
    ~CMyRoad();
    
    static bool loadRoads(const char* name, core::array<CMyRoad*> &roadList,
                          ISceneManager* p_smgr, IVideoDriver* p_driver, NewtonWorld *p_nWorld);
    static bool saveRoads(const char* name, core::array<CMyRoad*> &roadList);
    
    bool load(FILE* f);
    bool save(FILE* f);
    
    void addBasePoint(const core::vector3df& newPoint);

    void setBasePoints(const core::array<core::vector3df> &newBasePoints);
    void setSlicePoints(const core::array<core::vector2df> &newSlicePoints);
    void setSliceIndices(const core::array<int> &newSliceIndices);
    void setTexture(video::ITexture* newTexture) {texture = newTexture;}
    void setTextureName(const char* newTextureName);

    core::array<core::vector3df>& getBasePoints() {return basePoints;}
    core::array<core::vector2df>& getSlicePoints() {return slicePoints;}
    core::array<int>& getSliceIndices() {return sliceIndices;}
    video::ITexture* getTexture() {return texture;}
    char* getTextureName() {return textureName;}
    
    ISceneNode* generateRoadNode();
    
    float getFrictionMulti() {return friction_multi;}
    
private:
    core::array<core::vector3df> basePoints;

    core::array<core::vector2df> slicePoints;
    core::array<int> sliceIndices;
    
    ISceneNode* roadNode;
    NewtonBody* newtonBody;
    NewtonCollision* collision;
    
    float tRate;
    
    ISceneManager* smgr;
    IVideoDriver* driver;
    NewtonWorld *nWorld;
    
    video::ITexture* texture;
    char textureName[256];
    
    float friction_multi;
};

#endif // __MYROAD_H__

