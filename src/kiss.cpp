#include "frame_lib/kiss.hpp"
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>

std::vector<uint8_t> KISS::encode(const std::vector<uint8_t>& ax25Frame, uint8_t type) {
	size_t expandedSize = 0;
	for (uint8_t byte : ax25Frame) {
		expandedSize += (byte == 0xDB || byte == 0xC0) ? 2 : 1;
	}

	std::vector<uint8_t> frame = {0xC0, type};
	frame.reserve(expandedSize + 3);

	for (uint8_t byte : ax25Frame) {
		if (byte == 0xDB) {
			frame.push_back(0xDB);
			frame.push_back(0xDD);
		} else if (byte == 0xC0) {
			frame.push_back(0xDB);
			frame.push_back(0xDC);
		} else {
			frame.push_back(byte);
		}
	}

	frame.push_back(0xC0);
	return frame;
}

std::vector<uint8_t> KISS::decode(const std::vector<uint8_t>& kissFrame) {
	if (kissFrame.size() < 3) throw std::invalid_argument("KISS frame must be >= 3 characters");

	size_t i = 0;
	while (i < kissFrame.size() && kissFrame[i] == 0xC0) {
		i++;
	}
	i++;

	size_t expandedSize = 0;
	for (uint8_t byte : kissFrame) {
		expandedSize += (byte == 0xDB) ? 0 : 1;
	}
	expandedSize -= i;

	std::vector<uint8_t> ax25Frame;
	ax25Frame.reserve(expandedSize);

	for (; i < kissFrame.size() - 1; i++) {
		if (kissFrame[i] == 0xC0) {
			break;
		}

		if (kissFrame[i] == 0xDB && i + 1 < kissFrame.size()) {
			if (kissFrame[i + 1] == 0xDD) ax25Frame.push_back(0xDB);
			else if (kissFrame[i + 1] == 0xDC) ax25Frame.push_back(0xC0);
			else throw std::runtime_error("KISS Decode Error: Malformed escape sequence following 0xDB");
			i++;
		} else {
			ax25Frame.push_back(kissFrame[i]);
		}
	}

	return ax25Frame;
}
