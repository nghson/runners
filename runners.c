#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "runners.h"
#define MAXLENGTH 80 				//maximum length of user input

/* Get the command from user input to the command array */
int get_command(char *command) {
    int i, c;
    for (i = 0; i < MAXLENGTH && (c=getchar()) != '\n' && c != EOF; i++)
		command[i] = c;
    command[i] = '\0';

    //check if valid
    if (command[0] != 'Q' && command[0] != 'L') {  	
		if (strlen(command) <= 2 || command[1] != ' ') { //too short
			return 1;
		}
    } else {
		if (strlen(command) > 1) { 			//too long
			return 1;
		}
    }

    return 0;
}

/* Add a runner to the database (A).
 * Return the new database. */
Runner *add_runner(Runner *list, unsigned long size, char *command) {

    //copy name of runner
    char *i = command + 2; 			//move pointer to name field
    char *p = strstr(i, " "); 			//find the first space separator
    if (!p || *(p+1) == '\0') {
		command[0] = 0;
		return list; 				//invalid command, leave a sign in the command
    }
    Runner *new_list = realloc(list, ((size)+1)*sizeof(Runner));
    new_list[size].name = malloc((p-i+1)*sizeof(char));
    strncpy(new_list[size].name, i, p-i);
    new_list[size].name[p-i] = '\0';

    //copy team of runner
    i = p+1; 					//move to team field
    while (*(++p) != '\0') 			//move to end of command
	;
    new_list[size].team = malloc((p-i+1)*sizeof(char));
    strncpy(new_list[size].team, i, p-i+1);

    //initialize time
    new_list[size].hour = new_list[size].minute = new_list[size].second = 0;
    return new_list;
}

/* Update time to runner (U).
 * Return index of the updated runner. */
unsigned long update_time(Runner *list, unsigned long size, char *command) {
    int hour, minute, second, ret;
    char *i = command+2;

    //move pointer to the first space and check if there are enough arguments
    char *p;
    char *n;
    int space = 0;
    for (n = i; *n != '\0'; n++) {
	if (*n == ' ') {
	    space++;
	    if (space == 1)
		p = n;
	}
    }
    if (space != 3 || *(n-1) == ' ') { 		//invalid command,
		command[0] = 0; 			//leave a sign in the command
		return 1;
    }

    //get name
    char *name = malloc((p-i+1)*sizeof(char));
    strncpy(name, i, p-i);
    name[p-i] = '\0';
    p++;

    //get time
    ret = sscanf(p, "%d %d %d", &hour, &minute, &second);
    if (ret < 3) {
		free(name);
		command[0] = 0;
		return 1;
    }

    //update time
    int done = 0;
    unsigned long j = 0;
    for (j = 0; j < size; j++) {
		if (!strcmp(list[j].name, name)) { 		//name match
			second += list[j].second;
			list[j].second = second%60;
			minute += (list[j].minute + second/60);
			list[j].minute = minute%60;
			list[j].hour += (hour + minute/60);
			done = 1;
			break;
		}
    }

    free(name);
    if (done)
		return j;
    else {  				//runner not found
		command[0] = 0;
		return 1;
    }
}

/* Compare function for qsort */
int compare_time(const void *a, const void *b) {
    const Runner *s = (const Runner *)a; 	//first compare hour, then minute, then second
    const Runner *t = (const Runner *)b;
    if (s->hour != t->hour)
		return s->hour - t->hour;
    if (s->minute != t->minute)
		return s->minute - t->minute;

    return s->second - t->second;
}

/* Print results (L) */
void print_results(const Runner *list, unsigned long size) {
    printf("There are %ld runners.\n", size);
    for (unsigned long i = 0; i < size; i++)
		printf("Runner: %s, team: %s, %d:%d:%d\n", list[i].name, list[i].team, list[i].hour, list[i].minute, list[i].second);
}

/* Get filename for (W) and (O) */
char *get_filename(const char *command) {
    const char *i = command+2;
    const char *p;
    for (p = i; *p != '\0'; p++) { 		//move p to end of string
		if (*p == ' ')
			return NULL; 			//no space in filename
    }
    char *filename = malloc((p-i+1)*sizeof(char));
    strcpy(filename, i);
    return filename;
}

/* Save results (W).
 * Return 0 if succeeded, 1 if failed. */
int save_results(const Runner *list, unsigned long size, const char *command) {
    //get filename
    char *filename = get_filename(command);
    if (!filename)
		return 1;

    //write file
    FILE *f = fopen(filename, "w");
    if (!f) {
		free(filename);
    	return 1;
    }
    for (int j = 0; j < size; j++) {
		int ret = fprintf(f, "%s,%s,%d,%d,%d\n", list[j].name, list[j].team, list[j].hour, list[j].minute, list[j].second);
		if (ret < 0)
			return 1;
    }

    fclose(f);
    free(filename);
    return 0;
}

/* Load results (O).
 * Return the new database loaded from file. */
Runner *load_results(const char *command, unsigned long *size) {
    //get filename
    char *filename = get_filename(command);
    if (!filename)
		return NULL;

    //read file
    FILE *f = fopen(filename, "r");
    if (!f) {
		free(filename);
		return NULL;
    }

    Runner *list = malloc(sizeof(Runner));
    unsigned long i = 0; 				//size of database
    int j = 0, n = 0; 					//counter for string
    char line[81]; 					

    while (fgets(line, 81, f)) {
		list = realloc(list, (i+1)*sizeof(Runner)); 	//increase database
		
		//get name
		while (line[++j] != ',') 			//move j to ,
			;
		list[i].name = malloc((j+1)*sizeof(char));
		strncpy(list[i].name, line, j);
		list[i].name[j] = '\0';
		
		//get team
		n = j+1;
		while (line[++n] != ',')
			;
		list[i].team = malloc((n-j)*sizeof(char));
		strncpy(list[i].team, &line[j+1], n-j-1);
		list[i].team[n-j-1] = '\0';

		//get time
		sscanf(&line[n+1], "%d,%d,%d", &list[i].hour, &list[i].minute, &list[i].second);

		j=0; 						//reset counter
		i++; 						//increase size
    }

    *size = i;
    free(filename);
    fclose(f);
    return list;
}

/* Free the memory on quitting (Q) */
void free_mem(Runner *list, unsigned long size) {
    for (unsigned long i = 0; i < size; i++) {
		free(list[i].name);
		free(list[i].team);
    }
    free(list);
}

/* Execute the command.
 * Return 0 if done, -1 if failed, 1 if quit. */
int execute(Data *data, char *command) {
    unsigned long ret;
    Runner *new_list;
    unsigned long new_size;

    switch(command[0]) {
	case 'A':
	    data->list = add_runner(data->list, data->size, command);
	    if (!command[0]) {
			return -1;
	    }
	    printf("Successfully added runner %s of team %s.\n", data->list[data->size].name, data->list[data->size].team);
	    data->size++;
	    data->sort = 1;
	    return 0;
	
	case 'U':
	    ret = update_time(data->list, data->size, command);
	    if (ret == 1 && !command[0])
			return -1;
	    else {
			printf("Successfully updated runner %s: %d:%d:%d\n", data->list[ret].name, data->list[ret].hour, data->list[ret].minute, data->list[ret].second);
			data->sort = 1;
			return 0;
	    }

	case 'L':
	    if (data->sort) {
			qsort(data->list, data->size, sizeof(Runner), compare_time);
			data->sort = 0;
	    }
	    print_results(data->list, data->size);
	    return 0;

	case 'W':
	    if (save_results(data->list, data->size, command)) {
			printf("Failed to save results.\n");
			return -1;
	    } else {
			printf("Saved succesfully.\n");
			return 0;
	    }

	case 'O':
	    new_list = load_results(command, &new_size);
	    if (!new_list) {
			printf("Failed to load results.\n");
			return -1;
	    } else {
			free_mem(data->list, data->size);
			data->size = new_size;
			data->list = new_list;
			data->sort = 1;
			printf("Loaded successfully.\n");	
			return 0;
	    }

	case 'Q':
	    free_mem(data->list, data->size);
	    printf("Goodbye.\n");
	    return 1;

	default:
	    return -1;
    }

}

int main() {
    //initialize the database
    Data data;
    data.size = 0;
    data.list = malloc(sizeof(Runner));
    data.sort = 0;
    int end = 0;
    char command[MAXLENGTH+1]; 				

    while (end != 1) {
	if (get_command(command) || (end = execute(&data, command)) == -1)
	    printf("Invalid command.\n");
    }

    return 0;
}

