#include <stdio.h>

#include "tokenizer.h"

int main() {
    Tokenizer *t = Tokenizer_Create();
    if(Tokenizer_LoadFile(t, "../test/example.c")) {
        return 1;
    }

    printf("Buffer:\n%s\n\n", t->buffer);

    TokenStream *stream = Tokenizer_Emit(t);
    if(stream == NULL) {
        fprintf(stderr, "Failed to create TokenStream\n");
    }

    printf("Token Stream:\n");
    TokenStreamNode *node = stream;
    while(node != NULL && node->token->type != Token_EOF) {
        switch(node->token->type) {
            default:
                printf("%d: %.*s\n", node->token->type, node->token->size, node->token->bufferOffset);
                break;
        }
        node = node->next;
    }

    // TokenStream_Destroy(stream);
    Tokenizer_Destroy(t);

    return 0;
}
