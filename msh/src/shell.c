
#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "job.h"
#include "signal_handlers.h"
#include "history.h"
#include <sys/wait.h>
#include <errno.h>
#include <unistd.h>
#include <stdbool.h>
const int MAX_LINE = 1024;
const int MAX_JOBS = 32;
const int MAX_HISTORY = 100;
const char *builtin_commands[] = {
    "jobs",
    "history",
    "bg",
    "fg",
    "kill",
    "exit"};
int num_builtin_commands = sizeof(builtin_commands) / sizeof(builtin_commands[0]);

// Define the jobs array
// job_t jobs[MAX_JOBS] = {0};
job_t *jobs;

msh_t *alloc_shell(int max_jobs, int max_line, int max_history)
{
    msh_t *shell = malloc(sizeof(msh_t));
    if (!shell)
        return NULL;

    shell->max_jobs = max_jobs > 0 ? max_jobs : MAX_JOBS;
    shell->max_line = max_line > 0 ? max_line : MAX_LINE;
    shell->max_history = max_history > 0 ? max_history : MAX_HISTORY;

    shell->jobs = malloc(shell->max_jobs * sizeof(job_t));
    if (!shell->jobs)
    {
        free(shell);
        return NULL;
    }

    for (int i = 0; i < shell->max_jobs; i++)
    {
        shell->jobs[i].pid = 0;
        shell->jobs[i].state = FOREGROUND;
        shell->jobs[i].cmd_line = NULL;
        shell->jobs[i].jid = 0;
    }
    shell->history = alloc_history(shell->max_history);

    return shell;
}

// Function to parse tokens and determine job type
char *parse_tok(char *line, int *job_type)
{
    static char *next_token = NULL;
    char *current;
    const char *delims = "&;";

    // If line is not NULL, we start parsing a new command line.
    if (line != NULL)
    {
        next_token = line;
    }

    // If next_token is NULL, we've reached the end of the input string.
    if (next_token == NULL)
    {
        if (job_type != NULL)
            *job_type = -1;
        return NULL;
    }

    // Skip any leading delimiters to get to the start of the next job.
    while (*next_token && strchr(delims, *next_token))
    {
        if (job_type != NULL)
            *job_type = *next_token == '&' ? 0 : 1;
        next_token++;
    }

    // If we've reached the end of the string, return NULL.
    if (*next_token == '\0')
    {
        next_token = NULL;
        if (job_type != NULL)
            *job_type = -1;
        return NULL;
    }

    // Mark the start of the current job.
    current = next_token;

    // Find the end of the current job.
    while (*next_token && !strchr(delims, *next_token))
    {
        next_token++;
    }

    // If we're at a delimiter, set job type, put a null terminator, and move to the next character.
    if (*next_token)
    {
        if (job_type != NULL)
            *job_type = *next_token == '&' ? 0 : 1;
        *next_token = '\0';
        next_token++;
    }
    else
    {
        // We're at the end of the string, so we'll need to return NULL next time.
        if (job_type != NULL)
            *job_type = 1; // Default to foreground for last job if no delimiter is present.
    }

    return current;
}

char **separate_args(char *line, int *argc, bool *is_builtin)
{
    if (line == NULL || argc == NULL)
    {
        return NULL;
    }

    // First, count the number of arguments
    int count = 0;
    const char *tmp = line;
    while (*tmp)
    {
        while (*tmp == ' ' && *tmp)
        {
            tmp++;
        }
        if (*tmp == '\0')
        {
            break;
        }
        count++;
        while (*tmp != ' ' && *tmp)
        {
            tmp++;
        }
    }

    // Allocate memory for the argument array with an extra
    // slot for the NULL terminator
    char **argv = malloc((count + 1) * sizeof(char *));
    if (!argv)
    {
        perror("malloc");
        return NULL;
    }

    int i = 0;
    const char delim[] = " \t\r\n\v\f"; // Whitespace characters
    char *token = strtok(line, delim);
    while (token != NULL)
    {
        argv[i++] = strdup(token);
        if (!argv[i - 1])
        {
            // Handle allocation failure: clean up and return
            while (i-- > 0)
                free(argv[i]);
            free(argv);
            return NULL;
        }
        token = strtok(NULL, delim);
    }

    argv[i] = NULL; // Null-terminate the argument array
    *argc = count;

    return argv;
}

bool is_builtin_command(const char *command, const char *builtin_commands[], int num_builtin_commands)
{
    for (int i = 0; i < num_builtin_commands; i++)
    {
        if (strcmp(command, builtin_commands[i]) == 0)
        {
            return true;
        }
    }
    return false;
}

char *builtin_cmd(msh_t *shell, char **argv)
{
    if (strcmp(argv[0], "jobs") == 0)
    {
        // Handle the "jobs" command
        for (int i = 0; i < shell->max_jobs; i++)
        {
            job_t *job = &shell->jobs[i];
            if (job->pid != 0)
            {
                char *state_str = (job->state == BACKGROUND) ? "RUNNING" : "Stopped";
                printf("[%d] %d %s %s\n", job->jid, job->pid, state_str, job->cmd_line);
            }
        }
        return NULL; // No additional action needed
    }
    else if (strcmp(argv[0], "history") == 0)
    {
        // Handle the "history" command
        print_history(shell->history);
        return NULL; // No additional action needed
    }
    else if (argv[0][0] == '!')
    {
        // Handle the "!N" command (history expansion)
        printf("error: history expansion not supported\n");
        return NULL;
    }
    else if (strcmp(argv[0], "bg") == 0 || strcmp(argv[0], "fg") == 0)
    {
        // Handle the "bg" and "fg" commands
        int is_bg = (strcmp(argv[0], "bg") == 0);
        char *job_id_arg = argv[1];
        int job_id;

        // Check if the argument is a PID or a JOB_ID (specified as %JOB_ID)
        if (job_id_arg[0] == '%')
        {
            // Extract JOB_ID (skip the % character)
            job_id = atoi(&job_id_arg[1]);
        }
        else
        {
            // Argument is a PID
            job_id = atoi(job_id_arg);
        }

        // Find the job with the specified JOB_ID or PID
        job_t *job = find_job_by_id(shell->jobs, shell->max_jobs, job_id);
        if (job != NULL)
        {
            // Send a SIGCONT signal to the target job to restart it
            kill(-job->pid, SIGCONT);

            if (is_bg)
            {
                // If the command is "bg", run the target job in the background
                job->state = BACKGROUND;
                printf("[%d] %d %s %s\n", job->jid, job->pid, "RUNNING", job->cmd_line);
            }
            else
            {
                // If the command is "fg", wait for the target job to complete in the foreground
                job->state = FOREGROUND;
                waitfg(job->pid);
            }
        }
        else
        {
            printf("error: job not found\n");
        }

        return NULL;
    }
    else if (strcmp(argv[0], "kill") == 0)
    {
        // Handle the "kill" command
        printf("error: kill command not implemented\n");
        return NULL;
    }
    else if (strcmp(argv[0], "exit") == 0)
    {
        // Handle the "exit" command
        exit_shell(shell);
        exit(0);
    }

    return NULL; // Return NULL for non-built-in commands
}

// Function to find a job by its JOB_ID or PID
job_t *find_job_by_id(job_t *jobs, int max_jobs, int id)
{
    for (int i = 0; i < max_jobs; i++)
    {
        job_t *job = &jobs[i];
        if (job->pid == id || job->jid == id)
        {
            return job;
        }
    }
    return NULL; // Job not found
}

int evaluate(msh_t *shell, char *line)
{
    int type;
    char *job;
    char **argv;
    int argc;
    bool is_builtin;

    if (strlen(line) > shell->max_line)
    {
        printf("error: reached the maximum line limit.\n");
        return 0;
    }
    add_line_history(shell->history, line);

    // Block SIGCHLD signals
    sigset_t mask, prev_mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGCHLD);
    sigprocmask(SIG_BLOCK, &mask, &prev_mask);
    printf("Debug: Evaluating command: %s\n", line);

    job = parse_tok(line, &type);
    while (job != NULL)
    {
        argv = separate_args(job, &argc, &is_builtin);
        if (argv != NULL)
        {
            if (strcmp(job, "exit") == 0)
            {
                return 1;
            }
            is_builtin = is_builtin_command(argv[0], builtin_commands, num_builtin_commands); // Check if it's a built-in command

            if (is_builtin) // Check if it's a built-in command
            {
                char *result = builtin_cmd(shell, argv);
                if (result != NULL)
                {
                    // If the built-in command returns a non-null value, use it as the command line
                    strcpy(line, result);
                }
            }
            else
            {
                pid_t pid = fork();
                if (pid == 0)
                {
                    // Child process
                    setpgid(0, 0);                         // Set child in new process group
                    sigprocmask(SIG_UNBLOCK, &mask, NULL); // Unblock SIGCHLD
                    if (execve(argv[0], argv, NULL) == -1)
                    {
                        perror("execve");
                        exit(EXIT_FAILURE);
                    }
                }
                else if (pid < 0)
                {
                    perror("fork");
                }
                else
                {
                    // Parent process
                    add_job(shell->jobs, shell->max_jobs, pid, type == 0 ? BACKGROUND : FOREGROUND, job);
                    if (type == 1)
                    { // If foreground
                      // waitfg(pid);
                    }
                    sigprocmask(SIG_SETMASK, &prev_mask, NULL); // Unblock SIGCHLD
                }
            }

            // Free allocated memory
            for (int i = 0; i < argc; i++)
            {
                free(argv[i]);
            }
            free(argv);
        }
        job = parse_tok(NULL, &type);
    }

    return 0;
}

// Function to clean up and exit the shell/
// void exit_shell(msh_t *shell)
//{
//    if (shell)
//    {
//      free_jobs(shell->jobs, shell->max_jobs);
//      free(shell);
//   }
//}

void exit_shell(msh_t *shell)
{
    if (shell)
    {
        // Wait for all background jobs to complete
        for (int i = 0; i < shell->max_jobs; i++)
        {
            if (shell->jobs[i].pid != 0 && shell->jobs[i].state == BACKGROUND)
            {
                int status;
                waitpid(shell->jobs[i].pid, &status, 0);
            }
        }

        free_jobs(shell->jobs, shell->max_jobs);
        free(shell);
    }
}
