#include "Predator.h"
#include "Boid.h"

Predator::Predator()
{
	m_scale = 3.0f;
	CreateRandomDirection();
}

Predator::~Predator()
{
	if (targetedBoid != nullptr) targetedBoid = nullptr; delete targetedBoid;
}

void Predator::CreateRandomDirection()
{
	float x = (float)(rand() % 10);
	x -= 5;
	float y = (float)(rand() % 10);
	y -= 5;
	float z = 0;
	SetDirection(XMFLOAT3(x, y, z));
}

void Predator::SetDirection(XMFLOAT3 direction)
{
	XMVECTOR v = XMLoadFloat3(&direction);
	v = XMVector3Normalize(v);
	XMStoreFloat3(&m_direction, v);
}

void Predator::Update(float t, vecBoid* boidList)
{
	XMFLOAT3 nearbyBoidsVec = VecToNearbyBoids(boidList);
	m_direction = AddFloat3(m_direction, nearbyBoidsVec);
	//m_direction = VecToNearbyBoids(boidList);

	if (MagnitudeFloat3(m_direction) != 0)
	{
		m_direction = NormaliseFloat3(m_direction);
	}
	else
	{
		CreateRandomDirection(); // if no direction, make one
	}

	XMFLOAT3 dir = MultiplyFloat3(m_direction, t * speed);
	m_position = AddFloat3(m_position, dir);

	m_position.z = 0;
	m_direction.z = 0;

	DrawableGameObject::update(t);
}

XMFLOAT3 Predator::VecToNearbyBoids(vecBoid* boidList)
{
	if (boidList == nullptr)
		return XMFLOAT3(0, 0, 0);

	// work out which is nearest fish, and calculate a vector towards that
	Boid* nearest = nullptr;
	XMFLOAT3 directionNearest;
	float shortestDistance = FLT_MAX;

	for (Boid* b : *boidList)
	{
		if (!b->GetTargeted() || b == targetedBoid)
		{
			// calculate the distance to each boid and find the shortest
			XMFLOAT3 vB = *(b->getPosition());
			XMFLOAT3 vDiff = SubtractFloat3(m_position, vB);
			float l = MagnitudeFloat3(vDiff);
			if (l < shortestDistance)
			{
				shortestDistance = l;
				nearest = b;
			}
		}
	}

	if (nearest != nullptr)
	{
		if(targetedBoid != nullptr)
			targetedBoid->SetTargeted(false);

		targetedBoid = nearest;
		targetedBoid->SetTargeted(true);

		// get the direction from nearest boid to current boid
		directionNearest = SubtractFloat3(*nearest->getPosition(), m_position);
		return NormaliseFloat3(directionNearest);
	}

	// if there is not a nearby fish return 0
	return XMFLOAT3(0, 0, 0);
}

XMFLOAT3 Predator::AddFloat3(XMFLOAT3& f1, XMFLOAT3& f2)
{
	XMFLOAT3 out;
	out.x = f1.x + f2.x;
	out.y = f1.y + f2.y;
	out.z = f1.z + f2.z;

	return out;
}

XMFLOAT3 Predator::SubtractFloat3(XMFLOAT3& f1, XMFLOAT3& f2)
{
	XMFLOAT3 out;
	out.x = f1.x - f2.x;
	out.y = f1.y - f2.y;
	out.z = f1.z - f2.z;

	return out;
}

XMFLOAT3 Predator::MultiplyFloat3(XMFLOAT3& f1, const float scalar)
{
	XMFLOAT3 out;
	out.x = f1.x * scalar;
	out.y = f1.y * scalar;
	out.z = f1.z * scalar;

	return out;
}

XMFLOAT3 Predator::DivideFloat3(XMFLOAT3& f1, const float scalar)
{
	XMFLOAT3 out;
	out.x = f1.x / scalar;
	out.y = f1.y / scalar;
	out.z = f1.z / scalar;

	return out;
}

float Predator::MagnitudeFloat3(XMFLOAT3& f1)
{
	return sqrt((f1.x * f1.x) + (f1.y * f1.y) + (f1.z * f1.z));
}

XMFLOAT3 Predator::NormaliseFloat3(XMFLOAT3& f1)
{
	float length = sqrt((f1.x * f1.x) + (f1.y * f1.y) + (f1.z * f1.z));

	f1.x /= length;
	f1.y /= length;
	f1.z /= length;

	return f1;
}

void Predator::CheckIsOnScreenAndFix(const XMMATRIX& view, const XMMATRIX& proj)
{
	XMFLOAT4 v4;
	v4.x = m_position.x;
	v4.y = m_position.y;
	v4.z = m_position.z;
	v4.w = 1.0f;

	XMVECTOR vScreenSpace = XMLoadFloat4(&v4);
	XMVECTOR vScreenSpace2 = XMVector4Transform(vScreenSpace, view);
	XMVECTOR vScreenSpace3 = XMVector4Transform(vScreenSpace2, proj);

	XMFLOAT4 v;
	XMStoreFloat4(&v, vScreenSpace3);
	v.x /= v.w;
	v.y /= v.w;
	v.z /= v.w;
	v.w /= v.w;

	float fOffset = 10; // a suitable distance to rectify position within clip space
	if (v.x < -1 || v.x > 1 || v.y < -1 || v.y > 1)
	{
		if (v.x < -1 || v.x > 1) {
			v4.x = -v4.x + (fOffset * v.x);
		}
		else if (v.y < -1 || v.y > 1) {
			v4.y = -v4.y + (fOffset * v.y);
		}
	}

	m_position.x = v4.x;
	m_position.y = v4.y;
	m_position.z = v4.z;

	return;
}