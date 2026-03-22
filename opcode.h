#ifndef OPCODE_H
#define OPCODE_H

/* 
   Returns the opcode byte for a SIC mnemonic.
   Returns -1 if the mnemonic is not a valid instruction.
*/
int lookupOpcode(const char *mnemonic);

#endif
