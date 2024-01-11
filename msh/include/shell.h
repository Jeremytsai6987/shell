#ifndef SHELL_H
#define SHELL_H

#include <stdbool.h>
#include "job.h"
#include "history.h"

// Default values for the shell parameters
extern const int MAX_LINE;
extern const int MAX_JOBS;
extern const int MAX_HISTORY;
// Declare jobs as a global variable
extern job_t *jobs;

// Represents the state of the shell
typedef struct msh
{
    int max_jobs;
    int max_line;
    int max_history;
    history_t *history;
    job_t *jobs;
    // Additional fields will be added here as necessary
} msh_t;

// Function prototypes
msh_t *alloc_shell(int max_jobs, int max_line, int max_history);
char *parse_tok(char *line, int *job_type);
char **separate_args(char *line, int *argc, bool *is_builtin);
int evaluate(msh_t *shell, char *line);
void exit_shell(msh_t *shell);
char *builtin_cmd(msh_t *shell, char **argv);
bool is_builtin_command(const char *command, const char *builtin_commands[], int num_builtin_commands);
job_t *find_job_by_id(job_t *jobs, int max_jobs, int id);

#endif // SHELL_H
