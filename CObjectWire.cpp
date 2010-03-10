/****************************************************************
*                                                               *
*    Name: objectWire.cpp                                       *
*                                                               *
*    Creator: Balazs Tuska                                      *
*                                                               *
*    Description:                                               *
*                                                               *
****************************************************************/

#include "CObjectWire.h"

#define min(a, b) ((a) <= (b) ? (a) : (b))
#define max(a, b) ((a) >= (b) ? (a) : (b))

#ifdef MY_DEBUG
#define dprintf(x) x
#else
#define dprintf(x)
#endif

#ifdef MY_PDEBUG
#define pdprintf(x) x
#else
#define pdprintf(x)
#endif

static int visibleCount = 0;
static int allCount = 0;

CObjectWire::CObjectWire(const float p_sizeX, const float p_sizeY, const float p_offsetX, const float p_offsetY)
    : oldPosX(-20000), oldPosY(-20000),  // doUpgrade first, but maybe it will be forced anyway
      tiles(0), offsetX(p_offsetX), offsetY(p_offsetY)
{
    sizeX = (int)(p_sizeX / TILE_SIZE) + 1;
    sizeY = (int)(p_sizeY / TILE_SIZE) + 1;
    size = sizeX*sizeY;
    
    dprintf(printf("create new object wire sizex %d sizey %d offsetx %f offsety %f\n", sizeX, sizeY, offsetX, offsetY));
    
    tiles = new CObjectTile[size];
}

CObjectWire::~CObjectWire()
{
    if (tiles)
    {
        delete [] tiles;
        tiles = 0;
    }
}

void CObjectWire::updatePos(const float posX, const float posY, float limit, bool force)
{
    pdprintf(printf("objectwire updatePos() %f\n", TILE_SIZE);)
    const int newPosX = (int)((posX-offsetX) / TILE_SIZE);
    const int newPosY = (int)((posY-offsetY) / TILE_SIZE);
    
    if (newPosX == oldPosX && newPosY == oldPosY && !force) return;
    
    const int objLimit = (int)((limit + 1.f) / TILE_SIZE);
    //const int graLimit = (int)(((limit + 1.f) * 0.7f) / TILE_SIZE);
    
    oldPosX = newPosX;
    oldPosY = newPosY;
    
    visibleCount=0;
    allCount=0;
    
    pdprintf(printf("objectwire updatePos(): posx %d posy %d force %u olim %d limit %f limit/tilesize %f (%d)\n", oldPosX, oldPosY, force, objLimit, limit, limit / TILE_SIZE, (int)(limit / TILE_SIZE)));
    const int lim = objLimit + 1;
    const int downX = max(0, oldPosX - lim);
    const int upX = min(sizeX - 1, oldPosX + lim);
    const int downY = max(0, oldPosY - lim);
    const int upY = min(sizeY - 1, oldPosY + lim);
    
    pdprintf(printf("objectWire updatePos() lim %d x %d-%d y %d-%d\n", lim, downX, upX, downY, upY));
    for (int x = downX; x <= upX; x++)
    {
        for (int y = downY; y <= upY; y++)
        {
            if( abs(x-oldPosX) <= objLimit && abs(y-oldPosY) <= objLimit)
            {
                pdprintf(printf("t %d (%d)\n", x + (sizeX*y), size));
                tiles[x + (sizeX*y)].setVisibleObj(true, force);
            }
            else
            {
                pdprintf(printf("f %d (%d)\n", x + (sizeX*y), size));
                tiles[x + (sizeX*y)].setVisibleObj(false, force);
            }
            /*
            if( abs(x-oldPosX) <= graLimit && abs(y-oldPosY) <= graLimit)
            {
                dprintf(printf("t %d (%d)\n", x + (sizeX*y), size));
                tiles[x + (sizeX*y)].setVisibleGra(true, grass_density, force);
            }
            else
            {
                dprintf(printf("f %d (%d)\n", x + (sizeX*y), size));
                tiles[x + (sizeX*y)].setVisibleGra(false, grass_density, force);
            }
            */
        }
    }
    pdprintf(printf("objectWire updatePos() visibleCount %d / %d\n", visibleCount, allCount));
}

bool CObjectWire::addObject(const core::vector3df &pos, SObjectWrapper* objectWrapper)
{
    const int here = (int)((pos.X - offsetX)/TILE_SIZE) + (sizeX*(int)((pos.Z-offsetY)/TILE_SIZE));
    if (here >= size || here < 0) return false;
    tiles[here].addObject(objectWrapper);
    return true;
}

/*void CObjectWire::addGrass(const core::vector3df &pos, SObjectWrapper* grassWrapper)
{
    const int here = (int)(pos.X/TILE_SIZE) + (sizeX*(int)(pos.Z/TILE_SIZE));
    if (here >= size) return;
    tiles[here].addGrass(grassWrapper);
}
*/
CObjectTile::CObjectTile()
    : objectWrappers(), objVisible(false)
{
}

CObjectTile::~CObjectTile()
{
    objectWrappers.clear();
}


void CObjectTile::setVisible(bool newVisible, bool force, bool &visible, core::array<SObjectWrapper*> &wrappers)
{
    if (newVisible == visible && !force) return;
    
    visible = newVisible;
    
    pdprintf(printf("tile set visible %d len %d\n", visible, wrappers.size()));
    if (visible) visibleCount+=wrappers.size();
    allCount+=wrappers.size();
    
    for (int i = 0; i < wrappers.size(); i++)
    {
        //iterator->data->setNearVisible();
        wrappers[i]->setVisible(visible);
    }
       
}


void CObjectTile::setVisibleObj(bool newVisible, bool force)
{
    setVisible(newVisible, force, objVisible, objectWrappers);
}

//void CObjectTile::setVisibleGra(bool newVisible, int gra_density, bool force)
//{
//    setVisible(newVisible, gra_density, force, graVisible, grassWrappers);
//}

void CObjectTile::addObject(SObjectWrapper* objectWrapper)
{
    objectWrappers.push_back(objectWrapper);
}

//void CObjectTile::addGrass(SObjectWrapper* grassWrapper)
//{
//    grassWrappers.addLast(grassWrapper);
//}

        






