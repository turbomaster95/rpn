#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define MAX_STACK 256

enum Tokens {
    TOK_UNK = 0,
    TOK_ADD,
    TOK_SUB,
    TOK_INT,
    TOK_CHR
};

typedef struct Token {
    int type;
    int intg;
    char chr;
} Token;

int pop(int *stack, int *stackc, int *result) {
    if (*stackc == 0) {
        return 0;
    }

    (*stackc)--;
    *result = stack[*stackc];

    return 1;
}

int push(int *stack, int *stackc, int value, int capacity) {
    if (*stackc >= capacity) {
        return 0;
    }

    stack[*stackc] = value;
    (*stackc)++;

    return 1;
}

void print_stack(const int *stack, int stackc, int capacity) {
    printf("[");

    for (int i = 0; i < stackc; i++) {
        if (i > 0) {
            printf("|");
        }

        printf("%d", stack[i]);
    }

    if (stackc < capacity) {
        if (stackc > 0) {
            printf("|");
        }

        printf("0|...");
    }

    printf("]\n");
}

int main(void) {
    char input[128];
    Token tokens[256];
    int pos = 0;
    int stack[MAX_STACK];
    int stackc = 0;

    memset(tokens, 0, sizeof(tokens));
    memset(stack, 0, sizeof(stack));

    printf("!\n");

    while (printf("> "), fgets(input, sizeof(input), stdin) != NULL) {
        char *p = strtok(input, " \t\n");

        while (p != NULL) {
            if (pos >= (int)(sizeof(tokens) / sizeof(tokens[0]))) {
                fprintf(stderr, "!!! Too many tokens\n");
                return 1;
            }

            if (strcmp(p, ".s") == 0) {
                print_stack(stack, stackc, MAX_STACK);
            } else if (strcmp(p, "+") == 0) {
                int right;
                int left;

                tokens[pos] = (Token){ .type = TOK_ADD };
                pos++;

                if (stackc < 2) {
                    fprintf(stderr, "?? ops -> +\n");
                } else {
                    pop(stack, &stackc, &right);
                    pop(stack, &stackc, &left);

                    if (!push(stack, &stackc, left + right,
                               MAX_STACK)) {
                        fprintf(stderr, "[||///\n");
                        return 1;
                    }

                    printf("%d\n", stack[stackc - 1]);
                }
            } else if (strcmp(p, "dup") == 0) {
                tokens[pos] = (Token){ .type = TOK_UNK };
                pos++;

                if (stackc == 0) {
                    fprintf(stderr, "[] --/--> 2[] ==> [0|0|0|0|0|0]\n");
                } else if (stackc >= (int)MAX_STACK) {
                    fprintf(stderr, "[||///\n");
                    return 1;
                } else {
                    push(stack, &stackc, stack[stackc - 1],
                         MAX_STACK);
                }
            } else if (strcmp(p, "#") == 0) {
            		int temp;
            		if (stackc == 0) {
            		    fprintf(stderr, "[0|0|0|0] <--/-- ÷\n");
            		} else if (stackc >= (int)MAX_STACK) {
            		    fprintf(stderr, "[||///\n");
            		} else {
            		    pop(stack, &stackc, &temp);
            		}
            } else if (strcmp(p, ".e") == 0) {
                return EXIT_SUCCESS;
      	    } else if (strcmp(p, "-") == 0) {
                int right, left;
	              if (stackc < 2) {
	                  fprintf(stderr, "?? ops -> -\n");
      	        } else {
	                  pop(stack, &stackc, &right);
                    pop(stack, &stackc, &left);
	                  push(stack, &stackc, left - right, MAX_STACK);
	                  printf("%d\n", stack[stackc - 1]);
	              }
	          } else if (strcmp(p, "~") == 0) {
	              if (stackc < 2) {
                    fprintf(stderr, "?? ops -> ~\n");
	              } else {
	                  int top = stack[stackc - 1];
	                  stack[stackc - 1] = stack[stackc - 2];
	                                                              stack[stackc - 2] = top;
	                                                                  }
	                                                                  
	                                  }
	                  }
	          }
            } else {
                char *endptr;
                long value;

                errno = 0;
                value = strtol(p, &endptr, 10);

                if (
                    endptr != p &&
                    *endptr == '\0' &&
                    errno != ERANGE &&
                    value >= INT_MIN &&
                    value <= INT_MAX
                ) {
                    tokens[pos] = (Token){
                        .type = TOK_INT,
                        .intg = (int)value
                    };
                    pos++;

                    if (!push(stack, &stackc, (int)value,
                               MAX_STACK)) {
                        fprintf(stderr, "[||///\n");
                        return 1;
                    }
                } else {
                    printf("???, %s\n", p);

                    tokens[pos] = (Token){
                        .type = TOK_UNK,
                        .chr = p[0]
                    };
                    pos++;
                }
            }

            p = strtok(NULL, " \t\n");
        }
    }

    return EXIT_SUCCESS;
}
