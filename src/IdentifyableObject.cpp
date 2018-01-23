#include "GUIDGenerator.h"

#include "IdentifyableObject.h"

IdentifyableObject::IdentifyableObject()
	: m_guid(GUIDGenerator::instance().newGUID())
{
}

IdentifyableObject::~IdentifyableObject()
{
}
