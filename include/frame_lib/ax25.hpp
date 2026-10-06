#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

struct AX25Packet {
	std::string destCall;
	uint8_t destSSID = 0;
	std::string srcCall;
	uint8_t srcSSID = 0;
	std::string payload;
};

class AX25 {
  public:
	static std::vector<uint8_t> encode(std::string_view destCall, uint8_t destSSID, std::string_view srcCall, uint8_t srcSSID, std::string_view payload);
	static std::vector<uint8_t> encode(const AX25Packet& packet);
	static AX25Packet decode(const std::vector<uint8_t>& frame);

  private:
	static std::array<char, 6> normaliseCallsign(std::string_view callsign);
};
