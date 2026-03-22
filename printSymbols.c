#include "headers.h"

/*
   printTable:
   ---------------------------------------
   Simple forward traversal.
   Not required by the assignment, but provided
   for debugging or expansion.
*/
void printTable(SYMTAB table) {

    SYMBOL *curr = table;

    while (curr != NULL) {
        printf("%s\t%X\n", curr->name, curr->address);
        curr = curr->next;
    }
}

/*
   printTableRev:
   ---------------------------------------
   Required by:  project2 --pass1only <file>
   Prints symbol table in reverse order of insertion.
*/
void printTableRev(SYMTAB table) {

    if (table == NULL)
        return;

    printTableRev(table->next);
    printf("%s\t%X\n", table->name, table->address);
}