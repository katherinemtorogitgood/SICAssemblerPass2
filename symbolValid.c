#include "headers.h"

/*
   DIRECTIVE TABLE
   Renamed & simplified from Thompson logic.
   Supported directives are exactly those expected by SIC:
      START, END, BYTE, WORD, RESB, RESW
*/

static const char *directiveList[] = {
    "START", "END", "BYTE", "WORD",
    "RESB", "RESW", NULL
};

/* ------------------------------------------------------------
   Validate that a label is legal for SIC:
   - 1 to 6 chars
   - starts with alpha
   - rest alphanumeric
------------------------------------------------------------ */
int isValidLabel(char *name) {

    if (!name) return 0;
    int len = strlen(name);

    if (len < 1 || len > 6)
        return 0;

    if (!isalpha((unsigned char)name[0]))
        return 0;

    for (int i = 1; i < len; i++) {
        if (!isalnum((unsigned char)name[i]))
            return 0;
    }

    return 1;
}

/* ------------------------------------------------------------
   Returns 1 if token is a reserved SIC directive
------------------------------------------------------------ */
int isDirective(char *word) {
    if (!word) return 0;
    for (int i = 0; directiveList[i] != NULL; i++) {
        if (strcmp(directiveList[i], word) == 0)
            return 1;
    }
    return 0;
}

/* ------------------------------------------------------------
   Identify directive type:
       BYTE = 1
       WORD = 2
       RESB = 3
       RESW = 4
       START = 5  (not used for size or PASS2)
       END   = 6
------------------------------------------------------------ */
int directiveType(char *name) {
    if (!name) return -1;
    if (strcmp(name, "BYTE") == 0) return 1;
    if (strcmp(name, "WORD") == 0) return 2;
    if (strcmp(name, "RESB") == 0) return 3;
    if (strcmp(name, "RESW") == 0) return 4;
    if (strcmp(name, "START") == 0) return 5;
    if (strcmp(name, "END") == 0)   return 6;
    return -1;
}

/* ------------------------------------------------------------
   Evaluate operand size for BYTE directive.
   Supports:
       C'EOF'
       X'F1A0'
------------------------------------------------------------ */
int byteSize(char *operand) {
    if (!operand) return -1;
    int len = strlen(operand);

    if (len < 3)
        return -1;

    /* CHARACTER MODE: C'...' */
    if (operand[0] == 'C' && operand[1] == '\'') {
        char *start = strchr(operand, '\'');
        char *end   = strrchr(operand, '\'');
        if (!start || !end || end <= start) return -1;
        return (int)(end - start - 1);  /* number of characters */
    }

    /* HEX MODE: X'...' (must be even number of hex digits) */
    if (operand[0] == 'X' && operand[1] == '\'') {
        char *start = strchr(operand, '\'');
        char *end   = strrchr(operand, '\'');
        if (!start || !end || end <= start) return -1;

        int hexLen = (int)(end - start - 1);
        if (hexLen == 0 || (hexLen % 2) != 0)
            return -1;

        for (int i = 0; i < hexLen; i++) {
            if (!isxdigit(start[1 + i]))
                return -1;
        }

        return hexLen / 2;
    }

    return -1;
}

/* ------------------------------------------------------------
   WORD operand must be a valid decimal integer.
   Always returns size = 3 bytes for valid WORD.
------------------------------------------------------------ */
int wordSize(char *operand) {
    if (!operand) return -1;

    /* Copy & trim trailing spaces */
    char temp[64];
    strncpy(temp, operand, sizeof(temp)-1);
    temp[sizeof(temp)-1] = '\0';

    /* remove whitespace */
    int end = strlen(temp) - 1;
    while (end >= 0 && isspace((unsigned char)temp[end])) {
        temp[end--] = '\0';
    }

    if (temp[0] == '\0')
        return -1;

    /* validate numeric */
    char *endptr;
    long val = strtol(temp, &endptr, 10);

    /* endptr must point to '\0' */
    if (*endptr != '\0')
        return -1;

    /* limit WORD to 24-bit signed range */
    if (val > 0x7FFFFF || val < -0x800000)
        return -1;

    return 3;
}