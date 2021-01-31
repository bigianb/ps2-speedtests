// GSTransferTests.cpp : Defines the entry point for the application.
//
#include <chrono>
#include "GSTransferTests.h"

using namespace std;

extern void runBaseline();
extern void runSSEVersion();

int main()
{
	cout << "Hello CMake." << endl;
	cout << "Clock period: " << chrono::high_resolution_clock::period::den << endl;

	auto startTime = chrono::high_resolution_clock::now();
	runBaseline();
	auto endTime = chrono::high_resolution_clock::now();

	cout << "Elapsed: " << chrono::duration_cast<chrono::microseconds>(endTime - startTime).count() << " us" << endl;

	startTime = chrono::high_resolution_clock::now();
	runSSEVersion();
	endTime = chrono::high_resolution_clock::now();

	cout << "Elapsed: " << chrono::duration_cast<chrono::microseconds>(endTime - startTime).count() << " us" << endl;
	cout << "Done." << endl;

	return 0;
}

