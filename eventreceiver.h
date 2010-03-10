/****************************************************************
*                                                               *
*    Name: eventreceiver.h                                      *
*                                                               *
*    Creator: Balazs Tuska                                      *
*                                                               *
*    Description:                                               *
*       This file contains the event receiver common part.      *
*                                                               *
****************************************************************/

#ifndef __EVENTRECEIVER_H__
#define __EVENTRECEIVER_H__

#include "irrlicht.h"
// Irrlicht Namespaces
using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

#ifdef USE_MY_SOUNDENGINE
 #include "mySound.h"
#else
 #include <irrKlang.h>
 using namespace irrklang;
#endif

#include <Newton.h>

class eventreceiver : public IEventReceiver
{
public:
	eventreceiver(IrrlichtDevice* pdevice,
                    scene::ISceneNode* skybox,
                    scene::ISceneNode* skydome,
                    IVideoDriver* pdriver,
                    ISceneManager* psmgr,
                    IGUIEnvironment* penv,
                    NewtonWorld *pnWorld,
#ifdef USE_MY_SOUNDENGINE
                    CMySoundEngine* psoundEngine
#else
                    irrklang::ISoundEngine* psoundEngine
#endif
                    )
       :other(0),
   		Skybox(skybox), Skydome(skydome),
		device(pdevice),
		driver(pdriver), smgr(psmgr), nWorld(pnWorld),
		soundEngine(psoundEngine), env(penv)
    {
        if (Skybox)
        	Skybox->setVisible(false);
        if (Skydome)
        	Skydome->setVisible(true);
    }

	virtual bool OnEvent(const SEvent& event) = 0;
    virtual void render(/*irr::video::IVideoDriver* driver*/) = 0;
    virtual void prerender(/*irr::video::IVideoDriver* driver*/) = 0;

#ifdef USE_MY_SOUNDENGINE
    void playSound(CMySound* sound)
    {
        if (sound)
        {
            sound->setPosition(soundEngine->getListenerPosition());
            sound->play();
        }
    }
#else
    void playSound(irrklang::ISound* sound)
    {
        if (sound)
            soundEngine->play2D(sound->getSoundSource());
    }
#endif

    void setOther(eventreceiver* rec) {other=rec;}
    
    void setOtherReceiver() {device->setEventReceiver(other);}
    
    virtual void releaseResources() = 0;
    
protected:
	scene::ISceneNode* Skybox;
	scene::ISceneNode* Skydome;
    IrrlichtDevice* device;
	IVideoDriver* driver;
	ISceneManager* smgr;
    IGUIEnvironment* env;
	NewtonWorld *nWorld;
#ifdef USE_MY_SOUNDENGINE
    CMySoundEngine* soundEngine;
#else
    irrklang::ISoundEngine* soundEngine;
#endif
	SEvent::SJoystickEvent JoystickState;

	eventreceiver* other;
};


#endif // __EVENTRECEIVER_H__
