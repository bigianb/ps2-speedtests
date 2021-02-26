// GSTransferTests.cpp : Defines the entry point for the application.
//
#include <chrono>
#include <cstring>
#include <iomanip>

#include "GSTransferTests.h"

using namespace std;

typedef unsigned int uint32;
typedef unsigned char uint8;

extern void runBaselinePSMT8(int loops, uint8* pCvtBuffer, uint8* pRAM, int texW, int texH);
extern void runSSEVersionPSMT8(int loops, uint8* pCvtBuffer, uint8* pRAM, int texW, int texH);
extern void runBaselinePSMT4(int loops, uint8* pCvtBuffer, uint8* pRAM, int texW, int texH);
extern void runSSEVersionPSMT4(int loops, uint8* pCvtBuffer, uint8* pRAM, int texW, int texH);

enum CVTBUFFERSIZE
{
	CVTBUFFERSIZE = 0x800000,
};

enum RAMSIZE
{
	RAMSIZE = 0x00400000,
};

void logData(uint8* pBuf, int w, int h)
{
	uint8* p = pBuf;
	for (int y=0; y<h; ++y){
		for (int x=0; x<w; ++x){
			cout << hex << setw(2) << (int)*p << ' ';
			++p;
		}
		cout << endl;
	}
}

// ensure output is as expected (matches baseline)
bool runTestPSMT8(uint8* pRAM, uint8* pCvtBuffer)
{
	memset(pCvtBuffer, 0, CVTBUFFERSIZE);
	memset(pRAM, 0, RAMSIZE);
	for (int i = 0; i < 256; ++i) {
		pRAM[i] = i;
	}
	memset(pCvtBuffer, 0, CVTBUFFERSIZE);
	runBaselinePSMT8(1, pCvtBuffer, pRAM, 16, 16);

	uint8* pCvtBuffer2 = new uint8[CVTBUFFERSIZE];
	memset(pCvtBuffer2, 0, CVTBUFFERSIZE);
	runSSEVersionPSMT8(1, pCvtBuffer2, pRAM, 16, 16);

	bool match = true;
	for (int i = 0; i < 256; ++i) {
		if (pCvtBuffer[i] != pCvtBuffer2[i]) {
			match = false;
		}
	}

	if (!match){
		cout << "Expected" << endl;
		logData(pCvtBuffer, 16, 16);
		cout << endl << "Received" << endl;
		logData(pCvtBuffer2, 16, 16);
	}

	delete[] pCvtBuffer2;

	return match;
}

// ensure output is as expected (matches baseline)
bool runTestPSMT4(uint8* pRAM, uint8* pCvtBuffer)
{
	memset(pRAM, 0, RAMSIZE);
	for (int i = 0; i < 256; ++i) {
		pRAM[i] = i;
	}
	memset(pCvtBuffer, 0, CVTBUFFERSIZE);
	runBaselinePSMT4(1, pCvtBuffer, pRAM, 32, 16);

	uint8* pCvtBuffer2 = new uint8[CVTBUFFERSIZE];
	memset(pCvtBuffer2, 0, CVTBUFFERSIZE);
	runSSEVersionPSMT4(1, pCvtBuffer2, pRAM, 32, 16);

	bool match = true;
	for (int i = 0; i < 256; ++i) {
		if (pCvtBuffer[i] != pCvtBuffer2[i]) {
			match = false;
		}
	}

	if (!match) {
		cout << "Expected" << endl;
		logData(pCvtBuffer, 32, 16);
		cout << endl << "Received" << endl;
		logData(pCvtBuffer2, 32, 16);
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
	
	cout << "Testing PSMT8 ... ";
	if (runTestPSMT8(pRAM, pCvtBuffer)) {
		cout << "passed" << endl;
		auto startTime = chrono::high_resolution_clock::now();
		runBaselinePSMT8(10000, pCvtBuffer, pRAM, 512, 512);
		auto endTime = chrono::high_resolution_clock::now();

		cout << "Baseline elapsed: " << chrono::duration_cast<chrono::microseconds>(endTime - startTime).count() << " us" << endl;

		startTime = chrono::high_resolution_clock::now();
		runSSEVersionPSMT8(10000, pCvtBuffer, pRAM, 512, 512);
		endTime = chrono::high_resolution_clock::now();

		cout << "Optimised elapsed: " << chrono::duration_cast<chrono::microseconds>(endTime - startTime).count() << " us" << endl;
		cout << "Done." << endl;
	}
	else {
		cout << "Failed. Perf run terminated as reconciliation test failed" << endl;
	}
	
	cout << "Testing PSMT4 ... ";
	if (runTestPSMT4(pRAM, pCvtBuffer)) {
		cout << "passed" << endl;
		auto startTime = chrono::high_resolution_clock::now();
		runBaselinePSMT4(10000, pCvtBuffer, pRAM, 512, 512);
		auto endTime = chrono::high_resolution_clock::now();

		cout << "Baseline elapsed: " << chrono::duration_cast<chrono::microseconds>(endTime - startTime).count() << " us" << endl;

		startTime = chrono::high_resolution_clock::now();
		runSSEVersionPSMT4(10000, pCvtBuffer, pRAM, 512, 512);
		endTime = chrono::high_resolution_clock::now();

		cout << "SIMD elapsed: " << chrono::duration_cast<chrono::microseconds>(endTime - startTime).count() << " us" << endl;
		cout << "Done." << endl;
	}
	else {
		cout << "Failed. Perf run terminated as reconciliation test failed" << endl;
	}
	delete[] pRAM;
	delete[] pCvtBuffer;

	return 0;
}

