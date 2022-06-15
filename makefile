# definizione del compilatore e dei flag di compilazione
# che vengono usate dalle regole implicite
CC=gcc
CFLAGS=-g -Wall -O -D_POSIX_C_SOURCE=200809 -std=c99
LDLIBS=-lm -lrt -pthread

# singolo eseguilbile da compilare
MAIN=farm
 
all: $(MAIN)

farm: farm.o xerrori.o

# target che cancella eseguibili e file oggetto
.PHONY: clean
clean:
	rm -f $(MAIN)*.o  

# target che crea l'archivio dei sorgenti
zip:
	zip $(MAIN).zip makefile *.c *.h *.p
