#pragma once

#include <string.h>

#include "ipc_format.h"

/*!
 * \brief A class providing a simplified API for the Bots.
 *
 * \details
 * The main purpose of this class is to abstract away the layout details of the
 * shared memory used to interface with the gameserver. Methods are provided to
 * extract relevant data from the shared memory and the movement parameters for
 * the current frame can be easily set using public member variables.
 *
 * Most methods return pointers pointing into the shared memory to minimize the
 * overhead. The returned structures are described in ipc_format.h .
 */
class Api
{
	public:
		/*** variables ***/
		ipc_real_t angle; //!< The direction into which the bot should move in the current frame
		bool       boost; //!< Set this to true to boost in this frame

		/*** functions ***/
		/*!
		 * \brief Constructor for the API interface object.
		 *
		 * Initialized the internal variables and stores the pointer to the shared
		 * memory. No need to call this from the bot code.
		 *
		 * \param shm   Pointer to the (already set up) shared memory.
		 */
		Api(IpcSharedMemory *shm)
			: angle(0)
			, boost(false)
			, m_shm(shm)
		{}

		/*!
		 * \brief Get a pointer to the server config data.
		 *
		 * This is not a list, but a pointer to a single structure containing
		 * information about the server configuration and static world information.
		 *
		 * It is only written once, so if you overwrite it by accident, it will
		 * only be reset when your bot restarts.
		 *
		 * \returns    A pointer to the self information in the shared memory.
		 */
		const IpcServerConfig* getServerConfig(void) { return &(m_shm->serverConfig); }

		/*!
		 * \brief Get a pointer to the self information.
		 *
		 * This is not a list, but a pointer to a single structure containing
		 * information about your snake and parameters of the world.
		 *
		 * \returns    A pointer to the self information in the shared memory.
		 */
		const IpcSelfInfo* getSelfInfo(void) { return &(m_shm->selfInfo); }

		/*!
		 * \brief Get a pointer to the Food list.
		 *
		 * The Food list is sorted by distance from your snake's head.
		 *
		 * The length of the list can be determined using getFoodCount().
		 *
		 * \returns    A pointer to the Food information in the shared memory.
		 */
		const IpcFoodInfo* getFood(void)      { return m_shm->foodInfo; }

		/*!
		 * \brief Get the length of the Food list.
		 *
		 * \returns    The length of the array returned by getFood().
		 */
		size_t             getFoodCount(void) { return m_shm->foodCount; }

		/*!
		 * \brief Get a pointer to the Segment list.
		 *
		 * The Segment list is sorted by distance from your snake's head.
		 *
		 * The length of the list can be determined using getSegmentCount().
		 *
		 * \returns    A pointer to the Segment information in the shared memory.
		 */
		const IpcSegmentInfo* getSegments(void)     { return m_shm->segmentInfo; }

		/*!
		 * \brief Get the length of the Segment list.
		 *
		 * \returns    The length of the array returned by getSegments().
		 */
		size_t                getSegmentCount(void) { return m_shm->segmentCount; }

		/*!
		 * \brief Get a pointer to the Bot list.
		 *
		 * The length of the list can be determined using getBotCount().
		 *
		 * \returns    A pointer to the Bot information in the shared memory.
		 */
		const IpcBotInfo* getBots(void)     { return m_shm->botInfo; }

		/*!
		 * \brief Get the length of the Bot list.
		 *
		 * \returns    The length of the array returned by getBots().
		 */
		size_t            getBotCount(void) { return m_shm->botCount; }

		/*!
		 * \brief Clear the color list.
		 *
		 * You need to add new colors using addColor() or your snake will have lame
		 * default colors!
		 */
		void clearColors(void) { m_shm->colorCount = 0; }

		/*!
		 * \brief Add a color to the color list.
		 *
		 * You should do this during your bot's initialization. Colors cannot be
		 * changed afterwards.
		 *
		 * \param r    The red channel (0-255).
		 * \param g    The green channel (0-255).
		 * \param b    The blue channel (0-255).
		 *
		 * \returns    Whether the color was added successfully. False if the
		 *             maximum number of colors was reached.
		 */
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

		/*!
		 * \brief Send a log message.
		 *
		 * These messages will appear on the web interface and in the World update
		 * stream when you provide your viewer key to the server.
		 *
		 * Rate limiting is enforced by the gameserver, so messages are dropped
		 * when you send too many of them.
		 *
		 * \param msg    Pointer to the null-terminated message string.
		 */
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
