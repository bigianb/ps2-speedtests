
EE_BIN = ../build/memoryTest.elf
EE_OBJS = memoryMain.o 

.PHONY: all

all: $(EE_BIN)

.PHONY: clean
clean:
	rm -f *.o $(ELF)

include $(PS2SDK)/Defs.make
include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
