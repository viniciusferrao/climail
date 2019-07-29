# Universidade Federal do Rio de Janeiro
# Escola Politecnica
# Departamento de Eletronica e de Computacao
# Prof. Marcelo Lanza
# Internet e Arquitetura TCP/IP - Turma 2009/1
# Author: Vinicius Ferrao
# Description: Default Makefile

CC = gcc #Compiler
LD = gcc #Link-editor

CFLAGS = -march=native -O2 -pipe -c
LFLAGS = -Wall -o

# Executables
EXECS = sendmail pop

# Objects
OBJSENDMAIL = sendmail.o functions.o
OBJPOP = pop.o functions.o

# Librarys
#LIBNCURSES = -lncurses
#LIBCRYPT = -lcrypt

# Rules
.c.o:
	$(CC) $(CFLAGS) $<

all: $(EXECS)

sendmail: $(OBJSENDMAIL)
	$(LD) $(LFLAGS) $@ $(OBJSENDMAIL) 

pop: $(OBJPOP)
	$(LD) $(LFLAGS) $@ $(OBJPOP)

clean:
	rm -f *.o
	rm -f *.c~
	rm -f *.h~
	rm -f *.html~
	rm -f *.exe*
