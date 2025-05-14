/*
 * COP 3402 Systems Software
 * Homework 4 (part b): Updated P-Machine
 * Author: Esteban Ramirez
 * Date: April 14th, 2025
 * Description: Implements a virtual machine that simulates the execution of a P-Machine. Updated support for MOD
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_SIZE 500
#define UNUSED 10
#define TEXT_START 10
#define STACK_START 500

// VM Registers and Memory
int PAS[ARRAY_SIZE] = {0};
int ACT_BARS[ARRAY_SIZE] = {0};
int BP = 499, SP = 500, PC = 10;
int EOP = 1; // End Of Program flag

// Helper function that folows static links l levels down. Given in assignment file.
int base(int bp, int l)
{
    int arb = bp;
    while (l > 0)
    {
        arb = PAS[arb];
        l--;
    }
    return arb;
}

// Prints the instruction, L and M fields, PC and the stack contents.
void printStack(const char instr[], int l, int m, int pc, int bp, int sp)
{
    // Print instruction, L, M fields, and PC
    printf("    %-4s %-3d %-3d %-3d %-3d %-3d ", instr, l, m, pc, bp, sp);

    // Iterate through stack and print elements
    for (int i = STACK_START - 1; i >= sp; i--)
    {
        if (ACT_BARS[i] == 1) // Print activation bars
            printf("| ");
        printf("%d ", PAS[i]);
    }

    // Newline for formatting
    printf("\n");
}

// Implements a virtual machine that simulates the execution of a P-Machine. Requires a file to be passed as an argument.
int main(int argc, char *argv[])
{
    // Input validation to prevent running the program incorrectly by not passing an input file
    if (argc != 2)
    {
        printf("Usage: %s <input file>\n", argv[0]);
        return 1;
    }

    // Open file
    FILE *input_file = fopen(argv[1], "r");
    if (!input_file) // File not found / can't be opened
    {
        printf("Error: File was not found or can't be opened\n");
        return 1;
    }
    
    // Load program into the TEXT segment (starting at TEXT_START)
    int textIndex = TEXT_START;
    while (fscanf(input_file, "%d %d %d", &PAS[textIndex], &PAS[textIndex + 1], &PAS[textIndex + 2]) != EOF)
    {
        textIndex += 3;
    }

    // Close file
    fclose(input_file);

    // Print initial register values
    printf("                 PC  BP  SP  Stack\n");
    printf("Initial values:  %-3d %-3d %-3d\n\n", PC, BP, SP);

    // Main execution loop. Implements the P-Machine.
    while (EOP)
    {
        // Fetch the next instruction (3 ints)
        int IR_OP = PAS[PC];
        int IR_L = PAS[PC + 1];
        int IR_M = PAS[PC + 2];
        PC += 3;
        char instruction[4] = "";

        switch (IR_OP)
        {
        case 1: // LIT" push IR_M onto the stack.
            SP--;
            PAS[SP] = IR_M;
            strcpy(instruction, "LIT");
            break;

        case 2: // OPR: execute operation specified by IR_M.
            switch (IR_M)
            {
            case 0: // RTN: return from subroutine
                ACT_BARS[BP] = 0; // Disable activation bar at this BP index
                SP = BP + 1;
                BP = PAS[SP - 2];
                PC = PAS[SP - 3];
                strcpy(instruction, "RTN");
                break;
            case 1: // ADD +
                PAS[SP + 1] += PAS[SP];
                SP++;
                strcpy(instruction, "ADD");
                break;
            case 2: // SUB -
                PAS[SP + 1] -= PAS[SP];
                SP++;
                strcpy(instruction, "SUB");
                break;
            case 3: // MUL *
                PAS[SP + 1] *= PAS[SP];
                SP++;
                strcpy(instruction, "MUL");
                break;
            case 4: // DIV /
                PAS[SP + 1] /= PAS[SP];
                SP++;
                strcpy(instruction, "DIV");
                break;
            case 5: // EQL (Equal) ==
                PAS[SP + 1] = (PAS[SP + 1] == PAS[SP]);
                SP++;
                strcpy(instruction, "EQL");
                break;
            case 6: // NEQ (Not Equal)!=
                PAS[SP + 1] = (PAS[SP + 1] != PAS[SP]);
                SP++;
                strcpy(instruction, "NEQ");
                break;
            case 7: // LSS (Less Than) <
                PAS[SP + 1] = (PAS[SP + 1] < PAS[SP]);
                SP++;
                strcpy(instruction, "LSS");
                break;
            case 8: // LEQ (Less or Equal) <=
                PAS[SP + 1] = (PAS[SP + 1] <= PAS[SP]);
                SP++;
                strcpy(instruction, "LEQ");
                break;
            case 9: // GTR (Greater Than) >
                PAS[SP + 1] = (PAS[SP + 1] > PAS[SP]);
                SP++;
                strcpy(instruction, "GTR");
                break;
            case 10: // GEQ (Greater or Equal) >=
                PAS[SP + 1] = (PAS[SP + 1] >= PAS[SP]);
                SP++;
                strcpy(instruction, "GEQ");
                break;
            case 11: // MOD (Modulo)
                PAS[SP + 1] = PAS[SP + 1] % PAS[SP];
                SP++;
                strcpy(instruction, "MOD");
                break;
            default: // Error: Invalid instruction
                printf("Invalid OPR instruction.\n");
                EOP = 0;
                break;
            }
            break;

        case 3: // LOD: load value from earlier location in stack.
            SP--;
            PAS[SP] = PAS[base(BP, IR_L) - IR_M];
            strcpy(instruction, "LOD");
            break;

        case 4: // STO: store top-of-stack value into a var slot
            PAS[base(BP, IR_L) - IR_M] = PAS[SP];
            SP++;
            strcpy(instruction, "STO");
            break;
        case 5: // CAL: call a procedure.
            PAS[SP - 1] = base(BP, IR_L); // Static link
            PAS[SP - 2] = BP; // Dynamic link
            PAS[SP - 3] = PC; // Return address
            BP = SP - 1; // Update BP -> points to the static link location
            ACT_BARS[BP] = 1; // Mark this BP index to have an activation bar
            PC = IR_M; // Jump to procedure code
            strcpy(instruction, "CAL"); 
            break;
        case 6: // INC: allocate memory on the stack
            SP -= IR_M;
            strcpy(instruction, "INC");
            break;
        case 7: // JMP: unconditional jump.
            PC = IR_M;
            strcpy(instruction, "JMP");
            break;
        case 8: // JPC: jump if top-of-stack is zero.
            if (PAS[SP] == 0)
            {
                PC = IR_M;
            }
            SP++;
            strcpy(instruction, "JPC");
            break;
        case 9: // SYS: system call -> input/output/halt.
            if (IR_M == 1) // Output
            {
                printf("Output result is: %d\n", PAS[SP]);
                SP++;
                strcpy(instruction, "SYS");
            }
            else if (IR_M == 2) // Input
            {
                printf("Please Enter an Integer: ");
                SP--;
                scanf("%d", &PAS[SP]);
                strcpy(instruction, "SYS");
            }
            else if (IR_M == 3) // Halt
            {
                EOP = 0;
                strcpy(instruction, "SYS");
            }
            break;
        default: // Error: Invalid instruction
            printf("Invalid opcode.\n");
            EOP = 0;
        }

        // Print the stack's state after executing the current instruction
        printStack(instruction, IR_L, IR_M, PC, BP, SP);
    }
    return 0;
}