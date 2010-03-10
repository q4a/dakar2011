
#include "mySound.h"

//#include <conio.h>
#include <stdlib.h>
#include <AL/al.h>
#include <AL/alc.h>
//#include <AL/alu.h>
#include <AL/alut.h>

/*
#include "irrlicht.h"
// Irrlicht Namespaces
using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

#include "MyList.h"
*/

//#define MY_SOUND_DEBUG 1

//ALCchar DeviceName[] = "DirectSound3D";

CMySoundEngine::CMySoundEngine() : initialized(false), soundFileNameBuffer()
{
#ifdef MY_SOUND_DEBUG
    printf("Initialize OpenAL\n");
#endif

//#ifdef __linux__
    bool ret = alutInit(0, 0);
    int err = alGetError();
    if (ret && err == 0)
    {
        printf("OpenAL initialized successfully\n");
    }
    else
    {
        printf("OpenAL initialize failed ret %d (err: %d)\n", ret, err);
    }
    
//#else
//    pDevice = alcOpenDevice(0);
//    pContext = alcCreateContext(pDevice, NULL);
//    alcMakeContextCurrent(pContext);
//#endif

//    if (alGetError() != AL_NO_ERROR)
//    {
//#ifdef MY_SOUND_DEBUG
//        printf("AL: could not initialize sound device\n");
//#endif
//    }
    alGetError();
       
    initialized = true;
    
    listenerPos = core::vector3df(0.f,0.f,0.f);
    listenerVel = core::vector3df(0.f,0.f,0.f);
    listenerDir = core::vector3df(0.f,0.f,-1.f);
    listenerUp = core::vector3df(0.f,1.f,0.f);;

    updateListener();
}

CMySoundEngine::~CMySoundEngine()
{
#ifdef MY_SOUND_DEBUG
    printf("free up OpenAL: %d buffers in use\n", soundFileNameBuffer.length());
#endif
    for (int i = 0; i < soundFileNameBuffer.length();i++)
    {
#ifdef MY_SOUND_DEBUG
        printf("free up %s\n", soundFileNameBuffer[i]->filename);
#endif
        alDeleteBuffers(1, &soundFileNameBuffer[i]->buffer);
        delete soundFileNameBuffer[i];
    }

//#ifdef __linux__
    //alutExit();
//#else
//    alcMakeContextCurrent(NULL);
//    alcDestroyContext(pContext);
//    alcCloseDevice(pDevice);
//#endif
}
    
CMySound* CMySoundEngine::play3D(const char *soundFileName, core::vector3df pos, bool playLooped,
                                 bool startPaused, bool track)
{
    if (this==0 || !initialized) return 0;
#ifdef MY_SOUND_DEBUG
    printf("AL: play3D |%s| l %d sp %d\n", soundFileName, playLooped, startPaused);
#endif
    bool found = false;
    int i = 0;
    for (i = 0; i < soundFileNameBuffer.length(); i++)
    {
        if (strcmp(soundFileName, soundFileNameBuffer[i]->filename) == 0)
        {
            found = true;
            break;
        }
    }
    
    if (!found)
    {
#ifdef MY_SOUND_DEBUG
        printf("AL: buffer not found\n");
#endif
        SSoundFileNameBuffer* newElem = new SSoundFileNameBuffer();
        strcpy(newElem->filename, soundFileName);

        alGenBuffers(1, &newElem->buffer);
        if (alGetError() != AL_NO_ERROR)
        {
#ifdef MY_SOUND_DEBUG
            printf("AL: could not create buffer\n");
#endif
            delete newElem;
            return 0;
        }
        
        ALenum format;
        ALsizei size;
        ALvoid* data;
        ALsizei freq;
        ALboolean loop;
        newElem->buffer = alutCreateBufferFromFile(newElem->filename);
//        alutLoadWAVFile(newElem->filename, &format, &data, &size, &freq, &loop);
//        alBufferData(newElem->buffer, format, data, size, freq);
//        alutUnloadWAV(format, data, size, freq);
        alGetError();

        soundFileNameBuffer.addLast(newElem);
    }
    else
    {
#ifdef MY_SOUND_DEBUG
        printf("AL: buffer found %d\n", i);
#endif
    }
    
    CMySound* sound = new CMySound(soundFileNameBuffer[i]->buffer, playLooped, pos);
    
    if(!startPaused) sound->setIsPaused(false);
    
    return sound;
}

CMySound* CMySoundEngine::play2D(const char *soundFileName, bool playLooped,
                                 bool startPaused, bool track)
{
    return play3D(soundFileName, listenerPos, playLooped, startPaused, track);
}

void CMySoundEngine::setListenerPosition(const core::vector3df &pos, const core::vector3df &lookdir,
                             const core::vector3df &velPerSecond,
                             const core::vector3df &upVector)
{
    listenerPos = pos;
    listenerVel = velPerSecond;
    listenerDir = lookdir;
    listenerUp = upVector;
    
    updateListener();
}

core::vector3df CMySoundEngine::getListenerPosition() const
{
    return listenerPos;
}
        
void CMySoundEngine::updateListener()
{
    ALfloat ListenerOri[] = { listenerDir.X, listenerDir.Y, listenerDir.Z, listenerUp.X, listenerUp.Y, listenerUp.Z};
    alListenerfv(AL_POSITION,    &listenerPos.X);
    alListenerfv(AL_VELOCITY,    &listenerVel.X);
    alListenerfv(AL_ORIENTATION, ListenerOri);    
    alGetError();
}


CMySound::CMySound(unsigned int newBuffer, bool plooped, core::vector3df &pos) : 
        initialized(false), paused(true), looped(plooped), maxDistance(1000.0f), minDistance(1.0f), volume(1.0f),
        playbackSpeed(1.0f), buffer(newBuffer), isStopped(true), soundSourcePos(pos)
        
{
    alGenSources(1, &source);
    
    int err = alGetError();
    if (err != AL_NO_ERROR)
    {
#ifdef MY_SOUND_DEBUG
        printf("create sound failed: %d\n", err);
#endif
        return;
    }

#ifdef MY_SOUND_DEBUG
    printf("create sound %u\n", source);
#endif

    soundSourceVel = core::vector3df(0.f,0.f,0.f);
    
    initialized = true;
    alSourcei (source, AL_BUFFER,   buffer   );
    alSourcef (source, AL_PITCH,    playbackSpeed     );
    alSourcef (source, AL_GAIN,     volume     );
    alSourcefv(source, AL_POSITION, &soundSourcePos.X);
    alSourcefv(source, AL_VELOCITY, &soundSourceVel.X);
    alSourcei (source, AL_LOOPING,  looped     );
    alGetError();
}

CMySound::~CMySound()
{
    if (!this || !initialized) return;
    
    stop();
#ifdef MY_SOUND_DEBUG
    printf("destroy sound %u\n", source);
#endif
    alDeleteSources(1, &source);
    int err = alGetError();
    if (err != AL_NO_ERROR)
    {
#ifdef MY_SOUND_DEBUG
        printf("destroy sound failed: %d\n", err);
#endif
    }
}

void CMySound::setVolume(float newVolume)
{
    if (!this || !initialized) return;
    volume = newVolume;
    alSourcef (source, AL_GAIN,     volume     );
    alGetError();
}

float CMySound::getVolume() const
{
    if (!this || !initialized) return 0.f;
    return volume;
}

void CMySound::setPlaybackSpeed(float newPlaybackSpeed)
{
    if (!this || !initialized) return;
    playbackSpeed = newPlaybackSpeed;
    alSourcef (source, AL_PITCH,    playbackSpeed     );
    alGetError();
}

float CMySound::getPlaybackSpeed() const
{
    if (!this || !initialized) return 0.f;
    return playbackSpeed;
}

void CMySound::setIsLooped(bool newLoop)
{
    if (!this || !initialized) return;
    looped = newLoop;
    alSourcei (source, AL_LOOPING,  looped     );
    alGetError();
}

bool CMySound::isLooped() const
{
    if (!this || !initialized) return false;
    return looped;
}

void CMySound::setPosition(core::vector3df &newPos)
{
    if (!this || !initialized) return;
    soundSourcePos = newPos;
    alSourcefv(source, AL_POSITION, &soundSourcePos.X);
    alGetError();
}

void CMySound::setPosition(core::vector3df newPos)
{
    if (!this || !initialized) return;
    soundSourcePos = newPos;
    alSourcefv(source, AL_POSITION, &soundSourcePos.X);
    alGetError();
}

core::vector3df CMySound::getPosition() const
{
    if (!this || !initialized) return core::vector3df();
    return soundSourcePos;
}

void CMySound::setVelocity(core::vector3df &newVelocity)
{
    if (!this || !initialized) return;
    soundSourceVel = newVelocity;
    alSourcefv(source, AL_VELOCITY, &soundSourceVel.X);
    alGetError();
}

core::vector3df CMySound::getVelocity() const
{
    if (!this || !initialized) return core::vector3df();
    return soundSourceVel;
}

void CMySound::setMaxDistance(float newMaxDistance)
{
    if (!this || !initialized) return;
    maxDistance = newMaxDistance;
    alSourcef (source, AL_MAX_DISTANCE, maxDistance);
}

float CMySound::getMaxDistance() const
{
    if (!this || !initialized) return 0.f;
    return maxDistance;
}

void CMySound::setMinDistance(float newMinDistance)
{
    if (!this || !initialized) return;
    minDistance = newMinDistance;
    alSourcef (source, AL_REFERENCE_DISTANCE, minDistance);
}

float CMySound::getMinDistance() const
{
    if (!this || !initialized) return 0.f;
    return minDistance;
}


void CMySound::play()
{
    if (!this || !initialized) return;

#ifdef MY_SOUND_DEBUG
    printf("play sound %u\n", source);
#endif
    
    isStopped = false;
//    paused = false;
    alSourcePlay(source);    
    alGetError();
}

void CMySound::stop()
{
    if (!this || !initialized) return;

#ifdef MY_SOUND_DEBUG
    printf("stop sound %u\n", source);
#endif
    
    isStopped = true;
    alSourceStop(source);
    alGetError();
}

void CMySound::setIsPaused(bool newPaused)
{
    if (!this || !initialized) return;

#ifdef MY_SOUND_DEBUG
//    printf("pause sound %u %d -> %d\n", source, paused, newPaused);
#endif

    if (paused == newPaused) return;
    
    paused = newPaused;
    
    if (paused == false)
        alSourcePlay(source);    
    else
        alSourcePause(source);

    alGetError();
}

bool CMySound::isPaused() const
{
    if (!this || !initialized) return false;
    return paused;
}
