/* pass1.c - Pass 1 for a simple SIC assembler
   Produces:
     intermediate.txt   (ADDR  LABEL  OPCODE  OPERAND)
     symtab.txt         (LABEL  ADDR)        <-- LABEL first, addr second
     length.txt         (program length in hex)
   Assumptions:
     - input.txt lines have three columns: LABEL OPCODE OPERAND
       use "**" as placeholder for no label (example: "** LDA FIVE")
     - opcode table is in opcode.txt as: MNEMONIC OPCODEHEX
       e.g.  LDA 00
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXSYM 1000
#define MAXOP  500

int main(void) {
    FILE *fin = fopen("input.txt", "r");
    FILE *fop = fopen("opcode.txt", "r");
    FILE *fint = fopen("intermediate.txt", "w");
    FILE *fsym = fopen("symtab.txt", "w");
    FILE *flen = fopen("length.txt", "w");

    if (!fin || !fop || !fint || !fsym || !flen) {
        perror("Error opening files (pass1)");
        return 1;
    }

    /* load opcode table (mnemonic -> code) for checking */
    char op_mn[MAXOP][20], op_hex[MAXOP][10];
    int opcount = 0;
    while (fscanf(fop, "%19s %9s", op_mn[opcount], op_hex[opcount]) == 2) {
        opcount++;
        if (opcount >= MAXOP) break;
    }
    rewind(fop); /* not strictly necessary */

    char line[256];
    char label[50], opcode[50], operand[100];
    int locctr = 0, start = 0;
    int first_line = 1;

    /* in-memory symbol table to detect duplicates */
    char sym_name[MAXSYM][50];
    int sym_addr[MAXSYM];
    int symcount = 0;

    while (fgets(line, sizeof line, fin)) {
        /* skip empty lines */
        if (sscanf(line, "%49s %49s %99s", label, opcode, operand) < 1) continue;

        /* Normalize missing fields */
        if (strlen(opcode) == 0) strcpy(opcode, "");
        if (strlen(operand) == 0) strcpy(operand, "");

        if (first_line && strcmp(opcode, "START") == 0) {
            start = (int)strtol(operand, NULL, 16);
            locctr = start;
            fprintf(fint, "%04X\t%s\t%s\t%s\n", locctr, label, opcode, operand);
            first_line = 0;
            continue;
        }
        if (first_line) { /* no START given */
            start = 0;
            locctr = 0;
            first_line = 0;
        }

        /* If label is not placeholder, add to symtab */
        if (strcmp(label, "**") != 0) {
            /* duplicate check */
            int dup = 0;
            for (int k = 0; k < symcount; k++) {
                if (strcmp(sym_name[k], label) == 0) {
                    fprintf(stderr, "Error: duplicate symbol '%s'\n", label);
                    dup = 1; break;
                }
            }
            if (!dup) {
                strcpy(sym_name[symcount], label);
                sym_addr[symcount] = locctr;
                fprintf(fsym, "%s\t%04X\n", label, locctr); /* LABEL addr */
                symcount++;
            }
        }

        /* write intermediate line (address label opcode operand) */
        fprintf(fint, "%04X\t%s\t%s\t%s\n", locctr, label, opcode, operand);

        /* update LOCCTR */
        /* check opcode table */
        int is_op = 0;
        for (int k = 0; k < opcount; k++) {
            if (strcmp(opcode, op_mn[k]) == 0) {
                is_op = 1;
                break;
            }
        }
        if (is_op) {
            locctr += 3; /* SIC machine instruction size = 3 */
        } else if (strcmp(opcode, "WORD") == 0) {
            locctr += 3;
        } else if (strcmp(opcode, "RESW") == 0) {
            locctr += 3 * atoi(operand);
        } else if (strcmp(opcode, "RESB") == 0) {
            locctr += atoi(operand);
        } else if (strcmp(opcode, "BYTE") == 0) {
            /* BYTE C'EOF' => length is number of chars
               BYTE X'F1'  => length is number of hex digits / 2 */
            if (operand[0] == 'C' && operand[1] == '\'') {
                int len = (int)strlen(operand) - 3; /* subtract C' and ' */
                if (len < 0) len = 0;
                locctr += len;
            } else if (operand[0] == 'X' && operand[1] == '\'') {
                int hexlen = (int)strlen(operand) - 3;
                if (hexlen < 0) hexlen = 0;
                locctr += (hexlen + 1) / 2;
            } else {
                /* treat as one byte if malformed */
                locctr += 1;
            }
        } else if (strcmp(opcode, "END") == 0) {
            /* write last line and break after writing END */
            /* we already wrote intermediate line above */
            break;
        } else {
            /* Unknown opcode - warn and assume 3 bytes to continue */
            fprintf(stderr, "Warning: unknown opcode '%s' treated as 3 bytes\n", opcode);
            locctr += 3;
        }
    }

    int program_length = locctr - start;
    fprintf(flen, "%X\n", program_length);

    fclose(fin);
    fclose(fop);
    fclose(fint);
    fclose(fsym);
    fclose(flen);

    printf("Pass 1 completed. Wrote intermediate.txt, symtab.txt, length.txt\n");
    return 0;
}


