#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define SHELL_TOK_BUFSIZE 64
#define SHELL_TOK_DELIM " \t\r\n\a"

int shell_launch(char **args)
{
    pid_t pid;
    pid_t wpid;
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
            wpid = waitpid(pid, &status, WUNTRACED);
        }
        while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1; // Return 1 to continue the shell loop to keep runnning
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
    char *line = NULL;
    size_t bufsize = 0; // getline allocates memory automatically when bufsize is 0
    char **args;

    while (1)
    {
        printf("spook :> ");
        fflush(stdout); // Ensure the prompt prints immediately
        
        // Read line from stdin
        if (getline(&line, &bufsize, stdin) == -1)
        {
            // Handle EOF or error
            if (feof(stdin))
            {
                printf("\n");
                exit(EXIT_SUCCESS);
            }
            else
            {
                perror("getline");
                exit(EXIT_FAILURE);
            }
        }

        // split line into argument array
        args = shell_split_line(line);

        // Debug print to test tokenization
        if (args[0] != NULL)
        {
            printf("Command: %s\n", args[0]);
            for (int i = 1; args[i] != NULL; i++)
            {
                printf("  Arg[%d]: %s\n", i, args[i]);
            }
        }

        // free token array (line buffer free is handled by getline reuse)
    }

    free(line);
}

int main(int argc, char **argv)
{
    // Load config files if any

    // Run command loop
    shell_loop();

    // Perform shutdown/cleanup
    return EXIT_SUCCESS;
}