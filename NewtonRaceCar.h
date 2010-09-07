/****************************************************************
*                                                               *
*    Name: NewtonRaceCar.h                                      *
*                                                               *
*    Creator: Balazs Tuska                                      *
*                                                               *
*    Description:                                               *
*       This file contains the code of the car handling.        *
*       The car can be the user car or a multiplayer car.       *
*                                                               *
****************************************************************/

// NewtonRaceCar.h: interface for the RaceCar class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWTONRACECAR_H__96CF72A9_A2BD_4A11_9F6D_40DD9A3CCC12__INCLUDED_)
#define AFX_NEWTONRACECAR_H__96CF72A9_A2BD_4A11_9F6D_40DD9A3CCC12__INCLUDED_


#define OLD_CDGRCC
#include "CustomDGRayCastCar.h"

#include "irrlicht.h"
// Irrlicht Namespaces
using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

#ifdef USE_MY_SOUNDENGINE
 #include "mySound.h"
#else
 #include <irrKlang.h>
 using namespace irrklang;
#endif

#include "MyList.h"
//#include "gameplay.h"

#define view_max 3
#define view_multi 4


#define TIRE_COLLITION_ID	0x100
#define CHASIS_COLLITION_ID	0x200
#define MAX_SMOKES 50
#define GRAVITY    gravity
//-40.0f

#define TYRE_RADIUS_MIN 0.7f
#define TYRE_RADIUS_MAX 1.0f
#define TYRE_RADIUS_MULTI_DEFAULT 0.5f
#define TYRE_RADIUS_GET_FROM_MULTI(x) (TYRE_RADIUS_MIN+(TYRE_RADIUS_MAX-TYRE_RADIUS_MIN)*(x))

#define TYRE_MASS_MIN 0.7f
#define TYRE_MASS_MAX 1.3f
#define TYRE_MASS_MULTI_DEFAULT 0.5f
#define TYRE_MASS_GET_FROM_MULTI(x) (TYRE_MASS_MIN+(TYRE_MASS_MAX-TYRE_MASS_MIN)*(x))

#define TYRE_FRICTION_MIN 0.8f
#define TYRE_FRICTION_MAX 1.2f
#define TYRE_FRICTION_MULTI_DEFAULT 0.5f
#define TYRE_FRICTION_GET_FROM_MULTI(x) (TYRE_FRICTION_MIN+(TYRE_FRICTION_MAX-TYRE_FRICTION_MIN)*(x))


#define TYRE_PRESSURE_MIN 0.f
#define TYRE_PRESSURE_MAX 8.f
#define TYRE_PRESSURE_MULTI_DEFAULT 0.5f
#define TYRE_PRESSURE_GET_FROM_MULTI(x) (TYRE_PRESSURE_MIN+(TYRE_PRESSURE_MAX-TYRE_PRESSURE_MIN)*(x))


#define SUSPENSION_SPRING_MIN 0.4f
#define SUSPENSION_SPRING_MAX 2.5f
#define SUSPENSION_SPRING_MULTI_DEFAULT 0.28572f
#define SUSPENSION_SPRING_GET_FROM_MULTI(x) (SUSPENSION_SPRING_MIN+(SUSPENSION_SPRING_MAX-SUSPENSION_SPRING_MIN)*(x))

#define SUSPENSION_DAMPER_MIN 0.5f
#define SUSPENSION_DAMPER_MAX 2.0f
#define SUSPENSION_DAMPER_MULTI_DEFAULT 0.33334f
#define SUSPENSION_DAMPER_GET_FROM_MULTI(x) (SUSPENSION_DAMPER_MIN+(SUSPENSION_DAMPER_MAX-SUSPENSION_DAMPER_MIN)*(x))

#define SUSPENSION_LENGTH_MIN 0.5f
#define SUSPENSION_LENGTH_MAX 2.0f
#define SUSPENSION_LENGTH_MULTI_DEFAULT 0.33334f
#define SUSPENSION_LENGTH_GET_FROM_MULTI(x) (SUSPENSION_LENGTH_MIN+(SUSPENSION_LENGTH_MAX-SUSPENSION_LENGTH_MIN)*(x))

class OffsetObject;

class NewtonRaceCar
{
	struct RaceCarTire
	{
		RaceCarTire(ISceneManager* smgr, IVideoDriver* driver, NewtonRaceCar *root, int witchTire, FILE* f);
		virtual ~RaceCarTire();
		
		void activate(bool p_useOffset);
		void deactivate();
        
		// witchTire: 1 fr, 2 fl, 3 rr 4 rl
		//virtual void SetTirePhysics(const NewtonJoint* vehicle, void* tireId);
		void setMatrix(matrix4& newMatrix);
		void setMatrixWithNB(matrix4& newMatrix);
        void disconnect(const vector3df& force, NewtonWorld* nWorld);

        static void DestroyTire (const NewtonBody* body);
        static void SetTransformTire (const NewtonBody* body, const float* matrixPtr, int threadIndex);

		float m_width;
		float m_radius;
		matrix4 m_matrix;
		IAnimatedMeshSceneNode* m_tireNode;
        IShadowVolumeSceneNode* shadowNode;
        int tire_num;
	    matrix4 m_trafo;
        bool connected;
        float connectionStrength;
        NewtonBody* m_newtonBody;
        float tyre_mass;
        float suspension_length;
        NewtonRaceCar *root;
        vector3df tirePosition;
        float friction;
        float suspension_spring;
        float suspension_damper;
        bool hitRoad;
        OffsetObject* offsetObject;
        float suspension_length_orig;
        float friction_orig;
	};

    struct Smoke
    {
        Smoke(ISceneManager* smgr, IVideoDriver* driver, const float pspeed, const vector3df &pos, float offset, float p_waterHeight);
        void renew(ISceneManager* smgr, IVideoDriver* driver, const float pspeed, const vector3df &pos, float offset, float p_waterHeight);
        
        scene::IBillboardSceneNode* node;
        float speed;
        int animePhase;
        OffsetObject* offsetObject;
   };

public:
	NewtonRaceCar(IrrlichtDevice* pdevice, 
                  ISceneManager* psmgr, IVideoDriver* pdriver,
                  //const c8* fileName,
                  NewtonWorld* pnWorld,
#ifdef USE_MY_SOUNDENGINE
                  CMySoundEngine* psoundEngine,
#else
                  irrklang::ISoundEngine* psoundEngine,
#endif
                  const int pcarType,
                  const char* fileName,
                  bool p_autoGear = true);

    void activate(
            const vector3df& loc, const vector3df& rot,
            const c8* groundSoundName,
            const c8* puffSoundName,
            const c8* skidSoundName,
            const float pfriction_multi,
            scene::ISceneNode* skydome,
            video::ITexture* shadowMap,
            const float p_waterHeight,
            bool p_useOffset = true,
            const int savedCarDirt = 0,
            float pressure_multi = TYRE_PRESSURE_MULTI_DEFAULT,
            float ss_multi = SUSPENSION_SPRING_MULTI_DEFAULT,
            float sd_multi = SUSPENSION_DAMPER_MULTI_DEFAULT,
            float sl_multi = SUSPENSION_LENGTH_MULTI_DEFAULT
            );
    void deactivate();
                  
	virtual ~NewtonRaceCar();
    
    void OnRegisterSceneNode();
    void render();

	virtual void setSteering(float value);
	virtual void setSteeringKb(float value);
	virtual void setTireTorque(float torque);
	virtual void setHandBrakes(float value);
	virtual void applySteering(float value);
	virtual void applyTireTorque(float rvalue);
	virtual void applyHandBrakes(float value);
	virtual void setControl();

	float getSpeed() const;
	void setMatrixWithNB(matrix4& newMatrix);
	void setMatrixWithNB2(vector3df& newPos, vector3df& newRot);
#ifdef USE_MY_SOUNDENGINE
	void playSound(CMySound* sound, const vector3df& pos);
#else
	void playSound(irrklang::ISound* sound, const vector3df& pos);
#endif
	void pause();
	void resume();
	matrix4& getMatrix();
	float& getSteer() {return m_steer;}
	float& getSteerKb() {return m_steer_kb;}
	float& getSteerRate() {return m_steer_rate;}
	float& getTorque() {return m_torque;}
	float& getTorqueReal() {return m_torqueReal;}
	float& getBrake() {return m_brakes;}
	float& getTorqueMulti() {return torque_multi;}
	float& getClutch() {return m_clutch;}
	void setClutch(float value) {m_clutch=value;}

	NewtonBody* GetRigidBody() const;
    
    void reset(const core::vector3df& newpos);
    float getAngle();
    
    int getCarType() {return m_carType;}
    
    void updateGear(int pgear = -2, bool needClutch = false);
    int getMaxGear() {return gearLimits.length();}
    int getGear() {return gear;}
    
    void startEngine();
    void stopEngine();
    bool getEngineIsGoing() {return engineGoing;}
    
    float& getEngineRotate() {return engineRotate;}

    ISceneNode* getSceneNode() {return m_node;}
    ISceneNode* getTyreSceneNode(int num) {return m_tires[num]->m_tireNode;}
    
    int getLight() {return cLight;}
    void switchLight();

    void switchBrake(bool brake);
    
    void updateDirt();
    int getDirt() {return current_car_dirt;}
    
    scene::ILightSceneNode* getLightNode() {return clnode;}
    
    void collide(const vector3df &point, const vector3df &direction,
                 float speed, float demageMultip = 1.0f, float amortMultip = 1.0f);
    
    float getDemage() {return demage;}
    float getDemagePer() //{return 100.f - (demage * 100.f);}
    {
        float ts = m_tires.size()?(float)m_tires.size():2.f;
        float ret = demage * ts;
        for (int i = 0; i < m_tires.size(); i++)
        {
            ret += m_tires[i]->connectionStrength;
        }
        ret = 100.f - (ret * (50.f/ts));
        return ret;        
    }
    
    void repair();
    
    void loseTyre(int num);
    bool isTyreConnected(int num) { if (num >= 0 && num < 4) {return m_tires[num]->connected;} else return false;}
    float getTyreDemage(int num) { if (num >= 0 && num < 4) {return m_tires[num]->connectionStrength;} else return 1.f;}
    
    bool getAutoGear() {return autoGear;}
    void setAutoGear(bool p_autoGear) {autoGear = p_autoGear;}
    
    void setNameText(ITextSceneNode* p_nameText) {nameText = p_nameText;}
    
    float getFriction(int num)
    {
        if (num >= 0 && num < m_tires.size())
        {
            return m_vehicleJoint->GetTire(m_tires[num]->tire_num).m_groundFriction;
        }
        return 0.f;
    }

    unsigned int getHitBody(int num)
    {
        if (num >= 0 && num < m_tires.size())
        {
            return (unsigned int)m_vehicleJoint->GetTire(m_tires[num]->tire_num).m_HitBody;
        }
        return (unsigned int)0;
    }

    unsigned int getHitBodyID(int num)
    {
        if (num >= 0 && num < m_tires.size())
        {
            NewtonBody* hb = m_vehicleJoint->GetTire(m_tires[num]->tire_num).m_HitBody;
            return hb?NewtonBodyGetMaterialGroupID(hb):(unsigned int)-1;
        }
        return (unsigned int)0;
    }
    
    matrix4& getViewPos(unsigned int num)
    {
        if (num >= view_max*view_multi)
        {
            num = 0;
        }
        return viewpos[num];
    }

    matrix4& getViewDest(unsigned int num)
    {
        if (num >= view_max*view_multi)
        {
            num = 0;
        }
        return viewdest[num];
    }

    void setPressure(float pressure_multi = TYRE_PRESSURE_MULTI_DEFAULT);
    void setSuspensionSpring(float ss_multi = SUSPENSION_SPRING_MULTI_DEFAULT);
    void setSuspensionDamper(float sd_multi = SUSPENSION_DAMPER_MULTI_DEFAULT);
    void setSuspensionLength(float sl_multi = SUSPENSION_LENGTH_MULTI_DEFAULT);

private:
	//void Render() const;
	static void DestroyVehicle (const NewtonBody* body);
	static void ApplyGravityForce (const NewtonBody* body, float timestep, int threadIndex);
	static void SetTransform (const NewtonBody* body, const float* matrixPtr, int threadIndex);
	static void offsetObjectCallback(void* userData, const irr::core::vector3df& newPos);

	CustomDGRayCastCar* GetJoint() const;

	void setMatrix(matrix4& newMatrix);
	
	void addSmoke(const float speed, const vector3df &pos, float offset);
	void updateSmoke();
	
	IrrlichtDevice* device;
    ISceneManager* smgr;
    IVideoDriver* driver;
	NewtonWorld* nWorld;
#ifdef USE_MY_SOUNDENGINE
    CMySoundEngine* soundEngine;
    CMySound* engineSound;
    CMySound* groundSound;
    CMySound* puffSound;
    CMySound* skidSound;
#else
    irrklang::ISoundEngine* soundEngine;
    irrklang::ISound* engineSound;
    irrklang::ISound* groundSound;
    irrklang::ISound* puffSound;
    irrklang::ISound* skidSound;
#endif

	NewtonBody* m_vehicleBody;
	CustomDGRayCastCar* m_vehicleJoint;
	Smoke** smokes;
//public:
	matrix4 m_matrix;
	matrix4 m_trafo;
    ISceneNode* m_node;
    IShadowVolumeSceneNode* shadowNode;
    IAnimatedMeshSceneNode* bb_node;
    // this times build on the setTransform count, increase in every 16 ms (maybe more than once)
    u32 lastOnGroundTime;
    u32 lastNotOnGroundTime;
    u32 setTransformCount;
    u32 lastUpdateGearTime;
    u32 changeGearTime;
    
    // time based timers
    u32 lastSoundUpdateTime;
    bool lastOnGround;
    

	float m_steer;
	float m_steer_rate;
	float m_steer_kb;
	float m_torque;
	float m_torqueReal;
	float m_brakes;
	float m_clutch;
	int m_debug;
	float torque_multi;
    float friction_multi;
    int m_carType;
    
    int gear;
    struct gl
    {
        float low;
        float high;
        float max_torque;
        float max_torque_rate;
    };
    CMyList<struct gl> gearLimits;
    bool engineGoing;
    float engineRotate;
    int cLight;
    int current_car_dirt;
    
    video::ITexture* carTexs[4];
    int carTex;
    bool doBrake;
    bool prevBrake;
    scene::ILightSceneNode* clnode;
    char* omb;
    float demage;
	//int vId;
	bool autoGear;

	NewtonCollision* vehicleCollision;
	NewtonCollision* vehicleCollisionBox;
	int num_of_tires;
	core::array<RaceCarTire*> m_tires;
	core::array<RaceCarTire*> steer_tires;
	core::array<RaceCarTire*> torque_tires;
	core::array<RaceCarTire*> smoke_tires;
	core::array<RaceCarTire*> hb_tires;
	
	float mass;
	float engine_center_of_mass;
	float center_of_mass;
	float chassis_rotation_limit;
	float chassis_rotation_rate;
	float max_brake_force;
	float max_steer_angle;
	float max_steer_rate;
	float engine_steer_div;
	
	ITextSceneNode* nameText; // the name of the competitor
	float waterHeight;
    OffsetObject* offsetObject;
    matrix4 viewdest[view_max*view_multi];
    matrix4 viewpos[view_max*view_multi];
    
};

#endif // !defined(AFX_NEWTONRACECAR_H__96CF72A9_A2BD_4A11_9F6D_40DD9A3CCC12__INCLUDED_)
