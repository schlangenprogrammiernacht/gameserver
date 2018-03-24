#pragma once
#include "types.h"

class PositionObject
{
	public:

		PositionObject(const Vector2D& position)
			: m_pos(position) {}

		const Vector2D& pos() const { return m_pos; }
		void setPos(const Vector2D& pos) { m_pos = pos; }

	private:
		Vector2D m_pos;
};
