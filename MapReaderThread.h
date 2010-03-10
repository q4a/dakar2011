/****************************************************************
*                                                               *
*    Name: MapReaderThread.h                                    *
*                                                               *
*    Creator: Balazs Tuska                                      *
*                                                               *
*    Description:                                               *
*                                                               *
****************************************************************/

#ifndef __MAPREADERTHREAD_H__
#define __MAPREADERTHREAD_H__

#include "MyThread.h"
#include "MyList.h"
#include "irrlicht.h"
// Irrlicht Namespaces
using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

class BigTerrain;

struct SBTPos
{
    BigTerrain* m_bigTerrain;
    int new_x;
    int new_y;
    int obj_density;
};

class CMapReaderThread : public CMyThread
{
public:
    CMapReaderThread(irr::IrrlichtDevice* p_device);
    ~CMapReaderThread();
    
    virtual void run();
    
    void updateMap(BigTerrain* p_bigTerrain, int new_x, int new_y, int obj_density);

private:
    irr::IrrlichtDevice* device;
    bool running;
    CMyList<SBTPos*> updateList;
};

#endif // __MAPREADERTHREAD_H__
