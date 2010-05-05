/****************************************************************
*                                                               *
*    Name: error.h                                              *
*                                                               *
*    Creator: Balazs Tuska                                      *
*                                                               *
*    Description:                                               *
*       This file contains some error handling functions.       *
*       Messages towards the user.                              *
*                                                               *
****************************************************************/

#ifndef __ERROR_H__
#define __ERROR_H__

#include "irrlicht.h"
// Irrlicht Namespaces
using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;


#ifdef __cplusplus
extern "C" {
#endif

/* generate a fatal error, debug trap or a message. */
void myError(int num, const char *msg, ...);
//void myDebug(int num, const char *msg, ...);
void myMessage(int num, const char *msg, ...);



void initializeUsedMemory(irr::IrrlichtDevice* p_device);
unsigned int getUsedMemory();
void printUsedMemory(unsigned int num = 0);

#ifdef __cplusplus
}
#endif

#endif // __ERROR_H__
