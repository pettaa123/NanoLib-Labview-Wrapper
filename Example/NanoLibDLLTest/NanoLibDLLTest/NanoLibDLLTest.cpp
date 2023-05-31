// NanoLibDLLTest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "nanolibwrapper.h"
#include <conio.h>

int main()
{
	// list all hardware available


	std::vector<std::string> busHardwareIds;

	unsigned int* zeroP = 0;
	unsigned int** zeroPP = &zeroP;
	void** cPtr=(void**)zeroPP;

	if (getPorts(cPtr, busHardwareIds))
		return EXIT_FAILURE;

	if (busHardwareIds.empty()) {
		std::cout << "No hardware buses found." << std::endl;
		return EXIT_SUCCESS;
	}

	std::cout << std::endl << "Available hardware buses and protocols:" << std::endl;

	unsigned lineNum = 0;
	// print out available hardware
	for (const auto& busHwId : busHardwareIds) {
		std::cout << lineNum << ". " << busHwId << std::endl;
		lineNum++;
	}

	if (lineNum > 1) {

		std::cout << std::endl
			<< "Type bus hardware number (0-" << (lineNum - 1) << ") and press Enter:";
		std::cin >> lineNum;
		std::cout << std::endl;

		if (lineNum >= busHardwareIds.size()) {
			std::cerr << "Invalid bus hardware number." << std::endl;
			return EXIT_FAILURE;
		}

	}
	else {
		lineNum = 0;
		std::cout << std::endl;
	}


	openPort(cPtr, lineNum);


	// Scan the bus for available devices
	std::cout << "Scanning bus for devices..." << std::endl;

	std::vector<std::string> deviceIds;
		
	scanBus(cPtr,deviceIds);

	if (deviceIds.empty()) {
		std::cout << std::endl << "No devices found." << std::endl;
		return EXIT_SUCCESS;
	}

	std::cout << std::endl << "Available devices:" << std::endl << std::endl;
	lineNum = 0;
	// print out available devices
	for (const auto& deviceId : deviceIds) {
		std::cout << lineNum << ". " << deviceId << std::endl;
		lineNum++;
	}

	if (lineNum > 1) {

		std::cout << std::endl
			<< "Type device number (0-" << (lineNum - 1) << ") and press Enter:";
		std::cin >> lineNum;
		std::cout << std::endl;

		if (lineNum >= deviceIds.size()) {
			std::cerr << "Invalid device number." << std::endl;
			return EXIT_FAILURE;
		}

	}
	else {
		lineNum = 0;
		std::cout << std::endl;
	}

	// Use selected device. 
	if (connectDevice(cPtr, lineNum))
		return EXIT_FAILURE;

	//if (autoSetupMotPams(cPtr))
	//	return EXIT_FAILURE;

	if (setMotorMaxCurrent(cPtr, 1000))
		return EXIT_FAILURE;

	if (setHomingAcceleration(cPtr, 100))
		return EXIT_FAILURE;

	if(home(cPtr, 110))
		return EXIT_FAILURE;
	//wait for user input
	_getch();

	if (setRPM(cPtr, 110))
		return EXIT_FAILURE;

	if (setProfileAcceleration(cPtr, 100))
		return EXIT_FAILURE;

	int target=0;

	while (target >= 0) {
		std::cout << "target: ";
		std::cin >> target;
		std::cout << std::endl;
		if (target >= 0)
			moveToDeciMM(cPtr, target);
	}

	if (closePort(cPtr))
		return EXIT_FAILURE;

	return EXIT_SUCCESS;


}


