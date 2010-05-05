/****************************************************************
*                                                               *
*    Name: fonts.h                                              *
*                                                               *
*    Creator: Balazs Tuska                                      *
*                                                               *
*    Description:                                               *
*       This file contains the font stuff.                      *
*                                                               *
****************************************************************/

#ifndef __FONTS_H__
#define __FONTS_H__

#include "irrlicht.h"
// Irrlicht Namespaces
using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

enum {
    FONT_BUILTIN = 0,
    FONT_BASE1,
    FONT_BASE2,
    FONT_BASE3,

    FONT_SMALL,
    FONT_NORMAL,
    FONT_LARGE,
    FONT_EXTRALARGE,

    FONT_SMALLBOLD,
    FONT_NORMALBOLD,
    FONT_LARGEBOLD,
    FONT_EXTRALARGEBOLD,

    FONT_SPECIALSMALL,
    FONT_SPECIALNORMAL,
    FONT_SPECIALLARGE,
    FONT_SPECIALEXTRALARGE,

    FONT_SPECIALSMALLBOLD,
    FONT_SPECIALNORMALBOLD,
    FONT_SPECIALLARGEBOLD,
    FONT_SPECIALEXTRALARGEBOLD,

    FONT_SPECIAL14,
    FONT_SPECIAL16,
    FONT_SPECIAL18,

    FONT_COUNT,
};

extern gui::IGUIFont** fonts;

void setupFonts(gui::IGUIEnvironment* env);
void releaseFonts();

#endif // __FONTS_H__
