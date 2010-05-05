/****************************************************************
*                                                               *
*    Name: error.cpp                                            *
*                                                               *
*    Creator: Balazs Tuska                                      *
*                                                               *
*    Description:                                               *
*       This file contains some error handling functions.       *
*       Messages towards the user.                              *
*                                                               *
****************************************************************/

#include "error.h"
#include <stdio.h>
#include <stdarg.h>

static void printMessage(int num, const char *msg1, const char *msg2, va_list ap)
{
    fflush(stderr);
    fflush(stdout);
    if (num)
        fprintf(stderr, "\n%s - code: %d: ", msg1, num);
    else
        fprintf(stderr, "\n%s: ", msg1);
    vfprintf(stderr, msg2, ap);
    fprintf(stderr, "\n");
    fflush(stderr);
}

//****************************************************************************
// linux

#ifndef WIN32

extern "C" void myError(int num, const char *msg, ...)
{
    va_list ap;
    va_start(ap,msg);
    printMessage(num, "Dakar 2011 error", msg, ap);
    exit(1);
}

/*
extern "C" void myDebug(int num, const char *msg, ...)
{
    va_list ap;
    va_start(ap, msg);
    printMessage(num,"Dakar 2011 debug", msg, ap);
    // *((char *)0) = 0;   ... commit SEGVicide
    abort();
}
*/

extern "C" void myMessage(int num, const char *msg, ...)
{
    va_list ap;
    va_start(ap, msg);
    printMessage(num, "Dakar 2011 message", msg, ap);
}

#endif

//****************************************************************************
// windows

#ifdef WIN32

// isn't cygwin annoying!
#ifdef CYGWIN
#define _snprintf snprintf
#define _vsnprintf vsnprintf
#endif


#include "windows.h"


extern "C" void myError(int num, const char *msg, ...)
{
    va_list ap;
    va_start(ap, msg);
    char s[1000], title[100];
    _snprintf(title, sizeof(title), "Dakar 2011 - code: %d", num);
    _vsnprintf(s, sizeof(s), msg, ap);
    s[sizeof(s)-1] = 0;
    printMessage(num, "Dakar 2011 error", msg, ap);
    MessageBox(0, s, title, MB_OK | MB_ICONSTOP);
    exit(1);
}

/*
extern "C" void myDebug(int num, const char *msg, ...)
{
    va_list ap;
    va_start(ap, msg);
    char s[1000], title[100];
    _snprintf(title, sizeof(title), "Dakar 2011 debug %d", num);
    _vsnprintf(s, sizeof(s), msg, ap);
    s[sizeof(s)-1] = 0;
    printMessage(num,"Dakar 2011 debug", msg, ap);
    MessageBox(0, s, title, MB_OK | MB_ICONSTOP);
    abort();
}
*/

extern "C" void myMessage(int num, const char *msg, ...)
{
    va_list ap;
    va_start(ap, msg);
    char s[1000], title[100];
    _snprintf(title, sizeof(title), "Dakar 2011 message - code: %d", num);
    _vsnprintf(s, sizeof(s), msg, ap);
    s[sizeof(s)-1] = 0;
    printMessage(num, "Dakar 2011 message", msg, ap);
    MessageBox(0, s, title, MB_OK | MB_ICONWARNING);
//    printMessage (num,"ODE Message",msg,ap);
}

#endif

static irr::IrrlichtDevice* device;
static irr::IOSOperator* os;
static unsigned int initialTM = 0;
static unsigned int initialFM = 0;

void initializeUsedMemory(irr::IrrlichtDevice* p_device)
{
    device = p_device;
    os = device->getOSOperator();
    os->getSystemMemory(&initialTM, &initialFM);
}

unsigned int getUsedMemory()
{
    unsigned int tmpTM = 0;
    unsigned int tmpFM = 0;
    os->getSystemMemory(&tmpTM, &tmpFM);
    return initialFM - tmpFM;
}

void printUsedMemory(unsigned int num)
{
    myMessage(num, "Memory: %u", getUsedMemory()/(1024));
}
