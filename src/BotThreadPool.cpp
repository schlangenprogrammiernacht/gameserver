#include <memory>

#include "Bot.h"

#include "config.h"

#include "BotThreadPool.h"

BotThreadPool::BotThreadPool(std::size_t num_threads)
	: m_threads(num_threads), m_activeThreads(0)
{
	// create all the threads
	for(auto &thread : m_threads) {
		std::thread newThread(
					[this] ()
					{
						while(!m_shutdown) {
							m_workAvailSemaphore.wait();

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
