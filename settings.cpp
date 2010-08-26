/****************************************************************
*                                                               *
*    Name: settings.cpp                                         *
*                                                               *
*    Creator: Balazs Tuska                                      *
*                                                               *
*    Description:                                               *
*       This file contains the variables of the global settings *
*       like: display, conroller, object, grass density etc.    *
*                                                               *
****************************************************************/

#include <stdio.h>
#include "settings.h"
#include "gameplay.h"

#include "irrlicht.h"

#ifdef __linux__
#include "linux_includes.h"
#endif


using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

float cameraSpeed = 0.04f;
float deadZone = 0.02f;
bool globalLight = true;
bool stencil_shadows = false;
bool shadows = true;
int shadow_map_size = 2048;
bool useShaders = true;
bool useCgShaders = true;
bool useAdvCgShaders = true;
bool use_detailed_terrain = true;
bool use_highres_textures = true;
video::E_DRIVER_TYPE driverType = video::EDT_DIRECT3D9;
bool useSmokes =  true;
float objectVisibilityLimit = 400.f;
float farValue = 2400.f;
float nearValue = 0.5f;
bool full_screen = false;
bool auto_resolution = true;
int resolutionX = 800;
int resolutionY = 600;
bool start_with_mainmenu = true;
int LOD_distance = 17;
int density_objects = 6;
int density_grasses = 10;
int object_pool_size = 100;
int grass_pool_size = 1600;
int object_multiplier = 5;
int grass_multiplier = 5;
int grass_type = 0;
float min_fps = 60.f;
bool display_extra_info = false;
bool info_bg = true;
bool message_bg = true;
int anti_aliasing = 0;
bool vsync = false;
bool high_precision_fpu = false;
int display_bits = 32;
int joy_accel = -1;
int joy_brake = -1;
int joy_handbrake = -1;
int joy_left = -1;
int joy_right = -1;
int joy_look_left = -1;
int joy_look_right = -1;
int joy_reset_car = -1;
int joy_repair_car = -1;
int joy_change_view = -1;
int joy_change_light = -1;
int joy_show_compass = -1;
int joy_axis_accel = SEvent::SJoystickEvent::AXIS_Y;
int joy_axis_steer = SEvent::SJoystickEvent::AXIS_X;
int joy_axis_clutch = -1;
int joy_gear_up = -1;
int joy_gear_down = -1;
int joy_gear1 = -1;
int joy_gear2 = -1;
int joy_gear3 = -1;
int joy_gear4 = -1;
int joy_gear5 = -1;
int joy_gear6 = -1;
int joy_menu = -1;
char gear_type = 'a';

char server_name[256] = "127.0.0.1";
short server_port = 22010;
int send_server_delay = 20;
float gravity = -30.0f;
bool joy_steer_linear = true;

int skin_type = 2;
bool draw_hud = true;
float hud_speed_multiplier = 2.85f;
bool trace_net = true;
bool useBgImageToRender = false;
bool useScreenRTT = false;
bool shitATI = false;
bool flip_vert = false;
bool depth_effect = false;
bool useObjectLods = true;
bool skip_densitymap = true;
bool use_high_poly_objects = false;
bool show_compass_arrow = true;
bool show_reinitialize_button = false;
bool use_mipmaps = true;
bool use_openal = false;

bool this_will_display = false;
int activeJoystick = 0;

bool use_serialized_files = true;
char serialized_file_path[256] = "data/heightmaps/serialized/";

float obj_wire_size = 128.f;
float obj_wire_mult = 4.f;

bool show_names = true;

bool editorMode = false;
bool followCarlos = false;
unsigned int terrain_tesselation = 1;

bool use_threads = false;
bool use_demage = true;

char player_name[256] = "Player";
char team_name[256] = "Players_Team";

bool fps_compensation = true;

void readSettings(const char* fileName)
{
    FILE* f;
    int ret;
    c8 key[256];
    c8 values[256];
    int valuei;
    float valuef;
    
    dprintf(printf("Read settings: %s\n", fileName));

    f = fopen(fileName, "r");
    if (!f)
    {
        printf("settings file unable to open for read: %s\n", fileName);
        return;       
    }

    while (1)    
    {
        ret = fscanf(f, "%s: ", key);
        if ( ret <=0 ) break;
        
        key[strlen(key)-1] = 0;
        
        dprintf(printf("read key: %s\n", key));

        if (strcmp(key,"camera_speed")==0)
        {
            ret = fscanf(f, "%f\n", &cameraSpeed);
            if ( ret <=0 ) break;
        } else
        if (strcmp(key,"joy_dead_zone")==0)
        {
            ret = fscanf(f, "%f\n", &deadZone);
            if ( ret <=0 ) break;
        } else
        if (strcmp(key,"object_visibility_limit")==0)
        {
            ret = fscanf(f, "%f\n", &objectVisibilityLimit);
            if ( ret <=0 ) break;
        } else
        if (strcmp(key,"far_value")==0)
        {
            ret = fscanf(f, "%f\n", &farValue);
            if ( ret <=0 ) break;
        } else
        if (strcmp(key,"near_value")==0)
        {
            ret = fscanf(f, "%f\n", &nearValue);
            if ( ret <=0 ) break;
        } else
        if (strcmp(key,"min_fps")==0)
        {
            ret = fscanf(f, "%f\n", &min_fps);
            if ( ret <=0 ) break;
        } else
        if (strcmp(key,"gravity")==0)
        {
            ret = fscanf(f, "%f\n", &gravity);
            if ( ret <=0 ) break;
        } else
        if (strcmp(key,"hud_speed_multiplier")==0)
        {
            ret = fscanf(f, "%f\n", &hud_speed_multiplier);
            if ( ret <=0 ) break;
        } else
        if (strcmp(key,"obj_wire_size")==0)
        {
            ret = fscanf(f, "%f\n", &obj_wire_size);
            if ( ret <=0 ) break;
        } else
        if (strcmp(key,"obj_wire_mult")==0)
        {
            ret = fscanf(f, "%f\n", &obj_wire_mult);
            if ( ret <=0 ) break;
        } else
        if (strcmp(key,"resolution")==0)
        {
            ret = fscanf(f, "%dx%d\n", &resolutionX, &resolutionY);
            if ( ret <=0 ) break;
        } else
        if (strcmp(key,"shadow_map_size")==0)
        {
            ret = fscanf(f, "%d\n", &shadow_map_size);
            if ( ret <=0 ) break;
        } else
        if (strcmp(key,"LOD_distance")==0)
        {
            ret = fscanf(f, "%d\n", &LOD_distance);
            if ( ret <=0 ) break;
        } else
        if (strcmp(key,"terrain_tesselation")==0)
        {
            ret = fscanf(f, "%u\n", &terrain_tesselation);
            if ( ret <=0 ) break;
            if (terrain_tesselation == 0) terrain_tesselation = 1;
            if (terrain_tesselation > 16) terrain_tesselation = 16;
        } else
        if (strcmp(key,"density_objects")==0)
        {
            ret = fscanf(f, "%d\n", &density_objects);
            if ( ret <=0 ) break;
	        if (density_objects>100) density_objects = 100;
	        if (density_objects<0) density_objects = 0;
        } else
        if (strcmp(key,"density_grasses")==0)
        {
            ret = fscanf(f, "%d\n", &density_grasses);
            if ( ret <=0 ) break;
	        if (density_grasses>100) density_grasses = 100;
	        if (density_grasses<0) density_grasses = 0;
        } else
        if (strcmp(key,"object_pool_size")==0)
        {
            ret = fscanf(f, "%d\n", &object_pool_size);
            if ( ret <=0 ) break;
        } else
        if (strcmp(key,"grass_pool_size")==0)
        {
            ret = fscanf(f, "%d\n", &grass_pool_size);
            if ( ret <=0 ) break;
        } else
        if (strcmp(key,"object_multiplier")==0)
        {
            ret = fscanf(f, "%d\n", &object_multiplier);
            if ( ret <=0 ) break;
        } else
        if (strcmp(key,"grass_multiplier")==0)
        {
            ret = fscanf(f, "%d\n", &grass_multiplier);
            if ( ret <=0 ) break;
        } else
        if (strcmp(key,"grass_type")==0)
        {
            ret = fscanf(f, "%d\n", &grass_type);
            grass_type = grass_type % 4;
            if ( ret <=0 ) break;
        } else
        if (strcmp(key,"display_bits")==0)
        {
            ret = fscanf(f, "%d\n", &display_bits);
            if ( ret <=0 ) break;
        } else
        if (strcmp(key,"joy_accel")==0)
        {
            ret = fscanf(f, "%d\n", &joy_accel);
            if (joy_accel >= SEvent::SJoystickEvent::NUMBER_OF_BUTTONS) joy_accel = -1;
            if ( ret <=0 ) break;
        } else
        if (strcmp(key,"joy_brake")==0)
        {
            ret = fscanf(f, "%d\n", &joy_brake);
            if (joy_brake >= SEvent::SJoystickEvent::NUMBER_OF_BUTTONS) joy_brake = -1;
            if ( ret <=0 ) break;
        } else
        if (strcmp(key,"joy_handbrake")==0)
        {
            ret = fscanf(f, "%d\n", &joy_handbrake);
            if (joy_handbrake >= SEvent::SJoystickEvent::NUMBER_OF_BUTTONS) joy_handbrake = -1;
            if ( ret <=0 ) break;
        } else
        if (strcmp(key,"joy_left")==0)
        {
            ret = fscanf(f, "%d\n", &joy_left);
            if (joy_left >= SEvent::SJoystickEvent::NUMBER_OF_BUTTONS) joy_left = -1;
            if ( ret <=0 ) break;
        } else
        if (strcmp(key,"joy_right")==0)
        {
            ret = fscanf(f, "%d\n", &joy_right);
            if (joy_right >= SEvent::SJoystickEvent::NUMBER_OF_BUTTONS) joy_right = -1;
            if ( ret <=0 ) break;
        } else
        if (strcmp(key,"joy_look_left")==0)
        {
            ret = fscanf(f, "%d\n", &joy_look_left);
            if (joy_look_left >= SEvent::SJoystickEvent::NUMBER_OF_BUTTONS) joy_look_left = -1;
            if ( ret <=0 ) break;
        } else
        if (strcmp(key,"joy_look_right")==0)
        {
            ret = fscanf(f, "%d\n", &joy_look_right);
            if (joy_look_right >= SEvent::SJoystickEvent::NUMBER_OF_BUTTONS) joy_look_right = -1;
            if ( ret <=0 ) break;
        } else
        if (strcmp(key,"joy_reset_car")==0)
        {
            ret = fscanf(f, "%d\n", &joy_reset_car);
            if (joy_reset_car >= SEvent::SJoystickEvent::NUMBER_OF_BUTTONS) joy_reset_car = -1;
            if ( ret <=0 ) break;
        } else
        if (strcmp(key,"joy_repair_car")==0)
        {
            ret = fscanf(f, "%d\n", &joy_repair_car);
            if (joy_repair_car >= SEvent::SJoystickEvent::NUMBER_OF_BUTTONS) joy_repair_car = -1;
            if ( ret <=0 ) break;
        } else
        if (strcmp(key,"joy_change_view")==0)
        {
            ret = fscanf(f, "%d\n", &joy_change_view);
            if (joy_change_view >= SEvent::SJoystickEvent::NUMBER_OF_BUTTONS) joy_change_view = -1;
            if ( ret <=0 ) break;
        } else
        if (strcmp(key,"joy_change_light")==0)
        {
            ret = fscanf(f, "%d\n", &joy_change_light);
            if (joy_change_light >= SEvent::SJoystickEvent::NUMBER_OF_BUTTONS) joy_change_light = -1;
            if ( ret <=0 ) break;
        } else
        if (strcmp(key,"joy_show_compass")==0)
        {
            ret = fscanf(f, "%d\n", &joy_show_compass);
            if (joy_show_compass >= SEvent::SJoystickEvent::NUMBER_OF_BUTTONS) joy_show_compass = -1;
            if ( ret <=0 ) break;
        } else
        if (strcmp(key,"joy_gear_up")==0)
        {
            ret = fscanf(f, "%d\n", &joy_gear_up);
            if (joy_gear_up >= SEvent::SJoystickEvent::NUMBER_OF_BUTTONS) joy_gear_up = -1;
            if ( ret <=0 ) break;
        } else
        if (strcmp(key,"joy_gear_down")==0)
        {
            ret = fscanf(f, "%d\n", &joy_gear_down);
            if (joy_gear_down >= SEvent::SJoystickEvent::NUMBER_OF_BUTTONS) joy_gear_down = -1;
            if ( ret <=0 ) break;
        } else
        if (strcmp(key,"joy_gear1")==0)
        {
            ret = fscanf(f, "%d\n", &joy_gear1);
            if (joy_gear1 >= SEvent::SJoystickEvent::NUMBER_OF_BUTTONS) joy_gear1 = -1;
            if ( ret <=0 ) break;
        } else
        if (strcmp(key,"joy_gear2")==0)
        {
            ret = fscanf(f, "%d\n", &joy_gear2);
            if (joy_gear2 >= SEvent::SJoystickEvent::NUMBER_OF_BUTTONS) joy_gear2 = -1;
            if ( ret <=0 ) break;
        } else
        if (strcmp(key,"joy_gear3")==0)
        {
            ret = fscanf(f, "%d\n", &joy_gear3);
            if (joy_gear3 >= SEvent::SJoystickEvent::NUMBER_OF_BUTTONS) joy_gear3 = -1;
            if ( ret <=0 ) break;
        } else
        if (strcmp(key,"joy_gear4")==0)
        {
            ret = fscanf(f, "%d\n", &joy_gear4);
            if (joy_gear4 >= SEvent::SJoystickEvent::NUMBER_OF_BUTTONS) joy_gear4 = -1;
            if ( ret <=0 ) break;
        } else
        if (strcmp(key,"joy_gear5")==0)
        {
            ret = fscanf(f, "%d\n", &joy_gear5);
            if (joy_gear5 >= SEvent::SJoystickEvent::NUMBER_OF_BUTTONS) joy_gear5 = -1;
            if ( ret <=0 ) break;
        } else
        if (strcmp(key,"joy_gear6")==0)
        {
            ret = fscanf(f, "%d\n", &joy_gear6);
            if (joy_gear6 >= SEvent::SJoystickEvent::NUMBER_OF_BUTTONS) joy_gear6 = -1;
            if ( ret <=0 ) break;
        } else
        if (strcmp(key,"joy_menu")==0)
        {
            ret = fscanf(f, "%d\n", &joy_menu);
            if (joy_menu >= SEvent::SJoystickEvent::NUMBER_OF_BUTTONS) joy_menu = -1;
            if ( ret <=0 ) break;
        } else
        if (strcmp(key,"joy_axis_accel")==0)
        {
            ret = fscanf(f, "%d\n", &joy_axis_accel);
            if (joy_axis_accel >= SEvent::SJoystickEvent::NUMBER_OF_AXES ||
                joy_axis_accel < 0)
                joy_axis_accel = SEvent::SJoystickEvent::AXIS_Y;
            if ( ret <=0 ) break;
        } else
        if (strcmp(key,"joy_axis_steer")==0)
        {
            ret = fscanf(f, "%d\n", &joy_axis_steer);
            if (joy_axis_steer >= SEvent::SJoystickEvent::NUMBER_OF_AXES ||
                joy_axis_steer < 0)
                joy_axis_steer = SEvent::SJoystickEvent::AXIS_X;
            if ( ret <=0 ) break;
        } else
        if (strcmp(key,"joy_axis_clutch")==0)
        {
            ret = fscanf(f, "%d\n", &joy_axis_clutch);
            if (joy_axis_clutch >= SEvent::SJoystickEvent::NUMBER_OF_AXES ||
                joy_axis_clutch < 0)
                joy_axis_clutch = -1;
            if ( ret <=0 ) break;
        } else
        if (strcmp(key,"skin_type")==0)
        {
            ret = fscanf(f, "%d\n", &skin_type);
            if ( ret <=0 ) break;
        } else
        if (strcmp(key,"send_server_delay")==0)
        {
            ret = fscanf(f, "%d\n", &send_server_delay);
            if ( ret <=0 ) break;
        } else
        if (strcmp(key,"server_port")==0)
        {
            ret = fscanf(f, "%hd\n", &server_port);
            if ( ret <=0 ) break;
        } else
        if (strcmp(key,"server_name")==0)
        {
            ret = fscanf(f, "%s\n", server_name);
            if ( ret <=0 ) break;
        } else
        if (strcmp(key,"player_name")==0)
        {
            ret = fscanf(f, "%s\n", player_name);
            if ( ret <=0 ) break;
        } else
        if (strcmp(key,"team_name")==0)
        {
            ret = fscanf(f, "%s\n", team_name);
            if ( ret <=0 ) break;
        } else
        if (strcmp(key,"gear_type")==0)
        {
            ret = fscanf(f, " %c\n", &valuei);
            gear_type = (char)valuei;
            if ( ret <=0 ) break;
        } else
        if (strcmp(key,"full_screen")==0)
        {
            ret = fscanf(f, "%s\n", values);
            if ( ret <=0 ) break;
            if (strcmp(values,"yes")==0)
                full_screen = true;
            else
                full_screen = false;
        } else
        if (strcmp(key,"auto_resolution")==0)
        {
            ret = fscanf(f, "%s\n", values);
            if ( ret <=0 ) break;
            if (strcmp(values,"yes")==0)
                auto_resolution = true;
            else
                auto_resolution = false;
        } else
        if (strcmp(key,"light")==0)
        {
            ret = fscanf(f, "%s\n", values);
            if ( ret <=0 ) break;
            if (strcmp(values,"yes")==0)
                globalLight = true;
            else
                globalLight = false;
        } else
        if (strcmp(key,"shadows")==0)
        {
            ret = fscanf(f, "%s\n", values);
            if ( ret <=0 ) break;
            if (strcmp(values,"yes")==0)
                shadows = true;
            else
                shadows = false;
        } else
        if (strcmp(key,"stencil_shadows")==0)
        {
            ret = fscanf(f, "%s\n", values);
            if ( ret <=0 ) break;
            if (strcmp(values,"yes")==0)
                stencil_shadows = true;
            else
                stencil_shadows = false;
        } else
        if (strcmp(key,"use_mipmaps")==0)
        {
            ret = fscanf(f, "%s\n", values);
            if ( ret <=0 ) break;
            if (strcmp(values,"yes")==0)
                use_mipmaps = true;
            else
                use_mipmaps = false;
        } else
        if (strcmp(key,"use_openal")==0)
        {
            ret = fscanf(f, "%s\n", values);
            if ( ret <=0 ) break;
            if (strcmp(values,"yes")==0)
                use_openal = true;
            else
                use_openal = false;
        } else
        if (strcmp(key,"depth_effect")==0)
        {
            ret = fscanf(f, "%s\n", values);
            if ( ret <=0 ) break;
            if (strcmp(values,"yes")==0)
                depth_effect = true;
            else
                depth_effect = false;
        } else
        if (strcmp(key,"use_object_lods")==0)
        {
            ret = fscanf(f, "%s\n", values);
            if ( ret <=0 ) break;
            if (strcmp(values,"yes")==0)
                useObjectLods = true;
            else
                useObjectLods = false;
        } else
        if (strcmp(key,"skip_densitymap")==0)
        {
            ret = fscanf(f, "%s\n", values);
            if ( ret <=0 ) break;
            if (strcmp(values,"yes")==0)
                skip_densitymap = true;
            else
                skip_densitymap = false;
        } else
        if (strcmp(key,"use_high_poly_objects")==0)
        {
            ret = fscanf(f, "%s\n", values);
            if ( ret <=0 ) break;
            if (strcmp(values,"yes")==0)
                use_high_poly_objects = true;
            else
                use_high_poly_objects = false;
        } else
        if (strcmp(key,"show_compass_arrow")==0)
        {
            ret = fscanf(f, "%s\n", values);
            if ( ret <=0 ) break;
            if (strcmp(values,"yes")==0)
                show_compass_arrow = true;
            else
                show_compass_arrow = false;
        } else
        if (strcmp(key,"show_reinitialize_button")==0)
        {
            ret = fscanf(f, "%s\n", values);
            if ( ret <=0 ) break;
            if (strcmp(values,"yes")==0)
                show_reinitialize_button = true;
            else
                show_reinitialize_button = false;
        } else
        if (strcmp(key,"use_bg_image_to_render")==0)
        {
            ret = fscanf(f, "%s\n", values);
            if ( ret <=0 ) break;
            if (strcmp(values,"yes")==0)
                useBgImageToRender = true;
            else
                useBgImageToRender = false;
        } else
        if (strcmp(key,"use_screen_rtt")==0)
        {
            ret = fscanf(f, "%s\n", values);
            if ( ret <=0 ) break;
            if (strcmp(values,"yes")==0)
                useScreenRTT = true;
            else
                useScreenRTT = false;
        } else
        if (strcmp(key,"shit_ati")==0)
        {
            ret = fscanf(f, "%s\n", values);
            if ( ret <=0 ) break;
            if (strcmp(values,"yes")==0)
                shitATI = true;
            else
                shitATI = false;
        } else
        if (strcmp(key,"flip_vert")==0)
        {
            ret = fscanf(f, "%s\n", values);
            if ( ret <=0 ) break;
            if (strcmp(values,"yes")==0)
                flip_vert = true;
            else
                flip_vert = false;
        } else
        if (strcmp(key,"shaders")==0)
        {
            ret = fscanf(f, "%s\n", values);
            if ( ret <=0 ) break;
            if (strcmp(values,"yes")==0)
                useShaders = true;
            else
                useShaders = false;
        } else
        if (strcmp(key,"cg_shaders")==0)
        {
            ret = fscanf(f, "%s\n", values);
            if ( ret <=0 ) break;
            if (strcmp(values,"yes")==0)
                useCgShaders = true;
            else
                useCgShaders = false;
        } else
        if (strcmp(key,"adv_cg_shaders")==0)
        {
            ret = fscanf(f, "%s\n", values);
            if ( ret <=0 ) break;
            if (strcmp(values,"yes")==0)
                useAdvCgShaders = true;
            else
                useAdvCgShaders = false;
        } else
        if (strcmp(key,"use_detailed_terrain")==0)
        {
            ret = fscanf(f, "%s\n", values);
            if ( ret <=0 ) break;
            if (strcmp(values,"yes")==0)
                use_detailed_terrain = true;
            else
                use_detailed_terrain = false;
        } else
        if (strcmp(key,"use_highres_textures")==0)
        {
            ret = fscanf(f, "%s\n", values);
            if ( ret <=0 ) break;
            if (strcmp(values,"yes")==0)
                use_highres_textures = true;
            else
                use_highres_textures = false;
        } else
        if (strcmp(key,"start_with_mainmenu")==0)
        {
            ret = fscanf(f, "%s\n", values);
            if ( ret <=0 ) break;
            if (strcmp(values,"yes")==0)
                start_with_mainmenu = true;
            else
                start_with_mainmenu = false;
        } else
        if (strcmp(key,"display_extra_info")==0)
        {
            ret = fscanf(f, "%s\n", values);
            if ( ret <=0 ) break;
            if (strcmp(values,"yes")==0)
                display_extra_info = true;
            else
                display_extra_info = false;
        } else
        if (strcmp(key,"info_bg")==0)
        {
            ret = fscanf(f, "%s\n", values);
            if ( ret <=0 ) break;
            if (strcmp(values,"yes")==0)
                info_bg = true;
            else
                info_bg = false;
        } else
        if (strcmp(key,"message_bg")==0)
        {
            ret = fscanf(f, "%s\n", values);
            if ( ret <=0 ) break;
            if (strcmp(values,"yes")==0)
                message_bg = true;
            else
                message_bg = false;
        } else
        if (strcmp(key,"anti_aliasing")==0)
        {
            ret = fscanf(f, "%d\n", &anti_aliasing);
            if ( ret <=0 ) break;
            if (anti_aliasing >= 16)
            {
                anti_aliasing = 16;
            }
            else
            if (anti_aliasing >= 8)
            {
                anti_aliasing = 8;
            }
            else
            if (anti_aliasing >= 4)
            {
                anti_aliasing = 4;
            }
            else
            if (anti_aliasing >= 2)
            {
                anti_aliasing = 2;
            }
            else
            {
                anti_aliasing = 0;
            }
                
        } else
        if (strcmp(key,"vsync")==0)
        {
            ret = fscanf(f, "%s\n", values);
            if ( ret <=0 ) break;
            if (strcmp(values,"yes")==0)
                vsync = true;
            else
                vsync = false;
        } else
        if (strcmp(key,"high_precision_fpu")==0)
        {
            ret = fscanf(f, "%s\n", values);
            if ( ret <=0 ) break;
            if (strcmp(values,"yes")==0)
                high_precision_fpu = true;
            else
                high_precision_fpu = false;
        } else
        if (strcmp(key,"this_will_display")==0)
        {
            ret = fscanf(f, "%s\n", values);
            if ( ret <=0 ) break;
            if (strcmp(values,"yes")==0)
                this_will_display = true;
            else
                this_will_display = false;
        } else
        if (strcmp(key,"joy_steer_linear")==0)
        {
            ret = fscanf(f, "%s\n", values);
            if ( ret <=0 ) break;
            if (strcmp(values,"yes")==0)
                joy_steer_linear = true;
            else
                joy_steer_linear = false;
        } else
        if (strcmp(key,"draw_hud")==0)
        {
            ret = fscanf(f, "%s\n", values);
            if ( ret <=0 ) break;
            if (strcmp(values,"yes")==0)
                draw_hud = true;
            else
                draw_hud = false;
        } else
        if (strcmp(key,"show_names")==0)
        {
            ret = fscanf(f, "%s\n", values);
            if ( ret <=0 ) break;
            if (strcmp(values,"yes")==0)
                show_names = true;
            else
                show_names = false;
        } else
        if (strcmp(key,"editor_mode")==0)
        {
            ret = fscanf(f, "%s\n", values);
            if ( ret <=0 ) break;
            if (strcmp(values,"yes")==0)
                editorMode = true;
            else
                editorMode = false;
        } else
        if (strcmp(key,"follow_carlos")==0)
        {
            ret = fscanf(f, "%s\n", values);
            if ( ret <=0 ) break;
            if (strcmp(values,"yes")==0)
                followCarlos = true;
            else
                followCarlos = false;
        } else
        if (strcmp(key,"use_threads")==0)
        {
            ret = fscanf(f, "%s\n", values);
            if ( ret <=0 ) break;
            if (strcmp(values,"yes")==0)
                use_threads = true;
            else
                use_threads = false;
        } else
        if (strcmp(key,"use_demage")==0)
        {
            ret = fscanf(f, "%s\n", values);
            if ( ret <=0 ) break;
            if (strcmp(values,"yes")==0)
                use_demage = true;
            else
                use_demage = false;
        } else
        if (strcmp(key,"use_damage")==0)
        {
            ret = fscanf(f, "%s\n", values);
            if ( ret <=0 ) break;
            if (strcmp(values,"yes")==0)
                use_demage = true;
            else
                use_demage = false;
        } else
        if (strcmp(key,"fps_compensation")==0)
        {
            ret = fscanf(f, "%s\n", values);
            if ( ret <=0 ) break;
            if (strcmp(values,"yes")==0)
                fps_compensation = true;
            else
                fps_compensation = false;
        } else
        if (strcmp(key,"trace_net")==0)
        {
            ret = fscanf(f, "%s\n", values);
            if ( ret <=0 ) break;
            if (strcmp(values,"yes")==0)
                trace_net = true;
            else
                trace_net = false;
        } else
        if (strcmp(key,"use_serialized_files")==0)
        {
            ret = fscanf(f, "%s\n", values);
            if ( ret <=0 ) break;
            if (strcmp(values,"yes")==0)
                use_serialized_files = true;
            else
                use_serialized_files = false;
        } else
        if (strcmp(key,"serialized_file_path")==0)
        {
            ret = fscanf(f, "%s\n", serialized_file_path);
            if ( ret <=0 ) break;
            int len = strlen(serialized_file_path);
            if (len)
            {
                if (serialized_file_path[len-1] != '/')
                {
                    serialized_file_path[len] = '/';
                    serialized_file_path[len+1] = 0;
                }
            }
            else
            {
                strcpy(serialized_file_path, "data/heightmaps/serialized/");
            }
        } else
        if (strcmp(key,"driver_type")==0)
        {
            ret = fscanf(f, "%s\n", values);
            if ( ret <=0 ) break;
            if (strcmp(values,"opengl")==0)
                driverType = video::EDT_OPENGL;
            else
            /*
            if (strcmp(values,"opengl3")==0)
                driverType = video::EDT_OPENGL3;
            else
            */
            if (strcmp(values,"d3d9")==0)
#ifdef __linux__
                driverType = video::EDT_OPENGL;
#else
                driverType = video::EDT_DIRECT3D9;
#endif
            else
            if (strcmp(values,"d3d8")==0)
#ifdef __linux__
                driverType = video::EDT_OPENGL;
#else
                driverType = video::EDT_DIRECT3D8;
#endif
            else
                driverType = video::EDT_BURNINGSVIDEO;//EDT_SOFTWARE;
        } else
        if (strcmp(key,"smokes")==0)
        {
            ret = fscanf(f, "%s\n", values);
            if ( ret <=0 ) break;
            if (strcmp(values,"yes")==0)
                useSmokes = true;
            else
                useSmokes = false;
        }
/*      else
        if (strcmp(key,"")==0)
        {
            ret = fscanf(f, "%\n", &);
            if ( ret <=0 ) break;
        }
*/
    }    
    fclose(f);
    
    objectVisibilityLimit = obj_wire_size * obj_wire_mult;
}

bool writeSettings(const char* fileName)
{
    FILE* f;
    int ret;
    c8 values[256];
    int valuei;
    float valuef;
    
    dprintf(printf("Write settings: %s\n", fileName));

    f = fopen(fileName, "w+");
    if (!f)
    {
        printf("settings file unable to open for write: %s\n", fileName);
        return false;
    }

    ret = fprintf(f, "camera_speed: %f\n", cameraSpeed);
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "joy_dead_zone: %f\n", deadZone);
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "object_visibility_limit: %f\n", objectVisibilityLimit);
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "far_value: %f\n", farValue);
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "near_value: %f\n", nearValue);
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "min_fps: %f\n", min_fps);
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "gravity: %f\n", gravity);
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "hud_speed_multiplier: %f\n", hud_speed_multiplier);
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "obj_wire_size: %f\n", obj_wire_size);
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "obj_wire_mult: %f\n", obj_wire_mult);
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "resolution: %dx%d\n", resolutionX, resolutionY);
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "shadow_map_size: %d\n", shadow_map_size);
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "LOD_distance: %d\n", LOD_distance);
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "terrain_tesselation: %u\n", terrain_tesselation);
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "density_objects: %d\n", density_objects);
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "density_grasses: %d\n", density_grasses);
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "object_pool_size: %d\n", object_pool_size);
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "grass_pool_size: %d\n", grass_pool_size);
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "object_multiplier: %d\n", object_multiplier);
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "grass_multiplier: %d\n", grass_multiplier);
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "grass_type: %d\n", grass_type);
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "display_bits: %d\n", display_bits);
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "joy_accel: %d\n", joy_accel);
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "joy_brake: %d\n", joy_brake);
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "joy_handbrake: %d\n", joy_handbrake);
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "joy_left: %d\n", joy_left);
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "joy_right: %d\n", joy_right);
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "joy_look_left: %d\n", joy_look_left);
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "joy_look_right: %d\n", joy_look_right);
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "joy_reset_car: %d\n", joy_reset_car);
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "joy_repair_car: %d\n", joy_repair_car);
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "joy_change_view: %d\n", joy_change_view);
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "joy_change_light: %d\n", joy_change_light);
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "joy_show_compass: %d\n", joy_show_compass);
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "joy_gear_up: %d\n", joy_gear_up);
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "joy_gear_down: %d\n", joy_gear_down);
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "joy_gear1: %d\n", joy_gear1);
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "joy_gear2: %d\n", joy_gear2);
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "joy_gear3: %d\n", joy_gear3);
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "joy_gear4: %d\n", joy_gear4);
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "joy_gear5: %d\n", joy_gear5);
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "joy_gear6: %d\n", joy_gear6);
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "joy_menu: %d\n", joy_menu);
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "joy_axis_accel: %d\n", joy_axis_accel);
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "joy_axis_steer: %d\n", joy_axis_steer);
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "joy_axis_clutch: %d\n", joy_axis_clutch);
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "skin_type: %d\n", skin_type);
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "send_server_delay: %d\n", send_server_delay);
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "server_port: %hd\n", server_port);
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "server_name: %s\n", server_name);
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "player_name: %s\n", player_name);
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "team_name: %s\n", team_name);
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "gear_type: %c\n", gear_type);
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "full_screen: %s\n", full_screen?"yes":"no");
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "auto_resolution: %s\n", auto_resolution?"yes":"no");
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "light: %s\n", globalLight?"yes":"no");
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "shadows: %s\n", shadows?"yes":"no");
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "stencil_shadows: %s\n", stencil_shadows?"yes":"no");
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "use_mipmaps: %s\n", use_mipmaps?"yes":"no");
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "use_openal: %s\n", use_openal?"yes":"no");
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "depth_effect: %s\n", depth_effect?"yes":"no");
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "use_object_lods: %s\n", useObjectLods?"yes":"no");
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "skip_densitymap: %s\n", skip_densitymap?"yes":"no");
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "use_high_poly_objects: %s\n", use_high_poly_objects?"yes":"no");
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "show_compass_arrow: %s\n", show_compass_arrow?"yes":"no");
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "show_reinitialize_button: %s\n", show_reinitialize_button?"yes":"no");
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "use_bg_image_to_render: %s\n", useBgImageToRender?"yes":"no");
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "use_screen_rtt: %s\n", useScreenRTT?"yes":"no");
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "shit_ati: %s\n", shitATI?"yes":"no");
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "flip_vert: %s\n", flip_vert?"yes":"no");
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "shaders: %s\n", useShaders?"yes":"no");
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "cg_shaders: %s\n", useCgShaders?"yes":"no");
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "adv_cg_shaders: %s\n", useAdvCgShaders?"yes":"no");
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "use_detailed_terrain: %s\n", use_detailed_terrain?"yes":"no");
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "use_highres_textures: %s\n", use_highres_textures?"yes":"no");
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "start_with_mainmenu: %s\n", start_with_mainmenu?"yes":"no");
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "display_extra_info: %s\n", display_extra_info?"yes":"no");
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "info_bg: %s\n", info_bg?"yes":"no");
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "message_bg: %s\n", message_bg?"yes":"no");
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "anti_aliasing: %d\n", anti_aliasing);
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "vsync: %s\n", vsync?"yes":"no");
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "high_precision_fpu: %s\n", high_precision_fpu?"yes":"no");
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "this_will_display: %s\n", this_will_display?"yes":"no");
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "joy_steer_linear: %s\n", joy_steer_linear?"yes":"no");
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "draw_hud: %s\n", draw_hud?"yes":"no");
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "show_names: %s\n", show_names?"yes":"no");
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "editor_mode: %s\n", editorMode?"yes":"no");
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "follow_carlos: %s\n", followCarlos?"yes":"no");
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "use_threads: %s\n", use_threads?"yes":"no");
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "use_damage: %s\n", use_demage?"yes":"no");
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "fps_compensation: %s\n", fps_compensation?"yes":"no");
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "trace_net: %s\n", trace_net?"yes":"no");
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "smokes: %s\n", useSmokes?"yes":"no");
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "use_serialized_files: %s\n", use_serialized_files?"yes":"no");
    if ( ret <=0 ) {fclose(f); return false;}
    ret = fprintf(f, "serialized_file_path: %s\n", serialized_file_path);
    if ( ret <=0 ) {fclose(f); return false;}
    if (driverType == video::EDT_OPENGL)
    {
        ret = fprintf(f, "driver_type: opengl\n");
        if ( ret <=0 ) {fclose(f); return false;}
    } else
    /*
    if (driverType == video::EDT_OPENGL3)
    {
        ret = fprintf(f, "driver_type: opengl3\n");
        if ( ret <=0 ) {fclose(f); return false;}
    } else
    */
    if (driverType == video::EDT_DIRECT3D9)
    {
        ret = fprintf(f, "driver_type: d3d9\n");
        if ( ret <=0 ) {fclose(f); return false;}
    } else
    if (driverType == video::EDT_DIRECT3D8)
    {
        ret = fprintf(f, "driver_type: d3d8\n");
        if ( ret <=0 ) {fclose(f); return false;}
    } else
    {
        ret = fprintf(f, "driver_type: software\n");
        if ( ret <=0 ) {fclose(f); return false;}
    }

    fclose(f);
    
    return true;
}
