#include "user_units.h"

std::string UNITS::GetUserUnitString(uint32_t key) {
	auto it = std::find_if(kUserUnit.begin(), kUserUnit.end(),
		[&](const std::pair<uint32_t, const char*>& element) {
			return element.first == key;
		});

	if (it != kUserUnit.end()) {
		return it->second;
	}
	else {
		return "Key not found";
	}
}

std::string UNITS::GetUserUnitExpString(uint32_t key) {
	auto it = std::find_if(kUserUnitExp.begin(), kUserUnitExp.end(),
		[&](const std::pair<uint32_t, const char*>& element) {
			return element.first == key;
		});

	if (it != kUserUnitExp.end()) {
		return it->second;
	}
	else {
		return "Key not found";
	}
};

std::string UNITS::GetUserUnitTimeString(uint32_t key) {
	auto it = std::find_if(kUserUnitTime.begin(), kUserUnitTime.end(),
		[&](const std::pair<uint32_t, const char*>& element) {
			return element.first == key;
		});

	if (it != kUserUnitTime.end()) {
		return it->second;
	}
	else {
		return "Key not found";
	}
};
