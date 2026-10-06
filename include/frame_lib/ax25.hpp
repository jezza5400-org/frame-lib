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
	static std::vector<uint8_t> encode(const std::string& destCall, uint8_t destSSID, const std::string& srcCall, uint8_t srcSSID, const std::string& payload);
	static std::vector<uint8_t> encode(const AX25Packet& packet);
	static AX25Packet decode(const std::vector<uint8_t>& frame);

  private:
	static std::string normaliseCallsign(std::string callsign);
};
