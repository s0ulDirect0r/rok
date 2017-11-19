#include "lenv.h"
#include "lval.h"
#include "builtin.h"

/** Lenv functions **/
lenv* lenv_new(void) {
  lenv* env = malloc(sizeof(lenv));
  env->parent = NULL;
  env->count = 0;
  env->syms = NULL;
  env->vals = NULL;
  return env;
}

void lenv_del(lenv* env) {
  for(int i = 0; i < env->count; i++) {
    free(env->syms[i]);
    lval_del(env->vals[i]);
  }
  free(env->syms);
  free(env->vals);
  free(env);
}

lval* lenv_get(lenv* env, lval* var) {
  /* Iterate over all items in the environment */
  for (int i = 0; i < env->count; i++) {
    if(strcmp(env->syms[i], var->sym) == 0) {
      return lval_copy(env->vals[i]);
    }
  }

  if (env->parent) {
    return lenv_get(env->parent, var);
  } else {
    return lval_err("unbound symbol '%s'!", var->sym);
  }
}

void lenv_put(lenv* env, lval* var, lval* val) {
  /* Iterate */
  for (int i = 0; i < env->count; i++) {
    if(strcmp(env->syms[i], var->sym) == 0) {
      lval_del(env->vals[i]);
      env->vals[i] = lval_copy(val);
      return;
    }
  }

  /* If no existing entry found, allocate space for new entry */
  env->count++;
  env->vals = realloc(env->vals, sizeof(lval*) * env->count);
  env->syms = realloc(env->syms, sizeof(char*) * env->count);

  /* Copy contents of lval and symbol string to new location */
  env->vals[env->count-1] = lval_copy(val);
  env->syms[env->count-1] = malloc(strlen(var->sym)+1);
  strcpy(env->syms[env->count-1], var->sym);
}

lenv* lenv_copy(lenv* env) {
  lenv* copy = malloc(sizeof(lenv));
  copy->parent = env->parent;
  copy->count = env->count;
  copy->syms = malloc(sizeof(char*) * env->count);
  copy->vals = malloc(sizeof(lval*) * env->count);
  for (int i = 0; i < env->count; i++) {
    copy->syms[i] = malloc(strlen(env->syms[i]) + 1);
    strcpy(copy->syms[i], env->syms[i]);
    copy->vals[i] = lval_copy(env->vals[i]);
  }
  return copy;
}

void lenv_def(lenv* env, lval* var, lval* val) {
  /* Iterate till env has no parent */
  while (env->parent) { env = env->parent; }
  /* Put value in */
  lenv_put(env, var, val);
}

void lenv_add_builtin(lenv* env, char* name, lbuiltin func) {
  lval* var = lval_sym(name);
  lval* val = lval_fun(func);
  lenv_put(env, var, val);
  lval_del(var); lval_del(val);
}

void lenv_add_builtins(lenv* env) {
  /* List functions */
  lenv_add_builtin(env, "list", builtin_list);
  lenv_add_builtin(env, "head", builtin_head);
  lenv_add_builtin(env, "tail", builtin_tail);
  lenv_add_builtin(env, "eval", builtin_eval);
  lenv_add_builtin(env, "len", builtin_len);
  lenv_add_builtin(env, "join", builtin_join);

  /* Math functions */
  lenv_add_builtin(env, "+", builtin_add);
  lenv_add_builtin(env, "-", builtin_sub);
  lenv_add_builtin(env, "*", builtin_mul);
  lenv_add_builtin(env, "/", builtin_div);

  /* Comparison functions */
  lenv_add_builtin(env, ">", builtin_greater);
  lenv_add_builtin(env, ">=", builtin_greater_equal);
  lenv_add_builtin(env, "<", builtin_less);
  lenv_add_builtin(env, "<=", builtin_less_equal);
  lenv_add_builtin(env, "==", builtin_equal);
  lenv_add_builtin(env, "!=", builtin_not_equal);
  lenv_add_builtin(env, "if", builtin_if);

  /* Variable functions */
  lenv_add_builtin(env, "def", builtin_def);
  lenv_add_builtin(env, "=", builtin_put);

  /* Lambda functions */
  lenv_add_builtin(env, "\\", builtin_lambda);

  /* File system functions */
  lenv_add_builtin(env, "load", builtin_load);
  lenv_add_builtin(env, "print", builtin_print);
  lenv_add_builtin(env, "error", builtin_error);
}
