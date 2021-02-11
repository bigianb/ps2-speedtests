// GSTransferTests.cpp : Defines the entry point for the application.
//
#include <chrono>
#include <cstring>

#include "GSTransferTests.h"

using namespace std;

typedef unsigned int uint32;
typedef unsigned char uint8;

extern void runBaseline(uint8* pCvtBuffer, uint8* pRAM, int texW, int texH);
extern void runSSEVersion(uint8* pCvtBuffer, uint8* pRAM, int texW, int texH);

enum CVTBUFFERSIZE
{
	CVTBUFFERSIZE = 0x800000,
};

enum RAMSIZE
{
	RAMSIZE = 0x00400000,
};

// ensure output is as expected (matches baseline)
bool runTest(uint8* pRAM, uint8* pCvtBuffer)
{
	memset(pCvtBuffer, 0, CVTBUFFERSIZE);
	memset(pRAM, 0, RAMSIZE);
	for (int i = 0; i < 256; ++i) {
		pRAM[i] = i;
	}
	memset(pCvtBuffer, 0, CVTBUFFERSIZE);
	runBaseline(pCvtBuffer, pRAM, 16, 16);

	uint8* pCvtBuffer2 = new uint8[CVTBUFFERSIZE];
	memset(pCvtBuffer2, 0, CVTBUFFERSIZE);
	runSSEVersion(pCvtBuffer2, pRAM, 16, 16);

	bool match = true;
	for (int i = 0; i < 256; ++i) {
		if (pCvtBuffer[i] != pCvtBuffer2[i]) {
			match = false;
		}
	}

	delete[] pCvtBuffer2;

	return match;
}

int main()
{
	cout << "Hello CMake." << endl;
	cout << "Clock period: " << chrono::high_resolution_clock::period::den << endl;

	uint8* pRAM = new uint8[RAMSIZE];
	uint8* pCvtBuffer = new uint8[CVTBUFFERSIZE];
	if (runTest(pRAM, pCvtBuffer)) {
		auto startTime = chrono::high_resolution_clock::now();
		runBaseline(pCvtBuffer, pRAM, 512, 512);
		auto endTime = chrono::high_resolution_clock::now();

		cout << "Elapsed: " << chrono::duration_cast<chrono::microseconds>(endTime - startTime).count() << " us" << endl;

		startTime = chrono::high_resolution_clock::now();
		runSSEVersion(pCvtBuffer, pRAM, 512, 512);
		endTime = chrono::high_resolution_clock::now();

		cout << "Elapsed: " << chrono::duration_cast<chrono::microseconds>(endTime - startTime).count() << " us" << endl;
		cout << "Done." << endl;
	}
	else {
		cout << "Perf run terminated as reconciliation test failed" << endl;
	}
	delete[] pRAM;
	delete[] pCvtBuffer;

	return 0;
}

