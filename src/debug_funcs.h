#pragma once

#include <string>

static std::string hexdump(const std::string &binary)
{
	char nibbles[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

	std::string out;
	out.reserve(2 * binary.size());

	for(auto byte : binary) {
		out += nibbles[(static_cast<uint8_t>(byte) >> 4) & 0x0F];
		out += nibbles[(static_cast<uint8_t>(byte) >> 0) & 0x0F];
	}

	return out;
}
