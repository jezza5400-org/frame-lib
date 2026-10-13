#pragma once
#include <cstdint>
#include <vector>

struct KissPacket {
	uint8_t type;
	std::vector<uint8_t> ax25Frame;
};

class KISS {
  public:
	static std::vector<uint8_t> encode(const std::vector<uint8_t>& ax25Frame, uint8_t type = 0x00);
	static KissPacket decode(const std::vector<uint8_t>& kissFrame);
};
