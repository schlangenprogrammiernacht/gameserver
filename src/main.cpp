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

#include <signal.h>

#include "Game.h"

Game game;

/*!
 * Signal handler for terminating signals such as SIGINT, SIGTERM, etc.
 */
void sig_shutdown_handler(int sig)
{
	game.Shutdown();
	std::cerr << "Shutdown initiated on signal " << sig << std::endl;
	std::cerr << "Send the same signal again to terminate immediately." << std::endl;

	struct sigaction default_action;
	default_action.sa_handler = SIG_DFL;
	sigemptyset(&default_action.sa_mask);
	sigaction(sig, &default_action, NULL);
}

bool setup_signal_handlers(void)
{
	struct sigaction shutdown_action;
	memset(&shutdown_action, 0, sizeof(shutdown_action));

	shutdown_action.sa_handler = sig_shutdown_handler;
	sigemptyset(&shutdown_action.sa_mask);

	if(sigaction(SIGINT, &shutdown_action, NULL) == -1) {
		std::cerr << "sigaction(SIGINT) failed: " << strerror(errno) << std::endl;
		return false;
	}

	if(sigaction(SIGTERM, &shutdown_action, NULL) == -1) {
		std::cerr << "sigaction(SIGINT) failed: " << strerror(errno) << std::endl;
		return false;
	}

	return true;
}

int main(void)
{
	if(!setup_signal_handlers()) {
		return 1;
	}

	return game.Main();
}
