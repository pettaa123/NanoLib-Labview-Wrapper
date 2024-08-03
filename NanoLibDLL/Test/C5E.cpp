#include "C5E.h"
#include "../NanoLibDLL/nanolibwrapper.h"

C5E::C5E() {};
C5E::~C5E() {};

int C5E::Init() {

	std::vector<std::string> exceptions;

	std::vector<std::string> ports;
	if (getPorts(ports)) {
		getExceptions(exceptions);
	}


	return EXIT_SUCCESS;
}