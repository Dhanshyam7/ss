#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    char label[10], opcode[10], operand[10], code[10], addr[10];
    char symbol[10], symaddr[10];
    int start, loc, length;
    FILE *fp1, *fp2, *fp3, *fp4;

    fp1 = fopen("intert.txt", "r");   // intermediate file from pass 1
    fp2 = fopen("symtab.txt", "r");   // symbol table
    fp3 = fopen("optab.txt", "r");    // operation table
    fp4 = fopen("output.txt", "w");   // final object program

    fscanf(fp1, "%s%s%s", label, opcode, operand);

    // START
    if (strcmp(opcode, "START") == 0) {
        start = atoi(operand);
        fprintf(fp4, "H^%s^%04d\n", label, start);  // Header record
        fscanf(fp1, "%d%s%s%s", &loc, label, opcode, operand);
    }

    fprintf(fp4, "T^");  // Text record start

    // process each instruction until END
    while (strcmp(opcode, "END") != 0) {
        rewind(fp3);
        fscanf(fp3, "%s%s", code, addr);
        while (!feof(fp3)) {
            if (strcmp(opcode, code) == 0) {
                // found opcode → look up symbol
                rewind(fp2);
                fscanf(fp2, "%s%s", symbol, symaddr);
                while (!feof(fp2)) {
                    if (strcmp(operand, symbol) == 0) {
                        fprintf(fp4, "^%s%s", addr, symaddr);
                        break;
                    }
                    fscanf(fp2, "%s%s", symbol, symaddr);
                }
                break;
            }
            fscanf(fp3, "%s%s", code, addr);
        }

        if (strcmp(opcode, "WORD") == 0)
            fprintf(fp4, "^00000%s", operand);
        else if (strcmp(opcode, "BYTE") == 0)
            fprintf(fp4, "^%s", operand);

        fscanf(fp1, "%d%s%s%s", &loc, label, opcode, operand);
    }

    // END record
    fprintf(fp4, "\nE^%04d\n", start);

    printf("PASS 2 completed. Object program stored in output.txt\n");

    fclose(fp1);
    fclose(fp2);
    fclose(fp3);
    fclose(fp4);

    return 0;
}
