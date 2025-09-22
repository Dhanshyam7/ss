/* pass2.c - Pass 2 for a simple SIC assembler
   Input:
     intermediate.txt  (ADDR LABEL OPCODE OPERAND)
     symtab.txt        (LABEL ADDR)
     opcode.txt        (MNEMONIC OPCODEHEX)
     length.txt        (program length in hex)
   Output:
     assembly.txt      (human readable listing)
     obj.txt           (H/T/E records)
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAXSYM 1000
#define MAXOP  500
#define MAXOBJ 5000

int hexstr_to_int(const char *s) {
    return (int)strtol(s, NULL, 16);
}

int main(void) {
    FILE *fint = fopen("intermediate.txt", "r");
    FILE *fsym = fopen("symtab.txt", "r");
    FILE *fop  = fopen("opcode.txt", "r");
    FILE *flen = fopen("length.txt", "r");
    FILE *flst = fopen("assembly.txt", "w");
    FILE *fobj = fopen("obj.txt", "w");

    if (!fint || !fsym || !fop || !flen || !flst || !fobj) {
        perror("Error opening files (pass2)");
        return 1;
    }

    /* load opcode table */
    char op_mn[MAXOP][20], op_hex[MAXOP][10];
    int opcount = 0;
    while (fscanf(fop, "%19s %9s", op_mn[opcount], op_hex[opcount]) == 2) {
        opcount++;
        if (opcount >= MAXOP) break;
    }

    /* load symbol table (LABEL ADDR) */
    char sym_label[MAXSYM][50];
    int sym_addr[MAXSYM];
    int symcount = 0;
    while (fscanf(fsym, "%49s %x", sym_label[symcount], &sym_addr[symcount]) == 2) {
        symcount++;
        if (symcount >= MAXSYM) break;
    }

    /* read length */
    char length_str[50];
    int prog_length = 0;
    if (fscanf(flen, "%49s", length_str) == 1) {
        prog_length = hexstr_to_int(length_str);
    }

    /* process intermediate */
    char addr_str[20], label[50], opcode[50], operand[100];
    unsigned int addr = 0;
    char program_name[50] = "";
    unsigned int start_addr = 0;

    /* read first record */
    if (fscanf(fint, "%19s %49s %49s %99s", addr_str, label, opcode, operand) < 3) {
        fprintf(stderr, "intermediate.txt seems malformed or empty\n");
        return 1;
    }

    /* If first opcode is START, write header */
    if (strcmp(opcode, "START") == 0) {
        strcpy(program_name, label);
        start_addr = hexstr_to_int(operand);
        fprintf(flst, "%s\t%s\t%s\n", label, opcode, operand);

        /* Header: program name (6 chars), start (6 hex), length (6 hex) */
        char pname6[7] = "      ";
        strncpy(pname6, program_name, 6);
        fprintf(fobj, "H^%-6s^%06X^%06X\n", pname6, start_addr, prog_length);

        /* read next line */
        if (fscanf(fint, "%x %49s %49s %99s", &addr, label, opcode, operand) < 4) {
            fprintf(stderr, "No instructions after START\n");
            fprintf(fobj, "E^%06X\n", start_addr);
            fcloseall: ;
            fclose(fint); fclose(fsym); fclose(fop); fclose(flen); fclose(flst); fclose(fobj);
            return 0;
        }
    } else {
        /* no START: set start_addr to first address */
        addr = hexstr_to_int(addr_str);
        start_addr = addr;
    }

    /* Prepare for TEXT records */
    char textbuf[MAXOBJ] = "";   /* concatenated hex objectcodes (no separators) */
    unsigned int text_start = 0;
    int text_len = 0; /* bytes */

    while (1) {
        if (strcmp(opcode, "END") == 0) {
            fprintf(flst, "%04X\t%s\t%s\t%s\n", addr, label, opcode, operand);
            break;
        }

        char objcode[256] = "";
        int obj_bytes = 0;

        /* handle opcodes in opcode table (instructions) */
        int is_op = 0, opidx = -1;
        for (int i = 0; i < opcount; i++) {
            if (strcmp(opcode, op_mn[i]) == 0) { is_op = 1; opidx = i; break; }
        }

        if (is_op) {
            /* operand may be LABEL or LABEL,X */
            int indexed = 0;
            char sym[80];
            strcpy(sym, operand);
            char *comma = strchr(sym, ',');
            if (comma) {
                /* handle "LABEL,X" */
                *comma = '\0';
                if (strcasecmp(comma + 1, "X") == 0) indexed = 1;
            }

            /* look up symbol */
            int targ_addr = 0;
            int found_sym = 0;
            if (strlen(sym) > 0) {
                for (int k = 0; k < symcount; k++) {
                    if (strcmp(sym_label[k], sym) == 0) {
                        targ_addr = sym_addr[k];
                        found_sym = 1;
                        break;
                    }
                }
            }
            if (!found_sym && strlen(sym) > 0) {
                fprintf(stderr, "Warning: symbol '%s' not found. Using 0.\n", sym);
                targ_addr = 0;
            }
            if (indexed) targ_addr |= 0x8000; /* set indexing bit for SIC (conventional) */

            /* compose object code: opcodeHex + 4-hex-digit address (2 bytes) */
            /* opcode hex from op_hex[opidx] (assumed two hex digits) */
            sprintf(objcode, "%s%04X", op_hex[opidx], targ_addr & 0xFFFF);
            obj_bytes = 3; /* instruction is 3 bytes */
            fprintf(flst, "%04X\t%s\t%s\t%s\t%s\n", addr, label, opcode, operand, objcode);
        }
        else if (strcmp(opcode, "WORD") == 0) {
            int val = atoi(operand);
            sprintf(objcode, "%06X", val & 0xFFFFFF); /* 3 bytes */
            obj_bytes = 3;
            fprintf(flst, "%04X\t%s\t%s\t%s\t%s\n", addr, label, opcode, operand, objcode);
        }
        else if (strcmp(opcode, "BYTE") == 0) {
            /* C'AB' -> convert each char to two hex digits
               X'F1'  -> take hex directly (must be even length) */
            if (operand[0] == 'C' && operand[1] == '\'') {
                int len = (int)strlen(operand) - 3;
                if (len < 0) len = 0;
                char temp[200] = "";
                for (int i = 2; i < 2 + len; i++) {
                    char hex[3];
                    sprintf(hex, "%02X", (unsigned char)operand[i]);
                    strcat(temp, hex);
                }
                strcpy(objcode, temp);
                obj_bytes = (int)strlen(objcode) / 2;
            } else if (operand[0] == 'X' && operand[1] == '\'') {
                int hexlen = (int)strlen(operand) - 3;
                if (hexlen < 0) hexlen = 0;
                char temp[200] = "";
                strncpy(temp, operand + 2, hexlen);
                temp[hexlen] = '\0';
                strcpy(objcode, temp);
                obj_bytes = (int)strlen(objcode) / 2;
            } else {
                /* malformed - place single byte with ASCII of first char */
                unsigned char ch = operand[0];
                sprintf(objcode, "%02X", ch);
                obj_bytes = 1;
            }
            fprintf(flst, "%04X\t%s\t%s\t%s\t%s\n", addr, label, opcode, operand, objcode);
        }
        else if (strcmp(opcode, "RESW") == 0 || strcmp(opcode, "RESB") == 0) {
            /* When encountering a reservation, flush current text record */
            fprintf(flst, "%04X\t%s\t%s\t%s\n", addr, label, opcode, operand);
            if (text_len > 0) {
                /* flush */
                fprintf(fobj, "T^%06X^%02X^%s\n", text_start, text_len, textbuf);
                textbuf[0] = '\0';
                text_len = 0;
            }
            /* advance address (nothing emitted) */
            if (strcmp(opcode, "RESW") == 0) {
                /* handled in pass1; here just nothing */
            }
            /* read next line and continue */
            if (fscanf(fint, "%x %49s %49s %99s", &addr, label, opcode, operand) < 4) break;
            continue;
        }
        else {
            /* unknown opcode - just write to listing and warn */
            fprintf(stderr, "Warning: unknown opcode '%s' at %04X\n", opcode, addr);
            fprintf(flst, "%04X\t%s\t%s\t%s\t%s\n", addr, label, opcode, operand, "??");
            /* treat as 3 bytes to keep addresses going */
            obj_bytes = 3;
            strcpy(objcode, "000000");
        }

        /* Append object code to text record (unless RESW/RESB which were handled) */
        if (obj_bytes > 0) {
            if (text_len == 0) {
                text_start = addr;
                textbuf[0] = '\0';
            }
            /* if adding this object would exceed 30 bytes, flush first */
            if (text_len + obj_bytes > 30) {
                fprintf(fobj, "T^%06X^%02X^%s\n", text_start, text_len, textbuf);
                textbuf[0] = '\0';
                text_len = 0;
                text_start = addr;
            }
            /* append */
            strcat(textbuf, objcode);
            text_len += obj_bytes;
        }

        /* read next intermediate line */
        if (fscanf(fint, "%x %49s %49s %99s", &addr, label, opcode, operand) < 4) {
            /* no more lines */
            break;
        }
    } /* end while */

    /* flush last text record if any */
    if (text_len > 0) {
        fprintf(fobj, "T^%06X^%02X^%s\n", text_start, text_len, textbuf);
    }

    /* End record */
    fprintf(fobj, "E^%06X\n", start_addr);

    printf("Pass 2 completed. Wrote assembly.txt and obj.txt\n");

    fclose(fint);
    fclose(fsym);
    fclose(fop);
    fclose(flen);
    fclose(flst);
    fclose(fobj);
    return 0;
}
