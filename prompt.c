#include <stdio.h>
#include <stdlib.h>

/* Include editline's readline and history libraries */
#include <editline/readline.h>

int main(int argc, char** argv) {

  /* Print version and exit information */
  puts("Rok Version 0.0.0.0.1");
  puts("Press Ctrl-C to Exit\n");

  /* In a never ending loop */
  while (1) {
    /* Output our prompt and get input */
    char* input = readline("rok> ");

    /* Add input to history */
    add_history(input);

    /* Echo input back to user */
    printf("No you're a %s\n", input);

    /* Free retrieved input */
    free(input);
  }

  return 0;
}
