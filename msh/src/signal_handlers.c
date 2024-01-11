#include "signal_handlers.h"
#include "job.h"   // Assuming job.h contains job_t structure and necessary declarations
#include "shell.h" // Assuming job.h contains job_t structure and necessary declarations
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <stdbool.h>

// Assuming these are defined elsewhere

// Handler for SIGCHLD
void sigchld_handler(int sig)
{
    pid_t pid;
    int status;

    while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED)) > 0)
    {
        if (WIFSTOPPED(status))
        {
            update_job_status(jobs, pid, STOPPED); // Update to STOPPED state
        }
        else if (WIFEXITED(status) || WIFSIGNALED(status))
        {
            delete_job(jobs, pid); // Remove job from list
        }
    }
}

// Handler for SIGINT
void sigint_handler(int sig)
{
    pid_t fg_pid = get_foreground_job_pid(jobs, MAX_JOBS);
    if (fg_pid > 0)
    {
        kill(-fg_pid, SIGINT); // Send SIGINT to the foreground group
    }
}

// Handler for SIGTSTP
void sigtstp_handler(int sig)
{
    pid_t fg_pid = get_foreground_job_pid(jobs, MAX_JOBS);
    if (fg_pid > 0)
    {
        kill(-fg_pid, SIGTSTP); // Send SIGTSTP to the foreground group
    }
}

typedef void handler_t(int);
handler_t *setup_handler(int signum, handler_t *handler)
{
    struct sigaction action, old_action;

    action.sa_handler = handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = SA_RESTART;

    if (sigaction(signum, &action, &old_action) < 0)
    {
        perror("Signal error");
        exit(1);
    }
    return (old_action.sa_handler);
}

// Wait for foreground job to finish
void waitfg(pid_t pid)
{
    while (pid_is_fg(pid))
    {
        sleep(1);
    }
}

// Initialize signal handlers
void initialize_signal_handlers()
{

    setup_handler(SIGINT, sigint_handler);   // Ctrl-C
    setup_handler(SIGTSTP, sigtstp_handler); // Ctrl-Z
    setup_handler(SIGCHLD, sigchld_handler); // Child process status change
}

// Check if a job with given PID is in foreground
bool pid_is_fg(pid_t pid)
{
    for (int i = 0; i < MAX_JOBS; i++)
    {
        if (jobs[i].pid == pid && jobs[i].state == FOREGROUND)
        {
            return true;
        }
    }
    return false;
}

// Update the job status in the job list
void update_job_status(job_t *jobs, pid_t pid, job_state_t state)
{
    for (int i = 0; i < MAX_JOBS; i++)
    {
        if (jobs[i].pid == pid)
        {
            jobs[i].state = state;
            break;
        }
    }
}
