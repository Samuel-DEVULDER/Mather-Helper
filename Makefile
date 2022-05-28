###############################################################################
# Makefile for mathler-helpoer
###############################################################################
CC=gcc

OPENMP=-fopenmp
OPTIM=-O3 -fshort-enums
DEBUG=	#-DDEBUG
COPTS=-Wall -DSIMD -Dmsse4

ifeq ($(OS),Windows_NT)
EXE=.exe
else
EXE=
endif

ALL=$(patsubst %, mathler-%$(EXE), EASY NORMAL HARD NUMBLE)

###############################################################################

all: $(ALL)

clean:
	rm -f $(ALL)

peekasm: 
	$(CC) -o tmp.o mathler.c \
	-DHARD $(OPTIM) $(COPTS) $(DEBUG) -c -fno-inline -g
	objdump -d -S tmp.o | less

mathler-%$(EXE): mathler.c Makefile
	$(CC) -o $@ -D$* $(OPTIM) $(COPTS) $< CBack-1.0/SRC/CBack.c $(OPENMP)

###############################################################################
CORES:=$(shell grep -c ^processor /proc/cpuinfo)
ifeq (,$(CORES))
CORES:=1
endif
ifeq (,$(OPENMP))
CORES:=1
endif

tst: mathler-HARD$(EXE)
	bash -c 'for i in {1..$(CORES)};\
	do\
		OMP_THREAD_LIMIT=$$i\
		./$< </dev/null 7;\
	done'

profile-%:
	$(MAKE) "CC=$(CC) -g -pg" OPENMP= $*

