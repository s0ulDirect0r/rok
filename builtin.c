#include "builtin.h"
#include "lenv.h"
#include "lval.h"

#define LASSERT(args, cond, fmt, ...) \
  if (!(cond)) { \
    lval* err = lval_err(fmt, ##__VA_ARGS__); \
    lval_del(args); \
    return err; \
  }

#define LASSERT_TYPE(func, args, index, expect) \
  LASSERT(args, args->cell[index]->type == expect, \
    "Function '%s' passed incorrect number of arguments. " \
    "Got %i, Expected %i.", \
    func, index, ltype_name(args->cell[index]->type), ltype_name(expect))

#define LASSERT_NUM(func, args, num) \
  LASSERT(args, args->count == num, \
    "Function '%s' passed incorrect number of arguments. " \
    "Got %i, Expected %i.", \
    func, args->count, num)

#define LASSERT_NOT_EMPTY(func, args, index) \
  LASSERT(args, args->cell[index]->count != 0, \
    "Function '%s' passed {} for argument %i.", func, index)

lval* builtin_op(lenv* env, lval* args, char* op) {
  /* Ensure all arguments are numbers */
  for (int i = 0; i < args->count; i++) {
    if(args->cell[i]->type != LVAL_NUM) {
      lval_del(args);
      return lval_err("Cannot operate on non-numbers");
    }
  }

  /* Pop the first element */
  lval* x = lval_pop(args, 0);

  /* If no arguments and sub then perform unary negation */
  if ((strcmp(op, "-") == 0) && args->count == 0) {
    x->num = -x->num;
  }

  /* While there are still elements remaining */
  while (args->count > 0) {
    /* Pop the next element */
    lval* y = lval_pop(args, 0);

    if(strcmp(op, "+") == 0) { x->num += y->num; }
    if(strcmp(op, "-") == 0) { x->num -= y->num; }
    if(strcmp(op, "*") == 0) { x->num *= y->num; }
    if(strcmp(op, "/") == 0) {
      if(y->num == 0) {
        lval_del(x); lval_del(y);
        x = lval_err("Division By Zero!"); break;
      }
      x->num /= y->num;
    }
    if(strcmp(op, "%") == 0) { x->num %= y->num; }
    if(strcmp(op, "^") == 0) { x->num = (long)pow(x->num, y->num);}
    if(strcmp(op, ">") == 0) {
      x->type = LVAL_BOOL;
      if(x->num > y->num) {
        x->bool = "true";
      } else {
        x->bool = "false";
      }
    }
    if(strcmp(op, ">=") == 0) {
      x->type = LVAL_BOOL;
      if(x->num >= y->num) {
        x->bool = "true";
      } else {
        x->bool = "false";
      }
    }
    if(strcmp(op, "<") == 0) {
      x->type = LVAL_BOOL;
      if(x->num < y->num) {
        x->bool = "true";
      } else {
        x->bool = "false";
      }
    }
    if(strcmp(op, "<=") == 0) {
      x->type = LVAL_BOOL;
      if(x->num <= y->num) {
        x->bool = "true";
      } else {
        x->bool = "false";
      }
    }
    if(strcmp(op, "==") == 0) {
      x->type = LVAL_BOOL;
      if(x->num == y->num) {
        x->bool = "true";
      } else {
        x->bool = "false";
      }
    }
    lval_del(y);
  }
  lval_del(args); return x;
}

lval* builtin_add(lenv* env, lval* args) {
  return builtin_op(env, args, "+");
}

lval* builtin_sub(lenv* env, lval* args) {
  return builtin_op(env, args, "-");
}

lval* builtin_mul(lenv* env, lval* args) {
  return builtin_op(env, args, "*");
}

lval* builtin_div(lenv* env, lval* args) {
  return builtin_op(env, args, "/");
}

lval* builtin_order(lenv* env, lval* args, char* op) {
  return lval_new()
}

lval* builtin_greater(lenv* env, lval* args) {
  return builtin_order(env, args, ">");
}

lval* builtin_greater_equal(lenv* env, lval* args) {
  return builtin_order(env, args, ">=");
}

lval* builtin_less(lenv* env, lval* args) {
  return builtin_order(env, args, "<");
}

lval* builtin_less_equal(lenv* env, lval* args) {
  return builtin_order(env, args, "<=");
}

lval* builtin_equal(lenv* env, lval* args) {
  return builtin_op(env, args, "==");
}

lval* builtin_head(lenv* env, lval* args) {
  /* Check Error Conditions */
  int count = 1;
  LASSERT(args, args->cell[0]->type == LVAL_QEXPR,
     "Function 'head' passed incorrect types! \n"
     "Got %s, Expected %s.",
     ltype_name(args->cell[0]->type), ltype_name(LVAL_QEXPR));
  LASSERT(args, args->cell[0]->count != 0,
   "Function 'head' passed {}!");
  LASSERT(args, args->count == count,
    "Function 'head' passed too many arguments! \n"
    "Got %i, Expected %i.",
    args->count, 1);

  /* Otherwise take first argument */
  lval* head = lval_take(args, 0);

  /* Delete all elements that are not head and return */
  while(head->count > 1) { lval_del(lval_pop(head, 1)); }
  return head;
}

lval* builtin_tail(lenv* env, lval* args) {
  /* Check Error Conditions */
  LASSERT(args, args->count == 1,
    "Function 'tail' passed too many arguments! \n"
    "Got %i, Expected %i",
    args->count, 1);
  LASSERT(args, args->cell[0]->type == LVAL_QEXPR,
    "Function 'tail' passed incorrect types! \n"
    "Got %s, Expected %s",
    ltype_name(args->cell[0]->type), ltype_name(LVAL_QEXPR));
  LASSERT(args, args->cell[0]->count != 0,
    "Function 'tail' passed {}!");

  /* Other wise take first arg */
  lval* tail = lval_take(args, 0);

  /* Delete first element and return */
  lval_del(lval_pop(tail, 0));
  return tail;
}

lval* builtin_list(lenv* env, lval* args) {
  args->type = LVAL_QEXPR;
  return args;
}

lval* builtin_eval(lenv* env, lval* args) {
  LASSERT(args, args->count == 1,
    "Function 'eval' passed too many arguments! \n"
    "Got %s, Expected %s",
    args->count, 1);
  LASSERT(args, args->cell[0]->type == LVAL_QEXPR,
     "Function 'eval' passed incorrect type! \n"
     "Got %s, Expected %s",
     ltype_name(args->cell[0]->type), ltype_name(LVAL_QEXPR));
  lval* sexpr = lval_take(args, 0);
  sexpr->type = LVAL_SEXPR;
  return lval_eval(env, sexpr);
}

lval* builtin_join(lenv* env, lval* args) {
  for(int i = 0; i < args->count; i++) {
    LASSERT(args, args->cell[0]->type == LVAL_QEXPR,
      "Function 'join' passed incorrect types! \n"
      "Got %s, Expected %s",
      ltype_name(args->cell[0]->type), ltype_name(LVAL_QEXPR));
  }

  lval* joined_lval = lval_pop(args, 0);

  while(args->count) {
    joined_lval = lval_join(joined_lval, lval_pop(args, 0));
  }

  lval_del(args);
  return joined_lval;
}

// Return the number of elements in a Qexpression
lval* builtin_len(lenv* env, lval* args) {
  return lval_num(args->cell[0]->count);
}

// Define a new lambda
lval* builtin_lambda(lenv* env, lval* args) {
  /* Check two arguments, each of which are Q-Expressions */
  LASSERT_NUM("\\", args, 2);
  LASSERT_TYPE("\\", args, 0, LVAL_QEXPR);
  LASSERT_TYPE("\\", args, 1, LVAL_QEXPR);

  /* Check if first Q-expression contains only symbols */
  for (int i = 0; i < args->cell[0]->count; i++) {
    LASSERT(args, (args->cell[0]->cell[i]->type == LVAL_SYM),
    "Cannot define non-symbol. Got %s, Expected %s.",
    ltype_name(args->cell[0]->cell[i]->type), ltype_name(LVAL_SYM));
  }

  /* Pop first two arguments and pass them to lval_lambda */
  lval* formals = lval_pop(args, 0);
  lval* body = lval_pop(args, 0);
  lval_del(args);
  return lval_lambda(formals, body);
}

lval* builtin_def(lenv* env, lval* args) {
  return builtin_var(env, args, "def");
}

lval* builtin_put(lenv* env, lval* args) {
  return builtin_var(env, args, "=");
}

lval* builtin_var(lenv* env, lval* args, char* func) {
  LASSERT_TYPE(func, args, 0, LVAL_QEXPR);

  lval* syms = args->cell[0];
  for (int i = 0; i < syms->count; i++) {
    LASSERT(args, (syms->cell[i]->type == LVAL_SYM),
      "Function '%s' cannot define non-symbol. "
      "Got %s, Expected %s.", func,
      ltype_name(syms->cell[i]->type),
      ltype_name(LVAL_SYM));
  }

  LASSERT(args, (syms->count == args->count-1),
    "Function '%s' passed too many arguments for symbols. "
    "Got %i, Expected %i.", func, syms->count, args->count-1);

  for (int i = 0; i < syms->count; i++) {
    /* If 'def' define in globally. If 'put' define in locally */
    if (strcmp(func, "def") == 0) {
      lenv_def(env, syms->cell[i], args->cell[i+1]);
    }

    if (strcmp(func, "=") == 0) {
      lenv_put(env, syms->cell[i], args->cell[i+1]);
    }
  }

  lval_del(args);
  return lval_sexpr();
}
