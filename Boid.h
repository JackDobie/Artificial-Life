#pragma once

#include "DrawableGameObject.h"
#include "Timer.h"

// default scales for the forces applied to boids
#define SEPARATIONSCALE_DEFAULT	1.5f
#define ALIGNMENTSCALE_DEFAULT	1.0f
#define COHESIONSCALE_DEFAULT	1.0f
#define FLEESCALE_DEFAULT		1.5f

#define SPEED_DEFAULT			140.0f
#define SPEED_SCARED			170.0f

class Predator;

class Boid : public DrawableGameObject
{
public:
	Boid();
	~Boid();

	XMFLOAT3*							GetDirection() { return &m_direction; }
	void								CheckIsOnScreenAndFix(const XMMATRIX&  view, const XMMATRIX&  proj);
	void								Update(float t, vecBoid* boidList, vector<Predator*> predatorList);

	bool								GetAlive() { return isAlive; }
	
	bool								GetTargeted() { return targeted; }
	void								SetTargeted(bool target) { targeted = target; }

protected:
	void								SetDirection(XMFLOAT3 direction);

	vecBoid								NearbyBoids(vecBoid* boidList);
	XMFLOAT3							CalculateSeparationVector(vecBoid* boidList);
	XMFLOAT3							CalculateAlignmentVector(vecBoid* boidList);
	XMFLOAT3							CalculateCohesionVector(vecBoid* boidList);
	XMFLOAT3							VecToNearbyBoids(vecBoid* boidList);
	XMFLOAT3							CalculateFleeVector(vector<Predator*> predatorList);
	void								CreateRandomDirection();

	XMFLOAT3							AddFloat3(XMFLOAT3& f1, XMFLOAT3& f2);
	XMFLOAT3							SubtractFloat3(XMFLOAT3& f1, XMFLOAT3& f2);
	XMFLOAT3							NormaliseFloat3(XMFLOAT3& f1);
	float								MagnitudeFloat3(XMFLOAT3& f1);
	XMFLOAT3							MultiplyFloat3(XMFLOAT3& f1, const float scalar);
	XMFLOAT3							DivideFloat3(XMFLOAT3& f1, const float scalar);

	bool								CompareAngle(XMFLOAT3 pos1, XMFLOAT3 pos2, float range);

	XMFLOAT3							m_direction;

	//unsigned int*						m_nearbyDrawables;

	float								separationScale = SEPARATIONSCALE_DEFAULT;
	float								alignmentScale = ALIGNMENTSCALE_DEFAULT;
	float								cohesionScale = COHESIONSCALE_DEFAULT;
	float								fleeScale = FLEESCALE_DEFAULT;
	float								fleeDistance = 150.0f;
	float								killDistance = 2.0f;
	bool								isAlive = true;

	float								speed = SPEED_DEFAULT;
	float								FOV = 60.0f;
	bool								scared = false;
	bool								spotPredator = false;

	Timer*								_timer;
private:
	bool								targeted = false; // used by predators to avoid multiple predators targeting the same boid
};

