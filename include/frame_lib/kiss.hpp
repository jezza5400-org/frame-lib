#pragma once
#include <cstdint>
#include <vector>

class KISS {
  public:
	static std::vector<uint8_t> encode(const std::vector<uint8_t>& ax25Frame);
	static std::vector<uint8_t> decode(const std::vector<uint8_t>& kissFrame);
};
