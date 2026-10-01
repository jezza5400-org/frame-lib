#include "frame_lib/ax25.hpp"
#include <algorithm>
#include <cctype>
#include <iterator>
#include <stdexcept>

std::string AX25::normaliseCallsign(std::string callsign) {
	if (callsign.size() > 6) throw std::invalid_argument("AX.25 CALLSIGN must be <= 6 characters");

	callsign.resize(6, ' ');

	std::transform(callsign.begin(), callsign.end(), callsign.begin(), [](unsigned char c) { return static_cast<char>(std::toupper(c)); });

	return callsign;
}

std::vector<uint8_t> AX25::encode(const std::string& destCall, const uint8_t destSSID, const std::string& srcCall, const uint8_t srcSSID, const std::string& payload) {
	if (destSSID > 15 || srcSSID > 15) throw std::invalid_argument("AX.25 SSID must be 0-15");

	auto dest = normaliseCallsign(destCall);
	auto src = normaliseCallsign(srcCall);

	std::vector<uint8_t> frame;
	frame.reserve(16 + payload.length());

	std::transform(dest.begin(), dest.end(), std::back_inserter(frame), [](unsigned char c) { return static_cast<uint8_t>(c) << 1; });

	// Address SSID byte: CRRSSSSL (Command/Response bit, reserved bits, SSID, is-last-address? bit)
	frame.push_back(0b11100000 | (destSSID << 1));

	std::transform(src.begin(), src.end(), std::back_inserter(frame), [](unsigned char c) { return static_cast<uint8_t>(c) << 1; });

	frame.push_back(0b01100001 | (srcSSID << 1));

	frame.push_back(0x03);
	frame.push_back(0xf0);

	for (char c : payload) {
		frame.push_back(static_cast<uint8_t>(c));
	}

	return frame;
}

std::vector<uint8_t> AX25::encode(const AX25Packet& packet) {
	return encode(packet.destCall, packet.destSSID, packet.srcCall, packet.srcSSID, packet.payload);
}
