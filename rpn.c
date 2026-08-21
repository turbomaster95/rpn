#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_STACK 256
#define MAX_MEM 1024
#define MAX_VARS 32
#define MAX_WORDS 32
#define MAX_WORD_TOKENS 64
#define MAX_LINE_TOKENS 128

typedef struct {
    char name[32];
    int address;
} Variable;

typedef struct {
    char name[32];
    char tokens[MAX_WORD_TOKENS][32];
    int token_count;
} Word;

typedef struct {
    int start_token_idx;
    long long index;
    long long limit;
} LoopFrame;

LoopFrame loop_stack[32];
int loop_sp = 0;

Word dictionary[MAX_WORDS];
int word_count = 0;
int is_defining = 0;
Word *current_word = NULL;

int pop(long long *stack, int *stackc, long long *result) {
    if (*stackc == 0) return 0;
    (*stackc)--;
    *result = stack[*stackc];
    return 1;
}

int push(long long *stack, int *stackc, long long value, int capacity) {
    if (*stackc >= capacity) return 0;
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

void execute_tokens(char tokens[][32], int num_tokens, long long *stack, int *stackc, int *memory, int *mem_ptr, Variable *vars, int *var_count) {
    int ip = 0;

    while (ip < num_tokens) {
        char *p = tokens[ip];

        int word_found = 0;
        for (int i = 0; i < word_count; i++) {
            if (strcmp(p, dictionary[i].name) == 0) {
                execute_tokens(dictionary[i].tokens, dictionary[i].token_count, stack, stackc, memory, mem_ptr, vars, var_count);
                word_found = 1;
                break;
            }
        }
        if (word_found) {
            ip++;
            continue;
        }

        if (strcmp(p, "do") == 0) {
            long long index, limit;
            if (pop(stack, stackc, &index) && pop(stack, stackc, &limit)) {
                if (loop_sp < 32) {
                    loop_stack[loop_sp].start_token_idx = ip; // Store 'do' pos
                    loop_stack[loop_sp].index = index;
                    loop_stack[loop_sp].limit = limit;
                    loop_sp++;
                } else {
                    fprintf(stderr, "[l||//\n");
                }
            } else {
                fprintf(stderr, "?? ops -> do\n");
            }
        } else if (strcmp(p, "loop") == 0) {
            if (loop_sp > 0) {
                LoopFrame *frame = &loop_stack[loop_sp - 1];
                frame->index++;
                if (frame->index < frame->limit) {
                    ip = frame->start_token_idx;
                } else {
                    loop_sp--;
                }
            } else {
                fprintf(stderr, "?loop\n");
            }
        } else if (strcmp(p, "i") == 0) {
            if (loop_sp > 0) {
                push(stack, stackc, loop_stack[loop_sp - 1].index, MAX_STACK);
            } else {
                fprintf(stderr, "i --/--> loop\n");
            }
        } else if (strcmp(p, "?") == 0) {
            print_stack(stack, *stackc, MAX_STACK);
        } else if (strcmp(p, "+") == 0) {
            long long right, left;
            if (*stackc < 2) {
                fprintf(stderr, "?? ops -> +\n");
            } else {
                pop(stack, stackc, &right);
                pop(stack, stackc, &left);
                push(stack, stackc, left + right, MAX_STACK);
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
            }
        } else if (strcmp(p, "*") == 0) {
            long long right, left;
            if (*stackc < 2) {
                fprintf(stderr, "?? ops -> *\n");
            } else {
                pop(stack, stackc, &right);
                pop(stack, stackc, &left);
                push(stack, stackc, left * right, MAX_STACK);
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
        } else if (strcmp(p, "over") == 0) {
            if (*stackc < 2) {
                fprintf(stderr, "?? ops -> over\n");
            } else {
                push(stack, stackc, stack[*stackc - 2], MAX_STACK);
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
            if (ip + 1 < num_tokens) {
                ip++;
                char *var_name = tokens[ip];
                vars[*var_count].address = *mem_ptr;
                strcpy(vars[*var_count].name, var_name);
                memory[*mem_ptr] = 0;
                (*mem_ptr)++;
                (*var_count)++;
            } else {
                fprintf(stderr, "? name -> v\n");
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
        } else if (strcmp(p, ".") == 0) {
            long long val;
            if (pop(stack, stackc, &val)) {
                printf("%lld\n", val);
            } else {
                fprintf(stderr, "? ops -> .\n");
            }
        } else if (strcmp(p, ",") == 0) {
            long long val;
            if (pop(stack, stackc, &val)) {
                putchar((char)val);
                printf("\n");
            } else {
                fprintf(stderr, "? ops -> .\n");
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

        ip++;
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
        char tokens[MAX_LINE_TOKENS][32];
        int num_tokens = 0;

        char *p = strtok(input, " \t\n");
        while (p != NULL && num_tokens < MAX_LINE_TOKENS) {
            strcpy(tokens[num_tokens++], p);
            p = strtok(NULL, " \t\n");
        }

        int line_ip = 0;
        while (line_ip < num_tokens) {
            char *tok = tokens[line_ip];

            if (is_defining) {
                if (strcmp(tok, "}") == 0) {
                    is_defining = 0;
                    current_word = NULL;
                } else if (current_word->name[0] == '\0') {
                    strcpy(current_word->name, tok);
                } else {
                    if (current_word->token_count < MAX_WORD_TOKENS) {
                        strcpy(current_word->tokens[current_word->token_count], tok);
                        current_word->token_count++;
                    }
                }
                line_ip++;
            } else {
                if (strcmp(tok, "{") == 0) {
                    if (word_count < MAX_WORDS) {
                        is_defining = 1;
                        current_word = &dictionary[word_count];
                        memset(current_word, 0, sizeof(Word));
                        word_count++;
                    } else {
                        fprintf(stderr, "!!! Dictionary full\n");
                    }
                    line_ip++;
                } else {
                    char exec_tokens[MAX_LINE_TOKENS][32];
                    int exec_count = 0;
                    while (line_ip < num_tokens) {
                        strcpy(exec_tokens[exec_count++], tokens[line_ip++]);
                    }
                    execute_tokens(exec_tokens, exec_count, stack, &stackc, memory, &mem_ptr, vars, &var_count);
                }
            }
        }

        printf("> ");
        fflush(stdout);
    }

    return EXIT_SUCCESS;
}