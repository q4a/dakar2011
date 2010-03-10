/****************************************************************
*                                                               *
*    Name: eventreceiver_game.h                                 *
*                                                               *
*    Creator: Balazs Tuska                                      *
*                                                               *
*    Description:                                               *
*       This file contains the event receiver for the game.     *
*                                                               *
****************************************************************/

#ifndef __EVENTRECEIVER_GAME_H__
#define __EVENTRECEIVER_GAME_H__

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

class eventreceiver_game : public eventreceiver
{
public:
	eventreceiver_game(IrrlichtDevice* pdevice,
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
                    );
    ~eventreceiver_game();

	bool OnEvent(const SEvent& event);
	
	const SEvent::SJoystickEvent & GetJoystickState(void) const;
    
    void closeWindow(IGUIWindow* cw);

    void openMainWindow();
    
	void openOptionsWindow();

    void openHelpWindow();

    void render(/*irr::video::IVideoDriver* driver*/) {}
    void prerender(/*irr::video::IVideoDriver* driver*/) {}

    void releaseResources();
    
private:
};


#endif // __EVENTRECEIVER_GAME_H__
