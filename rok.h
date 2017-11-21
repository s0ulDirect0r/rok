#ifndef rok_h
#define rok_h

#include "mpc.h"

/* Parser Declarations */
mpc_parser_t* Number;
mpc_parser_t* Boolean;
mpc_parser_t* Symbol;
mpc_parser_t* String;
mpc_parser_t* Comment;
mpc_parser_t* Sexpr;
mpc_parser_t* Qexpr;
mpc_parser_t* Expr;
mpc_parser_t* Rok;

#endif
