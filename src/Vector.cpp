#include <math.h>

#include "Vector.h"

Vector::Vector()
	: m_x(0), m_y(0)
{
}

Vector::Vector(float_t x, float_t y)
	: m_x(x), m_y(y)
{
}

Vector::Vector(const Vector &v)
	: m_x(v.m_x), m_y(v.m_y)
{
}

float_t Vector::abs(void) const
{
	return sqrt(m_x*m_x + m_y*m_y);
}

float_t Vector::distanceTo(const Vector &other) const
{
	float_t dx = other.m_x - m_x;
	float_t dy = other.m_y - m_y;

	return sqrt(dx*dx + dy*dy);
}

