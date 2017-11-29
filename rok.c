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

char* check_filename_ext(char* filename) {
  char* dot = strchr(filename, '.');
  if (!dot || dot == filename) return "Wrong file type. Please use .rok extension.";
  if (strcmp(dot + 1, "rok") == 0) return "OK";
  return "Wrong file type. Please use .rok extension.";
}

int main(int argc, char** argv) {
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
    " symbol   : /[a-zA-Z0-9_+\\-*\\/\\\\=<>!&%]+/ ;        "
    " string   : /\"(\\\\.|[^\"])*\"/ ;                    "
    " comment  : /;[^\\r\\n]*/ ;                           "
    " sexpr    : '(' <expr>* ')' ;                         "
    " qexpr    : '{' <expr>* '}' ;                         "
    " expr     : <number> | <boolean> | <symbol> |         \
                 <string> | <comment> | <sexpr> | <qexpr> ;"
    " rok      : /^/ <expr>* /$/ ;                         ",
    Number, Boolean, Symbol, String, Comment, Sexpr, Qexpr, Expr, Rok);

  lenv* env = lenv_new();
  lenv_add_builtins(env);

  if (argc == 1) {

      /* Print version and exit information */
      puts("Rok Version 0.0.1");
      puts("Let's Rok!!!");
      puts("Press Ctrl-C to Exit\n");

      lval* standard = lval_add(lval_sexpr(), lval_str("standard.rok"));
      lval* load = builtin_load(env, standard);
      lval_del(load);
      puts("Standard library loaded.\n");
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
  }

  if (argc >= 2) {
    for (int i = 1; i < argc; i++) {
      /* Take filename and translate to lvals */
      char* check = check_filename_ext(argv[i]);
      if (strcmp(check, "OK") != 0) {
        lval* err = lval_err(check);
        lval_println(err);
        lval_del(err);
        return 0;
      }
      lval* args = lval_add(lval_sexpr(), lval_str(argv[i]));

      lval* standard = lval_add(lval_sexpr(), lval_str("standard.rok"));
      lval* load = builtin_load(env, standard);
      lval_del(load);

      /* Load and get result */
      lval* x = builtin_load(env, args);

      /* If the result is an error print it */
      if (x->type == LVAL_ERR) { lval_println(x); }
      lval_del(x);
    }
  }

  /* Undefine and Delete our Parsers */
  lenv_del(env);
  mpc_cleanup(9, Number, Boolean, Symbol, String, Comment, Sexpr, Qexpr, Expr, Rok);
  return 0;
}
