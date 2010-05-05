/****************************************************************
*                                                               *
*    Name: eventreceiver_game.cpp                               *
*                                                               *
*    Creator: Balazs Tuska                                      *
*                                                               *
*    Description:                                               *
*       This file contains the event receiver for the game.     *
*                                                               *
****************************************************************/

#include "eventreceiver_game.h"
#include "eventreceiver_menu.h"
#include "irrlicht.h"
#include "settings.h"
#include "message.h"
#include "gameplay.h"
#include "pools.h"
#include "multiplayer.h"
#include "editor.h"

#ifdef __linux__
#include "linux_includes.h"
#endif

// for the joystick 0.05f
#define DEAD_ZONE deadZone

//float joy_accel_p = 0.f;
//float joy_steer = 0.f;
int ablr = 0;
int joy_look_l = 0;
int joy_look_r = 0;
int joy_gu = 0;
int joy_gd = 0;
int joy_reset_car_p =0;
int joy_change_view_p = 0;
int joy_change_light_p = 0;
int joy_show_compass_p = 0;
int joy_repair_car_p = 0;
int joy_menu_p = 0;

void actualizeView()
{
     viewpos_cur = viewpos[view_num+view_mask];
     viewdest_cur = viewdest[view_num+view_mask];
     dynCamReset = true;
/*
    switch (view_mask)
    {
        case 0:
             viewpos_cur = viewpos[view_num];
             viewdest_cur = viewdest[view_num];
             break;
        case 1:
             viewpos_cur = viewpos[view_num+1];
             viewdest_cur = viewdest[view_num+1];
             break;
        case 2:
             viewpos_cur = viewpos[view_num+2];
             viewdest_cur = viewdest[view_num+2];
             break;
        case 3:
             viewpos_cur = viewpos[view_num+3];
             viewdest_cur = viewdest[view_num+3];
             break;
    }
*/
}

void lookLeft(bool set)
{
    if (set)
        view_mask |= 1;
    else
        view_mask &= ~1;
    actualizeView();
}

void lookRight(bool set)
{
    if (set)
        view_mask |= 2;
    else
        view_mask &= ~2;
    actualizeView();
}

void lookCenter()
{
    view_mask = 0;
    actualizeView();
}

void changeView()
{
    if (inGame == 0)
    {
	    view_num = (view_num + view_multi) % (view_max*view_multi);
        viewpos_cur = viewpos[view_num];
        viewdest_cur = viewdest[view_num];
        dynCamReset = true;
    }
}

eventreceiver_game::eventreceiver_game(IrrlichtDevice* pdevice,
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

eventreceiver_game::~eventreceiver_game()
{
    releaseResources();
}

bool eventreceiver_game::OnEvent(const SEvent& event)
{
    //printf("event: %d\n", event.EventType);
	if (event.EventType == irr::EET_KEY_INPUT_EVENT && !event.KeyInput.PressedDown)
	{
       	//scene::ISceneNode* terrain = bigTerrain ? bigTerrain->getTerrain(camera->getPosition().X,
        //                                                    camera->getPosition().Z)
        //                                            : 0;
		switch (event.KeyInput.Key)
		{
/*
		case irr::KEY_F1: // switch wire frame mode
			terrain->setMaterialFlag(video::EMF_WIREFRAME,
					!terrain->getMaterial(0).Wireframe);
			terrain->setMaterialFlag(video::EMF_POINTCLOUD, false);
			return true;
		case irr::KEY_F2: // switch wire frame mode
			terrain->setMaterialFlag(video::EMF_POINTCLOUD,
					!terrain->getMaterial(0).PointCloud);
			terrain->setMaterialFlag(video::EMF_WIREFRAME, false);
			return true;
		case irr::KEY_F3: // toggle detail map
			terrain->setMaterialType(
				terrain->getMaterial(0).MaterialType == video::EMT_SOLID ?
				video::EMT_DETAIL_MAP : video::EMT_SOLID);
			return true;
		case irr::KEY_F4: // toggle skies
			showBox=!showBox;
			Skybox->setVisible(showBox);
			Skydome->setVisible(!showBox);
			return true;
*/
		case irr::KEY_F1:
		    ((eventreceiver_menu*)other)->openHelpWindow();
			return true;
			break;
		//case irr::KEY_F1:
		//    ((eventreceiver_menu*)other)->openStateWindow();
		//	return true;
		//	break;
		case irr::KEY_F2:
		    display_extra_info = !display_extra_info;
            fpsText->setVisible(display_extra_info);
            polyText->setVisible(display_extra_info);
            posText->setVisible(display_extra_info);
            editorSetVisible(display_extra_info);
			return true;
		case irr::KEY_F3:
		    draw_hud = !draw_hud;
            hudImage->setVisible(draw_hud);
			return true;
		case irr::KEY_F4:
		    if (car) car->switchLight();
			return true;
        case irr::KEY_ESCAPE:
            {
                ((eventreceiver_menu*)other)->openMainWindow();
				return true;
                break;
            }
		case irr::KEY_F5: // change view
		    if (inGame == 0)
		    {
                changeView();
                return true;
            }
            return false;
		case irr::KEY_F6: // change view
		    if (inGame == 0)
		    {
                useDynCam = !useDynCam;
                dynCamReset = true;
                return true;
            }
            return false;
		case irr::KEY_F9:
            {
                ((eventreceiver_menu*)other)->openOptionsWindow();
                return true;
                break;
            }
/*
		case irr::KEY_F10:
             if (car)
                car->setControl();
             return true;
*/
        case irr::KEY_TAB:
             if (bigTerrain && !bigTerrain->getTimeEnded())
             {
                showCompass = !showCompass;
                hudCompassImage->setVisible(showCompass);
                compassText->setVisible(showCompass);
                if (showCompass && show_compass_arrow)
                {
                    compassArrow->setVisible(showCompass);
                }
                else
                {
                    compassArrow->setVisible(false);
                }
             }
             return true;
             break;
        case irr::KEY_UP:
             if (fpsCam) return false;
		case irr::KEY_KEY_W:
             ablr &= ~1;
             if (car)
                car->setTireTorque(0);
             return true;
        case irr::KEY_DOWN:
             if (fpsCam) return false;
		case irr::KEY_KEY_S:
             ablr &= ~2;
             if (car)
                car->setTireTorque(0);
             return true;
        case irr::KEY_LEFT:
             if (fpsCam) return false;
		case irr::KEY_KEY_A:
             ablr &= ~4;
             if (car)
             {
                car->setSteeringKb(0);
                car->setSteering(0);
             }
             return true;
        case irr::KEY_RIGHT:
             if (fpsCam) return false;
		case irr::KEY_KEY_D:
             ablr &= ~8;
             if (car)
             {
                car->setSteeringKb(0);
                car->setSteering(0);
             }
             return true;
		case irr::KEY_KEY_R:
             {
                if (car && bigTerrain && inGame == 0)
                {
                    //matrix4 mat = car->getMatrix();
                    //core::vector3df rot = mat.getRotationDegrees();
                    /*
                    mat.setTranslation(core::vector3df(-2000.f,-2000.f,-2000.f));
                    car->setMatrixWithNB(mat);
                    delete car;
                    NewtonInvalidateCache(nWorld);
                    car =
                         new NewtonRaceCar (smgr, driver, carName, nWorld, soundEngine,
                         core::vector3df(camera->getPosition().X,bigTerrain->getHeight(camera->getPosition().X,camera->getPosition().Z)+5.f,camera->getPosition().Z));
                    */
                    //mat.setTranslation(core::vector3df(camera->getPosition().X,bigTerrain->getHeight(camera->getPosition().X,camera->getPosition().Z)+5.f,camera->getPosition().Z));
                    //mat.setRotationDegrees(vector3df(0.f, rot.Y, 0.f));
                    //car->setMatrixWithNB(mat);
                    car->reset(core::vector3df(camera->getPosition().X,bigTerrain->getHeight(camera->getPosition().X,camera->getPosition().Z)+5.f,camera->getPosition().Z));
                    dynCamReset = true;
                    if (bigTerrain->addPenality(RESET_PENALITY)!=(u32)-1)
                    {
                        MessageText::addText(L"Add 2 minutes penality, because of restoring car.", 5);
                    }
                    return true;
                }
                return false;
             }
		case irr::KEY_KEY_T:
             {
                if (car && bigTerrain && inGame == 0)
                {
                    float demage = car->getDemagePer();
                    int penality = (int)(3.f*demage);
                    for (int i = 0; i < 4; i++) 
                    {
                        if (!car->isTyreConnected(i))
                        {
                            penality += 15;
                        }
                    }
                    if (!penality)
                    {
                        MessageText::addText(L"You don't need to repair the car.", 5);
                    }
                    else
                    {
                        car->repair();
                        if (bigTerrain->addPenality(penality)!=(u32)-1)
                        {
                            core::stringw str = L"Add ";
                            str += penality;
                            str += L" seconds penality, because of repairing the car.";
                            MessageText::addText(str.c_str(), 5);
                        }
                    }
                    return true;
                }
                return false;
             }
/*		case irr::KEY_KEY_Z:
             {
                if (car && bigTerrain && inGame == 0)
                {
                    car->loseTyre(0);
                    return true;
                }
                return false;
             }
*/		case irr::KEY_KEY_F:
             {
                if (car && inGame == 0)
                {
                    fpsCam = !fpsCam;
	                camera = fpsCam ? fps_camera : fix_camera;
                    smgr->setActiveCamera(camera);

                    matrix4 campos = car->getMatrix() * viewpos_cur;
                    matrix4 camtar = car->getMatrix() * viewdest_cur;
                    camera->setPosition(core::vector3df(campos[12],campos[13],campos[14]));
                    camera->setTarget(core::vector3df(camtar[12],camtar[13],camtar[14]));
	                camera->setFarValue(/*bigTerrain->getSmallTerrainSize()*FAR_VALUE_MULTI*/DEFAULT_FAR_VALUE);
	                camera->setNearValue(nearValue);
	                crossImage->setVisible(fpsCam);
                    return true;
                }
                return false;
             }
        case irr::KEY_KEY_Q:
             lookLeft(false);
             return true;
        case irr::KEY_KEY_E:
             lookRight(false);
             return true;
		case irr::KEY_SPACE:
             ablr &= ~16;
             if (car)
                 car->setHandBrakes(0.f);
             return true;
		case irr::KEY_KEY_M:
             MessageText::addText(0, 15);
             return true;
		case irr::KEY_KEY_P:
             printPoolStat();
             return true;
		case irr::KEY_KEY_O:
		case irr::KEY_KEY_I:
            {
                if (car)
                {
                    int g = car->getGear();
                    car->updateGear(g+1);
                }
                return true;
                break;
            }
		case irr::KEY_KEY_K:
		case irr::KEY_KEY_L:
            {
                if (car)
                {
                    int g = car->getGear();
                    car->updateGear(g-1);
                }
                return true;
                break;
            }
/*
		case irr::KEY_KEY_1:
            {
                if (car)
                    car->updateGear(1);
                return true;
                break;
            }
		case irr::KEY_KEY_2:
            {
                if (car)
                    car->updateGear(2);
                return true;
                break;
            }
		case irr::KEY_KEY_3:
            {
                if (car)
                    car->updateGear(3);
                return true;
                break;
            }
		case irr::KEY_KEY_4:
            {
                if (car)
                    car->updateGear(4);
                return true;
                break;
            }
		case irr::KEY_KEY_5:
            {
                if (car)
                    car->updateGear(5);
                return true;
                break;
            }
		case irr::KEY_KEY_6:
            {
                if (car)
                    car->updateGear(6);
                return true;
                break;
            }
		case irr::KEY_KEY_0:
            {
                if (car)
                    car->updateGear(0);
                return true;
                break;
            }
*/
#ifdef USE_EDITOR
		case irr::KEY_KEY_0:
		case irr::KEY_KEY_1:
		case irr::KEY_KEY_2:
		case irr::KEY_KEY_3:
		case irr::KEY_KEY_4:
		case irr::KEY_KEY_5:
		case irr::KEY_KEY_6:
		case irr::KEY_KEY_7:
		case irr::KEY_KEY_8:
		case irr::KEY_KEY_9:
		case irr::KEY_KEY_U:
		case irr::KEY_F7:
		case irr::KEY_F8:
		case irr::KEY_INSERT:
		case irr::KEY_DELETE:
		case irr::KEY_HOME:
		case irr::KEY_END:
            actionEditor(event.KeyInput.Key);
            return true;
            break;
#endif // USE_EDITOR
		case irr::KEY_KEY_G:
            {
                if (gear_type=='a')
                    gear_type='m';
                else
                    gear_type='a';
                if (car)
                    car->setAutoGear(gear_type=='a');
                return true;
                break;
            }
		default:
			break;
		}
	} else
	if (event.EventType == irr::EET_KEY_INPUT_EVENT && event.KeyInput.PressedDown)
	{
		switch (event.KeyInput.Key)
		{
        case irr::KEY_UP:
             if (fpsCam) return false;
		case irr::KEY_KEY_W:
             ablr |= 1;
             if (car)
                 car->setTireTorque(1.f);
             return true;
        case irr::KEY_DOWN:
             if (fpsCam) return false;
		case irr::KEY_KEY_S:
             ablr |= 2;
             if (car)
                 car->setTireTorque(-1.f);
             return true;
        case irr::KEY_LEFT:
             if (fpsCam) return false;
		case irr::KEY_KEY_A:
             ablr |= 4;
             if (car)
                 car->setSteeringKb(-1.f);
                 //car->setSteering(-1.f);
             return true;
        case irr::KEY_RIGHT:
             if (fpsCam) return false;
		case irr::KEY_KEY_D:
             ablr |= 8;
             if (car)
                 car->setSteeringKb(1.f);
                 //car->setSteering(1.f);
             return true;
        case irr::KEY_KEY_Q:
             lookLeft(true);
             return true;
        case irr::KEY_KEY_E:
             lookRight(true);
             return true;
		case irr::KEY_SPACE:
             ablr |= 16;
             if (car)
                 car->setHandBrakes(1.f);
             return true;
		default:
			break;
        }
    }

#ifdef USE_EDITOR
    if (event.EventType == irr::EET_MOUSE_INPUT_EVENT)
    {
        if (event.MouseInput.Event == EMIE_LMOUSE_PRESSED_DOWN)
        {
            actionEditor(irr::KEY_KEY_0);
            return true;
        }
        if (event.MouseInput.Event == EMIE_RMOUSE_PRESSED_DOWN)
        {
            actionEditor(irr::KEY_KEY_6);
            return true;
        }
    }
#endif // USE_EDITOR
	// The state of each connected joystick is sent to us
	// once every run() of the Irrlicht device.  Store the
	// state of the first joystick, ignoring other joysticks.
	// This is currently only supported on Windows and Linux.
	if (event.EventType == irr::EET_JOYSTICK_INPUT_EVENT
		&& event.JoystickEvent.Joystick == activeJoystick)
	{
        //printf("joyevent\n");
		JoystickState = event.JoystickEvent;

		f32 moveHorizontal = 0.f; // Range is -1.f for full left to +1.f for full right
		f32 moveVertical = 0.f; // -1.f for full down to +1.f for full up.

		moveHorizontal =
			(f32)JoystickState.Axis[joy_axis_steer] / 32767.f;
		if(fabs(moveHorizontal) < DEAD_ZONE)
			moveHorizontal = 0.f;
		//moveHorizontal *= moveHorizontal * moveHorizontal;
		if (!joy_steer_linear)
		{
			if (moveHorizontal>0)
			 moveHorizontal *= moveHorizontal;
			else
			 moveHorizontal *= moveHorizontal*-1;
        }
        //printf("joyevent %f\n", moveHorizontal);
        
		moveVertical =
			(f32)JoystickState.Axis[joy_axis_accel] / -32767.f;
		if(fabs(moveVertical) < DEAD_ZONE)
			moveVertical = 0.f;

        if (joy_axis_clutch!=-1)
        {
			f32 moveClutch = 0.f;
			moveClutch =
				((f32)JoystickState.Axis[joy_axis_clutch] / -65534.f) + 0.5f;
			if(fabs(moveClutch) < DEAD_ZONE)
				moveClutch = 0.f;
			//else
			//     printf("cl: %f\n", moveClutch);
			
			if (car)
			 car->setClutch(moveClutch);
        }
		// POV hat info is only currently supported on Windows, but the value is
		// guaranteed to be 65535 if it's not supported, so we can check its range.
		const u16 povDegrees = JoystickState.POV / 100;
		if(povDegrees < 360)
		{
			if(povDegrees > 0 && povDegrees < 180)
				moveHorizontal = 1.f;
			else if(povDegrees > 180)
				moveHorizontal = -1.f;

			if(povDegrees > 90 && povDegrees < 270)
				moveVertical = -1.f;
			else if(povDegrees > 270 || povDegrees < 90)
				moveVertical = +1.f;
		}

		//if(!core::equals(moveHorizontal, 0.f) || !core::equals(moveVertical, 0.f))
		//{
		//}
		// check joystick buttons
        /*
        float joy_accel_p = 0.f;
        float joy_steer = 0.f;
        int joy_look = 0;
        int joy_reset_car_p =0;
        int joy_change_view_p = 0;
        */
        //printf("event: %d\n", ablr &3);
// accelerate brake
        if ((ablr & 3) == 0)
        {
			if (joy_accel != -1 && JoystickState.IsButtonPressed(joy_accel))
			{
                if (car)
                    car->setTireTorque(1.f);
            }
            else
            if (joy_brake != -1 && JoystickState.IsButtonPressed(joy_brake))
            {
                if (car)
                    car->setTireTorque(-1.f);
            }
            else
                if (car)
                    car->setTireTorque(moveVertical);
        }
// hand brake
        if ((ablr & 16) == 0)
        {
            if (joy_handbrake != -1 && JoystickState.IsButtonPressed(joy_handbrake))
            {
                 if (car)
                     car->setHandBrakes(1.f);
            }
            else
                 if (car)
                     car->setHandBrakes(0.f);
        }
// turn left/right            
        if ((ablr & 12) == 0)
        {
			if (joy_left != -1 && JoystickState.IsButtonPressed(joy_left))
			{
                if (car)
                    car->setSteering(-1.f);
            }
            else
            if (joy_right != -1 && JoystickState.IsButtonPressed(joy_right))
            {
                if (car)
                    car->setSteering(1.f);
            }
            else
                if (car)
                    car->setSteering(moveHorizontal);
        }

// look left
		if (joy_look_left != -1 && JoystickState.IsButtonPressed(joy_look_left))
		{
            if (joy_look_l==0)
            {
                joy_look_l = 1;
                lookLeft(true);
            }
        }
        else
            if (joy_look_l!=0)
            {
                joy_look_l = 0;
                lookLeft(false);
            }
// look right
        if (joy_look_right != -1 && JoystickState.IsButtonPressed(joy_look_right))
        {
            if (joy_look_r==0)
            {
                joy_look_r = 1;
                lookRight(true);
            }
        }
        else
            if (joy_look_r!=0)
            {
                joy_look_r = 0;
                lookRight(false);
            }
// gear up
		if (joy_gear_up != -1 && JoystickState.IsButtonPressed(joy_gear_up))
		{
            if (joy_gu==0)
            {
                joy_gu = 1;
                if (car)
                    car->updateGear(car->getGear()+1);
            }
        }
        else
            if (joy_gu!=0)
            {
                joy_gu = 0;
            }
// gear down
		if (joy_gear_down != -1 && JoystickState.IsButtonPressed(joy_gear_down))
		{
            if (joy_gd==0)
            {
                joy_gd = 1;
                if (car)
                    car->updateGear(car->getGear()-1);
            }
        }
        else
            if (joy_gd!=0)
            {
                joy_gd = 0;
            }
// manual gear
        if (gear_type != 'a')
        {
			if (joy_gear1 != -1 && JoystickState.IsButtonPressed(joy_gear1))
			{
                if (car)
                    car->updateGear(1, true);
            }
            else
			if (joy_gear2 != -1 && JoystickState.IsButtonPressed(joy_gear2))
			{
                if (car)
                    car->updateGear(2, true);
            }
            else
			if (joy_gear3 != -1 && JoystickState.IsButtonPressed(joy_gear3))
			{
                if (car)
                    car->updateGear(3, true);
            }
            else
			if (joy_gear4 != -1 && JoystickState.IsButtonPressed(joy_gear4))
			{
                if (car)
                    car->updateGear(4, true);
            }
            else
			if (joy_gear5 != -1 && JoystickState.IsButtonPressed(joy_gear5))
			{
                if (car)
                    car->updateGear(5, true);
            }
            else
			if (joy_gear6 != -1 && JoystickState.IsButtonPressed(joy_gear6))
			{
                if (car)
                    car->updateGear(6, true);
            }
            else
                if (car)
                    car->updateGear(0);
        }
// reset car            
        if (joy_reset_car != -1 && JoystickState.IsButtonPressed(joy_reset_car))
        {
            if (joy_reset_car_p==0)
            {
                joy_reset_car_p = 1;
                if (car && bigTerrain && inGame == 0)
                {
                    car->reset(core::vector3df(camera->getPosition().X,bigTerrain->getHeight(camera->getPosition().X,camera->getPosition().Z)+5.f,camera->getPosition().Z));
                    dynCamReset = true;
                    if (bigTerrain->addPenality(RESET_PENALITY)!=(u32)-1)
                    {
                        MessageText::addText(L"Add 2 minutes penality, because of restoring car.", 5);
                    }
                }
            }
        }
        else
            joy_reset_car_p = 0;

// repair car            
        if (joy_repair_car != -1 && JoystickState.IsButtonPressed(joy_repair_car))
        {
            if (joy_repair_car_p==0)
            {
                joy_repair_car_p = 1;
                if (car && bigTerrain && inGame == 0)
                {
                    float demage = car->getDemagePer();
                    int penality = (int)(3.f*demage);
                    for (int i = 0; i < 4; i++) 
                    {
                        if (!car->isTyreConnected(i))
                        {
                            penality += 15;
                        }
                    }
                    if (!penality)
                    {
                        MessageText::addText(L"You don't need to repair the car.", 5);
                    }
                    else
                    {
                        car->repair();
                        if (bigTerrain->addPenality(penality)!=(u32)-1)
                        {
                            core::stringw str = L"Add ";
                            str += penality;
                            str += L" seconds penality, because of repairing the car.";
                            MessageText::addText(str.c_str(), 5);
                        }
                    }
                }
            }
        }
        else
            joy_repair_car_p = 0;

// show compass
        if (joy_show_compass != -1 && JoystickState.IsButtonPressed(joy_show_compass))
        {
            if (joy_show_compass_p==0)
            {
                joy_show_compass_p = 1;
                if (car && bigTerrain && !bigTerrain->getTimeEnded() && inGame == 0)
                {
                    showCompass = !showCompass;
                    hudCompassImage->setVisible(showCompass);
                    compassText->setVisible(showCompass);
                    if (showCompass && show_compass_arrow)
                    {
                        compassArrow->setVisible(showCompass);
                    }
                    else
                    {
                        compassArrow->setVisible(false);
                    }
                }
            }
        }
        else
            joy_show_compass_p = 0;

// change view            
        if (joy_change_view != -1 && JoystickState.IsButtonPressed(joy_change_view))
        {
            if (joy_change_view_p==0)
            {
                joy_change_view_p = 1;
                changeView();
            }
        }
        else
            joy_change_view_p = 0;
// change car light
        if (joy_change_light != -1 && JoystickState.IsButtonPressed(joy_change_light))
        {
            if (joy_change_light_p==0)
            {
                joy_change_light_p = 1;
                if (car)
                    car->switchLight();
            }
        }
        else
            joy_change_light_p = 0;

// quit to main menu
        if (joy_menu != -1 && JoystickState.IsButtonPressed(joy_menu))
        {
            if (joy_menu_p==0)
            {
                joy_menu_p = 1;
                ((eventreceiver_menu*)other)->openMainWindow();
				//return true;
            }
        }
        else
            joy_menu_p = 0;
		
	}
	return false;
}
	
const SEvent::SJoystickEvent& eventreceiver_game::GetJoystickState(void) const
{
	return JoystickState;
}

void eventreceiver_game::releaseResources()
{
}
