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
enum lval_types { LVAL_NUM, LVAL_ERR };

/* Enums for Errors */
enum error_types { LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM };

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

void lval_print(lval v) {
  switch(v.type) {
    /* In the case the type is a number print it */
    /* Then break out of the switch */
    case LVAL_NUM: printf("%li", v.num); break;

    /* In the case the type is an error */
    case LVAL_ERR:
      if(v.err == LERR_DIV_ZERO) {
        printf("Error: Division By Zero!");
      }
      if(v.err == LERR_BAD_OP) {
        printf("Error: Invalid Operator!");
      }
      if(v.err == LERR_BAD_NUM) {
        printf("Error: Invalid Number!");
      }
    break;
  }
}

/* Print an "lval" followed by a newline */
void lval_println(lval v) { lval_print(v); putchar('\n'); }

/* Use operator string to see which operation to perform */
lval eval_op(lval value1, char* op, lval value2) {
  /* If either value is an error return */
  if(value1.type == LVAL_ERR) { return value1; }
  if(value2.type == LVAL_ERR) { return value2; }
  if(strcmp(op, "+") == 0) { return lval_num(value1.num + value2.num); }
  if(strcmp(op, "-") == 0) { return lval_num(value1.num - value2.num); }
  if(strcmp(op, "*") == 0) { return lval_num(value1.num * value2.num); }
  if(strcmp(op, "/") == 0) {
    return value2.num == 0
      ? lval_err(LERR_DIV_ZERO)
      : lval_num(value1.num / value2.num);
  }
  if(strcmp(op, "%") == 0) { return lval_num(value1.num % value2.num); }
  if(strcmp(op, "^") == 0) { return lval_num((long)pow(value1.num, value2.num)); }
  return lval_err(LERR_BAD_OP);
}

lval eval(mpc_ast_t* t) {
  /* if tagged as number return it directly. */
  if (strstr(t->tag, "number")) {
    errno = 0;
    long x = strtol(t->contents, NULL, 10);
    return errno != ERANGE ? lval_num(x) : lval_err(LERR_BAD_NUM);
  }

  /* the operator is always second child */
  char* op = t->children[1]->contents;

  /* We store the third child in x */
  lval x = eval(t->children[2]);

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
  mpc_parser_t* Symbol   = mpc_new("symbol");
  mpc_parser_t* Sexpr    = mpc_new("sexpr");
  mpc_parser_t* Expr     = mpc_new("expr");
  mpc_parser_t* Rok      = mpc_new("rok");

  /* Define them with the following Language */
  mpca_lang(MPCA_LANG_DEFAULT,
    " number   : /[+-]?([0-9]*[.])?[0-9]+/ ;             "
    " symbol   : '+' | '-' | '*' | '/' | '%' | '^' ;     "
    " sexpr    : '(' <expr>* ')' ;                       "
    " expr     : <number> | <symbol> | <sexpr> ;         "
    " rok      : /^/ <expr>* /$/ ;                       ",
    Number, Symbol, Sexpr, Expr, Rok);

  /* Print version and exit information */
  puts("Rok Version 0.0.0.0.8");
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
      lval result = eval(r.output);
      lval_println(result);
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
  mpc_cleanup(5, Number, Symbol, Sexpr, Expr, Rok);
  return 0;
}
