#include "Boid.h"
#include "Predator.h"
#include "Debug.h"

#define NEARBY_DISTANCE	50.0f // how far boids can see

Boid::Boid()
{
	m_scale = 1.0f;
	speed = SPEED_DEFAULT + (rand() % 100);
	FOV += (rand() % 225);
	fleeDistance += (rand() % 100);
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
	XMFLOAT3  vSeparation = CalculateSeparationVector(&nearBoids); // vector away from nearby boids
	XMFLOAT3  vAlignment = CalculateAlignmentVector(&nearBoids); // average direction of nearby boids
	XMFLOAT3  vCohesion = CalculateCohesionVector(&nearBoids); // vector towards average position of nearby boids
	XMFLOAT3  vFlee = CalculateFleeVector(predatorList); // vector away from nearby predators

	// multiply each vector by a scale to make some more important than others
	vSeparation = MultiplyFloat3(vSeparation, separationScale);
	vAlignment = MultiplyFloat3(vAlignment, alignmentScale);
	vCohesion = MultiplyFloat3(vCohesion, cohesionScale);
	vFlee = MultiplyFloat3(vFlee, fleeScale);

	// add all four together and normalise
	XMFLOAT3 forces = XMFLOAT3(0.0f, 0.0f, 0.0f);
	forces = AddFloat3(forces, vSeparation);
	forces = AddFloat3(forces, vAlignment);
	forces = AddFloat3(forces, vCohesion);
	forces = AddFloat3(forces, vFlee);
	m_direction = AddFloat3(m_direction, forces);
	if (MagnitudeFloat3(m_direction) != 0)
	{
		m_direction = NormaliseFloat3(m_direction);
	}
	else
	{
		m_direction = VecToNearbyBoids(boidList); // if no direction, go to the nearest boid

		if (MagnitudeFloat3(m_direction) == 0) // if still no direction (no nearby boids), create random direction
			CreateRandomDirection();
	}

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

	float desiredSeparation = 12.5f;
	XMFLOAT3 nearby = XMFLOAT3(0, 0, 0);
	int count = 0;

	for (Boid* b : *boidList)
	{
		// find the distance between boids
		XMFLOAT3 vB = *(b->getPosition());
		XMFLOAT3 vDiff = SubtractFloat3(m_position, vB);
		float l = MagnitudeFloat3(vDiff);

		// ignore self (distance of 0 could only be self)
		if (l < 0)
		{
			continue;
		}

		// only separate from boids in the desired distance
		if (l < desiredSeparation)
		{
			XMFLOAT3 dif = NormaliseFloat3(vDiff);
			dif = DivideFloat3(dif, l); // closer boids will have a greater weight
			nearby = AddFloat3(nearby, dif);

			count++;
		}
	}

	if (MagnitudeFloat3(nearby) > 0)
	{
		nearby = DivideFloat3(nearby, count);
		nearby = NormaliseFloat3(nearby);
		return nearby;
	}

	return m_direction;
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
	if (boidList->size() > 0)
	{
		nearby = DivideFloat3(nearby, boidList->size());

		return NormaliseFloat3(nearby); // return the normalised (average) direction of nearby drawables
	}
	return m_direction;
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
	if (boidList->size() > 0)
	{
		nearby = DivideFloat3(nearby, boidList->size()); // this is the avg position

		nearby = SubtractFloat3(nearby, m_position); // this gets the direction to the avg position

		return NormaliseFloat3(nearby); // nearby is the direction to where the other drawables are
	}
	return m_direction;
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

	// if there is not a nearby fish return current direction
	return m_direction;
}

XMFLOAT3 Boid::CalculateFleeVector(vector<Predator*> predatorList)
{
	if (predatorList.empty())
		return XMFLOAT3(0, 0, 0);

	XMFLOAT3 dir = XMFLOAT3(0, 0, 0);

	for (Predator* p : predatorList)
	{
		// calculate the distance to each predator and flee if too close
		XMFLOAT3 vP = *(p->getPosition());
		XMFLOAT3 vDiff = SubtractFloat3(m_position, vP);
		
		float l = MagnitudeFloat3(vDiff);
		if (l > killDistance)
		{
			spotPredator = false;
			if (l < fleeDistance)
			{
				XMFLOAT3 toPredator = SubtractFloat3(vP, m_position);
				if (CompareAngle(m_direction, toPredator, FOV))
				{
					spotPredator = true;
				}
			}

			if (spotPredator)
			{
				dir = AddFloat3(dir, vDiff);
			}
		}
		else
		{
			if (canDie)
				isAlive = false;
			else
				dir = AddFloat3(dir, vDiff);
		}
	}
	if(MagnitudeFloat3(dir) > 0)
		return dir;
	
	return m_direction;
}

bool Boid::CompareAngle(XMFLOAT3 pos1, XMFLOAT3 pos2, float range)
{
	// get angle in degrees from vectors
	float n1 = 270 - atan2(pos1.y, pos1.x) * 180 / XM_PI;
	float angle1 = fmod(n1, 360);

	float n2 = 270 - atan2(pos2.y, pos2.x) * 180 / XM_PI;
	float angle2 = fmod(n2, 360);

	float lower = angle1 - (range * 0.5f);
	float upper = angle1 + (range * 0.5f);

	// if upper and lower go past 0 or 360 loop around and then 
	if (lower < 0.0f)
	{
		lower += 360.0f;
	}
	if (upper > 360.0f)
	{
		upper -= 360.0f;
	}

	// check if angle in range
	bool inRange = false;
	if (lower <= angle2 && angle2 <= upper)
	{
		inRange = true;
	}
	else if (upper - lower <= 0.0f)
	{
		// either upper or lower have looped around
		if (lower <= angle2 && angle2 <= 360.0f)
		{
			// angle between lower and 360
			inRange = true;
		}
		else if (0.0f <= angle2 && angle2 <= upper)
		{
			// angle between 0 and upper
			inRange = true;
		}
	}

	if (inRange)
	{
		/*Debug::Print("Lower: " + to_string(lower));
		Debug::Print("Upper: " + to_string(upper));
		Debug::Print("Angle: " + to_string(angle2));*/
		return true;
	}

	/*Debug::Print("Lower: " + to_string(lower));
	Debug::Print("Upper: " + to_string(upper));
	Debug::Print("Angle: " + to_string(angle2));*/

	// angle not in range - return false
	return false;
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