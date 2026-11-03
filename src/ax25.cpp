#include "frame_lib/ax25.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string_view>

std::array<uint8_t, 6> AX25::normaliseCallsign(std::string_view callsign) {
	std::array<uint8_t, 6> result;

	for (size_t i = 0; i < 6; ++i) {
		if (i < callsign.size()) {
			result[i] = static_cast<uint8_t>(std::toupper(static_cast<uint8_t>(callsign[i])) << 1);
		} else {
			result[i] = static_cast<uint8_t>(' ' << 1);
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
	if (destSSID > 15 || srcSSID > 15) throw std::invalid_argument("AX.25 SSID must be in range [0, 15]");
	if (destCall.size() > 6 || destCall.empty() || srcCall.size() > 6 || srcCall.empty()) throw std::invalid_argument("AX.25 CALLSIGN length must be in range [1, 6]");

	// 7 bytes destination + 7 bytes source + 2 bytes control/PID.
	std::vector<uint8_t> frame(16 + payload.size());

	size_t i = 0;

	// Destination callsign
	std::ranges::copy(normaliseCallsign(destCall), frame.begin());
	i += 6;

	// Destination SSID. CRRSSSSL: C = 1, RR = 11, SS = SSID, L = 0
	frame[i++] = 0b11100000 | (destSSID << 1);

	// Source callsign
	std::ranges::copy(normaliseCallsign(srcCall), frame.begin() + 7);
	i += 6;

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

AX25Packet AX25::decode(const std::vector<uint8_t>& frame) {
	if (frame.size() < 16) throw std::invalid_argument("AX.25 frame too short. Minimum length is 16 bytes");

	std::string destCall;
	for (size_t i = 0; i < 6; i++) {
		destCall += static_cast<char>(frame[i] >> 1);
	}

	uint8_t destSSID = (frame[6] >> 1) & 0b111;

	std::string srcCall;
	for (size_t i = 7; i < 13; i++) {
		srcCall += static_cast<char>(frame[i] >> 1);
	}

	uint8_t srcSSID = (frame[13] >> 1) & 0b111;

	std::string payload;
	for (size_t i = 16; i < frame.size(); i++) {
		payload += static_cast<char>(frame[i]);
	}

	return AX25Packet{.destCall = destCall, .destSSID = destSSID, .srcCall = srcCall, .srcSSID = srcSSID, .payload = payload};
}
