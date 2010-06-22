/****************************************************************
*                                                               *
*    Name: editor.h                                             *
*                                                               *
*    Creator: Balazs Tuska                                      *
*                                                               *
*    Description:                                               *
*                                                               *
****************************************************************/

#ifndef __EDITOR_H__
#define __EDITOR_H__

#include "irrlicht.h"
// Irrlicht Namespaces
using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

//#define USE_EDITOR 1

void initEditor(IGUIEnvironment* env);
void editorSetVisible(bool vis);
void updateEditor();
bool actionEditor(int key);

#endif // __EDITOR_H__
