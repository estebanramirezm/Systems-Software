# Homework 3: Tiny PL/0 Compiler

## Description

This program implements a compiler that processes a source program written in the PL/0 language. It performs the following:

 1. **Lexical Analysis**: Tokenizes the input program and generates a lexeme table and token list.
 2. **Parsing**: Does syntax analysis to make sure the input follows PL/0 grammar (provided in project's requirements)
 3. **Code Generation**: Produces assembly code that can be interpreted or executed in the VM machine previously coded on HW 1.

Additionally, it has: 

 **Error Handling**: Detects syntax errors and reports them with descriptive messages.

## Compilation Instructions

To compile the program, use the following command in the terminal:
```
gcc -o parsercodegen parsercodegen.c
```

## Usage

Run the compiled executable with an input file as an argument:

```
./parsercodegen input.txt
```

This will process `input.txt` and display the assembly code and symbol table on the console. It will also generate an `elf.txt` file, which serves as input for the PL/0 Virtual Machine previously coded on HW 1.

## Additional info

This submission includes 15 input/output (elf.txt) files. The first one showcases the expected output from the test case given. The rest of them showcase the different error messages that can be triggered by incorrect grammar/syntax on the PL/0 input files, as required by the assignment.
