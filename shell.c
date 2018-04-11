
#include <stdio.h> 
#include <stdlib.h> 
#include <sys/types.h> 
#include <unistd.h>
#include <string.h>

  #define max_letters 1000
# define max_command 100

char * * command;
int background;

void free_d() {
  int i;
  for (i = 0; i < max_command; i++)
    free(command[i]);
  free(command);
}

void alloc() /* Allocate the array */ {
  /* Check if allocation succeeded. (check for NULL pointer) */
  int i;
  command = malloc(max_command * sizeof(char * ));
  for (i = 0; i < max_command; i++)
    command[i] = malloc(max_letters * sizeof(char));

}

int exe_process() {

  pid_t p1 = fork();

  if (p1 == -1) {
    return 0;
  } else if (p1 == 0) {

    if (execvp(command[0], command) < 0) {
      return 0;
    }
    exit(0);
  } else {
    if (!background)
      wait(NULL);
    else
      exit(0);
    return 1;
  }
}

char * path() {
  char * cwd = malloc(max_letters);
  char tmp[max_letters];
  getcwd(tmp, sizeof(tmp));
  strcpy(cwd, tmp);
  return cwd;
}

void clear() {
  int a, b;
  for (a = 0; a < max_command; a++)
    for (b = 0; b < max_letters; b++)
      command[a][b] = 0;

}

void parse(char * line) {
  int x, y = 0, z = 0, arg = 0;

  for (x = 0; x < strlen(line); x++) {

    if (line[x] == ' ' || line[x + 1] == '\0') {

      strcpy(command[arg], strndup(line + z, y));

      arg++;
      z = x + 1;
      y = -1;
    }
    y++;
  }

  command[arg] = NULL;
}

int main() {
  int i;

  char * command_to_parse = (char * ) malloc((max_letters * max_command) * sizeof(char));
  while (1) {

    alloc();
    printf("%s ", path());
    fgets(command_to_parse, max_letters * max_command, stdin);
    parse(command_to_parse);
    if (!exe_process(command))
      printf("Nie znaleziono polecenia \n");

    free_d();
  }

}
