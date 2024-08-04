#include "../NanoLibDLL/nanolibwrapper.h"
#include "C5E.h"

C5E::C5E(std::string busType) : 
busType_(busType),
homingDone_(false),
stopThread_(false) {
	pollingThread_= std::thread(&C5E::PollingThreadFunc, this);
}
C5E::~C5E() {
	stopThread_ = true;
	if (pollingThread_.joinable()) {
		pollingThread_.join();
	}
}

void C5E::SetHomingCallback(Callback callback) {
	std::lock_guard<std::mutex> lock(mtx);
	homingCallback = callback;
}
void C5E::PollingThreadFunc() {
	while (!stopThread_) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Polling interval

		if (IsHomingDone()) {
			std::lock_guard<std::mutex> lock(mtx);
			if (homingCallback) {
				homingCallback();
			}
			homingDone_ = false; // Reset homing done flag
		}
	}
}


int C5E::FindSubstringInVector(const std::vector<std::string>& vec, const std::string& substring) const {
	auto it = std::find_if(vec.begin(), vec.end(), [this](const std::string& str) {
		return str.find(this->busType_) != std::string::npos;
		});

	if (it != vec.end()) {
		return std::distance(vec.begin(), it);
	}
	else {
		return -1; // Return -1 if the substring is not found
	}
}

int C5E::GetExceptions(std::vector<std::string>& exceptions) {
	if (NanoLibWrapper::GetExceptions(exceptions)) {
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int C5E::Init() {

	std::vector<std::string> ports;
	if (NanoLibWrapper::GetPorts(ports)) {
		return EXIT_FAILURE;
	}
	int busIndexToOpen = FindSubstringInVector(ports, busType_);
	if (NanoLibWrapper::ScanBus(ports)) {
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
int C5E::Start() {

	std::vector<std::string> ports;
	if (NanoLibWrapper::GetPorts(ports)) {
		return EXIT_FAILURE;
	}
	int busIndexToOpen = FindSubstringInVector(ports, busType_);
	if (NanoLibWrapper::ScanBus(ports)) {
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int C5E::Stop() {
	std::vector<std::string> exceptions;

	std::vector<std::string> ports;
	if (NanoLibWrapper::GetPorts(ports)) {
		return EXIT_FAILURE;
	}
	int busIndexToOpen = FindSubstringInVector(ports, busType_);
	if (NanoLibWrapper::ScanBus(ports)) {
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int C5E::Jog() {
	std::vector<std::string> exceptions;

	std::vector<std::string> ports;
	if (NanoLibWrapper::GetPorts(ports)) {
		return EXIT_FAILURE;
	}
	int busIndexToOpen = FindSubstringInVector(ports, busType_);
	if (NanoLibWrapper::ScanBus(ports)) {
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int C5E::Home() {
	std::vector<std::string> exceptions;

	std::vector<std::string> ports;
	if (NanoLibWrapper::GetPorts(ports)) {
		return EXIT_FAILURE;
	}
	int busIndexToOpen = FindSubstringInVector(ports, busType_);
	if (NanoLibWrapper::ScanBus(ports)) {
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int C5E::Close() {
	std::vector<std::string> exceptions;

	std::vector<std::string> ports;
	if (NanoLibWrapper::GetPorts(ports)) {
		return EXIT_FAILURE;
	}
	int busIndexToOpen = FindSubstringInVector(ports, busType_);
	if (NanoLibWrapper::ScanBus(ports)) {
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
