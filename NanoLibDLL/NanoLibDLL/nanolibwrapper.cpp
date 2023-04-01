#include "pch.h"
#include "nanolibwrapper.h"
#include "controller.h"


int getPorts() {

	Controller c;
	c.getAvailablePorts();
	return EXIT_SUCCESS;
	
}
