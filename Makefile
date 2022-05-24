CC=gcc
COPTS=-O3 -fshort-enums -fopenmp -Wall -DxDEBUG -g -Dfno-inline

all: mathler-EASY mathler-NORMAL mathler-HARD mathler-NUMBLE mathler-THENUMBLE

mathler-%: mathler.c Makefile
	$(CC) -o $@ $(COPTS) $< CBack-1.0/SRC/CBack.c -D$* 

tst: tst-EASY

tst-%: mathler-%
	$<* 2