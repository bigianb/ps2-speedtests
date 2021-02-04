// GSTransferTests.cpp : Defines the entry point for the application.
//
#include <chrono>
#include "GSTransferTests.h"

using namespace std;

typedef unsigned int uint32;
typedef unsigned char uint8;

extern void runBaseline(uint8* pCvtBuffer, uint8* pRAM);
extern void runSSEVersion(uint8* pCvtBuffer, uint8* pRAM);

enum CVTBUFFERSIZE
{
	CVTBUFFERSIZE = 0x800000,
};

enum RAMSIZE
{
	RAMSIZE = 0x00400000,
};

int main()
{
	cout << "Hello CMake." << endl;
	cout << "Clock period: " << chrono::high_resolution_clock::period::den << endl;

	uint8* pRAM = new uint8[RAMSIZE];
	uint8* pCvtBuffer = new uint8[CVTBUFFERSIZE];

	auto startTime = chrono::high_resolution_clock::now();
	runBaseline(pCvtBuffer, pRAM);
	auto endTime = chrono::high_resolution_clock::now();

	cout << "Elapsed: " << chrono::duration_cast<chrono::microseconds>(endTime - startTime).count() << " us" << endl;

	startTime = chrono::high_resolution_clock::now();
	runSSEVersion(pCvtBuffer, pRAM);
	endTime = chrono::high_resolution_clock::now();

	cout << "Elapsed: " << chrono::duration_cast<chrono::microseconds>(endTime - startTime).count() << " us" << endl;
	cout << "Done." << endl;

	delete[] pRAM;
	delete[] pCvtBuffer;

	return 0;
}

