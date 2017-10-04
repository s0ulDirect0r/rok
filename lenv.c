#include "lenv.h"
#include "lval.h"
#include "builtin.h"

/** Lenv functions **/
lenv* lenv_new(void) {
  lenv* e = malloc(sizeof(lenv));
  e->parent = NULL;
  e->count = 0;
  e->syms = NULL;
  e->vals = NULL;
  return e;
}

void lenv_del(lenv* e) {
  for(int i = 0; i < e->count; i++) {
    free(e->syms[i]);
    lval_del(e->vals[i]);
  }
  free(e->syms);
  free(e->vals);
  free(e);
}

lval* lenv_get(lenv* e, lval* k) {
  /* Iterate over all items in the environment */
  for (int i = 0; i < e->count; i++) {
    if(strcmp(e->syms[i], k->sym) == 0) {
      return lval_copy(e->vals[i]);
    }
  }

  if (e->parent) {
    return lenv_get(e->parent, k);
  } else {
    return lval_err("unbound symbol '%s'!", k->sym);
  }
}

void lenv_put(lenv* e, lval* k, lval* v) {
  /* Iterate */
  for (int i = 0; i < e->count; i++) {
    if(strcmp(e->syms[i], k->sym) == 0) {
      lval_del(e->vals[i]);
      e->vals[i] = lval_copy(v);
      return;
    }
  }

  /* If no existing entry found, allocate space for new entry */
  e->count++;
  e->vals = realloc(e->vals, sizeof(lval*) * e->count);
  e->syms = realloc(e->syms, sizeof(char*) * e->count);

  /* Copy contents of lval and symbol string to new location */
  e->vals[e->count-1] = lval_copy(v);
  e->syms[e->count-1] = malloc(strlen(k->sym)+1);
  strcpy(e->syms[e->count-1], k->sym);
}

lenv* lenv_copy(lenv* e) {
  lenv* copy = malloc(sizeof(lenv));
  copy->parent = e->parent;
  copy->count = e->count;
  copy->syms = malloc(sizeof(char*) * e->count);
  copy->vals = malloc(sizeof(lval*) * e->count);
  for (int i = 0; i < e->count; i++) {
    copy->syms[i] = malloc(strlen(e->syms[i]) + 1);
    strcpy(copy->syms[i], e->syms[i]);
    copy->vals[i] = lval_copy(e->vals[i]);
  }
  return copy;
}

void lenv_add_builtin(lenv* e, char* name, lbuiltin func) {
  lval* k = lval_sym(name);
  lval* v = lval_fun(func);
  lenv_put(e, k, v);
  lval_del(k); lval_del(v);
}

void lenv_add_builtins(lenv* e) {
  /* List functions */
  lenv_add_builtin(e, "list", builtin_list);
  lenv_add_builtin(e, "head", builtin_head);
  lenv_add_builtin(e, "tail", builtin_tail);
  lenv_add_builtin(e, "eval", builtin_eval);
  lenv_add_builtin(e, "len", builtin_len);
  lenv_add_builtin(e, "join", builtin_join);

  /* Math functions */
  lenv_add_builtin(e, "+", builtin_add);
  lenv_add_builtin(e, "-", builtin_sub);
  lenv_add_builtin(e, "*", builtin_mul);
  lenv_add_builtin(e, "/", builtin_div);

  /* Variable functions */
  lenv_add_builtin(e, "def", builtin_def);

  /* Lambda functions */
  lenv_add_builtin(e, "\\", builtin_lambda);
}
