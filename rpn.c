#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

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

int pop(int *stack, int *stackc) {
    if (*stackc == 0) {
        return 0;
    }

    (*stackc)--;
    return stack[*stackc];
}

#define PUSH(x)             \
    do {                    \
        stack[stackc] = (x); \
        stackc++;           \
    } while (0)

#define POP() pop(stack, &stackc);

int main(void) {
    char input[128];
    Token tokens[256];
    memset(tokens, 0, sizeof(tokens));
    int pos = 0;

    printf("!\n");

    int stack[256];
    int stackc = 0;
    memset(stack, 0, sizeof(stack));

    while (printf("> "), fgets(input, sizeof(input), stdin) != NULL) {
        char *p = strtok(input, " \t\n");

        while (p != NULL) {
          if (pos >= sizeof(tokens) / sizeof(tokens[0])) {
            fprintf(stderr, "Too many tokens\n");
            return 1;
          }

          if (strcmp(p, "+") == 0) {
//              printf("add\n");
              tokens[pos] = (Token){ .type = TOK_ADD };
	      int right = POP();
	      int left = POP();
	      printf("%d\n", left + right);
              pos++;
          } else {
              char *endptr;
              long value;

              errno = 0;
              endptr = NULL;
              value = strtol(p, &endptr, 10);

              if (endptr != p && *endptr == '\0' && errno != ERANGE && value >= INT_MIN &&  value <= INT_MAX) {
  //                printf("digit\n");
                  tokens[pos] = (Token){
                      .type = TOK_INT,
                      .intg = (int)value
                  };
		  PUSH((int)value);
                  pos++;
              } else {
                  printf("else, %s\n", p);
		  tokens[pos] = (Token){
                        .type = TOK_UNK,
			.chr = (char)p
                  };
                  pos++;
              }
	  }

          p = strtok(NULL, " \t\n");
        }
    }

    return 0;
}
