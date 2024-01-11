#include "job.h"
#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*
 *add_job: Adds a new job to the jobs array.
 *
 *jobs: Array of job_t structures.
 *max_jobs: Maximum number of jobs that can be managed.
 *pid: Process ID of the new job.
 *state: Current state of the new job (FOREGROUND, BACKGROUND, etc.).
 *cmd_line: Command line string for the new job.
 *
 *Returns: true if the job is successfully added, false if the array is full.
 */
bool add_job(job_t *jobs, int max_jobs, pid_t pid, job_state_t state, const char *cmd_line)
{
    for (int i = 0; i < max_jobs; i++)
    {
        if (jobs[i].pid == 0)
        {
            jobs[i].pid = pid;
            jobs[i].state = state;
            jobs[i].cmd_line = strdup(cmd_line);
            if (!jobs[i].cmd_line)
            {
                fprintf(stderr, "Failed to allocate memory for cmd_line\n");
                return false;
            }
            jobs[i].jid = i + 1;
            return true;
        }
    }
    return false;
}

/*
 *delete_job: Removes a job from the jobs array based on its PID.
 *
 *jobs: Array of job_t structures.
 *pid: Process ID of the job to be removed.
 *
 *Returns: true if the job is found and removed, false otherwise.
 */
bool delete_job(job_t *jobs, pid_t pid)
{
    for (int i = 0; i < MAX_JOBS; i++)
    {
        if (jobs[i].pid == pid)
        {
            free(jobs[i].cmd_line);
            jobs[i].cmd_line = NULL;
            jobs[i].pid = 0;
            jobs[i].state = UNDEFINED;
            jobs[i].jid = 0;
            return true;
        }
    }
    return false;
}

/*
 *free_jobs: Frees the memory allocated for the jobs array.
 *
 *jobs: Array of job_t structures to be freed.
 *max_jobs: Number of elements in the jobs array.
 *
 */
void free_jobs(job_t *jobs, int max_jobs)
{
    for (int i = 0; i < max_jobs; i++)
    {
        free(jobs[i].cmd_line);
    }
    free(jobs);
}

pid_t get_foreground_job_pid(job_t *jobs, int max_jobs)
{
    for (int i = 0; i < max_jobs; i++)
    {
        if (jobs[i].pid != 0 && jobs[i].state == FOREGROUND)
        {
            return jobs[i].pid;
        }
    }
    return -1; // No foreground job found
}
