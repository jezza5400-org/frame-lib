#include "frame_lib/ax25.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstddef>
#include <stdexcept>
#include <string_view>

std::array<char, 6> AX25::normaliseCallsign(std::string_view callsign) {
	if (callsign.size() > 6) throw std::invalid_argument("AX.25 CALLSIGN must be <= 6 characters");

	std::array<char, 6> result;

	for (std::size_t i = 0; i < 6; ++i) {
		if (i < callsign.size()) {
			result[i] = static_cast<char>(std::toupper(static_cast<unsigned char>(callsign[i])));
		} else {
			result[i] = ' ';
		}
	}

	return result;
}

std::vector<uint8_t> AX25::encode(
	std::string_view destCall,
	uint8_t destSSID,
	std::string_view srcCall,
	uint8_t srcSSID,
	std::string_view payload
) {
	if (destSSID > 15 || srcSSID > 15) throw std::invalid_argument("AX.25 SSID must be 0-15");

	if (destCall.size() > 6 || srcCall.size() > 6) throw std::invalid_argument("AX.25 CALLSIGN must be <= 6 characters");

	// 7 bytes destination + 7 bytes source + 2 bytes control/PID.
	std::vector<uint8_t> frame(16 + payload.size());

	std::size_t i = 0;

	// Destination callsign
	for (std::size_t j = 0; j < 6; ++j) {
		const unsigned char c = j < destCall.size() ? static_cast<unsigned char>(destCall[j]) : static_cast<unsigned char>(' ');

		frame[i++] = static_cast<uint8_t>(std::toupper(static_cast<unsigned char>(c))) << 1;
	}

	// Destination SSID. CRRSSSSL: C = 1, RR = 11, SS = SSID, L = 0
	frame[i++] = 0b11100000 | (destSSID << 1);

	// Source callsign
	for (std::size_t j = 0; j < 6; ++j) {
		const unsigned char c = j < srcCall.size() ? static_cast<unsigned char>(srcCall[j]) : static_cast<unsigned char>(' ');

		frame[i++] = static_cast<uint8_t>(std::toupper(static_cast<unsigned char>(c))) << 1;
	}

	// Source SSID. CRRSSSSL: C = 0, RR = 11, SS = SSID, L = 1
	frame[i++] = 0b01100001 | (srcSSID << 1);

	// UI frame
	frame[i++] = 0x03;

	// No layer 3 protocol
	frame[i++] = 0xF0;

	// Payload
	std::ranges::copy(payload, frame.begin() + static_cast<std::vector<uint8_t>::difference_type>(i));

	return frame;
}

std::vector<uint8_t> AX25::encode(const AX25Packet& packet) {
	return encode(
		packet.destCall,
		packet.destSSID,
		packet.srcCall,
		packet.srcSSID,
		packet.payload
	);
}
