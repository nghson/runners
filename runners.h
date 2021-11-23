typedef struct {
    char *name;
    char *team;
    int hour, minute, second;
} Runner;

typedef struct {
    unsigned long size;
    Runner *list;
    int sort;
} Data;

int get_command(char *command);

Runner *add_runner(Runner *list, unsigned long size, char *command);

unsigned long update_time(Runner *list, unsigned long size, char *command);

int compare_time(const void *a, const void *b);

void print_results(const Runner *list, unsigned long size);

char *get_filename(const char *command);

int save_results(const Runner *list, unsigned long size, const char *command);

Runner *load_results(const char *command, unsigned long *size);

void free_mem(Runner *list, unsigned long size);

int execute(Data *data, char *command);
