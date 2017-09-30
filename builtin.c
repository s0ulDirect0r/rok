#include "builtin.h"
#include "lenv.h"
#include "lval.h"

#define LASSERT(args, cond, fmt, ...) \
  if (!(cond)) { \
    lval* err = lval_err(fmt, ##__VA_ARGS__); \
    lval_del(args); \
    return err; \
  }

lval* builtin_op(lenv* e, lval* a, char* op) {
  /* Ensure all arguments are numbers */
  for (int i = 0; i < a->count; i++) {
    if(a->cell[i]->type != LVAL_NUM) {
      lval_del(a);
      return lval_err("Cannot operate on non-numbers");
    }
  }

  /* Pop the first element */
  lval* x = lval_pop(a, 0);

  /* If no arguments and sub then perform unary negation */
  if ((strcmp(op, "-") == 0) && a->count == 0) {
    x->num = -x->num;
  }

  /* While there are still elements remaining */
  while (a->count > 0) {
    /* Pop the next element */
    lval* y = lval_pop(a, 0);

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
    lval_del(y);
  }

  lval_del(a); return x;
}

lval* builtin_add(lenv* e, lval* a) {
  return builtin_op(e, a, "+");
}

lval* builtin_sub(lenv* e, lval* a) {
  return builtin_op(e, a, "-");
}

lval* builtin_mul(lenv* e, lval* a) {
  return builtin_op(e, a, "*");
}

lval* builtin_div(lenv* e, lval* a) {
  return builtin_op(e, a, "/");
}

lval* builtin_head(lenv* e, lval* a) {
  /* Check Error Conditions */
  int count = 1;
  LASSERT(a, a->cell[0]->type == LVAL_QEXPR,
     "Function 'head' passed incorrect types! \n"
     "Got %s, Expected %s.",
     ltype_name(a->cell[0]->type), ltype_name(LVAL_QEXPR));
  LASSERT(a, a->cell[0]->count != 0,
   "Function 'head' passed {}!");
  LASSERT(a, a->count == count,
    "Function 'head' passed too many arguments! \n"
    "Got %i, Expected %i.",
    a->count, 1);

  /* Otherwise take first argument */
  lval* v = lval_take(a, 0);

  /* Delete all elements that are not head and return */
  while(v->count > 1) { lval_del(lval_pop(v, 1)); }
  return v;
}

lval* builtin_tail(lenv* e, lval* a) {
  /* Check Error Conditions */
  LASSERT(a, a->count == 1,
    "Function 'tail' passed too many arguments! \n"
    "Got %i, Expected %i",
    a->count, 1);
  LASSERT(a, a->cell[0]->type == LVAL_QEXPR,
    "Function 'tail' passed incorrect types! \n"
    "Got %s, Expected %s",
    ltype_name(a->cell[0]->type), ltype_name(LVAL_QEXPR));
  LASSERT(a, a->cell[0]->count != 0,
    "Function 'tail' passed {}!");

  /* Other wise take first arg */
  lval* v = lval_take(a, 0);

  /* Delete first element and return */
  lval_del(lval_pop(v, 0));
  return v;
}

lval* builtin_list(lenv* e, lval* a) {
  a->type = LVAL_QEXPR;
  return a;
}

lval* builtin_eval(lenv* e, lval* a) {
  LASSERT(a, a->count == 1,
    "Function 'eval' passed too many arguments! \n"
    "Got %s, Expected %s",
    a->count, 1);
  LASSERT(a, a->cell[0]->type == LVAL_QEXPR,
     "Function 'eval' passed incorrect type! \n"
     "Got %s, Expected %s",
     ltype_name(a->cell[0]->type), ltype_name(LVAL_QEXPR));
  lval* x = lval_take(a, 0);
  x->type = LVAL_SEXPR;
  return lval_eval(e, x);
}

lval* builtin_join(lenv* e, lval* a) {
  for(int i = 0; i < a->count; i++) {
    LASSERT(a, a->cell[0]->type == LVAL_QEXPR,
      "Function 'join' passed incorrect types! \n"
      "Got %s, Expected %s",
      ltype_name(a->cell[0]->type), ltype_name(LVAL_QEXPR));
  }

  lval* x = lval_pop(a, 0);

  while(a->count) {
    x = lval_join(x, lval_pop(a, 0));
  }

  lval_del(a);
  return x;
}

lval* builtin_def(lenv* e, lval* a) {
  LASSERT(a, a->cell[0]->type == LVAL_QEXPR,
    "Function 'def' passed incorrect type! \n"
    "Got %s, Expected %s",
    ltype_name(a->cell[0]->type), ltype_name(LVAL_QEXPR));

  /* First argument is symbol list */
  lval* syms = a->cell[0];

  /* Ensure all elements of first list are symbols */
  for(int i = 0; i < syms->count; i++) {
    LASSERT(a, syms->cell[i]->type == LVAL_SYM,
    "Function 'def' cannot define non-symbols! \n"
    "Got %s, Expected %s",
    ltype_name(syms->cell[i]->type), ltype_name(LVAL_SYM));
  }

  /* Check correct number of symbols and values */
    LASSERT(a, syms->count == a->count-1,
    "Function 'def' cannot define incorrect "
    "number of values to symbols. \n"
    "Got %i values, Expected %i values",
    syms->count, a->count-1);

  /* Assign copies of values to symbols */
  for (int i = 0; i < syms->count; i++) {
    lenv_put(e, syms->cell[i], a->cell[i+1]);
  }

  lval_del(a);
  return lval_sexpr();
}

// Return the number of elements in a Qexpression
lval* builtin_len(lenv* e, lval* a) {
  return lval_num(a->cell[0]->count);
}

// Define a new lambda
lval* builtin_lambda(lenv* e, lvla* a) {
  /* Check two arguments, each of which are Q-Expressions */

  /* Check if first Q-expression contains only symbols */

  /* Pop first two arguments and pass them to lval_lambda */
}

lval* builtin(lenv* e, lval* a, char* func) {
  if(strcmp("list", func) == 0) { return builtin_list(e, a); }
  if(strcmp("head", func) == 0) { return builtin_head(e, a); }
  if(strcmp("tail", func) == 0) { return builtin_tail(e, a); }
  if(strcmp("eval", func) == 0) { return builtin_eval(e, a); }
  if(strcmp("join", func) == 0) { return builtin_join(e, a); }
  if(strcmp("len", func) == 0) { return builtin_len(e, a); }
  if(strcmp("def", func) == 0) { return builtin_def(e, a); }
  if(strstr("+-/*^%", func)) { return builtin_op(e, a, func); }
  lval_del(a);
  return lval_err("Unknown Function!");
}
