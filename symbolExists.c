#include "headers.h"

/*
   findSymbol:
   ---------------------------------------
   Searches the symbol table for a label.
   Returns the address associated with the
   label or 0 if not found.

   NOTE:
   A real SIC program cannot have a label at
   address 0 unless START = 0.  Therefore,
   returning 0 for "not found" is safe and
   consistent with how PASS 2 checks symbols.
*/

int findSymbol(SYMTAB table, char *name) {

    SYMBOL *current = table;

    while (current != NULL) {
        if (strcmp(current->name, name) == 0)
            return current->address;
        current = current->next;
    }

    return 0;   /* symbol not found */
}