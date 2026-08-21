#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFL 64

int initlex(size_t srclen, char **lexarray) {
    if (srclen == 0 || lexarray == NULL) {
        return -1;
    }

    for (size_t i = 0; i < srclen; i++) {
        lexarray[i] = malloc(DEFL);

        if (lexarray[i] == NULL) {
            for (size_t j = 0; j < i; j++) {
                free(lexarray[j]);
            }
            return -1;
        }
    }

    return 0;
}

int lex(const char *src) {
    size_t length = strlen(src);

    char *tokens[length];

    if (initlex(length, tokens) != 0) {
        return 1;
    }

    for (size_t i = 0; i < length; i++) {
        tokens[i][0] = src[i];
        tokens[i][1] = '\0';
    }

    for (size_t i = 0; i < length; i++) {
        printf("%s\n", tokens[i]);
        free(tokens[i]);
    }

    return 0;
}

int main() {
    lex((const char*)"(message 'hello')");
}
