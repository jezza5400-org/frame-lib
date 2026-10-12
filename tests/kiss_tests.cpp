#include "frame_lib/kiss.hpp"
#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cstdint>
#include <vector>

TEST_CASE("KISS Encoder - Functionality", "[kiss][encode]") {

	SECTION("Empty payload") {
		// An empty payload should only contain FEND, Command 0, and FEND
		std::vector<uint8_t> input = {};
		std::vector<uint8_t> expected = {0xC0, 0x00, 0xC0};

		REQUIRE(KISS::encode(input) == expected);
	}

	SECTION("Normal payload without escape bytes") {
		// Standard data bytes (e.g., ASCII characters or random safe data)
		std::vector<uint8_t> input = {0x01, 0x02, 0x03, 0x04};
		std::vector<uint8_t> expected = {0xC0, 0x00, 0x01, 0x02, 0x03, 0x04, 0xC0};

		REQUIRE(KISS::encode(input) == expected);
	}

	SECTION("Payload containing FEND (0xC0)") {
		// 0xC0 must escape into 0xDB 0xDC
		std::vector<uint8_t> input = {0x12, 0xC0, 0x34};
		std::vector<uint8_t> expected = {0xC0, 0x00, 0x12, 0xDB, 0xDC, 0x34, 0xC0};

		REQUIRE(KISS::encode(input) == expected);
	}

	SECTION("Payload containing FESC (0xDB)") {
		// 0xDB must escape into 0xDB 0xDD
		std::vector<uint8_t> input = {0x12, 0xDB, 0x34};
		std::vector<uint8_t> expected = {0xC0, 0x00, 0x12, 0xDB, 0xDD, 0x34, 0xC0};

		REQUIRE(KISS::encode(input) == expected);
	}

	SECTION("Payload with consecutive escape bytes") {
		// Multiple escape scenarios together
		std::vector<uint8_t> input = {0xC0, 0xDB, 0xC0, 0xDB};
		std::vector<uint8_t> expected = {
			0xC0,
			0x00,  // Header
			0xDB,
			0xDC,  // Escaped 0xC0
			0xDB,
			0xDD,  // Escaped 0xDB
			0xDB,
			0xDC,  // Escaped 0xC0
			0xDB,
			0xDD,  // Escaped 0xDB
			0xC0   // Footer
		};

		REQUIRE(KISS::encode(input) == expected);
	}
}

TEST_CASE("KISS Encoder - Benchmarks", "[kiss][benchmark]") {
	// Generate a typical dynamic payload size (e.g., 256 bytes) to benchmark
	std::vector<uint8_t> typical_frame;
	typical_frame.reserve(256);
	for (int i = 0; i < 256; ++i) {
		// Mix regular bytes with some escape bytes to trigger code branches
		if (i % 20 == 0) typical_frame.push_back(0xC0);
		else if (i % 20 == 10) typical_frame.push_back(0xDB);
		else typical_frame.push_back(static_cast<uint8_t>(i & 0xFF));
	}

	BENCHMARK("Encode 256-byte AX.25 frame") {
		return KISS::encode(typical_frame);
	};
}

TEST_CASE("KISS Decoder - Functionality", "[kiss][decode]") {

	SECTION("Empty payload frame") {
		// A minimal KISS frame containing only FEND, Command (0x00), and FEND
		std::vector<uint8_t> input = {0xC0, 0x00, 0xC0};
		std::vector<uint8_t> expected = {};

		REQUIRE(KISS::decode(input) == expected);
	}

	SECTION("Normal payload without escape bytes") {
		std::vector<uint8_t> input = {0xC0, 0x00, 0x01, 0x02, 0x03, 0x04, 0xC0};
		std::vector<uint8_t> expected = {0x01, 0x02, 0x03, 0x04};

		REQUIRE(KISS::decode(input) == expected);
	}

	SECTION("Payload containing escaped FEND (0xDB 0xDC)") {
		// 0xDB 0xDC must unescape back into 0xC0
		std::vector<uint8_t> input = {0xC0, 0x00, 0x12, 0xDB, 0xDC, 0x34, 0xC0};
		std::vector<uint8_t> expected = {0x12, 0xC0, 0x34};

		REQUIRE(KISS::decode(input) == expected);
	}

	SECTION("Payload containing escaped FESC (0xDB 0xDD)") {
		// 0xDB 0xDD must unescape back into 0xDB
		std::vector<uint8_t> input = {0xC0, 0x00, 0x12, 0xDB, 0xDD, 0x34, 0xC0};
		std::vector<uint8_t> expected = {0x12, 0xDB, 0x34};

		REQUIRE(KISS::decode(input) == expected);
	}

	SECTION("Payload with consecutive escape sequences") {
		std::vector<uint8_t> input = {
			0xC0,
			0x00,  // Header
			0xDB,
			0xDC,  // Unescapes to 0xC0
			0xDB,
			0xDD,  // Unescapes to 0xDB
			0xDB,
			0xDC,  // Unescapes to 0xC0
			0xDB,
			0xDD,  // Unescapes to 0xDB
			0xC0   // Footer
		};
		std::vector<uint8_t> expected = {0xC0, 0xDB, 0xC0, 0xDB};

		REQUIRE(KISS::decode(input) == expected);
	}

	SECTION("Round-trip consistency check") {
		// Ensures encode -> decode yields the original AX.25 payload
		std::vector<uint8_t> original = {0x5A, 0xC0, 0x3B, 0xDB, 0xFF};
		uint8_t type = 0x00;

		auto encoded = KISS::encode(original, type);
		auto decoded = KISS::decode(encoded);

		REQUIRE(decoded == original);
	}
}

TEST_CASE("KISS Decoder - Benchmarks", "[kiss][benchmark]") {
	// Construct a pre-encoded KISS frame matching the benchmark setup size (~256 bytes)
	std::vector<uint8_t> typical_kiss_frame;
	typical_kiss_frame.push_back(0xC0);
	typical_kiss_frame.push_back(0x00);

	for (int i = 0; i < 256; ++i) {
		if (i % 20 == 0) {
			typical_kiss_frame.push_back(0xDB);
			typical_kiss_frame.push_back(0xDC);  // Escaped 0xC0
		} else if (i % 20 == 10) {
			typical_kiss_frame.push_back(0xDB);
			typical_kiss_frame.push_back(0xDD);  // Escaped 0xDB
		} else {
			typical_kiss_frame.push_back(static_cast<uint8_t>(i & 0xFF));
		}
	}
	typical_kiss_frame.push_back(0xC0);

	BENCHMARK("Decode 256-byte KISS frame") {
		return KISS::decode(typical_kiss_frame);
	};
}
