/****************************************************************
*                                                               *
*    Name: MapReaderThread.cpp                                  *
*                                                               *
*    Creator: Balazs Tuska                                      *
*                                                               *
*    Description:                                               *
*                                                               *
****************************************************************/

#include "MapReaderThread.h"
#include "BigTerrain.h"
#include <stdio.h>

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

CMapReaderThread::CMapReaderThread(irr::IrrlichtDevice* p_device) :
    device(p_device),
    running(false),
    updateList()
{
    dprintf(printf("MapReader ctor\n");)
}
 
CMapReaderThread::~CMapReaderThread()
{
    dprintf(printf("MapReader dtor\n");)
    updateList.delList();
}
    
void CMapReaderThread::run()
{
    //dprintf(printf("MapReader run()\n");)
    //device->sleep(1500);
    running = true;
    SBTPos* btpos = updateList.removeFirst();
    while (btpos)
    {
        btpos->m_bigTerrain->updateMaps(btpos->new_x, btpos->new_y, btpos->obj_density);
        delete btpos;
        btpos = updateList.removeFirst();
    }
    running = false;
}

void CMapReaderThread::updateMap(BigTerrain* p_bigTerrain, int new_x, int new_y, int obj_density)
{
    SBTPos* btpos  = new SBTPos;
    btpos->m_bigTerrain = p_bigTerrain;
    btpos->new_x = new_x;
    btpos->new_y = new_y;
    btpos->obj_density = obj_density;
    updateList.push_back(btpos);
    if (!running || updateList.length()==1)
    {
        execute();
    }
}
