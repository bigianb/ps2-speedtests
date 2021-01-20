#include <tamtypes.h>
#include <stdio.h>

int data[255];

// Write 255 million words in a tight loop.
void writeWordsInLoop()
{
	int i, j;
	// Lots of writes to normal memory.
	for (j=0; j<100; ++j){
		for(i=0; i<255; ++i){
			data[i] = i;
		}
	}
}

#define CLOCK_ADDR 0x10001900
#define CLOCKS_PER_SEC_ADDR 0x10001904


void timeFunc(const char* name, void (*fun)())
{
	u32 startTime, endTime, clocksPerSec;
	fun(); // warm-up
	printf("%s\n", name);
	startTime = *((u32*)CLOCK_ADDR);
	fun();
	endTime = *((u32*)CLOCK_ADDR);
	clocksPerSec = *((u32*)CLOCKS_PER_SEC_ADDR);
	printf("elapsed clocks=%d, clocksPerSec=%d\n", endTime-startTime, clocksPerSec);
}

int main()
{
	timeFunc("Write Words in loop", writeWordsInLoop);
	return 0;
}
