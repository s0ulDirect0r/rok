#ifndef builtin_h
#define builtin_h

#include "lval.h"
#include "lenv.h"

lval* builtin_op(struct lenv* e, lval* a, char* op);
lval* builtin_add(struct lenv* e, lval* a);
lval* builtin_sub(struct lenv* e, lval* a);
lval* builtin_mul(struct lenv* e, lval* a);
lval* builtin_div(struct lenv* e, lval* a);
lval* builtin_head(struct lenv* e, lval* a);
lval* builtin_tail(struct lenv* e, lval* a);
lval* builtin_list(struct lenv* e, lval* a);
lval* builtin_eval(struct lenv* e, lval* a);
lval* builtin_join(struct lenv* e, lval* a);
lval* builtin_def(struct lenv* e, lval* a);
lval* builtin_len(struct lenv* e, lval* a);
lval* builtin(struct lenv* e, lval* a, char* func);

#endif
