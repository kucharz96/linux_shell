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

char* file;
int przekierowac;
int x;
pid_t p1;
int size;
char*** command;
int background;
int czy_pipe;
int pipe_command;
char history[20][max_letters * max_command];
int ziomek;
void handler()
{

    ziomek = 1;

    //kill(p1,SIGQUIT);
}

void add(char* text)
{
    strcpy(history[size], text);
    size++;
}

void przekierowanie(char* output)
{
    int fileDescriptor;
    fileDescriptor = open(output, O_CREAT | O_TRUNC | O_WRONLY, 0600);
    dup2(fileDescriptor, STDOUT_FILENO);
    close(fileDescriptor);
}

void free_d()
{
    int i, j;
    for (i = 0; i < max_pipes; i++) {
        for (j = 0; j < max_command; j++)
            free(command[i][j]);
        free(command[i]);
    }
    free(command);
}

void alloc() /* Allocate the array */
{
    /* Check if allocation succeeded. (check for NULL pointer) */
    int i, j;
    command = malloc(max_pipes * sizeof(char**));
    for (i = 0; i < max_pipes; i++) {
        command[i] = malloc(max_command * sizeof(char*));
        for (j = 0; j < max_command; j++)
            command[i][j] = malloc(max_letters * sizeof(char));
    }
}

int exe_process()
{

    p1 = fork();

    if (p1 == -1) {
        return 0;
    }
    else if (p1 == 0) {
        if (przekierowac)
            przekierowanie(file);
        if (execvp(command[0][0], command[0]) < 0) {
            return 0;
        }
        exit(0);
    }
    else {

        if (!background)

            wait(NULL);
        return 1;
    }
}

int exe_pipped_processes()
{
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
            if (przekierowac)
                przekierowanie(file);
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
        }
        else if (pid < 0) {
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
        wait(&status);
}

char* path()
{
    char* cwd = malloc(max_letters);
    char tmp[max_letters];
    getcwd(tmp, sizeof(tmp));
    strcpy(cwd, tmp);
    return cwd;
}

void parse(char* line)
{
    int x, y = 0, z = 0, arg = 0;
    pipe_command = 0;
    for (x = 0; x < strlen(line); x++) {

        if (line[x] == ' ' || line[x + 1] == '\0') {

            if (przekierowac)
                strcpy(file, strndup(line + z, y));

            else
                strcpy(command[pipe_command][arg], strndup(line + z, y));

            if (line[x + 1] == '|') {
                x += 2;
                czy_pipe = 1;
                command[pipe_command][++arg] = NULL;
                pipe_command++;

                arg = -1;
            }

            if (line[x + 1] == '>' && line[x + 2] == '>') {
                przekierowac = 1;
                x += 3;
            }
            else

                arg++;
            z = x + 1;
            y = -1;
        }
        y++;
    }
    if (command[pipe_command][arg - 1][0] == '&') {
        background = 1;
        command[pipe_command][arg - 1] = NULL;
    }
    else

        command[pipe_command][arg] = NULL;
}

void exe(char* line)
{
    alloc();
    add(line);
    parse(line);

    if (!czy_pipe) {
        if (!exe_process())
            printf("Nie znaleziono polecenia \n");
    }
    else
        exe_pipped_processes();

    free_d();
}

int main()
{
    int script = 0;
    FILE* script_file;
    size = 0;
    signal(SIGQUIT, handler);
    int fd;
    int i;
    char* command_to_parse = (char*)malloc((max_letters * max_command) * sizeof(char));
    file = (char*)malloc((max_letters * max_command) * sizeof(char));
    while (1) {

        background = 0;
        przekierowac = 0;
        czy_pipe = 0;

        printf("%s ", path());

        fgets(command_to_parse, max_letters * max_command, stdin);

        if (command_to_parse[0] == '.' && command_to_parse[1] == '/') {
            script = 1;
            fd = open(strndup(command_to_parse + 2, strlen(command_to_parse) - 3), O_RDONLY, 0666);
            script_file = fdopen(fd, "r");
            if (script_file == NULL)
                continue;

            char bufor[max_letters];
            while ((command_to_parse = fgets(bufor, sizeof(bufor), script_file)) != NULL)
                exe(command_to_parse);
        }
        else

            exe(command_to_parse);
    }
}
