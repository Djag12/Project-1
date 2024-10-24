#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

#define MAX_COMMAND_LINE_LEN 1024
#define MAX_COMMAND_LINE_ARGS 128

char prompt[] = "> ";
char delimiters[] = " \t\r\n";
extern char **environ;

// Signal handler for SIGINT (Ctrl+C)
void sigint_handler(int signum) {
    // Do nothing, just prevent shell from exiting
}

int main() {
    char command_line[MAX_COMMAND_LINE_LEN];
    char *arguments[MAX_COMMAND_LINE_ARGS];

    // Set up the SIGINT handler
    signal(SIGINT, sigint_handler);

    while (true) {
        printf("%s", prompt);
        fflush(stdout);

        if ((fgets(command_line, MAX_COMMAND_LINE_LEN, stdin) == NULL) && ferror(stdin)) {
            fprintf(stderr, "fgets error");
            exit(EXIT_FAILURE);
        }

        if (feof(stdin)) {
            printf("\n");
            fflush(stdout);
            return 0;
        }

        // Check for command line length
        if (strlen(command_line) >= MAX_COMMAND_LINE_LEN) {
            fprintf(stderr, "Input too long. Please try again.\n");
            continue;
        }

        command_line[strlen(command_line) - 1] = '\0';  // Remove newline

        // Tokenize the command line
        char *token = strtok(command_line, delimiters);
        int arg_count = 0;
        while (token != NULL && arg_count < MAX_COMMAND_LINE_ARGS - 1) {
            arguments[arg_count++] = token;
            token = strtok(NULL, delimiters);
        }
        arguments[arg_count] = NULL;

        if (arguments[0] == NULL) {
            continue;  // Skip empty input
        }

        // Check for background process
        bool background = false;
        if (strcmp(arguments[arg_count - 1], "&") == 0) {
            background = true;
            arguments[arg_count - 1] = NULL;  // Remove '&' from arguments
        }

        // Built-in Commands: env, setenv, cd, echo, exit
        if (strcmp(arguments[0], "env") == 0) {
            char **env = environ;
            while (*env) {
                printf("%s\n", *env);
                env++;
            }
            continue;
        }

        if (strcmp(arguments[0], "setenv") == 0) {
            if (arguments[1] && arguments[2]) {
                setenv(arguments[1], arguments[2], 1);
            } else {
                printf("Usage: setenv VAR VALUE\n");
            }
            continue;
        }

        if (strcmp(arguments[0], "cd") == 0) {
            if (arguments[1] == NULL) {
                printf("Usage: cd [directory]\n");
            } else {
                if (chdir(arguments[1]) != 0) {
                    perror("cd failed");
                }
            }
            continue;
        }

        if (strcmp(arguments[0], "echo") == 0) {
            for (int i = 1; arguments[i] != NULL; i++) {
                if (arguments[i][0] == '$') {
                    char *env_var = getenv(arguments[i] + 1);
                    if (env_var) {
                        printf("%s ", env_var);
                    }
                } else {
                    printf("%s ", arguments[i]);
                }
            }
            printf("\n");
            continue;
        }

        if (strcmp(arguments[0], "exit") == 0) {
            return EXIT_SUCCESS;
        }

        // Forking and executing external commands
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork failed");
            continue;
        }

        if (pid == 0) {  // Child process
            execvp(arguments[0], arguments);
            perror("exec failed");
            exit(EXIT_FAILURE);
        } else {  // Parent process
            if (!background) {
                wait(NULL);
            }
        }
    }
    return EXIT_SUCCESS;  // Use EXIT_SUCCESS for clarity
}
