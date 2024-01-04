#include "userUnits.h"

std::string UNITS::getStringFromUserUnits(uint32_t key) {
	auto it = std::find_if(USERUNIT.begin(), USERUNIT.end(),
		[&](const std::pair<uint32_t, const char*>& element) {
			return element.first == key;
		});

	if (it != USERUNIT.end()) {
		return it->second;
	}
	else {
		return "Key not found";
	}
}

std::string UNITS::getStringFromUserUnitsExp(uint32_t key) {
	auto it = std::find_if(USERUNIT_EXP.begin(), USERUNIT_EXP.end(),
		[&](const std::pair<uint32_t, const char*>& element) {
			return element.first == key;
		});

	if (it != USERUNIT_EXP.end()) {
		return it->second;
	}
	else {
		return "Key not found";
	}
};

std::string UNITS::getStringFromUserUnitsTime(uint32_t key) {
	auto it = std::find_if(USERUNIT_TIME.begin(), USERUNIT_TIME.end(),
		[&](const std::pair<uint32_t, const char*>& element) {
			return element.first == key;
		});

	if (it != USERUNIT_TIME.end()) {
		return it->second;
	}
	else {
		return "Key not found";
	}
};
