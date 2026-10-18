#pragma once
#include <cstdint>
#include <vector>

// Represents a decoded KISS packet containing port, command, and payload.
struct KissPacket {
	uint8_t port;
	uint8_t command;
	std::vector<uint8_t> payload;
};

// Handles encoding and decoding of KISS.
class KISS {
  public:
	// Encodes a raw payload into a KISS frame.
	// Throws std::invalid_argument if port or command is invalid.
	static std::vector<uint8_t> encode(const std::vector<uint8_t>& payload, uint8_t port = 0x00, uint8_t command = 0x00);

	// Decodes a KISS frame into a KissPacket structure.
	// Throws std::invalid_argument or std::runtime_error on malformed frames.
	static KissPacket decode(const std::vector<uint8_t>& kissFrame);
};
