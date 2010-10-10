/****************************************************************
*                                                               *
*    Name: settings.h                                           *
*                                                               *
*    Creator: Balazs Tuska                                      *
*                                                               *
*    Description:                                               *
*       This file contains the variables of the global settings *
*       like: display, conroller, object, grass density etc.    *
*                                                               *
****************************************************************/

#ifndef __SETTINGS_H__
#define __SETTINGS_H__

#include "irrlicht.h"

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

extern float cameraSpeed;
extern float deadZone;
extern bool globalLight;
extern bool shadows;
extern bool stencil_shadows;
extern int shadow_map_size;
extern bool useShaders;
extern bool useCgShaders;
extern bool useAdvCgShaders;
extern bool use_highres_textures;
extern video::E_DRIVER_TYPE driverType;
extern bool useSmokes;
extern float objectVisibilityLimit;
extern float farValue;
extern float nearValue;
extern bool full_screen;
extern int auto_resolution;
extern int resolutionX;
extern int resolutionY;
extern bool start_with_mainmenu;
extern int LOD_distance;
extern int density_objects;
extern int object_pool_size;
extern int grass_pool_size;
extern int grass_type;
extern bool display_extra_info;
extern bool info_bg;
extern bool message_bg;
extern int anti_aliasing;
extern bool vsync;
extern bool high_precision_fpu;
extern int display_bits;
extern int joy_accel;
extern int joy_brake;
extern int joy_handbrake;
extern int joy_left;
extern int joy_right;
extern int joy_look_left;
extern int joy_look_right;
extern int joy_reset_car;
extern int joy_repair_car;
extern int joy_change_view;
extern int joy_change_light;
extern int joy_show_map;
extern int joy_axis_accel;
extern int joy_axis_steer;
extern int joy_axis_clutch;
extern char server_name[256];
extern short server_port;
extern int send_server_delay;
extern float gravity;
extern bool joy_steer_linear;
extern int joy_gear_up;
extern int joy_gear_down;
extern int joy_gear1;
extern int joy_gear2;
extern int joy_gear3;
extern int joy_gear4;
extern int joy_gear5;
extern int joy_gear6;
extern int joy_menu;
extern char gear_type;
extern int skin_type;
extern bool draw_hud;
extern float hud_speed_multiplier;
extern bool trace_net;
extern bool useScreenRTT;
extern bool shitATI;
extern bool flip_vert;
extern bool depth_effect;
extern bool show_reinitialize_button;
extern bool use_mipmaps;

extern bool this_will_display;
extern int activeJoystick;

extern float obj_wire_size;
extern float obj_wire_mult;

extern bool show_names;

extern bool editorMode;
extern bool followCarlos;

extern bool use_threads;
extern bool use_demage;

extern char player_name[256];
extern char team_name[256];

extern bool fps_compensation;

extern int difficulty;

void readSettings(const char*);

bool writeSettings(const char*);

#endif // __SETTINGS_H__
