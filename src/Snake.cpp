#include "config.h"

#include "Field.h"

#include "Snake.h"

Snake::Snake(Field *field)
	: m_field(field), m_mass(1.0f), m_heading(0.0f)
{
	std::shared_ptr<Segment> segment = std::make_shared<Segment>();
	m_segments.push_back(segment);

	ensureSizeMatchesMass();
}

Snake::Snake(Field *field, const Vector &startPos, float_t start_mass,
		float_t start_heading)
	: m_field(field), m_mass(start_mass), m_heading(start_heading)
{
	// create the first segment manually
	std::shared_ptr<Segment> segment = std::make_shared<Segment>();
	segment->pos = startPos;

	m_segments.push_back(segment);

	// create the other segments
	ensureSizeMatchesMass();
}

void Snake::ensureSizeMatchesMass(void)
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

	// update segment radius
	m_segmentRadius = std::sqrt(m_mass) / 2;
}

float_t Snake::maxRotationPerStep(void)
{
	// TODO: make this better?
	return 10.0 / (m_mass/10.0 + 1);
}

void Snake::consume(const std::shared_ptr<Food>& food)
{
	m_mass += food->getValue();
	ensureSizeMatchesMass();
}

std::size_t Snake::move(float_t targetAngle, bool boost)
{
	// calculate delta angle
	float_t deltaAngle = targetAngle - m_heading;

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

	std::size_t oldSize = m_segments.size();

	// create multiple segments while boosting
	std::size_t steps = 1;
	if(boost) {
		steps = config::SNAKE_BOOST_STEPS;
	}

	// create new segments at head
	for(std::size_t i = 0; i < steps; i++) {
		// calculate new segment offset
		m_heading += deltaAngle;
		Vector movementVector(config::SNAKE_DISTANCE_PER_STEP, 0.0f);
		movementVector.rotate(m_heading * M_PI / 180);

		// create new segment
		std::shared_ptr<Segment> segment = std::make_shared<Segment>();
		segment->pos = m_field->wrapCoords(m_segments[0]->pos + movementVector);

		m_segments.push_front(segment);
	}

	// normalize heading
	if(m_heading > 180) {
		m_heading -= 360;
	} else if(m_heading < -180) {
		m_heading += 360;
	}

	// force size to previous size (removes end segments)
	m_segments.resize(oldSize);

	return steps; // == number of new segments at head
}

const Snake::SegmentList& Snake::getSegments(void) const
{
	return m_segments;
}

const Vector& Snake::getHeadPosition(void) const
{
	return m_segments[0]->pos;
}

float_t Snake::getSegmentRadius(void) const
{
	return m_segmentRadius;
}

bool Snake::canConsume(const std::shared_ptr<Food> &food)
{
	const Vector &headPos = m_segments[0]->pos;
	const Vector &foodPos = food->getPosition();

	float_t hx = headPos.x();
	float_t hy = headPos.y();

	Vector unwrappedFoodPos = m_field->unwrapCoords(foodPos, headPos);
	float_t fx = unwrappedFoodPos.x();
	float_t fy = unwrappedFoodPos.y();

	// quick range check
	float_t maxRange = m_segmentRadius * config::SNAKE_CONSUME_RANGE;
	if(fx > (hx + maxRange) ||
			fx < (hx - maxRange) ||
			fy > (hy + maxRange) ||
			fy < (hy - maxRange)) {
		return false;
	}

	// thorough range check
	return headPos.distanceTo(unwrappedFoodPos) < maxRange;
}
