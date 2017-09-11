#ifndef lval_h
#define lval_h

struct lval;
struct lenv;
struct lbuiltin;
typedef struct lval lval;
typedef struct lenv lenv;
typedef lval*(*lbuiltin)(lenv*, lval*);

lval* lval_num(long x);
lval* lval_err(char* fmt, ...);
lval* lval_sym(char* sym);
lval* lval_sexpr(void);
lval* lval_qexpr(void);
lval* lval_fun(lbuiltin fun)
void lval_del(lval* v)
lval* lval_eval_sexpr(lenv* e, lval* v)
lval* lval_eval(lenv* e, lval* v)
void lval_println(lval* v);
lval* lval_read_num(mpc_ast_t* t);
lval* lval_read(mpc_ast_t* t);
void lval_expr_print(lval* v, char open, char close);
void lval_print(lval* v);
lval* lval_pop(lval* v, int i);
lval* lval_take(lval* v, int i);
lval* lval_join(lval* x, lval* y)
lval* lval_copy(lval* v);

#endif
