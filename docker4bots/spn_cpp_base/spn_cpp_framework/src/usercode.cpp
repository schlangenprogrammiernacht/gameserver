#include <math.h>

#include "usercode.h"

bool init(Api *api)
{
	api->clearColors();
	api->addColor(255,   0,   0);
	api->addColor(255, 255,   0);
	api->addColor(  0, 255,   0);
	api->addColor(  0, 255, 255);
	api->addColor(  0,   0, 255);
	api->addColor(255,   0, 255);

	return true;
}

bool step(Api *api)
{
	static uint64_t frame = 0;

	api->angle = 0.05 * sin(0.03 * frame);
	api->boost = false;

	frame++;

	return true;
}

