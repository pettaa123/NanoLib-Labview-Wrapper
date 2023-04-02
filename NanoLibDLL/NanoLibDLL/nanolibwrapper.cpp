#include "pch.h"
#include "nanolibwrapper.h"
#include "controller.h"
#include <iostream>

int getPorts(char** list, int rows, int cols) {

	Controller c;
	std::vector<std::string> ports;
	int status = c.getAvailablePorts(ports);
	if (status != EXIT_SUCCESS)
		return status;

	for (int i = 0; (i < rows) && (i < ports.size()); i++) {
		if (strlen(ports[i].c_str()) > cols)
			return 2; //NOT ENOUGH COLS
		strcpy_s(list[i], cols, ports[i].c_str());
	}

	return EXIT_SUCCESS;

}


int getPortsLV(LStrHandleArray* arr, int rows, int cols) {

	Controller c;
	std::vector<std::string> ports;
	int status = c.getAvailablePorts(ports);
	if (status != EXIT_SUCCESS)
		return status;

	for (int i = 0; (i < rows) && (i < ports.size()); i++) {
		if (strlen(ports[i].c_str()) > cols)
			return 2; //NOT ENOUGH COLS
		//copy dll strings to labview str array
		//missing resize of labview array
		//this works
		strcpy_s((char*)(*arr[i])->String-4, cols, ports[i].c_str());
		//this not
		(*((*arr[i])->String))
	}

	return EXIT_SUCCESS;
}