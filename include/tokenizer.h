//
// Created by rombus on 8/15/21.
//

#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/stat.h>

typedef struct Token_Position {
    unsigned int line;
    unsigned int column;
} Token_Position;

typedef enum {
    Token_EOF = '\0',
    Token_LParen = '(',
    Token_RParen = ')',
    Token_LCurly = '{',
    Token_RCurly = '}',
    Token_LBrace = '[',
    Token_RBrace = ']',
    Token_DoubleQuote = '"',
    Token_SingleQuote = '\'',
    Token_Colon = ':',
    Token_Semicolon = ';',
    Token_Equals = '=',
    Token_Asterisk = '*',

    Token_Unknown,
    Token_String,
    Token_Number,
    Token_Identifier,
} Token_Type;

typedef struct Token {
    Token_Type type;
    Token_Position position;
    char *bufferOffset;
    int size;
} Token;

//
// TODO: Confirm why this should work
//
// I'm using a doubly linked list to lookbehind and lookahead with tokens.
// This could be a bad idea but I'll still try anyway.
//
typedef struct TokenStreamNode {
    Token *token;
    struct TokenStreamNode *prev;
    struct TokenStreamNode *next;
} TokenStreamNode;

// This is a helper shortcut to make the code a little more clear from a user perspective.
#define TokenStream TokenStreamNode

typedef struct Tokenizer {
    uintptr_t offset;
    char *buffer;
} Tokenizer;

TokenStreamNode *TokenStreamNode_Create(Token *token) {
    TokenStreamNode *node = malloc(sizeof(TokenStreamNode));
    if (node == NULL) {
        fprintf(stderr, "Failed to allocate a new TokenStreamNode\n");
        return NULL;
    }

    node->token = token;
    node->prev = NULL;
    node->next = NULL;

    return node;
}

TokenStream *TokenStream_Create(Token *token) {
    return TokenStreamNode_Create(token);
}

void TokenStream_Prepend(TokenStreamNode *stream, Token *token) {
    TokenStreamNode *newNode = TokenStreamNode_Create(token);
    if (stream == NULL) {
        stream = newNode;
        return;
    }
    stream->prev = newNode;
    newNode->next = stream;
    stream = newNode;
}

void TokenStream_Append(TokenStreamNode *stream, Token *token) {
    struct TokenStreamNode *temp = stream;
    struct TokenStreamNode *newNode = TokenStreamNode_Create(token);
    if (stream == NULL) {
        stream = newNode;
        return;
    }
    while (temp->next != NULL) temp = temp->next; // Go To last Node
    temp->next = newNode;
    newNode->prev = temp;
}

// TODO: Deallocate the doubly linked list, not important now
// This does not work lol
void TokenStream_Destroy(TokenStreamNode *stream) {
    TokenStreamNode *node = stream;

    while (node != NULL) {
        TokenStreamNode *temp = node;
        free(node->token);

        free(node);
        node = temp->next;
    }

    free(stream);
}

Tokenizer *Tokenizer_Create() {
    Tokenizer *tokenizer = malloc(sizeof(Tokenizer));
    if (tokenizer == NULL) {
        fprintf(stderr, "Failed to allocate Tokenizer\n");
        return NULL;
    }

    tokenizer->buffer = NULL;
    tokenizer->offset = 0;

    return tokenizer;
}

void Tokenizer_Destroy(Tokenizer *tokenizer) {
    free(tokenizer->buffer);
    tokenizer->offset = 0;
    free(tokenizer);
    tokenizer = NULL;
}

int Tokenizer_LoadFile(Tokenizer *tokenizer, const char *filePath) {
    char *buffer = NULL;

    // Open a file
    FILE *file = fopen(filePath, "r");
    if (!file) {
        fprintf(stderr, "Failed to open file %s: %s\n", filePath, strerror(errno));
        return errno;
    }

    // Check if the path is regular file
    struct stat fileInfo;
    int fileType = stat(filePath, &fileInfo);
    if (fileType != 0 || !S_ISREG(fileInfo.st_mode)) {
        fprintf(stderr, "Specified path must be a file %s\n", filePath);
        return 1;
    }

    // Get the file size
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Allocate, fill the buffer and null terminate
    buffer = malloc(fileSize + 1);
    fread(buffer, fileSize, 1, file);
    buffer[fileSize] = '\0';

    // Close the open file
    fclose(file);

    // Pass our buffer on
    tokenizer->buffer = buffer;

    return 0;
}

TokenStream *Tokenizer_Emit(Tokenizer *tokenizer) {
    TokenStream *stream = NULL;

    bool emitting = true;
    while (emitting) {
        // TODO: skips things we don't care about
        // TODO: We might want to emit comments as well?

        // Create a new unknown token
        Token *token = malloc(sizeof(Token));
        if (token == NULL) {
            return NULL;
        }

        // Default token state
        token->type = Token_Unknown;
        token->size = 1;
        token->bufferOffset = &tokenizer->buffer[tokenizer->offset];

        if(stream == NULL) {
            stream = TokenStream_Create(token);
        } else {
            TokenStream_Append(stream, token);
        }

        char currentChar = tokenizer->buffer[tokenizer->offset];
        switch (currentChar) {
            case '\0': // EOF
                token->type = Token_EOF;
                emitting = false;
                break;

            case '(': // Terminals
            case ')':
            case '{':
            case '}':
            case '[':
            case ']':
            case '"':
            case '\'':
            case ':':
            case ';':
            case '=':
            case '*':
                token->type = (unsigned char) currentChar;
                tokenizer->offset++;
                break;

            default: // Unknown token
                tokenizer->offset++;
                break;
        }
    }

    return stream;
}