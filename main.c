#include "headers.h"

/*
   MAIN DRIVER FOR SIC ASSEMBLER
   ------------------------------
   Expected usage:
       project2 <filename>
       project2 --pass1only <filename>

   PASS 1   build symbol table & calculate program length
   PASS 2   validate instructions; if successful  write object file
*/

int main(int argc, char *argv[]) {

    int pass1Only = 0;
    char *srcFile = NULL;

    /* ----------------------------------------------------------
       Parse command-line input
       ---------------------------------------------------------- */
    if (argc == 2) {
        srcFile = argv[1];
    }
    else if (argc == 3 && strcmp(argv[1], "--pass1only") == 0) {
        pass1Only = 1;
        srcFile = argv[2];
    }
    else {
        printf("ERROR: usage is %s <filename>\n", argv[0]);
        printf("       %s --pass1only <filename>\n", argv[0]);
        return 0;
    }

    /* ----------------------------------------------------------
       PASS 1
       ---------------------------------------------------------- */
    SYMTAB symbols = NULL;
    int startAddr = 0;
    int programLength = 0;

    if (!pass1(srcFile, &symbols, &startAddr, &programLength)) {
        /* pass1 prints its own error messages */
        clearTable(symbols);
        return 0;
    }

    /* ----------------------------------------------------------
       Optional: only run PASS 1 and print symbol table
       ---------------------------------------------------------- */
    if (pass1Only) {
        printTableRev(symbols);
        clearTable(symbols);
        return 0;
    }

    /* ----------------------------------------------------------
       PASS 2
       ---------------------------------------------------------- */
    if (!pass2(srcFile, symbols, startAddr, programLength)) {
        /* pass2 prints its own errors */
        clearTable(symbols);
        return 0;
    }

    /* ----------------------------------------------------------
       SUCCESS
       Object file has been written.
       No screen output allowed on success.
       ---------------------------------------------------------- */
    clearTable(symbols);
    return 0;
}
