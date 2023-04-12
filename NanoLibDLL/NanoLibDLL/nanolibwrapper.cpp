#include "pch.h"
#include "nanolibwrapper.h"
#include "controller.h"
#include <iostream>

int getPorts(char** list, int rows, int cols) {

	Controller c;
	std::vector<std::string> ports;
	int status = c.getAvailablePorts(ports);
	if (status != EXIT_SUCCESS)
		return EXIT_FAILURE;

	for (int i = 0; (i < rows) && (i < ports.size()); i++) {
		if (strlen(ports[i].c_str()) > cols)
			return 2; //NOT ENOUGH COLS
		strcpy_s(list[i], cols, ports[i].c_str());
	}

	return EXIT_SUCCESS;

}


MgErr getPortsLV(void **cPtr, LStrArrayHdl *arr) {
	MgErr err=0;

	if (!(uintptr_t)*cPtr) {
		*cPtr = new Controller();
	}
	Controller* c = static_cast<Controller*>(*cPtr);
	std::vector<std::string> ports;

	int status = c->getAvailablePorts(ports);
	if (status != EXIT_SUCCESS)
		return 501; //private error
	
	err = vecStrToLVStrArr(ports, arr);

	if (err) {
		closePortLV(cPtr);
	}
	
	return err;
}

MgErr openPort(void **cPtr, unsigned int portToOpen) {
	MgErr err=0;
	if (!(uintptr_t)*cPtr) {
		*cPtr = new Controller();

	}
	Controller* c = static_cast<Controller*>(*cPtr);

	err=c->openPort(portToOpen);	

	if (err) {
		closePortLV(cPtr);
	}
	return err;
}

MgErr scanBusLV(void** cPtr, LStrArrayHdl* LVAllocatedStrArray) {
	MgErr err = 0;
	if (!(uintptr_t)*cPtr) {
		*cPtr = new Controller();
	}
	Controller* c = static_cast<Controller*>(*cPtr);
	std::vector<std::string> devices;
	err = c->scanBus(devices);
	if (err == EXIT_SUCCESS)
		err = vecStrToLVStrArr(devices, LVAllocatedStrArray);
	if (err) {
		closePortLV(cPtr);
	}

	return err;
}

MgErr connectDevice(void** cPtr, unsigned int deviceToOpen) {
	MgErr err = 0;
	std::cout << err << std::endl;
	if (!(uintptr_t)*cPtr) {
		*cPtr = new Controller();
	}
	Controller* c = static_cast<Controller*>(*cPtr);
	std::cout << c << std::endl;
	err = c->connectDevice(deviceToOpen);
	std::cout << err << std::endl;
	if (err) {
		std::cout << err << std::endl;
		closePortLV(cPtr);
	}
	return err;
}

MgErr home(void** cPtr) {
	MgErr err = 0;
	if (!(uintptr_t)*cPtr) {
		*cPtr = new Controller();

	}
	Controller* c = static_cast<Controller*>(*cPtr);

	err = c->home();

	if (err) {
		closePortLV(cPtr);
	}
	return err;
}

MgErr stop(void** cPtr) {
	MgErr err = 0;
	if (!(uintptr_t)*cPtr) {
		*cPtr = new Controller();

	}
	Controller* c = static_cast<Controller*>(*cPtr);

	err = c->stop();

	if (err) {
		closePortLV(cPtr);
	}
	return err;
}

MgErr vecStrToLVStrArr(const std::vector<std::string>& s, LStrArrayHdl* arr) {
	MgErr err = 0;
	/* If the incoming array is bigger than what we need, we first need to loop through
	the superfluous items to see if they contain valid string handles and if so we
	need to deallocate them to avoid memory leaks */
	if (arr)
	{
		for (int i = static_cast<int>(std::size(s)); i < (**arr)->dimSize; i++)
		{
			LStrHandle* elt = &((**arr)->elt[i]);
			if (*elt)
			{
				err = DSDisposeHandle(*elt);
				*elt = NULL;
			}
		}
	}

	/* Now we need to make sure the StringArrayHandle is large enough to hold all ports */
	err = NumericArrayResize(uPtr, 1, (UHandle*)arr, std::size(s));
	for (int i = 0; !err && (i < std::size(s)); i++)
	{
		LStrHandle* elt = &((**arr)->elt[i]);
		// Resize string handle to be large enough to store the string data
		err = NumericArrayResize(uB, 1, (UHandle*)elt, s[i].length());
		if (!err)
		{
			MoveBlock(s[i].c_str(), LStrBuf(**elt), s[i].length());
			LStrLen(**elt) = static_cast<int>(s[i].length());
		}
	}
	if (!err)
	{
		(**arr)->dimSize = static_cast<int>(std::size(s));
	}
	return err;
}

MgErr closePortLV(void **cPtr) {
	MgErr err=0;
	if (!(uintptr_t)*cPtr) {
		Controller* c = static_cast<Controller*>(*cPtr);
		delete c;
	}
	*cPtr = NULL;
	return err;
}

MgErr autoSetupMotPams(void** cPtr) {
	MgErr err = 0;
	if (!(uintptr_t)*cPtr) {
		*cPtr = new Controller();
	}
	Controller* c = static_cast<Controller*>(*cPtr);
	err=c->autoSetupMotPams();
	
	if (err) {
		closePortLV(cPtr);
	}
	
	return err;
}

MgErr setRPM(void** cPtr,short rpm) {
	MgErr err = 0;
	if (!(uintptr_t)*cPtr) {
		*cPtr = new Controller();
	}
	Controller* c = static_cast<Controller*>(*cPtr);
	err = c->setProfileVelocity(rpm);

	if (err) {
		closePortLV(cPtr);
	}

	return err;
}

MgErr setFeedConstant(void** cPtr, unsigned int pitchZehntelMM) {
	MgErr err = 0;
	if (!(uintptr_t)*cPtr) {
		*cPtr = new Controller();
	}
	Controller* c = static_cast<Controller*>(*cPtr);
	err = c->setFeedConstant(pitchZehntelMM);

	if (err) {
		closePortLV(cPtr);
	}

	return err;
}

MgErr moveToDeciMM(void** cPtr, int deciMM) {
	MgErr err = 0;
	if (!(uintptr_t)*cPtr) {
		*cPtr = new Controller();
	}
	Controller* c = static_cast<Controller*>(*cPtr);
	std::cout << deciMM << std::endl;
	err = c->moveToPosition(deciMM);

	if (err) {
		closePortLV(cPtr);
	}

	return err;
}

