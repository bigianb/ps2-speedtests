#include <tamtypes.h>
#include <stdio.h>

#define CLOCK_ADDR 0x10001900
#define CLOCKS_PER_SEC_ADDR 0x10001904


void timeFunc(const char* name, void (*fun)())
{
	printf("%s\n", name);
	u32 startTime = *((u32*)CLOCK_ADDR);
	fun();
	u32 endTime = *((u32*)CLOCK_ADDR);
	u32 clocksPerSec = *((u32*)CLOCKS_PER_SEC_ADDR);
	printf("\nelapsed clocks=%d, clocksPerSec=%d\n", endTime-startTime, clocksPerSec);
}

// Quite a few games have simple routines like this which the recompiler could be smart about.
int getFizzVal()
{
	return 5;
}

int getBuzzVal()
{
	return 7;
}

void fizzbuzz()
{
	char* output="none";
	for (int i=1; i<500000; ++i){
		if (i % getFizzVal() == 0 && i % getBuzzVal() == 0){
			output = "fizzbuzz";
		} else if (i % getFizzVal() == 0) {
			output = "fizz";
		} else if (i % getBuzzVal() == 0) {
			output = "buzz";
		}
	}
}

int main()
{
	timeFunc("fizzbuzz", fizzbuzz);
	return 0;
}
