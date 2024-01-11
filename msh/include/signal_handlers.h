#ifndef SIGNAL_HANDLERS_H
#define SIGNAL_HANDLERS_H

#include <sys/types.h>
#include "job.h"
#include "shell.h"
#include <stdbool.h>

// Prototypes for signal handling functions
void initialize_signal_handlers();
void sigchld_handler(int sig);
void sigint_handler(int sig);
void sigtstp_handler(int sig);
void waitfg(pid_t pid);
// Function prototypes
bool pid_is_fg(pid_t pid);
void update_job_status(job_t *jobs, pid_t pid, job_state_t state);

#endif // SIGNAL_HANDLERS_H
