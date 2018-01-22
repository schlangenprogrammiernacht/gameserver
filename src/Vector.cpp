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

float_t Vector::angle(void) const
{
	return atan2(m_y, m_x) * 180 / M_PI;
}

float_t Vector::abs(void) const
{
	return sqrt(m_x*m_x + m_y*m_y);
}

void Vector::rotate(float_t phi)
{
	float_t cosphi = cos(phi);
	float_t sinphi = sin(phi);

	float_t tmp_x = cosphi * m_x - sinphi * m_y;
	float_t tmp_y = sinphi * m_x + cosphi * m_y;

	m_x = tmp_x;
	m_y = tmp_y;
}

void Vector::normalize(void)
{
	float_t norm = abs();

	m_x /= norm;
	m_y /= norm;
}

float_t Vector::distanceTo(const Vector &other) const
{
	float_t dx = other.m_x - m_x;
	float_t dy = other.m_y - m_y;

	return sqrt(dx*dx + dy*dy);
}

