
EE_BIN = ./fs/main.elf
EE_OBJS = main.o 

.PHONY: all

./fs/ZERO5M.BIN:
	fallocate -l 5m ./fs/ZERO5M.BIN

all: $(EE_BIN) ./fs/ZERO5M.BIN
	mkisofs -o ../build/cdread.iso ./fs

.PHONY: clean
clean:
	rm -f *.o $(EE_BIN) ./fs/ZERO5M.BIN ../build/cdread.iso

include $(PS2SDK)/Defs.make
include ../make/Makefile.pref
include ../make/Makefile.eeglobal
