// COMP1521 20T3 Assignment 1: mips_sim -- a MIPS simulator
// starting point code v0.1 - 13/10/20


// PUT YOUR HEADER COMMENT HERE


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#define MAX_LINE_LENGTH 256
#define INSTRUCTIONS_GROW 64


// ADD YOUR #defines HERE
#define REG_NUM     32
#define S_SHIFT     21
#define T_SHIFT     16
#define D_SHIFT     11
#define BIT_WIDTH   5
#define I_BIT_WIDTH 16

#define REG_V0		2
#define REG_A0		4

#define GET_S(ins)		(((ins) >> S_SHIFT) & ((1 << BIT_WIDTH) - 1))
#define GET_T(ins)		(((ins) >> T_SHIFT) & ((1 << BIT_WIDTH) - 1))
#define GET_D(ins)		(((ins) >> D_SHIFT) & ((1 << BIT_WIDTH) - 1))
#define GET_I(ins)		((ins) & ((1 << I_BIT_WIDTH) - 1))

void execute_instructions(int n_instructions,
                          uint32_t instructions[n_instructions],
                          int trace_mode);
char *process_arguments(int argc, char *argv[], int *trace_mode);
uint32_t *read_instructions(char *filename, int *n_instructions_p);
uint32_t *instructions_realloc(uint32_t *instructions, int n_instructions);


// ADD YOUR FUNCTION PROTOTYPES HERE
void write_reg(uint32_t *reg, int index, int value) {
    if (index == 0) {
        return;
    }
    if (index > 32) {
        fprintf(stderr, "Invalid Register\n");
        return;
    }
    reg[index] = value;
}

int read_reg(uint32_t *reg, int index) {
    if (index > 32) {
        fprintf(stderr, "Invalid Register\n");
        return -1;
    }
    return reg[index];
}

void print_msg(int trace_mode, int pc, int ins, char *operation, int r1, int r2, int r3, int val, int with_imme) {
	if (!trace_mode)
		return;
	if (with_imme) 
		printf("%d: 0x%08X %s  $%d, $%d, %d\n", pc, ins, operation, r1, r2, r3);
	else
		printf("%d: 0x%08X %s  $%d, $%d, $%d\n", pc, ins, operation, r1, r2, r3);
	printf(">>> $%d = %d\n", r1, val);
}


// YOU SHOULD NOT NEED TO CHANGE MAIN

int main(int argc, char *argv[]) {
    int trace_mode;
    char *filename = process_arguments(argc, argv, &trace_mode);

    int n_instructions;
    uint32_t *instructions = read_instructions(filename, &n_instructions);

    execute_instructions(n_instructions, instructions, trace_mode);

    free(instructions);
    return 0;
}


// simulate execution of  instruction codes in  instructions array
// output from syscall instruction & any error messages are printed
//
// if trace_mode != 0:
//     information is printed about each instruction as it executed
//
// execution stops if it reaches the end of the array

void execute_instructions(int n_instructions,
                          uint32_t instructions[n_instructions],
                          int trace_mode) {
    // REPLACE CODE BELOW WITH YOUR CODE

    uint32_t reg[REG_NUM] = {0};
    int pc = 0;
    while (pc < n_instructions) {
        uint32_t ins = instructions[pc];
        // if (trace_mode) {
        //     printf("%d: 0x%08X\n", pc, instructions[pc]);
        // }

		int reg_s = GET_S(ins);
		int reg_t = GET_T(ins);
		int reg_d = GET_D(ins);
		int16_t imme = GET_I(ins);
        /* add instruction */
        if ((ins & 0xFC0007FF) == 0x20) {
			
            uint32_t reg_s_val = read_reg(reg, reg_s);
            uint32_t reg_t_val = read_reg(reg, reg_t);
            write_reg(reg, reg_d, reg_s_val + reg_t_val);
			print_msg(trace_mode, pc, ins, "add", reg_d, reg_s, reg_t, reg_s_val + reg_t_val, 0);
			// printf("%d: 0x%08X add  $%d, $%d, $%d\n", pc, ins, reg_d, reg_s, reg_t);
			// printf(">>> $%d = %d\n", reg_d, reg_s_val + reg_t_val);
        /* sub instruction */
        } else if ((ins & 0xFC0007FF) == 0x22) {
            uint32_t reg_s_val = read_reg(reg, reg_s);
            uint32_t reg_t_val = read_reg(reg, reg_t);
            write_reg(reg, reg_d, reg_s_val - reg_t_val);
			print_msg(trace_mode, pc, ins, "sub", reg_d, reg_s, reg_t, reg_s_val - reg_t_val, 0);
			// printf("%d: 0x%08X sub  $%d, $%d, $%d\n", pc, ins, reg_d, reg_s, reg_t);
			// printf(">>> $%d = %d\n", reg_d, reg_s_val - reg_t_val);
		/* slt instruction */
        } else if ((ins & 0xFC0007FF) == 0x2A) {
			uint32_t reg_s_val = read_reg(reg, reg_s);
            uint32_t reg_t_val = read_reg(reg, reg_t);
            write_reg(reg, reg_d, reg_s_val < reg_t_val);
			print_msg(trace_mode, pc, ins, "slt", reg_d, reg_s, reg_t, reg_s_val < reg_t_val, 0);
			// printf(">>> $%d = %d\n", reg_d, reg_s_val < reg_t_val);
		/* mul instruction */
        } else if ((ins & 0xFC0007FF) == 0x70000002) {
			uint32_t reg_s_val = read_reg(reg, reg_s);
            uint32_t reg_t_val = read_reg(reg, reg_t);
            write_reg(reg, reg_d, reg_s_val * reg_t_val);
			print_msg(trace_mode, pc, ins, "mul", reg_d, reg_s, reg_t, reg_s_val * reg_t_val, 0);
			// printf(">>> $%d = %d\n", reg_d, reg_s_val * reg_t_val);
		/* beq instruction */
        } else if ((ins & 0xFC000000) == 0x10000000) {
			uint32_t reg_s_val = read_reg(reg, reg_s);
            uint32_t reg_t_val = read_reg(reg, reg_t);
			if (trace_mode) {
				printf("%d: 0x%08X beq  $%d, $%d, %hd\n", pc, ins, reg_s, reg_t, imme);
			}
			if (reg_s_val == reg_t_val) {
				pc += imme;
                if (trace_mode) {
                    printf(">>> pc = %d\n", pc + imme);
                }
                continue;
			} else if (trace_mode) {
                printf(">>> pc = %d\n", pc);
            }
		/* bne instruction */
        } else if ((ins & 0xFC000000) == 0x14000000) {
            uint32_t reg_s_val = read_reg(reg, reg_s);
            uint32_t reg_t_val = read_reg(reg, reg_t);
			if (trace_mode) {
				printf("%d: 0x%08X bne  $%d, $%d, %hd\n", pc, ins, reg_s, reg_t, imme);
			}
			if (reg_s_val != reg_t_val) {
				pc += imme;
                if (trace_mode) {
                    printf(">>> pc = %d\n", pc + imme);
                }
                continue;
			} else if (trace_mode) {
                printf(">>> pc = %d\n", pc);
            }
		/* addi instruction */
        } else if ((ins & 0xFC000000) == 0x20000000) {
			uint32_t reg_s_val = read_reg(reg, reg_s);
			write_reg(reg, reg_t, reg_s_val + imme);
			print_msg(trace_mode, pc, ins, "addi", reg_t, reg_s, imme, reg_s_val + imme, 1);
			// printf(">>> $%d = %d\n", reg_t, reg_s_val + imme);
		/* ori instruction */
        } else if ((ins & 0xFC000000) == 0x34000000) {
			uint32_t reg_s_val = read_reg(reg, reg_s);
			write_reg(reg, reg_t, reg_s_val | imme);
			print_msg(trace_mode, pc, ins, "ori", reg_t, reg_s, imme, reg_s_val | imme, 1);
			// printf(">>> $%d = %d\n", reg_t, reg_s_val | imme);
		/* lui instruction */
        } else if ((ins & 0xFFE00000) == 0x3C000000) {
			write_reg(reg, reg_t, imme << 16);
			if (trace_mode) {
				printf("%d: 0x%08X lui  $%d, %d\n", pc, ins, reg_t, imme);
				printf(">>> $%d = %d\n", reg_t, imme << 16);
			}
		/* syscall instruction */
        } else if (ins == 0xC) {
			if (trace_mode) {
				printf("%d: 0x%08X syscall %d\n", pc, ins, reg[REG_V0]);
				printf("<<< ");
			}
			if (reg[REG_V0] == 1)
				printf("%d", reg[REG_A0]);
			else if (reg[REG_V0] == 10)
				exit(0);
			else if (reg[REG_V0] == 11)
				printf("%c", reg[REG_A0]);
            else {
                printf("Unknown system call: %d\n", reg[REG_V0]);
                exit(0);
            }
            
			if (trace_mode) {
				printf("\n");
			}
		} else {
            
        }

        pc++;
		// printf("n_instructions=%d\tpc=%d\n", n_instructions, pc);
    }
}



// ADD YOUR FUNCTIONS HERE



// YOU DO NOT NEED TO CHANGE CODE BELOW HERE


// check_arguments is given command-line arguments
// it sets *trace_mode to 0 if -r is specified
//          *trace_mode is set to 1 otherwise
// the filename specified in command-line arguments is returned

char *process_arguments(int argc, char *argv[], int *trace_mode) {
    if (
        argc < 2 ||
        argc > 3 ||
        (argc == 2 && strcmp(argv[1], "-r") == 0) ||
        (argc == 3 && strcmp(argv[1], "-r") != 0)) {
        fprintf(stderr, "Usage: %s [-r] <file>\n", argv[0]);
        exit(1);
    }
    *trace_mode = (argc == 2);
    return argv[argc - 1];
}


// read hexadecimal numbers from filename one per line
// numbers are return in a malloc'ed array
// *n_instructions is set to size of the array

uint32_t *read_instructions(char *filename, int *n_instructions_p) {
    FILE *f = fopen(filename, "r");
    if (f == NULL) {
        fprintf(stderr, "%s: '%s'\n", strerror(errno), filename);
        exit(1);
    }

    uint32_t *instructions = NULL;
    int n_instructions = 0;
    char line[MAX_LINE_LENGTH + 1];
    while (fgets(line, sizeof line, f) != NULL) {

        // grow instructions array in steps of INSTRUCTIONS_GROW elements
        if (n_instructions % INSTRUCTIONS_GROW == 0) {
            instructions = instructions_realloc(instructions, n_instructions + INSTRUCTIONS_GROW);
        }

        char *endptr;
        instructions[n_instructions] = strtol(line, &endptr, 16);
        if (*endptr != '\n' && *endptr != '\r' && *endptr != '\0') {
            fprintf(stderr, "%s:line %d: invalid hexadecimal number: %s",
                    filename, n_instructions + 1, line);
            exit(1);
        }
        n_instructions++;
    }
    fclose(f);
    *n_instructions_p = n_instructions;
    // shrink instructions array to correct size
    instructions = instructions_realloc(instructions, n_instructions);
    return instructions;
}


// instructions_realloc is wrapper for realloc
// it calls realloc to grow/shrink the instructions array
// to the speicfied size
// it exits if realloc fails
// otherwise it returns the new instructions array
uint32_t *instructions_realloc(uint32_t *instructions, int n_instructions) {
    instructions = realloc(instructions, n_instructions * sizeof *instructions);
    if (instructions == NULL) {
        fprintf(stderr, "out of memory");
        exit(1);
    }
    return instructions;
}