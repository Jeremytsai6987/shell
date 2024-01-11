#include "history.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *HISTORY_FILE_PATH = "../data/.msh_history";

history_t *alloc_history(int max_history)
{
    history_t *history = malloc(sizeof(history_t));
    if (!history)
        return NULL;

    history->lines = malloc(sizeof(char *) * max_history);
    if (!history->lines)
    {
        free(history);
        return NULL;
    }

    for (int i = 0; i < max_history; i++)
    {
        history->lines[i] = NULL;
    }

    history->max_history = max_history;
    history->next = 0;

    // Load history from file
    FILE *file = fopen(HISTORY_FILE_PATH, "r");
    if (file)
    {
        char line[1024]; // Adjust size as needed
        while (fgets(line, sizeof(line), file) && history->next < max_history)
        {
            line[strcspn(line, "\n")] = 0; // Remove newline character
            history->lines[history->next++] = strdup(line);
        }
        fclose(file);
    }

    return history;
}

void add_line_history(history_t *history, const char *cmd_line)
{
    if (!cmd_line || strcmp(cmd_line, "") == 0 || strcmp(cmd_line, "exit") == 0)
    {
        return;
    }

    if (history->next == history->max_history)
    {
        free(history->lines[0]);
        memmove(history->lines, history->lines + 1, sizeof(char *) * (history->max_history - 1));
        char *new_line = strdup(cmd_line);
        if (!new_line)
        {
            perror("strdup");
            return; // Handle memory allocation failure
        }
        history->lines[history->max_history - 1] = new_line;
    }
    else
    {
        char *new_line = strdup(cmd_line);
        if (!new_line)
        {
            perror("strdup");
            return; // Handle memory allocation failure
        }
        history->lines[history->next++] = new_line;
    }
}

void print_history(history_t *history)
{
    for (int i = 1; i <= history->next; i++)
    {
        printf("%5d\t%s\n", i, history->lines[i - 1]);
    }
}

char *find_line_history(history_t *history, int index)
{
    if (index < 1 || index > history->next)
    {
        return NULL;
    }
    return history->lines[index - 1];
}

void free_history(history_t *history)
{
    FILE *file = fopen(HISTORY_FILE_PATH, "w");
    if (file)
    {
        for (int i = 0; i < history->next; i++)
        {
            fprintf(file, "%s\n", history->lines[i]);
            free(history->lines[i]);
        }
        fclose(file);
    }

    free(history->lines);
    free(history);
}
