#include <fcntl.h>
#include <stdio.h> 
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#define max_letters 1000
#define max_command 100
#define max_pipes 10
char * * * command;
int background;
int czy_pipe;
int pipe_command;
void handler(int signum) {
  printf("signal!\n");
  return;
}


void przekierowanie(char * output){
int fileDescriptor;
fileDescriptor = open(output, O_CREAT | O_TRUNC | O_WRONLY, 0600); 
dup2(fileDescriptor, STDOUT_FILENO); 
close(fileDescriptor);}

void free_d() {
  int i, j;
  for (i = 0; i < max_pipes; i++) {
    for (j = 0; j < max_command; j++)
      free(command[i][j]);
    free(command[i]);
  }
  free(command);
}

void alloc() /* Allocate the array */ {
  /* Check if allocation succeeded. (check for NULL pointer) */
  int i, j;
  command = malloc(max_pipes * sizeof(char * * ));
  for (i = 0; i < max_pipes; i++) {
    command[i] = malloc(max_command * sizeof(char * ));
    for (j = 0; j < max_command; j++)
      command[i][j] = malloc(max_letters * sizeof(char));
  }
}

int exe_process() {

  pid_t p1 = fork();

  if (p1 == -1) {
    return 0;
  } else if (p1 == 0) {
przekierowanie("a.txt");
    if (execvp(command[0][0], command[0]) < 0) {
      return 0;
    }
    exit(0);
  } else {

    if (background)
      sleep(60);
    else
      wait(NULL);
    return 1;
  }
}

int exe_pipped_processes() {
  int status;
  int i = 0;
  pid_t pid;

  int pipefds[2 * pipe_command];

  for (i = 0; i < (pipe_command); i++) {
    if (pipe(pipefds + i * 2) < 0) {
      perror("couldn't pipe");
      exit(EXIT_FAILURE);
    }
  }

  int j = 0;
  int c = 0;
  while (c <= pipe_command) {
    pid = fork();
    if (pid == 0) {
przekierowanie("a.txt");
      //if not last command
      if (c < pipe_command) {
        if (dup2(pipefds[j + 1], 1) < 0) {
          perror("dup2");
          exit(EXIT_FAILURE);
        }
      }

      //if not first command&& j!= 2*numPipes
      if (j != 0) {
        if (dup2(pipefds[j - 2], 0) < 0) {
          perror(" dup2"); ///j-2 0 j+1 1
          exit(EXIT_FAILURE);

        }
      }

      for (i = 0; i < 2 * pipe_command; i++) {
        close(pipefds[i]);
      }

      if (execvp(command[c][0], command[c]) < 0) {
        perror(command[c][0]);
        exit(EXIT_FAILURE);
      }
    } else if (pid < 0) {
      perror("error");
      exit(EXIT_FAILURE);
    }

    c++;
    j += 2;
  }
  /**Parent closes the pipes and wait for children*/

  for (i = 0; i < 2 * pipe_command; i++) {
    close(pipefds[i]);
  }

  for (i = 0; i < pipe_command + 1; i++)
    wait( & status);

}

char * path() {
  char * cwd = malloc(max_letters);
  char tmp[max_letters];
  getcwd(tmp, sizeof(tmp));
  strcpy(cwd, tmp);
  return cwd;
}

void parse(char * line) {
  int x, y = 0, z = 0, arg = 0;
  pipe_command = 0;
  for (x = 0; x < strlen(line); x++) {

    if (line[x] == ' ' || line[x + 1] == '\0') {

      strcpy(command[pipe_command][arg], strndup(line + z, y));

      if (line[x + 1] == '|') {
        x += 2;
        czy_pipe = 1;
        command[pipe_command][++arg] = NULL;
        pipe_command++;

        arg = -1;
      }
      arg++;
      z = x + 1;
      y = -1;
    }
    y++;
  }

  if (command[pipe_command][arg - 1][0] == '&') {
    background = 1;
    command[pipe_command][arg - 1] = NULL;
  } else {
    background = 0;
    command[pipe_command][arg] = NULL;
  }
}

int main() {

  int i;
  char * command_to_parse = (char * ) malloc((max_letters * max_command) * sizeof(char));

  while (1) {
    czy_pipe = 0;
    alloc();

    printf("%s ", path());
    fgets(command_to_parse, max_letters * max_command, stdin);
    parse(command_to_parse);

    if (!czy_pipe) {
      if (!exe_process())
        printf("Nie znaleziono polecenia \n");
    } else

      exe_pipped_processes();

    free_d();
  }

}
