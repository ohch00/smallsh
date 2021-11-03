/*
Name: Christina Oh
Date: 11/3/2021
Project: Assignment 3
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>
#include <fcntl.h>
#include <signal.h>

// Global booleans
bool is_sigtstp = false;
bool is_var_expansion = false;

// Citation for creating structs and processing user input into linked list
// Date Accessed: 10/20/2021
// Code adapted from Assignment 1: Students struct processing example
// Struct for processing user input
struct user_input
{
	char* command;
	char* args[514];
	char* input_file;
	char* output_file;
	bool ampersand;
	struct user_input* next;
};

struct user_input* createInput(char* input) {
	struct user_input* currInput = malloc(sizeof(struct user_input));

	currInput->ampersand = false;

	// Copy for second pointer
	// The second pointer will check if the next pointer after the first one is null
	// Used for ampersand processing
	char* saveptr_2;
	char* copy_input = calloc(strlen(input) + 1, sizeof(char));
	strcpy(copy_input, input);
	char* token_2 = strtok_r(copy_input, " ", &saveptr_2);

	// First token is command
	char* saveptr;
	char* token = strtok_r(input, " ", &saveptr);
	currInput->command = calloc(strlen(token) + 1, sizeof(char));
	strcpy(currInput->command, token);

	// Setting up char comparison variables
	int counter = 0;
	int conversion = 0;
	char ampersand = '&';
	char input_symbol = '<';
	char output_symbol = '>';
	bool symbol = false;

	// Put first token into argument list
	currInput->args[counter] = token;
	counter += 1;

	// Update second pointer
	token_2 = strtok_r(NULL, " ", &saveptr_2);

	// Put arguments into list
	while (token != NULL && !symbol) {
		token = strtok_r(NULL, " ", &saveptr);
		if (token != NULL) {
			conversion = token[0];
		}
		token_2 = strtok_r(NULL, " ", &saveptr_2);


		// if input or output symbol is encountered, arguments have ended
		if (conversion == input_symbol || conversion == output_symbol) {
			symbol = true;
			break;
		}

		// if & symbol is encountered and next token is NULL, arguments have ended
		if (conversion == ampersand && token_2 == NULL) {
			token = strtok_r(NULL, " ", &saveptr);
			symbol = true;
			currInput->ampersand = true;
			break;
		}

		// Otherwise, keep adding arguments into list
		currInput->args[counter] = token;
		counter += 1;
	}

	// Start adding input/output files and background process indicator (&) to struct if applicable
	while (token != NULL) {
		if (token != NULL) {
			conversion = token[0];
		}

		// Input_file - optional
		if (conversion == input_symbol) {
			token = strtok_r(NULL, " ", &saveptr);
			token_2 = strtok_r(NULL, " ", &saveptr_2);
			currInput->input_file = calloc(strlen(token) + 1, sizeof(char));
			strcpy(currInput->input_file, token);
		}

		// Output_file - optional
		else if (conversion == output_symbol) {
			token = strtok_r(NULL, " ", &saveptr);
			token_2 = strtok_r(NULL, " ", &saveptr_2);
			currInput->output_file = calloc(strlen(token) + 1, sizeof(char));
			strcpy(currInput->output_file, token);
		}
		// Ampersand - optional
		else if (conversion == ampersand && token_2 == NULL) {
			token = strtok_r(NULL, " ", &saveptr);
			currInput->ampersand = true;
		}
		else {
			token = strtok_r(NULL, " ", &saveptr);
			token_2 = strtok_r(NULL, " ", &saveptr_2);
		}

	}

	if (is_sigtstp) {
		currInput->ampersand = false;
	}

	currInput->next = NULL;

	return currInput;

}

// Process user input
struct user_input* process_user_input(char* user_input) {
	char copy_user_input[2049];
	strcpy(copy_user_input, user_input);

	// remove /n character at input
	if (!is_var_expansion) {
		copy_user_input[strlen(copy_user_input) - 1] = 0;
		is_var_expansion = false;
	}

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

	// Count the number of digits in pid
	int get_pid = getpid();
	char digits[10];
	sprintf(digits, "%d", get_pid);
	int length;
	length = strlen(digits);

	// Calculate size of char needed for expanded_var
	int original_length = strlen(user_input);
	int var_counter = 0;

	// Count the amount of "$$"s in input
	for (int i = 0; i < original_length - 1; i++) {
		if (copy_user_input[i] == '$' && copy_user_input[i + 1] == '$') {
			var_counter = var_counter + 1;
		}
	}
	if (var_counter > 0) {
		is_var_expansion = true;
	}

	int size_expanded_var = ((var_counter + 1) * length * strlen(copy_user_input) + 1);
	char hold_expanded_var[size_expanded_var];
	hold_expanded_var[0] = '\0';

	// Citation for processing variable expansion
	// Date Accessed: 11/2/2021
	// Code adapted from: https://edstem.org/us/courses/14269/discussion/773207
	for (int i = 0; i < original_length - 1; i++) {
		if (copy_user_input[i] == '$' && copy_user_input[i + 1] == '$') {
			strncat(hold_expanded_var, digits, length);
			i = i + 2;
		}
		else {
			hold_expanded_var[i] = copy_user_input[i];
		}
	}
	strcpy(expanded_var, hold_expanded_var);
}

// Struct for holding background processes
struct background_process
{
	int pid;
	struct background_process* next;
};

struct background_process* add_process(pid_t spawnPid, struct background_process** head) {
	// Citation for creating linked lists
	// Date Accessed: 10/27/2021
	// Code adapted from: https://www.geeksforgeeks.org/linked-list-set-2-inserting-a-node/
	struct background_process* newNode = malloc(sizeof(struct background_process));
	struct background_process* last = *head;

	newNode->pid = spawnPid;
	newNode->next = NULL;
	if (*head == NULL) {
		*head = newNode;
		return 0;
	}
	while (last->next != NULL) {
		last = last->next;
	}
	last->next = newNode;
	return 0;
}

// Have signal handler use this function for SIGTSTP if it's not already on
void sigtstp_function(int signo) {
	is_sigtstp = true;
	char* message = "\nEntering foreground-only mode (& is now ignored)\n: ";
	write(STDOUT_FILENO, message, 53);
}

// Have signal handler use this function for SIGTSTP is already on
void sigtstp_function_2(int signo) {
	is_sigtstp = false;
	char* message = "\nExiting foreground-only mode\n: ";
	write(STDOUT_FILENO, message, 33);
}

void foreground_commands(char** args, int* exit_status, char* input_file, char* output_file, bool* terminated) {
	// Citation for processes and forking child processes
	// Date Accessed: 10/27/2021
	// Code adapted from: https://www.youtube.com/watch?v=1R9h-H2UnLs
	pid_t spawnPid = -5;
	int childExitStatus = -5;
	int child_status = -5;
	int child_signal = -5;
	spawnPid = fork();

	// Citation for using signals
	// Date Accessed: 10/31/2021
	// Code adapted from Signals - Exploration: Signal Handling API

	// Handle SIGINT in parent function
	struct sigaction SIGINT_action = { 0 };
	SIGINT_action.sa_handler = SIG_IGN;
	sigfillset(&SIGINT_action.sa_mask);
	SIGINT_action.sa_flags = SA_RESTART;
	sigaction(SIGINT, &SIGINT_action, NULL);

	// Handle SIGTSTP in parent function
	struct sigaction SIGTSTP_action = { 0 };
	sigfillset(&SIGTSTP_action.sa_mask);
	SIGTSTP_action.sa_flags = 0;
	if (is_sigtstp) {
		SIGTSTP_action.sa_handler = sigtstp_function_2;
	}
	else if (!is_sigtstp) {
		SIGTSTP_action.sa_handler = sigtstp_function;
	}

	// Citation for using sigprocmask()
	// Date Accessed: 11/3/2021
	// Code adapted from: https://stackoverflow.com/questions/5288910/sigprocmask-blocking-signals-in-unix

	// Use sigprocmask() to enter foreground mode after child process finishes
	sigset_t signal_set;
	sigemptyset(&signal_set);
	sigaddset(&signal_set, SIGTSTP);
	sigprocmask(SIG_BLOCK, &signal_set, NULL);

	sigaction(SIGTSTP, &SIGTSTP_action, NULL);

	switch (spawnPid) {
	case -1: {
		perror("Error Occurred.\n");
		exit(1);
		break;
	}
	case 0: {
		// Switch SIGINT back to default in foreground child function
		SIGINT_action.sa_handler = SIG_DFL;
		sigaction(SIGINT, &SIGINT_action, NULL);
		// Ignore SIGTSTP in foreground child function
		SIGTSTP_action.sa_handler = SIG_IGN;
		sigfillset(&SIGTSTP_action.sa_mask);

		// Citation for handling I/O in processes
		// Date Accessed: 10/30/2021
		// Code adapted from Exploration: Processes and I/O
		int fd;
		int fd_o;
		if (input_file) {
			int sourceFD = open(input_file, O_RDONLY);
			if (sourceFD == -1) {
				perror("Input file cannot be opened.\n");
				exit(1);
			}
			int result = dup2(sourceFD, 0);
			if (result == -1) {
				perror("Error occurred, foreground dup() input_file.\n");
				exit(1);
			}
			fd = sourceFD;
		}
		if (output_file) {
			int targetFD = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
			if (targetFD == -1) {
				perror("Output file cannot be opened.\n");
				exit(1);
			}
			int result_2 = dup2(targetFD, 1);
			if (result_2 == -1) {
				perror("Error occurred, foreground dup() output_file.\n");
				exit(1);
			}
			fd_o = targetFD;
		}
		execvp(args[0], args);

		// Close input/output files if applicable
		if (input_file) {
			fcntl(fd, F_SETFD, FD_CLOEXEC);
		}
		if (output_file) {
			fcntl(fd_o, F_SETFD, FD_CLOEXEC);
		}

		// Error message if command isn't found using execvp
		perror("Command not found");
		fflush(stdout);
		exit(1);
		break;
	}
	default: {
		pid_t actualPid = waitpid(spawnPid, &childExitStatus, 0);

		sigfillset(&SIGTSTP_action.sa_mask);
		if (is_sigtstp) {
			SIGTSTP_action.sa_handler = sigtstp_function_2;
		}
		else if (!is_sigtstp) {
			SIGTSTP_action.sa_handler = sigtstp_function;
		}
		sigset_t signal_set;
		sigemptyset(&signal_set);
		sigaddset(&signal_set, SIGTSTP);
		sigprocmask(SIG_BLOCK, &signal_set, NULL);
		sigaction(SIGTSTP, &SIGTSTP_action, NULL);

		// If child process exited normally
		if (WIFEXITED(childExitStatus)) {
			child_status = WEXITSTATUS(childExitStatus);
			*exit_status = child_status;
		}
		// If child process was terminated by a signal
		else {
			child_signal = WTERMSIG(childExitStatus);
			*exit_status = child_signal;
			*terminated = true;
			printf("terminated by signal %d\n", child_signal);
			fflush(stdout);
		}
		break;
	}
	}
}

// Check background processes in linked list to see if they have finished
void background_check(struct background_process* head) {
	head = head->next;
	int check_finished = -5;
	int childExitStatus = -5;
	int child_status = -5;

	while (head != NULL) {
		check_finished = waitpid(head->pid, &childExitStatus, WNOHANG);
		if (check_finished == head->pid) {
			if (WIFEXITED(childExitStatus)) {
				child_status = WEXITSTATUS(childExitStatus);
				printf("background pid %d is done: exit value %d\n", head->pid, child_status);
				fflush(stdout);
			}
			else {
				child_status = WTERMSIG(childExitStatus);
				printf("background pid %d is done: terminated by signal %d\n", head->pid, child_status);
				fflush(stdout);
			}
		}
		head = head->next;
	}

}

// Used when exiting smallsh - terminates background processes if needed
void exit_processes(struct background_process* head) {
	head = head->next;
	while (head != NULL) {
		kill(head->pid, SIGKILL);
		head = head->next;
	}
}

void background_commands(char** args, struct background_process** head, char* input_file, char* output_file) {
	// Citation for processes and forking child processes
	// Date Accessed: 10/27/2021
	// Code adapted from: https://www.youtube.com/watch?v=1R9h-H2UnLs
	pid_t spawnPid = -5;
	int childExitStatus;
	int child_status = -5;

	// Citation for using signals
	// Date Accessed: 10/31/2021
	// Code adapted from Signals - Exploration: Signal Handling API
	struct sigaction SIGINT_action = { 0 };
	SIGINT_action.sa_handler = SIG_IGN;
	sigfillset(&SIGINT_action.sa_mask);
	SIGINT_action.sa_flags = SA_RESTART;
	sigaction(SIGINT, &SIGINT_action, NULL);

	struct sigaction SIGTSTP_action = { 0 };
	SIGTSTP_action.sa_handler = SIG_IGN;
	sigfillset(&SIGTSTP_action.sa_mask);
	SIGTSTP_action.sa_flags = SA_RESTART;
	sigaction(SIGTSTP, &SIGTSTP_action, NULL);

	spawnPid = fork();
	switch (spawnPid) {
	case -1: {
		perror("Error Occurred.\n");
		exit(1);
		break;
	}
	case 0: {
		// Citation for handling I/O in processes
		// Date Accessed: 10/30/2021
		// Code adapted from Exploration: Processes and I/O
		int fd;
		int fd_o;
		if (input_file) {
			int sourceFD = open(input_file, O_RDONLY);
			if (sourceFD == -1) {
				perror("Input file cannot be opened.\n");
				exit(1);
			}
			fd = sourceFD;
			int result = dup2(sourceFD, 0);
			if (result == -1) {
				perror("Error occurred, background dup() input_file.\n");
				exit(1);
			}
		}
		// If no input file is available, use null file
		else {
			int null = open("/dev/null", O_RDONLY);
			int result = dup2(null, 0);
			if (result == -1) {
				perror("Error occurred, background dup() null input_file.\n");
				exit(1);
			}
		}
		if (output_file) {
			int targetFD = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
			if (targetFD == -1) {
				perror("Output file cannot be opened.\n");
				exit(1);
			}
			fd_o = targetFD;
			int result_2 = dup2(targetFD, 1);
			if (result_2 == -1) {
				perror("Error occurred, background dup() output_file.\n");
				exit(1);
			}
		}
		// if not output file is available, use null file
		else {
			int null_out = open("/dev/null", O_WRONLY);
			int result_2 = dup2(null_out, 1);
			if (result_2 == -1) {
				perror("Error occurred, background dup() null output_file.\n");
				exit(1);
			}
		}

		execvp(args[0], args);

		// Close input/output files if applicable
		if (input_file) {
			fcntl(fd, F_SETFD, FD_CLOEXEC);
		}
		if (output_file) {
			fcntl(fd_o, F_SETFD, FD_CLOEXEC);
		}

		// Error message if command isn't found usign execvp
		perror("Command not found");
		exit(1); break;
	}
	default: {
		pid_t actualPid = waitpid(spawnPid, &childExitStatus, WNOHANG);

		// Exit normally
		bool state = WIFEXITED(childExitStatus);
		int state_value = WEXITSTATUS(childExitStatus);

		// Exit not normally
		bool state_terminated = WIFSIGNALED(childExitStatus);
		int state_termination_value = WTERMSIG(childExitStatus);

		// Slow-running background processes to keep track of
		if (actualPid != spawnPid) {
			child_status = WEXITSTATUS(childExitStatus);
			add_process(spawnPid, head);
			printf("background pid is %d\n", spawnPid);
			fflush(stdout);
		}

		else if (actualPid == spawnPid && state) {
			// Fast background processes
			if (state_value == 0) {
				child_status = WEXITSTATUS(childExitStatus);
				printf("background pid is %d\n", spawnPid);
				printf("background pid %d is done: exit value %d\n", spawnPid, child_status);
				fflush(stdout);
			}
			// Command error
			else if (state_value == 1) {
				break;
			}

		}
		else if (actualPid == spawnPid && state_terminated) {
			child_status = WTERMSIG(childExitStatus);
			printf("background pid %d is done: terminated by signal %d\n", spawnPid, child_status);
			fflush(stdout);
		}
		else {
			printf("Error\n");
			fflush(stdout);
		}
		break;
	}
	}
}

// Direct commands to the right processes
void direction(struct user_input* input, bool* continue_sh, bool* child_processed_bool, int* exit_status_int, struct background_process** head, bool* terminated_status) {
	struct user_input* input_2 = input;
	char* cmd = calloc(strlen(input->command) + 1, sizeof(char));
	strcpy(cmd, input_2->command);

	char dest_array[512][200];
	int arg_counter = 0;

	// Variables used to keep track of foreground processes and their exit/termination methods
	bool child_processed = *child_processed_bool;
	int exit_status = *exit_status_int;
	bool terminated_process = *terminated_status;

	char* home;
	char* filename;
	bool has_arg = false;

	// Check if directory name is present in args
	if (input_2->args[1]) {
		filename = calloc(strlen(input->args[1]) + 1, sizeof(char));
		strcpy(filename, input_2->args[1]);
		has_arg = true;

		while (input_2->args[arg_counter] != NULL) {
			strcpy(dest_array[arg_counter], input->args[arg_counter]);
			arg_counter = arg_counter + 1;
		}
	}

	// Discern if command is for a background process
	bool background = input_2->ampersand;

	// cd command
	if (strcmp(cmd, "cd") == 0) {
		// Go to appropriate directory if applicable
		if (has_arg) {
			chdir(filename);
		}
		// Go to home if no directory was inputted
		else {
			home = getenv("HOME");
			chdir(home);
		}
	}

	// exit command
	else if (strcmp(cmd, "exit") == 0) {
		*continue_sh = false;
		exit_processes(*head);
	}

	// status command
	else if (strcmp(cmd, "status") == 0) {
		if (child_processed && !terminated_process) {
			printf("exit value %d\n", exit_status);
			fflush(stdout);
		}
		else if (terminated_process) {
			printf("terminated by signal %d\n", exit_status);
			fflush(stdout);
		}
		else {
			printf("exit value 0\n");
			fflush(stdout);
		}
	}

	// background commands
	else if (background) {
		background_commands(input_2->args, head, input_2->input_file, input_2->output_file);
	}

	// foreground commands
	else {
		foreground_commands(input_2->args, &exit_status, input_2->input_file, input_2->output_file, &terminated_process);
		*child_processed_bool = true;
		*exit_status_int = exit_status;
		*terminated_status = terminated_process;
	}

}

int main() {
	bool continue_sh = true;
	char user_input[2049];
	char pound_sign = '#';

	struct background_process* head = NULL;
	head = malloc(sizeof(struct background_process));

	struct user_input* input;

	// Variable expansion
	char* expanded_var;
	expanded_var = malloc(sizeof(char) * 500);

	// Direction
	bool child_processed = false;
	int exit_status = -5;
	bool terminated = false;

	// Citation for using signals
	// Date Accessed: 10/31/2021
	// Code adapted from Signals - Exploration: Signal Handling API
	struct sigaction SIGINT_action = { 0 }, SIGTSTP_action = { 0 };
	SIGINT_action.sa_handler = SIG_IGN;
	sigfillset(&SIGINT_action.sa_mask);
	SIGINT_action.sa_flags = SA_RESTART;
	sigaction(SIGINT, &SIGINT_action, NULL);

	// Citation for using sigprocmask()
	// Date Accessed: 11/3/2021
	// Code adapted from: https://stackoverflow.com/questions/5288910/sigprocmask-blocking-signals-in-unix
	sigset_t signal_set;
	sigemptyset(&signal_set);
	sigaddset(&signal_set, SIGTSTP);
	sigprocmask(SIG_BLOCK, &signal_set, NULL);

	sigfillset(&SIGTSTP_action.sa_mask);
	SIGTSTP_action.sa_flags = SA_RESTART;
	if (is_sigtstp) {
		SIGTSTP_action.sa_handler = sigtstp_function_2;
	}
	else if (!is_sigtstp) {
		SIGTSTP_action.sa_handler = sigtstp_function;
	}
	sigaction(SIGTSTP, &SIGTSTP_action, NULL);

	printf("$ smallsh\n");
	fflush(stdout);
	// Prompt; Keep prompting user for commands until exit command is entered
	while (continue_sh) {
		background_check(head);
		fflush(stdout);

		// Unblock SIGTSTP signal here to activate it
		sigprocmask(SIG_UNBLOCK, &signal_set, NULL);

		printf(": ");
		fflush(stdout);

		// if fgets is not null
		if (strlen(fgets(user_input, 2049, stdin)) > 1) {

			// if fgets is not a comment (pound sign), start processing input
			if (user_input[0] != pound_sign) {
				variable_expansion(user_input, expanded_var);
				// If user input had "$$" characters
				if (is_var_expansion) {
					input = process_user_input(expanded_var);
					is_var_expansion = false;
				}
				else {
					input = process_user_input(user_input);
				}
				direction(input, &continue_sh, &child_processed, &exit_status, &head, &terminated);
			}
		}
		// if fgets is null
		else {
			continue;
		}
	}
	free(expanded_var);
	free(head);
	return 0;
}