#pragma once
#include "types.h"

class PositionObject
{
	public:
		Vector2D pos;

		PositionObject(const Vector2D& position)
			: pos(position) {}
};
