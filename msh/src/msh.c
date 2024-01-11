#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

int main(int argc, char *argv[])
{
    int max_jobs = 32, max_line = 1024, max_history = 100;
    int option;

    // Parse command-line arguments
    while ((option = getopt(argc, argv, "s:j:l:")) != -1)
    {
        switch (option)
        {
        case 's':
            max_history = atoi(optarg);
            if (max_history <= 0)
            {
                fprintf(stderr, "usage: msh [-s NUMBER] [-j NUMBER] [-l NUMBER]\n");
                return 1;
            }
            break;
        case 'j':
            max_jobs = atoi(optarg);
            if (max_jobs <= 0)
            {
                fprintf(stderr, "usage: msh [-s NUMBER] [-j NUMBER] [-l NUMBER]\n");
                return 1;
            }
            break;
        case 'l':
            max_line = atoi(optarg);
            if (max_line <= 0)
            {
                fprintf(stderr, "usage: msh [-s NUMBER] [-j NUMBER] [-l NUMBER]\n");
                return 1;
            }
            break;
        default:
            fprintf(stderr, "usage: msh [-s NUMBER] [-j NUMBER] [-l NUMBER]\n");
            return 1;
        }
    }

    if (optind < argc)
    {
        // Non-option argument is found
        fprintf(stderr, "usage: msh [-s NUMBER] [-j NUMBER] [-l NUMBER]\n");
        return 1;
    }

    // Allocate and initialize msh state
    msh_t *shell = alloc_shell(max_jobs, max_line, max_history);
    if (!shell)
    {
        fprintf(stderr, "Failed to allocate msh state\n");
        return 1;
    }

    char *line = NULL;
    size_t linecap = 0;
    ssize_t linelen;

    // REPL loop
    while (true)
    {
        printf("msh> ");
        fflush(stdout);

        linelen = getline(&line, &linecap, stdin);
        if (linelen <= 0)
        {
            break;
        }

        if (line[linelen - 1] == '\n')
        {
            line[linelen - 1] = '\0';
        }

        int result = evaluate(shell, line);
        free(line);
        line = NULL;

        if (result != 0)
        {
            // Exit if evaluate signals to do so
            break;
        }
    }

    exit_shell(shell);
    return 0;
}
