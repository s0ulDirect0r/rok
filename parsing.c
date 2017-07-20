#include <stdio.h>
#include <stdlib.h>
#include "mpc.h"

/* If we are compiling on Windows compile these functions */
#ifdef _WIN32
#include <string.h>

static char buffer[2048];

/* Fake readline function */
char* readline(char* prompt) {
  fputs(prompt, stdout);
  fgets(buffer, 2048, stdin);
  char* cpy = malloc(strlen(buffer)+1);
  strcpy(cpy, buffer);
  cpy[strlen(cpy) - 1] = '\0';
  return cpy;
}

/* Fake add_history function */
void add_history(char* unused) {}

/* Otherwise include the ditline headers */
#else
#include <editline/readline.h>
#endif

int main(int argc, char** argv) {
  /* Create some parsers */
  mpc_parser_t* Number   = mpc_new("number");
  mpc_parser_t* Operator = mpc_new("operator");
  mpc_parser_t* Expr     = mpc_new("expr");
  mpc_parser_t* Rok      = mpc_new("rok");

  /* Define them with the following language */
  mpca_lang(MPCA_LANG_DEFAULT,
    "                                                   \
      number   : /-?[0-9]+/ ;                           \
      operator :  '+' | '-' | '*' | '/' ;               \
      expr     : <number> | '(' <operator> <expr> ')' ; \
      rok      : /^/ <operator> <expr>+ /$/  ;          \
    ",
  Number, Operator, Expr, Rok);

  /* Undefine and Delete our Parsers */
  mpc_cleanup(4, Number, Operator, Expr, Rok);
  
  /* Print version and exit information */
  puts("Rok Version 0.0.0.0.1");
  puts("Let's Rok, Motherfuckers!!");
  puts("Press Ctrl-C to Exit\n");

  /* In a never ending loop */
  while (1) {
    /* Output our prompt and get input */
    char* input = readline("rok> ");

    /* Add input to history */
    add_history(input);

    /* Echo input back to user */
    printf("Get you some %s\n", input);

    /* Free retrieved input */
    free(input);
  }

  return 0;
}
