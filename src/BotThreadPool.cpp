#include <memory>

#include "Bot.h"

#include "config.h"

#include "BotThreadPool.h"

BotThreadPool::BotThreadPool(std::size_t num_threads)
	: m_threads(num_threads)
{
	// create all the threads
	for(auto &thread : m_threads) {
		std::thread newThread(
					[this] ()
					{
						while(!m_shutdown) {
							std::unique_ptr<Job> currentJob;

							{
								std::lock_guard<std::mutex> inputQueueGuard(m_inputQueueMutex);

								if(!m_inputJobs.empty()) {
									currentJob = std::move(m_inputJobs.front());
									m_inputJobs.pop();

									std::lock_guard<std::mutex> activeThreadsGuard(m_activeThreadsMutex);
									m_activeThreads++;
								} else {
									currentJob = NULL;
								}
							}

							if(currentJob) {
								currentJob->steps = currentJob->bot->move();

								std::lock_guard<std::mutex> processedQueueGuard(m_processedQueueMutex);
								m_processedJobs.push(std::move(currentJob));

								std::lock_guard<std::mutex> activeThreadsGuard(m_activeThreadsMutex);
								m_activeThreads--;
							} else {
								std::this_thread::sleep_for(config::THREAD_POOL_IDLE_SLEEP_TIME);
							}
						}
					});

		thread.swap(newThread);
	}
}

BotThreadPool::~BotThreadPool()
{
	// request thread shutdown
	m_shutdown = true;

	// wait for all threads to finish
	for(auto &thread : m_threads) {
		thread.join();
	}
}

void BotThreadPool::addJob(std::unique_ptr<Job> job)
{
	std::lock_guard<std::mutex> guard(m_inputQueueMutex);
	m_inputJobs.push(std::move(job));
}

void BotThreadPool::run(void)
{
	// run until the input queue is empty
	bool still_working;

	do {
		std::this_thread::sleep_for(config::THREAD_POOL_IDLE_SLEEP_TIME);

		std::lock_guard<std::mutex> inputQueueGuard(m_inputQueueMutex);
		std::lock_guard<std::mutex> activeThreadsGuard(m_activeThreadsMutex);
		still_working = !m_inputJobs.empty() || (m_activeThreads > 0);
	} while(still_working);
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
