#include "Field.h"

Field::Field()
	: m_width(128), m_height(128)
{
}

Field::Field(float_t w, float_t h)
	: m_width(w), m_height(h)
{
}

Vector Field::wrapCoords(const Vector &v)
{
	Vector result(v);

	while(result.x() < 0) {
		result.set_x(result.x() + m_width);
	}

	while(result.x() > m_width) {
		result.set_x(result.x() - m_width);
	}

	while(result.y() < 0) {
		result.set_y(result.y() + m_height);
	}

	while(result.y() > m_height) {
		result.set_y(result.y() - m_height);
	}

	return result;
}

Vector Field::unwrapCoords(const Vector &v, const Vector &ref)
{
	Vector result(v);

	while((result.x() - ref.x()) < -m_width/2) {
		result.set_x(result.x() + m_width);
	}

	while((result.x() - ref.x()) > m_width/2) {
		result.set_x(result.x() - m_width);
	}

	while((result.y() - ref.y()) < -m_height/2) {
		result.set_y(result.y() + m_height);
	}

	while((result.y() - ref.y()) > m_height/2) {
		result.set_y(result.y() - m_height);
	}

	return result;
}
