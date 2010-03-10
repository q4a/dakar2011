/* Copyright (c) <2009> <Newton Game Dynamics>
* 
* This software is provided 'as-is', without any express or implied
* warranty. In no event will the authors be held liable for any damages
* arising from the use of this software.
* 
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely
*/


// Vehicle.h: interface for the MutibodyVehicle class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CUSTOM_MULTIBODY_VEHICLE__INCLUDED_)
#define AFX_CUSTOM_MULTIBODY_VEHICLE__INCLUDED_

#include "NewtonCustomJoint.h"


class CustomMultiBodyVehicleAxleDifferencial;
#define MULTI_BODY_VEHICLE_MAX_TIRES	16
class CustomMultiBodyVehicleTire: public NewtonCustomJoint  
{
	public:
	CustomMultiBodyVehicleTire(
		const NewtonBody* hubBody, 
		NewtonBody* tire, 
		dFloat suspensionLength, dFloat springConst, dFloat damperConst, dFloat radio, float mass);

	~CustomMultiBodyVehicleTire(void);

	void GetInfo (NewtonJointRecord* info) const;

	dFloat GetSteerAngle () const;

	void SetSteerAngle (dFloat angle);

	void SetTorque (dFloat torque);

	void SetBrakeTorque (dFloat torque);

	void SetAngulaRollingDrag (dFloat angularDampingCoef);

	void ProjectTireMatrix();

	void SubmitConstraints (dFloat timestep, int threadIndex);

	dMatrix m_tireLocalMatrix;			// body1 is the tire 
	dMatrix m_chassisLocalMatrix;		// body0 is the chassis      
	dMatrix m_refChassisLocalMatrix;

	dFloat m_Ixx;
	dFloat m_radius;
	dFloat m_spring;
	dFloat m_damper;
	
	dFloat m_steeAngle;
	dFloat m_brakeToque;
	dFloat m_enginetorque;
	dFloat m_angularDragCoef;
	dFloat m_effectiveSpringMass;
	dFloat m_suspenstionSpan;
	
	// DAKAR
	bool m_tireIsOnAir;
	float m_mass;
	NewtonBody* tireBody;
};


class JOINTLIBRARY_API CustomMultiBodyVehicle: public NewtonCustomJoint  
{
	public:
	CustomMultiBodyVehicle (const dVector& frontDir, const dVector& upDir, const NewtonBody* carBody);
	virtual ~CustomMultiBodyVehicle(void);

	int AddSingleSuspensionTire (void* userData, const dVector& localPosition, 
								  dFloat mass, dFloat radius, dFloat with,
								  dFloat suspensionLength, dFloat springConst, dFloat springDamper);

	int AddSlipDifferencial (int leftTireIndex, int rightToreIndex, dFloat maxFriction);


	int GetTiresCount() const ;
	NewtonBody* GetTireBody(int tireIndex) const;
	CustomMultiBodyVehicleTire* GetTire (int index) const {return m_tires[index];}

	dFloat GetSpeed() const;
	virtual void ApplyTorque (dFloat torque);
	virtual void ApplySteering (dFloat angle);
	virtual void ApplyBrake (dFloat brakeTorque);

//	protected:
	void ApplyTireSteerAngle (int index, dFloat angle);
	void ApplyTireTorque (int index, dFloat angle);
	void ApplyTireBrake (int index, dFloat brakeTorque);
	void ApplyTireRollingDrag (int index, dFloat angularDampingCoef);

	dFloat GetSetTireSteerAngle (int index) const;
	
	virtual void SubmitConstraints (dFloat timestep, int threadIndex);
	virtual void GetInfo (NewtonJointRecord* info) const;

	int m_tiresCount;
	int m_diffencialCount;
	CustomMultiBodyVehicleTire* m_tires[MULTI_BODY_VEHICLE_MAX_TIRES];
	CustomMultiBodyVehicleAxleDifferencial* m_differencials[MULTI_BODY_VEHICLE_MAX_TIRES / 2];


	dMatrix m_localFrame;
};

#endif
