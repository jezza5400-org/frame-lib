#pragma once
#include <cstdint>
#include <string>
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
	std::vector<uint8_t> encode(const std::string& destCall, const uint8_t destSSID, const std::string& srcCall, const uint8_t srcSSID, const std::string& payload);
	std::vector<uint8_t> encode(const AX25Packet& frame);
	AX25Packet decode(const std::vector<uint8_t>& frame);

  private:
	std::string normaliseCallsign(std::string callsign);
};
