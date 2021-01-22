#include <tamtypes.h>
#include <stdio.h>
#include <libcdvd.h>
#include <sifrpc.h>

int data[255];

// Write 255 million words in a tight loop.
void writeWordsInLoop()
{
	// Lots of writes to normal memory.
	for (int j=0; j<100; ++j){
		for(int i=0; i<255; ++i){
			data[i] = i;
		}
	}
}

#define CLOCK_ADDR 0x10001900
#define CLOCKS_PER_SEC_ADDR 0x10001904


void timeFunc(const char* name, void (*fun)())
{
	fun(); // warm-up
	printf("%s\n", name);
	u32 startTime = *((u32*)CLOCK_ADDR);
	fun();
	u32 endTime = *((u32*)CLOCK_ADDR);
	u32 clocksPerSec = *((u32*)CLOCKS_PER_SEC_ADDR);
	printf("elapsed clocks=%d, clocksPerSec=%d\n", endTime-startTime, clocksPerSec);
}

int main()
{
	SifInitRpc(0);
	sceCdInit(SCECdINIT);
	
	sceCdlFILE file;
	int ret = sceCdSearchFile(&file, "\\ZERO5M.BIN");

	printf("search name %s\n", file.name);
	printf("search size %d\n", file.size);
	printf("search loc lnn %d\n", file.lsn);
	printf("search loc date %02X %02X %02X %02X %02X %02X %02X %02X\n",
			file.date[0], file.date[1], file.date[2], file.date[3],
			file.date[4], file.date[5], file.date[6], file.date[7]);
	printf("search loc date %02d %02d %02d %02d %02d %02d %02d %02d\n",
			file.date[0], file.date[1], file.date[2], file.date[3],
			file.date[4], file.date[5], file.date[6], file.date[7]);


	//ret = sceCdRead(fp.lsn, sectors, buf, &mode);
	//sceCdSync(0);

	timeFunc("Write Words in loop", writeWordsInLoop);
	return 0;
}
