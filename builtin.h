#ifndef builtin_h
#define builtin_h

#include "lval.h"
#include "lenv.h"

lval* builtin_op(struct lenv* env, lval* args, char* op);
lval* builtin_add(struct lenv* env, lval* args);
lval* builtin_sub(struct lenv* env, lval* args);
lval* builtin_mul(struct lenv* env, lval* args);
lval* builtin_div(struct lenv* env, lval* args);
lval* builtin_mod(struct lenv* env, lval* args);
lval* builtin_order(struct lenv* env, lval* args, char* op);
lval* builtin_greater(struct lenv* env, lval* args);
lval* builtin_greater_equal(struct lenv* env, lval* args);
lval* builtin_less(struct lenv* env, lval* args);
lval* builtin_less_equal(struct lenv* env, lval* args);
lval* builtin_equal(struct lenv* env, lval* args);
lval* builtin_not_equal(struct lenv* env, lval* args);
lval* builtin_if(struct lenv* env, lval* args);
lval* builtin_head(struct lenv* env, lval* args);
lval* builtin_tail(struct lenv* env, lval* args);
lval* builtin_list(struct lenv* env, lval* args);
lval* builtin_eval(struct lenv* env, lval* args);
lval* builtin_join(struct lenv* env, lval* args);
lval* builtin_def(struct lenv* env, lval* args);
lval* builtin_len(struct lenv* env, lval* args);
lval* builtin_lambda(struct lenv* env, lval* args);
lval* builtin_put(struct lenv* env, lval* args);
lval* builtin_var(struct lenv* env, lval* args, char* func);
lval* builtin_rok(struct lenv* env, lval* args);
lval* builtin_load(struct lenv* env, lval* args);
lval* builtin_print(struct lenv* env, lval* args);
lval* builtin_error(struct lenv* env, lval* args);

#endif
