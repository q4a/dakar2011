/****************************************************************
*                                                               *
*    Name: eventreceiver_dummy.h                                *
*                                                               *
*    Creator: Balazs Tuska                                      *
*                                                               *
*    Description:                                               *
*       This file contains the dummy event receiver.            *
*                                                               *
****************************************************************/

#ifndef __EVENTRECEIVER_DUMMY_H__
#define __EVENTRECEIVER_DUMMY_H__

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

#include "eventreceiver.h"

#include "MyList.h"
#include "NewtonRaceCar.h"

class eventreceiver_dummy : public eventreceiver
{
public:
	eventreceiver_dummy(IrrlichtDevice* pdevice,
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
                    ) :
        eventreceiver(pdevice, skybox, skydome, pdriver, psmgr, penv, pnWorld, psoundEngine)
    {
    }
                    
    ~eventreceiver_dummy() {}

	bool OnEvent(const SEvent& event) {return true;}
	
    void render(/*irr::video::IVideoDriver* driver*/) {}
    void prerender(/*irr::video::IVideoDriver* driver*/) {}
    
    void releaseResources() {}
    
private:
};


#endif // __EVENTRECEIVER_DUMMY_H__
