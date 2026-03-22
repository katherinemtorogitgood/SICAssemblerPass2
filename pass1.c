#include "headers.h"

/*
   PASS 1:
   ------------------------------------------------------
   - Reads source file line by line
   - Extracts label / opcode / operand (via parseLine)
   - Builds symbol table
   - Computes program length
   - Validates labels and directives
*/

int pass1(char *filename, SYMTAB *symtab, int *startAddr, int *progLen)
{
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        printf("ASSEMBLY ERROR:\nCould not open %s\n", filename);
        return 0;
    }

    char line[1024];
    int lineNum = 1;
    int locctr = 0;
    int sawStart = 0;

    *symtab = NULL;
    *startAddr = 0;

    while (fgets(line, sizeof(line), fp) != NULL) {

        /* Skip comment lines */
        if (line[0] == '#') {
            lineNum++;
            continue;
        }

        /* Blank line check */
        int onlySpace = 1;
        for (int i = 0; line[i]; i++) {
            if (!isspace((unsigned char)line[i])) {
                onlySpace = 0;
                break;
            }
        }
        if (onlySpace) {
            printf("ASSEMBLY ERROR:\nLine %d is blank\n", lineNum);
            fclose(fp);
            return 0;
        }

        /* Thompson-style parsing (implemented in object.c) */
        char *label = NULL;
        char *opcode = NULL;
        char *operand = NULL;

        parseLine(line, &label, &opcode, &operand);

        /* Missing opcode */
        if (!opcode) {
            printf("ASSEMBLY ERROR:\n%sLine %d Missing opcode\n", line, lineNum);
            if (label) free(label);
            if (operand) free(operand);
            fclose(fp);
            return 0;
        }

        /* START directive */
        if (strcmp(opcode, "START") == 0) {

            if (sawStart) {
                printf("ASSEMBLY ERROR:\n%sLine %d Duplicate START\n", line, lineNum);
                if (label) free(label);
                if (operand) free(operand);
                fclose(fp);
                return 0;
            }

            sawStart = 1;

            if (!operand) {
                printf("ASSEMBLY ERROR:\n%sLine %d Missing START operand\n", line, lineNum);
                if (label) free(label);
                if (operand) free(operand);
                fclose(fp);
                return 0;
            }

            /* starting address is hex */
            *startAddr = (int) strtol(operand, NULL, 16);
            locctr = *startAddr;

            /* If a label appears with START, it names the program */
            if (label) {
                *symtab = addSymbol(*symtab, label, *startAddr, lineNum);
            }

            if (label) free(label);
            if (operand) free(operand);
            lineNum++;
            continue;
        }

        /* LABEL PROCESSING */
        if (label != NULL) {

            if (!isValidLabel(label)) {
                printf("ASSEMBLY ERROR:\n%sLine %d Invalid label '%s'\n",
                       line, lineNum, label);
                free(label);
                if (operand) free(operand);
                fclose(fp);
                return 0;
            }

            if (isDirective(label)) {
                printf("ASSEMBLY ERROR:\n%sLine %d Label '%s' is a directive\n",
                       line, lineNum, label);
                free(label);
                if (operand) free(operand);
                fclose(fp);
                return 0;
            }

            /* Check for duplicates */
            if (findSymbol(*symtab, label)) {
                printf("ASSEMBLY ERROR:\n%sLine %d Duplicate label '%s'\n",
                       line, lineNum, label);
                free(label);
                if (operand) free(operand);
                fclose(fp);
                return 0;
            }

            *symtab = addSymbol(*symtab, label, locctr, lineNum);
        }

        /* DIRECTIVES & INSTRUCTIONS */
        int opc = lookupOpcode(opcode);
        int dtype = directiveType(opcode);

        if (opc >= 0) {
            /* standard SIC instruction */
            locctr += 3;
        }
        else if (dtype > 0) {

            if (dtype == 1) {          /* BYTE */
                int size = byteSize(operand);
                if (size < 0) {
                    printf("ASSEMBLY ERROR:\n%sLine %d Invalid BYTE '%s'\n",
                           line, lineNum, operand);
                    if (label) free(label);
                    if (operand) free(operand);
                    fclose(fp);
                    return 0;
                }
                locctr += size;
            }

            else if (dtype == 2) {     /* WORD */
                int size = wordSize(operand);
                if (size < 0) {
                    printf("ASSEMBLY ERROR:\n%sLine %d Invalid WORD '%s'\n",
                           line, lineNum, operand);
                    if (label) free(label);
                    if (operand) free(operand);
                    fclose(fp);
                    return 0;
                }
                locctr += size;
            }

            else if (dtype == 3) {     /* RESB */
                locctr += atoi(operand);
            }

            else if (dtype == 4) {     /* RESW */
                locctr += 3 * atoi(operand);
            }

            /* END is ignored in PASS 1 */
        }
        else {
            printf("ASSEMBLY ERROR:\n%sLine %d Invalid opcode '%s'\n",
                   line, lineNum, opcode);
            if (label) free(label);
            if (operand) free(operand);
            fclose(fp);
            return 0;
        }

        if (label) free(label);
        if (operand) free(operand);
        lineNum++;
    }

    fclose(fp);

    /* If no START, program begins at 0 */
    if (!sawStart)
        *startAddr = 0;

    *progLen = locctr - *startAddr;
    return 1;
}