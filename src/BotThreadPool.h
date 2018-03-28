#pragma once

#include <vector>
#include <thread>
#include <mutex>

#include <queue>

// forward declaration
class Bot;

class BotThreadPool
{
	public:
		struct Job {
			// inputs
			std::shared_ptr<Bot> bot;

			// output
			std::size_t steps;
		};

	private:
		std::vector<std::thread> m_threads;
		std::queue< std::unique_ptr<Job> > m_inputJobs;
		std::queue< std::unique_ptr<Job> > m_processedJobs;

		std::mutex m_inputQueueMutex;
		std::mutex m_processedQueueMutex;

	public:
		BotThreadPool(std::size_t num_threads);

		/*!
		 * \brief Add a job to be processed in parallel
		 *
		 * \param job  The Job to add.
		 */
		void addJob(std::unique_ptr<Job> job);

		/*!
		 * \brief Execute queued jobs.
		 *
		 * \details
		 * This function clears the queue of processed jobs and runs all queued
		 * jobs through the thread pool, which stores the processed jobs in the
		 * processed job queue.
		 */
		void run(void);

		/*!
		 * \brief Get next processed job.
		 * \returns The next queue entry (which is removed from the queue) or
		 *          NULL if queue is empty.
		 */
		std::unique_ptr<Job> getProcessedJob(void);
};
