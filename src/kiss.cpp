#include "frame_lib/kiss.hpp"
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>

std::vector<uint8_t> KISS::encode(const std::vector<uint8_t>& payload, uint8_t port, uint8_t command) {
	if (port > 0x0F) throw std::invalid_argument("KISS type port must be 0-15 (inclusive)");
	if (command > 6 && ((port << 4) | command) != 0xFF) throw std::invalid_argument("KISS type command must be 0-6 (inclusive) or 0xFF");

	size_t expandedSize = 0;
	for (uint8_t byte : payload) {
		expandedSize += (byte == 0xDB || byte == 0xC0) ? 2 : 1;
	}

	uint8_t type = (port << 4) | command;

	std::vector<uint8_t> frame = {0xC0, type};
	frame.reserve(expandedSize + 3);

	for (uint8_t byte : payload) {
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

KissPacket KISS::decode(const std::vector<uint8_t>& kissFrame) {
	if (kissFrame.size() < 3) throw std::invalid_argument("KISS frame must be >= 3 characters");
	if (kissFrame.back() != 0xC0) throw std::runtime_error("KISS frame missing trailing FEND");

	size_t i = 0;
	while (i < kissFrame.size() && kissFrame[i] == 0xC0) {
		i++;
	}
	if (i >= kissFrame.size()) throw std::runtime_error("KISS frame missing command byte");

	uint8_t command = kissFrame[i] & 0x0F;
	uint8_t port = (kissFrame[i++] & 0xF0) >> 4;

	size_t expandedSize = 0;
	for (uint8_t byte : kissFrame) {
		expandedSize += (byte == 0xDB) ? 0 : 1;
	}
	expandedSize -= i;

	std::vector<uint8_t> payload;
	payload.reserve(expandedSize);

	for (; i < kissFrame.size() - 1; i++) {
		if (kissFrame[i] == 0xC0) break;

		if (kissFrame[i] == 0xDB && i + 1 < kissFrame.size()) {
			if (kissFrame[i + 1] == 0xDD) payload.push_back(0xDB);
			else if (kissFrame[i + 1] == 0xDC) payload.push_back(0xC0);
			else throw std::runtime_error("KISS Decode Error: Malformed escape sequence following 0xDB");
			i++;
		} else {
			payload.push_back(kissFrame[i]);
		}
	}

	return {.port = port, .command = command, .payload = payload};
}
