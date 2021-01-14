
EE_BIN = ../build/loopTest.elf
EE_OBJS = loopMain.o 

.PHONY: all

all: $(EE_BIN)

.PHONY: clean
clean:
	rm -f *.o $(ELF)

include $(PS2SDK)/Defs.make
include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
