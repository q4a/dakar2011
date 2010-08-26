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

class BigTerrain;
class SmallTerrain;
class OffsetObject;

class CRoadType
{
public:
    CRoadType(IVideoDriver* p_driver);
    ~CRoadType();
    
    bool load(FILE* f);

    static bool loadRoadTypes(const char* name, IVideoDriver* p_driver);
    static core::array<CRoadType*> roadTypes;

    core::array<core::vector2df>& getSlicePoints() {return slicePoints;}
    core::array<int>& getSliceIndices() {return sliceIndices;}

//    void setTexture(video::ITexture* newTexture) {texture = newTexture;}
//    void setTextureName(const char* newTextureName);
//    video::ITexture* getTexture() {return texture;}
//    char* getTextureName() {return textureName;}

private:
    friend class CMyRoad;
    IVideoDriver* driver;

    core::array<core::vector2df> slicePoints;
    core::array<int> sliceIndices;

    video::ITexture* texture;
    float friction_multi;
    float tRate;
};

class CMyRoad
{
public:
    CMyRoad(ISceneManager* p_smgr, IVideoDriver* p_driver, NewtonWorld *p_nWorld);
    ~CMyRoad();
    
    static bool loadRoads(const char* name, core::array<CMyRoad*> &roadList,
                          ISceneManager* p_smgr, IVideoDriver* p_driver,
                          NewtonWorld *p_nWorld, BigTerrain* p_bigTerrain);
    static bool saveRoads(const char* name, core::array<CMyRoad*> &roadList);
    
    bool load(FILE* f, BigTerrain* p_bigTerrain);
    bool save(FILE* f);
    
    void addBasePoint(const core::vector3df& newPoint, BigTerrain* p_bigTerrain,
                      bool addToDensityMap, bool addToRoadMap, bool addToTextureMap);

    void setBasePoints(const core::array<core::vector3df> &newBasePoints);
    void setType(unsigned int newType);
    void setParent(int newParent) {parent = newParent;}

    core::array<core::vector3df>& getBasePoints() {return basePoints;}
    unsigned int getType() {return type;}
    int getParent() {return parent;}
    
    ISceneNode* generateRoadNode(SmallTerrain* p_smallTerrain, unsigned int regenerate, video::ITexture* p_shadowMap);
    
    float getFrictionMulti() {return roadType->friction_multi;}
    
private:
    core::array<core::vector3df> basePoints;

    ISceneNode* roadNode;
    NewtonBody* newtonBody;
    NewtonCollision* collision;
    
    ISceneManager* smgr;
    IVideoDriver* driver;
    NewtonWorld *nWorld;
    
    OffsetObject* offsetObject;

    unsigned int type;
    CRoadType* roadType;

    int parent;
};

#endif // __MYROAD_H__

