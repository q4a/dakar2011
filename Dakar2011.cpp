/****************************************************************
*                                                               *
*    Name: Dakar2010.cpp                                        *
*                                                               *
*    Creator: Balazs Tuska                                      *
*                                                               *
*    Description:                                               *
*       This file contains the main loop of the game. It        *
*       handles the keyboard and joystick events.               *
*                                                               *
****************************************************************/

#include "irrlicht.h"
#include <iostream>
#include <Newton.h>
#ifdef __linux__
#include <unistd.h>
#else
#include <io.h>
#endif
#include "NewtonRaceCar.h"
#include "Materials.h"
#include "BigTerrain.h"
#include "my_shaders.h"
#include "settings.h"
#include "message.h"
#include "gameplay.h"
#include "multiplayer.h"
#include "CGUITexturedSkin.h"
#include "CImageGUISkin.h"
#include "SkinLoader.h"
#include "eventreceiver_game.h"
#include "eventreceiver_menu.h"
#include "eventreceiver_dummy.h"
#include "effects.h"
#include "CScreenQuad.h"
#include "CQuad.h"
#include "pools.h"
#include "competitors.h"
#include "editor.h"
#include "Dakar2011_private.h"
#include "itiner_hud.h"
#include "error.h"
#include "fonts.h"

#include <CHeightmap.h>

#ifdef __linux__
#include "linux_includes.h"
#endif

/*
#ifdef MY_DEBUG
#define dprintf(x) x
#else
#define dprintf(x)
#endif
*/
//#define pdprintf(x) x

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

c8 currentDirectory[256];

void BodyLeaveWorld (const NewtonBody* body, int threadId);

int main()
{
    do {
    
    // new init
    quitGame = false;
    inGame = 0;
    car = 0;
    bigTerrain = 0;
    lasttick = 0;
    tick = 0;
    view_num = 0;
    view_mask = 0;
    loading = 0;
    startNewGame = 1;
    for (int i = 0; i < MAX_BGIMAGE+1; i++) bgImagesTextures[i] = 0;
    for (int i = 0; i < MAX_HUD+2; i++) hudTextures[i] = 0;
    for (int i = 0; i < MAX_SCREENRTT; i++) screenRTT[i];
    currentScreenRTT = 0;
    depthRTT = 0;
    bgImageTexture = 0;
    blurmap = 0;
    blurmapSide = 0;
    for (int i = 0; i < view_multi; i++) motiondir_map[i];
    for (int i = 0; i < MAX_CAR_DIRT; i++) car_dirttexture_array[i] = 0;
    //////
    
    getcwd(currentDirectory, 256);
    readSettings("data/settings.txt");
    
    if (reinitialize && driverType == video::EDT_DIRECT3D9)
        useCgShaders = false;
    reinitialize = false;

    // let user select driver type
	//video::E_DRIVER_TYPE driverType = video::EDT_OPENGL/*EDT_SOFTWARE*/;
/*
	printf("Please select the driver you want for this example:\n"\
		" (a) Direct3D 9.0c\n (b) Direct3D 8.1\n (c) OpenGL 1.5\n"\
		" (d) Software Renderer\n (e) Burning's Software Renderer\n"\
		" (f) NullDevice\n (otherKey) exit\n\n");
	char i;
	std::cin >> i;
	switch(i)
	{
		case 'a': driverType = video::EDT_DIRECT3D9;break;
		case 'b': driverType = video::EDT_DIRECT3D8;break;
		case 'c': driverType = video::EDT_OPENGL;   break;
		case 'd': driverType = video::EDT_SOFTWARE; break;
		case 'e': driverType = video::EDT_BURNINGSVIDEO;break;
		case 'f': driverType = video::EDT_NULL;     break;
		default: return 1;
	}	
*/
#ifdef __linux__
    if (driverType == video::EDT_DIRECT3D9 || driverType == video::EDT_DIRECT3D8) driverType = video::EDT_OPENGL;
#endif
    irr::SIrrlichtCreationParameters param;
    //param.WindowId = reinterpret_cast<void*>(hIrrlichtWindow); // hColorButton
    param.DriverType = driverType;
    param.Fullscreen = full_screen;
    param.WindowSize = dimension2d<s32>(resolutionX, resolutionY);
    param.Stencilbuffer = stencil_shadows; // shadows;
    param.AntiAlias = anti_aliasing;
    param.Vsync = vsync;
    param.HighPrecisionFPU = high_precision_fpu;
    
    if (auto_resolution && full_screen)
    {
        irr::IrrlichtDevice* nullDev = createDevice(EDT_NULL);
        dprintf(printf("Use full screen with auto-detection\n"));
        if (nullDev)
        {
            dprintf(printf("null device found auto-detection is possible. detected: %dx%d%d\n"));
#ifdef IRRLICHT_SDK_15
            core::dimension2d<s32> res = nullDev->getVideoModeList()->getDesktopResolution();
#else
            core::dimension2d<u32> res = nullDev->getVideoModeList()->getDesktopResolution();
#endif
            s32 dep = nullDev->getVideoModeList()->getDesktopDepth();
            
            nullDev->drop();
            
            display_bits = dep;
            resolutionX = res.Width;
            resolutionY = res.Height;
            dprintf(printf("null device found auto-detection is possible. detected: %dx%d%d\n", resolutionX, resolutionY, display_bits));
        }
        else
            dprintf(printf("null device not found auto-detection is not possible!!!\n"));
    }
    param.Bits = display_bits;
    param.WindowSize = dimension2d<s32>(resolutionX, resolutionY);

    irr::IrrlichtDevice* device = irr::createDeviceEx(param);

/*
    IrrlichtDevice* device = createDevice(driverType,                  // render type
                                          dimension2d<s32>(resolutionX, resolutionY),  // resolution/window size
                                          32,                          // bits
                                          full_screen,                 // fullscreen
                                          shadows,                     // stencilbuffer
                                          0,                           // vsync
                                          0);                          // eventReceiver
*/    
    if (!device)
    {
        myError(1, "Cannot initialize Irrlicht device!");
        return 1;  // failed to initialize driver
    }
    /*
    for (int i = 0; i < 100; i++)
    {
   	    dprintf(printf("DEBUG: %i device run device %p\n", i, device); device->run();)
    }
    */
    initializeUsedMemory(device);

	core::array<SJoystickInfo> joystickInfo;
	if(device->activateJoysticks(joystickInfo))
	{
		dprintf(printf("Joystick support is enabled and %d joystick(s) are present.\n", joystickInfo.size()));

        if (joystickInfo.size()==0)
    		joy_axis_clutch = -1;

		for(u32 joystick = 0; joystick < joystickInfo.size(); ++joystick)
		{
			dprintf(printf("Joystick %d:\n", joystick));
			dprintf(printf("\tName: '%s'\n", joystickInfo[joystick].Name.c_str()));
			dprintf(printf("\tAxes: %d\n", joystickInfo[joystick].Axes));
			dprintf(printf("\tButtons: %d\n", joystickInfo[joystick].Buttons));

			dprintf(printf("\tHat is: "));

			switch(joystickInfo[joystick].PovHat)
			{
			case SJoystickInfo::POV_HAT_PRESENT:
				dprintf(printf("present\n"));
				break;

			case SJoystickInfo::POV_HAT_ABSENT:
				dprintf(printf("absent\n"));
				break;

			case SJoystickInfo::POV_HAT_UNKNOWN:
			default:
				dprintf(printf("unknown\n"));
				break;
			}
		}
	}
	else
	{
		dprintf(printf("Joystick support is not enabled.\n"));
		joy_axis_clutch = -1;
	}

                                          
    device->setWindowCaption(L"Dakar 2011");

	// start the sound engine with default parameters
#ifdef USE_MY_SOUNDENGINE
    CMySoundEngine* soundEngine = new CMySoundEngine();
#else
	irrklang::ISoundEngine* soundEngine = createIrrKlangDevice();
#endif

	if (!soundEngine)
	{
        myError(2, "Cannot initialize sound device!");
		return 1; // error starting up the engine     
    }
		
    IVideoDriver* driver = device->getVideoDriver();
    ISceneManager* smgr = device->getSceneManager();
    IGUIEnvironment* env = device->getGUIEnvironment();

    screenSize = driver->getScreenSize();
    
    CScreenQuad screenQuad(driverType == video::EDT_OPENGL, flip_vert);

    assert(view_multi==4);
    if (driver->getDriverType() == EDT_OPENGL)
    {
        blurmap = driver->getTexture("data/posteffects/blurmap_gl.png");
        motiondir_map[0] = driver->getTexture("data/posteffects/motiondir_center_gl.png");
    }
    else
    {
        blurmap = driver->getTexture("data/posteffects/blurmap.png");
        motiondir_map[0] = driver->getTexture("data/posteffects/motiondir_center.png");
    }
    blurmapSide = driver->getTexture("data/posteffects/blurmap_side.png");
    //motiondir_mapSide = driver->getTexture("data/posteffects/motiondir_side.png");
    motiondir_map[1] = driver->getTexture("data/posteffects/motiondir_sideLeft.png");
    motiondir_map[2] = driver->getTexture("data/posteffects/motiondir_sideRight.png");
    motiondir_map[3] = motiondir_map[0];
    
    recreateRTTs(driver);
    
	// Newton vars
	NewtonWorld *nWorld = NewtonCreate(NULL, NULL);
	NewtonSetThreadsCount(nWorld, 2);

	// Set up default material properties for newton
	SetupMaterials(nWorld, soundEngine);
	
    //video::ITexture* bgTexture = driver->getTexture("data/bg/dakar_bg1.jpg");
    if (screenSize.Width > 1280)
    {
        bgImages = (char**)bgImagesHi;
    }
    else
    {
        bgImages = (char**)bgImagesLo;
    }
    
	bgImage = env->addImage(core::rect<int>(0,0,screenSize.Width,screenSize.Height), 0, -1, L"data/bg/dakar_bg1.jpg");
	bgImage->setScaleImage(true);
	bgImagesTextures[5] = bgImageTexture = driver->getTexture(bgImages[5]);
    bgImage->setImage(bgImageTexture);

    if (this_will_display)
    {
        env->addStaticText(
            L"This will be the Dakar 2011 simulator program",
	        core::rect<int>(10,10,190,26), true, true, 0, -1, true);
    }

    fpsText = env->addStaticText(L"FPS: ",
                        core::rect<int>(10,30,80,46),
                        info_bg, true, 0, -1, info_bg);
    fpsText->setVisible(false);
    polyText = env->addStaticText(L"POLYS: ",         // 110
                        core::rect<int>(10,50,350,66),
//                        core::rect<int>(screenSize.Width-150,30,screenSize.Width-10,46),
                        info_bg, true, 0, -1, info_bg);
    polyText->setVisible(false);
    posText = env->addStaticText(L"POS: ",
                        core::rect<int>(10,70,350,86),
                        info_bg, true, 0, -1, info_bg);
    posText->setVisible(false);
    {
        core::stringw str = L" Build ";
        str += VER_STRING;
        versionText = env->addStaticText(str.c_str(),
                            core::rect<int>(screenSize.Width - 120, screenSize.Height - 20,screenSize.Width - 4, screenSize.Height - 4),
                            false, true, 0, -1, true);
        versionText->setVisible(false);
    }
    
    if (editorMode)
    {
        initEditor(env);
    }

    const int fontH = 25;
    const int fontO = 10;

    ITexture* hudInfoTexture = driver->getTexture("data/hud/info_bg.png");
//	hudInfo = env->addImage(core::rect<int>(7, screenSize.Height-(fontO+fontH*3)-3, 7+350/*hudInfoTexture->getSize().Width*/, screenSize.Height-(fontO)+3/*+hudInfoTexture->getSize().Height*/), 0, -1, L"hudInfo");
	hudInfo = env->addImage(core::rect<int>(7, screenSize.Height-(fontO+fontH*3)-1, 7+hudInfoTexture->getSize().Width, screenSize.Height-(fontO+fontH*3)-1+hudInfoTexture->getSize().Height), 0, -1, L"hudInfo");
//	hudInfo->setScaleImage(true);
	hudInfo->setScaleImage(false);
	hudInfo->setUseAlphaChannel(true);
    hudInfo->setImage(hudInfoTexture);
    hudInfo->setVisible(false);

    demageText = env->addStaticText(L"Demage: ",
                        core::rect<int>(10,screenSize.Height-(fontO+fontH*3),1350,screenSize.Height-(fontO+fontH*2)),
                        false, true, 0, -1, false);
    demageText->setVisible(false);
    speedText = env->addStaticText(L"Speed: ",
                        core::rect<int>(10,screenSize.Height-(fontO+fontH*2),350,screenSize.Height-(fontO+fontH)),
                        false, true, 0, -1, false);
    speedText->setVisible(false);
    timeText = env->addStaticText(L"Time: ",
                        core::rect<int>(10,screenSize.Height-(fontO+fontH),350,screenSize.Height-fontO),
                        false, true, 0, -1, false);
    timeText->setVisible(false);

    MessageText::init(device, env, screenSize);

	driver->setTextureCreationFlag(video::ETCF_ALWAYS_32_BIT, true);
    
    // setskin
    switch (skin_type)
    {
        case 1:
        {
            gui::IGUISkin* newskin = env->createSkin(gui::EGST_BURNING_SKIN);
            env->setSkin(newskin);
            newskin->drop();    
            break;
        }
        case 2:
        {
            CGUITexturedSkin * pNewSkin = new CGUITexturedSkin(env, device->getFileSystem());
            pNewSkin->setSkin( "data/menu_textures/guiskin.xml" );
            env->setSkin(pNewSkin);
            pNewSkin->drop();
            break;
        }
        case 3:
        {
            // Load GUI
            gui::SImageGUISkinConfig guicfg = LoadGUISkinFromFile(device->getFileSystem(), driver, "data/menu_textures/ui/guiskin.cfg");
            gui::CImageGUISkin* skin = new gui::CImageGUISkin(driver, env->getSkin());
            skin->loadConfig(guicfg);
            env->setSkin(skin);
            skin->drop();
            break;
        }
        default:
        {
	        //env->getSkin()->setFont(env->getFont("data/fonts/fontlucida.png"));
	        //env->getSkin()->setFont(env->getBuiltInFont(), EGDF_TOOLTIP);
            break;
        }
    }
//    printf("gui textures: %d\n", env->getSkin()->getSpriteBank()->getTextureCount());
//    bgImage->setImage(env->getSkin()->getSpriteBank()->getTexture(0));
	//set other font
	//env->getSkin()->setFont(env->getFont("data/fonts/fonthaettenschweiler.bmp"));
	setupFonts(env);
	
	env->getSkin()->setFont(fonts[FONT_NORMALBOLD]);
	env->getSkin()->setFont(fonts[FONT_BUILTIN], EGDF_TOOLTIP);
/*
EGDC_3D_DARK_SHADOW 	
Dark shadow for three-dimensional display elements. 
EGDC_3D_SHADOW 	
Shadow color for three-dimensional display elements (for edges facing away from the light source). 
EGDC_3D_FACE 	
Face color for three-dimensional display elements and for dialog box backgrounds. 
EGDC_3D_HIGH_LIGHT 	
Highlight color for three-dimensional display elements (for edges facing the light source.). 
EGDC_3D_LIGHT 	
Light color for three-dimensional display elements (for edges facing the light source.). 
EGDC_ACTIVE_BORDER 	
Active window border. 
EGDC_ACTIVE_CAPTION 	
Active window title bar text. 
EGDC_APP_WORKSPACE 	
Background color of multiple document interface (MDI) applications. 
EGDC_BUTTON_TEXT 	
Text on a button. 
EGDC_GRAY_TEXT 	
Grayed (disabled) text. 
EGDC_HIGH_LIGHT 	
Item(s) selected in a control. 
EGDC_HIGH_LIGHT_TEXT 	
Text of item(s) selected in a control. 
EGDC_INACTIVE_BORDER 	
Inactive window border. 
EGDC_INACTIVE_CAPTION 	
Inactive window caption. 
EGDC_TOOLTIP 	
Tool tip text color. 
EGDC_TOOLTIP_BACKGROUND 	
Tool tip background color. 
EGDC_SCROLLBAR 	
Scrollbar gray area. 
EGDC_WINDOW 	
Window background. 
EGDC_WINDOW_SYMBOL 	
Window symbols like on close buttons, scroll bars and check boxes. 
EGDC_ICON 	
Icons in a list or tree. 
EGDC_ICON_HIGH_LIGHT 	
Selected icons in a list or tree. 
EGDC_COUNT 	
this value is not used, it only specifies the amount of default colors available.
*/
//    SColor guicol(255, 111, 85, 23);
    SColor guicol(255, 90, 70, 16);
    SColor guicol2(255, 255, 255, 255);
//	env->getSkin()->setColor(EGDC_ACTIVE_CAPTION, guicol);
//	env->getSkin()->setColor(EGDC_APP_WORKSPACE, guicol);
	env->getSkin()->setColor(EGDC_BUTTON_TEXT, guicol);
//	env->getSkin()->setColor(EGDC_GREY_TEXT, guicol);
//	env->getSkin()->setColor(EGDC_INACTIVE_CAPTION, guicol);
//	env->getSkin()->setColor(, guicol);
    demageText->setOverrideFont(fonts[FONT_SPECIAL16]);
    speedText->setOverrideFont(fonts[FONT_SPECIAL16]);
    timeText->setOverrideFont(fonts[FONT_SPECIAL16]);

    demageText->setOverrideColor(guicol2);
    speedText->setOverrideColor(guicol2);
    timeText->setOverrideColor(guicol2);

    MessageText::addText(L"Please wait [            ]", 1);

    for (u32 i=0; i<EGDC_COUNT ; ++i)
    {
        SColor col = env->getSkin()->getColor((EGUI_DEFAULT_COLOR)i);
        if (col.getAlpha() != 0)
            col.setAlpha(200);
        env->getSkin()->setColor((EGUI_DEFAULT_COLOR)i, col);
    }


    // first draw the wait message
    driver->beginScene(true, true, SColor(0,192,192,192));
    //smgr->drawAll();
    env->drawAll();
    driver->endScene();
   
    //driver->setFog(SColor(0, 255, 255, 255), false);

    ITexture* hudCompassTexture = driver->getTexture("data/hud/hud_compass.png");
    hudTextures[0] = driver->getTexture("data/hud/hud0.png");
    hudTextures[1] = driver->getTexture("data/hud/hud1.png");
    hudTextures[2] = driver->getTexture("data/hud/hud2.png");
    hudTextures[3] = driver->getTexture("data/hud/hud3.png");
    hudTextures[4] = driver->getTexture("data/hud/hud4.png");
    hudTextures[5] = driver->getTexture("data/hud/hud5.png");
    hudTextures[6] = driver->getTexture("data/hud/hud6.png");
    hudTextures[7] = driver->getTexture("data/hud/hudr.png");
    hudTextures[8] = driver->getTexture("data/hud/hudd.png");
    const float hudRate = (float)(screenSize.Width)/2600.f;
    float hudSize;
    if (hudTextures[1])
        hudSize = (float)hudTextures[1]->getSize().Width * hudRate;
    else
        hudSize = 512.f * hudRate;
    const float hudCompassRate = 0.3f;
    const float hudCompassSize = hudSize * hudCompassRate;
    const float hudCenter1Size = 255.f * hudRate;
    const float hudPos1X = (255.f-120.f) * hudRate;
    const float hudPos1Y = (255.f-390.f) * hudRate;
    const float hudCenter2Size = 440.f * hudRate;
    const float hudPos2X = (255.f-181.f) * hudRate;
    const float hudPos2Y = (440.f-397.f) * hudRate;
    const float hudCenter3Size = hudCenter1Size * hudCompassRate;
    const float hudPos3X = 0.f;
    const float hudPos3Y = (255.f - 100.f) * hudCompassRate * hudRate;
    const int hudPositionX = screenSize.Width-(int)hudSize-10;
    const int hudPositionY = screenSize.Height-(int)hudSize-10;
	hudImage = env->addImage(core::rect<int>(hudPositionX, hudPositionY, screenSize.Width-10, screenSize.Height-10), 0, -1, L"hud");
	hudImage->setScaleImage(true);
	hudImage->setUseAlphaChannel(true);
    hudImage->setImage(hudTextures[1]);
    dprintf(printf("1 %p\n", hudImage));
    hudImage->setVisible(false);
    
    ItinerHud::init(device, env, screenSize, (int)hudSize);

    const int hudCompassPositionX = 10; //hudPositionX - hudCompassSize / 2.f;
    const int hudCompassPositionY = screenSize.Height-(fontO+fontH*3)-30-(int)hudCompassSize;//(int)hudCompassSize-10;
    compassText = env->addStaticText(L"DOUBLE TIME!",
                        core::rect<int>(hudCompassPositionX, screenSize.Height-(fontO+fontH*3)-20, 210, screenSize.Height-(fontO+fontH*3)-4),
                        info_bg, true, 0, -1, info_bg);
    compassText->setVisible(false);
	hudCompassImage = env->addImage(core::rect<int>(hudCompassPositionX, hudCompassPositionY,
                                                    (int)(hudCompassSize)+hudCompassPositionX, (int)(hudCompassSize)+hudCompassPositionY),
                                    0, -1, L"hudCompass");
	hudCompassImage->setScaleImage(true);
	hudCompassImage->setUseAlphaChannel(true);
    hudCompassImage->setImage(hudCompassTexture);
    hudCompassImage->setVisible(false);

    ITexture* crossTexture = driver->getTexture("data/hud/cross.png");
	crossImage = env->addImage(core::rect<int>(screenSize.Width/2-10, screenSize.Height/2-10,
                                               screenSize.Width/2+10, screenSize.Height/2+10),
                               0, -1, L"cross");
	crossImage->setScaleImage(false);
	crossImage->setUseAlphaChannel(true);
    crossImage->setImage(crossTexture);
    crossImage->setVisible(false);

/*
    hudCompassImage = new CQuad(vector3df(0.f, 0.f, 0.f), vector3df(1.0f, 1.0f, 0.f));
    hudCompassImage->texture = hudCompassTexture;
    hudCompassImage->setVisible(false);
    hudCompassImage->getMaterial().setTexture(0, hudCompassImage->texture);
    hudCompassImage->getMaterial().

    CScreenQuad test(driverType == video::EDT_OPENGL, flip_vert);
*/    
    const core::vector2df hudCenter1((float)hudPositionX+hudCenter1Size, (float)hudPositionY+hudCenter1Size);
    const core::position2d<s32> hudCenter2d1((s32)hudCenter1.X,(s32)hudCenter1.Y);
    const core::vector2df hudPos1(hudCenter1.X-hudPos1X, hudCenter1.Y-hudPos1Y);

    const core::vector2df hudCenter2((float)hudPositionX+hudCenter1Size, (float)hudPositionY+hudCenter2Size);
    const core::position2d<s32> hudCenter2d2((s32)hudCenter2.X,(s32)hudCenter2.Y);
    const core::vector2df hudPos2(hudCenter2.X-hudPos2X, hudCenter2.Y-hudPos2Y);

    const core::vector2df hudCenter3((float)hudCompassPositionX+hudCenter3Size, (float)hudCompassPositionY+hudCenter3Size);
    const core::position2d<s32> hudCenter2d3((s32)hudCenter3.X,(s32)hudCenter3.Y);
    const core::vector2df hudPos3(hudCenter3.X-hudPos3X, hudCenter3.Y-hudPos3Y);
    const core::vector2df hudPos3c(hudCenter3.X-hudPos3X, hudCenter3.Y+(hudPos3Y*3/4));
    const core::vector2df hudPos3l(hudCenter3.X-hudPos3X-5, hudCenter3.Y+hudPos3Y-4);
    const core::vector2df hudPos3r(hudCenter3.X-hudPos3X+5, hudCenter3.Y+hudPos3Y-4);

    core::vector2df hudPos;
    core::position2d<s32> hudPos2d;
    core::position2d<s32> hudPos2d2;
    core::position2d<s32> hudPos2d3;
    core::position2d<s32> hudPos2d4;

    compassArrow = smgr->addAnimatedMeshSceneNode(smgr->addArrowMesh("arrowMesh", video::SColor(0, 0, 155, 0), video::SColor(0, 0, 155, 0),
                            4, 8, 1.5f, 1.0f, 0.1f, 0.2f));
    compassArrow->setVisible(false);

    MessageText::addText(L"Please wait [*           ]", 1, true);

	// add camera
	fix_camera = smgr->addCameraSceneNode();
	fix_camera->setFarValue(DEFAULT_FAR_VALUE);
    fix_camera->setNearValue(nearValue);	

	fps_camera = smgr->addCameraSceneNodeFPS(0, 100.f, CAMERA_SPEED);
	fps_camera->setFarValue(DEFAULT_FAR_VALUE);
    fps_camera->setNearValue(nearValue);	

	lightCam = smgr->addCameraSceneNode();
	lightCamCar = smgr->addCameraSceneNode(); //lightCamCar->setFOV(0.02f);

	car_selector_camera = smgr->addCameraSceneNode();
	car_selector_camera->setFarValue(DEFAULT_FAR_VALUE);
    car_selector_camera->setNearValue(nearValue);	
	
	camera = fix_camera;
	smgr->setActiveCamera(camera);

    device->getCursorControl()->setPosition(0.f,0.f);
    
    // set amb light
//	smgr->setAmbientLight(video::SColor(255, 115, 115, 115));
	smgr->setShadowColor();
/*
	smgr->addLightSceneNode(0,
        core::vector3df(((float)max_x*SMALLTERRAIN_SIZE/(float)2),5000.f,((float)max_y*SMALLTERRAIN_SIZE/(float)2)),
		video::SColorf(1.0f, 1.0f, 1.0f, 1.0f), ((float)max_x*SMALLTERRAIN_SIZE*(float)2));
*/
    float lightColor = 1.0f;
   	lnode = smgr->addLightSceneNode(0,
            core::vector3df(5300.f,20000.f,10000.f),
		    video::SColorf(lightColor, lightColor, lightColor), 50000.f);
    lnode->getLightData().Type=ELT_DIRECTIONAL; 
    lnode->setRotation(core::vector3df(110.f, 0.f, 0.f));
    if (useShaders)
    {
    	lnode_4_shaders = smgr->addLightSceneNode(0,
            core::vector3df(1.f,10.f,5.f),
		    video::SColorf(lightColor, lightColor, lightColor), 50000.f);
        lnode_4_shaders->getLightData().Type=ELT_DIRECTIONAL; 
        lnode_4_shaders->setRotation(core::vector3df(110.f, 0.f, 0.f));
        lnode_4_shaders->setVisible(false);
    }
    float lightStrength = 0.6f;
    float lightStrengthS = 0.6f;
    lnode->getLightData().AmbientColor = video::SColorf(lightStrength,lightStrength,lightStrength);
    lnode->getLightData().SpecularColor = video::SColorf(lightStrengthS,lightStrengthS,lightStrengthS);
/*
	clnode = smgr->addLightSceneNode(0//,
//        core::vector3df(0.f,-10.f,-1.f),
//        core::vector3df(5300.f,10000.f,10000.f),
//		video::SColorf(lightColor, lightColor, lightColor), 50000.f
            );
    clnode->getLightData().Type=ELT_SPOT; 
    clnode->setRotation(core::vector3df(90.f, 0.f, 0.f));
    lightColor = 1.0f;
    lightStrength = 1.0f;
    lightStrengthS = 1.0f;
    clnode->getLightData().DiffuseColor = video::SColorf(lightColor,lightColor,lightColor);
    clnode->getLightData().AmbientColor = video::SColorf(lightStrength,lightStrength,lightStrength);
    clnode->getLightData().SpecularColor = video::SColorf(lightStrengthS,lightStrengthS,lightStrengthS);
*/
/*
//    lnode->setLightType(video::ELT_DIRECTIONAL);
    video::SLight* ldata;
    ldata = &lnode->getLightData();
    ldata->Type = video::ELT_DIRECTIONAL;
//    ldata->Position = core::vector3df(0,-1,0);
    ldata->Position = core::vector3df(3300,100,-500);
//    ldata->Direction = core::vector3df(0,-1,0);
//    lnode->setLightData(*ldata);
*/
	// disable mouse cursor
    //device->getCursorControl()->setVisible(false);

    useShaders = useCgShaders = true; // force using of shaders
    dprintf(printf("2 %p %d\n", hudImage, useCgShaders));
    try
    {
        setupShaders2(device, driver, driverType, smgr, camera, true/*usehls*/, lnode_4_shaders);
    } catch(...)
    {
        printf("Cg shader setup casued exception, fall back to standard shaders\n");
        myError(3, "Cannot initialize shaders! Maybe your hardware does not support it.");
        return 1;
    }
    //myError(0, "hello");
    // assert(0); // for shader debug
    
    if (!useCgShaders)
    {
        shadowMap = shadowMapGame = shadowMapMenu = shadowMapCar = 0;
    }

    dprintf(printf("2b %p\n", hudImage));

	driver->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS, false);
//	scene::ISceneNode* skydome=0;

	// create skybox and skydome
    skydome=smgr->addSkyDomeSceneNode(driver->getTexture("data/skys/skydome.jpg"),
             16,8,0.95f,2.0f);
    if (useShaders && useCgShaders)
    {
        skydome->setMaterialType((video::E_MATERIAL_TYPE)myMaterialType_tex);
        skydome->setMaterialTexture(1, driver->getTexture("data/skys/skystars.jpg"));
    }
    if (useShaders)
        skydome->setMaterialFlag(video::EMF_LIGHTING, false);
    else    
        skydome->setMaterialFlag(video::EMF_LIGHTING, globalLight);
        
    sunSphere = smgr->addBillboardSceneNode(0, core::dimension2d<f32>(1000.f, 1000.f));;//smgr->addSphereSceneNode(1.f);
    //sunSphere->setScale(vector3df(300.f, 300.f, 300.f));
    sunSphere->setVisible(false);
    sunSphere->setMaterialTexture(0, driver->getTexture("data/posteffects/sun.png"));
    sunSphere->setMaterialType((video::E_MATERIAL_TYPE)myMaterialType_depthSun);
    
    smokeTexture = driver->getTexture("data/bigterrains/smoke/dirt.png");    
    smokeWaterTexture = driver->getTexture("data/bigterrains/smoke/dirt_water.png");    

    MessageText::addText(L"Please wait [ *          ]", 1, true);

    for (int i = 0; i < MAX_CAR_DIRT; i++)
        car_dirttexture_array[i] = driver->getTexture(car_dirt_array[i]);

    MessageText::addText(L"Please wait [  *         ]", 1, true);

    //addToShadowNodes(skydome);
    /*    
	scene::ISceneNode* skybox=smgr->addSkyBoxSceneNode(
		driver->getTexture("data/skys/irrlicht2_up.jpg"),
		driver->getTexture("data/skys/irrlicht2_dn.jpg"),
		driver->getTexture("data/skys/irrlicht2_lf.jpg"),
		driver->getTexture("data/skys/irrlicht2_rt.jpg"),
		driver->getTexture("data/skys/irrlicht2_ft.jpg"),
		driver->getTexture("data/skys/irrlicht2_bk.jpg"));
    */
	driver->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS, true);

    // here was the creation if the bigTerrain
//	camera->setPosition(
//         core::vector3df(3400.0f,bigTerrain->getHeight(3400.f, 100.f)+5.f,100.0f));
//	camera->setFarValue(bigTerrain->getSmallTerrainSize()*FAR_VALUE_MULTI);

    NewtonSetBodyLeaveWorldEvent (nWorld, BodyLeaveWorld); 
    // here was the creation of the car

    loadGameplay("data/gameplay/gameplay.txt",
                 device,
                 driver,
                 smgr,
                 env,
                 nWorld,
                 soundEngine);

    MessageText::addText(L"Please wait [   *        ]", 1, true);

    loadObjectTypes("data/objects/object_types.txt", smgr, driver, nWorld);
    MessageText::addText(L"Please wait [      *     ]", 1, true);
    loadGrassTypes("data/objects/grass_types.txt", smgr, driver, nWorld);
    MessageText::addText(L"Please wait [        *   ]", 1, true);
    loadTreeTypes("data/objects/tree_types.txt", smgr, driver, device, nWorld);
    loadMyTreeTypes("data/objects/my_tree_types.txt", smgr, driver, device, nWorld);
    MessageText::addText(L"Please wait [         *  ]", 1, true);
    printPoolStat();
    vehiclePool = new CVehiclePool(device, smgr, driver, nWorld, soundEngine, "data/vehicles/vehicle_list.txt");
    //terrainPool = new TerrainPool(12, smgr, driver);
    MessageText::addText(L"Please wait [          * ]", 1, true);
    loadItinerTypes("data/itiner/itiner_types.txt", smgr, driver, nWorld);
    loadCompetitors("data/competitors/car.txt");
    playerCompetitor = new SCompetitor(444, "Player_1", "Player_1_Co-pilot", "Player_1_Team",
                                       0, 100, false);

    MessageText::addText(L"Please wait [           *]", 1, true);
    
    mapReader = new CMapReaderThread(device);

// 1. cam
    viewpos[0][12] = -6.f;
    viewpos[0][13] = 3.f;
    viewdest[0][13] = 1.f;
// left
    viewpos[1][12] = 0.f;
    viewpos[1][13] = 3.f;
    viewpos[1][14] = -6.f;
    viewdest[1][13] = 1.f;
// right
    viewpos[2][12] = 0.f;
    viewpos[2][13] = 3.f;
    viewpos[2][14] = 6.f;
    viewdest[2][13] = 1.f;
// behind
    viewpos[3][12] = 6.f;
    viewpos[3][13] = 3.f;
    viewdest[3][13] = 1.f;

// 2. cam
    viewpos[4][12] = 0.f;
    viewpos[4][13] = 1.2f;
    viewdest[4][12] = 6.5f;
    viewdest[4][13] = 0.f;
// left
    viewpos[5][12] = viewpos[4][12];
    viewpos[5][13] = viewpos[4][13];
    viewpos[5][14] = viewpos[4][14];
    viewdest[5][13] = 0.f;
    viewdest[5][14] = 6.5f;
// right
    viewpos[6][12] = viewpos[4][12];
    viewpos[6][13] = viewpos[4][13];
    viewpos[6][14] = viewpos[4][14];
    viewdest[6][13] = 0.f;
    viewdest[6][14] = -6.5f;
// behind
    viewpos[7][12] = 0.f;
    viewpos[7][13] = 1.2f;
    viewdest[7][12] = -6.5f;
    viewdest[7][13] = 0.f;

// 3. cam
    viewpos[8][12] = 0.f;
    viewpos[8][13] = 1.0f;
    viewpos[8][14] = 0.4f;
    viewdest[8][12] = 6.5f;
    viewdest[8][13] = 0.f;
    viewdest[8][14] = 0.4f;
// left
    viewpos[9][12] = viewpos[8][12];
    viewpos[9][13] = viewpos[8][13];
    viewpos[9][14] = viewpos[8][14];
    viewdest[9][13] = 0.f;
    viewdest[9][14] = 6.5f;
// right
    viewpos[10][12] = viewpos[8][12];
    viewpos[10][13] = viewpos[8][13];
    viewpos[10][14] = viewpos[8][14];
    viewdest[10][13] = 0.f;
    viewdest[10][14] = -6.5f;
// behind
    viewpos[11][12] = 0.f;
    viewpos[11][13] = 1.0f;
    viewpos[11][14] = 0.4f;
    viewdest[11][12] = -6.5f;
    viewdest[11][13] = 0.f;
    viewdest[11][14] = 0.4f;

    viewpos_cur = viewpos[view_num];
    viewdest_cur = viewdest[view_num];

    if (useShaders)
    {
        cLightPos_loc[12] = 3.0f;
        cLightPos_loc[13] = 1.0f;
        cLightDest_loc[12] = 15.0f;
        cLightDest_loc[13] = 0.0f;
    }
    else
    {
        cLightPos_loc[12] = 15.0f;
        cLightPos_loc[13] = 30.0f;
        cLightDest_loc[12] = 15.0f;
        cLightDest_loc[13] = 0.0f;
    }

/*
{
#define width  10
#define height 10
    scene::CGrassPatchSceneNode *grass[width*height];
    
	video::IImage* heightMap  = driver->createImageFromFile ("media/terrain-heightmap.bmp");  
	video::IImage* textureMap = driver->createImageFromFile ("media/terrain-grasscol.bmp");  
	video::IImage* grassMap   = driver->createImageFromFile ("media/terrain-grassmap.png"); 
    video::ITexture *tex1 = driver->getTexture("media/grass.png");

    scene::IWindGenerator *wind = scene::createWindGenerator( 30.0f, 3.0f );
    for (int x=0; x<width; ++x)
      for (int z=0; z<height; ++z)
      {   
            printf("grass %d %d\n", x, z);
        // add a grass node        
        grass[x*width + z] = new scene::CGrassPatchSceneNode((scene::ITerrainSceneNode*)bigTerrain->getTerrain(1.0f,1.0f), smgr, -1, core::vector3d<s32>(x,0,z), "grass", heightMap, textureMap, grassMap, wind);
        grass[x*width + z]->setMaterialFlag(video::EMF_LIGHTING, false);
        grass[x*width + z]->setMaterialType(video::EMT_TRANSPARENT_ALPHA_CHANNEL);
		grass[x*width + z]->getMaterial(0).TextureLayer[0].TextureWrap = video::ETC_CLAMP;
		grass[x*width + z]->getMaterial(0).MaterialTypeParam = 0.5f;
        grass[x*width + z]->setMaterialTexture(0, tex1);
		//grass[x*width + z]->setDebugDataVisible(-1);
        grass[x*width + z]->drop();
    }

//    CGrassPatchSceneNode* grass = new
//    scene::CGrassPatchSceneNode((scene::ITerrainSceneNode*)bigTerrain->getTerrain(1.0f,1.0f), smgr, -1, core::vector3d<s32>(1,0,1), "grass", heightMap, textureMap, grassMap, wind);

    new CGrassPatchSceneNode(
           (scene::ITerrainSceneNode*)bigTerrain->getTerrain(200.0f, 200.0f),
           smgr,
           -1,
           core::vector3df(200.0f, 0.f, 200.0f),
           "grass",
           wind
          );

    grass->setMaterialFlag(video::EMF_LIGHTING, false);
    grass->setMaterialType(video::EMT_TRANSPARENT_ALPHA_CHANNEL);
	grass->getMaterial(0).TextureLayer[0].TextureWrap = video::ETC_CLAMP;
	grass->getMaterial(0).MaterialTypeParam = 0.5f;
    grass->setMaterialTexture(0, tex1);
	//grass->setDebugDataVisible(1);
	grass->setVisible(true);
    //grass[x*width + z]->drop();
    //smgr->getParameters()->setAttribute(scene::ALLOW_ZWRITE_ON_TRANSPARENT, true);

}
*/
    
/*
    device->run();
    MessageText::addText(
            L"Welcome in the Dakar 2010 Game\n\n" \
            L"Go through (slowly) all the flashing checkpoints\n\n" \
            L"Press ESC to access Main Menu" \
            L""
            , 5);
*/
    
	// create event receiver
   	dprintf(printf("DEBUG: create event receiver game\n");)
	eventreceiver_game* game_receiver = new eventreceiver_game(device, 0, skydome, driver, smgr, env, nWorld, soundEngine);
   	dprintf(printf("DEBUG: create event receiver menu\n");)
	eventreceiver_menu* menu_receiver = new eventreceiver_menu(device, 0, skydome, driver, smgr, env, nWorld, soundEngine);

   	dprintf(printf("DEBUG: create event receiver dummy\n");)
	eventreceiver_dummy* dummy_receiver = new eventreceiver_dummy(device, 0, skydome, driver, smgr, env, nWorld, soundEngine);
	
	game_receiver->setOther(menu_receiver);
	menu_receiver->setOther(game_receiver);

   	device->setEventReceiver(dummy_receiver);
    device->getCursorControl()->setPosition(0.f,0.f);
    
   	device->setEventReceiver(game_receiver);
   	
   	dprintf(printf("DEBUG: check restore state\n");)

    if (!restoreState())
    {
        if (start_with_mainmenu)
        {
            menu_receiver->openMainWindow();
        } else {
            if (!loadGame(SAVE_FILE))
                currentStage = 0;
            startGame(currentStage);
        }
    }

   	dprintf(printf("DEBUG: device run device %p\n", device);)
    device->run();
    
    menu_receiver->startEnter();

    int lastFPS = 1;
    lasttick = device->getTimer()->getTime();
    u32 lastPrintout = lasttick;
    u32 lastMP = lasttick;
    tick = lasttick;
    vector3df lookDir;
    vector3df lookPos;
    const float lookFOV = cosf(camera->getFOV());
    //NewtonSetMinimumFrameRate( nWorld, min_fps);
    vector3df dynCamPos;
    vector3df dynCamRight;
    vector3df dynCamUp;
    //int dynCamReset = 1;
    float dynCamDist;
    core::vector3df nearestCP;
    unsigned int drawnObjs;
    unsigned int drawnObjsPolys;
    unsigned int notDrawnObjs;
    unsigned int notDrawnObjsPolys;
    unsigned int shadowObjs;
    unsigned int shadowObjsPolys;
    float compassDotp = 0.f;
    float compassAngle = 0.f;
    
    int failed_render = 0;
    //intf("device %p\n", device);
    dprintf(printf("device %p\n", device));
    MessageText::hide();
    while(device->run())
    { 
        if (device->isWindowActive())
        {
            tick = device->getTimer()->getTime();
            pdprintf(printf("1\n"));
            if (!fpsCam && car && inGame == 0)
            {
                vector3df campos;
                vector3df camtar;
                vector3df centar;
                bool useCarlosView = false;
                int carlosNum = 1;
                
                if (followCarlos && raceEngine)
                {
                    if (raceEngine->getStarters()[carlosNum]->competitor == playerCompetitor) carlosNum--;
                    if (raceEngine->getStarters()[carlosNum]->vehicle)
                    {
                        useCarlosView = true;
                        printf("%f %f\n", raceEngine->getStarters()[carlosNum]->vehicle->getSpeed()*1.6f, raceEngine->getStarters()[carlosNum]->vehicle->getTorqueReal());
                    }
                }
                
                //vector3df carpos = car->getMatrix().getTranslation();
                matrix4 tcentar;
                tcentar.setTranslation(vector3df(viewdest_cur[12], viewpos_cur[13] , viewdest_cur[14]));
                if (!useCarlosView)
                {
                    /*vector3df */campos = (car->getMatrix() * viewpos_cur).getTranslation();
                    /*vector3df */camtar = (car->getMatrix() * viewdest_cur).getTranslation();
                    /*vector3df */centar = (car->getMatrix() * tcentar).getTranslation();
                }
                else
                {
                    campos = (raceEngine->getStarters()[carlosNum]->vehicle->getMatrix() * viewpos_cur).getTranslation();
                    camtar = (raceEngine->getStarters()[carlosNum]->vehicle->getMatrix() * viewdest_cur).getTranslation();
                    centar = (raceEngine->getStarters()[carlosNum]->vehicle->getMatrix() * tcentar).getTranslation();
                }
                
                camera->setTarget(camtar);
                if ((view_num+view_mask)!=0 || !useDynCam || dynCamReset)
                {
                    //campos = car->getMatrix() * viewpos_cur;
                    dynCamPos = campos;
                    camera->setPosition(dynCamPos);
                    dynCamDist = fabsf((centar - dynCamPos).getLength());
                    //dynCamPos = carpos;
                    dynCamReset = false;
                }
                else
                {
                    vector3df dir = (dynCamPos - centar).normalize();
                   
                    float fact = fabsf(car->getSpeed())*0.02f;
                   
                    dir = dir * (dynCamDist + fact * fact) ;
                   
                    dynCamPos = centar + dir;
                    camera->setPosition(dynCamPos);
                }
            }
            pdprintf(printf("2\n"));
            if (!driver->beginScene(true, true, SColor(0,192,192,192)))
            {
                printf("beginScene failed (Irrlicht bug). Program will be quit.\n");
                reinitialize = true;
                //env->drawAll();
                driver->endScene();
                int rtt_count = 0;
                
                for (int i = 0; i< driver->getTextureCount(); i++)
                {
                    if (driver->getTextureByIndex(i)->isRenderTarget())
                    {
                        rtt_count++;
                    }
                }
                
                printf("rtt_count: %d driver->check %d failed renders %d\n", rtt_count, driver->checkDriverReset(), failed_render);
                if (quitGame) break;
                failed_render++;
                if (failed_render < 5)
                {
                    continue;
                }
                break;
            }     
            pdprintf(printf("3\n"));
            if (showCompass && compassArrow->isVisible() && car && inGame == 0)
            {
                vector3df pos = (car->getMatrix() * viewdest_cur).getTranslation();
                pos.Y += 3.6f;
                
                compassArrow->setPosition(pos);
                
                lookDir = vector3df(0.f, 0.f, 1.f);

                compassDotp = lookDir.dotProduct(nearestCP);
                compassAngle = acosf(compassDotp) * 57.2956; //* 180.f / 3.14f;
                if (nearestCP.X*lookDir.Z - nearestCP.Z*lookDir.X < 0) compassAngle = -compassAngle;

                compassArrow->setRotation(vector3df(90.f, compassAngle, 0.f));
            }
            pdprintf(printf("4\n"));
            if (car && inGame == 0 && useShaders)
            {
                if (car->getLight())
                {
                    cLightPos = car->getMatrix() * cLightPos_loc;
                    cLightTar = car->getMatrix() * cLightDest_loc;
                    cLightDir = cLightTar - cLightPos;
                }
                //if (useCgShaders)
                    updateEffects(tick);
            }
            pdprintf(printf("5\n"));

            lookDir = (camera->getTarget() - camera->getPosition()).normalize();
            lookPos = camera->getPosition();

            notDrawnObjs = drawnObjs = shadowObjs = 0;
            notDrawnObjsPolys = drawnObjsPolys = shadowObjsPolys = 0;
            for (int i = 0; i<objectNodes.size();i++)
            {
                vector3df objDir = (objectNodes[i]->getPosition() - lookPos).normalize();
                float dist = camera->getPosition().getDistanceFrom(objectNodes[i]->getPosition());
                if(dist < 30.f || (lookDir.dotProduct(objDir)>lookFOV && (/*useCgShaders ||*/ dist < objectVisibilityLimit)))
                {
                    objectNodes[i]->setVisible(true);
                    drawnObjs++;
                    //drawnObjsPolys += objectNodes[i]->getTriangleSelector()->getTriangleCount();
                }
                else
                {
                    objectNodes[i]->setVisible(false);
                    notDrawnObjs++;
                    //notDrawnObjsPolys += objectNodes[i]->getTriangleSelector()->getTriangleCount();
                }
            }
            pdprintf(printf("6\n"));

            // shadow mapping
            if (shadows && shadowMapGame)
            {
                lightCam->setPosition(vector3df(camera->getPosition().X+2.0f,
                                      camera->getPosition().Y + 150.f,
                                      camera->getPosition().Z + 2.0f));
                lightCam->setTarget(camera->getPosition());
                lightCam->setFarValue(DEFAULT_FAR_VALUE);
                smgr->setActiveCamera(lightCam);

                lightCam->OnAnimate(tick);
                lightCam->OnRegisterSceneNode();
                lightCam->render();
                driver->setTransform(ETS_VIEW, lightCam->getViewMatrix());
                driver->setTransform(ETS_PROJECTION, lightCam->getProjectionMatrix());

    			driver->setRenderTarget(shadowMapGame, true, true, video::SColor(0, 0, 0, 0));
                
                for (int i = 0; i<shadowNodes.size();i++)
                {
                    if (shadowNodes[i]->isVisible() && camera->getPosition().getDistanceFrom(shadowNodes[i]->getPosition())<160.f)
                    {
                        const u32 CurrentMaterialCount = shadowNodes[i]->getMaterialCount();
                        core::array<s32> BufferMaterialList(CurrentMaterialCount);
                        bool cardraw = false;
                        BufferMaterialList.set_used(0);
                            
                        for(u32 m = 0;m < CurrentMaterialCount;++m)
                        {
                            BufferMaterialList.push_back(shadowNodes[i]->getMaterial(m).MaterialType);
                            shadowNodes[i]->getMaterial(m).MaterialType = (E_MATERIAL_TYPE)myMaterialType_shadow;
        				}
                        if (car && (car->getSceneNode()==shadowNodes[i] ||
                            car->getTyreSceneNode(0)==shadowNodes[i] ||
                            car->getTyreSceneNode(1)==shadowNodes[i] ||
                            car->getTyreSceneNode(2)==shadowNodes[i] ||
                            car->getTyreSceneNode(3)==shadowNodes[i])
                            )
                        {
                            max_shadow = 1.0f;
                            cardraw = true;
                        }
                        shadowNodes[i]->OnAnimate(tick);
                        shadowNodes[i]->OnRegisterSceneNode();
                        shadowNodes[i]->render();
                        shadowObjs++;
                        //shadowObjsPolys += objectNodes[i]->getTriangleSelector()->getTriangleCount();
                        if (cardraw)
                        {
                           max_shadow = 0.0f;
                           cardraw = false;
                        }
    
                        const u32 BufferMaterialListSize = BufferMaterialList.size();
                        for(u32 m = 0;m < BufferMaterialListSize;++m)
                            shadowNodes[i]->getMaterial(m).MaterialType = (E_MATERIAL_TYPE)BufferMaterialList[m];
                    }
                }
                smgr->setActiveCamera(camera);
            }

            ((eventreceiver*)device->getEventReceiver())->prerender();
            pdprintf(printf("8\n"));
            const int usedScreenRTT = currentScreenRTT / 2;

            // old way depth
            if (depth_effect && useCgShaders)
            {
                pdprintf(printf("10e\n"));
                driver->setRenderTarget(depthRTT, true, true, video::SColor(0, 0, 0, 255));
                   
                smgr->setActiveCamera(camera);
                camera->OnAnimate(tick);
                camera->OnRegisterSceneNode();
                camera->render();
                driver->setTransform(ETS_VIEW, camera->getViewMatrix());
                driver->setTransform(ETS_PROJECTION, camera->getProjectionMatrix());
                   
                //if (!fpsCam)
                {
                    for (int i = 0; i<depthNodes.size();i++)
                    {
                        if (depthNodes[i]->isVisible())
                        {
                            const u32 CurrentMaterialCount = depthNodes[i]->getMaterialCount();
                            core::array<irr::s32> BufferMaterialList(CurrentMaterialCount);
                            BufferMaterialList.set_used(0);
                                                                                                          
                            for(u32 m = 0;m < CurrentMaterialCount;++m)
                            {
                                BufferMaterialList.push_back(depthNodes[i]->getMaterial(m).MaterialType);
                                depthNodes[i]->getMaterial(m).MaterialType = (E_MATERIAL_TYPE)myMaterialType_depth;
                            }
           
                            depthNodes[i]->OnAnimate(tick);
                            depthNodes[i]->OnRegisterSceneNode();
                            depthNodes[i]->render();
           
                            const u32 BufferMaterialListSize = BufferMaterialList.size();
                            for(u32 m = 0;m < BufferMaterialListSize;++m)
                                depthNodes[i]->getMaterial(m).MaterialType = (E_MATERIAL_TYPE)BufferMaterialList[m];
                        }
                    }
                }
                   
                sunSphere->setPosition(camera->getPosition()+vector3df(100.f, 250.f, 600.f));
                sunSphere->setVisible(true);
                sunSphere->OnAnimate(tick);
                //sunSphere->OnRegisterSceneNode();
                sunSphere->render();
                sunSphere->setVisible(false);
                   
                pdprintf(printf("9\n"));
            }
            pdprintf(printf("10\n"));

            // post effects, or not
            if (useScreenRTT && depth_effect && useCgShaders)
            {
                pdprintf(printf("10b\n"));
                driver->setRenderTarget(screenRTT[(usedScreenRTT)%MAX_SCREENRTT], true, true, video::SColor(0, 255, 255, 0));
                pdprintf(printf("10c\n"));
                smgr->drawAll();
                pdprintf(printf("10d\n"));
                /* // for the new way depth
                driver->setRenderTarget(depthRTT, true, true, video::SColor(0, 0, 0, 0));
                //smgr->setActiveCamera(camera);
                //camera->OnAnimate(tick);
                //camera->OnRegisterSceneNode();
                //camera->render();
                driver->setTransform(ETS_VIEW, camera->getViewMatrix());
                driver->setTransform(ETS_PROJECTION, camera->getProjectionMatrix());
                for (int i = 0; i<depthNodes.size();i++)
                {
                    depthNodes[i]->setVisible(true);
                    depthNodes[i]->OnAnimate(tick);
                    //depthNodes[i]->OnRegisterSceneNode();
                    depthNodes[i]->render();
                    depthNodes[i]->setVisible(false);
                }
                */
	            driver->setRenderTarget(0, true, true, video::SColor(0, 255, 0, 0));
                if (!useBgImageToRender)
	            {
                    int i = 1;
                    int j = 1;
                    screenQuad.getMaterial().setTexture(0, screenRTT[(usedScreenRTT)%3]);
                    //for (;i<MAX_SCREENRTT && j<3;i+=1, j++)
                    //  screenQuad.getMaterial().setTexture(j, screenRTT[(usedScreenRTT+i)%3]);
                    screenQuad.getMaterial().setTexture(1, depthRTT);
                    screenQuad.getMaterial().setTexture(2, motiondir_map[view_mask]);
                    screenQuad.getMaterial().MaterialType = (E_MATERIAL_TYPE)myMaterialType_screenRTT;
                    screenQuad.render(driver);
                }
                //currentScreenRTT++;
           }
           else
           {
                driver->setRenderTarget(0, true, true, video::SColor(0, 0, 0, 255));
                pdprintf(printf("10b\n"));
                smgr->drawAll();
                pdprintf(printf("10c\n"));
           }
           if (draw_hud && car)
           {
                int gear = car->getGear();
                if (gear >= 0 && gear < MAX_HUD)
                {
                    if (car->getSpeed()>=0.f || car->getTorqueReal() > -0.01f)
                        hudImage->setImage(hudTextures[gear]); // 1 - 6
                    else
                        hudImage->setImage(hudTextures[MAX_HUD]); // R
                }
                else
                    hudImage->setImage(hudTextures[0]);
           }
           //driver->setRenderTarget(0, true, true, video::SColor(0, 255, 0, 0));
           //test.getMaterial().setTexture(0, hudCompassTexture);
           //test.render(driver);
           // draw hud stuff 1: compass
           pdprintf(printf("11\n"));
           if (bigTerrain)
           {
                pdprintf(printf("11a\n"));
                nearestCP = bigTerrain->updatePos(camera->getPosition().X, camera->getPosition().Z, density_objects, false);
                pdprintf(printf("11b\n"));
                if (showCompass && inGame == 0)
                {
                    lookDir.Y = 0.f;
                    nearestCP.Y = 0.f;
                    lookDir.normalize();
                    nearestCP.normalize();
                    compassDotp = lookDir.dotProduct(nearestCP);
                    compassAngle = acosf(compassDotp) * 57.2956; //* 180.f / 3.14f;
                    if (nearestCP.X*lookDir.Z - nearestCP.Z*lookDir.X < 0) compassAngle = -compassAngle;
                    
                    hudPos = hudPos3;
                    hudPos.rotateBy(compassAngle, hudCenter3);
                    hudPos2d.X = (s32)hudPos.X;
                    hudPos2d.Y = (s32)hudPos.Y;
    
                    //hudPos2d2 = hudCenter2d3 - (hudPos2d - hudCenter2d3);
                    
                    //driver->draw2DLine(hudPos2d, hudPos2d2, SColor(255,255,0,0));
                    //hudPos2d2 = hudPos2d;
    
                    hudPos = hudPos3l;
                    hudPos.rotateBy(compassAngle, hudCenter3);
                    hudPos2d2.X = (s32)hudPos.X;
                    hudPos2d2.Y = (s32)hudPos.Y;
                    //driver->draw2DLine(hudPos2d, hudPos2d2, SColor(255,255,0,0));
    
                    hudPos = hudPos3r;
                    hudPos.rotateBy(compassAngle, hudCenter3);
                    hudPos2d3.X = (s32)hudPos.X;
                    hudPos2d3.Y = (s32)hudPos.Y;
                    //driver->draw2DLine(hudPos2d, hudPos2d2, SColor(255,255,0,0));
    
                    hudPos = hudPos3c;
                    hudPos.rotateBy(compassAngle, hudCenter3);
                    hudPos2d4.X = (s32)hudPos.X;
                    hudPos2d4.Y = (s32)hudPos.Y;
                    
                    driver->draw2DLine(hudPos2d, hudPos2d2, SColor(255,255,0,0));
                    driver->draw2DLine(hudPos2d2, hudPos2d4, SColor(255,255,0,0));
                    driver->draw2DLine(hudPos2d4, hudPos2d3, SColor(255,255,0,0));
                    driver->draw2DLine(hudPos2d3, hudPos2d, SColor(255,255,0,0));
                }
           }
           //((eventreceiver*)device->getEventReceiver())->prerender();
           pdprintf(printf("12\n"));
           env->drawAll();
           pdprintf(printf("12b\n"));
           ((eventreceiver*)device->getEventReceiver())->render();
           //printf("render\n");
           // draw hud stuff 2: speedometer
           pdprintf(printf("13\n"));
           if (draw_hud && car && inGame == 0)
           {
                // speed
                f64 speed = (f64)(fabsf(car->getSpeed())*hud_speed_multiplier);
                hudPos = hudPos1;
                hudPos.rotateBy(speed, hudCenter1);
                
                hudPos2d.X = (s32)hudPos.X;
                hudPos2d.Y = (s32)hudPos.Y;
                
                driver->draw2DLine(hudPos2d, hudCenter2d1,SColor(255,255,0,0));

                // engine rotate
                if (car->getEngineRotate() > 0.f)
                    speed = (f64)(fabsf(car->getEngineRotate())*60.f-30.f);
                else
                    speed = 0.0;
                hudPos = hudPos2;
                hudPos.rotateBy(speed, hudCenter2);
                
                hudPos2d.X = (s32)hudPos.X;
                hudPos2d.Y = (s32)hudPos.Y;
                
                driver->draw2DLine(hudPos2d, hudCenter2d2,SColor(255,255,0,0));
           }
           driver->endScene();
           pdprintf(printf("14\n"));
           // display frames per second in window title
		   int fps = driver->getFPS();
           //tick = device->getTimer()->getTime();
		   if (lastFPS != fps || tick - lastPrintout > 1000)
		   {
                if (car && bigTerrain)
                {
                    core::stringw str;
                    if (display_extra_info)
                    {
                        str = L"FPS: ";
                        str += (int)fps;
                        fpsText->setText(str.c_str());
        
                        str = L"POLYS: ";
                        str += driver->getPrimitiveCountDrawn();
                        str += L", D: ";
                        str += drawnObjs;
                        str += L" (";
                        str += drawnObjsPolys;
                        str += L"), ND: ";
                        str += notDrawnObjs;
                        str += L" (";
                        str += notDrawnObjsPolys;
                        str += L"), SH: ";
                        str += shadowObjs;
                        str += L" (";
                        str += shadowObjsPolys;
                        str += L")";
                        polyText->setText(str.c_str());
        
                        str = L"POS: ";
                        str += (int)camera->getPosition().X;
                        str += ", ";
                        str += (int)camera->getPosition().Y;
                        str += ", ";
                        str += (int)camera->getPosition().Z;
                        str += " (";
                        str += (int)(camera->getPosition().X/20.f);
                        str += ", ";
#ifdef USE_IMAGE_HM
                        str += (int)(bigTerrain->getHeightMap()->getDimension().Height - 1 - (camera->getPosition().Z/20.f));
#else
                        str += (int)(bigTerrain->getHeightMap()->getYSize() - 1 - (camera->getPosition().Z/20.f));
#endif
                        str += ")";
                        posText->setText(str.c_str());
                        
                    }
/*
                    str = L"Demage: ";
                    str += (int)(car->getDemagePer());
                    str += "%";
                    demageText->setText(str.c_str());
    
                    str = L"Speed: ";
                    str += (int)(car->getSpeed()*1.6f); // 3.0f
                    str += " km/h (";
                    if (gear_type=='a')
                        str += L"A";
                    else
                        str += L"M";
                    str += ": ";
                    str += (int)(car->getGear()/*1.6); // 3.0f
                    str += ")";
                    speedText->setText(str.c_str());
                    
                    //u32 endTime = bigTerrain->getEndTime() ? bigTerrain->getEndTime() : tick + bigTerrain->getPenality();
                    u32 diffTime;
                    if (bigTerrain->getTimeEnded())
                        diffTime = bigTerrain->getCurrentTime();
                    else
                        diffTime = bigTerrain->getCurrentTime() + bigTerrain->getPenality();//bigTerrain->getStartTime()? (endTime - bigTerrain->getStartTime()):0;
                    // stage time
                    str = L"Time: ";
                    BigTerrain::addTimeToStr(str, diffTime);
                    // stagetime plus global time
                    diffTime += globalTime;
                    str += L" (";
                    BigTerrain::addTimeToStr(str, diffTime);
                    //str += day_delta_multi;
                    str += L")";
                    timeText->setText(str.c_str());
*/    
                    //bigTerrain->updatePos(camera->getPosition().X, camera->getPosition().Z, density_objects, density_grasses, false);
                }
                pdprintf(printf("14c\n"));
                MessageText::updateText(tick);
                pdprintf(printf("14cb\n"));
                ItinerHud::updateItiner(tick);

                lastFPS = fps;
                lastPrintout = tick;
                if (quitGame) break;
                calculate_day_delta(tick);
          }
          pdprintf(printf("14d\n"));
          if (car && bigTerrain)
          {
                core::stringw str;
                str = L"Demage: ";
                str += (int)(car->getDemagePer());
                str += "%";
                    /*
                if (car)
                {
                    str += (int)(car->getFriction(0)*100.f);
                    str += ", ";
                    str += (int)(car->getFriction(1)*100.f);
                    str += ", ";
                    str += (int)(car->getFriction(2)*100.f);
                    str += ", ";
                    str += (int)(car->getFriction(3)*100.f);

                    str += car->getHitBody(0);
                    str += ", ";
                    str += car->getHitBody(1);
                    str += ", ";
                    str += car->getHitBody(2);
                    str += ", ";
                    str += car->getHitBody(3);

                    str += car->getHitBodyID(0);
                    str += ", ";
                    str += car->getHitBodyID(1);
                    str += ", ";
                    str += car->getHitBodyID(2);
                    str += ", ";
                    str += car->getHitBodyID(3);
                }
                    */
                demageText->setText(str.c_str());
        
                str = L"Speed: ";
                str += (int)(car->getSpeed()*1.6f/*3.0f*/);
                str += " km/h (";
                if (gear_type=='a')
                    str += L"A";
                else
                    str += L"M";
                str += ": ";
                str += (int)(car->getGear()/*1.6/*3.0f*/);
                str += ")";
                speedText->setText(str.c_str());
                
                //u32 endTime = bigTerrain->getEndTime() ? bigTerrain->getEndTime() : tick + bigTerrain->getPenality();
                u32 diffTime;
                if (bigTerrain->getTimeEnded())
                    diffTime = bigTerrain->getCurrentTime();
                else
                    diffTime = bigTerrain->getCurrentTime() + bigTerrain->getPenality();//bigTerrain->getStartTime()? (endTime - bigTerrain->getStartTime()):0;
                // stage time
                str = L"Time: ";
                BigTerrain::addTimeToStr(str, diffTime);
                // stagetime plus global time
                diffTime += globalTime;
                str += L" (";
                BigTerrain::addTimeToStr(str, diffTime);
                //str += day_delta_multi;
                str += L")";
                timeText->setText(str.c_str());
          }
          if (editorMode && car && bigTerrain && inGame == 0)
          {
                pdprintf(printf("14e\n"));
                updateEditor(); // TODO: put back into the one sec. update
          }
          pdprintf(printf("15\n"));
          
          if (isMultiplayer)
          {
                if (tick > lastMP + send_server_delay)
                {
                    updateConnection();
                    lastMP = tick;
                }
          }
          // Update newton 100 times / second
          //tick = device->getTimer()->getTime();
          if (inGame == 0)
          {
              if (tick > lasttick + 16/*(1000/min_fps/*(lastFPS+1)) && inGame == 0*/)
              {
                  if (bigTerrain)
                      bigTerrain->checkMapsQueue();
                  if (bigTerrain)
                      bigTerrain->updateTime(tick);
                  //#ifndef USE_EDITOR
                      if (raceEngine)
                          raceEngine->update(tick, camera->getPosition(), playerCompetitor);
                  //#endif
                  //for (int i = 0; i<2; i++)
                  while (lasttick+16<tick)
                  {
                      if (car)
                      {
                          //if (car->getBrake()>0.f)
                          //    car->applyHandBrakes(car->getBrake());
                          //car->applyTireTorque(car->getTorqueReal());
                          float kbs = car->getSteerKb();
                          //printf("kbs: %f\n", kbs);
                          if (fabsf(kbs) > 0.01f)
                          {
                              float cs = car->getSteer();
                              if (fabsf(kbs-cs) > car->getSteerRate()/*0.01f*/)
                              {
                                    //car->setSteering(kbs);
                                    if (kbs > cs)
                                       car->setSteering(cs + car->getSteerRate());
                                    else
                                        if (kbs < cs)
                                           car->setSteering(cs - car->getSteerRate());
                              }
                              else
                                       car->setSteering(kbs);
                          }
                          //car->applySteering(car->getSteer());
                          //if (gear_type=='a')
                          //  car->updateGear();
                          
                          if (isMultiplayer)
                            updateOtherDatas();
    
                          pdprintf(printf("16b\n"));
                          vehiclePool->updateActiveVehicles();
                      }
                      //NewtonUpdate(nWorld, (1.0f/min_fps)*0.5f/*1.066667f*/);
                      //NewtonUpdate(nWorld, (1.0f/min_fps)/*1.066667f*/);
                      pdprintf(printf("17\n"));
                      //NewtonWorldCriticalSectionLock(nWorld);
                      pdprintf(printf("17b\n"));
                      NewtonUpdate(nWorld, 0.015f/*1.066667f*/);
                      //NewtonWorldCriticalSectionUnlock(nWorld);
                      pdprintf(printf("17c\n"));
                      //NewtonUpdate(nWorld, 0.0167f/*1.066667f*/);
                      //NewtonUpdate(nWorld, ((float)(tick-lasttick)/1000.f)*1.066667f);
                      //NewtonUpdate(nWorld, 0.0106f);
                      lasttick += 16;
                  }
                  pdprintf(printf("18\n"));
                  soundEngine->setListenerPosition(camera->getPosition(), camera->getTarget()-camera->getPosition());
                  //lasttick = tick;
              }
          }
          else
            lasttick = tick;

       }
       else
       {
            menu_receiver->openMainWindow();
            /*
            driver->beginScene(true, true, SColor(0,192,192,192));
            smgr->drawAll();
            env->drawAll();
            driver->endScene();
            */
            device->sleep(100);
            lasttick = device->getTimer()->getTime();
       }
	}

    if (reinitialize)
    {
        printf("save state\n");
        saveState();
    }
    
    printf("end game\n");
	endGame();
    printf("disconnect from server\n");
	disconnectFromServer(false);
    printf("delete event receivers\n");
	delete menu_receiver; //.releaseResources();
	delete game_receiver; //.releaseResources();
	delete dummy_receiver;
	
	printf("release game stuffs\n");
	releaseGameStuff();
	
	printf("release competitors\n");
	if (!reinitialize)
	{
    	destroyCompetitors();
    	delete playerCompetitor;
    	playerCompetitor = 0;
    	CRaceEngine::getRaceState().clear();
    }

	printf("release pools\n"); // will release tree designs, and my tree designs
	releasePools();

	printf("release itiner types\n");
	releaseItinerTypes();
	
	printf("delete vehiclePool");
	if (vehiclePool)
	{
	   delete vehiclePool;
	   vehiclePool = 0;
    }
    
    printf("release fonts\n");
    releaseFonts();

    printf("cleanup materials\n");
	CleanUpMaterials(nWorld);
	// finish newton & irrlicht
    printf("destroy world\n");
	NewtonDestroy(nWorld);

    printf("sound engine drop\n");
#ifdef USE_MY_SOUNDENGINE
    delete soundEngine;
#else
    soundEngine->drop(); // delete engine
#endif

    printf("cg shader gpu drop\n");
    deleteCgShaders(driver);
    
    printf("stop map reader thread\n");
    if (mapReader)
    {
        mapReader->kill();
        delete mapReader;
        mapReader = 0;
    }
    
    printf("scene manager clean\n");
    smgr->clear();

    //printf("scene manager drop\n");
    //smgr->drop();

    printf("gui env clean\n");
    env->clear();

    //printf("gui env drop\n");
    //env->drop();

    printf("driver remove all textures\n");
    driver->removeAllTextures();
    printf("driver remove all hw buffers\n");
    driver->removeAllHardwareBuffers();
    printf("driver drop\n");
    driver->drop();

    printf("device drop\n");
    //if (reinitialize)
        device->closeDevice();
    //else
    //    device->drop();
    
    } while (reinitialize);

    printf("return 0\n");
    return(0);
}

// callback to handle when the car leave the world
void BodyLeaveWorld (const NewtonBody* body, int threadId)
{
    // Set the position of the body
    dprintf(printf("leave world b %p c->b %p c %p bt %p\n", body, car?car->GetRigidBody():0, car, bigTerrain));
    if (car && bigTerrain && car->GetRigidBody() == body)
    {
        car->reset(bigTerrain->getStartPos());
        /*
        matrix4 mat = car->getMatrix();
        core::vector3df rot = mat.getRotationDegrees();
        mat.setTranslation(bigTerrain->getStartPos());
        mat.setRotationDegrees(vector3df(0.f, rot.Y, 0.f));
        car->setMatrixWithNB(mat);
        */
    }
}

