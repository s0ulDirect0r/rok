#ifndef lenv_h
#define lenv_h
#include "builtin.h"
#include "lval.h"

typedef struct lenv lenv;
struct lval;
typedef struct lval*(*lbuiltin)(lenv*, struct lval*);

struct lenv {
  int count;
  char** syms;
  struct lval** vals;
};

lenv* lenv_new(void);
void lenv_del(lenv* e); struct lval* lenv_get(lenv* e, struct lval* k);
void lenv_put(lenv* e, struct lval* k, struct lval* v);
lenv* lenv_copy(lenv* e);
void lenv_add_builtin(lenv* e, char* name, lbuiltin func);
void lenv_add_builtins(lenv* e);


#endif
