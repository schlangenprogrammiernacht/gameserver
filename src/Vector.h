#pragma once

#include "types.h"

/*!
 * A vector in the game’s coordinate system (2D for now).
 */
class Vector
{
	private:
		float_t m_x;
		float_t m_y;

	public:
		Vector();
		Vector(float_t x, float_t y);
		Vector(const Vector &v);

		float_t abs(void) const;
		float_t distanceTo(const Vector &other) const;

		/* Short, potentially inlined functions */
		float_t x(void) const { return m_x; };
		float_t y(void) const { return m_y; };

		void set_x(float_t x) { m_x = x; }
		void set_y(float_t y) { m_y = y; }

		Vector& operator = (const Vector &other)
		{
			m_x = other.m_x;
			m_y = other.m_y;
			return *this;
		}

		Vector operator + (const Vector &delta)
		{
			return Vector(m_x + delta.m_x, m_y + delta.m_y);
		}

		Vector operator - (const Vector &delta)
		{
			return Vector(m_x - delta.m_x, m_y - delta.m_y);
		}

		Vector& operator += (const Vector &delta)
		{
			m_x += delta.m_x;
			m_y += delta.m_y;
			return *this;
		}

		Vector& operator -= (const Vector &delta)
		{
			m_x -= delta.m_x;
			m_y -= delta.m_y;
			return *this;
		}

		Vector& operator *= (float_t factor)
		{
			m_x *= factor;
			m_y *= factor;
			return *this;
		}

		Vector& operator /= (float_t divisor)
		{
			m_x /= divisor;
			m_y /= divisor;
			return *this;
		}
};
