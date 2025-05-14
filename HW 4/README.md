# Homework 4: PL/0 Compiler and VM

## Description

This program implements a compiler for the PL/0 programming language (extended from Homework 3). This version adds support for:

 1. **Lexical Analysis**: Tokenizes the input program and generates a lexeme table and token list.
 2. **Parsing**: Does syntax analysis to make sure the input follows PL/0 grammar (provided in project's requirements)
 3. **Code Generation**: Produces assembly code that can be interpreted or executed in the VM machine previously coded on HW 1.
 4. **Error Handling**: Reports syntax and semantic errors with descriptive messages

## Compilation Instructions

### PL/0 Compiler
Use the following command in the terminal:
```
gcc -std=c17 -Wall -o hw4compiler hw4compiler.c
```

### Virtual Machine
Use the following command in the terminal:
```
gcc -std=c17 -Wall -o vm vm.c
```

## Usage

### PL/0 Compiler
Use the following command in the terminal:
```
./hw4compiler input.txt
```
This will process `input.txt` and display the assembly code and symbol table on the console. It will also generate an `elf.txt` file, which serves as input for the PL/0 VM.

### Virtual Machine
Use the following command in the terminal:
```
./vm elf.txt
```
This will execute the compiled p-code and display the output (results from write statements) and the stack trace.

## Contents

- `hw4compiler.c` - The main compiler source code
- `vm.c` - Updated Virtual Machine source code
- `README.md` — This document
- `test1_...` - Input/Output for test case 1, as well as the elf.txt and VM output. Shows correct functioning of the program.
- `test2_...` - Input/Output for test case 2, as well as the elf.txt and VM output. Shows correct functioning of the program.
- `/Errors` - A folder containing the following (error) test cases
    - `error1_...` - Input/Output for test case 3, which shows one error case for call. No elf.txt generated.
    - `error2_...` - Input/Output for test case 4, which shows one error case for procedure. No elf.txt generated.
