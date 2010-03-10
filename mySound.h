
#ifndef __MYSOUND_H__
#define __MYSOUND_H__

#include <AL/alc.h>

#include "irrlicht.h"
// Irrlicht Namespaces
using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

#include "MyList.h"

#define NUM_OF_BUFFERS 64
#define NUM_OF_SOURCES 32
#define EMPTY_SLOT 0xFFFFFFFF

class CMySound;

struct SSoundFileNameBuffer
{
    char filename[256];
    unsigned int buffer;
};

class CMySoundEngine
{
public:
    CMySoundEngine();
    ~CMySoundEngine();
    
    CMySound* play3D(const char *soundFileName, core::vector3df pos, bool playLooped=false,
                      bool startPaused=false, bool track=false);
    CMySound* play2D(const char *soundFileName, bool playLooped=false,
                      bool startPaused=false, bool track=false);
    void setListenerPosition(const core::vector3df &pos, const core::vector3df &lookdir,
                             const core::vector3df &velPerSecond=core::vector3df(0, 0, 0),
                             const core::vector3df &upVector=core::vector3df(0, 1, 0));

    core::vector3df getListenerPosition() const;
    
private:
    CMyList<SSoundFileNameBuffer*> soundFileNameBuffer;
    
    void updateListener();
    
    bool initialized;
    
    core::vector3df listenerPos;
    core::vector3df listenerVel;
    core::vector3df listenerDir;
    core::vector3df listenerUp;

    ALCdevice* pDevice;
    ALCcontext* pContext;  
};


class CMySound
{
public:
    CMySound(unsigned int newBuffer, bool plooped, core::vector3df &pos);
    ~CMySound();
    
    void setVolume(float newVolume);
    float getVolume() const;

    void setPlaybackSpeed(float newPlaybackSpeed);
    float getPlaybackSpeed() const;
    
    void setIsLooped(bool newLoop);
    bool isLooped() const;
    
    void setPosition(core::vector3df &newPos);
    void setPosition(core::vector3df newPos);
    core::vector3df getPosition() const;
    
    void setVelocity(core::vector3df &newVelocity);
    core::vector3df getVelocity() const;
    
    void setMaxDistance(float newMaxDistance);
    float getMaxDistance() const;
    void setMinDistance(float newMinDistance);
    float getMinDistance() const;
    
    void play();
    void stop();
    
    void setIsPaused(bool newPaused);
    bool isPaused() const;
    
private:
    
    unsigned int source;

    core::vector3df soundSourcePos;
    core::vector3df soundSourceVel;
    
    bool initialized;
    unsigned int buffer;
    bool paused;
    float volume;
    float playbackSpeed;
    float maxDistance;
    float minDistance;
    bool looped;
    bool isStopped;
    
};

#endif // __MYSOUND_H__
