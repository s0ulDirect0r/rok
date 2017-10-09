clean:
	rm -f rok
rok:
	cc -std=c99 -g -Wall -Wextra rok.c mpc.c lval.c lenv.c builtin.c -ledit -lm -o rok
