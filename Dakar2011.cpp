/****************************************************************
*                                                               *
*    Name: Dakar2011.cpp                                        *
*                                                               *
*    Creator: Balazs Tuska                                      *
*                                                               *
*    Description:                                               *
*       This file contains the main loop of the game. It        *
*       handles the keyboard and joystick events.               *
*                                                               *
****************************************************************/

// debug defines
/*
-DMY_DEBUG
-DUSE_EDITOR
-DUSE_MY_SOUNDENGINE
*/

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
#include "MyRoad.h"

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
const static video::SColor colors_for_hdr[] =
{
	video::SColor(255,96,96,96),
	video::SColor(255,96,96,96),
	video::SColor(255,96,96,96),
	video::SColor(255,96,96,96)
};

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
    startNewGame = BrandNewGame;
    for (int i = 0; i < MAX_BGIMAGE+1; i++) bgImagesTextures[i] = 0;
    for (int i = 0; i < MAX_HUD+2; i++) hudTextures[i] = 0;
    for (int i = 0; i < MAX_SCREENRTT; i++) screenRTT[i] = 0;
    currentScreenRTT = 0;
    depthRTT = 0;
    bgImageTexture = 0;
    blurmap = 0;
    blurmapSide = 0;
    for (int i = 0; i < view_multi; i++) motiondir_map[i] = 0;
    for (int i = 0; i < MAX_CAR_DIRT; i++) car_dirttexture_array[i] = 0;
    //////
    
    getcwd(currentDirectory, 256);
    readSettings("data/settings.txt");
    
    reinitialize = false;

#ifdef __linux__
    if (driverType == video::EDT_DIRECT3D9 || driverType == video::EDT_DIRECT3D8) driverType = video::EDT_OPENGL;
#endif

    irr::SIrrlichtCreationParameters param;
    param.DriverType = driverType;
    param.Fullscreen = full_screen;
    param.WindowSize = dimension2d<s32>(resolutionX, resolutionY);
    param.Stencilbuffer = stencil_shadows; // shadows;
    param.AntiAlias = anti_aliasing;
    param.Vsync = vsync;
    param.HighPrecisionFPU = high_precision_fpu;

    // remove me
    if (auto_resolution==2)
    {
        irr::IrrlichtDevice* nullDev = createDevice(EDT_NULL);
        dprintf(printf("Use auto-detection optimal\n"));
        if (nullDev)
        {
            dprintf(printf("null device found auto-detection is possible."));
            core::dimension2d<u32> resd = nullDev->getVideoModeList()->getDesktopResolution();
            core::dimension2d<u32> res = resd;
            float rated = (float)resd.Width / (float)resd.Height;
            float minRateDiff = 10.f;
            s32 dep = nullDev->getVideoModeList()->getDesktopDepth();
            for(int i=0;i<nullDev->getVideoModeList()->getVideoModeCount();i++)
            {
                core::dimension2d<u32> resi = nullDev->getVideoModeList()->getVideoModeResolution(i);
                float ratei = (float)resi.Width / (float)resi.Height;
                if (resi.Width >= 1024 && resi.Width <= 1280 && fabsf(ratei-rated) <= minRateDiff+0.00001f)
                {
                    minRateDiff = fabsf(ratei-rated);
                    res = resi;
                }
            }
            nullDev->drop();
            display_bits = dep;
            resolutionX = res.Width;
            resolutionY = res.Height;
            dprintf(printf("null device found auto-detection is possible for optimal. detected: %dx%dx%d\n", resolutionX, resolutionY, display_bits));
            printf("auto-detection optimal, detected: %dx%dx%d\n", resolutionX, resolutionY, display_bits);
        }
        //assert(0);
    }
    else
    if (auto_resolution==1 && full_screen)
    {
        irr::IrrlichtDevice* nullDev = createDevice(EDT_NULL);
        dprintf(printf("Use full screen with auto-detection\n"));
        if (nullDev)
        {
            dprintf(printf("null device found auto-detection is possible."));
            core::dimension2d<u32> res = nullDev->getVideoModeList()->getDesktopResolution();
            s32 dep = nullDev->getVideoModeList()->getDesktopDepth();
            
            nullDev->drop();
            
            display_bits = dep;
            resolutionX = res.Width;
            resolutionY = res.Height;
            dprintf(printf("null device found auto-detection is possible. detected: %dx%dx%d\n", resolutionX, resolutionY, display_bits));
        }
        else
            dprintf(printf("null device not found auto-detection is not possible!!!\n"));
    }
    param.Bits = display_bits;
    param.WindowSize = dimension2d<s32>(resolutionX, resolutionY);

    irr::IrrlichtDevice* device = irr::createDeviceEx(param);

    if (!device)
    {
        myError(1, "Cannot initialize Irrlicht device!");
        return 1;  // failed to initialize driver
    }

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
    
    CScreenQuad screenQuad(driverType == video::EDT_OPENGL /*|| driverType == video::EDT_OPENGL3*/, flip_vert);
    CScreenQuad speedPalca(false, false);
    CScreenQuad speedPalca2(false, false);
    CScreenQuad speedHud(false, false);

    assert(view_multi==4);
    if (driver->getDriverType() == EDT_OPENGL /*|| driver->getDriverType() == EDT_OPENGL3*/)
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
    motiondir_map[1] = driver->getTexture("data/posteffects/motiondir_sideLeft.png");
    motiondir_map[2] = driver->getTexture("data/posteffects/motiondir_sideRight.png");
    motiondir_map[3] = motiondir_map[0];
    
    if (driver->queryFeature(video::EVDF_TEXTURE_NSQUARE) &&
        driver->queryFeature(video::EVDF_TEXTURE_NPOT))
    {
        shitATI = false;
    }
    else
    {
        shitATI = true;
    }
    //recreateRTTs(driver);
    
	// Newton vars
#if NEWTON_MINOR_VERSION < 24
	NewtonWorld *nWorld = NewtonCreate(NULL, NULL);
#else
	NewtonWorld *nWorld = NewtonCreate(/*NULL, NULL*/);
#endif
	NewtonSetThreadsCount(nWorld, /*use_threads?*/2/*:1*/);
	/*
    {
        NewtonCollision* collision = NewtonCreateTreeCollision(nWorld, roadID);
        FILE* f = fopen("last_road_points", "r");
        NewtonTreeCollisionBeginBuild(collision);

        bool canRead = true;
        float vArray[9];

        printf("read points\n");
        while (canRead)
        {
            for (int wi = 0; wi < 9 && canRead; wi++)
            {
                int ret = fscanf(f, "%f\n", &vArray[wi]);
                if (ret < 1) canRead = false;
            }
            if (canRead)
            {
                NewtonTreeCollisionAddFace(collision, 3, (float*)vArray, 3 * sizeof(float), 1);
            }
        }
        fclose(f);
        printf("collisionendbuild\n");
        NewtonTreeCollisionEndBuild(collision, 0);
        printf("collisionendbuild done\n");
    }
    */
    // Set up default material properties for newton
    SetupMaterials(nWorld, soundEngine);
	
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
	        core::rect<int>(10,10,210,26), true, true, 0, -1, true);
    }

    fpsText = env->addStaticText(L"FPS: ",
                        core::rect<int>(10,30,80,46),
                        info_bg, true, 0, -1, info_bg);
    fpsText->setVisible(false);
    polyText = env->addStaticText(L"POLYS: ",         // 110
#ifdef USE_EDITOR
                        core::rect<int>(10,50,420,66),
#else
                        core::rect<int>(10,50,140,66),
#endif
                        info_bg, true, 0, -1, info_bg);
    polyText->setVisible(false);

#ifdef USE_EDITOR
    posText = env->addStaticText(L"POS: ",
                        core::rect<int>(10,70,420,86),
                        info_bg, true, 0, -1, info_bg);
    posText->setVisible(false);
#endif // USE_EDITOR
    {
        core::stringw str = L" Build ";
        str += VER_STRING;
        versionText = env->addStaticText(str.c_str(),
                            core::rect<int>(screenSize.Width - 140, screenSize.Height - 20,screenSize.Width - 4, screenSize.Height - 4),
                            false, true, 0, -1, true);
        versionText->setVisible(false);
    }
    
#ifdef USE_EDITOR
    if (editorMode)
    {
        initEditor(env);
    }
#endif // USE_EDITOR

    const int fontH = 25;
    const int fontO = 10;

    ITexture* hudInfoTexture = driver->getTexture("data/hud/info_bg.png");
	hudInfo = env->addImage(core::rect<int>(7, screenSize.Height-(fontO+fontH*3)-1, 7+hudInfoTexture->getSize().Width, screenSize.Height-(fontO+fontH*3)-1+hudInfoTexture->getSize().Height), 0, -1, L"hudInfo");
	hudInfo->setScaleImage(false);
	hudInfo->setUseAlphaChannel(true);
    hudInfo->setImage(hudInfoTexture);
    hudInfo->setVisible(false);

    demageText = env->addStaticText(L"Damage: ",
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
            break;
        }
    }
	setupFonts(env);
	
	env->getSkin()->setFont(fonts[FONT_NORMALBOLD]);
	env->getSkin()->setFont(fonts[FONT_BUILTIN], EGDF_TOOLTIP);
    SColor guicol(255, 90, 70, 16);
    SColor guicol2(255, 255, 255, 255);
	env->getSkin()->setColor(EGDC_BUTTON_TEXT, guicol);
    demageText->setOverrideFont(fonts[FONT_SPECIAL16]);
    speedText->setOverrideFont(fonts[FONT_SPECIAL16]);
    timeText->setOverrideFont(fonts[FONT_SPECIAL16]);

    demageText->setOverrideColor(guicol2);
    speedText->setOverrideColor(guicol2);
    timeText->setOverrideColor(guicol2);

    MessageText::addText(L"Please wait [            ]", 1, false, false);

    for (u32 i=0; i<EGDC_COUNT ; ++i)
    {
        SColor col = env->getSkin()->getColor((EGUI_DEFAULT_COLOR)i);
        if (col.getAlpha() != 0)
            col.setAlpha(200);
        env->getSkin()->setColor((EGUI_DEFAULT_COLOR)i, col);
    }


    // first draw the wait message
    driver->beginScene(true, true, SColor(0,192,192,192));
    env->drawAll();
    driver->endScene();

    hudTextures[0] = driver->getTexture("data/hud/hud0.png");
    hudTextures[1] = driver->getTexture("data/hud/hud1.png");
    hudTextures[2] = driver->getTexture("data/hud/hud2.png");
    hudTextures[3] = driver->getTexture("data/hud/hud3.png");
    hudTextures[4] = driver->getTexture("data/hud/hud4.png");
    hudTextures[5] = driver->getTexture("data/hud/hud5.png");
    hudTextures[6] = driver->getTexture("data/hud/hud6.png");
    hudTextures[7] = driver->getTexture("data/hud/hudr.png");
    hudTextures[8] = driver->getTexture("data/hud/hudd.png");
    
    hudUserOnMapTexture = driver->getTexture("data/hud/useronmap.png");
    speedPalca.getMaterial().setTexture(0, driver->getTexture("data/hud/palca.png"));
    //speedPalca.getMaterial().MaterialTypeParam = 0.5f;
    speedPalca2.getMaterial().setTexture(0, driver->getTexture("data/hud/palca_e.png"));
    //speedPalca2.getMaterial().MaterialTypeParam = 0.5f;
    speedHud.getMaterial().setTexture(0, hudTextures[1]);
    //speedHud.getMaterial().MaterialTypeParam = 0.5f;

    const float hudRate = (float)(screenSize.Width)/2600.f;
    float hudSize;
    if (hudTextures[1])
        hudSize = (float)hudTextures[1]->getSize().Width * hudRate;
    else
        hudSize = 512.f * hudRate;
    const float hudCenter1Size = 255.f * hudRate;
    const float hudPos1X = (255.f-120.f) * hudRate;
    const float hudPos1Y = (255.f-390.f) * hudRate;
    const float hudCenter2Size = 440.f * hudRate;
    const float hudPos2X = (255.f-181.f) * hudRate;
    const float hudPos2Y = (440.f-397.f) * hudRate;
    const int hudPositionX = screenSize.Width-(int)hudSize-10;
    const int hudPositionY = screenSize.Height-(int)hudSize-10;
	hudImage = env->addImage(core::rect<int>(hudPositionX, hudPositionY, screenSize.Width-10, screenSize.Height-10), 0, -1, L"hud");

    speedHud.set2DVertexPos(0, irr::core::position2d<s32>(hudPositionX, screenSize.Height-10), screenSize);
    speedHud.set2DVertexPos(1, irr::core::position2d<s32>(hudPositionX, hudPositionY), screenSize);
    speedHud.set2DVertexPos(2, irr::core::position2d<s32>(screenSize.Width-10, hudPositionY), screenSize);
    speedHud.set2DVertexPos(3, irr::core::position2d<s32>(screenSize.Width-10, screenSize.Height-10), screenSize);

	hudImage->setScaleImage(true);
	hudImage->setUseAlphaChannel(true);
    hudImage->setImage(hudTextures[1]);
    dprintf(printf("1 %p\n", hudImage));
    hudImage->setVisible(false);
    
    ItinerHud::init(device, env, screenSize, (int)hudSize);

    // FPS cross at the middle of the screen
    ITexture* crossTexture = driver->getTexture("data/hud/cross.png");
	crossImage = env->addImage(core::rect<int>(screenSize.Width/2-10, screenSize.Height/2-10,
                                               screenSize.Width/2+10, screenSize.Height/2+10),
                               0, -1, L"cross");
	crossImage->setScaleImage(false);
	crossImage->setUseAlphaChannel(true);
    crossImage->setImage(crossTexture);
    crossImage->setVisible(false);


    // helper variables for the hud pointers
    const core::vector2df hudCenter1((float)hudPositionX+hudCenter1Size, (float)hudPositionY+hudCenter1Size);
    const core::position2d<s32> hudCenter2d1((s32)hudCenter1.X,(s32)hudCenter1.Y);
    const core::vector2df hudPos1(hudCenter1.X-hudPos1X, hudCenter1.Y-hudPos1Y);

    const core::vector2df hudCenter2((float)hudPositionX+hudCenter1Size, (float)hudPositionY+hudCenter2Size);
    const core::position2d<s32> hudCenter2d2((s32)hudCenter2.X,(s32)hudCenter2.Y);
    const core::vector2df hudPos2(hudCenter2.X-hudPos2X, hudCenter2.Y-hudPos2Y);

    core::vector2df hudPos;
    core::vector2df hudPos_;
    core::vector2df hudPos_h;
    core::vector2df hudPos_v;
    float hudPos_v_tmp;
    core::position2d<s32> hudPos2d;
    core::position2d<s32> hudPos2d2;
    core::position2d<s32> hudPos2d3;
    core::position2d<s32> hudPos2d4;

    MessageText::addText(L"Please wait [*           ]", 1, true, false);
    
    offsetManager = new OffsetManager();

    // add camera
    fix_camera = smgr->addCameraSceneNode();
    fix_camera->setFarValue(DEFAULT_FAR_VALUE);
    fix_camera->setNearValue(nearValue);

    fix_cameraOffsetObject = new OffsetObject(fix_camera, true);

    fps_camera = smgr->addCameraSceneNodeFPS(0, 100.f, CAMERA_SPEED);
    fps_camera->setFarValue(DEFAULT_FAR_VALUE);
    fps_camera->setNearValue(nearValue);
    
    fps_cameraOffsetObject = new OffsetObject(fps_camera, true);

    lightCam = smgr->addCameraSceneNode();
    lightCamCar = smgr->addCameraSceneNode(); //lightCamCar->setFOV(0.02f);

    car_selector_camera = smgr->addCameraSceneNode();
    car_selector_camera->setFarValue(DEFAULT_FAR_VALUE);
    car_selector_camera->setNearValue(nearValue);

    camera = fix_camera;
    smgr->setActiveCamera(camera);

    device->getCursorControl()->setPosition(0.f,0.f);
    
    // set amb light
    smgr->setShadowColor();
    float lightColor = 1.0f;
    lnode = smgr->addLightSceneNode(0,
            core::vector3df(3750.f,20000.f,3750.f),
            video::SColorf(lightColor, lightColor, lightColor), 50000.f);
    if (useShaders && !useCgShaders) useCgShaders = true;
    if (useShaders)
    {
        lnode_4_shaders = smgr->addLightSceneNode(0,
            core::vector3df(2.f,150.f,20.f),
            video::SColorf(lightColor, lightColor, lightColor), 50000.f);
        lnode_4_shaders->getLightData().Type=ELT_DIRECTIONAL; 
        lnode_4_shaders->setRotation(core::vector3df(110.f, 0.f, 0.f));
        lnode_4_shaders->setVisible(false);
    }
    float lightStrength = 0.6f;
    float lightStrengthS = 0.6f;
    lnode->getLightData().AmbientColor = video::SColorf(lightStrength,lightStrength,lightStrength);
    lnode->getLightData().SpecularColor = video::SColorf(lightStrengthS,lightStrengthS,lightStrengthS);

    dprintf(printf("T&L:                  %d\n", driver->queryFeature(video::EVDF_HARDWARE_TL)));
    dprintf(printf("Multitexturing:       %d\n", driver->queryFeature(video::EVDF_MULTITEXTURE)));
    dprintf(printf("Stencil buffer:       %d\n", driver->queryFeature(video::EVDF_STENCIL_BUFFER)));
    dprintf(printf("Vertex shader 1.1:    %d\n", driver->queryFeature(video::EVDF_VERTEX_SHADER_1_1)));
    dprintf(printf("Vertex shader 2.0:    %d\n", driver->queryFeature(video::EVDF_VERTEX_SHADER_2_0)));
    dprintf(printf("Vertex shader 3.0:    %d\n", driver->queryFeature(video::EVDF_VERTEX_SHADER_3_0)));
    dprintf(printf("Pixel shader 1.1:     %d\n", driver->queryFeature(video::EVDF_PIXEL_SHADER_1_1)));
    dprintf(printf("Pixel shader 1.2:     %d\n", driver->queryFeature(video::EVDF_PIXEL_SHADER_1_2)));
    dprintf(printf("Pixel shader 1.3:     %d\n", driver->queryFeature(video::EVDF_PIXEL_SHADER_1_3)));
    dprintf(printf("Pixel shader 1.4:     %d\n", driver->queryFeature(video::EVDF_PIXEL_SHADER_1_4)));
    dprintf(printf("Pixel shader 2.0:     %d\n", driver->queryFeature(video::EVDF_PIXEL_SHADER_2_0)));
    dprintf(printf("Pixel shader 3.0:     %d\n", driver->queryFeature(video::EVDF_PIXEL_SHADER_3_0)));
    dprintf(printf("ARB vertex program:   %d\n", driver->queryFeature(video::EVDF_ARB_VERTEX_PROGRAM_1)));
    dprintf(printf("ARB fragment program: %d\n", driver->queryFeature(video::EVDF_ARB_FRAGMENT_PROGRAM_1)));
    dprintf(printf("GLSL:                 %d\n", driver->queryFeature(video::EVDF_ARB_GLSL)));
    dprintf(printf("HLSL:                 %d\n", driver->queryFeature(video::EVDF_HLSL)));
    dprintf(printf("Geometry shader:      %d\n", driver->queryFeature(video::EVDF_GEOMETRY_SHADER)));
    dprintf(printf("RTT:                  %d\n", driver->queryFeature(video::EVDF_RENDER_TO_TARGET)));
    dprintf(printf("MRT:                  %d\n", driver->queryFeature(video::EVDF_MULTIPLE_RENDER_TARGETS)));
    dprintf(printf("Non square textures:  %d\n", driver->queryFeature(video::EVDF_TEXTURE_NSQUARE)));
    dprintf(printf("Non POT:              %d\n", driver->queryFeature(video::EVDF_TEXTURE_NPOT)));

    printf("T&L:                  %d\n", driver->queryFeature(video::EVDF_HARDWARE_TL));
    printf("Multitexturing:       %d\n", driver->queryFeature(video::EVDF_MULTITEXTURE));
    printf("Stencil buffer:       %d\n", driver->queryFeature(video::EVDF_STENCIL_BUFFER));
    printf("Vertex shader 1.1:    %d\n", driver->queryFeature(video::EVDF_VERTEX_SHADER_1_1));
    printf("Vertex shader 2.0:    %d\n", driver->queryFeature(video::EVDF_VERTEX_SHADER_2_0));
    printf("Vertex shader 3.0:    %d\n", driver->queryFeature(video::EVDF_VERTEX_SHADER_3_0));
    printf("Pixel shader 1.1:     %d\n", driver->queryFeature(video::EVDF_PIXEL_SHADER_1_1));
    printf("Pixel shader 1.2:     %d\n", driver->queryFeature(video::EVDF_PIXEL_SHADER_1_2));
    printf("Pixel shader 1.3:     %d\n", driver->queryFeature(video::EVDF_PIXEL_SHADER_1_3));
    printf("Pixel shader 1.4:     %d\n", driver->queryFeature(video::EVDF_PIXEL_SHADER_1_4));
    printf("Pixel shader 2.0:     %d\n", driver->queryFeature(video::EVDF_PIXEL_SHADER_2_0));
    printf("Pixel shader 3.0:     %d\n", driver->queryFeature(video::EVDF_PIXEL_SHADER_3_0));
    printf("ARB vertex program:   %d\n", driver->queryFeature(video::EVDF_ARB_VERTEX_PROGRAM_1));
    printf("ARB fragment program: %d\n", driver->queryFeature(video::EVDF_ARB_FRAGMENT_PROGRAM_1));
    printf("GLSL:                 %d\n", driver->queryFeature(video::EVDF_ARB_GLSL));
    printf("HLSL:                 %d\n", driver->queryFeature(video::EVDF_HLSL));
    printf("Geometry shader:      %d\n", driver->queryFeature(video::EVDF_GEOMETRY_SHADER));
    printf("RTT:                  %d\n", driver->queryFeature(video::EVDF_RENDER_TO_TARGET));
    printf("MRT:                  %d\n", driver->queryFeature(video::EVDF_MULTIPLE_RENDER_TARGETS));
    printf("Non square textures:  %d\n", driver->queryFeature(video::EVDF_TEXTURE_NSQUARE));
    printf("Non POT:              %d\n", driver->queryFeature(video::EVDF_TEXTURE_NPOT));
    //assert(0);
    dprintf(printf("2 %p %d\n", hudImage, useCgShaders));
    try
    {
        setupShaders2(device, driver, driverType, smgr, camera, lnode_4_shaders);
    } catch(...)
    {
        printf("Cg shader setup casued exception, fall back to standard shaders\n");
        myError(3, "Cannot initialize shaders! Maybe your hardware does not support it.");
        return 1;
    }
    //myError(0, "hello");
    //assert(0); // for shader debug
    
    if (!useCgShaders)
    {
        shadowMap = shadowMapGame = shadowMapMenu = shadowMapCar = 0;
    }
    recreateRTTs(driver);
    
    dprintf(printf("2b %p\n", hudImage));

    driver->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS, false);

    // create skybox and skydome
    skydome=smgr->addSkyDomeSceneNode(driver->getTexture("data/skys/skydome.jpg"),
             16,8,0.95f,2.0f);
    if (useShaders && useCgShaders)
    {
        skydome->setMaterialType((video::E_MATERIAL_TYPE)myMaterialType_sky);
        skydome->setMaterialTexture(1, driver->getTexture("data/skys/skystars.jpg"));
    }
    if (useShaders)
        skydome->setMaterialFlag(video::EMF_LIGHTING, false);
    else    
        skydome->setMaterialFlag(video::EMF_LIGHTING, globalLight);
        
    sunSphere = smgr->addBillboardSceneNode(0, core::dimension2d<f32>(80.f, 80.f));//smgr->addSphereSceneNode(1.f);
    sunSphere1 = smgr->addBillboardSceneNode(0, core::dimension2d<f32>(70.f, 70.f));//smgr->addSphereSceneNode(1.f);
    sunSphere2 = smgr->addBillboardSceneNode(0, core::dimension2d<f32>(80.f, 80.f));//smgr->addSphereSceneNode(1.f);
    sunSphere3 = smgr->addBillboardSceneNode(0, core::dimension2d<f32>(50.f, 50.f));//smgr->addSphereSceneNode(1.f);
    sunSphere4 = smgr->addBillboardSceneNode(0, core::dimension2d<f32>(50.f, 50.f));//smgr->addSphereSceneNode(1.f);
    //sunSphere->setScale(vector3df(300.f, 300.f, 300.f));
    sunSphere->setVisible(false);
    //sunSphere->setMaterialTexture(0, driver->getTexture("data/posteffects/sun.png"));
    sunSphere->setMaterialTexture(0, driver->getTexture("data/posteffects/flare0.png"));
    sunSphere1->setVisible(false);
    sunSphere1->setMaterialTexture(0, driver->getTexture("data/posteffects/flare1.png"));
    sunSphere2->setVisible(false);
    sunSphere2->setMaterialTexture(0, driver->getTexture("data/posteffects/flare2.png"));
    sunSphere3->setVisible(false);
    sunSphere3->setMaterialTexture(0, driver->getTexture("data/posteffects/flare3.png"));
    sunSphere4->setVisible(false);
    sunSphere4->setMaterialTexture(0, driver->getTexture("data/posteffects/flare4.png"));
    if (useAdvCgShaders)
    {
        sunSphere->setMaterialType((video::E_MATERIAL_TYPE)myMaterialType_sun);
        sunSphere1->setMaterialType((video::E_MATERIAL_TYPE)myMaterialType_sun);
        sunSphere2->setMaterialType((video::E_MATERIAL_TYPE)myMaterialType_sun);
        sunSphere3->setMaterialType((video::E_MATERIAL_TYPE)myMaterialType_sun);
        sunSphere4->setMaterialType((video::E_MATERIAL_TYPE)myMaterialType_sun);
    }
    else
    {
        sunSphere->setMaterialType((video::E_MATERIAL_TYPE)myMaterialType_depthSun);
    }
    
    smokeTexture = driver->getTexture("data/bigterrains/smoke/dirt.png");    
    smokeWaterTexture = driver->getTexture("data/bigterrains/smoke/dirt_water.png");    

    MessageText::addText(L"Please wait [ *          ]", 1, true, false);

    for (int i = 0; i < MAX_CAR_DIRT; i++)
        car_dirttexture_array[i] = driver->getTexture(car_dirt_array[i]);

    MessageText::addText(L"Please wait [  *         ]", 1, true, false);

    driver->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS, true);

    //NewtonSetBodyLeaveWorldEvent (nWorld, BodyLeaveWorld); 

    loadGameplay("data/gameplay/gameplay.txt",
                 device,
                 driver,
                 smgr,
                 env,
                 nWorld,
                 soundEngine);

    MessageText::addText(L"Please wait [   *        ]", 1, true, false);

    loadObjectTypes("data/objects/object_types.txt", smgr, driver, nWorld);
    MessageText::addText(L"Please wait [      *     ]", 1, true, false);
    loadGrassTypes("data/objects/grass_types.txt", smgr, driver, nWorld);
    CRoadType::loadRoadTypes("data/roads/road_types.txt", driver);
    MessageText::addText(L"Please wait [        *   ]", 1, true, false);
    loadTreeTypes("data/objects/tree_types.txt", smgr, driver, device, nWorld);
    loadMyTreeTypes("data/objects/my_tree_types.txt", smgr, driver, device, nWorld);
    MessageText::addText(L"Please wait [         *  ]", 1, true, false);
#ifdef MY_DEBUG
    printPoolStat();
#endif
    vehiclePool = new CVehiclePool(device, smgr, driver, nWorld, soundEngine, "data/vehicles/vehicle_list.txt");
    MessageText::addText(L"Please wait [          * ]", 1, true, false);
    loadItinerTypes("data/itiner/itiner_types.txt", smgr, driver, nWorld);
    loadCompetitors("data/competitors/car.txt");
    playerCompetitor = new SCompetitor(444, player_name, "", "-", team_name,
                                       0, 100, false);

    MessageText::addText(L"Please wait [           *]", 1, true, false);
    
    mapReader = new CMapReaderThread(device);

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

    lasttick = device->getTimer()->getTime();

    if (!restoreState())
    {
        dprintf(printf("DEBUG: there is not state to be restore, start_with_main: %u\n", start_with_mainmenu);)
        if (start_with_mainmenu)
        {
            menu_receiver->openMainWindow();
        }
        else
        {
            dprintf(printf("DEBUG: try load game\n");)
            if (!loadGame(SAVE_FILE))
                currentStage = 0;
            dprintf(printf("DEBUG: start stage: %u\n", currentStage);)
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
    vector3df dynCamPos;
    float dynCamDist;
    core::vector3df nearestCP;
    unsigned int drawnObjs;
    unsigned int notDrawnObjs;
    unsigned int shadowObjs;
    
    u32 drawTick = 0;
    newtonUpdateCount = 0;
    u32 newtonUpdateCount_last = 0;
    s32 sleepTime = 0;
#define NEWTONUPDATECOUNTCHANGE_LIMIT 10
    s32 fasterDraw = NEWTONUPDATECOUNTCHANGE_LIMIT;
    int failed_render = 0;
    
    const int min_fps = 60;
    const int ms_step = 1000 / min_fps;
    const float sec_step = 1 / (float)min_fps;
    //NewtonSetMinimumFrameRate(nWorld, (float)min_fps);
    
    dprintf(printf("device %p\n", device));
    MessageText::hide();
    while(device->run())
    { 
        if (device->isWindowActive()
#ifdef USE_EDITOR
            || (followCarlos && !editorMode)
#endif
        )
        {
            tick = device->getTimer()->getTime();
            if (!fps_compensation)
            {
                sleepTime = 0;
                newtonUpdateCount = (tick - lasttick) / ms_step;
            }
            pdprintf(printf("1\n"));
            if (car && bigTerrain && inGame == 0)
            {
                pdprintf(printf("1b %f %f\n", camera->getPosition().X, camera->getPosition().Z));
                offsetManager->update(offsetManager->getOffset()+camera->getPosition());
                pdprintf(printf("1c\n"));
            }
            if (!fpsCam && car && inGame == 0)
            {
                vector3df campos;
                vector3df camtar;
                vector3df centar;
                vector3df camup = vector3df(0.f, 1.f, 0.f);
                vector3df carrot;
                
                // up_with_matrix: matrix4 camup_m;
                // up_with_matrix: matrix4 carrot_m;
                // up_with_matrix: camup_m[13] = 1.f;
#ifdef USE_EDITOR
                bool useCarlosView = false;
                int carlosNum = 1;
#endif
                
                viewpos_cur = car->getViewPos(view_num+view_mask);
                viewdest_cur = car->getViewDest(view_num+view_mask);
                
#ifdef USE_EDITOR
                if (followCarlos && raceEngine)
                {
                    if (raceEngine->getStarters()[carlosNum]->competitor == playerCompetitor) carlosNum--;
                    if (raceEngine->getStarters()[carlosNum]->vehicle)
                    {
                        useCarlosView = true;
                        dprintf(printf("s: %f, t: %f, d: %d\n", raceEngine->getStarters()[carlosNum]->vehicle->getSpeed()*1.6f, raceEngine->getStarters()[carlosNum]->vehicle->getTorqueReal(), (int)(raceEngine->getStarters()[carlosNum]->vehicle->getDemagePer()));)
                    }
                }
#endif // USE_EDITOR                
                //vector3df carpos = car->getMatrix().getTranslation();
                matrix4 tcentar;
                tcentar.setTranslation(vector3df(viewdest_cur[12], viewpos_cur[13] , viewdest_cur[14]));
#ifdef USE_EDITOR
                if (!useCarlosView)
                {
#endif // USE_EDITOR
                    campos = (car->getMatrix() * viewpos_cur).getTranslation();
                    camtar = (car->getMatrix() * viewdest_cur).getTranslation();
                    centar = (car->getMatrix() * tcentar).getTranslation();
                    carrot = car->getMatrix().getRotationDegrees();
                    // up_with_matrix: carrot_m = car->getMatrix();
#ifdef USE_EDITOR
                }
                else
                {
                    campos = (raceEngine->getStarters()[carlosNum]->vehicle->getMatrix() * viewpos_cur).getTranslation();
                    camtar = (raceEngine->getStarters()[carlosNum]->vehicle->getMatrix() * viewdest_cur).getTranslation();
                    centar = (raceEngine->getStarters()[carlosNum]->vehicle->getMatrix() * tcentar).getTranslation();
                    carrot = raceEngine->getStarters()[carlosNum]->vehicle->getMatrix().getRotationDegrees();
                    // up_with_matrix: carrot_m = raceEngine->getStarters()[carlosNum]->vehicle->getMatrix();
                }
#endif // USE_EDITOR
                
                camera->setTarget(camtar);
                if (view_num==0)
                {
                    camera->setUpVector(camup);
                }
                else
                {
                    // up_with_matrix: carrot_m[12] = carrot_m[13] = carrot_m[14] = 0.f;
                    // up_with_matrix: matrix4 up_m = carrot_m * camup_m;
                    // up_with_matrix: camera->setUpVector(up_m.getTranslation());
                    camera->setUpVector(carrot.rotationToDirection(camup));
                }
                if ((view_num+view_mask)!=0 || !useDynCam || dynCamReset)
                {
                    //campos = car->getMatrix() * viewpos_cur;
                    ///dynCamPos = campos;
                    ///camera->setPosition(dynCamPos);
                    camera->setPosition(campos);
                    dynCamDist = fabsf((centar - campos).getLength());
                    ///dynCamDist = fabsf((centar - dynCamPos).getLength());
                    //dynCamPos = carpos;
                    dynCamReset = false;
                }
                else
                {
                    ///vector3df dir = (dynCamPos - centar).normalize();
                    vector3df dir = (camera->getPosition() - centar).normalize();
                   
                    float fact = fabsf(car->getSpeed())*0.02f;
                   
                    dir = dir * (dynCamDist + fact * fact) ;
                   
                    ///dynCamPos = centar + dir;
                    ///camera->setPosition(dynCamPos);
                    if (dir.Y < 0.05f) dir.Y = 0.05f;
                    camera->setPosition(centar + dir);
                }
                pdprintf(printf("1d %f %f\n", camera->getPosition().X, camera->getPosition().Z));
            }
            pdprintf(printf("2\n"));
            if (!driver->beginScene(true, true, SColor(0,192,192,192)))
            {
                printf("beginScene failed (Irrlicht bug). Program will be quit.\n");
                reinitialize = true;
                //env->drawAll();
                driver->endScene();
                driver->setMaterial(video::SMaterial());
                recreateRTTs(driver);
                /*
                int rtt_count = 0;
                
                for (int i = 0; i< driver->getTextureCount(); i++)
                {
                    if (driver->getTextureByIndex(i)->isRenderTarget())
                    {
                        rtt_count++;
                    }
                }
                
                printf("rtt_count: %d driver->check %d failed renders %d\n", rtt_count, driver->checkDriverReset(), failed_render);
                */
                if (quitGame) break;
                failed_render++;
                if (failed_render < 5)
                {
                    continue;
                }
                saveState();
                break;
            }     
            pdprintf(printf("3\n"));

            pdprintf(printf("4\n"));
            if (car && inGame == 0 && useShaders)
            {
                if (car->getLight())
                {
                    cLightPos = car->getMatrix() * cLightPos_loc;
                    cLightTar = car->getMatrix() * cLightDest_loc;
                    cLightDir = cLightTar - cLightPos;
                }
#ifdef USE_EDITOR
                if (editorMode)
#endif
                    updateEffects(tick);
            }
            pdprintf(printf("5\n"));

            lookDir = (camera->getTarget() - camera->getPosition()).normalize();
            lookPos = camera->getPosition();

            notDrawnObjs = drawnObjs = shadowObjs = 0;

            for (int i = 0; i<objectNodes.size();i++)
            {
                vector3df objDir = (objectNodes[i]->getPosition() - lookPos).normalize();
                float dist = camera->getPosition().getDistanceFrom(objectNodes[i]->getPosition());
                if(dist < 70.f || (lookDir.dotProduct(objDir)>lookFOV && (/*useCgShaders ||*/ dist < objectVisibilityLimit)))
                {
                    objectNodes[i]->setVisible(true);
                    drawnObjs++;
                }
                else
                {
                    objectNodes[i]->setVisible(false);
                    notDrawnObjs++;
                }
            }
            pdprintf(printf("6\n"));

            // shadow mapping
            if (shadows && shadowMapGame)
            {
                pdprintf(printf("6a\n"));
                lightCam->setPosition(camera->getPosition() + lnode_4_shaders->getPosition());
                //lightCam->setPosition(vector3df(camera->getPosition().X+2.0f,
                //                      camera->getPosition().Y + 150.f,
                //                      camera->getPosition().Z + 2.0f));
                pdprintf(printf("6b\n"));
                lightCam->setTarget(camera->getPosition());
                pdprintf(printf("6c\n"));
                lightCam->setFarValue(DEFAULT_FAR_VALUE);
                pdprintf(printf("6d\n"));
                smgr->setActiveCamera(lightCam);

                pdprintf(printf("6e\n"));
                lightCam->OnAnimate(tick);
                pdprintf(printf("6f\n"));
                lightCam->OnRegisterSceneNode();
                pdprintf(printf("6g\n"));
                lightCam->render();
                pdprintf(printf("6h\n"));
                driver->setTransform(ETS_VIEW, lightCam->getViewMatrix());
                driver->setTransform(ETS_PROJECTION, lightCam->getProjectionMatrix());

                pdprintf(printf("6i\n"));
    			driver->setRenderTarget(shadowMapGame, true, true, video::SColor(0, 0, 0, 0));
                
                pdprintf(printf("6j\n"));
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
                        pdprintf(printf("6k\n"));
                        shadowNodes[i]->OnAnimate(tick);
                        pdprintf(printf("6l\n"));
                        shadowNodes[i]->OnRegisterSceneNode();
                        pdprintf(printf("6m cardraw: %u, shadowNodes[i]: %p, i: %u/%u\n", cardraw, shadowNodes[i], i, shadowNodes.size()));
                        shadowNodes[i]->render();
                        pdprintf(printf("6n\n"));
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
                pdprintf(printf("6o\n"));
                smgr->setActiveCamera(camera);
                pdprintf(printf("6p\n"));
            }

            pdprintf(printf("8\n"));
            ((eventreceiver*)device->getEventReceiver())->prerender();
            pdprintf(printf("8b\n"));
            const int usedScreenRTT = 0; // currentScreenRTT / 2;
            const int bloomScreenRTT = 1; // currentScreenRTT / 2;
            const int finalScreenRTT = 2; // currentScreenRTT / 2;

            // old way depth
            if (depth_effect && useCgShaders && !useAdvCgShaders)
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
                        if (depthNodes[i]->isVisible() &&
                            car && (car->getSceneNode()==depthNodes[i] ||
                            car->getTyreSceneNode(0)==depthNodes[i] ||
                            car->getTyreSceneNode(1)==depthNodes[i] ||
                            car->getTyreSceneNode(2)==depthNodes[i] ||
                            car->getTyreSceneNode(3)==depthNodes[i])
                            )
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
            pdprintf(printf("10 useScreenRTT: %u, depth_effect: %u\n", useScreenRTT, depth_effect));

            // post effects, or not
            if (useScreenRTT && depth_effect && useCgShaders)
            {
                pdprintf(printf("10b\n"));
                if (useAdvCgShaders)
                {
                    sunSphere->setPosition(camera->getPosition()+lnode_4_shaders->getPosition());
                    sunSphere1->setPosition(camera->getPosition()+lnode_4_shaders->getPosition()*0.9);
                    sunSphere2->setPosition(camera->getPosition()+lnode_4_shaders->getPosition()*0.8);
                    sunSphere3->setPosition(camera->getPosition()+lnode_4_shaders->getPosition()*0.7);
                    sunSphere4->setPosition(camera->getPosition()+lnode_4_shaders->getPosition()*0.6);
                    driver->setRenderTarget(mrtList, true, true, video::SColor(0, 255, 255, 0));
                }
                else
                {
                    driver->setRenderTarget(screenRTT[usedScreenRTT], true, true, video::SColor(0, 255, 255, 0));
                }
                pdprintf(printf("10c\n"));
                smgr->drawAll();
                pdprintf(printf("10d\n"));
#if 1
                if (useAdvCgShaders)
                {
	                driver->setRenderTarget(screenRTT[bloomScreenRTT], true, true, video::SColor(0, 0, 0, 0));
                    screenQuad.getMaterial().setTexture(0, screenRTT[finalScreenRTT]);
                    screenQuad.getMaterial().MaterialType = (E_MATERIAL_TYPE)myMaterialType_palca;
                    screenQuad.render(driver);
                }
	            driver->setRenderTarget(0, true, true, video::SColor(0, 255, 0, 0));
                screenQuad.getMaterial().setTexture(0, screenRTT[usedScreenRTT]);
                screenQuad.getMaterial().setTexture(1, depthRTT);
                screenQuad.getMaterial().setTexture(2, motiondir_map[view_mask]);
                screenQuad.getMaterial().setTexture(3, screenRTT[/*finalScreenRTT*/bloomScreenRTT]);
                screenQuad.getMaterial().MaterialType = (E_MATERIAL_TYPE)myMaterialType_screenRTT;
                screenQuad.render(driver);
#else
	            driver->setRenderTarget(screenRTT[bloomScreenRTT], true, true, video::SColor(0, 0, 0, 0));
                driver->draw2DImage(screenRTT[usedScreenRTT],
                    core::rect<s32>(0,0,screenRTT[bloomScreenRTT]->getOriginalSize().Width,
                        screenRTT[bloomScreenRTT]->getOriginalSize().Height),
                    core::rect<s32>(0,0,screenRTT[usedScreenRTT]->getOriginalSize().Width,
                        screenRTT[usedScreenRTT]->getOriginalSize().Height),
					0,
                    colors_for_hdr
				);
                screenQuad.getMaterial().setTexture(0, screenRTT[(usedScreenRTT)%MAX_SCREENRTT]);
                screenQuad.getMaterial().setTexture(1, screenRTT[(bloomScreenRTT)%MAX_SCREENRTT]);
                screenQuad.getMaterial().MaterialType = (E_MATERIAL_TYPE)myMaterialType_screenRTT;
	            driver->setRenderTarget(0, true, true, video::SColor(0, 255, 0, 0));
                screenQuad.render(driver);
#endif
            }
            else
            {
                driver->setRenderTarget(0, true, true, video::SColor(0, 0, 0, 255));
                pdprintf(printf("10e\n"));
                smgr->drawAll();
                pdprintf(printf("10f\n"));
            }
            if (draw_hud && car)
            {
                int gear = car->getGear();
                if (gear >= 0 && gear < MAX_HUD)
                {
                    if (car->getSpeed()>=0.f || car->getTorqueReal() > -0.01f)
                    {
                        hudImage->setImage(hudTextures[gear]); // 1 - 6
                        speedHud.getMaterial().setTexture(0, hudTextures[gear]);
                    }
                    else
                    {
                        hudImage->setImage(hudTextures[MAX_HUD]); // R
                        speedHud.getMaterial().setTexture(0, hudTextures[MAX_HUD]);
                    }
                }
                else
                {
                    hudImage->setImage(hudTextures[0]);
                    speedHud.getMaterial().setTexture(0, hudTextures[0]);
                }
            }
            pdprintf(printf("11\n"));
            if (bigTerrain)
            {
                pdprintf(printf("11a\n"));
                nearestCP = bigTerrain->updatePos(offsetManager->getOffset().X+camera->getPosition().X,
                    offsetManager->getOffset().Z+camera->getPosition().Z, density_objects, false);
                pdprintf(printf("11b\n"));
            }
            if (bigTerrain && showMap)
            {
                //const int mapDef = (screenSize.Height>bigTerrain->getMapTexture()->getOriginalSize().Height+40) ? 0 : (bigTerrain->getMapTexture()->getOriginalSize().Height - screenSize.Height + 40);
                const int mapSizeX = (screenSize.Width - 40 > bigTerrain->getMapTexture()->getOriginalSize().Width) ? bigTerrain->getMapTexture()->getOriginalSize().Width : (screenSize.Width - 40);
                const int mapSizeY = (screenSize.Height - 40 > bigTerrain->getMapTexture()->getOriginalSize().Height) ? bigTerrain->getMapTexture()->getOriginalSize().Height : (screenSize.Height - 40);
                const int mapUpX = (screenSize.Width-mapSizeX) / 2;
                const int mapUpY = (screenSize.Height-mapSizeY) / 2;
                const core::position2d<s32> mapPos(mapUpX, mapUpY);
                const core::dimension2di mapSize(mapSizeX, mapSizeY);
                core::position2d<s32> userPosOnMap;
                userPosOnMap.X = (s32)(((offsetManager->getOffset().X+camera->getPosition().X)/TERRAIN_SCALE) * bigTerrain->getMapScaleX());
#ifdef USE_IMAGE_HM
                userPosOnMap.Y = (s32)(((float)bigTerrain->getHeightMap()->getDimension().Height - 1.0f - ((offsetManager->getOffset().Z+camera->getPosition().Z)/TERRAIN_SCALE)) * bigTerrain->getMapScaleY());
#else
                userPosOnMap.Y = (s32)(((float)bigTerrain->getHeightMap()->getYSize() - 1 - ((offsetManager->getOffset().Z+camera->getPosition().Z)/TERRAIN_SCALE)) * bigTerrain->getMapScaleY());
#endif
                userPosOnMap += bigTerrain->getMapUp();
                //printf("map  size: %d, %d\n", bigTerrain->getMapTexture()->getSize().Width, bigTerrain->getMapTexture()->getSize().Height);
                //printf("map osize: %d, %d\n", bigTerrain->getMapTexture()->getOriginalSize().Width, bigTerrain->getMapTexture()->getOriginalSize().Height);
                //printf("useron map: %d, %d\n", userPosOnMap.X, userPosOnMap.Y);
                core::dimension2di drawOffset = userPosOnMap - mapSize / 2;
                core::dimension2di drawOffsetUser = mapSize / 2;
                if (drawOffset.Width < 0)
                {
                    drawOffsetUser.Width += drawOffset.Width;
                    drawOffset.Width = 0;
                }
                if (drawOffset.Height < 0)
                {
                    drawOffsetUser.Height += drawOffset.Height;
                    drawOffset.Height = 0;
                }
                if (drawOffset.Width + mapSizeX > bigTerrain->getMapTexture()->getOriginalSize().Width)
                {
                    drawOffsetUser.Width += drawOffset.Width - (bigTerrain->getMapTexture()->getOriginalSize().Width - mapSizeX);
                    drawOffset.Width = bigTerrain->getMapTexture()->getOriginalSize().Width - mapSizeX;
                }
                if (drawOffset.Height + mapSizeY > bigTerrain->getMapTexture()->getOriginalSize().Height)
                {
                    drawOffsetUser.Height += drawOffset.Height - (bigTerrain->getMapTexture()->getOriginalSize().Height - mapSizeY);
                    drawOffset.Height = bigTerrain->getMapTexture()->getOriginalSize().Height - mapSizeY;
                }
                //printf("drawoffset: %d, %d\n", drawOffset.Width, drawOffset.Height);
                driver->draw2DImage(bigTerrain->getMapTexture(), mapPos, core::rect<s32>(drawOffset.Width, drawOffset.Height, drawOffset.Width+mapSizeX, drawOffset.Height+mapSizeY));
                //printf("drawoffsetuser: %d, %d\n", drawOffsetUser.Width, drawOffsetUser.Height);
                drawOffsetUser.Width -= (hudUserOnMapTexture->getOriginalSize().Width/2);
                drawOffsetUser.Height -= (hudUserOnMapTexture->getOriginalSize().Height/2);
                driver->draw2DImage(hudUserOnMapTexture, mapPos+drawOffsetUser,
                    core::rect<s32>(0,0,hudUserOnMapTexture->getOriginalSize().Width, hudUserOnMapTexture->getOriginalSize().Height),
                    0, SColor(255, 255, 255, 255), true);
            }
            pdprintf(printf("12\n"));
            env->drawAll();
            pdprintf(printf("12b\n"));
            ((eventreceiver*)device->getEventReceiver())->render();
            pdprintf(printf("13\n"));
            if (draw_hud && car && inGame == 0)
            {
                // speed
                f64 speed = (f64)(fabsf(car->getSpeed())*hud_speed_multiplier);
                hudPos = hudPos1;
                hudPos.rotateBy(speed, hudCenter1);
                
                hudPos2d.X = (s32)hudPos.X;
                hudPos2d.Y = (s32)hudPos.Y;
                
                if (useCgShaders)
                {
                    speedHud.getMaterial().MaterialType = (E_MATERIAL_TYPE)myMaterialType_palca;
                    speedHud.render(driver);

                    /* new part */
                    hudPos_ = hudCenter1 + ((hudCenter1 - hudPos) * 0.215f);
                    hudPos2d2.X = (s32)hudPos_.X;
                    hudPos2d2.Y = (s32)hudPos_.Y;
                    
                    hudPos_v = (hudPos_ - hudPos) * 0.5f;
                    hudPos_h = hudPos + hudPos_v;
                    
                    //hudPos_v.rotateBy(90.f, vector2df(0.f, 0.f));
                    hudPos_v_tmp = hudPos_v.X;
                    hudPos_v.X = -hudPos_v.Y;
                    hudPos_v.Y = hudPos_v_tmp;
                    
                    hudPos2d3.X = (s32)(hudPos_h.X+hudPos_v.X);
                    hudPos2d3.Y = (s32)(hudPos_h.Y+hudPos_v.Y);
                    
                    hudPos2d4.X = (s32)(hudPos_h.X-hudPos_v.X);
                    hudPos2d4.Y = (s32)(hudPos_h.Y-hudPos_v.Y);
                    
                    speedPalca.set2DVertexPos(0, hudPos2d4, screenSize);
                    speedPalca.set2DVertexPos(1, hudPos2d, screenSize);
                    speedPalca.set2DVertexPos(2, hudPos2d3, screenSize);
                    speedPalca.set2DVertexPos(3, hudPos2d2, screenSize);
                    speedPalca.getMaterial().MaterialType = (E_MATERIAL_TYPE)myMaterialType_palca;
                    speedPalca.render(driver);
                    /* new part  end*/
                }
                else
                {
                    driver->draw2DLine(hudPos2d, hudCenter2d1,SColor(255,255,0,0));
                }

                // engine rotate
                if (car->getEngineRotate() > 0.f)
                {
                    speed = (f64)(fabsf(car->getEngineRotate())*60.f-30.f);
                }
                else
                {
                    speed = 0.0;
                }
                hudPos = hudPos2;
                hudPos.rotateBy(speed, hudCenter2);
                
                hudPos2d.X = (s32)hudPos.X;
                hudPos2d.Y = (s32)hudPos.Y;

                if (useCgShaders)
                {
                    /* new part */
                    hudPos_ = hudCenter2 + ((hudCenter2 - hudPos) * 0.11f);
                    hudPos2d2.X = (s32)hudPos_.X;
                    hudPos2d2.Y = (s32)hudPos_.Y;
                    
                    hudPos_v = (hudPos_ - hudPos) * 0.5f;
                    hudPos_h = hudPos + hudPos_v;
                    
                    hudPos_v_tmp = hudPos_v.X;
                    hudPos_v.X = -hudPos_v.Y;
                    hudPos_v.Y = hudPos_v_tmp;
                    
                    hudPos2d3.X = (s32)(hudPos_h.X+hudPos_v.X);
                    hudPos2d3.Y = (s32)(hudPos_h.Y+hudPos_v.Y);
                    
                    hudPos2d4.X = (s32)(hudPos_h.X-hudPos_v.X);
                    hudPos2d4.Y = (s32)(hudPos_h.Y-hudPos_v.Y);
                    
                    speedPalca2.set2DVertexPos(0, hudPos2d4, screenSize);
                    speedPalca2.set2DVertexPos(1, hudPos2d, screenSize);
                    speedPalca2.set2DVertexPos(2, hudPos2d3, screenSize);
                    speedPalca2.set2DVertexPos(3, hudPos2d2, screenSize);
                    speedPalca2.getMaterial().MaterialType = (E_MATERIAL_TYPE)myMaterialType_palca;
                    speedPalca2.render(driver);
                    /* new part  end*/
                }
                else
                {
                    driver->draw2DLine(hudPos2d, hudCenter2d2,SColor(255,255,0,0));
                }
            }
            driver->endScene();
            device->getTimer()->tick();
            drawTick = device->getTimer()->getTime() - tick;
            // with sleep
            if (fps_compensation)
            {
                newtonUpdateCount = (drawTick + ms_step) / ms_step;
                /*
                if (newtonUpdateCount < newtonUpdateCount_last)
                {
                    fasterDraw--;
                    if (fasterDraw)
                    {
                        newtonUpdateCount = newtonUpdateCount_last;
                    }
                    else
                    {
                        fasterDraw = NEWTONUPDATECOUNTCHANGE_LIMIT;
                    }
                }
                else
                {
                    fasterDraw = NEWTONUPDATECOUNTCHANGE_LIMIT;
                }
                */
                //newtonUpdateCount = 2;
                sleepTime = (newtonUpdateCount * ms_step) - drawTick - (newtonUpdateCount/2);
            }
            newtonUpdateCount_last = newtonUpdateCount;
            if (newtonUpdateCount > 10) newtonUpdateCount = 10;
            //printf("dt: %u nuc: %u st: %u\n", drawTick, newtonUpdateCount, sleepTime);
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
#ifdef USE_EDITOR
                        str += L", D: ";
                        str += drawnObjs;
                        str += L", ND: ";
                        str += notDrawnObjs;
                        str += L", SH: ";
                        str += shadowObjs;
#endif // USE_EDITOR
                        polyText->setText(str.c_str());
        
#ifdef USE_EDITOR
                        str = L"POS: ";
                        str += (int)(offsetManager->getOffset().X+camera->getPosition().X);
                        str += ", ";
                        /*
                        str += (int)(offsetManager->getOffset().X);
                        str += "), ";
                        */
                        str += (int)camera->getPosition().Y;
                        str += ", ";
                        str += (int)(offsetManager->getOffset().Z+camera->getPosition().Z);
                        /*
                        str += ", ";
                        str += (int)(offsetManager->getOffset().Z);
                        */
                        str += " (";
                        str += (int)((offsetManager->getOffset().X+camera->getPosition().X)/20.f);
                        str += ", ";
#ifdef USE_IMAGE_HM
                        str += (int)(bigTerrain->getHeightMap()->getDimension().Height - 1 - ((offsetManager->getOffset().Z+camera->getPosition().Z)/TERRAIN_SCALE));
#else
                        str += (int)(bigTerrain->getHeightMap()->getYSize() - 1 - ((offsetManager->getOffset().Z+camera->getPosition().Z)/TERRAIN_SCALE));
#endif
                        str += ")";
                        posText->setText(str.c_str());
#endif // USE_EDITOR
                    }
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
                str = L"Damage: ";
                str += (int)(car->getDemagePer());
                str += "%";
                demageText->setText(str.c_str());
        
                str = L"Speed: ";
                str += (int)(car->getSpeed()*1.6f/*3.0f*/);
                str += " km/h (";
                if (gear_type=='a')
                    str += L"A";
                else
                    str += L"M";
                str += ": ";
                str += (int)(car->getGear());
                str += ")";
                speedText->setText(str.c_str());
                
                u32 diffTime;
                if (bigTerrain->getTimeEnded())
                {
                    diffTime = bigTerrain->getCurrentTime();
                }
                else
                {
                    diffTime = bigTerrain->getCurrentTime() + bigTerrain->getPenality();
                }
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
#ifdef USE_EDITOR
            if (editorMode && car && bigTerrain && inGame == 0)
            {
                pdprintf(printf("14e\n"));
                updateEditor(); // TODO: put back into the one sec. update
            }
#endif // USE_EDITOR
            pdprintf(printf("15\n"));
          
#ifdef USE_MULTIPLAYER
            if (isMultiplayer)
            {
                if (tick > lastMP + send_server_delay)
                {
                    updateConnection();
                    lastMP = tick;
                }
            }
#endif // USE_MULTIPLAYER
          
            if (inGame == 0)
            {
                if (newtonUpdateCount)
                {
                    if (bigTerrain)
                    {
                        bigTerrain->checkMapsQueue();
                        bigTerrain->updateTime(tick);
                    }
                    if (raceEngine)
                    {
                        raceEngine->update(tick, offsetManager->getOffset()+camera->getPosition(), playerCompetitor, device);
                    }
                    while (newtonUpdateCount)
                    {
                        if (car)
                        {
                            float kbs = car->getSteerKb();
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
                                {
                                    car->setSteering(kbs);
                                }
                            }
                          
#ifdef USE_MULTIPLAYER
                            if (isMultiplayer)
                            {
                                updateOtherDatas();
                            }
#endif // USE_MULTIPLAYER
    
                            pdprintf(printf("16b\n"));
                            vehiclePool->updateActiveVehicles();
                        }
                        pdprintf(printf("17\n"));
                        NewtonUpdate(nWorld, 0.015f/*sec_step*//*1.066667f*/);
                        //NewtonUpdate(nWorld, 0.0075f/*sec_step*//*1.066667f*/);
                        //NewtonUpdate(nWorld, sec_step/*1.066667f*/);
                        pdprintf(printf("17c\n"));
                        lasttick += ms_step;
                        newtonUpdateCount--;
                    }
                    pdprintf(printf("18\n"));
                    soundEngine->setListenerPosition(camera->getPosition(), camera->getTarget()-camera->getPosition());
                }
            }
            else // inGame == 0
            {
                lasttick = tick;
            }
            
            if (sleepTime > 1 && sleepTime < 1000)
            {
                device->sleep(sleepTime);
            }
        }
        else // device->isWindowActive()
        {
            if (inGame == 0)
            {
                printf("save state\n");
                saveState();
            }
            menu_receiver->openMainWindow();
            device->sleep(100);
            lasttick = device->getTimer()->getTime();
        }
    } // while (run)

    if (reinitialize)
    {
        printf("save state\n");
        saveState();
    }
    else
    {
        removeState();
    }
    
    driver->setMaterial(video::SMaterial());
    printf("end game\n");
	endGame();
#ifdef USE_MULTIPLAYER
    printf("disconnect from server\n");
	disconnectFromServer(false);
#endif // USE_MULTIPLAYER
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

    printf("offsetmanager clean\n");
    if (fix_cameraOffsetObject)
    {
        delete fix_cameraOffsetObject; // do not remove from manager, because endGame will do that
        fix_cameraOffsetObject = 0;
    }
    if (fps_cameraOffsetObject)
    {
        delete fps_cameraOffsetObject; // do not remove from manager, because endGame will do that
        fps_cameraOffsetObject = 0;
    }
    assert(offsetManager->empty());
    delete offsetManager;
    offsetManager = 0;
    
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
    
    } while (0/*reinitialize*/);

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
        //car->reset(bigTerrain->getStartPos());
        /*
        matrix4 mat = car->getMatrix();
        core::vector3df rot = mat.getRotationDegrees();
        mat.setTranslation(bigTerrain->getStartPos());
        mat.setRotationDegrees(vector3df(0.f, rot.Y, 0.f));
        car->setMatrixWithNB(mat);
        */
    }
}

