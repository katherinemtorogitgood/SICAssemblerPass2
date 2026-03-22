#include "headers.h"

/*
   addSymbol:
   ---------------------------------------
   Inserts a symbol into the table.
   List insertion is at the head for speed.

   PARAMETERS
       table    current symbol list
       name     label (validated in PASS 1)
       addr     assigned address
       line     source line number

   RETURNS
       Updated head of the symbol table
*/

SYMTAB addSymbol(SYMTAB table, char *name, int addr, int line) {

    SYMBOL *node = (SYMBOL*) malloc(sizeof(SYMBOL));
    if (!node) {
        printf("FATAL ERROR: Out of memory\n");
        exit(1);
    }

    memset(node, 0, sizeof(SYMBOL));

    strncpy(node->name, name, 6);
    node->name[6] = '\0';    /* ensure null termination */

    node->address = addr;
    node->line    = line;

    node->next = table;
    return node;    /* new head */
}