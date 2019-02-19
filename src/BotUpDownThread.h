/*
 * Schlangenprogrammiernacht: A programming game for GPN18.
 * Copyright (C) 2018  bytewerk
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <thread>
#include <mutex>
#include <memory>
#include <queue>
#include <set>

// forward declaration
class Bot;

class BotUpDownThread
{
	public:
		struct Result {
			std::shared_ptr<Bot> bot;
			std::string          message;
			bool                 success;
		};

	private:
		std::thread m_thread;
		std::queue< std::shared_ptr<Bot> > m_startupInQueue;
		std::queue< std::shared_ptr<Bot> > m_shutdownInQueue;
		std::queue< std::unique_ptr<Result> > m_startupOutQueue;
		std::queue< std::unique_ptr<Result> > m_shutdownOutQueue;

		std::mutex m_startupQueueMutex;
		std::mutex m_shutdownQueueMutex;

		std::set<int> m_dbIdsInProgress;

		bool m_shutdown = false;

	public:
		BotUpDownThread(void);

		~BotUpDownThread();

		/*!
		 * \brief Add a bot to start up asynchronously.
		 *
		 * \param bot  The Bot to start.
		 */
		void addStartupBot(const std::shared_ptr<Bot> &bot);

		/*!
		 * \brief Add a bot to shut down asynchronously.
		 *
		 * \param bot  The Bot to shut down.
		 */
		void addShutdownBot(const std::shared_ptr<Bot> &bot);

		/*!
		 * \brief Get a started bot from the internal queue.
		 *
		 * \returns A unique pointer to a Result structure. A NULL pointer will be
		 *          returned if the queue is empty.
		 */
		std::unique_ptr<Result> getStartupResult(void);

		/*!
		 * \brief Get a shut down bot from the internal queue.
		 *
		 * \returns A unique pointer to a Result structure. A NULL pointer will be
		 *          returned if the queue is empty.
		 */
		std::unique_ptr<Result> getShutDownResult(void);

		/*!
		 * \brief Check if a bot database ID is currently managed by this class.
		 */
		bool containsDatabaseId(int id) { return m_dbIdsInProgress.count(id) > 0; }
};
