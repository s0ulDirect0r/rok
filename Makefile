clean:
	rm -f rok
rok:
	cc -std=c99 -Wall rok.c mpc.c -ledit -lm -o rok
