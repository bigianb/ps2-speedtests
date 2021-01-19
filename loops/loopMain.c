#include <tamtypes.h>
#include <stdio.h>

static void Kputc(char c) {
    while (*((u32*)0x1000f130) & 0x8000) { __asm__ ("nop\nnop\nnop\n"); }
    
	*((char*)0x1000f180) = c;
}

static void Kputs(char *s) {
	while (*s != 0) {
		Kputc(*s++);
	}
}

int main()
{
	u32 startTime, endTime, clocksPerSec;
	char buf[256];
	int i;

	Kputs("Loop test\n");

	startTime = *((u32*)0x10001900);

	for (i=0; i<0x10000; ++i){
	__asm__ ("nop\nnop\nnop\n");
	}

	endTime = *((u32*)0x10001900);
	clocksPerSec = *((u32*)0x10001904);

	sprintf(buf, "start=%d, end=%d, clocksPerSec=%d\n", startTime, endTime, clocksPerSec);
	Kputs(buf);

    return 0;
}