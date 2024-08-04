#include "C5E.h"

int main() {
	std::vector<std::string> exceptions;
	C5E MotorController("USB Bus protocol: MSC");
	if (MotorController.Init()) {
		MotorController.GetExceptions(exceptions);
	}
	if (MotorController.Start()) {
		MotorController.GetExceptions(exceptions);
	}
	if (MotorController.Home()) {
		MotorController.GetExceptions(exceptions);
	}
	//GENERATE LV EVENT
	if (MotorController.Jog()) {
		MotorController.GetExceptions(exceptions);
	}
	//EVENT
	if (MotorController.Stop()) {
		MotorController.GetExceptions(exceptions);
	}
	if (MotorController.Close()) {
		MotorController.GetExceptions(exceptions);
	}
	return 0;
}

