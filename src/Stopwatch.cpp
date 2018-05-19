/*

  Copyright (c) 2016 Hubert Denkmair <hubert@denkmair.de>

  This file is part of cangaroo.

  cangaroo is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 2 of the License, or
  (at your option) any later version.

  cangaroo is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with cangaroo.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "Stopwatch.h"

Stopwatch::Stopwatch(std::string name)
	: m_name(name)
{
	Start();
}

void Stopwatch::Start()
{
	clock_gettime(CLOCK_MONOTONIC, &m_tStartMonotonic);
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &m_tStartProcess);
	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &m_tStartThread);
}

void Stopwatch::Stop()
{
	clock_gettime(CLOCK_MONOTONIC, &m_tStopMonotonic);
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &m_tStopProcess);
	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &m_tStopThread);

	m_tMonotonic += timespecDiffToNano(m_tStartMonotonic, m_tStopMonotonic);
	m_tProcess += timespecDiffToNano(m_tStartProcess, m_tStopProcess);
	m_tThread += timespecDiffToNano(m_tStartThread, m_tStopThread);
}

void Stopwatch::Reset()
{
	m_tMonotonic = 0;
	m_tProcess = 0;
	m_tThread = 0;
}

void Stopwatch::Print()
{
	printf(
		"%16s time: %6luus process: %6luus thread: %6luus\n",
		m_name.c_str(),
		m_tMonotonic / 1000,
		m_tProcess / 1000,
		m_tThread / 1000
	);
}

long Stopwatch::timespecDiffToNano(timespec &ts1, timespec &ts2)
{
	long seconds = ts2.tv_sec - ts1.tv_sec;
	long nano = ts2.tv_nsec - ts1.tv_nsec;
	return (1000000000l * seconds) + nano;
}
