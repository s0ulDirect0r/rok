#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "mpc.h"
#include "builtin.h"
#include "lenv.h"
#include "lval.h"
#include "rok.h"

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

int main() {
  /* Create some parsers */
  Number   = mpc_new("number");
  Boolean  = mpc_new("boolean");
  Symbol   = mpc_new("symbol");
  String   = mpc_new("string");
  Comment  = mpc_new("comment");
  Sexpr    = mpc_new("sexpr");
  Qexpr    = mpc_new("qexpr");
  Expr     = mpc_new("expr");
  Rok      = mpc_new("rok");

  /* Define them with the following Language */
  mpca_lang(MPCA_LANG_DEFAULT,
    " number   : /[+-]?([0-9]*[.])?[0-9]+/ ;               "
    " boolean  : /true|false/ ;                            "
    " symbol   : /[a-zA-Z0-9_+\\-*\\/\\\\=<>!&]+/ ;        "
    " string   : /\"(\\\\.|[^\"])*\"/ ;                    "
    " comment  : /;[^\\r\\n]*/ ;                           "
    " sexpr    : '(' <expr>* ')' ;                         "
    " qexpr    : '{' <expr>* '}' ;                         "
    " expr     : <number> | <boolean> | <symbol> |         \
                 <string> | <sexpr> | <qexpr> ;            "
    " rok      : /^/ <expr>* /$/ ;                         ",
    Number, Boolean, Symbol, String, Comment, Sexpr, Qexpr, Expr, Rok);

  /* Print version and exit information */
  puts("Rok Version 0.0.0.0.12");
  puts("Let's Rok!!!");
  puts("Press Ctrl-C to Exit\n");

  lenv* env = lenv_new();
  lenv_add_builtins(env);

  /* In a never ending loop */
  while (1) {
    /* Output our prompt and get input */
    char* input = readline("rok> ");

    /* Add input to history */
    add_history(input);

    /* Attempt to parse the user Input */
    mpc_result_t result;
    if (mpc_parse("<stdin>", input, Rok, &result)) {
      /* On Success Print the AST */
      lval* x = lval_eval(env, lval_read(result.output));
      lval_println(x);
      lval_del(x);
      mpc_ast_delete(result.output);
    } else {
      /* Otherwise print the error */
      mpc_err_print(result.error);
      mpc_err_delete(result.error);
    }

    /* Free retrieved input */
    free(input);
  }

  /* Undefine and Delete our Parsers */
  lenv_del(env);
  mpc_cleanup(9, Number, Boolean, Symbol, String, Comment, Sexpr, Qexpr, Expr, Rok);
  return 0;
}
