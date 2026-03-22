#include <string.h>
#include "opcode.h"

/*
   Internal opcode lookup table for standard SIC instructions.
   Renamed and reorganized from Daniel Thompson's implementation.
*/

typedef struct {
    const char *mnemonic;
    int opcode;
} OPCODE_ENTRY;

static OPCODE_ENTRY opTable[] = {
    {"ADD",  0x18},
    {"AND",  0x40},
    {"COMP", 0x28},
    {"DIV",  0x24},
    {"J",    0x3C},
    {"JEQ",  0x30},
    {"JGT",  0x34},
    {"JLT",  0x38},
    {"JSUB", 0x48},
    {"LDA",  0x00},
    {"LDB",  0x68},
    {"LDCH", 0x50},
    {"LDL",  0x08},
    {"LDS",  0x6C},
    {"LDT",  0x74},
    {"LDX",  0x04},
    {"MUL",  0x20},
    {"OR",   0x44},
    {"RD",   0xD8},
    {"RSUB", 0x4C},
    {"STA",  0x0C},
    {"STB",  0x78},
    {"STCH", 0x54},
    {"STL",  0x14},
    {"STS",  0x7C},
    {"STT",  0x84},
    {"STX",  0x10},
    {"SUB",  0x1C},
    {"TD",   0xE0},
    {"TIX",  0x2C},
    {"WD",   0xDC},
    {NULL,   -1}
};

int lookupOpcode(const char *mnemonic) {
    for (int i = 0; opTable[i].mnemonic != NULL; i++) {
        if (strcmp(opTable[i].mnemonic, mnemonic) == 0)
            return opTable[i].opcode;
    }
    return -1; /* invalid mnemonic */
}