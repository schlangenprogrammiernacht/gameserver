#include "config.h"

#include "Field.h"

#include "Snake.h"

Snake::Snake(Field *field)
	: m_field(field), m_mass(1.0f)
{
	std::shared_ptr<Segment> segment = std::make_shared<Segment>();
	m_segments.push_back(segment);

	ensureLengthMatchesMass();
}

Snake::Snake(Field *field, const Vector &startPos, float_t start_mass)
	: m_field(field), m_mass(start_mass)
{
	// create the first segment manually
	std::shared_ptr<Segment> segment = std::make_shared<Segment>();
	segment->pos = startPos;

	m_segments.push_back(segment);

	// create the other segments
	ensureLengthMatchesMass();
}

void Snake::ensureLengthMatchesMass(void)
{
	std::size_t curLen = m_segments.size();
	std::size_t targetLen = static_cast<std::size_t>(m_mass + 0.5);

	// ensure there are at least 2 segments to define movement direction
	if(targetLen < 2) {
		targetLen = 2;
	}

	if(curLen < targetLen) {
		// segments have to be added:
		// repeat the last segment until the new target length is reached
		const std::shared_ptr<Segment> refSegment = m_segments[curLen-1];
		for(std::size_t i = 0; i < (targetLen - curLen); i++) {
			std::shared_ptr<Segment> segment = std::make_shared<Segment>();
			*segment = *refSegment;

			m_segments.push_back(segment);
		}
	} else if(curLen > targetLen) {
		// segments must be removed
		m_segments.resize(targetLen);
	}
}

float_t Snake::maxRotationPerStep(void)
{
	// TODO: make this better?
	return 10.0 / (m_mass/10.0 + 1);
}

Vector Snake::currentMovementVector(void)
{
	Vector head = m_segments[0]->pos;
	Vector next = m_field->unwrapCoords(m_segments[1]->pos, head);

	if(head == next) {
		// movement vector undefined
		return Vector(1, 0);
	} else {
		return head - next;
	}
}

void Snake::consume(const std::shared_ptr<Food>& food)
{
	m_mass += food->getValue();
	ensureLengthMatchesMass();
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
	movementVector *= config::SNAKE_DISTANCE_PER_STEP;

	std::size_t oldSize = m_segments.size();

	// create multiple segments while boosting
	std::size_t steps = 1;
	if(boost) {
		steps = config::SNAKE_BOOST_STEPS;
	}

	// create new segments at head
	for(std::size_t i = 0; i < steps; i++) {
		movementVector.rotate(deltaAngle * M_PI / 180);

		std::shared_ptr<Segment> segment = std::make_shared<Segment>();
		segment->pos = m_field->wrapCoords(m_segments[0]->pos + movementVector);

		m_segments.push_front(segment);
	}

	// force size to previous size (removes end segments)
	m_segments.resize(oldSize);
}

const Snake::SegmentList& Snake::getSegments(void)
{
	return m_segments;
}

const Vector& Snake::getHeadPosition(void)
{
	return m_segments[0]->pos;
}
