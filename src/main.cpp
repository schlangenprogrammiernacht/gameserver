#include <thread>
#include <chrono>

#include <iostream>

#include "Bot.h"
#include "Field.h"

#include "MsgPackUpdateTracker.h"

int main(void)
{
	std::shared_ptr<UpdateTracker> tracker = std::make_shared<MsgPackUpdateTracker>();
	std::shared_ptr<Field> f = std::make_shared<Field>(60, 20, 30, tracker);

	f->newBot("testBot");

	for(std::size_t i = 0; /*forever*/ ; i++) {
		std::cout << "Frame number #" << i << std::endl;

		f->updateFood();
		f->consumeFood();

		f->moveAllBots();

		std::cout << tracker->getSerializedEvents() << std::endl;
		tracker->reset();

		f->debugVisualization();

		std::this_thread::sleep_for(std::chrono::milliseconds(200));
	}

	return 0;
}
