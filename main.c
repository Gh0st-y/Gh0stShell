#include <stdio.h>
#include <stdlib.h>

void shell_loop(void)
{
    char *line = NULL;
    size_t bufsize = 0; // getline allocates memory automatically when bufsize is 0

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
                break; // Exit loop on EOF
            }
            else
            {
                perror("getline");
                break; // Exit loop on error
            }
        }

        // For now, just echo what was entered
        printf("You typed: %s", line);
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