#include "Boid.h"
#include "Predator.h"

#define NEARBY_DISTANCE	20.0f // how far boids can see

Boid::Boid()
{
	m_scale = 1.0f;
	CreateRandomDirection();
}

Boid::~Boid()
{
}

void Boid::CreateRandomDirection()
{
	float x = (float)(rand() % 10);
	x -= 5;
	float y = (float)(rand() % 10);
	y -= 5;
	float z = 0;
	SetDirection(XMFLOAT3(x, y, z));
}

void Boid::SetDirection(XMFLOAT3 direction)
{
	XMVECTOR v = XMLoadFloat3(&direction);
	v = XMVector3Normalize(v);
	XMStoreFloat3(&m_direction, v);
}

void Boid::Update(float t, vecBoid* boidList, vector<Predator*> predatorList)
{
	// create a list of nearby boids
	vecBoid nearBoids = NearbyBoids(boidList);

	// NOTE these functions should always return a normalised vector
	XMFLOAT3  vSeparation = CalculateSeparationVector(&nearBoids); // vector away from nearest boid
	XMFLOAT3  vAlignment = CalculateAlignmentVector(&nearBoids); // average direction of nearby boids
	XMFLOAT3  vCohesion = CalculateCohesionVector(&nearBoids); // vector towards average position of nearby boids
	XMFLOAT3  vFlee = CalculateFleeVector(predatorList); // vector away from nearby predators

	// multiply each vector by a scale to make some more important than others
	vSeparation = MultiplyFloat3(vSeparation, separationScale);
	vAlignment = MultiplyFloat3(vAlignment, alignmentScale);
	vCohesion = MultiplyFloat3(vCohesion, cohesionScale);
	vFlee = MultiplyFloat3(vFlee, fleeScale);

	// add all four together and normalise
	m_direction = AddFloat3(m_direction, vSeparation);
	m_direction = AddFloat3(m_direction, vAlignment);
	m_direction = AddFloat3(m_direction, vCohesion);
	m_direction = AddFloat3(m_direction, vFlee);
	if (MagnitudeFloat3(m_direction) != 0)
	{
		m_direction = NormaliseFloat3(m_direction);
	}
	else
	{
		m_direction = VecToNearbyBoids(boidList); // if no direction, go to the nearest boid
	}

	float speed = SPEED_DEFAULT;
	XMFLOAT3 dir = MultiplyFloat3(m_direction, t * speed);
	m_position = AddFloat3(m_position, dir);

	m_position.z = 0;
	m_direction.z = 0;

	DrawableGameObject::update(t);
}

XMFLOAT3 Boid::CalculateSeparationVector(vecBoid* boidList)
{
	if (boidList == nullptr)
		return XMFLOAT3(0, 0, 0);

	// work out which is nearest fish, and calculate a vector away from that
	Boid* nearest = nullptr;
	XMFLOAT3 directionNearest;
	float shortestDistance = FLT_MAX;

	for (Boid* b : *boidList)
	{
		// ignore self
		if (b == this)
			continue;

		if (nearest == nullptr)
		{
			nearest = b;
		}
		else
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
		// get the direction from nearest boid to current boid
		directionNearest = SubtractFloat3(m_position, *nearest->getPosition());
		directionNearest = NormaliseFloat3(directionNearest);
		if (shortestDistance < 2.0f)
		{
			separationScale = 10.0f;
		}
		else
		{
			separationScale = SEPARATIONSCALE_DEFAULT;
		}
		return directionNearest;
	}

	// if there is not a nearby fish - simply return the current direction. 
	return NormaliseFloat3(m_direction);
}

XMFLOAT3 Boid::CalculateAlignmentVector(vecBoid* boidList)
{
	XMFLOAT3 nearby = XMFLOAT3(0, 0, 0);
	if (boidList == nullptr)
		return nearby;

	for (Boid* b : *boidList)
	{
		nearby = AddFloat3(*b->GetDirection(), nearby);
	}
	nearby = DivideFloat3(nearby, boidList->size());

	return NormaliseFloat3(nearby); // return the normalised (average) direction of nearby drawables
}

XMFLOAT3 Boid::CalculateCohesionVector(vecBoid* boidList)
{
	XMFLOAT3 nearby = XMFLOAT3(0, 0, 0);

	if (boidList == nullptr)
		return nearby;

	// calculate average position of nearby
	for (Boid* boid : *boidList) 
	{
		nearby = AddFloat3(*boid->getPosition(), nearby);
	}
	nearby = DivideFloat3(nearby, boidList->size()); // this is the avg position

	nearby = SubtractFloat3(nearby, m_position); // this gets the direction to the avg position

	return NormaliseFloat3(nearby); // nearby is the direction to where the other drawables are
}

XMFLOAT3 Boid::VecToNearbyBoids(vecBoid* boidList)
{
	if (boidList == nullptr)
		return XMFLOAT3(0, 0, 0);

	// work out which is nearest fish, and calculate a vector away from that
	Boid* nearest = nullptr;
	XMFLOAT3 directionNearest;
	float shortestDistance = FLT_MAX;

	for (Boid* b : *boidList)
	{
		// ignore self
		if (b == this)
			continue;

		if (nearest == nullptr)
		{
			nearest = b;
		}
		else
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
		// get the direction from nearest boid to current boid
		directionNearest = SubtractFloat3(*nearest->getPosition(), m_position);
		return NormaliseFloat3(directionNearest);
	}

	// if there is not a nearby fish return 0
	return XMFLOAT3(0, 0, 0);
}

XMFLOAT3 Boid::CalculateFleeVector(vector<Predator*> predatorList)
{
	if (predatorList.empty())
		return XMFLOAT3(0, 0, 0);

	XMFLOAT3 dir = XMFLOAT3(0, 0, 0);

	for (Predator* p : predatorList)
	{
		// calculate the distance to each predator and find the shortest
		XMFLOAT3 vP = *(p->getPosition());
		XMFLOAT3 vDiff = SubtractFloat3(m_position, vP);
		float angle = GetAngle(m_direction, *p->GetDirection());
		float l = MagnitudeFloat3(vDiff);
		if (l < killDistance)
		{
			isAlive = false;
		}
		else if (l < fleeDistance)
		{
			dir = AddFloat3(dir, vDiff);
		}
	}

	return dir;
}

// use but don't alter the methods below
XMFLOAT3 Boid::AddFloat3(XMFLOAT3& f1, XMFLOAT3& f2)
{
	XMFLOAT3 out;
	out.x = f1.x + f2.x;
	out.y = f1.y + f2.y;
	out.z = f1.z + f2.z;

	return out;
}

XMFLOAT3 Boid::SubtractFloat3(XMFLOAT3& f1, XMFLOAT3& f2)
{
	XMFLOAT3 out;
	out.x = f1.x - f2.x;
	out.y = f1.y - f2.y;
	out.z = f1.z - f2.z;

	return out;
}

XMFLOAT3 Boid::MultiplyFloat3(XMFLOAT3& f1, const float scalar)
{
	XMFLOAT3 out;
	out.x = f1.x * scalar;
	out.y = f1.y * scalar;
	out.z = f1.z * scalar;

	return out;
}

XMFLOAT3 Boid::DivideFloat3(XMFLOAT3& f1, const float scalar)
{
	XMFLOAT3 out;
	out.x = f1.x / scalar;
	out.y = f1.y / scalar;
	out.z = f1.z / scalar;

	return out;
}

float Boid::GetAngle(XMFLOAT3 pos1, XMFLOAT3 pos2)
{
	//get angle between pos1 and pos2
	float angle = atan2(pos2.z - pos1.z, pos2.x - pos1.x);
	return angle;
}

float Boid::MagnitudeFloat3(XMFLOAT3& f1)
{
	return sqrt((f1.x * f1.x) + (f1.y * f1.y) + (f1.z * f1.z));
}

XMFLOAT3 Boid::NormaliseFloat3(XMFLOAT3& f1)
{
	float length = sqrt((f1.x * f1.x) + (f1.y * f1.y) + (f1.z * f1.z));

	f1.x /= length;
	f1.y /= length;
	f1.z /= length;

	return f1;
}

vecBoid Boid::NearbyBoids(vecBoid* boidList)
{
	vecBoid nearBoids;
	if (boidList->size() == 0)
		return nearBoids;

	for (Boid* boid : *boidList) {
		// ignore self
		if (boid == this)
			continue;

		// get the distance between the two
		XMFLOAT3 vB = *(boid->getPosition());
		XMFLOAT3 vDiff = SubtractFloat3(m_position, vB);
		float l = MagnitudeFloat3(vDiff);
		if (l < NEARBY_DISTANCE) {
			nearBoids.push_back(boid);
		}
	}

	return nearBoids;
}

void Boid::CheckIsOnScreenAndFix(const XMMATRIX&  view, const XMMATRIX&  proj)
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

		// throw a bit of randomness into the mix
		//createRandomDirection();
	}

	// method 1 - appear on the other side
	m_position.x = v4.x;
	m_position.y = v4.y;
	m_position.z = v4.z;


	// method2 - bounce off sides and head to centre
	//if (v.x < -1 || v.x > 1 || v.y < -1 || v.y > 1)
	//{
	//	m_direction = multiplyFloat3(m_direction, -1);
	//	m_direction = normaliseFloat3(m_direction);
	//}

	return;
}