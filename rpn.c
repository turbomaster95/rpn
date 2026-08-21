#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define MAX_STACK 256
#define MAX_MEM 1024
#define MAX_VARS 32
#define MAX_WORDS 32
#define MAX_WORD_TOKENS 64

typedef struct {
    char name[32];
    int address;
} Variable;

typedef struct {
    char name[32];
    char tokens[MAX_WORD_TOKENS][32];
    int token_count;
} Word;

Word dictionary[MAX_WORDS];
int word_count = 0;
int is_defining = 0;
Word *current_word = NULL;

int pop(long long *stack, int *stackc, long long *result) {
    if (*stackc == 0) {
        return 0;
    }
    (*stackc)--;
    *result = stack[*stackc];
    return 1;
}

int push(long long *stack, int *stackc, long long value, int capacity) {
    if (*stackc >= capacity) {
        return 0;
    }
    stack[*stackc] = value;
    (*stackc)++;
    return 1;
}

void print_stack(const long long *stack, int stackc, int capacity) {
    printf("[");
    for (int i = 0; i < stackc; i++) {
        if (i > 0) printf("|");
        printf("%lld", stack[i]);
    }
    if (stackc < capacity) {
        if (stackc > 0) printf("|");
        printf("0|...");
    }
    printf("]\n");
}

void execute_token(char *p, long long *stack, int *stackc, int *memory, int *mem_ptr, Variable *vars, int *var_count) {
    for (int i = 0; i < word_count; i++) {
        if (strcmp(p, dictionary[i].name) == 0) {
            for (int j = 0; j < dictionary[i].token_count; j++) {
                execute_token(dictionary[i].tokens[j], stack, stackc, memory, mem_ptr, vars, var_count);
            }
            return;
        }
    }

    if (strcmp(p, "?") == 0) {
        print_stack(stack, *stackc, MAX_STACK);
    } else if (strcmp(p, "+") == 0) {
        long long right, left;
        if (*stackc < 2) {
            fprintf(stderr, "?? ops -> +\n");
        } else {
            pop(stack, stackc, &right);
            pop(stack, stackc, &left);
            push(stack, stackc, left + right, MAX_STACK);
            printf("%lld\n", stack[*stackc - 1]);
        }
    } else if (strcmp(p, "dup") == 0) {
        if (*stackc == 0) {
            fprintf(stderr, "[] --/--> 2[] ==> [0|0|0|0|0|0]\n");
        } else if (*stackc >= MAX_STACK) {
            fprintf(stderr, "[||///\n");
        } else {
            push(stack, stackc, stack[*stackc - 1], MAX_STACK);
        }
    } else if (strcmp(p, "$") == 0) {
        long long temp;
        if (*stackc == 0) {
            fprintf(stderr, "[0|0|0|0] <--/-- ÷\n");
        } else {
            pop(stack, stackc, &temp);
        }
    } else if (strcmp(p, ".e") == 0) {
        exit(EXIT_SUCCESS);
    } else if (strcmp(p, "-") == 0) {
        long long right, left;
        if (*stackc < 2) {
            fprintf(stderr, "?? ops -> -\n");
        } else {
            pop(stack, stackc, &right);
            pop(stack, stackc, &left);
            push(stack, stackc, left - right, MAX_STACK);
            printf("%lld\n", stack[*stackc - 1]);
        }
    } else if (strcmp(p, "*") == 0) {
        long long right, left;
        if (*stackc < 2) { 
            fprintf(stderr, "?? ops -> *\n");
        } else {
            pop(stack, stackc, &right);
            pop(stack, stackc, &left);
            push(stack, stackc, left * right, MAX_STACK);
            printf("%lld\n", stack[*stackc - 1]);
        }
    } else if (strcmp(p, "~") == 0) {
        if (*stackc < 2) {
            fprintf(stderr, "?? ops -> ~\n");
        } else {
            long long top, second;
            pop(stack, stackc, &top);
            pop(stack, stackc, &second);
            push(stack, stackc, top, MAX_STACK);
            push(stack, stackc, second, MAX_STACK);
        }
    } else if (strcmp(p, "%") == 0) {
        if (*stackc < 2) {
            fprintf(stderr, "?? ops -> %%\n");
        } else {
            long long top, second;
            pop(stack, stackc, &top);
            pop(stack, stackc, &second);
            push(stack, stackc, second, MAX_STACK);
            push(stack, stackc, top, MAX_STACK);
            push(stack, stackc, second, MAX_STACK);
        }
    } else if (strcmp(p, "@") == 0) {
        if (*stackc < 3) {
            fprintf(stderr, "??? ops -> @\n");
        } else {
            long long top, second, third;
            pop(stack, stackc, &top);
            pop(stack, stackc, &second);
            pop(stack, stackc, &third);
            push(stack, stackc, second, MAX_STACK);
            push(stack, stackc, top, MAX_STACK);
            push(stack, stackc, third, MAX_STACK);
        }
    } else if (strcmp(p, "v") == 0) {
        char *var_name = strtok(NULL, " \t\n");
        if (!var_name) {
            fprintf(stderr, "? name -> v\n");
        } else {
            vars[*var_count].address = *mem_ptr;
            strcpy(vars[*var_count].name, var_name);
            memory[*mem_ptr] = 0;
            (*mem_ptr)++;
            (*var_count)++;
        }
    } else if (strcmp(p, "^") == 0) {
        if (*stackc < 1) {
            fprintf(stderr, "? addr -> ^\n");
        } else {
            long long addr;
            pop(stack, stackc, &addr);
            if (addr < 0 || addr >= *mem_ptr) {
                fprintf(stderr, "[a] --/--> mem\n");
            } else {
                push(stack, stackc, memory[addr], MAX_STACK);
            }
        }
    } else if (strcmp(p, "=") == 0) {
        if (*stackc < 2) {
            fprintf(stderr, "?? a,v -> =\n");
        } else {
            long long addr, val;
            pop(stack, stackc, &addr);
            pop(stack, stackc, &val);
            if (addr < 0 || addr >= *mem_ptr) {
                fprintf(stderr, "[a] --/--> mem\n");
            } else {
                memory[addr] = (int)val;
            }
        }
    } else {
        int found_var = 0;
        for (int i = 0; i < *var_count; i++) {
            if (strcmp(p, vars[i].name) == 0) {
                push(stack, stackc, vars[i].address, MAX_STACK);
                found_var = 1;
                break;
            }
        }

        if (!found_var) {
            char *endptr;
            long long value;
            errno = 0;
            value = strtoll(p, &endptr, 10);
            if (endptr != p && *endptr == '\0' && errno != ERANGE) {
                push(stack, stackc, value, MAX_STACK);
            } else {
                printf("???, %s\n", p);
            }
        }
    }
}

int main(void) {
    char input[128];
    long long stack[MAX_STACK];
    int stackc = 0;
    int memory[MAX_MEM];
    int mem_ptr = 0;
    Variable vars[MAX_VARS];
    int var_count = 0;

    memset(stack, 0, sizeof(stack));
    memset(memory, 0, sizeof(memory));
    memset(vars, 0, sizeof(vars));
    memset(dictionary, 0, sizeof(dictionary));

    printf("!\n> ");
    fflush(stdout);

    while (fgets(input, sizeof(input), stdin) != NULL) {
        char *p = strtok(input, " \t\n");

        while (p != NULL) {
            if (is_defining) {
                if (strcmp(p, "}") == 0) {
                    is_defining = 0;
                    current_word = NULL;
                } else if (current_word->name[0] == '\0') {
                    strcpy(current_word->name, p);
                } else {
                    if (current_word->token_count < MAX_WORD_TOKENS) {
                        strcpy(current_word->tokens[current_word->token_count], p);
                        current_word->token_count++;
                    }
                }
            } else {
                if (strcmp(p, "{") == 0) {
                    if (word_count < MAX_WORDS) {
                        is_defining = 1;
                        current_word = &dictionary[word_count];
                        memset(current_word, 0, sizeof(Word));
                        word_count++;
                    } else {
                        fprintf(stderr, "!!! Dictionary full\n");
                    }
                } else {
                    execute_token(p, stack, &stackc, memory, &mem_ptr, vars, &var_count);
                }
            }
            p = strtok(NULL, " \t\n");
        }
        printf("> ");
        fflush(stdout);
    }

    return EXIT_SUCCESS;
}