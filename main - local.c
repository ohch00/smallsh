/*
Name: Christina Oh
Date: 10/22/2021
Project: Assignment 3
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>

// struct for processing user input
struct user_input
{
	char* command;
	char* args[514];
	char* input_file;
	char* output_file;
	char* ampersand;
	struct user_input* next;
};

struct user_input* createInput(char* input) {
	struct user_input* currInput = malloc(sizeof(struct user_input));

	char* saveptr;

	// First token is command
	char* token = strtok_r(input, " ", &saveptr);
	currInput->command = calloc(strlen(token) + 1, sizeof(char));
	strcpy(currInput->command, token);

	//char args_array[512][500];
	int counter = 0;
	int conversion;
	char ampersand = '&';
	char input_symbol = '<';
	char output_symbol = '>';
	bool symbol = false;

	currInput->args[counter] = token;
	counter += 1;

	// put arguments into list
	while (token != NULL && !symbol) {
		conversion = saveptr[0];
		token = strtok_r(NULL, " ", &saveptr);

		// if input or output symbol is encountered, arguments have ended
		if (conversion == input_symbol || conversion == output_symbol) {
			symbol = true;
			break;
		}

		// if & symbol is encountered and next token is NULL, arguments have ended
		if (conversion == ampersand || token == NULL) {
			symbol = true;
			break;
		}

		currInput->args[counter] = token;
		counter += 1;

	}


	while (token != NULL) {
		conversion = saveptr[0];

		// Input_file - optional
		if (conversion == input_symbol) {
			token = strtok_r(NULL, " ", &saveptr);
			currInput->input_file = calloc(strlen(token) + 1, sizeof(char));
			strcpy(currInput->input_file, token);
		}

		// Output_file - optional
		else if (conversion == output_symbol) {
			token = strtok_r(NULL, " ", &saveptr);
			currInput->output_file = calloc(strlen(token) + 1, sizeof(char));
			strcpy(currInput->output_file, token);
		}
		// Ampersand - optional
		else if (conversion == ampersand && token == NULL) {
			token = strtok_r(NULL, " ", &saveptr);
			currInput->ampersand = calloc(strlen(token) + 1, sizeof(char));
			strcpy(currInput->ampersand, token);
		}
	}

	currInput->next = NULL;

	return currInput;

}


// Process user input
struct user_input* process_user_input(char* user_input) {
	char copy_user_input[2049];
	strcpy(copy_user_input, user_input);

	// remove /n character at input
	copy_user_input[strlen(copy_user_input) - 1] = 0;

	// create input struct
	struct user_input* head = NULL;
	struct user_input* tail = NULL;

	struct user_input* newNode = createInput(copy_user_input);
	if (head == NULL) {
		head = newNode;
		tail = newNode;
	}
	else {
		tail->next = newNode;
		tail = newNode;
	}

	return head;

}


// $$ Expansion
void variable_expansion(char* user_input, char* expanded_var) {
	char* copy_user_input = calloc(strlen(user_input) + 1, sizeof(char));
	strcpy(copy_user_input, user_input);
	char var[] = "$$";

	copy_user_input[strlen(copy_user_input) - 1] = 0;

	// process pid - count digits in pid
	int get_pid = getpid();
	char digits[10];
	sprintf(digits, "%d", get_pid);
	int length;
	length = strlen(digits);

	// calculate size of char needed for expanded_var
	int size_expanded_var = (length * strlen(copy_user_input) + 1);
	char hold_expanded_var[size_expanded_var];
	hold_expanded_var[0] = '\0';

	// https://www.linuxquestions.org/questions/programming-9/replace-a-substring-with-another-string-in-c-170076/
	bool replacing = true;
	char* i;

	i = strstr(copy_user_input, var);

	if (i) {
		while (replacing) {
			i = strstr(copy_user_input, var);
			if (i) {
				strncpy(hold_expanded_var, copy_user_input, i - copy_user_input);
				hold_expanded_var[i - copy_user_input] = '\0';
				sprintf(hold_expanded_var + (i - copy_user_input), "%s%s", digits, i + strlen(var));
				strcpy(copy_user_input, hold_expanded_var);
			}
			else {
				replacing = false;
				break;
			}
		}
		strcpy(expanded_var, hold_expanded_var);
	}


}

struct background_process
{
	int pid;
	struct background_process* next;
};

struct background_process* add_process(pid_t spawnPid) {
	struct background_process* currBack = malloc(sizeof(struct background_process));
	currBack->pid = spawnPid;
	currBack->next = NULL;
	return currBack;
}



void direction(struct user_input* input, bool* continue_sh) {
	struct user_input* input_2 = input;
	char* cmd = calloc(strlen(input->command) + 1, sizeof(char));
	strcpy(cmd, input_2->command);

	bool child_processed = false;
	int exit_status = -5;

	char dest_array[512][200];
	int arg_counter = 0;

	char* home;
	char* filename;
	bool has_arg = false;
	if (input_2->args[1]) {
		filename = calloc(strlen(input->args[1]) + 1, sizeof(char));
		strcpy(filename, input_2->args[1]);
		has_arg = true;

		while (input_2->args[arg_counter] != NULL) {
			strcpy(dest_array[arg_counter], input->args[arg_counter]);
			arg_counter = arg_counter + 1;
		}

	}

	bool background = false;
	if (input_2->ampersand) {
		background = true;
	}

	if (strcmp(cmd, "cd") == 0) {
		if (has_arg) {
			chdir(filename);
		}
		else {
			home = getenv("HOME");
			chdir(home);
		}
	}

	else if (strcmp(cmd, "exit") == 0) {
		*continue_sh = true;
		exit(0);
	}

	else if (strcmp(cmd, "status") == 0) {
		if (child_processed) {
			printf("exit value %d", exit_status);
		}
		else {
			printf("exit value 0");
			fflush(stdout);
		}
	}

	else if (background) {
		background_commands(input_2);

	}

	else {
		foreground_commands(input_2->args, &exit_status);
		child_processed = true;
	}

}

void foreground_commands(char** args, int exit_status) {
	// https://www.youtube.com/watch?v=1R9h-H2UnLs
	pid_t spawnPid = -5;
	int childExitStatus = -5;
	int child_status = -5;
	int child_signal = -5;

	int fork_counter = 0;

	spawnPid = fork();
	int pid = getpid();

	fork_counter = fork_counter + 1;

	if (fork_counter > 25) {
		abort();
	}
	switch (spawnPid) {
	case -1: {
		perror("Error Occurred.\n");
		exit(1);
		fork_counter = fork_counter + 1;
		if (fork_counter > 25) {
			abort();
		}

		break;
	}
	case 0: {
		fork_counter = fork_counter + 1;
		if (fork_counter > 25) {
			abort();
		}
		execvp(args[0], args);
		break;
	}
	default: {
		fork_counter = fork_counter + 1;
		if (fork_counter > 25) {
			abort();
		}
		pid_t actualPid = waitpid(spawnPid, &childExitStatus, 0);
		if (WIFEXITED(childExitStatus)) {
			child_status = WEXITSTATUS(childExitStatus);
		}
		else {
			child_signal = WTERMSIG(childExitStatus);
		}
		if (child_status != -5) {
			exit_status = child_status;
		}
		else if (child_signal != -5) {
			exit_status = child_signal;
		}
		break;
	}
	}
}

void background_commands(char** args) {

	// https://www.youtube.com/watch?v=1R9h-H2UnLs
	pid_t spawnPid = -5;
	int childExitStatus = -5;
	int fork_counter = 0;

	spawnPid = fork();
	fork_counter = fork_counter + 1;

	struct background_process* head = NULL;
	struct background_process* tail = NULL;
	struct background_process* newNode = add_process(spawnPid);
	if (head == NULL) {
		head = newNode;
		tail = newNode;
	}
	else {
		tail->next = newNode;
		tail = newNode;
	}


	if (fork_counter > 25) {
		abort();
	}

	int child_status = -5;
	int child_signal = -5;
	switch (spawnPid) {
	case -1: {
		perror("Error Occurred.\n");
		exit(1);
		fork_counter = fork_counter + 1;
		if (fork_counter > 25) {
			abort();
		}
		break;
	}
	case 0: {
		fork_counter = fork_counter + 1;
		if (fork_counter > 25) {
			abort();
		}
		execvp(args[0], args);
		break;
	}
	default: {
		fork_counter = fork_counter + 1;
		if (fork_counter > 25) {
			abort();
		}
		spawnPid = waitpid(spawnPid, &childExitStatus, 0);
		if (WIFEXITED(childExitStatus)) {
			child_status = WEXITSTATUS(childExitStatus);
		}
		else {
			child_signal = WTERMSIG(childExitStatus);
		}
		break;
	}
	}
}


int main() {
	bool continue_sh = true;
	char user_input[2049];
	char pound_sign = '#';

	//
	struct user_input* input;

	// Variable expansion
	char* expanded_var[2049];


	printf("$ smallsh\n");
	// Prompt
	while (continue_sh) {
		printf("\n: ");
		fflush(stdout);
		// if fgets is not null
		if (strlen(fgets(user_input, 2049, stdin)) > 1) {

			// if fgets is not a comment (pound sign)
			if (user_input[0] != pound_sign) {
				variable_expansion(user_input, expanded_var);
				input = process_user_input(user_input);
				direction(input, &continue_sh);
			}
		}
		// if fgets is null
		else {
			continue;
		}
	}
	return 0;
}