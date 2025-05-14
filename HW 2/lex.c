/*
 * COP 3402 Systems Software
 * Homework 2: Lexical Analyzer
 * Author: Esteban Ramirez
 * Date: Feb 18th, 2025
 * Description: Reads a PL/0 source file, tokenizes the input, and outputs a lexeme table and token list
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LEXEMES 10000
#define MAX_ID_LENGTH 11 
#define MAX_NUM_LENGTH 5   
#define MAX_LEXEME_LENGTH 64

// Definition for token types
typedef enum
{
    skipsym = 1, identsym, numbersym, plussym, minussym, multsym, slashsym, fisym, eqlsym, neqsym, lessym, leqsym, gtrsym, geqsym, lparentsym,
    rparentsym, commasym, semicolonsym, periodsym, becomessym, beginsym, endsym, ifsym, thensym, whilesym, dosym, callsym, constsym, varsym, 
    procsym,writesym,readsym, elsesym
} tokenType;

// Data type for reserved words
typedef struct
{
    char *name;
    tokenType token;
} ReservedWord; 

// List of reserved keywords
ReservedWord reservedWords[] = 
{
    {"const", constsym}, {"var", varsym}, {"procedure", procsym}, {"call", callsym}, {"begin", beginsym}, {"end", endsym}, {"if", ifsym}, {"fi", fisym}, {"then", thensym}, {"else", elsesym}, {"while", whilesym}, {"do", dosym}, {"read", readsym}, {"write", writesym}
};
int numReservedWords = sizeof(reservedWords) / sizeof(ReservedWord);

// Data type for lexemes
typedef struct
{
    char lexeme[MAX_LEXEME_LENGTH];
    int token;
    char errorMsg[MAX_LEXEME_LENGTH];
    int isError;
} LexemeEntry;
LexemeEntry lexemes[MAX_LEXEMES];
int lexCount = 0;

// Function that finds the token type of the given lexeme
tokenType getToken(char *lexeme)
{
    for (int i = 0; i < numReservedWords; i++)
    {
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
                addToken(leqsym, "<=");
            else if (c == '>') // not equal to
                addToken(neqsym, "<>"); 
            else
                addToken(lessym, "<"); // regular less than
            break;
        case '>': // Greater than
            if ((c = fgetc(input)) == '=') // greater than or equal to
                addToken(geqsym, ">=");
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

// Funcion that reads a PL/0 source file, tokenizes the input, and outputs a lexeme table + token list
int main(int argc, char *argv[])
{
    // No input file provided
    if (argc < 2)
    {
        printf("Usage: %s <input_file>\n", argv[0]);
        return 1;
    }

    // Error opening input file
    FILE *input = fopen(argv[1], "r");
    if (!input)
    {
        perror("Error opening file");
        return 1;
    }

    // Print Source Program
    printf("Source Program:\n");
    char line[256];
    while (fgets(line, sizeof(line), input))
    {
        printf("%s", line);
    }
    rewind(input); // Reset fpointer

    // Analyze input and tokenize it
    lexicalAnalyzer(input);

    // Close input file
    fclose(input);

    // Print lexeme table with lexemes and token types
    printf("\n\nLexeme Table:\n\n");
    printf("lexeme\t\ttoken type\n");
    for (int i = 0; i < lexCount; i++)
    {
        if (lexemes[i].isError)
        {
            printf("%-10s\t%s\n", lexemes[i].lexeme, lexemes[i].errorMsg); // Error message
        }
        else
        {
            printf("%-10s\t%d\n", lexemes[i].lexeme, lexemes[i].token); // Token
        }
    }

    // Print Token List
    printf("\nToken List:\n");
    for (int i = 0; i < lexCount; i++)
    {
        // Only print valid tokens
        if (!lexemes[i].isError)
        {
            printf("%d", lexemes[i].token); // Token number

            // For identifiers and nymbers print associated lexeme as well
            if (lexemes[i].token == identsym || lexemes[i].token == numbersym)
            {
                printf(" %s", lexemes[i].lexeme); 
            }
            // Separate each token for formatting
            printf(" ");                         
        }
    }
    
    // End program successfully
    printf("\n");
    return 0;
}