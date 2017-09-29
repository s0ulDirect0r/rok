clean:
	rm -f rok
rok:
	cc -std=c99 -Wall rok.c mpc.c lval.c lenv.c builtin.c -ledit -lm -o rok
