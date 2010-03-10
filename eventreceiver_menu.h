/****************************************************************
*                                                               *
*    Name: eventreceiver_menu.h                                 *
*                                                               *
*    Creator: Balazs Tuska                                      *
*                                                               *
*    Description:                                               *
*       This file contains the event receiver for the menu.     *
*                                                               *
****************************************************************/

#ifndef __EVENTRECEIVER_MENU_H__
#define __EVENTRECEIVER_MENU_H__

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

#define SAVE_FILE "savegames/save1.txt"

class eventreceiver_menu : public eventreceiver
{
public:
	eventreceiver_menu(IrrlichtDevice* pdevice,
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
                    
    ~eventreceiver_menu();

	bool OnEvent(const SEvent& event);
	
	const SEvent::SJoystickEvent & GetJoystickState(void) const;
    
    void closeWindow(IGUIElement* cw, bool noact = false);

    void openMainWindow();
    
    void openSelectionWindow();
    
	void openOptionsWindow();

    void openHelpWindow();

    void openStateWindow();

    void render(/*irr::video::IVideoDriver* driver*/);
    void prerender(/*irr::video::IVideoDriver* driver*/);
    
    void refreshActiveElements();
    
    bool do_up();
    bool do_down();
    bool do_left(bool pov = false);
    bool do_right(bool pov = false);
    bool do_enter();
    bool do_esc();

    void releaseResources();
    
    void startEnter();
    
    void refreshStateWindow(bool leporget);
    
private:
    IGUIElement* window;
    IGUIElement* mainWindow;
    IGUIElement* selectionWindow;
    IGUIElement* optionsWindow;
    IGUIElement* helpWindow;
    IGUIElement* stateWindow;
    IGUIElement* mainWindowP;
    IGUIElement* selectionWindowP;
    IGUIElement* optionsWindowP;
    IGUIElement* helpWindowP;
    IGUIElement* stateWindowP;
	gui::IGUIStaticText* ov_limit_text;
	gui::IGUIStaticText* gravity_text;
	gui::IGUIStaticText* view_distance_text;
	gui::IGUIStaticText* transp_text;
	gui::IGUIEditBox* carfilename_text; // not used yet
	gui::IGUIEditBox* server_name_text;
	gui::IGUIEditBox* server_port_text;
	gui::IGUIEditBox* server_delay_text;
	gui::IGUIComboBox* carfilename_cbox;
	gui::IGUIComboBox* driverType_cbox;
	gui::IGUIComboBox* resolution_cbox;
	gui::IGUIComboBox* display_bits_cbox;
	gui::IGUIComboBox* LOD_distance_cbox;
	gui::IGUIComboBox* joys_cbox;
	gui::IGUIComboBox* joy_cbox;
	gui::IGUIComboBox* shadow_map_size_cbox;
	gui::IGUIComboBox* effects_cbox;
	gui::IGUIComboBox* texturedetail_cbox;
	gui::IGUIStaticText* texturedetail_text;
	gui::IGUIStaticText* obj_density_text;
	gui::IGUIStaticText* gra_density_text;
	gui::IGUIStaticText* joy_text;
	gui::IGUIStaticText* dead_zone_text;
    gui::IGUITabControl* tabControl;
    gui::IGUITabControl* stateTabControl;
    gui::IGUIStaticText* helpBox;
    gui::IGUIStaticText* stateBox;
    gui::IGUIStaticText* stateGlobalBox;
    gui::IGUIScrollBar* helpScroll;
    gui::IGUIScrollBar* stateScroll;
    gui::IGUIScrollBar* stateGlobalScroll;
    int helpBase;
    int stateBase;
    int stateGlobalBase;
#ifdef USE_MY_SOUNDENGINE
	CMySound* openSound;
	CMySound* clickSound;
	CMySound* closeSound;
#else
	irrklang::ISound* openSound;
	irrklang::ISound* clickSound;
	irrklang::ISound* closeSound;
#endif
	bool optionsWindowOpened;
	bool helpWindowOpened;
	bool stateWindowOpened;
	bool mainWindowOpened;
	bool selectionWindowOpened;
	int** joyHelper;

    bool firstJoyState;
	SEvent::SJoystickEvent centerJoystickState;
    
    CMyList<gui::IGUIElement*> activeElements;
    int activeElement;
    gui::IGUIElement* lastParent;
    gui::IGUIButton* saveSettingsButton;
    gui::IGUIButton* closeOptionsButton;
    
    bool skipHover;

    int pov_up_p;
    int pov_down_p;
    int pov_left_p;
    int pov_right_p;
    int joy_enter_p;
    int joy_esc_p;
    
    //gui::IGUIImage* bg_frame;
    video::ITexture* car_selector_rtt;
    NewtonRaceCar* car_to_draw;
	gui::IGUIStaticText* car_selector_text;
	
	int enterPressed;

    video::ITexture* mainmenu_texture;
    video::ITexture* selector_texture;

    video::ITexture* button_bg_next;
    video::ITexture* button_bg_resume;
    video::ITexture* button_bg_restart;
    video::ITexture* button_bg_continue;
    video::ITexture* button_bg_new;
    video::ITexture* button_bg_load;
    video::ITexture* button_bg_save;
    video::ITexture* button_bg_options;
    video::ITexture* button_bg_help;
    video::ITexture* button_bg_state;
    video::ITexture* button_bg_quit;
    video::ITexture* button_bg_sel_new;
    video::ITexture* button_bg_sel_next;
    video::ITexture* button_bg_sel_prev;
    video::ITexture* button_bg_sel_back;
};


#endif // __EVENTRECEIVER_MENU_H__
