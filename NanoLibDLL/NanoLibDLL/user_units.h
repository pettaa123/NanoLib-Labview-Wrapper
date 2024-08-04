#pragma once


#include <array>
#include <string>

namespace  UNITS {

	//User units

	constexpr std::array<std::pair<const uint32_t, const char*>, 9> kUserUnit{
	{{0x01,"meter"},
	{0xC1,"inch"},
	{0xC2, "foot"},
	{0x40, "grade"},
	{0x10, "radian"},
	{0x41, "degree"},
	{0x42, "arcminute"},
	{0x43, "arcsecond"},
	{0xB4, "revs"}}
	};

	//Exponent positioning and velocity units

	constexpr std::array<std::pair<const uint32_t, const char*>, 13> kUserUnitExp{
	{{0x06,	"6"},
	{0x05,	"5"},
	{0x04,	"4"},
	{0x03,	"3"},
	{0x02,	"2"},
	{0x01,	"1"},
	{0x00,	"0"},
	{0xFF, "-1"},
	{0xFE, "-2"},
	{0xFD, "-3"},
	{0xFC, "-4"},
	{0xFB, "-5"},
	{0xFA, "-6"}}
	};

	//Time units

	constexpr std::array<std::pair<const uint32_t, const char*>, 5> kUserUnitTime{
	{{0x03,	"second"},
	{0x47,	"minute"},
	{0x48,	"hour"},
	{0x49,	"day"},
	{0x4A, "year"}}
	};


	// Function to get the key by pair value
	constexpr uint32_t GetUserUnitKeyByValue(const char* value) {
		auto it = std::find_if(kUserUnit.begin(), kUserUnit.end(),
			[value](const std::pair<const uint32_t, const char*>& pair) {
				return std::strcmp(pair.second, value) == 0;
			});

		return (it != kUserUnit.end()) ? it->first : 0; // Return 0 if not found (assuming 0 is not a valid key)
	}

	constexpr uint32_t GetUserUnitExpKeyByValue(const char* value) {
		auto it = std::find_if(kUserUnitExp.begin(), kUserUnitExp.end(),
			[value](const std::pair<const uint32_t, const char*>& pair) {
				return std::strcmp(pair.second, value) == 0;
			});

		return (it != kUserUnitExp.end()) ? it->first : 0; // Return 0 if not found (assuming 0 is not a valid key)
	}

	constexpr uint32_t GetUserUnitTimeKeyByValue(const char* value) {
		auto it = std::find_if(kUserUnitTime.begin(), kUserUnitTime.end(),
			[value](const std::pair<const uint32_t, const char*>& pair) {
				return std::strcmp(pair.second, value) == 0;
			});

		return (it != kUserUnitTime.end()) ? it->first : 0; // Return 0 if not found (assuming 0 is not a valid key)
	}

	std::string GetUserUnitString(uint32_t key);

	std::string GetUserUnitExpString(uint32_t key);

	std::string GetUserUnitTimeString(uint32_t key);

}
