#ifndef _HISTORY_H_
#define _HISTORY_H_

extern const char *HISTORY_FILE_PATH;

// Represents the state of the history of the shell
typedef struct history
{
    char **lines;
    int max_history;
    int next;
} history_t;

history_t *alloc_history(int max_history);

void add_line_history(history_t *history, const char *cmd_line);

void print_history(history_t *history);

char *find_line_history(history_t *history, int index);

void free_history(history_t *history);

#endif
