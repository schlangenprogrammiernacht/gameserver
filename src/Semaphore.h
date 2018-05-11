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

// Thanks to StackOverflow!
//
// https://stackoverflow.com/a/4793662

#include <mutex>
#include <condition_variable>

class Semaphore
{
	private:
		std::mutex mutex_;
		std::condition_variable condition_;
		unsigned long count_ = 0; // Initialized as locked.

	public:
		Semaphore(unsigned long init = 0)
			: count_(init)
		{
		}

		void post()
		{
			std::unique_lock<decltype(mutex_)> lock(mutex_);
			++count_;
			condition_.notify_one();
		}

		void wait()
		{
			std::unique_lock<decltype(mutex_)> lock(mutex_);
			while(!count_) { // Handle spurious wake-ups.
				condition_.wait(lock);
			}
			--count_;
		}

		bool try_wait()
		{
			std::unique_lock<decltype(mutex_)> lock(mutex_);
			if(count_) {
				--count_;
				return true;
			}
			return false;
		}
};
