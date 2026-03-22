#include "headers.h"

/*
   clearTable:
   ---------------------------------------
   Frees the symbol table linked list.
   Safe, recursive implementation.
*/

void clearTable(SYMTAB table) {

    if (table == NULL)
        return;

    clearTable(table->next);
    free(table);
}