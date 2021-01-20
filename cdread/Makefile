
EE_BIN = ./fs/main.elf
EE_OBJS = main.o 

.PHONY: all

all: $(EE_BIN)
	mkisofs -o ../build/cdread.iso ./fs

.PHONY: clean
clean:
	rm -f *.o $(EE_BIN) ../build/cdread.iso

include $(PS2SDK)/Defs.make
include ../make/Makefile.pref
include ../make/Makefile.eeglobal
