#include "frame_lib/kiss.hpp"
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>

std::vector<uint8_t> KISS::encode(const std::vector<uint8_t>& payload, uint8_t port, uint8_t command) {
	if (port > 0x0F) throw std::invalid_argument("KISS type port must be in range [0, 15]");
	if (command > 6 && ((port << 4) | command) != 0xFF) throw std::invalid_argument("KISS type command must be in range [0, 6] or 0xFF");

	size_t escapedPayloadSize = 0;
	for (uint8_t byte : payload) {
		escapedPayloadSize += (byte == 0xDB || byte == 0xC0) ? 2 : 1;
	}

	uint8_t type = (port << 4) | command;

	std::vector<uint8_t> frame(escapedPayloadSize + 3);
	size_t j = 0;
	frame[j++] = 0xC0;
	frame[j++] = type;

	for (uint8_t i : payload) {
		if (i == 0xDB) {
			frame[j++] = 0xDB;
			frame[j++] = 0xDD;
		} else if (i == 0xC0) {
			frame[j++] = 0xDB;
			frame[j++] = 0xDC;
		} else {
			frame[j++] = i;
		}
	}

	frame[j] = 0xC0;
	return frame;
}

KissPacket KISS::decode(const std::vector<uint8_t>& kissFrame) {
	if (kissFrame.size() < 3) throw std::invalid_argument("KISS frame minimum length is 3 characters");
	if (kissFrame.front() != 0xC0) throw std::runtime_error("KISS frame missing leading FEND");
	if (kissFrame.back() != 0xC0) throw std::runtime_error("KISS frame missing trailing FEND");
	if (kissFrame[1] == 0xC0 || kissFrame[1] == 0xDB) throw std::runtime_error("Invalid type byte");

	uint8_t command = kissFrame[1] & 0x0F;
	uint8_t port = (kissFrame[1] & 0xF0) >> 4;
	if (command > 6 && kissFrame[1] != 0xFF) throw std::runtime_error("Invalid type byte");

	size_t unescapedPayloadSize = kissFrame.size() - 3;
	for (size_t j = 2; j < kissFrame.size() - 1; j++) {
		if (kissFrame[j] == 0xDB) {
			unescapedPayloadSize -= 1;
			j++;
		}
	}

	std::vector<uint8_t> payload(unescapedPayloadSize);
	size_t i = 0;

	for (size_t j = 2; j < kissFrame.size() - 1; j++) {
		if (kissFrame[j] == 0xC0) break;

		if (kissFrame[j] == 0xDB && j + 1 < kissFrame.size()) {
			if (kissFrame[j + 1] == 0xDD) payload[i++] = 0xDB;
			else if (kissFrame[j + 1] == 0xDC) payload[i++] = 0xC0;
			else throw std::runtime_error("KISS Decode Error: Malformed escape sequence following 0xDB");
			j++;
		} else {
			payload[i++] = kissFrame[j];
		}
	}

	return {.port = port, .command = command, .payload = payload};
}
