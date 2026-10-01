#include "frame_lib/ax25.hpp"
#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>
#include <vector>

TEST_CASE("AX.25 Frame Encoded") {
	AX25 ax25;

	std::vector<uint8_t> expectedResult = {
		0xAC,
		0x96,
		0x66,
		0xB0,
		0xB2,
		0xB4,
		0xE0,
		0xAC,
		0x96,
		0x66,
		0x82,
		0x84,
		0x86,
		0x61,
		0x03,
		0xF0,
		0x74,
		0x65,
		0x73,
		0x74
	};

	REQUIRE(
		ax25.encode(
			"VK3XYZ",
			0,
			"VK3ABC",
			0,
			"test"
		) == expectedResult
	);
}

TEST_CASE("AX.25 Invalid SSID") {
	AX25 ax25;

	REQUIRE_THROWS_AS(
		ax25.encode("VK3XYZ", 16, "VK3ABC", 0, "test"),
		std::invalid_argument
	);

	REQUIRE_THROWS_AS(
		ax25.encode("VK3XYZ", 0, "VK3ABC", 16, "test"),
		std::invalid_argument
	);
}

TEST_CASE("AX.25 Callsign Too Long") {
	AX25 ax25;

	REQUIRE_THROWS_AS(
		ax25.encode("VK3XYZ7", 0, "VK3ABC", 0, "test"),
		std::invalid_argument
	);

	REQUIRE_THROWS_AS(
		ax25.encode("VK3XYZ", 0, "VK3ABC7", 0, "test"),
		std::invalid_argument
	);
}

TEST_CASE("AX.25 Short Callsign Padding") {
	AX25 ax25;

	auto result = ax25.encode(
		"VK3",
		0,
		"ABC",
		0,
		"test"
	);

	REQUIRE(result.size() == 20);
}

TEST_CASE("AX.25 Packet Struct Encoding") {
	AX25 ax25;

	AX25Packet packet{
		.destCall = "VK3XYZ",
		.destSSID = 0,
		.srcCall = "VK3ABC",
		.srcSSID = 0,
		.payload = "test"
	};

	REQUIRE(
		ax25.encode(packet) ==
		ax25.encode(
			packet.destCall,
			packet.destSSID,
			packet.srcCall,
			packet.srcSSID,
			packet.payload
		)
	);
}

// ---------------- BENCHMARKS ----------------

TEST_CASE("AX.25 Encode Benchmark Small Packet", "[!benchmark]") {
	AX25 ax25;

	BENCHMARK("Small UI frame") {
		return ax25.encode(
			"VK3XYZ",
			0,
			"VK3ABC",
			0,
			"test"
		);
	};
}

TEST_CASE("AX.25 Encode Benchmark Maximum Callsign", "[!benchmark]") {
	AX25 ax25;

	BENCHMARK("6 character callsigns") {
		return ax25.encode(
			"VK3XYZ",
			15,
			"VK3ABC",
			15,
			"test"
		);
	};
}

TEST_CASE("AX.25 Encode Benchmark Short Callsigns", "[!benchmark]") {
	AX25 ax25;

	BENCHMARK("Calls with padding") {
		return ax25.encode(
			"VK3",
			0,
			"ABC",
			0,
			"test"
		);
	};
}

TEST_CASE("AX.25 Encode Benchmark Long Payload", "[!benchmark]") {
	AX25 ax25;

	std::string payload(256, 'A');

	BENCHMARK("256 byte payload") {
		return ax25.encode(
			"VK3XYZ",
			0,
			"VK3ABC",
			0,
			payload
		);
	};
}

TEST_CASE("AX.25 Encode Benchmark Packet Struct", "[!benchmark]") {
	AX25 ax25;

	AX25Packet packet{
		.destCall = "VK3XYZ",
		.destSSID = 0,
		.srcCall = "VK3ABC",
		.srcSSID = 0,
		.payload = "This is a test AX.25 packet"
	};

	BENCHMARK("AX25Packet overload") {
		return ax25.encode(packet);
	};
}
