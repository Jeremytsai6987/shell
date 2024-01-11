#ifndef _JOB_H_
#define _JOB_H_

#include <sys/types.h>
#include <stdbool.h>

typedef enum job_state
{
    FOREGROUND,
    BACKGROUND,
    SUSPENDED,
    UNDEFINED,
    TERMINATED,
    STOPPED
} job_state_t;

// Represents a job in a shell.
typedef struct job
{
    char *cmd_line;    // The command line for this specific job.
    job_state_t state; // The current state for this job
    pid_t pid;         // The process id for this job
    int jid;           // The job number for this job
} job_t;

// Function to add a new job
bool add_job(job_t *jobs, int max_jobs, pid_t pid, job_state_t state, const char *cmd_line);

// Function to delete a job
bool delete_job(job_t *jobs, pid_t pid);

// Function to free up job array
void free_jobs(job_t *jobs, int max_jobs);

job_t *get_job_by_id(job_t *jobs, int max_jobs, pid_t pid);

void set_job_stopped(job_t *jobs, int max_jobs, pid_t pid);

pid_t get_foreground_job_pid(job_t *jobs, int max_jobs);

#endif
