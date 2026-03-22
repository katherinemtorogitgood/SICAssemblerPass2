#ifndef HEADERS_H
#define HEADERS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ============================================================
   SYMBOL TABLE STRUCTURE
   ============================================================ */

typedef struct SymbolNode {
    char name[7];          /* SIC label: up to 6 chars */
    int  address;          /* address assigned in PASS 1 */
    int  line;             /* source line where label appears */
    struct SymbolNode *next;
} SYMBOL;

/* pointer to head of symbol table */
typedef SYMBOL* SYMTAB;

/* ============================================================
   SYMBOL TABLE OPERATIONS
   ============================================================ */

SYMTAB addSymbol(SYMTAB table, char *name, int addr, int line);
int    findSymbol(SYMTAB table, char *name);   /* returns address or 0 if missing */
void   clearTable(SYMTAB table);
void   printTable(SYMTAB table);
void   printTableRev(SYMTAB table);

/* ============================================================
   DIRECTIVE VALIDATION
   ============================================================ */

int isValidLabel(char *name);
int isDirective(char *name);
int directiveType(char *name);   /* BYTE=1, WORD=2, RESB=3, RESW=4, etc. */
int byteSize(char *operand);
int wordSize(char *operand);

/* ============================================================
   CORE ASSEMBLER PASSES
   ============================================================ */

int pass1(char *filename, SYMTAB *symtab, int *startAddr, int *progLen);
int pass2(char *filename, SYMTAB symtab, int startAddr, int progLen);

/* ============================================================
   OBJECT FILE CREATION
   ============================================================ */

int writeObject(char *srcFile, SYMTAB symtab, int startAddr, int progLen);

/* ============================================================
   LINE PARSER (merged from Thompson logic)
   Splits a SIC source line into:
     label   (may be NULL)
     opcode
     operand (may contain commas)
   ============================================================ */

void parseLine(char *src, char **label, char **opcode, char **operand);

#endif
