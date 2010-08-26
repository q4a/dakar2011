/****************************************************************
*                                                               *
*    Name: message.cpp                                          *
*                                                               *
*    Creator: Balazs Tuska                                      *
*                                                               *
*    Description:                                               *
*       This file contains the code of the message text class   *
*       that displayed on the screen when an event happen.      *
*                                                               *
****************************************************************/

#include "message.h"
#include "settings.h"

gui::IGUIStaticText* MessageText::messageText = 0;
u32 MessageText::timeout = 0;
IrrlichtDevice* MessageText::device = 0;
core::list<core::stringw> MessageText::messageHistory;

#ifdef IRRLICHT_SDK_15
void MessageText::init(IrrlichtDevice* pdevice, IGUIEnvironment* env, core::dimension2d<s32> screenSize)
#else
void MessageText::init(IrrlichtDevice* pdevice, IGUIEnvironment* env, core::dimension2d<u32> screenSize)
#endif
{
    messageText = env->addStaticText(L"messageText",
                        core::rect<int>(screenSize.Width/2-200,10/*screenSize.Height/2-50*/,screenSize.Width/2+200,110/*screenSize.Height/2+50*/),
                        message_bg, // border
                        true,  // wordwrap
                        0, -1,
                        message_bg); // show bg
    timeout = 0;
    device = pdevice;
}

void MessageText::addText(const wchar_t* text, u32 timeoutSec, bool renderRefresh, bool addToHistory)
{
    core::rect<int> messageRect = messageText->getRelativePosition();
    timeout = device->getTimer()->getTime()+timeoutSec*1000;
    if (text)
    {
        messageText->setText(text);
        if (addToHistory)
        {
            core::stringw ins(text);
            core::list<core::stringw> insList;
            u32 ret = ins.split(insList, L"\n");
            messageHistory.push_front(core::stringw(L"---------------------------"));
            if (ret == 0)
            {
                messageHistory.push_front(ins);
            }
            else
            {
                core::list<core::stringw>::ConstIterator it = insList.getLast();
                while (true)
                {
                    messageHistory.push_front(*it);
                    if (it == insList.begin()) break;
                    it--;
                }
            }
            
            
            /*
            core::stringw ins(text);
            ins.replace(L'\n', L'|');
            messageHistory.push_front(ins);
            */
        }
    }
    messageRect.LowerRightCorner.Y = messageRect.UpperLeftCorner.Y + messageText->getTextHeight();
    messageText->setRelativePosition(messageRect);
    messageText->setVisible(true);
    if (renderRefresh)
    {
       device->run();
       device->getVideoDriver()->beginScene(true, true, SColor(0,192,192,192));
       device->getGUIEnvironment()->drawAll();
       device->getVideoDriver()->endScene();
    }
}

void MessageText::updateText(int p_tick)
{
    //printf("updateText %u %u\n", timeout, device->getTimer()->getTime());
    if (timeout==0 || p_tick<timeout) return;
    messageText->setVisible(false);
    timeout = 0;
}

void MessageText::hide()
{
    messageText->setVisible(false);
    timeout = 0;
}

void MessageText::refresh()
{
    device->run();
    device->getVideoDriver()->beginScene(true, true, SColor(0,192,192,192));
    device->getGUIEnvironment()->drawAll();
    device->getVideoDriver()->endScene();
}

