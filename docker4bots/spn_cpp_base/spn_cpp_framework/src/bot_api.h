#pragma once

#include <string.h>

#include "ipc_format.h"

class Api
{
	public:
		// variables
		ipc_real_t angle;
		bool       boost;

		// functions
		Api(IpcSharedMemory *shm)
			: angle(0)
			, boost(false)
			, m_shm(shm)
		{}

		const IpcFoodInfo* getFood(void)      { return m_shm->foodInfo; }
		size_t             getFoodCount(void) { return m_shm->foodCount; }

		const IpcSegmentInfo* getSegments(void)     { return m_shm->segmentInfo; }
		size_t                getSegmentCount(void) { return m_shm->segmentCount; }

		const IpcBotInfo* getBots(void)     { return m_shm->botInfo; }
		size_t            getBotCount(void) { return m_shm->botCount; }

		void clearColors(void) { m_shm->colorCount = 0; }
		bool addColor(uint8_t r, uint8_t g, uint8_t b)
		{
			if(m_shm->colorCount >= IPC_COLOR_MAX_COUNT) {
				return false;
			}

			m_shm->colors[m_shm->colorCount].r = r;
			m_shm->colors[m_shm->colorCount].g = g;
			m_shm->colors[m_shm->colorCount].b = b;
			m_shm->colorCount++;

			return true;
		}

		void log(const char *msg)
		{
			size_t len = strlen(m_shm->logData);

			if(len >= IPC_COLOR_MAX_COUNT-2) {
				return;
			}

			size_t n = IPC_LOG_MAX_BYTES - len - 1;

			if(len != 0) {
				m_shm->logData[len] = '\n';
				m_shm->logData[len+1] = '\0';
			}

			strncat(m_shm->logData, msg, n);
		}

	private:
		IpcSharedMemory *m_shm;
};
