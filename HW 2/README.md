# Homework 2: Lexical Analyzer

## Description

This program implements a lexical analyzer for the PL/0 programming language. It reads a source program from an input file, tokenizes the input, and outputs the following:

- Lexeme Table – each lexeme along with its corresponding token type.
- Token List –  tokenized version of the input program.

The program handles:

-   Identifiers (limited to 11 characters).
-   Numbers (limited to 5 digits).
-   Reserved keywords (e.g., `var`, `begin`, `if`, etc.).
-   Special symbols (e.g., `+`, `-`, `:=`, `>=`, `<=`, etc.).
-   Multi-line comments (`/* ... */`)** are ignored.
-   Error detection for invalid symbols, too-long identifiers, and numbers.

## Compilation Instructions

To compile the program, use the following command in the terminal:
```
gcc -o lex lex.c
```

## Usage

Run the compiled executable with an input file as an argument:

```
./lex input.txt
```

This will execute the instructions in `input.txt` and display the execution trace on the console.
