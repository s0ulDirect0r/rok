#include <stdio.h>
#include <stdlib.h>
#include <math.h>
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

/* Declare new lval struct */
typedef struct {
  int type;
  long num;
  int err;
} lval;

/* Declare Enumerations for lval types */
enum { LVAL_NUM, LVAL_ERR };

/* Enums for Errors */
enum { LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM };

/** Lval functions **/
lval lval_num(long x) {
  lval v;
  v.type = LVAL_NUM;
  v.num = x;
  return v;
}

lval lval_err(int x) {
  lval v;
  v.type = LVAL_ERR;
  v.err = x;
  return v;
}

/* Use operator string to see which operation to perform */
long eval_op(long value1, char* op, long value2) {
  if(strcmp(op, "+") == 0) { return value1 + value2; }
  if(strcmp(op, "-") == 0) { return value1 - value2; }
  if(strcmp(op, "*") == 0) { return value1 * value2; }
  if(strcmp(op, "/") == 0) { return value1 / value2; }
  if(strcmp(op, "%") == 0) { return value1 % value2; }
  if(strcmp(op, "^") == 0) { return (long)pow(value1, value2); }
  return 0;
}

long eval(mpc_ast_t* t) {
  /* if tagged as number return it directly. */
  if (strstr(t->tag, "number")) {
    return atoi(t->contents);
  }

  /* the operator is always second child */
  char* op = t->children[1]->contents;

  /* We store the third child in x */
  long x = eval(t->children[2]);

  /* Iterate the remaining children and combining */
  int i = 3;
  while(strstr(t->children[i]->tag, "expr")) {
    x = eval_op(x, op, eval(t->children[i]));
    i++;
  }

  return x;
}

int main(int argc, char** argv) {
  /* Create some parsers */
  mpc_parser_t* Number   = mpc_new("number");
  mpc_parser_t* Operator = mpc_new("operator");
  mpc_parser_t* Expr     = mpc_new("expr");
  mpc_parser_t* Rok      = mpc_new("rok");

  /* Define them with the following Language */
  mpca_lang(MPCA_LANG_DEFAULT,
    " number   : /[+-]?([0-9]*[.])?[0-9]+/ ;             "
    " operator : '+' | '-' | '*' | '/' | '%' | '^' ;     "
    " expr     : <number> | '(' <operator> <expr>+ ')' ; "
    " rok      : /^/ <operator> <expr>+ /$/ ;            ",
    Number, Operator, Expr, Rok);

  /* Print version and exit information */
  puts("Rok Version 0.0.0.0.3");
  puts("Let's Rok, Motherfuckers!!");
  puts("Press Ctrl-C to Exit\n");

  /* In a never ending loop */
  while (1) {
    /* Output our prompt and get input */
    char* input = readline("rok> ");

    /* Add input to history */
    add_history(input);

    /* Attempt to parse the user Input */
    mpc_result_t r;
    if (mpc_parse("<stdin>", input, Rok, &r)) {
      /* On Success Print the AST */
      long result = eval(r.output);
      printf("%li\n", result);
      mpc_ast_delete(r.output);
    } else {
      /* Otherwise print the error */
      mpc_err_print(r.error);
      mpc_err_delete(r.error);
    }

    /* Free retrieved input */
    free(input);
  }

  /* Undefine and Delete our Parsers */
  mpc_cleanup(4, Number, Operator, Expr, Rok);
  return 0;
}
