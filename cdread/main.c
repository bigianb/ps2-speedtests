#include <tamtypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <libcdvd.h>
#include <sifrpc.h>

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

// Read a 5Mb file 100 times.
void readFile()
{
	sceCdlFILE file;
	int ret = sceCdSearchFile(&file, "\\ZERO5M.BIN;1");
	printf("search file return value: %d\n", ret);
	printf("name %s\n", file.name);
	printf("size %d\n", file.size);
	printf("loc %d\n", file.lsn);
	printf("date %02X %02X %02X %02X %02X %02X %02X %02X\n",
			file.date[0], file.date[1], file.date[2], file.date[3],
			file.date[4], file.date[5], file.date[6], file.date[7]);

	const int SECTOR_SIZE = 2048;
	const int sectorsToRead = (file.size + SECTOR_SIZE - 1) / SECTOR_SIZE;

	sceCdRMode mode;
	mode.trycount = 32;
	mode.spindlctrl = SCECdSpinNom;
	mode.datapattern = SCECdSecS2048;
	mode.pad = 0;

	printf("Sectors to read: %d\n", sectorsToRead);

	const int bufSize = sectorsToRead * SECTOR_SIZE;
	char* buf = malloc(bufSize);

	for (int i=0; i<20; ++i){
		sceCdRead(file.lsn, sectorsToRead, buf, &mode);
		sceCdSync(0);
	}
	free(buf);
}

int main()
{
	SifInitRpc(0);
	sceCdInit(SCECdINIT);
	
	timeFunc("read file", readFile);
	
	return 0;
}
