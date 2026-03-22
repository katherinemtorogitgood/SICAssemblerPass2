#include "headers.h"
#include "opcode.h"

/* ============================================================
   INTERNAL UTILITIES
   ============================================================ */

/* Write uppercase hex with width padding */
static void writeHex(FILE *fp, int value, int width)
{
    fprintf(fp, "%0*X", width, value);
}

/* Trim leading and trailing whitespace */
static char *trim(char *s)
{
    while (isspace((unsigned char)*s)) s++;
    if (*s == 0) return s;

    char *end = s + strlen(s) - 1;
    while (end > s && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    return s;
}

/* ============================================================
   PARSE LINE  Daniel Thompson logic (rewritten + renamed)
   ============================================================ */

/*
   parseLine:
   Splits a SIC line into:
       label   (may be NULL)
       opcode
       operand (may contain commas or spaces)

   Notes:
   - Line may begin with label OR whitespace (no label).
   - Opcode always first token after label.
   - Operand is everything after opcode, trimmed.
*/
void parseLine(char *src, char **label, char **opcode, char **operand)
{
    *label = NULL;
    *opcode = NULL;
    *operand = NULL;

    char buf[1024];
    strcpy(buf, src);

    char *p = buf;

    /* If label starts in column 1, extract it */
    if (!isspace((unsigned char)buf[0])) {

        char *tok = strtok(p, " \t\r\n");
        if (!tok) return;

        *label = malloc(strlen(tok) + 1);
        strcpy(*label, tok);

        p = strtok(NULL, ""); /* remainder of line */
        if (!p) return;
    }

    /* Extract opcode */
    char *tok = strtok(p, " \t\r\n");
    if (!tok) return;

    *opcode = malloc(strlen(tok) + 1);
    strcpy(*opcode, tok);

    /* Extract operand = everything after opcode */
    char *rest = strtok(NULL, "");
    if (!rest) return;

    rest = trim(rest);

    if (strlen(rest) > 0) {
        *operand = malloc(strlen(rest) + 1);
        strcpy(*operand, rest);
    }
}

/* ============================================================
   OBJECT FILE GENERATION
   ============================================================ */

#define TEXT_MAX 30

/*
   We generate:
       H  header
       T  text records
       E  end record
       M  modification records (AFTER END)
*/

typedef struct ModRec {
    int address;
    struct ModRec *next;
} MODREC;

static MODREC *addMod(MODREC *list, int addr)
{
    MODREC *node = malloc(sizeof(MODREC));
    node->address = addr;
    node->next = list;
    return node;
}

/* ------------------------------------------------------------
   writeObject
   ------------------------------------------------------------ */
int writeObject(char *srcFile, SYMTAB sym, int startAddr, int progLen)
{
    char outname[256];
    strcpy(outname, srcFile);
    strcat(outname, ".obj");

    FILE *in = fopen(srcFile, "r");
    if (!in) return 0;

    FILE *out = fopen(outname, "w");
    if (!out) {
        fclose(in);
        return 0;
    }

    /* ============================================================
       HEADER RECORD
       ============================================================ */

    char progName[7] = "      ";
    SYMBOL *s = sym;
    while (s) {
        if (s->address == startAddr) {
            strncpy(progName, s->name, 6);
            progName[6] = '\0';
            break;
        }
        s = s->next;
    }

    fprintf(out, "H%-6s", progName);
    writeHex(out, startAddr, 6);
    writeHex(out, progLen, 6);
    fprintf(out, "\n");

    /* ============================================================
       TEXT RECORDS
       ============================================================ */

    unsigned char text[TEXT_MAX];
    int textLen = 0;
    int textStart = startAddr;
    int locctr = startAddr;

    MODREC *mods = NULL;

    char line[1024];

    while (fgets(line, sizeof(line), in) != NULL) {

        if (line[0] == '#') continue;

        char *L = NULL, *O = NULL, *Op = NULL;
        parseLine(line, &L, &O, &Op);

        if (!O) { if (L) free(L); if (Op) free(Op); continue; }

        if (strcmp(O, "END") == 0) {
            if (L) free(L);
            if (Op) free(Op);
            break;
        }

        int opc = lookupOpcode(O);
        int dtype = directiveType(O);

        /* ---------------------------- MACHINE INSTRUCTION ---------------------------- */
        if (opc >= 0) {

            /* auto-flush if next instruction will exceed record */
            if (textLen + 3 > TEXT_MAX) {
                fprintf(out, "T");
                writeHex(out, textStart, 6);
                writeHex(out, textLen, 2);
                for (int i = 0; i < textLen; i++)
                    writeHex(out, text[i], 2);
                fprintf(out, "\n");

                textLen = 0;
                textStart = locctr;
            }

            int operandAddr = 0;

            if (strcmp(O, "RSUB") != 0 && Op != NULL) {

                char temp[64];
                strncpy(temp, Op, sizeof(temp)-1);
                temp[sizeof(temp)-1] = 0;

                int indexed = 0;
                char *comma = strchr(temp, ',');
                if (comma) {
                    *comma = '\0';
                    indexed = 1;
                }

                operandAddr = findSymbol(sym, temp);

                if (indexed)
                    operandAddr |= 0x8000;

                /* Add modification record (always absolute addressing) */
                mods = addMod(mods, locctr + 1);
            }

            /* write object bytes */
            text[textLen++] = (unsigned char) opc;
            text[textLen++] = (operandAddr >> 8) & 0xFF;
            text[textLen++] = operandAddr & 0xFF;

            locctr += 3;
        }

        /* ---------------------------- DIRECTIVES ---------------------------- */
        else if (dtype > 0) {

            if (dtype == 1) { /* BYTE */

                char *start = strchr(Op, '\'');
                char *end = strrchr(Op, '\'');

                if (Op[0] == 'C') {

                    int n = end - start - 1;

                    if (textLen + n > TEXT_MAX) {
                        fprintf(out, "T");
                        writeHex(out, textStart, 6);
                        writeHex(out, textLen, 2);
                        for (int i = 0; i < textLen; i++)
                            writeHex(out, text[i], 2);
                        fprintf(out, "\n");

                        textLen = 0;
                        textStart = locctr;
                    }

                    for (int i = 0; i < n; i++)
                        text[textLen++] = (unsigned char) start[1 + i];

                    locctr += n;
                }

                else if (Op[0] == 'X') {

                    int hexLen = end - start - 1;
                    int n = hexLen / 2;

                    if (textLen + n > TEXT_MAX) {
                        fprintf(out, "T");
                        writeHex(out, textStart, 6);
                        writeHex(out, textLen, 2);
                        for (int i = 0; i < textLen; i++)
                            writeHex(out, text[i], 2);
                        fprintf(out, "\n");

                        textLen = 0;
                        textStart = locctr;
                    }

                    for (int i = 0; i < n; i++) {
                        char tmp[3];
                        tmp[0] = start[1 + i*2];
                        tmp[1] = start[2 + i*2];
                        tmp[2] = 0;
                        text[textLen++] = (unsigned char) strtol(tmp, NULL, 16);
                    }

                    locctr += n;
                }
            }

            else if (dtype == 2) { /* WORD */
                int val = atoi(Op);

                if (textLen + 3 > TEXT_MAX) {
                    fprintf(out, "T");
                    writeHex(out, textStart, 6);
                    writeHex(out, textLen, 2);
                    for (int i = 0; i < textLen; i++)
                        writeHex(out, text[i], 2);
                    fprintf(out, "\n");

                    textLen = 0;
                    textStart = locctr;
                }

                text[textLen++] = (val >> 16) & 0xFF;
                text[textLen++] = (val >> 8)  & 0xFF;
                text[textLen++] = val & 0xFF;

                locctr += 3;
            }

            else if (dtype == 3 || dtype == 4) {
                /* RESB / RESW  flush any active T record */
                if (textLen > 0) {
                    fprintf(out, "T");
                    writeHex(out, textStart, 6);
                    writeHex(out, textLen, 2);
                    for (int i = 0; i < textLen; i++)
                        writeHex(out, text[i], 2);
                    fprintf(out, "\n");
                    textLen = 0;
                }

                int amt = atoi(Op);
                if (dtype == 4) amt *= 3;

                locctr += amt;
                textStart = locctr;
            }
        }

        if (L) free(L);
        if (Op) free(Op);
        free(O);
    }

    /* ============================================================
       END RECORD
       ============================================================ */

    if (textLen > 0) {
        fprintf(out, "T");
        writeHex(out, textStart, 6);
        writeHex(out, textLen, 2);
        for (int i = 0; i < textLen; i++)
            writeHex(out, text[i], 2);
        fprintf(out, "\n");
    }

    fprintf(out, "E");
    writeHex(out, startAddr, 6);
    fprintf(out, "\n");
    
    /* ============================================================
       MODIFICATION RECORDS (AFTER END)  required format B
       ============================================================ */

    MODREC *m = mods;
    while (m) {
        fprintf(out, "M");
        writeHex(out, m->address, 6);
        fprintf(out, "04+%s\n", progName);
        m = m->next;
    }

    /* ============================================================
       CLEANUP
       ============================================================ */
    /* free mod list */
    while (mods) {
        MODREC *tmp = mods;
        mods = mods->next;
        free(tmp);
    }

    fclose(in);
    fclose(out);
    return 1;
}
