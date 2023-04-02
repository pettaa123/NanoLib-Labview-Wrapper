// NanoLibDLLTest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "nanolibwrapper.h"

int main()
{
	char* list[20];
	for (int i = 0; i < 20; i++)
		list[i] = new char[100];
	
	int success = getPorts(list,20,100);

	for (int i = 0; i < 20; i++)
		std::cout << list[i] << std::endl;


	//free
	for (int i = 0; i < 20; i++)
		delete[] list[i];
	
}


