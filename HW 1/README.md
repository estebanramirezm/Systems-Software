# Homework 1: P-Machine Simulator

## Description

This program implements a virtual machine that simulates the execution of a stack-based P-Machine. It reads an input file containing machine instructions, processes them according to the defined ISA (Instruction Set Architecture) given in the assignment's description file, and outputs the program's execution trace including register states and stack operations.

## Compilation Instructions

To compile the program, use the following command in the terminal:
```
gcc -o vm vm.c
```

## Usage

Run the compiled executable with an input file as an argument:

```
./vm input.txt
```

This will execute the instructions in `input.txt` and display the execution trace on the console.
