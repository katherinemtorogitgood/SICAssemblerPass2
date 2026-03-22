#include "headers.h"
#include "opcode.h"
#include "object.h"

/*
   PASS 2:
   ------------------------------------------------------
   Performs error checking using the symbol table from PASS 1.
   If no errors are detected, object.c will generate the .obj file.

   Key tasks:
     - Validate opcode
     - Validate directive operands
     - Validate symbol references
     - Ensure END appears exactly once
*/

static void err(const char *line, int num, const char *msg)
{
    printf("ASSEMBLY ERROR:\n");
    printf("%s", line);
    printf("Line %d %s\n", num, msg);
}

int pass2(char *filename, SYMTAB table, int startAddr, int progLen)
{
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        printf("ASSEMBLY ERROR:\nCould not reopen %s in PASS 2\n", filename);
        return 0;
    }

    char line[1024];
    int lineNum = 1;
    int locctr = startAddr;
    int endSeen = 0;

    while (fgets(line, sizeof(line), fp) != NULL) {

        /* Skip comment lines */
        if (line[0] == '#') {
            lineNum++;
            continue;
        }

        /* Quickly detect empty/blank lines (illegal) */
        int blank = 1;
        for (int i = 0; line[i]; i++) {
            if (!isspace((unsigned char) line[i])) {
                blank = 0;
                break;
            }
        }
        if (blank) {
            err(line, lineNum, "Blank line");
            fclose(fp);
            return 0;
        }

        /* Parse line via multi-token parser (in object.c) */
        char *label = NULL;
        char *opcode = NULL;
        char *operand = NULL;

        parseLine(line, &label, &opcode, &operand);

        if (!opcode) {
            err(line, lineNum, "Missing opcode");
            if (label) free(label);
            if (operand) free(operand);
            fclose(fp);
            return 0;
        }

        /* END directive */
        if (strcmp(opcode, "END") == 0) {

            if (endSeen) {
                err(line, lineNum, "Duplicate END directive");
                if (label) free(label);
                if (operand) free(operand);
                fclose(fp);
                return 0;
            }

            endSeen = 1;

            if (!operand) {
                err(line, lineNum, "Missing END operand");
                if (label) free(label);
                if (operand) free(operand);
                fclose(fp);
                return 0;
            }

            /* After END, we stop parsing file */
            if (label) free(label);
            if (operand) free(operand);
            break;
        }

        /* MACHINE INSTRUCTION? */
        int opc = lookupOpcode(opcode);
        int dir = directiveType(opcode);

        if (opc >= 0) {

            /* If not RSUB, operand is required */
            if (strcmp(opcode, "RSUB") != 0) {

                if (!operand) {
                    err(line, lineNum, "Missing operand");
                    if (label) free(label);
                    fclose(fp);
                    return 0;
                }

                /* If operand is SYMBOL,X extract symbol */
                char temp[64];
                strncpy(temp, operand, sizeof(temp)-1);
                temp[sizeof(temp)-1] = '\0';

                char *comma = strchr(temp, ',');
                if (comma) *comma = '\0';

                if (!findSymbol(table, temp)) {
                    char msg[80];
                    sprintf(msg, "Undefined symbol '%s'", temp);
                    err(line, lineNum, msg);
                    if (label) free(label);
                    if (operand) free(operand);
                    fclose(fp);
                    return 0;
                }
            }

            locctr += 3;
        }

        /* DIRECTIVE? */
        else if (dir > 0) {

            if (dir == 1) { /* BYTE */
                if (!operand) {
                    err(line, lineNum, "Missing BYTE operand");
                    if (label) free(label);
                    fclose(fp);
                    return 0;
                }
                int n = byteSize(operand);
                if (n < 0) {
                    err(line, lineNum, "Invalid BYTE operand");
                    if (label) free(label);
                    if (operand) free(operand);
                    fclose(fp);
                    return 0;
                }
                locctr += n;
            }

            else if (dir == 2) { /* WORD */
                if (!operand) {
                    err(line, lineNum, "Missing WORD operand");
                    if (label) free(label);
                    fclose(fp);
                    return 0;
                }
                int n = wordSize(operand);
                if (n < 0) {
                    err(line, lineNum, "Invalid WORD operand");
                    if (label) free(label);
                    if (operand) free(operand);
                    fclose(fp);
                    return 0;
                }
                locctr += n;
            }

            else if (dir == 3) { /* RESB */
                if (!operand) {
                    err(line, lineNum, "Missing RESB operand");
                    if (label) free(label);
                    fclose(fp);
                    return 0;
                }
                locctr += atoi(operand);
            }

            else if (dir == 4) { /* RESW */
                if (!operand) {
                    err(line, lineNum, "Missing RESW operand");
                    if (label) free(label);
                    fclose(fp);
                    return 0;
                }
                locctr += 3 * atoi(operand);
            }

            /* START/END are handled elsewhere */
        }

        else {
            char msg[80];
            sprintf(msg, "Invalid opcode '%s'", opcode);
            err(line, lineNum, msg);
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

    if (!endSeen) {
        printf("ASSEMBLY ERROR:\nMissing END directive\n");
        return 0;
    }

    /* If PASS 2 completed without errors  generate object file */
    return writeObject(filename, table, startAddr, progLen);
}