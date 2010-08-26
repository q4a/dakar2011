/****************************************************************
*                                                               *
*    Name: message.h                                            *
*                                                               *
*    Creator: Balazs Tuska                                      *
*                                                               *
*    Description:                                               *
*       This file contains the code of the message text class   *
*       that displayed on the screen when an event happen.      *
*                                                               *
****************************************************************/

#ifndef __MESSAGETEXT_H__
#define __MESSAGETEXT_H__

#include "irrlicht.h"
// Irrlicht Namespaces
using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

class MessageText
{
public:
#ifdef IRRLICHT_SDK_15
    static void init(IrrlichtDevice* pdevice, IGUIEnvironment* env, core::dimension2d<s32> screenSize);  
#else
    static void init(IrrlichtDevice* pdevice, IGUIEnvironment* env, core::dimension2d<u32> screenSize);  
#endif
    static void addText(const wchar_t* text, u32 timeoutSec, bool renderRefresh = false, bool addToHistory = true);
    static void updateText(int p_tick);
    static void refresh();
    static void hide();
    
    static gui::IGUIStaticText* messageText;
    static u32 timeout;
    static IrrlichtDevice* device;
    static core::list<core::stringw> messageHistory;
};

#endif // __MESSAGETEXT_H__

