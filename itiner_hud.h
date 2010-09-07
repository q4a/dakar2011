/****************************************************************
*                                                               *
*    Name: itiner_hud.h                                         *
*                                                               *
*    Creator: Balazs Tuska                                      *
*                                                               *
*    Description:                                               *
*                                                               *
****************************************************************/

#ifndef __ITINER_HUD_H__
#define __ITINER_HUD_H__

#include "irrlicht.h"
// Irrlicht Namespaces
using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

class ItinerHud
{
public:
    static void init(IrrlichtDevice* pdevice, IGUIEnvironment* env, core::dimension2d<u32> screenSize, int p_hudSize);
    static void addItiner(int itinerType, u32 timeoutSec = 5, bool renderRefresh = false);
    static void updateItiner(int p_tick);
    static void hide();
    
    static gui::IGUIImage* itinerHudImage;
    static u32 timeout;
    static IrrlichtDevice* device;
};

#endif // __ITINER_HUD_H__

