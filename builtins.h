#ifndef builtins_h
#define builtins_h

#include "lval.h"
#include "lenv.h"

typedef lval*(*lbuiltin)(lenv*, lval*);

lval* builtin_op(lenv* e, lval* a, char* op);
lval* builtin_add(lenv* e, lval* a);
lval* builtin_sub(lenv* e, lval* a);
lval* builtin_mul(lenv* e, lval* a);
lval* builtin_div(lenv* e, lval* a);
lval* builtin_head(lenv* e, lval* a);
lval* builtin_tail(lenv* e, lval* a);
lval* builtin_list(lenv* e, lval* a);
lval* builtin_eval(lenv* e, lval* a);
lval* builtin_join(lenv* e, lval* a);
lval* builtin_def(lenv* e, lval* a);
lval* builtin_len(lenv* e, lval* a);
lval* builtin_cons(lenv* e, lval* a);
lval* builtin(lenv* e, lval* a, char* func);

#endif
