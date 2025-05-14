# PL/0 Compiler and Virtual Machine

## Overview

This project is a multi-stage implementation of a complete compiler and execution environment for the PL/0 programming language, developed across four major assignments. It showcases the construction of core components typically found in a compiler toolchain, including lexical analysis, parsing, code generation, and a stack-based virtual machine. The implementation uses C and gcc.

## Components

### 🧮 Homework 1: P-Machine Virtual Machine

* A stack-based virtual machine (VM) that executes instructions defined by a custom instruction set architecture (ISA).
* Reads compiled code and outputs a detailed execution trace including stack operations and register states.

### 🔤 Homework 2: Lexical Analyzer

* Tokenizes PL/0 source code into a list of tokens and lexemes.
* Identifies valid identifiers, numbers, keywords, symbols, and skips comments.
* Performs error checking for invalid symbols and constraints like identifier length.

### 🏗️ Homework 3: PL/0 Compiler (Phase 1)

* Adds a recursive descent parser and code generator on top of the lexical analyzer.
* Outputs an intermediate assembly (p-code) format to be interpreted by the VM.
* Includes syntax error reporting and elf.txt generation as VM input.

### ⚙️ Homework 4: Extended Compiler + Final VM

* Extends the compiler to support procedures, nested lexical scopes, and additional grammar rules.
* Enhances the virtual machine to support new opcodes (e.g., `mod`, procedure calls).
* Includes robust error handling for syntax and semantic issues.
* Provides comprehensive test cases demonstrating correct and incorrect PL/0 programs.

## Compilation and Usage

Each component includes its own compilation and usage instructions using `gcc`.

## Author

**Esteban Ramirez**
Email: [esteban.ramirezmejia@ucf.edu](mailto:esteban.ramirezmejia@ucf.edu)
