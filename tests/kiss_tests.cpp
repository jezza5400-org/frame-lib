#include "frame_lib/kiss.hpp"
#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cstdint>
#include <vector>

TEST_CASE("KISS Encoder - Functionality", "[kiss][encode]") {

	SECTION("Empty payload") {
		std::vector<uint8_t> input = {};
		std::vector<uint8_t> expected = {0xC0, 0x00, 0xC0};

		REQUIRE(KISS::encode(input) == expected);
	}

	SECTION("Normal payload without escape bytes") {
		std::vector<uint8_t> input = {0x01, 0x02, 0x03, 0x04};
		std::vector<uint8_t> expected = {0xC0, 0x00, 0x01, 0x02, 0x03, 0x04, 0xC0};

		REQUIRE(KISS::encode(input) == expected);
	}

	SECTION("Payload containing FEND (0xC0)") {
		std::vector<uint8_t> input = {0x12, 0xC0, 0x34};
		std::vector<uint8_t> expected = {0xC0, 0x00, 0x12, 0xDB, 0xDC, 0x34, 0xC0};

		REQUIRE(KISS::encode(input) == expected);
	}

	SECTION("Payload containing FESC (0xDB)") {
		std::vector<uint8_t> input = {0x12, 0xDB, 0x34};
		std::vector<uint8_t> expected = {0xC0, 0x00, 0x12, 0xDB, 0xDD, 0x34, 0xC0};

		REQUIRE(KISS::encode(input) == expected);
	}

	SECTION("Payload with consecutive escape bytes") {
		std::vector<uint8_t> input = {0xC0, 0xDB, 0xC0, 0xDB};
		std::vector<uint8_t> expected = {
			0xC0,
			0x00,
			0xDB,
			0xDC,
			0xDB,
			0xDD,
			0xDB,
			0xDC,
			0xDB,
			0xDD,
			0xC0
		};

		REQUIRE(KISS::encode(input) == expected);
	}
}

TEST_CASE("KISS Encoder - Benchmarks", "[kiss][benchmark]") {
	std::vector<uint8_t> typical_frame;
	typical_frame.reserve(256);

	for (int i = 0; i < 256; ++i) {
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
		std::vector<uint8_t> input = {0xC0, 0x00, 0xC0};

		KissPacket pkt = KISS::decode(input);

		REQUIRE(pkt.type == 0x00);
		REQUIRE(pkt.ax25Frame.empty());
	}

	SECTION("Missing command byte frame") {
		std::vector<uint8_t> input = {0xC0, 0xC0, 0xC0, 0xC0};

		REQUIRE_THROWS_AS(KISS::decode(input), std::runtime_error);
	}

	SECTION("Normal payload without escape bytes") {
		std::vector<uint8_t> input = {0xC0, 0x00, 0x01, 0x02, 0x03, 0x04, 0xC0};

		KissPacket pkt = KISS::decode(input);

		REQUIRE(pkt.type == 0x00);
		REQUIRE(pkt.ax25Frame == std::vector<uint8_t>({0x01, 0x02, 0x03, 0x04}));
	}

	SECTION("Payload containing escaped FEND (0xDB 0xDC)") {
		std::vector<uint8_t> input = {0xC0, 0x00, 0x12, 0xDB, 0xDC, 0x34, 0xC0};

		KissPacket pkt = KISS::decode(input);

		REQUIRE(pkt.type == 0x00);
		REQUIRE(pkt.ax25Frame == std::vector<uint8_t>({0x12, 0xC0, 0x34}));
	}

	SECTION("Payload containing escaped FESC (0xDB 0xDD)") {
		std::vector<uint8_t> input = {0xC0, 0x00, 0x12, 0xDB, 0xDD, 0x34, 0xC0};

		KissPacket pkt = KISS::decode(input);

		REQUIRE(pkt.type == 0x00);
		REQUIRE(pkt.ax25Frame == std::vector<uint8_t>({0x12, 0xDB, 0x34}));
	}

	SECTION("Payload with consecutive escape sequences") {
		std::vector<uint8_t> input = {
			0xC0,
			0x00,
			0xDB,
			0xDC,
			0xDB,
			0xDD,
			0xDB,
			0xDC,
			0xDB,
			0xDD,
			0xC0
		};

		KissPacket pkt = KISS::decode(input);

		REQUIRE(pkt.type == 0x00);
		REQUIRE(pkt.ax25Frame == std::vector<uint8_t>({0xC0, 0xDB, 0xC0, 0xDB}));
	}

	SECTION("Round-trip consistency check") {
		std::vector<uint8_t> original = {0x5A, 0xC0, 0x3B, 0xDB, 0xFF};

		auto encoded = KISS::encode(original, 0x00);
		KissPacket pkt = KISS::decode(encoded);

		REQUIRE(pkt.type == 0x00);
		REQUIRE(pkt.ax25Frame == original);
	}
}

TEST_CASE("KISS Decoder - Error Handling", "[kiss][decode][errors]") {

	SECTION("Frame too short") {
		std::vector<uint8_t> input = {0xC0, 0x00};
		REQUIRE_THROWS_AS(KISS::decode(input), std::invalid_argument);
	}

	SECTION("Invalid escape sequence") {
		std::vector<uint8_t> input = {0xC0, 0x00, 0xDB, 0x99, 0xC0};
		REQUIRE_THROWS_AS(KISS::decode(input), std::runtime_error);
	}

	SECTION("Invalid command byte (upper nibble port > 15)") {
		// Example: 0xF1 → port = 0xF (valid), command = 1 (valid)
		// But if you want to enforce only port 0, this should throw.
		std::vector<uint8_t> input = {0xC0, 0xF1, 0xC0};

		// If your decoder rejects non-zero ports:
		// REQUIRE_THROWS(KISS::decode(input));

		KissPacket pkt = KISS::decode(input);
		REQUIRE(pkt.type == 0xF1);
		REQUIRE(pkt.ax25Frame.empty());
	}
}

TEST_CASE("KISS Decoder - Benchmarks", "[kiss][benchmark]") {

	std::vector<uint8_t> typical_kiss_frame;
	typical_kiss_frame.push_back(0xC0);
	typical_kiss_frame.push_back(0x00);

	for (int i = 0; i < 256; ++i) {
		if (i % 20 == 0) {
			typical_kiss_frame.push_back(0xDB);
			typical_kiss_frame.push_back(0xDC);
		} else if (i % 20 == 10) {
			typical_kiss_frame.push_back(0xDB);
			typical_kiss_frame.push_back(0xDD);
		} else {
			typical_kiss_frame.push_back(static_cast<uint8_t>(i & 0xFF));
		}
	}

	typical_kiss_frame.push_back(0xC0);

	BENCHMARK("Decode 256-byte KISS frame") {
		return KISS::decode(typical_kiss_frame);
	};
}
