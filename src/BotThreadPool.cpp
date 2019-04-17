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

#include <memory>
#include <sstream>

#include "Bot.h"

#include "config.h"

#include "BotThreadPool.h"

BotThreadPool::BotThreadPool(std::size_t num_threads)
	: m_threads(num_threads), m_activeThreads(0)
{
	// create all the threads
	int threadnum = 0;
	for(auto &thread : m_threads) {
		thread = std::thread(
					[this] ()
					{
						while(true) {
							m_workAvailSemaphore.wait();

							if(m_shutdown) {
								break;
							}

							{
								std::unique_lock<std::mutex> lock(m_finishedMutex);
								m_activeThreads++;
								m_finishedCV.notify_one();
							}

							std::unique_ptr<Job> currentJob;

							{
								std::lock_guard<std::mutex> inputQueueGuard(m_inputQueueMutex);

								if(!m_inputJobs.empty()) {
									currentJob = std::move(m_inputJobs.front());
									m_inputJobs.pop();
								} else {
									currentJob = NULL;
								}
							}

							if(currentJob) {
								switch(currentJob->jobType) {
									case Move:
										currentJob->steps = currentJob->bot->move();
										break;

									case CollisionCheck:
										currentJob->killer = currentJob->bot->checkCollision();
										break;
								}

								std::lock_guard<std::mutex> processedQueueGuard(m_processedQueueMutex);
								m_processedJobs.push(std::move(currentJob));
							}

							{
								std::unique_lock<std::mutex> lock(m_finishedMutex);
								m_activeThreads--;
								m_finishedCV.notify_one();
							}
						}
					});

		// remove this line if it does not compile on your system. It does not affect
		// the program's functionality.
		std::ostringstream namestream;
		namestream << "bot_worker_" << (threadnum++);
		pthread_setname_np(thread.native_handle(), namestream.str().c_str());
	}
}

BotThreadPool::~BotThreadPool()
{
	// request thread shutdown
	m_shutdown = true;

	// unlock all threads so they read the shutdown signal
	for(auto &thread : m_threads) {
		m_workAvailSemaphore.post();
	}

	// wait for all threads to finish
	for(auto &thread : m_threads) {
		thread.join();
	}
}

void BotThreadPool::addJob(std::unique_ptr<Job> job)
{
	std::lock_guard<std::mutex> guard(m_inputQueueMutex);
	m_inputJobs.push(std::move(job));

	m_workAvailSemaphore.post();
}

void BotThreadPool::waitForCompletion(void)
{
	std::unique_lock<std::mutex> lock(m_finishedMutex);
	m_finishedCV.wait(lock, [this]() {
			std::lock_guard<std::mutex> inputQueueGuard(m_inputQueueMutex);
			return m_inputJobs.empty() && (m_activeThreads == 0);
		});
}

std::unique_ptr<BotThreadPool::Job> BotThreadPool::getProcessedJob()
{
	std::lock_guard<std::mutex> guard(m_processedQueueMutex);

	if(!m_processedJobs.empty()) {
		std::unique_ptr<Job> job(std::move(m_processedJobs.front()));
		m_processedJobs.pop();

		return job;
	} else {
		return NULL;
	}
}
