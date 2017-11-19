#include "lenv.h"
#include "lval.h"
#include "builtin.h"

/** Lval functions **/
lval* lval_num(long num) {
  lval* val = malloc(sizeof(lval));
  val->type = LVAL_NUM;
  val->num = num;
  return val;
}

lval* lval_err(char* fmt, ...) {
  lval* val = malloc(sizeof(lval));
  val->type = LVAL_ERR;

  /* Create a va_list and initialize it */
  va_list va;
  va_start(va, fmt);

  val->err = malloc(512);

  /* Print the error string with a maximum of 511 characters */
  vsnprintf(val->err, 511, fmt, va);

  /* Reallocate to number of bytes actually used */
  val->err = realloc(val->err, strlen(val->err)+1);

  /* Clean up VA list */
  va_end(va);
  return val;
}

lval* lval_sym(char* sym) {
  lval* val = malloc(sizeof(lval));
  val->type = LVAL_SYM;
  val->sym = malloc(strlen(sym) + 1);
  strcpy(val->sym, sym);
  return val;
}

lval* lval_bool(char* bool) {
  lval* val = malloc(sizeof(lval));
  val->type = LVAL_BOOL;
  val->bool = malloc(strlen(bool) + 1);
  strcpy(val->bool, bool);
  return val;
}

lval* lval_str(char* str) {
  lval* val = malloc(sizeof(lval));
  val->type = LVAL_STR;
  val->str = malloc(strlen(str) + 1);
  strcpy(val->str, str);
  return val;
}

lval* lval_sexpr(void) {
  lval* val = malloc(sizeof(lval));
  val->type = LVAL_SEXPR;
  val->count = 0;
  val->cell = NULL;
  return val;
}

lval* lval_qexpr(void) {
  lval* val = malloc(sizeof(lval));
  val->type = LVAL_QEXPR;
  val->count = 0;
  val->cell = NULL;
  return val;
}

lval* lval_fun(lbuiltin builtin) {
  lval* val = malloc(sizeof(lval));
  val->type = LVAL_FUN;
  val->builtin = builtin;
  return val;
}

lval* lval_eq(lval* x, lval* y) {
  /* Different types are always unequal */
  if (x->type != y->type) { return lval_bool("false");}


  /* Compare based upon type */
  switch (x->type) {
    /* Compare Number Value */
    case LVAL_NUM:
      if (x->num == y->num) {
        return lval_bool("true");
      } else {
        return lval_bool("false");
      }
    /* Compare String Values */
    case LVAL_BOOL:
      if (strcmp(x->bool, y->bool) == 0) {
        return lval_bool("true");
      } else {
        return lval_bool("false");
      }
    case LVAL_SYM:
      if (strcmp(x->sym, y->sym) == 0) {
        return lval_bool("true");
      } else {
        return lval_bool("false");
      }
    case LVAL_STR:
      if (strcmp(x->str, y->str) == 0) {
        return lval_bool("true");
      } else {
        return lval_bool("false");
      }
    case LVAL_ERR:
      if (strcmp(x->err, y->err) == 0) {
        return lval_bool("true");
      } else {
        return lval_bool("false");
      }
    /* If builtin compare, otherwise compare formals and body */
    case LVAL_FUN:
      if (x->builtin || y->builtin) {
         if (x->builtin == y->builtin) {
          return lval_bool("true");
        } else {
          return lval_bool("false");
        }
      } else {
        lval* formal_result = lval_eq(x->formals, y->formals);
        lval* body_result = lval_eq(x->body, y->body);
        return lval_eq(formal_result, body_result);
      }

    /* If list compare every individual element */
    case LVAL_QEXPR:
    case LVAL_SEXPR:
      if (x->count != y->count) {
        return lval_bool("false");
      } else {
        for (int i = 0; i < x->count; i++) {
          lval* result = lval_eq(x->cell[i], y->cell[i]);
          if (strcmp(result->bool, "true") != 0) {
            return result;
          }
        }
        /* Otherwise lists must be equal */
        return lval_bool("true");
      }
    break;
  }
  return lval_bool("false");
}

lval* lval_add(lval* val, lval* x) {
  val->count++;
  val->cell = realloc(val->cell, sizeof(lval*) * val->count);
  val->cell[val->count-1] = x;
  return val;
}

void lval_del(lval* val) {
  switch (val->type) {
    /* Do nothing special for number type */
    case LVAL_NUM: break;
    case LVAL_BOOL: break;

    /* For err or sym free the data */
    case LVAL_ERR: free(val->err); break;
    case LVAL_SYM: free(val->sym); break;
    case LVAL_STR: free(val->str); break;
    case LVAL_FUN:
      if (!val->builtin) {
        lenv_del(val->env);
        lval_del(val->formals);
        lval_del(val->body);
      }
    break;

    /* If qexpr or sexpr then delete all elements inside */
    case LVAL_QEXPR:
    case LVAL_SEXPR:
      for (int i = 0; i < val->count; i++) {
        lval_del(val->cell[i]);
      }

      /* Free memory allocated to store pointers */
      free(val->cell);
    break;
  }
  /* Free memory allocated for the lval struct itself */
  free(val);
}

lval* lval_read_num(mpc_ast_t* tree) {
  errno = 0;
  long num = strtol(tree->contents, NULL, 10);
  return errno != ERANGE ?
    lval_num(num) : lval_err("invalid number");
}

lval* lval_read_str(mpc_ast_t* tree) {
  tree->contents[strlen(tree->contents)-1] = '\0';
  char* unescaped = malloc(strlen(tree->contents+1)+1);
  strcpy(unescaped, tree->contents+1);
  unescaped = mpcf_unescape(unescaped);
  lval* str = lval_str(unescaped);
  free(unescaped);
  return str;
}

lval* lval_read(mpc_ast_t* tree) {
  /* If symbol or number return conversion to that type */
  if (strstr(tree->tag, "number")) { return lval_read_num(tree); }
  if (strstr(tree->tag, "boolean")) {
    return lval_bool(tree->contents);
  }
  if (strstr(tree->tag, "symbol")) { return lval_sym(tree->contents); }

  if (strstr(tree->tag, "string")) { return lval_read_str(tree); }

  /* If root (>) or sexpr then create empty list */
  lval* x = NULL;
  if (strcmp(tree->tag, ">") == 0) { x = lval_sexpr(); }
  if (strstr(tree->tag, "sexpr"))  { x = lval_sexpr(); }
  if (strstr(tree->tag, "qexpr"))  { x = lval_qexpr(); }

  /* Fill this list with any valid expression contained within */
  for (int i = 0; i < tree->children_num; i++) {
    if (strcmp(tree->children[i]->contents, "(") == 0) { continue; }
    if (strcmp(tree->children[i]->contents, ")") == 0) { continue; }
    if (strcmp(tree->children[i]->contents, "{") == 0) { continue; }
    if (strcmp(tree->children[i]->contents, "}") == 0) { continue; }
    if (strcmp(tree->children[i]->tag,  "regex") == 0) { continue; }
    if (strstr(tree->children[i]->tag, "comment")) { continue; }
    x = lval_add(x, lval_read(tree->children[i]));
  }

  return x;
}


void lval_expr_print(lval* val, char open, char close) {
  putchar(open);
  for (int i = 0; i < val->count; i++) {
    /* Print value contained within */
    lval_print(val->cell[i]);

    /* Don't print trailing space if last element */
    if (i != (val->count-1)) {
      putchar(' ');
    }
  }
  putchar(close);
}

void lval_print(lval* val) {
  switch(val->type) {
    /* In the case the type is a number print it */
    /* Then break out of the switch */
    case LVAL_NUM: printf("%li", val->num); break;
    case LVAL_ERR: printf("Error: %s", val->err); break;
    case LVAL_SYM: printf("%s", val->sym); break;
    case LVAL_STR: lval_print_str(val); break;
    case LVAL_BOOL: printf("%s", val->bool); break;
    case LVAL_FUN:
      if (val->builtin) {
        printf("<builtin>");
      } else {
        printf("(\\ "); lval_print(val->formals);
        putchar(' '); lval_print(val->body); putchar(')');
      }
    break;
    case LVAL_SEXPR: lval_expr_print(val, '(', ')'); break;
    case LVAL_QEXPR: lval_expr_print(val, '{', '}'); break;
    break;
  }
}

/* Print a lval string */
void lval_print_str(lval* val) {
  char* escaped = malloc(strlen(val->str)+1);
  strcpy(escaped, val->str);
  escaped = mpcf_escape(escaped);
  printf("\"%s\"", escaped);
  free(escaped);
}

lval* lval_pop(lval* val, int i) {
  /* Find the item at i */
  lval* x = val->cell[i];

  /* Shift memory after the item at "i" over the top */
  /* WTF is memmove? */
  memmove(&val->cell[i], &val->cell[i+1],
    sizeof(lval*) * (val->count-i-1));

  /* Decrease the count of items in the list */
  val->count--;

  /* Reallocate the memory used */
  val->cell = realloc(val->cell, sizeof(lval*) * val->count);
  return x;
}

lval* lval_take(lval* val, int i) {
  lval* x = lval_pop(val, i);
  lval_del(val);
  return x;
}


lval* lval_join(lval* x, lval* y) {
  /* For each cell in 'y' add it to 'x' */
  while(y->count) {
    x = lval_add(x, lval_pop(y, 0));
  }

  /* Delete the empty y and return x */
  lval_del(y);
  return x;
}

lval* lval_copy(lval* val) {
  lval* x = malloc(sizeof(lval));
  x->type = val->type;

  switch (val->type) {
    case LVAL_NUM: x->num = val->num; break;
    case LVAL_FUN:
      if (val->builtin) {
        x->builtin = val->builtin;
      } else {
        x->builtin = NULL;
        x->env = lenv_copy(val->env);
        x->formals = lval_copy(val->formals);
        x->body = lval_copy(val->body);
      }
    break;
    case LVAL_SYM:
      x->sym = malloc(strlen(val->sym) + 1);
      strcpy(x->sym, val->sym); break;
    case LVAL_STR:
      x->str = malloc(strlen(val->str) + 1);
      strcpy(x->str, val->str); break;
    case LVAL_BOOL:
      x->bool = malloc(strlen(val->bool) + 1);
      strcpy(x->bool, val->bool); break;

    case LVAL_ERR:
      x->err = malloc(strlen(val->err) + 1);
      strcpy(x->err, val->err); break;

    case LVAL_SEXPR:
    case LVAL_QEXPR:
      x->count = val->count;
      x->cell = malloc(sizeof(lval*) * x->count);
      for (int i = 0; i < x->count; i++) {
        x->cell[i] = lval_copy(val->cell[i]);
      }
    break;
  }

  return x;
}

lval* lval_lambda(lval* formals, lval* body) {
  lval* val = malloc(sizeof(lval));
  val->type = LVAL_FUN;

  /* Set builtin to null */
  val->builtin = NULL;

  /* build new environment */
  val->env = lenv_new();

  /* set formals and body */
  val->formals = formals;
  val->body = body;
  return val;
}

lval* lval_eval_sexpr(lenv* env, lval* val) {

  /* Evaluate Children */
  for (int i = 0; i < val->count; i++) {
    val->cell[i] = lval_eval(env, val->cell[i]);
  }

  /* Error checking */
  for (int i = 0; i < val->count; i++) {
    if (val->cell[i]->type == LVAL_ERR) { return lval_take(val, i); }
  }

  /* Empty Expression */
  if (val->count == 0) { return val; }

  /* Single Expression */
  if (val->count == 1) { return lval_eval(env, lval_take(val, 0)); }

  /* Ensure First Element is a function after evaluation */
  lval* fun = lval_pop(val, 0);
  if (fun->type != LVAL_FUN) {
    lval* err = lval_err(
      "S-Expression starts with incorrect type. "
      "Got %s, Expected %s. ",
      ltype_name(fun->type), ltype_name(LVAL_FUN));
    lval_del(fun); lval_del(val);
    return err;
  }

  /* If so call function to get result */
  lval* result = lval_call(env, fun, val);
  lval_del(fun);
  return result;
}

lval* lval_eval(lenv* env, lval* val) {
  if (val->type == LVAL_SYM) {
    lval* x = lenv_get(env, val);
    lval_del(val);
    return x;
  }
  /* Evaluate Sexpressions */
  if (val->type == LVAL_SEXPR) { return lval_eval_sexpr(env, val); }
  /* All other lval types remain the same */
  return val;
}

lval* lval_call(lenv* env, lval* fun, lval* args) {
  /* If Builtin then simply call that */
  if (fun->builtin) { return fun->builtin(env, args); };

  /* Record argument size */
  int given = args->count;
  int total = fun->formals->count;

  /* While arguments still remain to be processed */
  while (args->count) {
    /* If we've ran out of formal arguments to bind */
    if (fun->formals->count == 0) {
      lval_del(args); return lval_err(
        "Function passed too many arguments. "
        "Got %i, Expected %i",
        given, total);
    }

    /* Pop the first symbol from the formals */
    lval* sym = lval_pop(fun->formals, 0);

    /* Special case to deal with '&' */
    if (strcmp(sym->sym, "&") == 0) {
      /* Ensure & is followed by another symbol */
      if (fun->formals->count != 1) {
        lval_del(args);
        return lval_err("Function format invalid. "
          "Symbol '&' not followed by single symbol.");
      }

      /* Next formal should be bound to remaining arguments */
      lval* nsym = lval_pop(fun->formals, 0);
      lenv_put(fun->env, nsym, builtin_list(env, args));
      lval_del(sym); lval_del(nsym);
      break;
    }

    /* Pop the next argument from the list */
    lval* val = lval_pop(args, 0);

    /* Bind a copy into the function's argument */
    lenv_put(fun->env, sym, val);

    /* Delete symbol and value */
    lval_del(sym); lval_del(val);
  }

  /* Argument list is now bound so can be cleared up */
  lval_del(args);

  /* If '&' remains in formal list bind to empty list */
  if (fun->formals->count > 0 && strcmp(fun->formals->cell[0]->sym, "&") == 0) {
    /* Check to ensure that & is not passed invalidly. */
    if (fun->formals->count != 2) {
      return lval_err("Function format invalid. "
        "Symbol '&' not followed by single symbol.");
    }

    /* Pop and delete '&' symbol */
    lval_del(lval_pop(fun->formals, 0));

    /* Pop next symbol and create empty list */
    lval* sym = lval_pop(fun->formals, 0);
    lval* val = lval_qexpr();

    /* Bind to environment and delete */
    lenv_put(fun->env, sym, val);
    lval_del(sym); lval_del(val);
  }

  /* If all formals have been bound evaluate */
  if (fun->formals->count == 0) {

    /* Set environment parent to evaluation environment */
    fun->env->parent = env;

    /* Evaluate and return */
    return builtin_eval(fun->env, lval_add(lval_sexpr(), lval_copy(fun->body)));
  } else {
    /* Otherwise return partially evaluated function */
    return lval_copy(fun);
  }
}

/* Print an "lval" followed by a newline */
void lval_println(lval* val) { lval_print(val); putchar('\n'); }

/* Return string describing passed Lval Type */
char* ltype_name(int type) {
  switch(type) {
    case LVAL_FUN: return "Function";
    case LVAL_NUM: return "Number";
    case LVAL_BOOL: return "Boolean";
    case LVAL_ERR: return "Error";
    case LVAL_SYM: return "Symbol";
    case LVAL_STR: return "String";
    case LVAL_SEXPR: return "S-Expression";
    case LVAL_QEXPR: return "Q-Expression";
    default: return "Unknown";
  }
}
