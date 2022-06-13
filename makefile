# definizione del compilatore e dei flag di compilazione
# che vengono usate dalle regole implicite
CC=gcc
CFLAGS=-g -Wall -O -D_POSIX_C_SOURCE=200809 -std=c99
LDLIBS=-lm -lrt -pthread

# singolo eseguilbile da compilare
MAIN=farm
EXECS=

# se si scrive solo make di default compila main 
all: $(MAIN) $(EXECS)

farm: farm.o xerrori.o

# target che cancella eseguibili e file oggetto
clean:
	rm -f $(MAIN) $(EXECS) *.o  

# target che crea l'archivio dei sorgenti
zip:
	zip $(MAIN).zip makefile *.c *.h *.p
