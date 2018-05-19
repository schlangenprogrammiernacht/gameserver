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
#include <string>
#include <time.h>

class Stopwatch
{
	public:
		Stopwatch(std::string name);
		void Start();
		void Stop();
		void Reset();
		void Print();

		long GetMonotonicTimeNs() { return m_tMonotonic; }
		long GetProcessTimeNs() { return m_tProcess; }
		long GetThreadTimeNs() { return m_tThread; }

	private:
		std::string m_name;
		struct timespec m_tStartMonotonic, m_tStopMonotonic;
		struct timespec m_tStartProcess, m_tStopProcess;
		struct timespec m_tStartThread, m_tStopThread;
		long m_tMonotonic = 0;
		long m_tProcess = 0;
		long m_tThread = 0;

		long timespecDiffToNano(struct timespec& ts1, timespec &ts2);

};
