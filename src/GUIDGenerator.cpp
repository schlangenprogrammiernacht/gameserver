#include "GUIDGenerator.h"

GUIDGenerator::GUIDGenerator()
	: m_nextID(0)
{
}

guid_t GUIDGenerator::newGUID(void)
{
	return m_nextID++;
}
