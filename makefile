# ============================================================
#   SIC Assembler  Makefile
#   Works on z/OS USS with xlc -q64
# ============================================================

CC = xlc
CFLAGS = -q64 -c

OBJS = main.o pass1.o pass2.o object.o opcode.o \
       insertSymbol.o destroySymbolTable.o symbolExists.o \
       symbolValid.o printSymbols.o

project2: $(OBJS)
	$(CC) -q64 -o project2 $(OBJS)

main.o: main.c headers.h
	$(CC) $(CFLAGS) main.c

pass1.o: pass1.c headers.h opcode.h
	$(CC) $(CFLAGS) pass1.c

pass2.o: pass2.c headers.h opcode.h object.h
	$(CC) $(CFLAGS) pass2.c

object.o: object.c headers.h opcode.h object.h
	$(CC) $(CFLAGS) object.c

opcode.o: opcode.c opcode.h
	$(CC) $(CFLAGS) opcode.c

insertSymbol.o: insertSymbol.c headers.h
	$(CC) $(CFLAGS) insertSymbol.c

destroySymbolTable.o: destroySymbolTable.c headers.h
	$(CC) $(CFLAGS) destroySymbolTable.c

symbolExists.o: symbolExists.c headers.h
	$(CC) $(CFLAGS) symbolExists.c

symbolValid.o: symbolValid.c headers.h
	$(CC) $(CFLAGS) symbolValid.c

printSymbols.o: printSymbols.c headers.h
	$(CC) $(CFLAGS) printSymbols.c

clean:
	rm -f *.o 
	rm -f *.obj
	rm -f project2