#pragma once

#include "DrawableGameObject.h"

class Boid;

class Predator : public DrawableGameObject
{
public:
	Predator();
	~Predator();

	XMFLOAT3*							GetDirection() { return &m_direction; }
	void								CheckIsOnScreenAndFix(const XMMATRIX& view, const XMMATRIX& proj);
	void								Update(float t, vecBoid* boidList);

protected:
	void								SetDirection(XMFLOAT3 direction);

	//vecBoid								NearbyBoids(vecBoid* boidList);
	XMFLOAT3							VecToNearbyBoids(vecBoid* boidList);
	void								CreateRandomDirection();

	XMFLOAT3							AddFloat3(XMFLOAT3& f1, XMFLOAT3& f2);
	XMFLOAT3							SubtractFloat3(XMFLOAT3& f1, XMFLOAT3& f2);
	XMFLOAT3							NormaliseFloat3(XMFLOAT3& f1);
	float								MagnitudeFloat3(XMFLOAT3& f1);
	XMFLOAT3							MultiplyFloat3(XMFLOAT3& f1, const float scalar);
	XMFLOAT3							DivideFloat3(XMFLOAT3& f1, const float scalar);

	XMFLOAT3							m_direction;

	Boid*								targetedBoid = nullptr;
};