#include <memory>

#include "Bot.h"

#include "BotThreadPool.h"

BotThreadPool::BotThreadPool(std::size_t num_threads)
	: m_threads(num_threads)
{
}

void BotThreadPool::addJob(std::unique_ptr<Job> job)
{
	m_inputJobs.push(std::move(job));
}

void BotThreadPool::run(void)
{
	// prevent threads from running before pool is fully initialized
	m_inputQueueMutex.lock();

	for(auto &thread : m_threads) {
		std::thread newThread(
					[this] ()
					{
						m_inputQueueMutex.lock();

						while(!m_inputJobs.empty()) {

							std::unique_ptr<Job> currentJob(std::move(m_inputJobs.front()));
							m_inputJobs.pop();

							m_inputQueueMutex.unlock();

							currentJob->steps = currentJob->bot->move();

							m_processedQueueMutex.lock();
							m_processedJobs.push(std::move(currentJob));
							m_processedQueueMutex.unlock();

							m_inputQueueMutex.lock();
						}

						m_inputQueueMutex.unlock();
					});

		thread.swap(newThread);
	}

	// run it all
	m_inputQueueMutex.unlock();

	// wait for completion
	for(auto &thread : m_threads) {
		thread.join();
	}
}

std::unique_ptr<BotThreadPool::Job> BotThreadPool::getProcessedJob()
{
	if(!m_processedJobs.empty()) {
		std::unique_ptr<Job> job(std::move(m_processedJobs.front()));
		m_processedJobs.pop();

		return job;
	} else {
		return NULL;
	}
}
