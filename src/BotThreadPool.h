#pragma once

#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>

#include <queue>

#include "Semaphore.h"

// forward declaration
class Bot;

class BotThreadPool
{
	public:
		enum JobType {
			Move,
			CollisionCheck
		};

		struct Job {
			JobType jobType;

			// inputs
			std::shared_ptr<Bot> bot;

			// output
			// for jobType == Move
			std::size_t steps;
			// for jobType == CollisionCheck
			std::shared_ptr<Bot> killer;

			Job(JobType type, const std::shared_ptr<Bot> myBot)
				: jobType(type), bot(myBot)
			{}
		};

	private:
		std::vector<std::thread> m_threads;
		std::queue< std::unique_ptr<Job> > m_inputJobs;
		std::queue< std::unique_ptr<Job> > m_processedJobs;

		std::mutex m_inputQueueMutex;
		std::mutex m_processedQueueMutex;

		Semaphore m_workAvailSemaphore; // blocks worker threads if there is no work

		bool m_shutdown = false;

		std::condition_variable m_finishedCV;
		std::mutex m_finishedMutex;

		std::size_t m_activeThreads;

	public:
		BotThreadPool(std::size_t num_threads);

		~BotThreadPool();

		/*!
		 * \brief Add a job to be processed in parallel.
		 *
		 * Processing starts immediately.
		 *
		 * \param job  The Job to add.
		 */
		void addJob(std::unique_ptr<Job> job);

		/*!
		 * \brief Wait until all jobs are processed.
		 *
		 * \details
		 * This function blocks until all jobs have been processed by the worker
		 * threads.
		 */
		void waitForCompletion(void);

		/*!
		 * \brief Get next processed job.
		 * \returns The next queue entry (which is removed from the queue) or
		 *          NULL if queue is empty.
		 */
		std::unique_ptr<Job> getProcessedJob(void);
};
