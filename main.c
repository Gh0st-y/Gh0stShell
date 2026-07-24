#define _GNU_SOURCE
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define SHELL_TOK_BUFSIZE 64
#define SHELL_TOK_DELIM " \t\r\n\a"

// Forward declarations for built-in shell commands
int shell_cd(char **args);
int shell_help(char **args);
int shell_exit(char **args);

// String array of built-in command names
char *builtin_str[] = 
{
    "cd",
    "help",
    "exit"
};

// Array of matching function pointers
int (*builtin_func[]) (char **) =
{
    &shell_cd,
    &shell_help,
    &shell_exit
};

int shell_num_builtins(void)
{
    return sizeof(builtin_str) / sizeof(char *);
}

/* Built-in implementations */

int shell_cd(char **args)
{
    if (args[1] == NULL)
    {
        // If no argument is passed, default to HOME directory
        char *home = getenv("HOME");
        if (home == NULL)
        {
            fprintf(stderr, "spook :> expected argument to \"cd\"\n");
        }
        else 
        {
            if (chdir(home) != 0)
            {
                perror("spook");
            }
        }
    }
    else
    {
        if (chdir(args[1]) != 0)
        {
            perror("spook");
        }
    }
    return 1; // Keep running
}

int shell_help(char **args)
{
    (void)args; // Silence unused variable compiler warnings
    printf("Custom C Unix Shell\n");
    printf("Type program names and arguments, then hit enter.\n");
    printf("The following commands are built in:\n");

    for (int i = 0; i < shell_num_builtins(); i++)
    {
        printf("  %s\n", builtin_str[i]);
    }

    printf("Use the 'man' command for information on other programs.\n");
    return 1;
}

int shell_exit(char **args)
{
    (void)args;
    return 0; // Returning 0 will break our REPL loop and exit!
}

/* Dispatcher function */
int shell_launch(char **args)
{
    pid_t pid;
    int status;

    pid = fork();
    if (pid == 0)
    {
        // --- CHILD PROCESS ---
        // execvp looks up 'args[0]' in PATH and executes it with 'args'
        if (execvp(args[0], args) == -1)
        {
            perror("spook");
        }
        exit(EXIT_FAILURE);
    }
    else if (pid < 0)
    {
        // --- ERROR FORKING ---
        perror("spook :> fork error");
    }
    else
    {
        // --- PARENT PROCESS ---
        // Wait until child process finishes state change
        do 
        {
            waitpid(pid, &status, WUNTRACED);
        }
        while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1; // Return 1 to continue the shell loop to keep runnning
}

int shell_execute(char **args)
{
    if (args[0] == NULL)
    {
        // An empty command was entered (user just hit enter)
        return 1;
    }

    // Check for built-in commands first
    for (int i = 0; i < shell_num_builtins(); i++)
    {
        if (strcmp(args[0], builtin_str[i]) == 0)
        {
            return (*builtin_func[i])(args);
        }
    }

    // If not a built-in, run as an external program
    return shell_launch(args);
}

char *shell_read_line(void)
{
    char *line = NULL;
    size_t bufsize = 0; // getline allocates memory automatically

    if (getline(&line, &bufsize, stdin) == -1)
    {
        if (feof(stdin))
        {
            // EOF (Ctrl+D) reached
            printf("\n");
            exit(EXIT_SUCCESS);
        }
        else
        {
            perror("spook :> read line error");
            exit(EXIT_FAILURE);
        }
    }

    // Remove trailing newline AND carriage return characters
    line[strcspn(line, "\r\n")] = '\0';

    return line;
}

// Split the input line into tokens
char **shell_split_line(char *line)
{
    int bufsize = SHELL_TOK_BUFSIZE;
    int position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token;

    if (!tokens)
    {
        fprintf(stderr, "spook :> allocation error\n");
        exit(EXIT_FAILURE);
    }

    // Extract the first token
    token = strtok(line, SHELL_TOK_DELIM);
    while (token != NULL)
    {
        tokens[position] = token;
        position++;

        // Reallocate buffer if we run out of space
        if (position >= bufsize)
        {
            bufsize += SHELL_TOK_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char*));
            if (!tokens)
            {
                fprintf(stderr, "spook :> allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        // Extract subsequent tokens
        token = strtok(NULL, SHELL_TOK_DELIM);
    }
    // Null-terminate the list of arguments
    tokens[position] = NULL;
    return tokens;
}

// Handles the shells input loop
void shell_loop(void)
{
    char *line;
    char **args;
    int status;

    do
    {
        printf("spook :> ");
        fflush(stdout);

        line = shell_read_line();
        args = shell_split_line(line);
        status = shell_execute(args);

        // Free dynamically allcoated memory for the iteration
        free(line);
        free(args);
    } while (status);
}

int main(int argc, char **argv)
{
    // Load config files if any

    (void)argc;
    (void)argv;

    // Run command loop
    shell_loop();

    // Perform shutdown/cleanup
    return EXIT_SUCCESS;
}