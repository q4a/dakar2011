/****************************************************************
*                                                               *
*    Name: eventreceiver_menu.cpp                               *
*                                                               *
*    Creator: Balazs Tuska                                      *
*                                                               *
*    Description:                                               *
*       This file contains the event receiver for the menu.     *
*                                                               *
****************************************************************/

#include "eventreceiver_menu.h"
#include "irrlicht.h"
#include "settings.h"
#include "message.h"
#include "gameplay.h"
#include "pools.h"
#include "multiplayer.h"
#include "my_shaders.h"
#include <assert.h>

#ifdef __linux__
#include "linux_includes.h"
#endif

// for the joystick 0.05f
//#define DEAD_ZONE deadZone

// Define some values that we'll use to identify individual GUI controls.
enum
{
	GUI_ID_BACK_BUTTON = 101,
	GUI_ID_TRANSPARENCY_SCROLL_BAR,
	GUI_ID_OV_SCROLL_BAR,
	GUI_ID_VIEW_DISTANCE_SCROLL_BAR,
	GUI_ID_VIEW_OBJ_DENSITY_SCROLL_BAR,
	GUI_ID_VIEW_GRA_DENSITY_SCROLL_BAR,
	GUI_ID_GRAVITY_SCROLL_BAR,
	GUI_ID_DEAD_ZONE_SCROLL_BAR,
	GUI_ID_CHOOSE_CAR_BUTTON,
	GUI_ID_APPLY_CAR_BUTTON,
	GUI_ID_CONNECT_BUTTON,
	GUI_ID_CALIBRATE_BUTTON,
	GUI_ID_REINITIALIZE_BUTTON,
	GUI_ID_CAR_FILE_OPEN,
	GUI_ID_USE_SMOKES,
	GUI_ID_DRAW_HUD,
	GUI_ID_SHOW_NAMES,
	GUI_ID_FULL_SCREEN,
	GUI_ID_AUTO_RES,
	GUI_ID_ANTI_ALIASING,
	GUI_ID_VSYNC,
	GUI_ID_HIGH_PRECISION_FPU,
	GUI_ID_LIGHT,
	GUI_ID_SHADOWS,
	GUI_ID_USESHADERS,
	GUI_ID_USECGSHADERS,
	GUI_ID_STENCIL_SHADOWS,
	GUI_ID_USE_SCREEN_RTT,
	GUI_ID_USE_DEPTH_RTT,
	GUI_ID_SHIT_ATI,
	GUI_ID_FLIP_VERT,
	GUI_ID_TRACE_NET,
	GUI_ID_STEER_LINEAR,
	GUI_ID_GEAR_TYPE_AUTO,
	GUI_ID_SHOW_COMPASS_ARROW,
	GUI_ID_NEW_BUTTON,
	GUI_ID_SEL_NEW_BUTTON,
	GUI_ID_SEL_NEXT_BUTTON,
	GUI_ID_SEL_PREV_BUTTON,
	GUI_ID_SEL_BACK_BUTTON,
	GUI_ID_RESUME_BUTTON,
	GUI_ID_RESTART_BUTTON,
	GUI_ID_NEXT_BUTTON,
	GUI_ID_LOAD_BUTTON,
	GUI_ID_SAVE_BUTTON,
	GUI_ID_SAVESETTINGS_BUTTON,
	GUI_ID_OPTIONS_BUTTON,
	GUI_ID_HELP_BUTTON,
	GUI_ID_STATE_BUTTON,
	GUI_ID_EXIT_BUTTON,
	GUI_ID_LEPORGET_BUTTON,
	GUI_ID_DRIVERTYPE_CBOX,
	GUI_ID_RESOLUTION_CBOX,
	GUI_ID_DISPLAY_BITS_CBOX,
	GUI_ID_LOD_DISTANCE_CBOX,
	GUI_ID_JOYS_CBOX,
	GUI_ID_JOY_CBOX,
	GUI_ID_SHADOW_MAP_SIZE_CBOX,
	GUI_ID_EFFECTS_CBOX,
	GUI_ID_TEXTUREDETAIL_CBOX,
    GUI_ID_SERVER_NAME_EBOX,
    GUI_ID_SERVER_PORT_EBOX,
    GUI_ID_SERVER_DELAY_EBOX,
	GUI_ID_OPTIONS_WINDOW,
	GUI_ID_HELP_WINDOW,
	GUI_ID_STATE_WINDOW,
	GUI_ID_MAIN_WINDOW,
	GUI_ID_HELP_BOX,
	GUI_ID_HELP_SCROLL,
	GUI_ID_HELP_TAB,
	GUI_ID_STATE_BOX,
	GUI_ID_STATE_SCROLL,
	GUI_ID_STATE_TAB,
	GUI_ID_STATEGLOBAL_BOX,
	GUI_ID_STATEGLOBAL_SCROLL,
	GUI_ID_STATEGLOBAL_TAB,
	GUI_ID_STATE_TABCONTROL
};

#define JOY_HELPER_NUM 24

extern int joy_look_l;
extern int joy_look_r;
extern int joy_gu;
extern int joy_gd;
extern int joy_reset_car_p;
extern int joy_change_view_p;
extern int joy_change_light_p;
extern int joy_show_compass_p;
extern int joy_repair_car_p;
extern int joy_menu_p;


eventreceiver_menu::eventreceiver_menu(IrrlichtDevice* pdevice,
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
        eventreceiver(pdevice, skybox, skydome, pdriver, psmgr, penv, pnWorld, psoundEngine),
		optionsWindowOpened(false), mainWindowOpened(false), helpWindowOpened(false), stateWindowOpened(false),
        selectionWindowOpened(false),
        window(0), joys_cbox(0), firstJoyState(false), activeElements(), activeElement(0),
        tabControl(0), lastParent(0), saveSettingsButton(0), closeOptionsButton(0),
        skipHover(true), car_selector_rtt(0), car_to_draw(0), joyHelper(0),
        enterPressed(-1),
        helpBox(0), helpScroll(0), helpBase(40),
        stateBox(0), stateScroll(0), stateBase(40),
        stateGlobalBox(0), stateGlobalScroll(0), stateGlobalBase(40)
{
    openSound = soundEngine->play2D("data/menu_sounds/open_window.wav", false, true, true);
    if (openSound)
#ifdef USE_MY_SOUNDENGINE
        openSound->setVolume(0.2f);
#else
        openSound->getSoundSource()->setDefaultVolume(0.2f);
#endif
    clickSound = soundEngine->play2D("data/menu_sounds/click.wav", false, true, true);
    if (clickSound)
#ifdef USE_MY_SOUNDENGINE
        clickSound->setVolume(0.2f);
#else
        clickSound->getSoundSource()->setDefaultVolume(0.2f);
#endif
    closeSound = soundEngine->play2D("data/menu_sounds/close_window.wav", false, true, true);
    if (closeSound)
#ifdef USE_MY_SOUNDENGINE
        closeSound->setVolume(0.2f);
#else
        closeSound->getSoundSource()->setDefaultVolume(0.2f);
#endif

    joyHelper = new int*[JOY_HELPER_NUM];
        
    joyHelper[0] = &joy_axis_accel;
    joyHelper[1] = &joy_axis_steer;
    joyHelper[2] = &joy_axis_clutch;
    joyHelper[3] = &joy_accel;
    joyHelper[4] = &joy_brake;
    joyHelper[5] = &joy_handbrake;
    joyHelper[6] = &joy_left;
    joyHelper[7] = &joy_right;
    joyHelper[8] = &joy_look_left;
    joyHelper[9] = &joy_look_right;
    joyHelper[10] = &joy_reset_car;
    joyHelper[11] = &joy_change_view;
    joyHelper[12] = &joy_change_light;
    joyHelper[13] = &joy_show_compass;
    joyHelper[14] = &joy_repair_car;
    joyHelper[15] = &joy_gear_up;
    joyHelper[16] = &joy_gear_down;
    joyHelper[17] = &joy_gear1;
    joyHelper[18] = &joy_gear2;
    joyHelper[19] = &joy_gear3;
    joyHelper[20] = &joy_gear4;
    joyHelper[21] = &joy_gear5;
    joyHelper[22] = &joy_gear6;
    joyHelper[23] = &joy_menu;

    joy_text = 0;
    joy_cbox = 0;

    pov_up_p = 0;
    pov_down_p = 0;
    pov_left_p = 0;
    pov_right_p = 0;
    joy_enter_p = 0;
    joy_esc_p = 0;

  	bool tempTexFlagMipMaps = driver->getTextureCreationFlag(ETCF_CREATE_MIP_MAPS);
    bool tempTexFlag32 = driver->getTextureCreationFlag(ETCF_ALWAYS_32_BIT);
    car_selector_rtt = driver->addRenderTargetTexture(
              //!driver->getVendorInfo().equals_ignore_case("NVIDIA Corporation") ? dimension2du(512, 512) :
#ifdef IRRLICHT_SDK_15
              dimension2d<s32>(1024, 1024));
#else
              dimension2d<u32>(1024, 1024));
#endif
    driver->setTextureCreationFlag(ETCF_CREATE_MIP_MAPS, tempTexFlagMipMaps);
    driver->setTextureCreationFlag(ETCF_ALWAYS_32_BIT, tempTexFlag32);
    
    if (screenSize.Width > 1280)
    {
        mainmenu_texture = driver->getTexture("data/menu_textures/bg_frame_main.png");
        selector_texture = driver->getTexture("data/menu_textures/bg_frame_select.png");
    }
    else
    {
        mainmenu_texture = driver->getTexture("data/menu_textures/bg_frame_main_1280.png");
        selector_texture = driver->getTexture("data/menu_textures/bg_frame_select_1280.png");
    }
    
    button_bg_next = driver->getTexture("data/menu_textures/button_bg_next.png");
    button_bg_resume = driver->getTexture("data/menu_textures/button_bg_resume.png");
    button_bg_restart = driver->getTexture("data/menu_textures/button_bg_restart.png");
    button_bg_continue = driver->getTexture("data/menu_textures/button_bg_continue.png");
    button_bg_new = driver->getTexture("data/menu_textures/button_bg_new.png");
    button_bg_load = driver->getTexture("data/menu_textures/button_bg_load.png");
    button_bg_save = driver->getTexture("data/menu_textures/button_bg_save.png");
    button_bg_options = driver->getTexture("data/menu_textures/button_bg_options.png");
    button_bg_help = driver->getTexture("data/menu_textures/button_bg_help.png");
    button_bg_state = driver->getTexture("data/menu_textures/button_bg_standings.png");
    button_bg_quit = driver->getTexture("data/menu_textures/button_bg_quit.png");
    button_bg_sel_new = driver->getTexture("data/menu_textures/button_bg_sel_new.png");
    button_bg_sel_next = driver->getTexture("data/menu_textures/button_bg_sel_next.png");
    button_bg_sel_prev = driver->getTexture("data/menu_textures/button_bg_sel_prev.png");
    button_bg_sel_back = driver->getTexture("data/menu_textures/button_bg_sel_back.png");
}

eventreceiver_menu::~eventreceiver_menu()
{
    releaseResources();    
}

void stepNext(IGUIComboBox* cbox, bool round = true)
{
    s32 ix = cbox->getSelected();
    if (ix + 1 >= cbox->getItemCount())
    {
        if (round)
            cbox->setSelected(0);
    }
    else
        cbox->setSelected(ix+1);
}

void stepBack(IGUIComboBox* cbox, bool round = true)
{
    s32 ix = cbox->getSelected();
    if (ix - 1 < 0)
    {
        if (round)
            cbox->setSelected(cbox->getItemCount()-1);
    }
    else
        cbox->setSelected(ix-1);
}

bool eventreceiver_menu::do_up()
{
    if (activeElement>=0 && activeElement<activeElements.length())
    {
        switch (activeElements[activeElement]->getType())
        {
            //case EGUIET_COMBO_BOX:
            //    stepNext((IGUIComboBox*)activeElements[activeElement], false);
            default:
            {
                activeElement--;
                if (activeElement<0) activeElement = activeElements.length()-1;
                if (activeElement<0) activeElement = 0;
                env->setFocus(activeElements[activeElement]);
				return true;
                break;
            }
        }
    }
    return false;
}

bool eventreceiver_menu::do_down()
{
    if (activeElement>=0 && activeElement<activeElements.length())
    {
        switch (activeElements[activeElement]->getType())
        {
            //case EGUIET_COMBO_BOX:
            //    stepBack((IGUIComboBox*)activeElements[activeElement],false);
            default:
            {
                activeElement++;
                if (activeElement>=activeElements.length()) activeElement = 0;
                env->setFocus(activeElements[activeElement]);
				return true;
                break;
            }
        }
    }
    return false;
}

bool eventreceiver_menu::do_left(bool pov)
{
    if (activeElement>=0 && activeElement<activeElements.length())
    {
        switch ( activeElements[activeElement]->getType())
        {
            case EGUIET_TAB_CONTROL:
            {
                s32 ix = ((IGUITabControl*)activeElements[activeElement])->getActiveTab();
                ix--;
                if (ix < 0) ix = ((IGUITabControl*)activeElements[activeElement])->getTabCount()-1;
                if (ix < 0) ix = 0;
                ((IGUITabControl*)activeElements[activeElement])->setActiveTab(ix);
                refreshActiveElements();
                break;
            }
            case EGUIET_COMBO_BOX:
                SEvent sevent;
                sevent.EventType = EET_GUI_EVENT;
                sevent.GUIEvent.Caller = activeElements[activeElement];
                sevent.GUIEvent.Element = activeElements[activeElement];
                sevent.GUIEvent.EventType = EGET_COMBO_BOX_CHANGED;

                stepBack((IGUIComboBox*)activeElements[activeElement]);

                return OnEvent(sevent);
                break;

            case EGUIET_SCROLL_BAR:
            {
                if (pov)
                {
                    SEvent sevent;
                    sevent.EventType = EET_GUI_EVENT;
                    sevent.GUIEvent.Caller = activeElements[activeElement];
                    sevent.GUIEvent.Element = activeElements[activeElement];
                    sevent.GUIEvent.EventType = EGET_SCROLL_BAR_CHANGED;
                
                    s32 smallStep = ((IGUIScrollBar*)activeElements[activeElement])->getSmallStep();
                    
                    if (((IGUIScrollBar*)activeElements[activeElement])->getPos() - smallStep > 0)
                    {
                        ((IGUIScrollBar*)activeElements[activeElement])->setPos(
                                    ((IGUIScrollBar*)activeElements[activeElement])->getPos() - smallStep);
                    }
                    else
                    {
                        ((IGUIScrollBar*)activeElements[activeElement])->setPos(0);
                    }
                    
                    env->postEventFromUser(sevent);
                    return OnEvent(sevent);
                }
                else
                    return false;
                break;
            }

            default:
                return false;
                break;
        }
    }
	return true;
}

bool eventreceiver_menu::do_right(bool pov)
{
    if (activeElement>=0 && activeElement<activeElements.length())
    {
        switch ( activeElements[activeElement]->getType())
        {
            case EGUIET_TAB_CONTROL:
            {
                s32 ix = ((IGUITabControl*)activeElements[activeElement])->getActiveTab();
                ix++;
                if (ix >= ((IGUITabControl*)activeElements[activeElement])->getTabCount()) ix = 0;
                ((IGUITabControl*)activeElements[activeElement])->setActiveTab(ix);
                refreshActiveElements();
                break;
            }
            case EGUIET_COMBO_BOX:
                SEvent sevent;
                sevent.EventType = EET_GUI_EVENT;
                sevent.GUIEvent.Caller = activeElements[activeElement];
                sevent.GUIEvent.Element = activeElements[activeElement];
                sevent.GUIEvent.EventType = EGET_COMBO_BOX_CHANGED;

                stepNext((IGUIComboBox*)activeElements[activeElement]);

                return OnEvent(sevent);
                break;

            case EGUIET_SCROLL_BAR:
            {
                if (pov)
                {
                    SEvent sevent;
                    sevent.EventType = EET_GUI_EVENT;
                    sevent.GUIEvent.Caller = activeElements[activeElement];
                    sevent.GUIEvent.Element = activeElements[activeElement];
                    sevent.GUIEvent.EventType = EGET_SCROLL_BAR_CHANGED;
                    
                    s32 smallStep = ((IGUIScrollBar*)activeElements[activeElement])->getSmallStep();
                    
                    if (((IGUIScrollBar*)activeElements[activeElement])->getPos() + smallStep <= 
                        ((IGUIScrollBar*)activeElements[activeElement])->getMax())
                    {
                        ((IGUIScrollBar*)activeElements[activeElement])->setPos(
                                    ((IGUIScrollBar*)activeElements[activeElement])->getPos() + smallStep);
                    }
                    else
                    {
                        ((IGUIScrollBar*)activeElements[activeElement])->setPos(((IGUIScrollBar*)activeElements[activeElement])->getMax());
                    }
                    
                    env->postEventFromUser(sevent);
                    return OnEvent(sevent);
                }
                else
                    return false;
                break;
            }

            default:
                return false;
                break;
        }
    }
	return true;
}

bool eventreceiver_menu::do_enter()
{
    if (activeElement>=0 && activeElement<activeElements.length())
    {
        SEvent sevent;
        sevent.EventType = EET_GUI_EVENT;
        sevent.GUIEvent.Caller = activeElements[activeElement];
        sevent.GUIEvent.Element = activeElements[activeElement];
        
        switch ( activeElements[activeElement]->getType())
        {
            case EGUIET_BUTTON:
                sevent.GUIEvent.EventType = EGET_BUTTON_CLICKED;
                break;
            case EGUIET_CHECK_BOX:
                sevent.GUIEvent.EventType = EGET_CHECKBOX_CHANGED;
                ((IGUICheckBox*)activeElements[activeElement])->setChecked(!((IGUICheckBox*)activeElements[activeElement])->isChecked());
                env->postEventFromUser(sevent);
                break;
            case EGUIET_COMBO_BOX:
                sevent.GUIEvent.EventType = EGET_COMBO_BOX_CHANGED;
                break;
            case EGUIET_EDIT_BOX:
                sevent.GUIEvent.EventType = EGET_EDITBOX_ENTER;
                break;
            //case EGUIET_SCROLL_BAR:
            //    return true;
            //    break;
            default:
                return false;
                //sevent.EventType = EGET_BUTTON_CLICKED;
        }
        
        //env->postEventFromUser(sevent);
        return OnEvent(sevent);
    }
    return true;
}

bool eventreceiver_menu::do_esc()
{
    if (car || window != mainWindow)
        closeWindow(window);
//    else
//        window->setVisible(!window->isVisible());
	return true;
}

void eventreceiver_menu::startEnter()
{
    //printf("start enter, state was: %d\n", enterPressed);
    if (enterPressed==-1)
    {
        enterPressed = 0;
    }
    else if (enterPressed==-2)
        enterPressed = -3;
}

bool eventreceiver_menu::OnEvent(const SEvent& event)
{
    //printf("event: %d\n", event.EventType);
	if (event.EventType == irr::EET_KEY_INPUT_EVENT && event.KeyInput.PressedDown)
	{
		switch (event.KeyInput.Key)
		{
            case irr::KEY_RETURN:
                //printf("enter pressed, state was: %d\n", enterPressed);
                if (enterPressed<0)
                {
                    if (enterPressed == -1)
                        enterPressed = -2;
                }
                else
                    enterPressed = 1;
                break;
            case irr::KEY_UP:
            case irr::KEY_DOWN:
                if (activeElement>=0 && activeElement<activeElements.length())
                {
                    switch (activeElements[activeElement]->getType())
                    {
                        case EGUIET_COMBO_BOX:
                        case EGUIET_SCROLL_BAR:
                        return true;
                    }
                }
                break;
            case irr::KEY_TAB:
                if (event.KeyInput.Control)
                    return true;
                break;
        }
        return false;
    } 
    else
	if (event.EventType == irr::EET_KEY_INPUT_EVENT && !event.KeyInput.PressedDown)
	{
        if (event.KeyInput.Key != irr::KEY_ESCAPE)
            device->getCursorControl()->setVisible(false);
            
		switch (event.KeyInput.Key)
		{
        case irr::KEY_ESCAPE:
            {
                return do_esc();
            }
        case irr::KEY_UP:
            {
                return do_up();
            }
        case irr::KEY_DOWN:
            {
                return do_down();
            }
        case irr::KEY_LEFT:
            {
                return do_left();
            }
        case irr::KEY_RIGHT:
            {
                return do_right();
            }
        case irr::KEY_RETURN:
            {
                //printf("enter released, state was: %d\n", enterPressed);
                if (enterPressed>0)
                {
                    enterPressed = 0;
                    return do_enter();
                }
                else if (enterPressed == -3)
                    enterPressed = 0;
                else
                    enterPressed = -1;
                return true;
            }
        case irr::KEY_DELETE:
            {
                if (joy_cbox->getSelected() > 0)
                {
                    core::stringw str = L"wait...";
                    *joyHelper[joy_cbox->getSelected()-1] = -1;
                    joy_text->setText(str.c_str());
	            }
    			return true;
                break;
            }
        case irr::KEY_TAB:
            {
                if (event.KeyInput.Control)
                {
                    if (window == optionsWindow && tabControl)
                    {
                        if (event.KeyInput.Shift)
                        {
                            s32 ix = tabControl->getActiveTab();
                            ix--;
                            if (ix < 0) ix = tabControl->getTabCount()-1;
                            if (ix < 0) ix = 0;
                            tabControl->setActiveTab(ix);
                            refreshActiveElements();
                        }
                        else
                        {
                            s32 ix = tabControl->getActiveTab();
                            ix++;
                            if (ix >= tabControl->getTabCount()) ix = 0;
                            tabControl->setActiveTab(ix);
                            refreshActiveElements();
                        }
            			return true;
                        break;
                    }
                    else
                    if (window == stateWindow && stateTabControl)
                    {
                        if (event.KeyInput.Shift)
                        {
                            s32 ix = stateTabControl->getActiveTab();
                            ix--;
                            if (ix < 0) ix = stateTabControl->getTabCount()-1;
                            if (ix < 0) ix = 0;
                            stateTabControl->setActiveTab(ix);
                            refreshActiveElements();
                        }
                        else
                        {
                            s32 ix = stateTabControl->getActiveTab();
                            ix++;
                            if (ix >= stateTabControl->getTabCount()) ix = 0;
                            stateTabControl->setActiveTab(ix);
                            refreshActiveElements();
                        }
                        if (stateTabControl->getActiveTab()==0)
                            env->setFocus(stateScroll);
                        else
                            env->setFocus(stateGlobalScroll);
            			return true;
                        break;
                    }
                }
            }
		default:
			break;
		}
	} else
/*
	if (event.EventType == irr::EET_KEY_INPUT_EVENT && event.KeyInput.PressedDown)
	{
		switch (event.KeyInput.Key)
		{
		default:
			break;
        }
    }
*/
/*        
    if (event.EventType == irr::EET_MOUSE_INPUT_EVENT && event.MouseInput.Event == EMIE_LMOUSE_PRESSED_DOWN)
    {
    }
*/
	// The state of each connected joystick is sent to us
	// once every run() of the Irrlicht device.  Store the
	// state of the first joystick, ignoring other joysticks.
	// This is currently only supported on Windows and Linux.
/*
	if (event.EventType == irr::EET_JOYSTICK_INPUT_EVENT
		&& event.JoystickEvent.Joystick == 0)
	{
        //printf("joyevent\n");
		JoystickState = event.JoystickEvent;
	}
*/

	if (event.EventType == EET_GUI_EVENT)
	{
		s32 id = event.GUIEvent.Caller->getID();
		//printf ("gui event type %d id %d\n", event.GUIEvent.EventType, id);
		//IGUIEnvironment* env = device->getGUIEnvironment();

		switch(event.GUIEvent.EventType)
		{
			case EGET_SCROLL_BAR_CHANGED:
				switch (id)
				{
    				case GUI_ID_TRANSPARENCY_SCROLL_BAR:
    				{
    					s32 pos = ((IGUIScrollBar*)event.GUIEvent.Caller)->getPos();

    					for (u32 i=0; i<EGDC_COUNT ; ++i)
    					{
    						SColor col = env->getSkin()->getColor((EGUI_DEFAULT_COLOR)i);
    						col.setAlpha(pos);
    						env->getSkin()->setColor((EGUI_DEFAULT_COLOR)i, col);
    					}

                    	core::stringw str = L" ";
                    	str += env->getSkin()->getColor(EGDC_WINDOW).getAlpha();
                    	transp_text->setText(str.c_str());
    					
    				    break;	
    				}
    				case GUI_ID_GRAVITY_SCROLL_BAR:
    				{
    					s32 pos = ((IGUIScrollBar*)event.GUIEvent.Caller)->getPos();
    					
    					pos -= 100;
    					
    					gravity = (float)pos;

                    	core::stringw str = L" ";
                    	str += (int)gravity;
                    	gravity_text->setText(str.c_str());
    					
    				    break;	
    				}
    				case GUI_ID_OV_SCROLL_BAR:
    				{
        				s32 pos = ((IGUIScrollBar*)event.GUIEvent.Caller)->getPos();
                       	core::stringw str = L" ";

        				objectVisibilityLimit = (float)pos;
                       	str += (int)objectVisibilityLimit;
                       	ov_limit_text->setText(str.c_str());
                        if (bigTerrain)
                        {
        					dprintf(printf("set OV to %d\n", pos));
        					bigTerrain->setOVLimit(objectVisibilityLimit);
                            //bigTerrain->updatePos(camera->getPosition().X, camera->getPosition().Z, density_objects, density_grasses, true);
                        }
    				    break;	
    				}
    				case GUI_ID_VIEW_DISTANCE_SCROLL_BAR:
    				{
    					s32 pos = ((IGUIScrollBar*)event.GUIEvent.Caller)->getPos();
    					dprintf(printf("set view distance to %d\n", pos));
    					farValue = (float)pos;
    					camera->setFarValue(farValue);

                    	core::stringw str = L" ";
                    	str += (int)farValue;
                    	view_distance_text->setText(str.c_str());
    				    break;	
    				}
    				case GUI_ID_VIEW_OBJ_DENSITY_SCROLL_BAR:
    				{
        				density_objects = ((IGUIScrollBar*)event.GUIEvent.Caller)->getPos();
                        if (bigTerrain)
                        {
                            //bigTerrain->updatePos(camera->getPosition().X, camera->getPosition().Z, density_objects, density_grasses, true);
                        }
                    	core::stringw str = L" ";
                    	str += density_objects;
                    	obj_density_text->setText(str.c_str());
    				    break;	
    				}
    				case GUI_ID_VIEW_GRA_DENSITY_SCROLL_BAR:
    				{
       					density_grasses = ((IGUIScrollBar*)event.GUIEvent.Caller)->getPos();
                        if (bigTerrain)
                        {
                            //bigTerrain->updatePos(camera->getPosition().X, camera->getPosition().Z, density_objects, density_grasses, true);
                        }
                    	core::stringw str = L" ";
                    	str += density_grasses;
                    	gra_density_text->setText(str.c_str());
    				    break;	
    				}
    				case GUI_ID_DEAD_ZONE_SCROLL_BAR:
    				{
    					s32 pos = ((IGUIScrollBar*)event.GUIEvent.Caller)->getPos();
    					
    					deadZone = ((float)pos)/100.f;

                    	core::stringw str = L"0.";
                        if (deadZone<0.1f) str+=L"0";
                    	str += pos;
                    	dead_zone_text->setText(str.c_str());
    					
    				    break;	
    				}
    				case GUI_ID_HELP_SCROLL:
    				{
                        core::rect<s32> rect = helpBox->getRelativePosition();
                        rect.UpperLeftCorner.Y = helpBase - helpScroll->getPos();
                        rect.LowerRightCorner.Y = helpBase - helpScroll->getPos() + helpBox->getTextHeight();
                        helpBox->setRelativePosition(rect);
                        break;
                    }
    				case GUI_ID_STATE_SCROLL:
    				{
                        core::rect<s32> rect = stateBox->getRelativePosition();
                        rect.UpperLeftCorner.Y = stateBase - stateScroll->getPos();
                        rect.LowerRightCorner.Y = stateBase - stateScroll->getPos() + stateBox->getTextHeight();
                        stateBox->setRelativePosition(rect);
                        break;
                    }
    				case GUI_ID_STATEGLOBAL_SCROLL:
    				{
                        core::rect<s32> rect = stateGlobalBox->getRelativePosition();
                        rect.UpperLeftCorner.Y = stateGlobalBase - stateGlobalScroll->getPos();
                        rect.LowerRightCorner.Y = stateGlobalBase - stateGlobalScroll->getPos() + stateGlobalBox->getTextHeight();
                        stateGlobalBox->setRelativePosition(rect);
                        break;
                    }
                }
				break;
			case EGET_BUTTON_CLICKED:
				switch(id)
				{
    				case GUI_ID_NEW_BUTTON:
                        {
                            //IGUIWindow* pwindow = (IGUIWindow*)event.GUIEvent.Caller->getParent();
                            //closeWindow(pwindow);
                            //currentStage = 0;
                            //startGame(0);
                            openSelectionWindow();
                            return true;
                            break;
                        }
    				case GUI_ID_SEL_NEW_BUTTON:
                        {
                            IGUIWindow* pwindow = (IGUIWindow*)event.GUIEvent.Caller->getParent();
                            closeWindow(pwindow, true);
                            if (window)
                                closeWindow(window);
                            currentStage = 0;
                            startGame(0);
                            return true;
                            break;
                        }
    				case GUI_ID_SEL_NEXT_BUTTON:
                        {
                            carType++;
                            if (carType>=vehiclePool->getVehicleTypesSize()) carType = 0;
                            if (carType<0) carType = vehiclePool->getVehicleTypesSize()-1;
                            
                            core::stringw str = vehiclePool->getName(carType);
                            car_selector_text->setText(str.c_str());
                            
                            core::rect<int> messageRect = car_selector_text->getRelativePosition();
                            messageRect.LowerRightCorner.X = messageRect.UpperLeftCorner.X + car_selector_text->getTextWidth();
                            car_selector_text->setRelativePosition(messageRect);
                            
                            vehiclePool->putVehicle(car_to_draw);
                            car_to_draw = 0;
                            car_to_draw = vehiclePool->getVehicle(carType);
                            assert(car_to_draw);
                            car_to_draw->activate(core::vector3df(0.f,0.f,0.f),
                                                  core::vector3df(0.f,0.f,0.f), 
                                                  "", "", "",
                                                  0.5,
                                                  skydome,
                                                  shadowMap);
                            //NewtonUpdate(nWorld, 0.015f);
                            car_to_draw->pause();
                            return true;
                            break;
                        }
    				case GUI_ID_SEL_PREV_BUTTON:
                        {
                            carType--;
                            if (carType>=vehiclePool->getVehicleTypesSize()) carType = 0;
                            if (carType<0) carType = vehiclePool->getVehicleTypesSize()-1;
                            
                            core::stringw str = vehiclePool->getName(carType);
                            car_selector_text->setText(str.c_str());
                            
                            core::rect<int> messageRect = car_selector_text->getRelativePosition();
                            messageRect.LowerRightCorner.X = messageRect.UpperLeftCorner.X + car_selector_text->getTextWidth();
                            car_selector_text->setRelativePosition(messageRect);
                            
                            vehiclePool->putVehicle(car_to_draw);
                            car_to_draw = 0;
                            car_to_draw = vehiclePool->getVehicle(carType);
                            assert(car_to_draw);
                            car_to_draw->activate(core::vector3df(0.f,0.f,0.f),
                                                  core::vector3df(0.f,0.f,0.f), 
                                                  "", "", "",
                                                  0.5,
                                                  skydome,
                                                  shadowMap);
                            //NewtonUpdate(nWorld, 0.015f);
                            car_to_draw->pause();
                            return true;
                            break;
                        }
    				case GUI_ID_SEL_BACK_BUTTON:
                        {
                            IGUIWindow* pwindow = (IGUIWindow*)event.GUIEvent.Caller->getParent();
                            closeWindow(pwindow);
                            return true;
                            break;
                        }
                    case GUI_ID_RESTART_BUTTON:
                        {
                            IGUIWindow* pwindow = (IGUIWindow*)event.GUIEvent.Caller->getParent();
                            closeWindow(pwindow);
                            restartStage();
                            return true;
                            break;
                        }
    				case GUI_ID_NEXT_BUTTON:
                        {
                            IGUIWindow* pwindow = (IGUIWindow*)event.GUIEvent.Caller->getParent();
                            closeWindow(pwindow);
                            //if (bigTerrain && bigTerrain->getEndTime() != 0/* && bigTerrain->cps==0*/)
                            if (bigTerrain && bigTerrain->getTimeEnded()/* && bigTerrain->cps==0*/)
                            {
                                currentStage++;
                                startGame(currentStage);
                            }
                            else
                            {
                                playSound(clickSound);
                                MessageText::addText(L"You cannot go to the next stage!!!", 5);
                            }
                            if (!car)
                                openMainWindow();
                            return true;
                            break;
                        }
    				case GUI_ID_LOAD_BUTTON:
                        {
                            IGUIWindow* pwindow = (IGUIWindow*)event.GUIEvent.Caller->getParent();
                            closeWindow(pwindow);
                            if (loadGame(SAVE_FILE))
                            {
                                //startGame(currentStage);
                                //MessageText::addText(L"Game loaded", 5);
                            }
                            else
                            {
                                playSound(clickSound);
                                MessageText::addText(L"Unable to load game!!!", 5);
                            }
                            if (!car)
                                openMainWindow();
                            return true;
                            break;
                        }
    				case GUI_ID_SAVE_BUTTON:
                        {
                            IGUIWindow* pwindow = (IGUIWindow*)event.GUIEvent.Caller->getParent();
                            if (car)
                                closeWindow(pwindow);
                            if (saveGame(SAVE_FILE))
                                MessageText::addText(L"Game saved", 5);
                            else
                                MessageText::addText(L"Unable to save game!!!", 5);
                            //bgImage->setVisible(false);
                            return true;
                            break;
                        }
    				case GUI_ID_SAVESETTINGS_BUTTON:
                        {
                            // update the server stuffs
                            core::stringw str = server_name_text->getText();
                            const wchar_t* orig = str.c_str();
                            size_t origsize = wcslen(orig) + 1;
                            wcstombs(server_name, orig, origsize);

                            char portString[256];
                            str = server_port_text->getText();
                            orig = str.c_str();
                            origsize = wcslen(orig) + 1;
                            wcstombs(portString, orig, origsize);
                            sscanf(portString, "%d", &server_port);

                            str = server_delay_text->getText();
                            orig = str.c_str();
                            origsize = wcslen(orig) + 1;
                            wcstombs(portString, orig, origsize);
                            sscanf(portString, "%d", &send_server_delay);
                            // server stuff update end

                            if (writeSettings("data/settings.txt"))
                                MessageText::addText(L"Settings saved", 5);
                            else
                                MessageText::addText(L"Unable to save settings!!!", 5);
                            return true;
                            break;
                        }
    				case GUI_ID_OPTIONS_BUTTON:
                        openOptionsWindow();
                        return true;
                        break;
    				case GUI_ID_HELP_BUTTON:
                        openHelpWindow();
                        return true;
                        break;
    				case GUI_ID_STATE_BUTTON:
                        openStateWindow();
                        return true;
                        break;
                	case GUI_ID_REINITIALIZE_BUTTON:
                        reinitialize = true;
                        // fall through
    				case GUI_ID_EXIT_BUTTON:
                        playSound(clickSound);
                        quitGame = true;
                        return true;
                        break;
    				case GUI_ID_LEPORGET_BUTTON:
                        playSound(clickSound);
                        stateWindow->remove();
                        while (raceEngine->update(0, vector3df(), playerCompetitor, CRaceEngine::AtTheEnd));
                        // todo update global states here
                        CRaceEngine::refreshRaceState(raceEngine);
                        refreshStateWindow(false); // false - no need the leporget button again
                        return true;
                        break;
    				case GUI_ID_BACK_BUTTON:
                        {
                            IGUIWindow* pwindow = (IGUIWindow*)event.GUIEvent.Caller->getParent();
                            closeWindow(pwindow);
                            return true;
                            break;
                        }
    				case GUI_ID_RESUME_BUTTON:
                        {
                            IGUIWindow* pwindow = (IGUIWindow*)event.GUIEvent.Caller->getParent();
                            closeWindow(pwindow);
                            return true;
                            break;
                        }
                    case GUI_ID_CHOOSE_CAR_BUTTON:
                        playSound(openSound);
                        env->addFileOpenDialog(L"Please choose a file.", true, window, GUI_ID_CAR_FILE_OPEN);
                        return true;
                        break;
                    case GUI_ID_APPLY_CAR_BUTTON:
                     {
                        playSound(clickSound);
                        /*
                        core::stringw str = carfilename_text->getText();
                        const wchar_t* orig = str.c_str();
                        size_t origsize = wcslen(orig) + 1;
                        wcstombs(carName, orig, origsize);
                        chdir(currentDirectory);
                        */
                        carType = carfilename_cbox->getSelected();
                        /*
                        if (ind < carList.length())
                        {
                            strcpy(carName, "data/vehicles/");
                            strcat(carName, carList[ind]->carFileName);
                        }
                        */
                        if (carType >= vehiclePool->getVehicleTypesSize() || carType < 0) carType = 0;
                        if (car && bigTerrain)
                        {
                            matrix4 mat = car->getMatrix();
                            core::vector3df rot = mat.getRotationDegrees();
                            
                            //mat.setTranslation(core::vector3df(-20000.f,-20000.f,-20000.f));
                            //car->setMatrixWithNB(mat);
                            vehiclePool->putVehicle(car);
                            NewtonInvalidateCache(nWorld);
                            
                            car = vehiclePool->getVehicle(carType);
                            car->activate(
                                 core::vector3df(camera->getPosition().X,bigTerrain->getHeight(camera->getPosition().X,camera->getPosition().Z)+5.f,camera->getPosition().Z),
                                 rot, bigTerrain->getGroundSoundName(), bigTerrain->getPuffSoundName(),
                                 bigTerrain->getSkidSoundName(),
                                 bigTerrain->getFrictionMulti(),
                                 skydome,
                                 shadowMap);
                            car->pause();
                            dynCamReset = true;
                        }
                        return true;
                        break;
                     }
                    case GUI_ID_CONNECT_BUTTON:
                     {
                        IGUIButton* button = (IGUIButton*)event.GUIEvent.Caller;
                        playSound(clickSound);
                        
                        if (isMultiplayer)
                        {
                            disconnectFromServer(true);
                            button->setText(L"Conn.");
                        }
                        else
                        {
                            core::stringw str = server_name_text->getText();
                            const wchar_t* orig = str.c_str();
                            size_t origsize = wcslen(orig) + 1;
                            wcstombs(server_name, orig, origsize);

                            char portString[256];
                            str = server_port_text->getText();
                            orig = str.c_str();
                            origsize = wcslen(orig) + 1;
                            wcstombs(portString, orig, origsize);
                            sscanf(portString, "%d", &server_port);

                            str = server_delay_text->getText();
                            orig = str.c_str();
                            origsize = wcslen(orig) + 1;
                            wcstombs(portString, orig, origsize);
                            sscanf(portString, "%d", &send_server_delay);

                            connectToServer(device,
                                            driver,
                                            smgr,
                                            env,
                                            nWorld,
                                            soundEngine);
                            
                            button->setText(L"Discon.");
                        }                            
                        return true;
                        break;
                     }
    				case GUI_ID_CALIBRATE_BUTTON:
                        playSound(clickSound);
                        centerJoystickState = JoystickState;
                        return true;
                        break;
                }
                break;
			case EGET_FILE_SELECTED:
				switch(id)
				{
                    case GUI_ID_CAR_FILE_OPEN:
                        {
                            playSound(closeSound);
                            carfilename_text->setText(((IGUIFileOpenDialog*)event.GUIEvent.Caller)->getFileName());
                            break;
                        }
                }
                break;
			case EGET_EDITBOX_ENTER:
				switch(id)
				{
                    case GUI_ID_SERVER_NAME_EBOX:
                        {
                            playSound(closeSound);

                            const wchar_t* orig = server_name_text->getText();
                            size_t origsize = wcslen(orig) + 1;
                            wcstombs(server_name, orig, origsize);

                            break;
                        }
                    case GUI_ID_SERVER_PORT_EBOX:
                        {
                            playSound(closeSound);

                            char portString[256];
                            const wchar_t* orig = server_port_text->getText();
                            size_t origsize = wcslen(orig) + 1;
                            wcstombs(portString, orig, origsize);
                            sscanf(portString, "%d", &server_port);

                            break;
                        }
                    case GUI_ID_SERVER_DELAY_EBOX:
                        {
                            playSound(closeSound);

                            char portString[256];
                            const wchar_t* orig = server_delay_text->getText();
                            size_t origsize = wcslen(orig) + 1;
                            wcstombs(portString, orig, origsize);
                            sscanf(portString, "%d", &send_server_delay);
                            break;
                        }
                }
                break;
			case EGET_COMBO_BOX_CHANGED:
				switch(id)
				{
                    case GUI_ID_DRIVERTYPE_CBOX:
                    {
                        playSound(closeSound);
                        
                        s32 pos = driverType_cbox->getSelected();
                        switch (pos)
                        {
                            case 0:
                                driverType = video::EDT_DIRECT3D9;
                                break;
                            case 1:
                                driverType = video::EDT_DIRECT3D8;
                                break;
                            case 2:
                                driverType = video::EDT_OPENGL;
                                break;
                            case 3:
                                driverType = video::EDT_SOFTWARE;
                                break;
                        }
                        break;
                    }
                    case GUI_ID_RESOLUTION_CBOX:
                    {
                        playSound(closeSound);
                        
                        s32 pos = resolution_cbox->getSelected();
#ifdef IRRLICHT_SDK_15
                        core::dimension2d<s32> res = device->getVideoModeList()->getVideoModeResolution(pos);
#else
                        core::dimension2d<u32> res = device->getVideoModeList()->getVideoModeResolution(pos);
#endif
                        resolutionX = res.Width;
                        resolutionY = res.Height;
                        display_bits = device->getVideoModeList()->getVideoModeDepth(pos);
                        /*
                        switch (pos)
                        {
                            case 0:
                                resolutionX = 320;
                                resolutionY = 240;
                                break;
                            case 1:
                                resolutionX = 640;
                                resolutionY = 480;
                                break;
                            case 2:
                                resolutionX = 800;
                                resolutionY = 600;
                                break;
                            case 3:
                                resolutionX = 1024;
                                resolutionY = 768;
                                break;
                            case 4:
                                resolutionX = 1280;
                                resolutionY = 1024;
                                break;
                            case 5:
                                resolutionX = 1400;
                                resolutionY = 1050;
                                break;
                            case 6:
                                resolutionX = 1600;
                                resolutionY = 1200;
                                break;
                            case 7:
                                resolutionX = 1280;
                                resolutionY = 720;
                                break;
                            case 8:
                                resolutionX = 1280;
                                resolutionY = 768;
                                break;
                            case 9:
                                resolutionX = 1280;
                                resolutionY = 800;
                                break;
                            case 10:
                                resolutionX = 1680;
                                resolutionY = 1050;
                                break;
                            case 11:
                                resolutionX = 1920;
                                resolutionY = 1080;
                                break;
                            case 12:
                                resolutionX = 1920;
                                resolutionY = 1200;
                                break;
                        }
                        */
                        break;
                    }
                    case GUI_ID_DISPLAY_BITS_CBOX:
                    {
                        playSound(closeSound);
                        
                        s32 pos = display_bits_cbox->getSelected();
                        /*
                        switch (pos)
                        {
                            case 0:
                                display_bits = 16;
                                break;
                            case 1:
                                display_bits = 32;
                                break;
                        }
                        */
                        break;
                    }
                    case GUI_ID_SHADOW_MAP_SIZE_CBOX:
                    {
                        playSound(closeSound);
                        
                        s32 pos = shadow_map_size_cbox->getSelected();
                        switch (pos)
                        {
                            case 0:
                                shadow_map_size = 256;
                                break;
                            case 1:
                                shadow_map_size = 512;
                                break;
                            case 2:
                                shadow_map_size = 1024;
                                break;
                            case 3:
                                shadow_map_size = 2048;
                                break;
                            case 4:
                                shadow_map_size = 4096;
                                break;
                        }
                        break;
                    }
                    case GUI_ID_EFFECTS_CBOX:
                    {
                        playSound(closeSound);
                        
                        s32 pos = effects_cbox->getSelected();
                        switch (pos)
                        {
                            case 0:
                                globalLight = false;
                                useShaders = false;
                                useCgShaders = false;
                                break;
                            case 1:
                                globalLight = true;
                                useShaders = false;
                                useCgShaders = false;
                                break;
                            case 2:
                                globalLight = true;
                                useShaders = true;
                                useCgShaders = false;
                                break;
                            case 3:
                                globalLight = true;
                                useShaders = true;
                                useCgShaders = true;
                                break;
                        }
                        if (useShaders)
                        {
                            texturedetail_text->setVisible(false);
                            texturedetail_cbox->setVisible(true);
                        }
                        else
                        {
                            texturedetail_cbox->setVisible(false);
                            texturedetail_text->setVisible(true);
                        }
                        break;
                    }
                    case GUI_ID_TEXTUREDETAIL_CBOX:
                    {
                        playSound(closeSound);
                        
                        s32 pos = texturedetail_cbox->getSelected();
                        switch (pos)
                        {
                            case 0:
                                use_detailed_terrain = false;
                                use_highres_textures = false;
                                break;
                            case 1:
                                use_detailed_terrain = true;
                                use_highres_textures = false;
                                break;
                            case 2:
                                use_detailed_terrain = true;
                                use_highres_textures = true;
                                break;
                        }
                        break;
                    }
                    case GUI_ID_LOD_DISTANCE_CBOX:
                    {
                        playSound(closeSound);
                        
                        s32 pos = LOD_distance_cbox->getSelected();
                        switch (pos)
                        {
                            case 0:
                                LOD_distance = 9;
                                break;
                            case 1:
                                LOD_distance = 17;
                                break;
                            case 2:
                                LOD_distance = 33;
                                break;
                        }
                        break;
                    }
                    case GUI_ID_JOY_CBOX:
                    {
                       	core::stringw str;
                        playSound(closeSound);
                        if (joy_cbox->getSelected() > 0)
                        {
                            str = L"w...";
                            str += *joyHelper[joy_cbox->getSelected()-1];
                        } else
                            str = L"wait...";
                        joy_text->setText(str.c_str());
                        break;
                    }
                    case GUI_ID_JOYS_CBOX:
                    {
                        playSound(closeSound);

                        activeJoystick = joys_cbox->getSelected();
                        break;
                    }
                }
                break;
			case EGET_CHECKBOX_CHANGED:
				switch(id)
				{
                    case GUI_ID_USE_SMOKES:
                        {
                            playSound(clickSound);
                            useSmokes = ((IGUICheckBox*)event.GUIEvent.Caller)->isChecked();
                            return true;
                            break;
                        }
                    case GUI_ID_DRAW_HUD:
                        {
                            playSound(clickSound);
                            draw_hud = ((IGUICheckBox*)event.GUIEvent.Caller)->isChecked();
                            if (bigTerrain)
                                hudImage->setVisible(draw_hud);
                            return true;
                            break;
                        }
                    case GUI_ID_SHOW_NAMES:
                        {
                            playSound(clickSound);
                            show_names = ((IGUICheckBox*)event.GUIEvent.Caller)->isChecked();
                            if (raceEngine)
                                raceEngine->updateShowNames();
                            return true;
                            break;
                        }
                    case GUI_ID_FULL_SCREEN:
                        {
                            playSound(clickSound);
                            full_screen = ((IGUICheckBox*)event.GUIEvent.Caller)->isChecked();
                            return true;
                            break;
                        }
                    case GUI_ID_AUTO_RES:
                        {
                            playSound(clickSound);
                            auto_resolution = ((IGUICheckBox*)event.GUIEvent.Caller)->isChecked();
                            return true;
                            break;
                        }
                    case GUI_ID_ANTI_ALIASING:
                        {
                            playSound(clickSound);
                            anti_aliasing = ((IGUICheckBox*)event.GUIEvent.Caller)->isChecked();
                            return true;
                            break;
                        }
                    case GUI_ID_VSYNC:
                        {
                            playSound(clickSound);
                            vsync = ((IGUICheckBox*)event.GUIEvent.Caller)->isChecked();
                            return true;
                            break;
                        }
                    case GUI_ID_HIGH_PRECISION_FPU:
                        {
                            playSound(clickSound);
                            high_precision_fpu = ((IGUICheckBox*)event.GUIEvent.Caller)->isChecked();
                            return true;
                            break;
                        }
                    case GUI_ID_LIGHT:
                        {
                            playSound(clickSound);
                            globalLight = ((IGUICheckBox*)event.GUIEvent.Caller)->isChecked();
                            return true;
                            break;
                        }
                    case GUI_ID_SHADOWS:
                        {
                            playSound(clickSound);
                            shadows = ((IGUICheckBox*)event.GUIEvent.Caller)->isChecked();
                            if (shadows && useShaders && useCgShaders && shadowMap==0 &&
                                driver->queryFeature(video::EVDF_RENDER_TO_TARGET))
                            {
                                dprintf(printf("shadow map is supported\n"));
#ifdef IRRLICHT_SDK_15
                                shadowMap = shadowMapGame = shadows ? driver->addRenderTargetTexture(core::dimension2d<s32>(shadow_map_size,shadow_map_size), "RTT1")
                                                     : 0;
                                shadowMapCar = shadows ? driver->addRenderTargetTexture(core::dimension2d<s32>(shadow_map_size,shadow_map_size), "RTT1")
                                                     : 0;
#else
                                shadowMap = shadowMapGame = shadows ? driver->addRenderTargetTexture(core::dimension2d<u32>(shadow_map_size,shadow_map_size), "RTT1")
                                                     : 0;
                                shadowMapCar = shadows ? driver->addRenderTargetTexture(core::dimension2d<u32>(shadow_map_size,shadow_map_size), "RTT1")
                                                     : 0;
#endif
                                //hudImage->setImage(shadowMap);
                            }
                            return true;
                            break;
                        }
                    case GUI_ID_USESHADERS:
                        {
                            playSound(clickSound);
                            useShaders = ((IGUICheckBox*)event.GUIEvent.Caller)->isChecked();
                            return true;
                            break;
                        }
                    case GUI_ID_USECGSHADERS:
                        {
                            playSound(clickSound);
                            useCgShaders = ((IGUICheckBox*)event.GUIEvent.Caller)->isChecked();
                            return true;
                            break;
                        }
                    case GUI_ID_STENCIL_SHADOWS:
                        {
                            playSound(clickSound);
                            stencil_shadows = ((IGUICheckBox*)event.GUIEvent.Caller)->isChecked();
                            return true;
                            break;
                        }
                    case GUI_ID_USE_SCREEN_RTT:
                        {
                            playSound(clickSound);
                            useScreenRTT = ((IGUICheckBox*)event.GUIEvent.Caller)->isChecked();
                            depth_effect = ((IGUICheckBox*)event.GUIEvent.Caller)->isChecked();
                            recreateRTTs(driver);
                            return true;
                            break;
                        }
                    case GUI_ID_USE_DEPTH_RTT:
                        {
                            playSound(clickSound);
                            depth_effect = ((IGUICheckBox*)event.GUIEvent.Caller)->isChecked();
                            recreateRTTs(driver);
                            return true;
                            break;
                        }
                    case GUI_ID_SHIT_ATI:
                        {
                            playSound(clickSound);
                            shitATI = ((IGUICheckBox*)event.GUIEvent.Caller)->isChecked();
                            recreateRTTs(driver);
                            return true;
                            break;
                        }
                    case GUI_ID_FLIP_VERT:
                        {
                            playSound(clickSound);
                            flip_vert = ((IGUICheckBox*)event.GUIEvent.Caller)->isChecked();
                            return true;
                            break;
                        }
                    case GUI_ID_TRACE_NET:
                        {
                            playSound(clickSound);
                            trace_net = ((IGUICheckBox*)event.GUIEvent.Caller)->isChecked();
                            return true;
                            break;
                        }
                    case GUI_ID_STEER_LINEAR:
                        {
                            playSound(clickSound);
                            joy_steer_linear = ((IGUICheckBox*)event.GUIEvent.Caller)->isChecked();
                            return true;
                            break;
                        }
                    case GUI_ID_GEAR_TYPE_AUTO:
                        {
                            playSound(clickSound);
                            if (((IGUICheckBox*)event.GUIEvent.Caller)->isChecked())
                                gear_type = 'a';
                            else
                                gear_type = 'm';
                            return true;
                            break;
                        }
                    case GUI_ID_SHOW_COMPASS_ARROW:
                        {
                            playSound(clickSound);
                            show_compass_arrow = ((IGUICheckBox*)event.GUIEvent.Caller)->isChecked();
                            if (showCompass)
                            {
                                compassArrow->setVisible(show_compass_arrow);
                            }
                            return true;
                            break;
                        }
                }
                break;
			case EGET_ELEMENT_CLOSED:
				switch(id)
				{
                    case GUI_ID_OPTIONS_WINDOW:
                            closeWindow(optionsWindow);
                            return true;
                            break;
                    case GUI_ID_MAIN_WINDOW:
                            if (car)
                                closeWindow(mainWindow);
                            return true;
                            break;
                    case GUI_ID_HELP_WINDOW:
                            closeWindow(helpWindow);
                            return true;
                            break;
                    case GUI_ID_STATE_WINDOW:
                            closeWindow(stateWindow);
                            return true;
                            break;
                }
                break;
			case EGET_TAB_CHANGED:
                refreshActiveElements();
                break;
			case EGET_ELEMENT_HOVERED:
                //if (id==GUI_ID_HELP_BOX || event.GUIEvent.Caller->getType() == EGUIET_WINDOW
                //     || event.GUIEvent.Caller->getType() == EGUIET_TAB
                //     ) break;
                if (!skipHover)
                {
                    int i = 0;
                    for (; i < activeElements.length(); i++)
                        if (activeElements[i] == event.GUIEvent.Caller)
                        {
                            activeElement = i;
                            break;
                        }
                    if (i < activeElements.length())
                        env->setFocus(event.GUIEvent.Caller);
                }
                else
                {
                    skipHover = false;
                }
                //refreshActiveElements();
                break;
			case EGET_ELEMENT_FOCUSED:
                if (id==GUI_ID_HELP_BOX || id==GUI_ID_HELP_WINDOW)
                    env->setFocus(helpScroll);
                
                if (id==GUI_ID_STATE_BOX)
                    env->setFocus(stateScroll);
                if (id==GUI_ID_STATEGLOBAL_BOX)
                    env->setFocus(stateGlobalScroll);
                /*
                if (id==GUI_ID_STATE_WINDOW || id == GUI_ID_STATE_TABCONTROL )
                {
                    if (stateTabControl->getActiveTab()==0)
                        env->setFocus(stateScroll);
                    else
                        env->setFocus(stateGlobalScroll);
                }
                */
                break;
        }
    }
    else
	if (event.EventType == irr::EET_JOYSTICK_INPUT_EVENT
		&& event.JoystickEvent.Joystick == activeJoystick)
	{
		JoystickState = event.JoystickEvent;
		int pushed = -1;
       	core::stringw str = L" ";
       	
       	if (firstJoyState)
       	{
            firstJoyState = false;
            centerJoystickState = JoystickState;
        }

		const u16 povDegrees = JoystickState.POV / 100;
		if(povDegrees < 360)
		{
            if (pov_up_p == 0)
            {
    			pov_up_p = 1;
                device->getCursorControl()->setVisible(false);
                
    			if(povDegrees > 0 && povDegrees < 180)
    				do_right(true);
    			else if(povDegrees > 180)
    				do_left(true);
    
    			if(povDegrees > 90 && povDegrees < 270)
    				do_down();
    			else if(povDegrees > 270 || povDegrees < 90)
    				do_up();
            }
		}
		else
		{
            pov_up_p = 0;
            pov_down_p = 0;
            pov_left_p = 0;
            pov_right_p = 0;
        }

		if (env->getFocus() != joy_cbox)
		{
    		if (JoystickState.IsButtonPressed(0))
    		{
                if (joy_enter_p == 0)
                {
                    joy_enter_p = 1;
                    device->getCursorControl()->setVisible(false);
                    do_enter();
                }
            }
            else
                joy_enter_p = 0;

    		if (JoystickState.IsButtonPressed(1))
    		{
                if (joy_esc_p == 0)
                {
                    joy_esc_p = 1;
                    do_esc();
                }
            }
            else
                joy_esc_p = 0;
        }
        else
        {
            if (!joy_text) return false;
		
    		for (int i = 0; i<SEvent::SJoystickEvent::NUMBER_OF_AXES;i++)
    		{
                if (abs(JoystickState.Axis[i] - centerJoystickState.Axis[i])>16000)
                {
                    pushed = i;
                    break;
                }
            }
            
            if (pushed!=-1)
            {
                if (joy_cbox && joy_cbox->getSelected()>0 && joy_cbox->getSelected()<4)
                {
                    str = L"SA: ";
                    for (int i = 0; i < 3; i++)
                        if (*joyHelper[i] == pushed)
                        {
                            *joyHelper[i] = -1;
                        }
                    *joyHelper[joy_cbox->getSelected()-1] = pushed;
                }
                else
                    str = L"AX: ";
                    
                str += pushed;
                joy_text->setText(str.c_str());
            }
            else
            {
        		for (int i = 0; i<SEvent::SJoystickEvent::NUMBER_OF_BUTTONS;i++)
        		{
                    if (JoystickState.IsButtonPressed(i))
                    {
                        pushed = i;
                        break;
                    }
                }
                if (pushed!=-1)
                {
                    if (joy_cbox && joy_cbox->getSelected()>3 && joy_cbox->getSelected()<JOY_HELPER_NUM)
                    {
                        str = L"SB: ";
                        for (int i = 3; i < JOY_HELPER_NUM; i++)
                            if (*joyHelper[i] == pushed)
                            {
                                *joyHelper[i] = -1;
                            }
                        *joyHelper[joy_cbox->getSelected()-1] = pushed;
                    }
                    else
                        str = L"BUT: ";
                        
                    str += pushed;
                    joy_text->setText(str.c_str());
                }
                else
                {
                    if (joy_cbox->getSelected() > 0)
                    {
                        str = L"w...";
                        str += *joyHelper[joy_cbox->getSelected()-1];
                    } else
                        str = L"wait...";
                    joy_text->setText(str.c_str());
                }
            }
        }
    }
    else
	if (event.EventType == irr::EET_MOUSE_INPUT_EVENT)
	{
        if (!device->getCursorControl()->isVisible())
        {
            skipHover = true;
        }
        device->getCursorControl()->setVisible(true);
    }

	return false;
}
	
const SEvent::SJoystickEvent& eventreceiver_menu::GetJoystickState(void) const
{
	return JoystickState;
}

void eventreceiver_menu::closeWindow(IGUIElement* cw, bool noact)
{
    inGame--;
    if (!noact)
    {
        playSound(closeSound);
    }
    if (inGame == 0)
    {
        joy_look_l = 1;
        joy_look_r = 1;
        joy_gu = 1;
        joy_gd = 1;
        joy_reset_car_p =1;
        joy_change_view_p = 1;
        joy_change_light_p = 1;
        joy_show_compass_p = 1;
        joy_repair_car_p = 1;
        joy_menu_p = 1;
        setOtherReceiver();
        resumeGame();
    }
    //printf("clb w %d %d %d %p %p\n", mainWindowOpened, optionsWindowOpened, helpWindowOpened, window, cw);

    if (cw == mainWindow)
    {
        //printf("mc\n");
        mainWindowOpened = false;
        mainWindow->remove();
        mainWindow = 0;
        if (window==cw) window = mainWindowP;
    }
    else
    if (cw == optionsWindow)
    {
        //printf("oc\n");
        optionsWindowOpened = false;
        optionsWindow->remove();
        optionsWindow = 0;
        joy_text = 0;
        joy_cbox = 0;
        tabControl = 0;
        saveSettingsButton = 0;
        closeOptionsButton = 0;
        if (window==cw) window = optionsWindowP;
    }
    else
    if (cw == helpWindow)
    {
        //printf("hc\n");
        helpWindowOpened = false;
        helpWindow->remove();
        helpWindow = 0;
        helpBox = 0;
        helpScroll = 0;
        if (window==cw) window = helpWindowP;
    }
    else
    if (cw == stateWindow)
    {
        //printf("hc\n");
        stateWindowOpened = false;
        stateWindow->remove();
        stateWindow = 0;
        stateBox = 0;
        stateScroll = 0;
        stateGlobalBox = 0;
        stateGlobalScroll = 0;
        if (window==cw) window = stateWindowP;
    }
    else
    if (cw == selectionWindow)
    {
        selectionWindowOpened = false;
        selectionWindow->remove();
        selectionWindow = 0;
        shadowMap = shadowMapGame;
        if (car_to_draw)
        {
            //printf("delete car to draw\n");
            car_selector_camera->removeAnimators();
            vehiclePool->putVehicle(car_to_draw);
            //printf("delete car to draw end\n");
            car_to_draw = 0;
        }
        if (window==cw) window = selectionWindowP;
        if (window)
            window->setVisible(true);
    }
    
    if (window)
    {
        env->setFocus(window);
        refreshActiveElements();
    }
    else
    {
        env->setFocus(0);
        lastParent = 0;
    }
    //printf("cle w %d %d %d %p\n", mainWindowOpened, optionsWindowOpened, helpWindowOpened, window);
}	

void eventreceiver_menu::openMainWindow()
{
    const int outdist = (screenSize.Width - 500) / 2; // 150;
    const int indist = (int)((float)screenSize.Height/1368.f*50.f);
    const int buttonWidth = (int)((float)screenSize.Height/1368.f*512.f);
    const int buttonHeight = (int)((float)screenSize.Height/1368.f*90.f);
    const int diff = (int)((float)screenSize.Height/1368.f*22.5f) + buttonHeight;
    IGUIButton* button;

    int skipButtons = 0;
    
    if (!car)
    {
        skipButtons += 4;
        if (checkLoadGame(SAVE_FILE))
        {
            skipButtons--;
        }
    }
    if (!checkLoadGame(SAVE_FILE))
    {
        skipButtons++;
    }
    if (bigTerrain && bigTerrain->getTimeEnded() &&
        oldStage+1 < MAX_STAGES && stages[oldStage+1] != 0)
    {}
    else
    {
        skipButtons++;
    }

    //printf("skipButtons: %d\n", skipButtons);
    int line = (int)((float)screenSize.Height/1368.f*330.f) + (diff - buttonHeight) + (skipButtons*diff)/2;
    //printf("mb w %d %d %d %p\n", mainWindowOpened, optionsWindowOpened, helpWindowOpened, window);
    
    if (mainWindowOpened || loading > 0 || window) return;

    mainWindowOpened = true;
    if (inGame == 0)
    {
        joy_enter_p = 1;
        joy_esc_p = 1;
        pov_up_p = pov_down_p = pov_left_p = pov_right_p = 1;
        other = (eventreceiver*)device->getEventReceiver();
       	device->setEventReceiver(this);
        pauseGame();
    }
    inGame++;
    
    mainWindowP = window;
    
    if (car==0 || bigTerrain==0)
    {
        bgImage->setVisible(true);
        versionText->setVisible(true);
    }


    IGUIImage* bg_frame = env->addImage(core::rect<int>(0,0,screenSize.Width,screenSize.Height), 0, -1, L"bg_frame");
    bg_frame->setScaleImage(true);
    bg_frame->setImage(mainmenu_texture);
    bg_frame->setUseAlphaChannel(true);
    //bg_frame->setVisible(false);
    mainWindow = window = bg_frame;
/*
    window = mainWindow = env->addWindow(
		rect<s32>(outdist, outdist, screenSize.Width - outdist, screenSize.Height - outdist),
		false, // modal?
		L"  Main Menu", 0, GUI_ID_MAIN_WINDOW);
*/
    env->setFocus(window);

    if (bigTerrain && bigTerrain->getTimeEnded() &&
        oldStage+1 < MAX_STAGES && stages[oldStage+1] != 0)
    {
        button = env->addButton(rect<s32>(indist, line, indist+buttonWidth, line+buttonHeight),
            window, GUI_ID_NEXT_BUTTON,
            L"", 0/*L"Next stage (if possible)"*/);
        button->setImage(button_bg_next);
        button->setScaleImage(true);
        button->setUseAlphaChannel(true);
        line += diff;
    }
    if (car)
    {
        button = env->addButton(rect<s32>(indist, line, indist+buttonWidth, line+buttonHeight),
            window, GUI_ID_RESUME_BUTTON,
            L"", 0/*L"Back to the game"*/);
        button->setImage(button_bg_resume);
        button->setScaleImage(true);
        button->setUseAlphaChannel(true);
        /*
        line += diff;
        button = env->addButton(rect<s32>(indist, line, indist+buttonWidth, line+buttonHeight),
            window, GUI_ID_RESTART_BUTTON,
            L"", 0);
        button->setImage(button_bg_restart);
        button->setScaleImage(true);
        button->setUseAlphaChannel(true);
        */
        line += diff;
        button = env->addButton(rect<s32>(indist, line, indist+buttonWidth, line+buttonHeight),
            window, GUI_ID_STATE_BUTTON,
            L"", 0);
        button->setImage(button_bg_state);
        button->setScaleImage(true);
        button->setUseAlphaChannel(true);
    
        line += diff;
    } else
    {
        if (checkLoadGame(SAVE_FILE))
        {
            button = env->addButton(rect<s32>(indist, line, indist+buttonWidth, line+buttonHeight),
                window, GUI_ID_LOAD_BUTTON,
                L"", 0/*L"Continue previously saved game"*/);
            line += diff;
            button->setImage(button_bg_continue);
            button->setScaleImage(true);
            button->setUseAlphaChannel(true);
        }
    }
    button = env->addButton(rect<s32>(indist, line, indist+buttonWidth, line+buttonHeight),
        window, GUI_ID_NEW_BUTTON,
        L"", 0/*L"Start new game"*/);
    button->setImage(button_bg_new);
    button->setScaleImage(true);
    button->setUseAlphaChannel(true);
    line += diff;
    if (checkLoadGame(SAVE_FILE))
    {
        button = env->addButton(rect<s32>(indist, line, indist+buttonWidth, line+buttonHeight),
            window, GUI_ID_LOAD_BUTTON,
            L"", 0/*L"Load an exist game"*/);
        button->setImage(button_bg_load);
        button->setScaleImage(true);
        button->setUseAlphaChannel(true);
        line += diff;
    }
    if (car)
    {
        button = env->addButton(rect<s32>(indist, line, indist+buttonWidth, line+buttonHeight),
            window, GUI_ID_SAVE_BUTTON,
            L"", 0/*L"Save the current game"*/);
        button->setImage(button_bg_save);
        button->setScaleImage(true);
        button->setUseAlphaChannel(true);
        line += diff;
    }
    button = env->addButton(rect<s32>(indist, line, indist+buttonWidth, line+buttonHeight),
        window, GUI_ID_OPTIONS_BUTTON,
        L"", 0/*L"Open settings window"*/);
    button->setImage(button_bg_options);
    button->setScaleImage(true);
    button->setUseAlphaChannel(true);
    
    line += diff;
    button = env->addButton(rect<s32>(indist, line, indist+buttonWidth, line+buttonHeight),
        window, GUI_ID_HELP_BUTTON,
        L"", 0/*L"Show some help"*/);
    button->setImage(button_bg_help);
    button->setScaleImage(true);
    button->setUseAlphaChannel(true);
    
    line += diff;
    button = env->addButton(rect<s32>(indist, line, indist+buttonWidth, line+buttonHeight),
        window, GUI_ID_EXIT_BUTTON,
        L"", 0/*L"Exit from the game"*/);
    button->setImage(button_bg_quit);
    button->setScaleImage(true);
    button->setUseAlphaChannel(true);
    line += diff;
    
    int outdistH = (screenSize.Height - (line + indist)) / 2;
    //window->setRelativePosition(
	//	rect<s32>(outdist, outdistH, screenSize.Width - outdist, outdistH + line + indist));

    refreshActiveElements();

    playSound(openSound);

    //printf("me w %d %d %d %p\n", mainWindowOpened, optionsWindowOpened, helpWindowOpened, window);
}	

void eventreceiver_menu::openSelectionWindow()
{
    const int outdist = (screenSize.Width - 500) / 2; // 150;
    const int indist = (int)((float)screenSize.Height/1368.f*50.f);
    const int buttonWidth = (int)((float)screenSize.Height/1368.f*512.f);
    const int buttonHeight = (int)((float)screenSize.Height/1368.f*90.f);
    const int diff = (int)((float)screenSize.Height/1368.f*22.5f) + buttonHeight;
    const int startLine = (int)((float)screenSize.Height/1368.f*330.f);
    IGUIButton* button;

    int skipButtons = 5;
    
    //printf("skipButtons: %d\n", skipButtons);
    int line = (int)((float)screenSize.Height/1368.f*330.f) + (diff - buttonHeight) + (skipButtons*diff)/2;
    //printf("mb w %d %d %d %p\n", mainWindowOpened, optionsWindowOpened, helpWindowOpened, window);
    
    if (selectionWindowOpened || loading > 0 || window != mainWindow) return;

    window->setVisible(false);

    selectionWindowOpened = true;
    if (inGame == 0)
    {
        joy_enter_p = 1;
        joy_esc_p = 1;
        pov_up_p = pov_down_p = pov_left_p = pov_right_p = 1;
        other = (eventreceiver*)device->getEventReceiver();
       	device->setEventReceiver(this);
        pauseGame();
    }
    inGame++;
    
    selectionWindowP = window;

    IGUIImage* bg_frame = env->addImage(core::rect<int>(0,0,screenSize.Width,screenSize.Height), 0, -1, L"bg_frame_select");
    bg_frame->setScaleImage(true);
    bg_frame->setImage(selector_texture);
    bg_frame->setUseAlphaChannel(true);
    //bg_frame->setVisible(false);
    selectionWindow = window = bg_frame;
/*
    window = mainWindow = env->addWindow(
		rect<s32>(outdist, outdist, screenSize.Width - outdist, screenSize.Height - outdist),
		false, // modal?
		L"  Main Menu", 0, GUI_ID_MAIN_WINDOW);
*/
    env->setFocus(window);

    button = env->addButton(rect<s32>(indist, line, indist+buttonWidth, line+buttonHeight),
        window, GUI_ID_SEL_NEW_BUTTON,
        L"", 0/*L"Start new game"*/);
    button->setImage(button_bg_sel_new);
    button->setScaleImage(true);
    button->setUseAlphaChannel(true);
    line += diff;
    
    button = env->addButton(rect<s32>(indist, line, indist+buttonWidth, line+buttonHeight),
        window, GUI_ID_SEL_NEXT_BUTTON,
        L"", 0/*L"Start new game"*/);
    button->setImage(button_bg_sel_next);
    button->setScaleImage(true);
    button->setUseAlphaChannel(true);
    line += diff;
    
    button = env->addButton(rect<s32>(indist, line, indist+buttonWidth, line+buttonHeight),
        window, GUI_ID_SEL_PREV_BUTTON,
        L"", 0/*L"Start new game"*/);
    button->setImage(button_bg_sel_prev);
    button->setScaleImage(true);
    button->setUseAlphaChannel(true);
    line += diff;
    
    button = env->addButton(rect<s32>(indist, line, indist+buttonWidth, line+buttonHeight),
        window, GUI_ID_SEL_BACK_BUTTON,
        L"", 0/*L"Start new game"*/);
    button->setImage(button_bg_sel_back);
    button->setScaleImage(true);
    button->setUseAlphaChannel(true);
    line += diff;
    
    int outdistH = (screenSize.Height - (line + indist)) / 2;
    //window->setRelativePosition(
	//	rect<s32>(outdist, outdistH, screenSize.Width - outdist, outdistH + line + indist));
    
    core::stringw str = L" ";
    str = vehiclePool->getName(carType);
    
    car_selector_text = env->addStaticText(str.c_str(),
		rect<s32>(indist*2+buttonWidth,startLine-20,indist*2+buttonWidth+300,startLine-4),
		false, // border?
		false, // wordwrap?
		window,
        -1,
        true);
        
    core::rect<int> messageRect = car_selector_text->getRelativePosition();
    messageRect.LowerRightCorner.X = messageRect.UpperLeftCorner.X + car_selector_text->getTextWidth();
    car_selector_text->setRelativePosition(messageRect);

    bg_frame = env->addImage(core::rect<int>(indist*2+buttonWidth, startLine ,
                                             screenSize.Width - indist, screenSize.Height - indist),
                            window, -1, L"bg_frame_select");
    bg_frame->setScaleImage(true);
    bg_frame->setImage(car_selector_rtt);
    //bg_frame->setImage(driver->getTexture("data/bg/dakar_bg1.jpg"));
    bg_frame->setUseAlphaChannel(true);
    
    ISceneNodeAnimator* animator = smgr->createFlyCircleAnimator(core::vector3df(0.f, 1.7f, 0.f), 4.0f, 0.001f);
    car_selector_camera->addAnimator(animator);

    //bgImage->setImage(car_selector_rtt);
    shadowMap = shadowMapMenu;
    
    printf("create car: %d\n", vehiclePool->getVehicleTypesSize());
    car_to_draw = vehiclePool->getVehicle(carType);
    printf("create car 2\n");
    assert(car_to_draw);
    printf("create car 3\n");
    car_to_draw->activate(core::vector3df(0.f,0.f,0.f),
                          core::vector3df(0.f,0.f,0.f), 
                          "", "", "",
                          0.5,
                          skydome,
                          shadowMap);
    printf("create car end\n");
    //NewtonUpdate(nWorld, 0.015f);
    //printf("update car end\n");
    car_to_draw->pause();

    refreshActiveElements();

    playSound(openSound);
    
    //printf("me w %d %d %d %p\n", mainWindowOpened, optionsWindowOpened, helpWindowOpened, window);
}	

void eventreceiver_menu::openOptionsWindow()
{
   const int outdist = (screenSize.Width - 750) / 2;//30;
   const int indist = 30;
   const int firsttextlen = 130;
   const int valuelen = 50;
   int line;
   int maxLine = 0;
   const int startLine = 10;
   core::stringw str = L" ";

   //printf("ob w %d %d %d %p\n", mainWindowOpened, optionsWindowOpened, helpWindowOpened, window);

   if (optionsWindowOpened || loading > 0) return;
   playSound(openSound);

   optionsWindowOpened = true;
   if (inGame == 0)
   {
       other = (eventreceiver*)device->getEventReceiver();
       device->setEventReceiver(this);
       pauseGame();
   }
   inGame++;
   
   firstJoyState = true;

   optionsWindowP = window;
   
   window = optionsWindow = env->addWindow(
		rect<s32>(outdist, outdist, screenSize.Width - outdist, screenSize.Height - outdist),
		false, // modal?
		L"  Options", 0, GUI_ID_OPTIONS_WINDOW);
    env->setFocus(window);
		
	tabControl = env->addTabControl(
		rect<s32>(indist/2, indist+20, screenSize.Width - (outdist*2) - indist/2, screenSize.Height - (outdist*2) - indist/2),
		window,
		true, // fillbg
		true // frame
    );
    
    IGUITab* gameTab = tabControl->addTab(L"Game");
    IGUITab* graphicTab = tabControl->addTab(L"Graphic");
    IGUITab* graphic2Tab = tabControl->addTab(L"Graphic2");
    IGUITab* networkTab = tabControl->addTab(L"Network");
    IGUITab* joyTab = tabControl->addTab(L"Controller");

// ------------------------
//  Game tab
// ------------------------
    line = startLine;
/*
	env->addStaticText(L"Car",
		rect<s32>(indist,line,indist+firsttextlen,line+16),
		false, // border?
		false, // wordwrap?
		gameTab);
	carfilename_cbox = env->addComboBox(
		rect<s32>(indist*2+firsttextlen, line, screenSize.Width - (indist*3+outdist*2+valuelen), line+16),
		gameTab);
    for (int j = 0; j < carList.length(); j++)
    {
        str = carList[j]->carName;
        str.replace(L'_', L' ');
        carfilename_cbox->addItem(str.c_str());
    }
    carfilename_cbox->setSelected(carType);
*/
	/*
	carfilename_text = env->addEditBox(str.c_str(),
		rect<s32>(indist*2+firsttextlen, line, screenSize.Width - (indist*2+outdist*2+valuelen), line+16),
		false, // border?
		window);
    env->addButton(
        rect<s32>(screenSize.Width - (indist+outdist*2+valuelen),line,screenSize.Width - (indist+outdist*2+valuelen/2+indist/2),line+16),
        window, GUI_ID_CHOOSE_CAR_BUTTON,
        L"..", L"Open a car");
    */
/*    env->addButton(
        rect<s32>(screenSize.Width - (indist*2+outdist*2+valuelen),line,screenSize.Width - (indist*2+outdist*2),line+16),
//            rect<s32>(screenSize.Width - (indist+outdist*2+valuelen/2),line,screenSize.Width - (indist+outdist*2),line+16),
        gameTab, GUI_ID_APPLY_CAR_BUTTON,
        L"Set", 0);
        
    line += 40;
*/	env->addStaticText(L"Draw hud",
		rect<s32>(indist,line,indist+firsttextlen,line+16),
		false, // border?
		false, // wordwrap?
		gameTab);
	env->addCheckBox(draw_hud,
		rect<s32>(indist*2+firsttextlen, line, indist*2+firsttextlen+16, line+16),
		gameTab, GUI_ID_DRAW_HUD, L"Hello2");

    line += 20;
	env->addStaticText(L"Auto gear",
		rect<s32>(indist,line,indist+firsttextlen,line+16),
		false, // border?
		false, // wordwrap?
		gameTab);
	env->addCheckBox(gear_type=='a',
		rect<s32>(indist*2+firsttextlen, line, indist*2+firsttextlen+16, line+16),
		gameTab, GUI_ID_GEAR_TYPE_AUTO, L"Hello12");

/*    line += 20;
	env->addStaticText(L"Show compass arrow",
		rect<s32>(indist,line,indist+firsttextlen,line+16),
		false, // border?
		false, // wordwrap?
		gameTab);
	env->addCheckBox(show_compass_arrow,
		rect<s32>(indist*2+firsttextlen, line, indist*2+firsttextlen+16, line+16),
		gameTab, GUI_ID_SHOW_COMPASS_ARROW, L"Hello38");
	env->addStaticText(L"(If the compass is active.)",
		rect<s32>(indist*3+firsttextlen, line, screenSize.Width - (indist*2+outdist*2), line+16),
		false, // border?
		false, // wordwrap?
		gameTab);
*/
    line += 20;
	env->addStaticText(L"Show names",
		rect<s32>(indist,line,indist+firsttextlen,line+16),
		false, // border?
		false, // wordwrap?
		gameTab);
	env->addCheckBox(show_names,
		rect<s32>(indist*2+firsttextlen, line, indist*2+firsttextlen+16, line+16),
		gameTab, GUI_ID_SHOW_NAMES, L"Hello75");

	line += 40;
	env->addStaticText(L"Gravity",
		rect<s32>(indist,line,indist+firsttextlen,line+16),
		false, // border?
		false, // wordwrap?
		gameTab);
	IGUIScrollBar* scrollbar = env->addScrollBar(true,
			rect<s32>(indist*2+firsttextlen, line, screenSize.Width - (indist*3+outdist*2+valuelen), line+16),
            gameTab, GUI_ID_GRAVITY_SCROLL_BAR);
	scrollbar->setMax(150);
    scrollbar->setSmallStep(1);
    scrollbar->setLargeStep(5);
	// set scrollbar position to alpha value of an arbitrary element
	scrollbar->setPos((int)gravity+100);
	str = L" ";
	str += (int)gravity;
	gravity_text = env->addStaticText(str.c_str(),
		rect<s32>(screenSize.Width - (indist*2+outdist*2+valuelen),line,screenSize.Width - (indist*2+outdist*2),line+16),
		false, // border?
		false, // wordwrap?
		gameTab);
    
    if (line>maxLine) maxLine = line;

// ------------------------
//  Graphic tab
// ------------------------

    line = startLine;
/*
	env->addStaticText(L"Object visibility limit",
		rect<s32>(indist,line,indist+firsttextlen,line+16),
		false, // border?
		false, // wordwrap?
		graphicTab);
	scrollbar = env->addScrollBar(true,
			rect<s32>(indist*2+firsttextlen, line, screenSize.Width - (indist*3+outdist*2+valuelen), line+16),
            graphicTab, GUI_ID_OV_SCROLL_BAR);
	//scrollbar->setMin(1);
	if (bigTerrain)
	{
	   scrollbar->setMax((int)bigTerrain->getSmallTerrainSize()-1);
    }
    else
    {
	   scrollbar->setMax(2000);
    }
	str = L"";
	scrollbar->setPos((int)objectVisibilityLimit);
	str += (int)objectVisibilityLimit;
	ov_limit_text = env->addStaticText(str.c_str(),
		rect<s32>(screenSize.Width - (indist*2+outdist*2+valuelen),line,screenSize.Width - (indist*2+outdist*2),line+16),
		false, // border?
		false, // wordwrap?
		graphicTab);

	line += 20;
*/
	env->addStaticText(L"Transparency",
		rect<s32>(indist,line,indist+firsttextlen,line+16),
		false, // border?
		false, // wordwrap?
		graphicTab);
	scrollbar = env->addScrollBar(true,
			rect<s32>(indist*2+firsttextlen, line, screenSize.Width - (indist*3+outdist*2+valuelen), line+16),
            graphicTab, GUI_ID_TRANSPARENCY_SCROLL_BAR);
	scrollbar->setMax(255);
    scrollbar->setSmallStep(1);
    scrollbar->setLargeStep(5);
	// set scrollbar position to alpha value of an arbitrary element
	scrollbar->setPos(env->getSkin()->getColor(EGDC_WINDOW).getAlpha());
	str = L" ";
	str += env->getSkin()->getColor(EGDC_WINDOW).getAlpha();
	transp_text = env->addStaticText(str.c_str(),
		rect<s32>(screenSize.Width - (indist*2+outdist*2+valuelen),line,screenSize.Width - (indist*2+outdist*2),line+16),
		false, // border?
		false, // wordwrap?
		graphicTab);
	
	line += 40;
	env->addStaticText(L"Smoke",
		rect<s32>(indist,line,indist+firsttextlen,line+16),
		false, // border?
		false, // wordwrap?
		graphicTab);
	env->addCheckBox(useSmokes,
		rect<s32>(indist*2+firsttextlen, line, indist*2+firsttextlen+16, line+16),
		graphicTab, GUI_ID_USE_SMOKES, L"Hello");



	line += 40;
	env->addStaticText(L"View distance",
		rect<s32>(indist,line,indist+firsttextlen,line+16),
		false, // border?
		false, // wordwrap?
		graphicTab);
	scrollbar = env->addScrollBar(true,
			rect<s32>(indist*2+firsttextlen, line, screenSize.Width - (indist*3+outdist*2+valuelen), line+16),
            graphicTab, GUI_ID_VIEW_DISTANCE_SCROLL_BAR);
	//scrollbar->setMin(1);
	scrollbar->setMax(10000);
    scrollbar->setSmallStep(10);
    scrollbar->setLargeStep(50);
	// set scrollbar position to alpha value of an arbitrary element
	scrollbar->setPos((int)farValue);
	str = L" ";
	str += (int)farValue;
	view_distance_text = env->addStaticText(str.c_str(),
		rect<s32>(screenSize.Width - (indist*2+outdist*2+valuelen),line,screenSize.Width - (indist*2+outdist*2),line+16),
		false, // border?
		false, // wordwrap?
		graphicTab);

	line += 40;
	env->addStaticText(L"Object settings (restart require)",
		rect<s32>(indist,line,screenSize.Width - (indist*2+outdist*2),line+16),
		false, // border?
		false, // wordwrap?
		graphicTab);
    line += 25;
	env->addStaticText(L"Object density",
		rect<s32>(indist,line,indist+firsttextlen,line+16),
		false, // border?
		false, // wordwrap?
		graphicTab);
	scrollbar = env->addScrollBar(true,
			rect<s32>(indist*2+firsttextlen, line, screenSize.Width - (indist*3+outdist*2+valuelen), line+16),
            graphicTab, GUI_ID_VIEW_OBJ_DENSITY_SCROLL_BAR);
	//scrollbar->setMin(1);
	scrollbar->setMax(100);
    scrollbar->setSmallStep(1);
    scrollbar->setLargeStep(5);
	scrollbar->setPos(density_objects);
	str = L" ";
	str += density_objects;
	obj_density_text = env->addStaticText(str.c_str(),
		rect<s32>(screenSize.Width - (indist*2+outdist*2+valuelen),line,screenSize.Width - (indist*2+outdist*2),line+16),
		false, // border?
		false, // wordwrap?
		graphicTab);
/*		
	line += 20;
	env->addStaticText(L"Grass density",
		rect<s32>(indist,line,indist+firsttextlen,line+16),
		false, // border?
		false, // wordwrap?
		graphicTab);
	scrollbar = env->addScrollBar(true,
			rect<s32>(indist*2+firsttextlen, line, screenSize.Width - (indist*3+outdist*2+valuelen), line+16),
            graphicTab, GUI_ID_VIEW_GRA_DENSITY_SCROLL_BAR);
	//scrollbar->setMin(1);
	scrollbar->setMax(100);
    scrollbar->setSmallStep(1);
    scrollbar->setLargeStep(5);
	scrollbar->setPos(density_grasses);
	str = L" ";
	str += density_grasses;
	gra_density_text = env->addStaticText(str.c_str(),
		rect<s32>(screenSize.Width - (indist*2+outdist*2+valuelen),line,screenSize.Width - (indist*2+outdist*2),line+16),
		false, // border?
		false, // wordwrap?
		graphicTab);
*/
    if (line>maxLine) maxLine = line;

// ------------------------
//  Graphic2 tab
// ------------------------
    line = startLine;
    /*
	env->addStaticText(L"Changing these settings will be applied after save and restart",
		rect<s32>(indist,line,screenSize.Width - (indist*2+outdist*2),line+16),
		false, // border?
		false, // wordwrap?
		graphic2Tab);

    line += 30;
    */
	env->addStaticText(L"Device settings (restart require)",
		rect<s32>(indist,line,screenSize.Width - (indist*2+outdist*2),line+16),
		false, // border?
		false, // wordwrap?
		graphic2Tab);
    line += 25;
	env->addStaticText(L"Driver type",
		rect<s32>(indist,line,indist+firsttextlen,line+16),
		false, // border?
		false, // wordwrap?
		graphic2Tab);
	driverType_cbox = env->addComboBox(
		rect<s32>(indist*2+firsttextlen, line, screenSize.Width - (indist*3+outdist*2+valuelen), line+16),
		graphic2Tab,
        GUI_ID_DRIVERTYPE_CBOX);
    driverType_cbox->addItem(L"D3D9");
    driverType_cbox->addItem(L"D3D8");
    if (driverType == video::EDT_DIRECT3D8) driverType_cbox->setSelected(1);
    driverType_cbox->addItem(L"OpenGL");
    if (driverType == video::EDT_OPENGL) driverType_cbox->setSelected(2);
    driverType_cbox->addItem(L"Software");
    if (driverType == video::EDT_BURNINGSVIDEO) driverType_cbox->setSelected(3);
        
    line += 20;
	env->addStaticText(L"Resolution",
		rect<s32>(indist,line,indist+firsttextlen,line+16),
		false, // border?
		false, // wordwrap?
		graphic2Tab);
	resolution_cbox = env->addComboBox(
		rect<s32>(indist*2+firsttextlen, line, screenSize.Width - (indist*3+outdist*2+valuelen), line+16),
		graphic2Tab,
        GUI_ID_RESOLUTION_CBOX);
    for(int i=0;i<device->getVideoModeList()->getVideoModeCount();i++)
    {
        str = L"";
#ifdef IRRLICHT_SDK_15
        core::dimension2d<s32> res = device->getVideoModeList()->getVideoModeResolution(i);
#else
        core::dimension2d<u32> res = device->getVideoModeList()->getVideoModeResolution(i);
#endif
        s32 dep = device->getVideoModeList()->getVideoModeDepth(i);
        str += res.Width;
        str += L"x";
        str += res.Height;
        str += L"x";
        str += dep;
        resolution_cbox->addItem(str.c_str());
        if (resolutionX==res.Width && resolutionY==res.Height && display_bits==dep) resolution_cbox->setSelected(i);
    }
    /*
    resolution_cbox->addItem(L"320x240");
    resolution_cbox->addItem(L"640x480");
    if (resolutionX==640 && resolutionY==480 ) resolution_cbox->setSelected(1);
    resolution_cbox->addItem(L"800x600");
    if (resolutionX==800 && resolutionY==600 ) resolution_cbox->setSelected(2);
    resolution_cbox->addItem(L"1024x768");
    if (resolutionX==1024 && resolutionY==768 ) resolution_cbox->setSelected(3);
    resolution_cbox->addItem(L"1280x1024");
    if (resolutionX==1280 && resolutionY==1024 ) resolution_cbox->setSelected(4);
    resolution_cbox->addItem(L"1400x1050");
    if (resolutionX==1400 && resolutionY==1050 ) resolution_cbox->setSelected(5);
    resolution_cbox->addItem(L"1600x1200");
    if (resolutionX==1600 && resolutionY==1200 ) resolution_cbox->setSelected(6);
    resolution_cbox->addItem(L"1280x720");
    if (resolutionX==1280 && resolutionY==720 ) resolution_cbox->setSelected(7);
    resolution_cbox->addItem(L"1280x768");
    if (resolutionX==1280 && resolutionY==768) resolution_cbox->setSelected(8);
    resolution_cbox->addItem(L"1280x800");
    if (resolutionX==1280 && resolutionY==800 ) resolution_cbox->setSelected(9);
    resolution_cbox->addItem(L"1680x1050");
    if (resolutionX==1680 && resolutionY==1050 ) resolution_cbox->setSelected(10);
    resolution_cbox->addItem(L"1920x1080");
    if (resolutionX==1920 && resolutionY==1080 ) resolution_cbox->setSelected(11);
    resolution_cbox->addItem(L"1920x1200");
    if (resolutionX==1920 && resolutionY==1200 ) resolution_cbox->setSelected(12);
    
    line += 20;
	env->addStaticText(L"Display bits",
		rect<s32>(indist,line,indist+firsttextlen,line+16),
		false, // border?
		false, // wordwrap?
		graphic2Tab);
	display_bits_cbox = env->addComboBox(
		rect<s32>(indist*2+firsttextlen, line, screenSize.Width - (indist*3+outdist*2+valuelen), line+16),
		graphic2Tab,
        GUI_ID_DISPLAY_BITS_CBOX);
    display_bits_cbox->addItem(L"16");
    display_bits_cbox->addItem(L"32");
    if (display_bits==32) display_bits_cbox->setSelected(1);
    */
    
    line += 20;
	env->addStaticText(L"Full screen",
		rect<s32>(indist,line,indist+firsttextlen,line+16),
		false, // border?
		false, // wordwrap?
		graphic2Tab);
	env->addCheckBox(full_screen,
		rect<s32>(indist*2+firsttextlen, line, indist*2+firsttextlen+16, line+16),
		graphic2Tab, GUI_ID_FULL_SCREEN, L"Hello3");

    line += 20;
	env->addStaticText(L"Auto-detection",
		rect<s32>(indist,line,indist+firsttextlen,line+16),
		false, // border?
		false, // wordwrap?
		graphic2Tab);
	env->addCheckBox(auto_resolution,
		rect<s32>(indist*2+firsttextlen, line, indist*2+firsttextlen+16, line+16),
		graphic2Tab, GUI_ID_AUTO_RES, L"Hello42");
	env->addStaticText(L"(Only works in full screen mode. Automatically detects the display resolution.)",
		rect<s32>(indist*3+firsttextlen, line, screenSize.Width - (indist*2+outdist*2), line+16),
		false, // border?
		false, // wordwrap?
		graphic2Tab);

    line += 20;
	env->addStaticText(L"Anti aliasing",
		rect<s32>(indist,line,indist+firsttextlen,line+16),
		false, // border?
		false, // wordwrap?
		graphic2Tab);
	env->addCheckBox(anti_aliasing,
		rect<s32>(indist*2+firsttextlen, line, indist*2+firsttextlen+16, line+16),
		graphic2Tab, GUI_ID_ANTI_ALIASING, L"Hello4");

    line += 20;
	env->addStaticText(L"Vsync",
		rect<s32>(indist,line,indist+firsttextlen,line+16),
		false, // border?
		false, // wordwrap?
		graphic2Tab);
	env->addCheckBox(vsync,
		rect<s32>(indist*2+firsttextlen, line, indist*2+firsttextlen+16, line+16),
		graphic2Tab, GUI_ID_VSYNC, L"Hello5");

    line += 20;
	env->addStaticText(L"Shadows",
		rect<s32>(indist,line,indist+firsttextlen,line+16),
		false, // border?
		false, // wordwrap?
		graphic2Tab);
	env->addCheckBox(shadows,
		rect<s32>(indist*2+firsttextlen, line, indist*2+firsttextlen+16, line+16),
		graphic2Tab, GUI_ID_SHADOWS, L"Hello8");
/*	env->addStaticText(L"(Only works with high effects detail)",
		rect<s32>(indist*3+firsttextlen, line, screenSize.Width - (indist*2+outdist*2), line+16),
		false, // border?
		false, // wordwrap?
		graphic2Tab);

    line += 20;
	env->addStaticText(L"Stencil Shadow",
		rect<s32>(indist,line,indist+firsttextlen,line+16),
		false, // border?
		false, // wordwrap?
		graphic2Tab);
	env->addCheckBox(stencil_shadows,
		rect<s32>(indist*2+firsttextlen, line, indist*2+firsttextlen+16, line+16),
		graphic2Tab, GUI_ID_STENCIL_SHADOWS, L"Hello15");
	env->addStaticText(L"(Works with low effects detail or above, but give some performace drop)",
		rect<s32>(indist*3+firsttextlen, line, screenSize.Width - (indist*2+outdist*2), line+16),
		false, // border?
		false, // wordwrap?
		graphic2Tab);
*/
/*
    line += 20;
	env->addStaticText(L"High precision fpu",
		rect<s32>(indist,line,indist+firsttextlen,line+16),
		false, // border?
		false, // wordwrap?
		graphic2Tab);
	env->addCheckBox(high_precision_fpu,
		rect<s32>(indist*2+firsttextlen, line, indist*2+firsttextlen+16, line+16),
		graphic2Tab, GUI_ID_HIGH_PRECISION_FPU, L"Hello6");
	env->addStaticText(L"(Only for D3D.)",
		rect<s32>(indist*3+firsttextlen, line, screenSize.Width - (indist*2+outdist*2), line+16),
		false, // border?
		false, // wordwrap?
		graphic2Tab);
*/
    line += 30;
	env->addStaticText(L"Effects (restart require)",
		rect<s32>(indist,line,screenSize.Width - (indist*2+outdist*2),line+16),
		false, // border?
		false, // wordwrap?
		graphic2Tab);
    line += 25;
	env->addStaticText(L"Terrain detail",
		rect<s32>(indist,line,indist+firsttextlen,line+16),
		false, // border?
		false, // wordwrap?
		graphic2Tab);
	LOD_distance_cbox = env->addComboBox(
		rect<s32>(indist*2+firsttextlen, line, screenSize.Width - (indist*3+outdist*2+valuelen), line+16),
		graphic2Tab,
        GUI_ID_LOD_DISTANCE_CBOX);
    LOD_distance_cbox->addItem(L"Low (9)");
    LOD_distance_cbox->addItem(L"Medium (17)");
    if (LOD_distance==17) LOD_distance_cbox->setSelected(1);
    LOD_distance_cbox->addItem(L"Hight (33)");
    if (LOD_distance==33) LOD_distance_cbox->setSelected(2);

    line += 20;
	env->addStaticText(L"Shadow detail",
		rect<s32>(indist,line,indist+firsttextlen,line+16),
		false, // border?
		false, // wordwrap?
		graphic2Tab);
	shadow_map_size_cbox = env->addComboBox(
		rect<s32>(indist*2+firsttextlen, line, screenSize.Width - (indist*3+outdist*2+valuelen), line+16),
		graphic2Tab,
        GUI_ID_SHADOW_MAP_SIZE_CBOX);
    shadow_map_size_cbox->addItem(L"Very low (256)");
    if (shadow_map_size==256) shadow_map_size_cbox->setSelected(0);
    shadow_map_size_cbox->addItem(L"Low (512)");
    if (shadow_map_size==512) shadow_map_size_cbox->setSelected(1);
    shadow_map_size_cbox->addItem(L"Medium (1024)");
    if (shadow_map_size==1024) shadow_map_size_cbox->setSelected(1);
    shadow_map_size_cbox->addItem(L"High (2048)");
    if (shadow_map_size==2048) shadow_map_size_cbox->setSelected(3);
    shadow_map_size_cbox->addItem(L"Very high (4096 - require strong hardware)");
    if (shadow_map_size==4096) shadow_map_size_cbox->setSelected(4);
/*
    line += 20;
	env->addStaticText(L"Effects detail",
		rect<s32>(indist,line,indist+firsttextlen,line+16),
		false, // border?
		false, // wordwrap?
		graphic2Tab);
	effects_cbox = env->addComboBox(
		rect<s32>(indist*2+firsttextlen, line, screenSize.Width - (indist*3+outdist*2+valuelen), line+16),
		graphic2Tab,
        GUI_ID_EFFECTS_CBOX);
    effects_cbox->addItem(L"None");
    if (globalLight == false && useShaders == false && useCgShaders == false)
        effects_cbox->setSelected(0);
    effects_cbox->addItem(L"Low - Basic light, night");
    if (globalLight == true && useShaders == false && useCgShaders == false)
        effects_cbox->setSelected(1);
    effects_cbox->addItem(L"Medium - Nicer terrain and light, car dirt, car reflection, no night");
    if (globalLight == true && useShaders == true && useCgShaders == false)
        effects_cbox->setSelected(2);
    effects_cbox->addItem(L"High - Night, car light, car dirt, car reflection and shadow effects");
    if (globalLight == true && useShaders == true && useCgShaders == true)
        effects_cbox->setSelected(3);
*/
    line += 20;
	env->addStaticText(L"Texture detail",
		rect<s32>(indist,line,indist+firsttextlen,line+16),
		false, // border?
		false, // wordwrap?
		graphic2Tab);
	texturedetail_cbox = env->addComboBox(
		rect<s32>(indist*2+firsttextlen, line, screenSize.Width - (indist*3+outdist*2+valuelen), line+16),
		graphic2Tab,
        GUI_ID_TEXTUREDETAIL_CBOX);
    texturedetail_cbox->addItem(L"Low");
    if (use_detailed_terrain == false)
        texturedetail_cbox->setSelected(0);
    texturedetail_cbox->addItem(L"Medium");
    if (use_detailed_terrain == true && use_highres_textures == false)
        texturedetail_cbox->setSelected(1);
    texturedetail_cbox->addItem(L"High");
    if (use_detailed_terrain == true && use_highres_textures == true)
        texturedetail_cbox->setSelected(2);
	texturedetail_text = env->addStaticText(L"Only available at medium and high effects detail level",
		rect<s32>(indist*2+firsttextlen, line, screenSize.Width - (indist*3+outdist*2+valuelen), line+16),
		false, // border?
		false, // wordwrap?
		graphic2Tab);
    if (useShaders)
        texturedetail_text->setVisible(false);
    else
        texturedetail_cbox->setVisible(false);

/*
    line += 20;
	env->addStaticText(L"Lights",
		rect<s32>(indist,line,indist+firsttextlen,line+16),
		false, // border?
		false, // wordwrap?
		graphic2Tab);
	env->addCheckBox(globalLight,
		rect<s32>(indist*2+firsttextlen, line, indist*2+firsttextlen+16, line+16),
		graphic2Tab, GUI_ID_LIGHT, L"Hello7");

    line += 20;
	env->addStaticText(L"Shaders",
		rect<s32>(indist,line,indist+firsttextlen,line+16),
		false, // border?
		false, // wordwrap?
		graphic2Tab);
	env->addCheckBox(useShaders,
		rect<s32>(indist*2+firsttextlen, line, indist*2+firsttextlen+16, line+16),
		graphic2Tab, GUI_ID_USESHADERS, L"Hello9");
	env->addStaticText(L"(Nicer terrain and light)",
		rect<s32>(indist*3+firsttextlen, line, screenSize.Width - (indist*2+outdist*2), line+16),
		false, // border?
		false, // wordwrap?
		graphic2Tab);

    line += 20;
	env->addStaticText(L"Cg shaders",
		rect<s32>(indist,line,indist+firsttextlen,line+16),
		false, // border?
		false, // wordwrap?
		graphic2Tab);
	env->addCheckBox(useCgShaders,
		rect<s32>(indist*2+firsttextlen, line, indist*2+firsttextlen+16, line+16),
		graphic2Tab, GUI_ID_USECGSHADERS, L"Hello10");
	env->addStaticText(L"(Night, car light, car dirt and shadow effects. Only works with shaders.)",
		rect<s32>(indist*3+firsttextlen, line, screenSize.Width - (indist*2+outdist*2), line+16),
		false, // border?
		false, // wordwrap?
		graphic2Tab);
*/
    line += 30;
	env->addStaticText(L"RTT - Post effects",
		rect<s32>(indist,line,screenSize.Width - (indist*2+outdist*2),line+16),
		false, // border?
		false, // wordwrap?
		graphic2Tab);
    line += 25;
	env->addStaticText(L"Use post effects",
		rect<s32>(indist,line,indist+firsttextlen,line+16),
		false, // border?
		false, // wordwrap?
		graphic2Tab);
	env->addCheckBox(useScreenRTT,
		rect<s32>(indist*2+firsttextlen, line, indist*2+firsttextlen+16, line+16),
		graphic2Tab, GUI_ID_USE_SCREEN_RTT, L"Hello16");
	env->addStaticText(L"(For some special effects.)",
		rect<s32>(indist*3+firsttextlen, line, screenSize.Width - (indist*2+outdist*2), line+16),
		false, // border?
		false, // wordwrap?
		graphic2Tab);
/*
    line += 20;
	env->addStaticText(L"Depth effect",
		rect<s32>(indist,line,indist+firsttextlen,line+16),
		false, // border?
		false, // wordwrap?
		graphic2Tab);
	env->addCheckBox(depth_effect,
		rect<s32>(indist*2+firsttextlen, line, indist*2+firsttextlen+16, line+16),
		graphic2Tab, GUI_ID_USE_DEPTH_RTT, L"Hello18");
	env->addStaticText(L"(Only works with screen RTT.)",
		rect<s32>(indist*3+firsttextlen, line, screenSize.Width - (indist*2+outdist*2), line+16),
		false, // border?
		false, // wordwrap?
		graphic2Tab);
*/
    line += 20;
	env->addStaticText(L"ATI card",
		rect<s32>(indist,line,indist+firsttextlen,line+16),
		false, // border?
		false, // wordwrap?
		graphic2Tab);
	env->addCheckBox(shitATI,
		rect<s32>(indist*2+firsttextlen, line, indist*2+firsttextlen+16, line+16),
		graphic2Tab, GUI_ID_SHIT_ATI, L"Hello17");
	env->addStaticText(L"(If you have some extereme slowness with post effects, turn this on.)",
		rect<s32>(indist*3+firsttextlen, line, screenSize.Width - (indist*2+outdist*2), line+16),
		false, // border?
		false, // wordwrap?
		graphic2Tab);
/*
    line += 20;
	env->addStaticText(L"Flip vertical",
		rect<s32>(indist,line,indist+firsttextlen,line+16),
		false, // border?
		false, // wordwrap?
		graphic2Tab);
	env->addCheckBox(flip_vert,
		rect<s32>(indist*2+firsttextlen, line, indist*2+firsttextlen+16, line+16),
		graphic2Tab, GUI_ID_FLIP_VERT, L"Hello20");
*/

    line += 10;

    if (line>maxLine) maxLine = line;

// ------------------------
//  Network tab
// ------------------------
    line = startLine;
	env->addStaticText(L"Server (addr./port)",
		rect<s32>(indist,line,indist+firsttextlen,line+16),
		false, // border?
		false, // wordwrap?
		networkTab);
	int portW = 100;
	str = server_name;
	server_name_text = env->addEditBox(str.c_str(),
		rect<s32>(indist*2+firsttextlen, line, screenSize.Width - (indist*4+outdist*2+valuelen+portW), line+16),
		true, // border?
		networkTab,
        GUI_ID_SERVER_NAME_EBOX);
	str = L"";
    str += server_port;
	server_port_text = env->addEditBox(str.c_str(),
		rect<s32>(screenSize.Width - (indist*3+outdist*2+valuelen+portW), line, screenSize.Width - (indist*3+outdist*2+valuelen), line+16),
		true, // border?
		networkTab,
        GUI_ID_SERVER_PORT_EBOX);
	if (isMultiplayer)
	  str = L"Discon.";
	else
	  str = L"Conn.";
    env->addButton(
        rect<s32>(screenSize.Width - (indist*2+outdist*2+valuelen),line,screenSize.Width - (indist*2+outdist*2),line+16),
        networkTab, GUI_ID_CONNECT_BUTTON,
        str.c_str(), L"connect/disconnect");
    
    line += 20;
	env->addStaticText(L"Send server delay",
		rect<s32>(indist,line,indist+firsttextlen,line+16),
		false, // border?
		false, // wordwrap?
		networkTab);
	str = L"";
	str += send_server_delay;
	server_delay_text = env->addEditBox(str.c_str(),
		rect<s32>(indist*2+firsttextlen, line, screenSize.Width - (indist*4+outdist*2+valuelen+portW), line+16),
		true, // border?
		networkTab,
        GUI_ID_SERVER_DELAY_EBOX);

    line += 20;
	env->addStaticText(L"Network messages",
		rect<s32>(indist,line,indist+firsttextlen,line+16),
		false, // border?
		false, // wordwrap?
		networkTab);
	env->addCheckBox(trace_net,
		rect<s32>(indist*2+firsttextlen, line, indist*2+firsttextlen+16, line+16),
		networkTab, GUI_ID_TRACE_NET, L"Hello30");

    if (line>maxLine) maxLine = line;

// ------------------------
//  joy tab
// ------------------------
    line = startLine;
	env->addStaticText(L"Select joystic options, then push the button or push pedal or steer the wheel, or press DEL to remove assoc.",
		rect<s32>(indist,line,screenSize.Width - (indist*2+outdist*2),line+16),
		false, // border?
		false, // wordwrap?
		joyTab);
    line += 20;
	env->addStaticText(L"Before set, set all axis to center and push calibrate, then you should see the wait... or w...",
		rect<s32>(indist,line,screenSize.Width - (indist*2+outdist*2),line+16),
		false, // border?
		false, // wordwrap?
		joyTab);
    env->addButton(
        rect<s32>(screenSize.Width - (indist*2+outdist*2+valuelen*2),line,screenSize.Width - (indist*2+outdist*2),line+16),
//            rect<s32>(screenSize.Width - (indist+outdist*2+valuelen/2),line,screenSize.Width - (indist+outdist*2),line+16),
        joyTab, GUI_ID_CALIBRATE_BUTTON,
        L"Calibrate", 0/*L"Push if all axis are in center position"*/);

	core::array<SJoystickInfo> joystickInfo;
	if(device->activateJoysticks(joystickInfo))
	{
        str = L"Joystick support is enabled and ";
        str += joystickInfo.size();
		str += " joystick(s) are present";

        line += 40;
    	env->addStaticText(str.c_str(),
    		rect<s32>(indist,line,screenSize.Width - (indist*2+outdist*2),line+16),
    		false, // border?
    		false, // wordwrap?
    		joyTab);

        if (joystickInfo.size()==0)
        {
    		joy_axis_clutch = -1;
    		joys_cbox = 0;
        }
    	else
    	{
            line += 30;
        	joys_cbox = env->addComboBox(
		      rect<s32>(indist, line, screenSize.Width - (indist*2+outdist*2), line+16),
		      joyTab,
              GUI_ID_JOYS_CBOX);

    		for(u32 joystick = 0; joystick < joystickInfo.size(); ++joystick)
    		{
                str = L"";
                str += joystick + 1;
                str += L". ";
                str += joystickInfo[joystick].Name;
                /*
            	env->addStaticText(str.c_str(),
            		rect<s32>(indist,line,screenSize.Width - (indist*2+outdist*2),line+16),
            		false, // border?
            		false, // wordwrap?
            		joyTab);
            	*/
            	str += L" | Axes: ";
            	str += joystickInfo[joystick].Axes;
            	str += L" | Buttons: ";
                str += joystickInfo[joystick].Buttons;
    			str += L" | Hat is ";
    			switch(joystickInfo[joystick].PovHat)
    			{
    			case SJoystickInfo::POV_HAT_PRESENT:
    				str += L"present";
    				break;
    
    			case SJoystickInfo::POV_HAT_ABSENT:
    				str += L"absent";
    				break;
    
    			case SJoystickInfo::POV_HAT_UNKNOWN:
    			default:
    				str += L"unknown\n";
    				break;
    			}
    			joys_cbox->addItem(str.c_str());
                if (joystick == activeJoystick) joys_cbox->setSelected(joystick);
    		}
        }
	}
	else
	{
		str = L"Joystick support is not enabled.";
		joy_axis_clutch = -1;
        line += 40;
    	env->addStaticText(str.c_str(),
    		rect<s32>(indist,line,screenSize.Width - (indist*2+outdist*2),line+16),
    		false, // border?
    		false, // wordwrap?
    		joyTab);
	}

    line += 40;
	env->addStaticText(L"Control items",
		rect<s32>(indist,line,indist+firsttextlen,line+16),
		false, // border?
		false, // wordwrap?
		joyTab);
	joy_cbox = env->addComboBox(
		rect<s32>(indist*2+firsttextlen, line, screenSize.Width - (indist*3+outdist*2+valuelen), line+16),
		joyTab,
        GUI_ID_JOY_CBOX);
    joy_cbox->addItem(L"select item");
    joy_cbox->addItem(L"accelerate/brake - AXIS");
    joy_cbox->addItem(L"steer - AXIS");
    joy_cbox->addItem(L"clutch - AXIS");
    joy_cbox->addItem(L"accelerate - BUTTON");
    joy_cbox->addItem(L"brake - BUTTON");
    joy_cbox->addItem(L"handbrake");
    joy_cbox->addItem(L"left");
    joy_cbox->addItem(L"right");
    joy_cbox->addItem(L"look left");
    joy_cbox->addItem(L"look right");
    joy_cbox->addItem(L"reset car");
    joy_cbox->addItem(L"change view");
    joy_cbox->addItem(L"switch car light");
    joy_cbox->addItem(L"switch compass (doubles the time)");
    joy_cbox->addItem(L"repair car");
    joy_cbox->addItem(L"gear up");
    joy_cbox->addItem(L"gear down");
    joy_cbox->addItem(L"gear 1");
    joy_cbox->addItem(L"gear 2");
    joy_cbox->addItem(L"gear 3");
    joy_cbox->addItem(L"gear 4");
    joy_cbox->addItem(L"gear 5");
    joy_cbox->addItem(L"gear 6");
    joy_cbox->addItem(L"Main menu");
	joy_text = env->addStaticText(L" ",
		rect<s32>(screenSize.Width - (indist*2+outdist*2+valuelen),line,screenSize.Width - (indist*2+outdist*2),line+16),
		false, // border?
		false, // wordwrap?
		joyTab);

    line += 20;
	env->addStaticText(L"Linear steer",
		rect<s32>(indist,line,indist+firsttextlen,line+16),
		false, // border?
		false, // wordwrap?
		joyTab);
	env->addCheckBox(joy_steer_linear,
		rect<s32>(indist*2+firsttextlen, line, indist*2+firsttextlen+16, line+16),
		joyTab, GUI_ID_STEER_LINEAR, L"Hello12");

	line += 20;
	env->addStaticText(L"Dead zone",
		rect<s32>(indist,line,indist+firsttextlen,line+16),
		false, // border?
		false, // wordwrap?
		joyTab);
	scrollbar = env->addScrollBar(true,
			rect<s32>(indist*2+firsttextlen, line, screenSize.Width - (indist*3+outdist*2+valuelen), line+16),
            joyTab, GUI_ID_DEAD_ZONE_SCROLL_BAR);
	scrollbar->setMax(99);
	// set scrollbar position to alpha value of an arbitrary element
    scrollbar->setSmallStep(1);
    scrollbar->setLargeStep(5);
	scrollbar->setPos((int)(deadZone*100.f));
	str = L"0.";
    if (deadZone<0.1f) str+=L"0";
	str += (int)(deadZone*100.f);
	dead_zone_text = env->addStaticText(str.c_str(),
		rect<s32>(screenSize.Width - (indist*2+outdist*2+valuelen),line,screenSize.Width - (indist*2+outdist*2),line+16),
		false, // border?
		false, // wordwrap?
		joyTab);

    if (line>maxLine) maxLine = line;

// ----------------
//  common
// ----------------
    maxLine += 120;
    line = maxLine;
    int outdistH = (screenSize.Height - (line + indist)) / 2;
    window->setRelativePosition(
		rect<s32>(outdist, outdistH, screenSize.Width - outdist, outdistH + line + indist));
    tabControl->setRelativePosition(
		rect<s32>(indist/2, indist+20, screenSize.Width - (outdist*2) - indist/2, line + indist - 30 - indist/2));
		
    // reinitialize button
    if (show_reinitialize_button)
    {
        env->addButton(
             rect<s32>(screenSize.Width - (120*3+outdist*2+indist*2),screenSize.Height - (42+outdistH*2),screenSize.Width - (120*2+outdist*2+indist*2),screenSize.Height - (10+outdistH*2)),
             window, GUI_ID_REINITIALIZE_BUTTON,
             L"Restart program"/*, L"Reinitialize the device and the whole game, current game will be lost."*/);
    }
    // save settings button
    saveSettingsButton = env->addButton(
         rect<s32>(screenSize.Width - (240+outdist*2+indist),screenSize.Height - (42+outdistH*2),screenSize.Width - (120+outdist*2+indist),screenSize.Height - (10+outdistH*2)),
         window, GUI_ID_SAVESETTINGS_BUTTON,
         L"Save settings", 0/*L"Save the settings to data/settings.txt file"*/);
    // close button
    closeOptionsButton = env->addButton(
         rect<s32>(screenSize.Width - (120+outdist*2),screenSize.Height - (42+outdistH*2),screenSize.Width - (10+outdist*2),screenSize.Height - (10+outdistH*2)),
         window, GUI_ID_BACK_BUTTON,
         L"Close", 0/*L"Close options window"*/);

    refreshActiveElements();
    //printf("oe w %d %d %d %p\n", mainWindowOpened, optionsWindowOpened, helpWindowOpened, window);
}

void eventreceiver_menu::openHelpWindow()
{
    const int outdist = 50;
    const int indist = 40;
    const int sw = 20;// scrollwidth
    const int helpHeight = screenSize.Height - (indist+outdist*2) - indist;
    
    //printf("hb w %d %d %d %p\n", mainWindowOpened, optionsWindowOpened, helpWindowOpened, window);

    if (helpWindowOpened || loading > 0) return;
    playSound(openSound);

    helpWindowOpened = true;
    if (inGame == 0)
    {
        other = (eventreceiver*)device->getEventReceiver();
       	device->setEventReceiver(this);
        pauseGame();
    }
    inGame++;

    helpWindowP = window;
    
    window = helpWindow = env->addWindow(
		rect<s32>(outdist, outdist, screenSize.Width - outdist, screenSize.Height - outdist),
		false, // modal?
		L"  Help", 0, GUI_ID_HELP_WINDOW);
    env->setFocus(window);

    IGUITab* tab = env->addTab(
		rect<s32>(indist,indist,screenSize.Width - (indist+outdist*2+sw), indist + helpHeight),
		window,
		GUI_ID_HELP_TAB
		);
    helpBox = env->addStaticText(
	//IGUISpinBox* box = env->addSpinBox(
	//IGUIEditBox* helpBox = env->addEditBox(
	//IGUITab* box = env->addTab(
        L" ",
		rect<s32>(0,0,screenSize.Width - (indist+outdist*2+sw), helpHeight),
		false, // border?
		true, // wordwrap?
		tab
        );
    //box->setWordWrap(true);
    //box->setMultiLine(true);
    //box->setAutoScroll(true);
    //box->setTextAlignment(EGUIA_UPPERLEFT ,EGUIA_UPPERLEFT);
    helpBox->setText(
        L"-----------\n" \
        L" THE GAME\n" \
        L"-----------\n" \
        L"\n" \
        L"The Dakar 2010 is a 3D game, where you can drive like in the Dakar rally.\n" \
        L"There aren't any opponents yet. There are 14 different stages, and each\n" \
        L"stages have two parts. First part starts morning, the second starts afternoon.\n" \
        L"If you drive long time it is possible to getting dark (with the high effects)\n" \
        L"\n" \
        L"------\n" \
        L" KEYS\n" \
        L"------\n" \
        L"\n" \
        L"ESC - Main menu\n" \
        L"F1  - Help (this window)\n" \
        L"F2  - Display extra information\n" \
        L"F3  - Switch hud (dashboard)\n" \
        L"F4  - Turn the car light on/off\n" \
        L"F5  - Change view\n" \
        L"F5  - Change camera follow mode\n" \
        L"F9  - Options\n" \
        L"R   - reset car\n" \
        L"T   - repair car\n" \
        L"F   - toggle view (free or behind car)\n" \
        L"M   - Display last message for 15 seconds\n" \
        L"\n" \
        L"W, S   - Accelerate, brake the car\n" \
        L"A, D   - Turn left, right the car\n" \
        L"Q, E   - Look left, right\n" \
        L"I, K   - Shift gear up, down\n" \
        L"O, L   - Shift gear up, down\n" \
        L"Space  - Handbrake\n" \
        L"G      - Change automatic/manual gear\n" \
        L"\n" \
        L"Arrows  - Move camera in free (FPS) mode\n" \
        L"\n" \
        L"---------\n" \
        L" ADVICES\n" \
        L"---------\n" \
        L"\n" \
        L"Here I want to give some advices, write some tips and tricks to make the race easier.\n" \
        L"\n" \
        L"-------------\n" \
        L" NAVIGATION\n" \
        L"-------------\n" \
        L"\n" \
        L"First of all there is no map or GPS system in the game, but there is a\n" \
        L"compass ('tab' button) which shows the nearest checkpoint. But you should\n" \
        L"know while the compass is active the time elapses twice faster than it is not\n" \
        L"active. And maybe the nearest checkpoint is not the next one, you can easily\n" \
        L"miss the next. Use it warily. It is not mandatory to use the compass (every time)\n" \
        L"avoid the penality. You can find the checkpoints also by following the road\n" \
        L"or the path.\n" \
        L"\n" \
        L"There are 3 types of the stages:\n" \
        L"  - valley style\n" \
        L"  - valley with road\n" \
        L"  - open terrain with road\n" \
        L"\n" \
        L"The valley style has no road but has a path. The path color is different from the\n" \
        L"other part of the terrain. The path go along a 'valley', and it is planner then\n" \
        L"the normal terrain. But it can contains several leaps that make the movement harder.\n" \
        L"Maybe the valley terrain has a road.\n" \
        L"\n" \
        L"The open terrain with road has now path and does not contains special leaps. It\n" \
        L"has only the natural hard terrain leaps.\n" \
        L"\n" \
        L"Generally true for all kind of stages that they can have junctions where you\n" \
        L"need to decide where to go. Some junctions are easy because a checkpoint inticates\n" \
        L"the right direction, but several junctions has no indicatior where to go.\n" \
        L"Some road leads to dead end, but maybe there can be more than one right way.\n" \
        L"If there is a road which goes two ways without any indication, and after a while\n" \
        L"they can run into each other again there should be a checkpoint to show that\n" \
        L"you are still on the right way. Some roads can be easier, some can be faster.\n" \
        L"\n" \
        L"-------------\n" \
        L" ROADBLOCKS\n" \
        L"-------------\n" \
        L"\n" \
        L"In many cases higher speed does not mean automatically the better position at\n" \
        L"finish. If you fall over with your car, you must reset it ('R' key). But it means\n" \
        L"that you will get penality. So some roadblock easier to pass with lower speed.\n" \
        L"\n" \
        L"----------\n" \
        L" CONTACT\n" \
        L"----------\n" \
        L"\n" \
        L"The game was designed by Balazs Tuska <btuska@elte.hu>.\n" \
        L"\n" \
        L"\n" \
        L"Enjoy the game!\n" \
        L"\n" \
    );
    //core::dimension2du textDimension = helpBox->getTextDimension();
    int scrollMax = helpBox->getTextHeight() - helpHeight;
    helpBase = 0;
    if (scrollMax > 0)
    {
    	helpScroll = env->addScrollBar(false,
    		rect<s32>(screenSize.Width - (indist+outdist*2+sw),indist,screenSize.Width - (indist+outdist*2), indist + helpHeight),
            window, GUI_ID_HELP_SCROLL);
    	helpScroll->setMax(scrollMax);
    	// set scrollbar position to alpha value of an arbitrary element
        helpScroll->setSmallStep(10);
        helpScroll->setLargeStep(helpHeight - 5);
    	helpScroll->setPos(0);
    
        env->setFocus(helpScroll);
    }
    else
        env->setFocus(window);
        
    helpBox->setRelativePosition(
		rect<s32>(0,helpBase,screenSize.Width - (indist+outdist*2+sw), helpBase + helpBox->getTextHeight()));
    
    //printf("he w %d %d %d %p\n", mainWindowOpened, optionsWindowOpened, helpWindowOpened, window);
    refreshActiveElements();
}	

void eventreceiver_menu::openStateWindow()
{
    //printf("hb w %d %d %d %p\n", mainWindowOpened, optionsWindowOpened, helpWindowOpened, window);

    if (stateWindowOpened || loading > 0 || !raceEngine || !bigTerrain) return;
    playSound(openSound);

    stateWindowOpened = true;
    if (inGame == 0)
    {
        other = (eventreceiver*)device->getEventReceiver();
       	device->setEventReceiver(this);
        pauseGame();
    }
    inGame++;

    stateWindowP = window;

    refreshStateWindow(true);
    
}	

void eventreceiver_menu::refreshStateWindow(bool leporget)
{
    const int outdist = 50;
    const int indist = 30;
    const int sw = 20;// scrollwidth
    int stateHeight = 0;
    
    //bool leporget = false;
    
    if (raceEngine && raceEngine->isRaceFinished()) leporget = false;
    
    if (!leporget) // no leporget button needed
        stateHeight = screenSize.Height - (indist+outdist*2) - indist - 33 - indist;
    else
        stateHeight = screenSize.Height - (indist+outdist*2) - indist - 33 - indist - 25;
    
    window = stateWindow = env->addWindow(
		rect<s32>(outdist, outdist, screenSize.Width - outdist, screenSize.Height - outdist),
		false, // modal?
		L"  Standings", 0, GUI_ID_STATE_WINDOW);
    env->setFocus(window);

    stateTabControl = env->addTabControl(
		rect<s32>(indist,indist+10,screenSize.Width - (indist+outdist*2), stateHeight + indist*2 + 33 /*screenSize.Height - outdist*2 - indist*/),
		window,
		true, // fillbg
		true, // frame
		GUI_ID_STATE_TABCONTROL
    );

    IGUITab* tab1 = stateTabControl->addTab(L"Stage");
    IGUITab* stab = env->addTab(
		rect<s32>(indist,indist/2,screenSize.Width - (indist*3+outdist*2+sw), stateHeight),
		tab1,
		GUI_ID_STATE_TAB
		);

    IGUITab* tab2 = stateTabControl->addTab(L"Overall");
    IGUITab* gtab = env->addTab(
		rect<s32>(indist,indist/2,screenSize.Width - (indist*3+outdist*2+sw), stateHeight),
		tab2,
		GUI_ID_STATEGLOBAL_TAB
		);
    /*
    env->addTab(
		rect<s32>(indist,indist,screenSize.Width - (indist+outdist*2+sw), indist + stateHeight),
		window,
		GUI_ID_STATE_TAB
		);
	*/
    stateBox = env->addStaticText(
	//IGUISpinBox* box = env->addSpinBox(
	//IGUIEditBox* helpBox = env->addEditBox(
	//IGUITab* box = env->addTab(
        L" ",
		rect<s32>(0,0,screenSize.Width - (indist*3+outdist*2+sw), stateHeight),
		false, // border?
		true, // wordwrap?
		stab
        );

    stateGlobalBox = env->addStaticText(
        L" ",
		rect<s32>(0,0,screenSize.Width - (indist*3+outdist*2+sw), stateHeight),
		false, // border?
		true, // wordwrap?
		gtab
        );
    //box->setWordWrap(true);
    //box->setMultiLine(true);
    //box->setAutoScroll(true);
    //box->setTextAlignment(EGUIA_UPPERLEFT ,EGUIA_UPPERLEFT);
//BigTerrain::addTimeToStr(core::stringw& str, u32 diffTime)
    core::stringw str = L"";
    if (raceEngine->getFinishedState().size()==0)
    {
        str = L"There is no result yet. This is the state at the begining of the stage:\n\n";
// todo comment this out in the real game
        if (bigTerrain)
        {
            str += L"Stage length: ";
            str += (int)bigTerrain->getStageLength();
            str += L" m, Stage time: ";
            str += bigTerrain->getStageTime();
            str += L" sec\n\n";
        }
        core::array<SStarter*> stageStarters;
        for (int i = 0; i < raceEngine->getStarters().size(); i++)
        {
            int j = 0;
            
            // update the player pos
            if (raceEngine->getStarters()[i]->competitor == playerCompetitor &&
                bigTerrain && car)
            {
                float dist = 100000.f;
                
                for (int k = 0; k < bigTerrain->getAIPoints().size(); k++)
                {
                    float cd = bigTerrain->getAIPoints()[k]->getPosition().getDistanceFrom(car->getMatrix().getTranslation());
                    if (cd < dist)
                    {
                        cd = dist;
                        raceEngine->getStarters()[i]->nextPoint = k;
                    }
                }
            }
            while (j < stageStarters.size() &&
                   raceEngine->getStarters()[i]->nextPoint <= stageStarters[j]->nextPoint
                  )
            {
                j++;
            }
            stageStarters.insert(raceEngine->getStarters()[i], j);
        }
        for (int i = 0; i < stageStarters.size(); i++)
        {
            if (i < 99)
            {
                str += L"  ";
                if (i < 9)
                {
                    str += L"  ";
                }
            }
            str += (i+1);
            str += L".     ";
            str += stageStarters[i]->competitor->num;
            str += L"   ";
            str += stageStarters[i]->competitor->getName();
            str += L"   Pos: ";
            str += (int)stageStarters[i]->currentPos.X;
            str += L", ";
            str += (int)stageStarters[i]->currentPos.Y;
            str += L",  Next point: ";
            str += stageStarters[i]->nextPoint;
            str += L" (";
#ifdef SPEED_BASE_AI
            str += (int)stageStarters[i]->nextPointCD;
#else
            str += (int)stageStarters[i]->passedDistance;
            str += L", ";
            str += stageStarters[i]->distanceStep;
#endif
            str += L") startCD: ";
            str += stageStarters[i]->startingCD;
            str += L"\n";
        }
/*
        for (int i = 0; i < raceEngine->getStarters().size(); i++)
        {
            if (i < 99)
            {
                str += L"  ";
                if (i < 9)
                {
                    str += L"  ";
                }
            }
            str += (i+1);
            str += L".     ";
            str += raceEngine->getStarters()[i]->competitor->num;
            str += L"   ";
            str += raceEngine->getStarters()[i]->competitor->getName();
            str += L"   Pos: ";
            str += (int)raceEngine->getStarters()[i]->currentPos.X;
            str += L", ";
            str += (int)raceEngine->getStarters()[i]->currentPos.Y;
            str += L",  Next point: ";
            str += raceEngine->getStarters()[i]->nextPoint;
            str += L" (";
            str += (int)raceEngine->getStarters()[i]->nextPointCD;
            str += L")\n";
        }
*/
    }
    else
    {
        for (int i = 0; i < raceEngine->getFinishedState().size(); i++)
        {
            if (i < 99)
            {
                str += L"  ";
                if (i < 9)
                {
                    str += L"  ";
                }
            }
            str += (i+1);
            str += L".     ";
            bigTerrain->addTimeToStr(str, raceEngine->getFinishedState()[i]->lastTime);
            str += L"     ";
            str += raceEngine->getFinishedState()[i]->num;
            str += L"   ";
            str += raceEngine->getFinishedState()[i]->getName();
            str += L"   ";
            str += raceEngine->getFinishedState()[i]->getCoName();
            str += L"   ";
            str += raceEngine->getFinishedState()[i]->getTeamName();
            if (raceEngine->getFinishedState()[i]->lastPenalityTime > 0)
            {
                str += L"   Penality: ";
                bigTerrain->addTimeToStr(str, raceEngine->getFinishedState()[i]->lastPenalityTime);
            }
            str += L"\n";
        }
    }
    stateBox->setText(str.c_str());
    //core::dimension2du textDimension = stateBox->getTextDimension();
    int scrollMax = stateBox->getTextHeight() - stateHeight;
    stateBase = 0;
    if (scrollMax > 0)
    {
    	stateScroll = env->addScrollBar(false,
    		rect<s32>(screenSize.Width - (indist*3+outdist*2+sw),indist/2,screenSize.Width - (indist*3+outdist*2), stateHeight),
            tab1, GUI_ID_STATE_SCROLL);
    	stateScroll->setMax(scrollMax);
    	// set scrollbar position to alpha value of an arbitrary element
        stateScroll->setSmallStep(10);
        stateScroll->setLargeStep(stateHeight - 5);
    	stateScroll->setPos(0);
    
        env->setFocus(stateScroll);
    }
    else
        env->setFocus(window);
        
    stateBox->setRelativePosition(
		rect<s32>(0, stateBase,screenSize.Width - (indist*3+outdist*2+sw), stateBase + stateBox->getTextHeight()));

    // ---------------
    // global stanging
    // ---------------
    str = L"";
    CRaceEngine::refreshRaceState(raceEngine);
    if (CRaceEngine::getRaceState().size()==0)
        str = L"There is no result yet.";

    for (int i = 0; i < CRaceEngine::getRaceState().size(); i++)
    {
        if (i < 99)
        {
            str += L"  ";
            if (i < 9)
            {
                str += L"  ";
            }
        }
        str += (i+1);
        str += L".     ";
        bigTerrain->addTimeToStr(str, CRaceEngine::getRaceState()[i]->globalTime);
        str += L"     ";
        str += CRaceEngine::getRaceState()[i]->num;
        str += L"   ";
        str += CRaceEngine::getRaceState()[i]->getName();
        str += L"   ";
        str += CRaceEngine::getRaceState()[i]->getCoName();
        str += L"   ";
        str += CRaceEngine::getRaceState()[i]->getTeamName();
        if (CRaceEngine::getRaceState()[i]->globalPenalityTime > 0)
        {
            str += L"   Penality: ";
            bigTerrain->addTimeToStr(str, CRaceEngine::getRaceState()[i]->globalPenalityTime);
        }
        str += L"\n";
    }
    stateGlobalBox->setText(str.c_str());
    //core::dimension2du textDimension = stateBox->getTextDimension();
    scrollMax = stateGlobalBox->getTextHeight() - stateHeight;
    stateGlobalBase = 0;
    if (scrollMax > 0)
    {
    	stateGlobalScroll = env->addScrollBar(false,
    		rect<s32>(screenSize.Width - (indist*3+outdist*2+sw),indist/2,screenSize.Width - (indist*3+outdist*2), stateHeight),
            tab2, GUI_ID_STATEGLOBAL_SCROLL);
    	stateGlobalScroll->setMax(scrollMax);
    	// set scrollbar position to alpha value of an arbitrary element
        stateGlobalScroll->setSmallStep(10);
        stateGlobalScroll->setLargeStep(stateHeight - 5);
    	stateGlobalScroll->setPos(0);
    
        //env->setFocus(stateScroll);
    }
    //else
    //    env->setFocus(window);
        
    stateGlobalBox->setRelativePosition(
		rect<s32>(0,stateGlobalBase,screenSize.Width - (indist*3+outdist*2+sw), stateGlobalBase + stateGlobalBox->getTextHeight()));

    if (leporget)
        env->addButton(
            rect<s32>(screenSize.Width - (140+outdist*2),screenSize.Height - (47+outdist*2),screenSize.Width - (25+outdist*2),screenSize.Height - (15+outdist*2)),
            window, GUI_ID_LEPORGET_BUTTON,
            L"Run Down", 0/*L"Close options window"*/);
    
    //printf("he w %d %d %d %p\n", mainWindowOpened, optionsWindowOpened, stateWindowOpened, window);
    refreshActiveElements();
}

void eventreceiver_menu::render()
{
    if (activeElement >= 0 && activeElement < activeElements.length())
    {
        rect<s32> rec = activeElements[activeElement]->getAbsolutePosition();
        if (activeElements[activeElement]==tabControl)
        {
            rec.LowerRightCorner.Y = rec.UpperLeftCorner.Y + 33;
        }
        driver->draw2DRectangleOutline(rec,
                        SColor(255, 90, 54, 54));

        rec.UpperLeftCorner.X--;
        rec.UpperLeftCorner.Y--;
        rec.LowerRightCorner.X++;
        rec.LowerRightCorner.Y++;
        driver->draw2DRectangleOutline(rec,
                        SColor(255, 200, 158, 158));

        rec.UpperLeftCorner.X--;
        rec.UpperLeftCorner.Y--;
        rec.LowerRightCorner.X++;
        rec.LowerRightCorner.Y++;
        driver->draw2DRectangleOutline(rec,
                        SColor(255, 90, 54, 54));
    }
    //prerender();
}

void eventreceiver_menu::prerender()
{
    if (car_to_draw)
    {
        //printf("render begin\n");
        driver->setRenderTarget(car_selector_rtt, true, true, video::SColor(0, 255, 0, 0));
        
        //car_selector_camera->setPosition(vector3df(-3.f, 2.f, 3.f));
        car_selector_camera->setTarget(vector3df(0.f,0.f,0.f));
        car_selector_camera->setFarValue(DEFAULT_FAR_VALUE);
        smgr->setActiveCamera(car_selector_camera);

        car_selector_camera->OnAnimate(device->getTimer()->getTime());
        car_selector_camera->OnRegisterSceneNode();
        car_selector_camera->render();
        
        driver->setTransform(ETS_VIEW, car_selector_camera->getViewMatrix());
        driver->setTransform(ETS_PROJECTION, car_selector_camera->getProjectionMatrix());

        //printf("render begin 2\n");

        car_to_draw->OnRegisterSceneNode();
        car_to_draw->render();
        smgr->setActiveCamera(camera);
        //printf("render begin end\n");
        //driver->setRenderTarget(0, true, true, video::SColor(0, 0, 0, 255));
    }
}

void eventreceiver_menu::refreshActiveElements()
{
    IGUIElement* parent = window;
    if (window==optionsWindow)
    {
        parent = tabControl->getTab(tabControl->getActiveTab());
    }
    
    if (parent == lastParent) return;
    
    lastParent = parent;
    activeElements.delList();
    activeElement = 0;

    if (window==optionsWindow)
    {
        activeElements.addLast(tabControl/*->getTab(tabControl->getActiveTab())*/);
    }
    
    for (irr::core::list<IGUIElement*>::ConstIterator iter = parent->getChildren().begin(); iter != parent->getChildren().end(); iter++)
    {
        if ((*iter)->isVisible())
            switch ((*iter)->getType())
            {
                case EGUIET_STATIC_TEXT:
                case EGUIET_IMAGE:
                    break;
                default:
                   activeElements.addLast(*iter);
            }
    }

    if (window==optionsWindow)
    {
        activeElements.addLast(saveSettingsButton/*->getTab(tabControl->getActiveTab())*/);
        activeElements.addLast(closeOptionsButton/*->getTab(tabControl->getActiveTab())*/);
    }
    else
    {
        if (window == helpWindow)
            activeElements.delList();
        if (window == stateWindow)
            activeElements.delList();
        if (window != mainWindow && window != selectionWindow)
          activeElements.delFirst();
        if (activeElements.length() > 0) env->setFocus(activeElements[activeElement]);
    }
}

void eventreceiver_menu::releaseResources()
{
    if (joyHelper)
    {
        delete [] joyHelper;
        joyHelper = 0;
    }
    
    if (car_to_draw)
    {
        //vehiclePool->putVehicle(car_to_draw);
        car_to_draw = 0;
    }
}
