/****************************************************************
*                                                               *
*    Name: itiner_hud.cpp                                       *
*                                                               *
*    Creator: Balazs Tuska                                      *
*                                                               *
*    Description:                                               *
*                                                               *
****************************************************************/

#include "itiner_hud.h"
#include "settings.h"
#include "pools.h"

gui::IGUIImage* ItinerHud::itinerHudImage = 0;
u32 ItinerHud::timeout = 0;
IrrlichtDevice* ItinerHud::device = 0;

#ifdef IRRLICHT_SDK_15
void ItinerHud::init(IrrlichtDevice* pdevice, IGUIEnvironment* env, core::dimension2d<s32> screenSize, int p_hudSize)
#else
void ItinerHud::init(IrrlichtDevice* pdevice, IGUIEnvironment* env, core::dimension2d<u32> screenSize, int p_hudSize)
#endif
{
    const int hudSize = p_hudSize / 3;
    const int hudPositionX = screenSize.Width-(hudSize*2)-10;
    const int hudPositionY = screenSize.Height-10-p_hudSize-hudSize-10;
	itinerHudImage = env->addImage(core::rect<int>(hudPositionX, hudPositionY, hudPositionX+hudSize, hudPositionY+hudSize), 0, -1, L"itiner_hud");
	itinerHudImage->setScaleImage(true);
	itinerHudImage->setUseAlphaChannel(true);
	itinerHudImage->setVisible(false);
    timeout = 0;
    device = pdevice;
}

void ItinerHud::addItiner(int itinerType, u32 timeoutSec, bool renderRefresh)
{
    video::ITexture* texture = getItinerTextureFromId(itinerType);
    timeout = device->getTimer()->getTime()+timeoutSec*1000;
    if (texture)
        itinerHudImage->setImage(texture);
    itinerHudImage->setVisible(true);
    if (renderRefresh)
    {
       device->run();
       device->getVideoDriver()->beginScene(true, true, SColor(0,192,192,192));
       device->getGUIEnvironment()->drawAll();
       device->getVideoDriver()->endScene();
    }
}

void ItinerHud::updateItiner(int p_tick)
{
    //printf("updateText %u %u\n", timeout, device->getTimer()->getTime());
    if (timeout==0 || p_tick<timeout) return;
    itinerHudImage->setVisible(false);
    timeout = 0;
}

void ItinerHud::hide()
{
    itinerHudImage->setVisible(false);
    timeout = 0;
}
