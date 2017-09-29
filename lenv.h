#ifndef lenv_h
#define lenv_h
#include "builtin.h"
#include "lval.h"

typedef struct lenv {
  int count;
  char** syms;
  struct lval** vals;
} lenv;

lenv* lenv_new(void);
void lenv_del(lenv* e); lval* lenv_get(lenv* e, lval* k);
void lenv_put(lenv* e, lval* k, lval* v);
lenv* lenv_copy(lenv* e);
void lenv_add_builtin(lenv* e, char* name, lbuiltin func);
void lenv_add_builtins(lenv* e);


#endif
