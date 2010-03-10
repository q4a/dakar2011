/****************************************************************
*                                                               *
*    Name: Materials.h                                          *
*                                                               *
*    Creator: Balazs Tuska                                      *
*                                                               *
*    Description:                                               *
*       This file contains the materials. Play sound when the   *
*       car contact to some object.                             *
*                                                               *
****************************************************************/

#if !defined(AFX_MATERIALS_H__C8152D99_8176_4CE4_BF86_9D241D3B54ED__INCLUDED_)
#define AFX_MATERIALS_H__C8152D99_8176_4CE4_BF86_9D241D3B54ED__INCLUDED_

//#define DEBUG_NEWTON

#ifdef USE_MY_SOUNDENGINE
 #include "mySound.h"
#else
 #include <irrKlang.h>
 using namespace irrklang;
#endif

struct NewtonWorld;
//extern int woodID; 
//extern int metalID; 
extern int treeID; 
extern int levelID; 
extern int vehicleID; 
extern int roadID; 
extern int tireID; 
//extern int characterID; 


void SetupMaterials (NewtonWorld* nWorld,
#ifdef USE_MY_SOUNDENGINE
                     CMySoundEngine* soundEngine
#else
                     irrklang::ISoundEngine* soundEngine
#endif
                     );
void CleanUpMaterials (NewtonWorld* nWorld);

#endif 
