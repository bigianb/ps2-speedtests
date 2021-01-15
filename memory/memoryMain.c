#include <tamtypes.h>
#include <stdio.h>

int data[256];

void doTest()
{
	int i, j;
	// Lots of writes to normal memory.
	for (j=0; j<1000000; ++j){
		for(i=0; i<255; ++i){
			data[i] = i;
		}
	}
}

#define CLOCK_ADDR 0x10001900
#define CLOCKS_PER_SEC_ADDR 0x10001904


int main()
{
	u32 startTime, endTime, clocksPerSec;

	printf("Memory test\n");

	startTime = *((u32*)CLOCK_ADDR);

	doTest();

	endTime = *((u32*)CLOCK_ADDR);
	clocksPerSec = *((u32*)CLOCKS_PER_SEC_ADDR);

	printf("start=%d, end=%d, clocksPerSec=%d\n", startTime, endTime, clocksPerSec);
	return 0;
}
