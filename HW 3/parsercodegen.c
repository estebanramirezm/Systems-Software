/*
 * COP 3402 Systems Software
 * Homework 3: PL/O Tiny Parser 
 * Author: Esteban Ramirez
 * Date: March 16th, 2025
 * Description: Implements a PL/0 Tiny Compiler. Generates P-code instructions
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LEXEMES 10000   // From lex.c
#define MAX_ID_LENGTH 11    // From lex.c
#define MAX_NUM_LENGTH 5    // From lex.c
#define MAX_LEXEME_LENGTH 64 // From lex.c
#define MAX_INSTRUCTIONS 9999
#define MAX_SYMBOLS 500

// P-code opcodes
#define LIT 1    // Load literal
#define OPR 2  // Arithmetic and logical operations
#define LOD 3    // Load variable
#define STO 4    // Store variable
#define INC 6    // Allocate memory
#define JMP 7    // Jump
#define JPC 8    // Jump conditional
#define SYS 9    // System call (HALT)

// Arithmetic/Comparison operations
#define ADD 2   // Addition
#define SUB 3   // Subtraction
#define MUL 4   // Multiplication
#define DIV 5   // Division
#define OPR_MOD 7  // Modulus
#define MOD OPR_MOD
#define ODD 6   // Odd test
#define EQL 8   // Equals
#define NEQ 9   // Not equals
#define LSS 10  // Less than
#define LEQ 11  // Less than or equal
#define GTR 12  // Greater than
#define GEQ 13  // Greater than or equal

// Data structures

// Tokens
typedef enum {
    modsym = 1, identsym = 2, numbersym = 3, plussym = 4, minussym = 5, multsym = 6, slashsym = 7, fisym = 8, 
    eqlsym = 9, negsym = 10, neqsym, lessym = 11, legsym = 12, gtrsym = 13, gegsym = 14, lparentsym = 15, rparentsym = 16, 
    commasym = 17, semicolonsym = 18, periodsym = 19, becomessym = 20, beginsym = 21, endsym = 22, ifsym = 23, thensym = 24, 
    whilesym = 25, dosym = 26, callsymNon = 27, constsym = 28, varsym = 29, procsymNon = 30, writesym = 31, readsym = 32, 
    elsesym = 33, oddsym = 34 
} tokenType; 

// Reserved words
typedef struct {
    char *name;
    tokenType token;
} ReservedWord;

// List of reserved words
ReservedWord reservedWords[] = 
{
    {"const", constsym}, {"var", varsym}, {"begin", beginsym}, {"end", endsym}, 
    {"if", ifsym}, {"fi", fisym}, {"then", thensym}, {"else", elsesym}, 
    {"while", whilesym}, {"do", dosym}, {"read", readsym}, {"write", writesym}, 
    {"mod", modsym}
}; 

// Lexeme entry
typedef struct {
    char lexeme[MAX_LEXEME_LENGTH];
    int token;
    char errorMsg[MAX_LEXEME_LENGTH];
    int isError;
} LexemeEntry;


// Instruction
typedef struct {
    int op;  // Opcode
    int l;   // Level
    int m;   // Modifier (address, value, etc.)
} Instruction;


// Symbol table
typedef struct {
    int kind;      // 1 = const, 2 = var, 3 = procedure
    char name[MAX_LEXEME_LENGTH]; 
    int value;     // For constants
    int level;     // Scope level
    int address;   // Memory location
    int mark;      // 0 if active, 1 if marked for deletion
} SymbolEntry;

// Initializations for Global Variables

// Token list 
int tokenIndex = 0;
tokenType currentToken;

// Number of reserved words
int numReservedWords = sizeof(reservedWords) / sizeof(ReservedWord); 

// Lexeme list
LexemeEntry lexemes[MAX_LEXEMES];
int lexCount = 0;

// Symbol table
SymbolEntry symbolTable[MAX_SYMBOLS];
int symbolCount = 0;

// P-code instructions
Instruction instructions[MAX_INSTRUCTIONS];
int instructionCount = 0;

// Function Prototypes

// Lexical Analyzer function prototypes -> From lex.c
tokenType getToken(char *lexeme);
void addToken(int token, const char *lexeme);
void addErrorToken(const char *lexeme, const char *errMsg);
void lexicalAnalyzer(FILE *input);

// Tiny PL/0 Compiler
void getNextToken();
void program();
void block();
void constDeclaration();
int varDeclaration();
void statement();
void expression();
void term();
void factor();
void condition();
void emit(int op, int l, int m);
int symbolTableCheck(char *name);
void printAssemblyCode();
void printSymbolTable();
void error(const char *msg);
const char* getKindName(int kind);

// Error Function
void error(const char *msg) 
{
    printf("Error: %s\n", msg);

    // Print error to output file
    FILE *output = fopen("elf.txt", "w");
    fprintf(output, "Error: %s\n", msg);
    fclose(output);

    // End program unsucessfully
    exit(1);
}

// Lexical Analyzer functions -> From lex.c
tokenType getToken(char *lexeme) {
    for (int i = 0; i < numReservedWords; i++) {
        if (strcmp(lexeme, reservedWords[i].name) == 0)
            return reservedWords[i].token;
    }
    return identsym;
}

// Function that adds a lexeme (+ token type) to the lexeme table
void addToken(int token, const char *lexeme)
{
    // Check for available space
    if (lexCount < MAX_LEXEMES)
    {
        // Store the lexeme
        strncpy(lexemes[lexCount].lexeme, lexeme, MAX_LEXEME_LENGTH - 1);
        lexemes[lexCount].lexeme[MAX_LEXEME_LENGTH - 1] = '\0';
        lexemes[lexCount].token = token; // Token type

        // Indicate its a valid lexeme (no error flag/message)
        lexemes[lexCount].errorMsg[0] = '\0';
        lexemes[lexCount].isError = 0;

        // Continue
        lexCount++;
    }
    else
    {
        fprintf(stderr, "Error: Lexeme buffer overflow.\n"); // Debugging - edge cases
        exit(1);
    }
}

// Function that adds an invalid lexeme and stores an error message for it
void addErrorToken(const char *lexeme, const char *errMsg)
{
    // Check for available space
    if (lexCount < MAX_LEXEMES)
    {
        // Store the invalid lexeme
        strncpy(lexemes[lexCount].lexeme, lexeme, MAX_LEXEME_LENGTH - 1);
        lexemes[lexCount].lexeme[MAX_LEXEME_LENGTH - 1] = '\0';
        lexemes[lexCount].token = 0; // No valid token type

        // Store error message
        strncpy(lexemes[lexCount].errorMsg, errMsg, sizeof(lexemes[lexCount].errorMsg) - 1);
        lexemes[lexCount].errorMsg[sizeof(lexemes[lexCount].errorMsg) - 1] = '\0';
        lexemes[lexCount].isError = 1; // Invalid flag

        // Continue
        lexCount++;
    }
    else
    {
        fprintf(stderr, "Error: Lexeme buffer overflow.\n"); // Debugging - edge cases
        exit(1);
    }
}

// Function that tokenizes a PL/0 file
void lexicalAnalyzer(FILE *input)
{
    // Initialize variables
    char c, buffer[MAX_LEXEME_LENGTH];
    int pos, line = 1, column = 1;

    // Read entire file char by char
    while ((c = fgetc(input)) != EOF)
    {
        // Skip whitespaces
        if (isspace(c))
        {
            // For newlines
            if (c == '\n')
            {
                line++; // Move to next line
                column = 1; // Reset column
            }
            // For spaces and tabs
            else
            {
                column++; // Move to next col
            }

            // Process next char
            continue;
        }

        // Comment handling
        if (c == '/')
        {
            int next = fgetc(input);
            
            // Opening comment symbol /* found
            if (next == '*')
            {
                int prev = 0, closed = 0;

                // Now look for closing */ until found or end of file
                while ((c = fgetc(input)) != EOF)
                {
                    // Newline -> same as earlier
                    if (c == '\n')
                    {
                        line++; // Go to next line
                        column = 1; // Reset column
                    }
                    // Proper closing comment symbol */ found
                    if (prev == '*' && c == '/')
                    {
                        closed = 1; // Mark as closed
                        break;
                    }
                    prev = c;
                }

                // No valid closing comment symbol found
                if (!closed)
                {
                    // Treat as normal tokens
                    addToken(slashsym, "/");
                    addToken(multsym, "*");
                }
                continue;
            }
            // Just a slash
            else
            {
                ungetc(next, input); // Put back in input stream
                addToken(slashsym, "/"); // Add as token

                // Process next char
                continue;
            }
        }

        // Tokenize identifiers (vars, keywords)
        if (isalpha(c))
        {
            // Initialize position trackers
            pos = 0;
            buffer[pos++] = c;
            column++;
            
            // Read full identifier char by char in a loop
            while ((c = fgetc(input)) != EOF && (isalpha(c) || isdigit(c)))
            {
                // Check that it's a valid size (not exceding 11)
                if (pos < MAX_ID_LENGTH) 
                {
                    buffer[pos++] = c; // Store in buffer
                }
                // Too long
                else
                {
                    // Discard remaining chars
                    while (isalpha(c) || isdigit(c))
                    {
                        c = fgetc(input);
                        column++;
                    }
                    buffer[pos] = '\0'; // Terminate lexeme
                    addErrorToken(buffer, "Error: Ident length too long."); // Error: identifier is too long

                    // Exit loop
                    break;
                }
                column++;
            }
            buffer[pos] = '\0';

            // If not the end of file
            if (c != EOF)
                ungetc(c, input); // Push back to input stream

            // If lenght was valid
            if (pos <= MAX_ID_LENGTH)
                addToken(getToken(buffer), buffer); // Add as token

            // Process next char
            continue;
        }

        // Tokenize numbers
        if (isdigit(c))
        {
            // Initialize position trackers
            pos = 0;
            buffer[pos++] = c;
            column++;

            // Read all digits char by char in a loop
            while ((c = fgetc(input)) != EOF && isdigit(c))
            {
                // Check that it's a valid size (not exceding 5)
                if (pos < MAX_NUM_LENGTH)
                {
                    buffer[pos++] = c; // Store in buffer
                }
                else
                {
                    // Discard remaining digits
                    while (isdigit(c))
                    {
                        c = fgetc(input);
                        column++;
                    }
                    buffer[pos] = '\0'; // Terminate lexeme
                    addErrorToken(buffer, "Error: Number too long."); // Error: number is too long

                    // Exit loop
                    break;
                }
                column++;
            }
            buffer[pos] = '\0';
            
            // If not the end of file
            if (c != EOF)
                ungetc(c, input); // Push back to input stream
            
            // If length was valid
            if (pos <= MAX_NUM_LENGTH)
                addToken(numbersym, buffer); // Add as token

            // Process next char
            continue;
        }

        // Add appropriate token based on token
        switch (c)
        {
        case '+': // Plus
            addToken(plussym, "+");
            break;
        case '-': // Minus
            addToken(minussym, "-");
            break;
        case '*': // Multiplication
            addToken(multsym, "*");
            break;
        case '/': // Division
            addToken(slashsym, "/");
            break;
        case '(': // Left parenthesis
            addToken(lparentsym, "(");
            break;
        case ')': // Right parenthesis
            addToken(rparentsym, ")");
            break;
        case '=': // Equal
            addToken(eqlsym, "=");
            break;
        case ',': // Comma
            addToken(commasym, ",");
            break;
        case '.': // Period
            addToken(periodsym, ".");
            break;
        case ';': // Semicolon
            addToken(semicolonsym, ";");
            break;
        case ':': // Colon
            if ((c = fgetc(input)) == '=')
                addToken(becomessym, ":="); // Definition of
            else
                addErrorToken(":", "Error: Invalid symbol ':'"); // Not a valid symbol
            break;
        case '<': // Less than
            if ((c = fgetc(input)) == '=') // less than or equal to
                addToken(legsym, "<=");
            else if (c == '>') // not equal to
                addToken(negsym, "<>"); 
            else
                addToken(lessym, "<"); // regular less than
            break;
        case '>': // Greater than
            if ((c = fgetc(input)) == '=') // greater than or equal to
                addToken(gegsym, ">=");
            else
                addToken(gtrsym, ">"); // regular greater than
            break;

        // Invalid symbols
        default:
        {
            // Add error message
            char lex[2] = {c, '\0'};
            char err[64];
            snprintf(err, sizeof(err), "Error: Invalid symbol '%c'", c);

            // Add error token to list
            addErrorToken(lex, err); 

            // Process next char
            break;
        }
        }
    }
}

// Tiny Compiler functions -> New in parsercodegen.c

// Function that advances to the next token in the lexeme table
void getNextToken() 
{
    if (tokenIndex < lexCount) 
    {
        currentToken = lexemes[tokenIndex++].token; // Go to next index
    }
}

// Function that processes the block
void program() 
{
    // Save index for JMP instruction and emit 
    int jumpIdx = instructionCount;
    emit(JMP, 0, 0);

    // Process block after the JMP
    int blockStart = instructionCount;    
    getNextToken(); // Get 1st token
    block(); 

    // Check for period at end of program
    if (currentToken != periodsym) 
    {
        error("Expected '.' at end of program"); // Error in input file
    }

    // Emit HALT instruction and fix the JMP instruction with actual address
    emit(SYS, 0, 3);  
    instructions[jumpIdx].m = 10 + (blockStart * 3);
}

// Function that processes constants, variables, allocates memory, and then statements
void block() 
{
    // Process constants
    constDeclaration();

    // Process declarations
    int numVars = varDeclaration();

    // Allocate memory for variables
    emit(INC, 0, 3 + numVars); 

    // Process statements
    statement();
}

// Function that reads constant definitions and adds them to the symbol table
void constDeclaration() 
{
    // Process constants
    if (currentToken == constsym) 
    {
        do {
            // Get identifier
            getNextToken();
            if (currentToken != identsym) // Check that it's a valid identifier
            {
                error("Expected identifier after 'const'");
            }
            char name[MAX_LEXEME_LENGTH];
            strcpy(name, lexemes[tokenIndex - 1].lexeme);   // Save name

            // Get equal sign
            getNextToken();
            if (currentToken != eqlsym)
             {
                error("Expected '=' after identifier");
            }
            
            // Get number (value for the const)
            getNextToken();
            if (currentToken != numbersym) {
                error("Expected number after '='");
            }
            int value = atoi(lexemes[tokenIndex - 1].lexeme); // Convert to int

            // Add constant to symbol table
            strcpy(symbolTable[symbolCount].name, name);
            symbolTable[symbolCount].kind = 1;
            symbolTable[symbolCount].value = value;
            symbolTable[symbolCount].level = 0;
            symbolTable[symbolCount].address = 0;
            symbolTable[symbolCount].mark = 0;
            symbolCount++;
            
            getNextToken(); // Get next token
        } while (currentToken == commasym);
        
        // Check for semicolon at end of declaration
        if (currentToken != semicolonsym) 
        {
            error("Expected ';' after constant declaration");
        }

        // Get next token
        getNextToken();
    }
}

// Function that reads variable definitions and adds them to the symbol table
int varDeclaration() 
{
    // Process variables
    int numVars = 0;
    if (currentToken == varsym) 
    {
        do {
            // Get next variable (and update count)
            numVars++;
            getNextToken();
            if (currentToken != identsym) 
            {
                error("Expected identifier after 'var'");
            }
            char name[MAX_LEXEME_LENGTH];
            strcpy(name, lexemes[tokenIndex - 1].lexeme); // Save name
            
            // Add variable to symbol table
            strcpy(symbolTable[symbolCount].name, name);
            symbolTable[symbolCount].kind = 2;
            symbolTable[symbolCount].value = 0;
            symbolTable[symbolCount].level = 0;
            symbolTable[symbolCount].address = numVars + 2;
            symbolTable[symbolCount].mark = 0;
            symbolCount++;
            
            // Get next token
            getNextToken();
        } while (currentToken == commasym);
        
        // Check for semicolon at end of declaration
        if (currentToken != semicolonsym) {
            error("Expected ';' after variable declaration");
        }

        // Get next token
        getNextToken();
    }

    // Return number of variables
    return numVars;
}

// Function that processes a term, then processes additional terms with '+' or '-' operators.
void expression() 
{
    term();
    while (currentToken == plussym || currentToken == minussym) 
    {
        if (currentToken == plussym) {
            getNextToken();
            term();
            emit(ADD, 0, 0);  // Emit addition
        } else {
            getNextToken();
            term();
            emit(SUB, 0, 0);  // Emit subtraction
        }
    }
}

// Function that processes a factor, then processes additional factors with MUL, DIV or MOD
void term() 
{
    // Process factor
    factor();

    // Process MOD, DIV, and MOD operators
    while (currentToken == multsym || currentToken == slashsym || currentToken == modsym) 
    {
        // Case MUL
        if (currentToken == multsym) 
        {
            getNextToken();
            factor();
            emit(OPR, 0, 3);  // Emit MUL
        } 
        // Case DIV
        else if (currentToken == slashsym) 
        {
            getNextToken();
            factor();
            emit(DIV, 0, 0);  // Emit DIV
        } 
        // Case MOD
        else 
        {
            getNextToken();
            factor();
            emit(MOD, 0, 11);  // Emit MOD
        }
    }
}

// Function that processes identifiers, literals, or expressions enclosed in parentheses
void factor() 
{
    // Process identifier
    if (currentToken == identsym) 
    {
        // Check if identifier is in symbol table
        int symIdx = symbolTableCheck(lexemes[tokenIndex - 1].lexeme);
        if (symIdx == -1)
        {
            error("Undeclared identifier");
        }
        emit(LOD, 0, symbolTable[symIdx].address);

        // Get next token
        getNextToken();
    } 
    // Process numbers
    else if (currentToken == numbersym) 
    {
        int num = atoi(lexemes[tokenIndex - 1].lexeme);
        emit(LIT, 0, num);
        getNextToken();
    } 
    // Process expressions enclosed in parentheses
    else if (currentToken == lparentsym) 
    {
        // Process expression
        getNextToken();
        expression();

        // Check for closing parenthesis
        if (currentToken != rparentsym)
        {
            error("Expected closing parenthesis");
        }

        // Get next token
        getNextToken();
    } 
    // Return invalid
    else 
    {
        error("Invalid factor");
    }
}

// Function that processes odd condition or relational comparisons
void condition() 
{
    // Process odd condition
    if (currentToken == oddsym) 
    {
        getNextToken();
        expression();
        emit(ODD, 0, 0); // Emit ODD
    } 
    
    else 
    {
        // Process comparisons
        expression();

        // Equal to
        if (currentToken == eqlsym) 
        {
            getNextToken();
            expression();
            emit(EQL, 0, 0); // Emit EQL
        } 

        // Not equal to
        else if (currentToken == neqsym) 
        { 
            getNextToken();
            expression();
            emit(NEQ, 0, 0); // Emit NEQ
        } 

        // Less than
        else if (currentToken == lessym) 
        {
            getNextToken();
            expression();
            emit(LSS, 0, 0); // Emit LSS
        } 

        // Less than or equal to
        else if (currentToken == legsym) 
        { 
            getNextToken();
            expression();
            emit(LEQ, 0, 0); // Emit LEQ
        } 

        // Greater than
        else if (currentToken == gtrsym) 
        {
            getNextToken();
            expression();
            emit(GTR, 0, 0); // Emit GTR
        } 
        // Greater than or equal to
        else if (currentToken == gegsym) 
        { 
            getNextToken();
            expression();
            emit(GEQ, 0, 0); // Emit GEQ
        } 
        // Not a valid condition operator
        else {
            error("Invalid condition operator");
        }
    }
}

// Function that processes statements based on the current token
void statement() 
{
    // Process statement
    if (currentToken == identsym) 
    {
        //  Check if identifier is in symbol table
        int symIdx = symbolTableCheck(lexemes[tokenIndex - 1].lexeme);

        // Not found
        if (symIdx == -1)
        {
            error("Undeclared identifier");
        }

        // Not a variable
        if (symbolTable[symIdx].kind != 2)
        {
            error("Assignment to a non-variable");
        }
        getNextToken(); // Get next token

        // Check for assignment operator
        if (currentToken != becomessym)
        {
            error("Expected ':=' for assignment");
        }
        getNextToken(); // Get next token

        // Process expression
        expression();

        // Store result
        emit(STO, 0, symbolTable[symIdx].address);
    } 
    // Process compound statements
    else if (currentToken == beginsym) 
    {
        // Process statement block
        do {
            getNextToken();
            statement();
        } while (currentToken == semicolonsym);
        
        // Check for end of block
        if (currentToken != endsym)
        {
            error("Expected 'end'");
        }
        getNextToken(); // Get next token
    } 
    // Process conditional statements
    else if (currentToken == ifsym) 
    {
        // Process condition
        getNextToken();
        condition();
        int jpcIdx = instructionCount;
        emit(JPC, 0, 0);  // Jump if false
        
        // Check for 'then' in condition
        if (currentToken != thensym)
        {
            error("Expected 'then'");
        }
        getNextToken(); // Get next token

        // Process statement and fix JPC jump address
        statement();
        instructions[jpcIdx].m = instructionCount;
        
        // Check for 'fi' in condition
        if (currentToken != fisym)
        {
            error("Expected 'fi'");
        }
        getNextToken(); // Get next token
    } 
    // Process while loops
    else if (currentToken == whilesym) 
    {
        // Process condition
        getNextToken();
        int loopIdx = instructionCount;
        condition();
        
        // Check for 'do' in loop
        if (currentToken != dosym)
        {
            error("Expected 'do'");
        }
        getNextToken(); // Get next token
        
        // Save index for JPC instruction 
        int jpcIdx = instructionCount;
        emit(JPC, 0, 0);  // Jump if false

        // Process statement and fix JPC address
        statement();
        emit(JMP, 0, loopIdx);  // Back to loop condition
        instructions[jpcIdx].m = instructionCount;
    } 
    // Read statement
    else if (currentToken == readsym) 
    {
        // Process read statement
        getNextToken();

        // Check for identifier
        if (currentToken != identsym)
        {
            error("Expected an identifier after 'read'");
        }
        int symIdx = symbolTableCheck(lexemes[tokenIndex - 1].lexeme);

        // Not found
        if (symIdx == -1)
        {
            error("Undeclared identifier");
        }
        // Not a variable
        if (symbolTable[symIdx].kind != 2)
        {
            error("Cannot read into a non-variable");
        }
        getNextToken(); // Get next token

        // Emit read and store input
        emit(SYS, 0, 14);
        emit(STO, 0, symbolTable[symIdx].address);
    } 
    else if (currentToken == writesym) 
    {
        // Process write statement
        getNextToken();
        expression();
        emit(SYS, 0, 15); // Emit write
    }
}

// Function that appends an instruction to the instruction array
void emit(int op, int l, int m) 
{
    // Check that instruction array is not full 
    if (instructionCount >= MAX_INSTRUCTIONS) 
    {
        printf("Error: Instruction limit exceeded\n");
        exit(1);
    }

    // Add instruction to array
    instructions[instructionCount].op = op;
    instructions[instructionCount].l = l;
    instructions[instructionCount].m = m;
    instructionCount++;
}

// Function that searches the symbol table for an identifier
int symbolTableCheck(char *name) 
{
    // Loop through symbol table
    for (int i = 0; i < symbolCount; i++) 
    {
        // Check if identifier is in table
        if (strcmp(symbolTable[i].name, name) == 0) 
        {
            return i; // Success
        }
    }

    // Not found
    return -1;
}

// Helper function that prints the generated assembly code.
void printAssemblyCode() {

    // Print header
    printf("Assembly Code:\n\n");
    printf("Line OP L M\n");

    // Print all instructions
    for (int i = 0; i < instructionCount; i++) 
    {
        // printf("%d %s %d %d\n", i, instructions[i].op, instructions[i].l, instructions[i].m);
        printf("%d %s %d %d\n", i, getKindName(instructions[i].op), instructions[i].l, instructions[i].m);    
    }
}

// Helper function that prints the symbol table.
void printSymbolTable() 
{
    // Print table header
    printf("\n\nSymbol Table:\n");
    printf("Kind | Name      | Value | Level | Address | Mark\n");
    printf("-----------------------------------------------------\n");

    // Print all symbols
    for (int i = 0; i < symbolCount; i++) 
    {
        symbolTable[i].mark = 1; //
        printf("%4d | %-10s | %5d | %5d | %7d | %4d\n",
            symbolTable[i].kind, 
            symbolTable[i].name, 
            symbolTable[i].value, 
            symbolTable[i].level, 
            symbolTable[i].address, 
            symbolTable[i].mark);
    }
}

// Helper function that returns the name of the instruction kind (based on opcode)
const char* getKindName(int kind) 
{
    // Switch case to determine appropriate string
    switch (kind) {
        case 1: return "LIT";
        case 2: return "OPR";
        case 3: return "LOD";
        case 4: return "STO";
        case 6: return "INC";
        case 7: return "JMP";
        case 8: return "JPC";
        case 9: return "SYS";
        default: return "UNKNOWN";
    }
}

void printOutputFile(int flag)
{
    if(flag)
    {
        
    }
}

// Function that implements a PL/0 tiny compiler and generates P-code instructions
int main(int argc, char *argv[]) 
{
    // Check for input file
    if (argc < 2) {
        printf("Usage: %s <input_file>\n", argv[0]);
        return 1;
    }
    
    // Open input file
    FILE *input = fopen(argv[1], "r");
    if (!input) {
        perror("Error opening file"); // Error opening file
        return 1;
    }
    
    // Initialize instructions array
    for (int i = 0; i < MAX_INSTRUCTIONS; i++) 
    {
        instructions[i].op = 0;
        instructions[i].l = 0;
        instructions[i].m = 0;
    }
        
    // Do lexical analysis on input file
    lexicalAnalyzer(input);
    fclose(input);
    
    // Parse lexemes and generate P-code instructions 
    program();
    
    // Print the generated assembly code and symbol table
    printAssemblyCode();
    printSymbolTable();
    
    // Create the elf.txt file for the VM input
    FILE *output = fopen("elf.txt", "w");

    // Write the P-code instructions to the output file
    for (int i = 0; i < instructionCount; i++) 
    {
        fprintf(output, "%d %d %d\n", instructions[i].op, instructions[i].l, instructions[i].m);
    }
    fclose(output);
    
    // End program successfully
    return 0;
}