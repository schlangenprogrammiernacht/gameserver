#include "Snake.h"

const float_t Snake::DISTANCE_PER_STEP = 1.0;

Snake::Snake()
{
	std::shared_ptr<Segment> segment = std::make_shared<Segment>();
	m_segments.push_back(segment);
}

Snake::Snake(const Vector &startPos, std::size_t startLen)
{
	for(std::size_t i = 0; i < startLen; i++) {
		std::shared_ptr<Segment> segment = std::make_shared<Segment>();
		segment->pos = startPos;

		m_segments.push_back(segment);
	}
}

float_t Snake::maxRotationPerStep(void)
{
	// TODO: make this better?
	return 10.0 / (m_segments.size()/10.0 + 1);
}

Vector Snake::currentMovementVector(void)
{
	return m_segments[0]->pos - m_segments[1]->pos;
}

void Snake::move(float_t targetAngle, bool boost)
{
	// calculate delta angle
	Vector movementVector = currentMovementVector();
	float_t curAngle = movementVector.angle();
	float_t deltaAngle = targetAngle - curAngle;

	// normalize delta angle
	if(deltaAngle > 180) {
		deltaAngle -= 360;
	} else if(deltaAngle < -180) {
		deltaAngle += 360;
	}

	// limit rotation rate
	float_t maxDelta = maxRotationPerStep();
	if(deltaAngle > maxDelta) {
		deltaAngle = maxDelta;
	} else if(deltaAngle < -maxDelta) {
		deltaAngle = -maxDelta;
	}

	// ensure length of movement vector
	movementVector.normalize();
	movementVector *= DISTANCE_PER_STEP;

	std::size_t oldSize = m_segments.size();

	// create multiple segments while boosting
	std::size_t steps = 1;
	if(boost) {
		steps = BOOST_STEPS;
	}

	// create new segments at head
	for(std::size_t i = 0; i < steps; i++) {
		movementVector.rotate(deltaAngle);

		std::shared_ptr<Segment> segment = std::make_shared<Segment>();
		segment->pos = m_segments[0]->pos + movementVector;

		m_segments.push_front(segment);
	}

	// force size to previous size (removes end segments)
	m_segments.resize(oldSize);
}
