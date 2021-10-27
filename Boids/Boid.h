#pragma once

#include "DrawableGameObject.h"

// default scales for the forces applied to boids
#define SEPARATIONSCALE_DEFAULT	1.1f
#define ALIGNMENTSCALE_DEFAULT	2.0f
#define COHESIONSCALE_DEFAULT	1.0f
#define FLEESCALE_DEFAULT		10.0f

class Predator;

class Boid : public DrawableGameObject
{
public:
	Boid();
	~Boid();

	XMFLOAT3*							getDirection() { return &m_direction; }
	void								checkIsOnScreenAndFix(const XMMATRIX&  view, const XMMATRIX&  proj);
	void								update(float t, vecBoid* boidList, vector<Predator*> predatorList);

protected:
	void								setDirection(XMFLOAT3 direction);

	vecBoid								nearbyBoids(vecBoid* boidList);
	XMFLOAT3							calculateSeparationVector(vecBoid* boidList);
	XMFLOAT3							calculateAlignmentVector(vecBoid* boidList);
	XMFLOAT3							calculateCohesionVector(vecBoid* boidList);
	XMFLOAT3							vecToNearbyBoids(vecBoid* boidList);
	XMFLOAT3							calculateFleeVector(vector<Predator*> predatorList);
	void								createRandomDirection();

	XMFLOAT3							addFloat3(XMFLOAT3& f1, XMFLOAT3& f2);
	XMFLOAT3							subtractFloat3(XMFLOAT3& f1, XMFLOAT3& f2);
	XMFLOAT3							normaliseFloat3(XMFLOAT3& f1);
	float								magnitudeFloat3(XMFLOAT3& f1);
	XMFLOAT3							multiplyFloat3(XMFLOAT3& f1, const float scalar);
	XMFLOAT3							divideFloat3(XMFLOAT3& f1, const float scalar);

	XMFLOAT3							m_direction;
	//unsigned int*						m_nearbyDrawables;

	float								separationScale = SEPARATIONSCALE_DEFAULT;
	float								alignmentScale = ALIGNMENTSCALE_DEFAULT;
	float								cohesionScale = COHESIONSCALE_DEFAULT;
	float								fleeScale = FLEESCALE_DEFAULT;
	float								fleeDistance = 100.0f;
	float								killDistance = 2.0f;
};

