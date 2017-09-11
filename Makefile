clean:
	rm -f rok
rok:
	cc -std=c99 -Wall rok.c mpc.c lval.c lenv.c builtins.c -ledit -lm -o rok
