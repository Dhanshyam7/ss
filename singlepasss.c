#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define INSTR_SIZE 3

int main() {
    char opcode[10], operand[10], label[10], symbol[10], ch;
    char mnemonic[6][10] = {"START", "END", "WORD", "BYTE", "RESW", "RESB"};
    int locctr, start = 0, length = 0;
    int st = 0, address = 0, add = 0;
    int finaddr = 0, prevaddr = 0;

    FILE *fp1, *fp2, *fp3, *fp4, *fp5, *fp6, *fp7, *fp_op;
    fp1 = fopen("input.txt", "r");       // Source input
    fp2 = fopen("symtab1.txt", "w");      // Symbol table
    fp3 = fopen("intermediate1.txt", "w");// Intermediate file
    fp_op = fopen("opcode.txt", "r");     // Opcode file

    if (!fp1 || !fp2 || !fp3 || !fp_op) {
        printf("Error opening input files.\n");
        return 1;
    }

    fscanf(fp1, "%s%s%s", label, opcode, operand);

    // ---- PASS 1 ----
    if (strcmp(opcode, "START") == 0) {
        start = (int)strtol(operand, NULL, 16);
        locctr = start;
        fprintf(fp3, "%s\t%s\t%s\n", label, opcode, operand);
        fscanf(fp1, "%s%s%s", label, opcode, operand);
    } else {
        locctr = 0;
    }

    while (strcmp(opcode, "END") != 0) {
        fprintf(fp3, "%04X\t", locctr);

        if (strcmp(label, "") != 0) {
            fprintf(fp2, "%s\t%04X\n", label, locctr);
        }

        // Check opcode in opcode.txt
        int found = 0;
        char temp_mnemonic[10], temp_code[10];
        rewind(fp_op);
        while (fscanf(fp_op, "%s%s", temp_mnemonic, temp_code) != EOF) {
            if (strcmp(opcode, temp_mnemonic) == 0) {
                locctr += INSTR_SIZE;
                found = 1;
                break;
            }
        }

        if (!found) {
            if (strcmp(opcode, "WORD") == 0)
                locctr += 3;
            else if (strcmp(opcode, "RESW") == 0)
                locctr += 3 * atoi(operand);
            else if (strcmp(opcode, "RESB") == 0)
                locctr += atoi(operand);
            else if (strcmp(opcode, "BYTE") == 0)
                locctr += strlen(operand) - 3;
        }

        fprintf(fp3, "%s\t%s\t%s\n", label, opcode, operand);
        fscanf(fp1, "%s%s%s", label, opcode, operand);
    }

    fprintf(fp3, "%04X\t%s\t%s\t%s\n", locctr, label, opcode, operand);
    finaddr = locctr;
    length = finaddr - start;

    fclose(fp1);
    fclose(fp2);
    fclose(fp3);
    fclose(fp_op);

    // ---- PASS 2 ----
    fp4 = fopen("output1.txt", "w");
    fp5 = fopen("symtab1.txt", "r");
    fp6 = fopen("intermediate1.txt", "r");
    fp7 = fopen("objcode1.txt", "w");
    fp_op = fopen("opcode.txt", "r");

    if (!fp4 || !fp5 || !fp6 || !fp7 || !fp_op) {
        printf("Error opening output files.\n");
        return 1;
    }

    fscanf(fp6, "%s%s%s", label, opcode, operand);
    int startAddr = (int)strtol(operand, NULL, 16);

    if (strcmp(opcode, "START") == 0) {
        fprintf(fp4, "\t%s\t%s\t%s\n", label, opcode, operand);
        fprintf(fp7, "H^%s^%06X^%06X\n", label, startAddr, length);

        fscanf(fp6, "%x%s%s%s", &address, label, opcode, operand);
        st = address;
        fprintf(fp7, "T^%06X^", address);
    }

    while (strcmp(opcode, "END") != 0) {
        // Handle BYTE
        if (strcmp(opcode, "BYTE") == 0) {
            fprintf(fp4, "%04X\t%s\t%s\t%s\t", address, label, opcode, operand);
            fprintf(fp7, "^");
            for (int k = 2; k < strlen(operand) - 1; k++) {
                fprintf(fp4, "%02X", operand[k]);
                fprintf(fp7, "%02X", operand[k]);
            }
            fprintf(fp4, "\n");
        }
        // Handle WORD
        else if (strcmp(opcode, "WORD") == 0) {
            int val = atoi(operand);
            fprintf(fp4, "%04X\t%s\t%s\t%s\t%06X\n", address, label, opcode, operand, val);
            fprintf(fp7, "^%06X", val);
        }
        // Handle RESB, RESW (no object code)
        else if (strcmp(opcode, "RESB") == 0 || strcmp(opcode, "RESW") == 0) {
            fprintf(fp4, "%04X\t%s\t%s\t%s\n", address, label, opcode, operand);
        }
        // Handle regular opcode
        else {
            rewind(fp_op);
            char temp_mnemonic[10], temp_code[10];
            int found = 0;
            while (fscanf(fp_op, "%s%s", temp_mnemonic, temp_code) != EOF) {
                if (strcmp(opcode, temp_mnemonic) == 0) {
                    found = 1;
                    break;
                }
            }

            rewind(fp5);
            int symbol_found = 0;
            while (fscanf(fp5, "%s%x", symbol, &add) != EOF) {
                if (strcmp(operand, symbol) == 0) {
                    symbol_found = 1;
                    break;
                }
            }

            if (found && symbol_found) {
                fprintf(fp4, "%04X\t%s\t%s\t%s\t%s%03X\n", address, label, opcode, operand, temp_code, add);
                fprintf(fp7, "^%s%03X", temp_code, add);
            } else {
                fprintf(fp4, "%04X\t%s\t%s\t%s\tERROR\n", address, label, opcode, operand);
            }
        }

        fscanf(fp6, "%x%s%s%s", &address, label, opcode, operand);
    }

    fprintf(fp4, "%04X\t%s\t%s\t%s\n", address, label, opcode, operand);
    fprintf(fp7, "\nE^%06X\n", st);

    fclose(fp4);
    fclose(fp5);
    fclose(fp6);
    fclose(fp7);
    fclose(fp_op);

    printf("\n✅ Single Pass Assembler Completed Successfully!\n");

    return 0;
}
