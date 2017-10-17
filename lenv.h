#ifndef lenv_h
#define lenv_h
#include "builtin.h"
#include "lval.h"

typedef struct lenv lenv;
struct lval;
typedef struct lval*(*lbuiltin)(lenv*, struct lval*);

struct lenv {
  lenv* parent;
  int count;
  char** syms;
  struct lval** vals;
};

lenv* lenv_new(void);
void lenv_del(lenv* env);
struct lval* lenv_get(lenv* env, struct lval* var);
void lenv_put(lenv* env, struct lval* var, struct lval* val);
lenv* lenv_copy(lenv* env);
void lenv_def(lenv* env, struct lval* var, struct lval* val);
void lenv_add_builtin(lenv* env, char* name, lbuiltin func);
void lenv_add_builtins(lenv* env);


#endif
