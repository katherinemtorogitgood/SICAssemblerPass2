#ifndef OBJECT_H
#define OBJECT_H

#include "headers.h"

/*
    writeObject:
    ----------------------------------------------------------
    Generates the final SIC object file in this order:

        H    Header
        T    Text Records
        E    End Record
        M    Modification Records (AFTER END)

    Called ONLY by PASS 2 after all validation succeeds.
*/
int writeObject(char *srcFile, SYMTAB symtab, int startAddr, int progLen);

#endif