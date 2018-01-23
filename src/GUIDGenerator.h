#pragma once

#include "types.h"

/*!
 * A singleton class generating unique IDs.
 */
class GUIDGenerator
{
	private:
		guid_t m_nextID;

		GUIDGenerator();

	public:
		guid_t newGUID(void);

		static GUIDGenerator& instance(void)
		{
			static GUIDGenerator theOneAndOnly;
			return theOneAndOnly;
		}
};
