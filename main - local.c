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
#include <signal.h>
#include <fcntl.h>
#include <signal.h>

// Global booleans
bool is_sigtstp = false;
bool is_var_expansion = false;


// struct for processing user input
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
	char* saveptr_2;
	char* copy_input = calloc(strlen(input) + 1, sizeof(char));
	strcpy(copy_input, input);
	char* token_2 = strtok_r(copy_input, " ", &saveptr_2);

	// First token is command
	char* saveptr;
	char* token = strtok_r(input, " ", &saveptr);
	currInput->command = calloc(strlen(token) + 1, sizeof(char));
	strcpy(currInput->command, token);


	int counter = 0;
	int conversion = 0;
	char ampersand = '&';
	char input_symbol = '<';
	char output_symbol = '>';
	bool symbol = false;

	currInput->args[counter] = token;
	counter += 1;

	//bool next_token_null = false;
	token_2 = strtok_r(NULL, " ", &saveptr_2);

	// put arguments into list
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

		currInput->args[counter] = token;
		counter += 1;

	}


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

	// process pid - count digits in pid
	int get_pid = getpid();
	char digits[10];
	sprintf(digits, "%d", get_pid);
	int length;
	length = strlen(digits);

	// calculate size of char needed for expanded_var
	int original_length = strlen(user_input);
	int var_counter = 0;

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

	// https://edstem.org/us/courses/14269/discussion/773207

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

struct background_process
{
	int pid;
	struct background_process* next;
};

struct background_process* add_process(pid_t spawnPid, struct background_process** head) {
	// adapted from https://www.geeksforgeeks.org/linked-list-set-2-inserting-a-node/
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

void sigtstp_function(int signo) {
	is_sigtstp = true;
	char* message = "\nEntering foreground-only mode (& is now ignored)\n: ";
	write(STDOUT_FILENO, message, 53);

}

void sigtstp_function_2(int signo) {
	is_sigtstp = false;
	char* message = "\nExiting foreground-only mode\n: ";
	write(STDOUT_FILENO, message, 33);
}

void foreground_commands(char** args, int* exit_status, char* input_file, char* output_file, bool* terminated) {
	// https://www.youtube.com/watch?v=1R9h-H2UnLs
	pid_t spawnPid = -5;
	int childExitStatus = -5;
	int child_status = -5;
	int child_signal = -5;
	spawnPid = fork();

	struct sigaction SIGINT_action = { 0 };
	SIGINT_action.sa_handler = SIG_IGN;
	sigfillset(&SIGINT_action.sa_mask);
	SIGINT_action.sa_flags = SA_RESTART;
	sigaction(SIGINT, &SIGINT_action, NULL);

	struct sigaction SIGTSTP_action = { 0 };
	sigfillset(&SIGTSTP_action.sa_mask);
	SIGTSTP_action.sa_flags = 0;
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


	switch (spawnPid) {
	case -1: {
		perror("Error Occurred.\n");
		exit(1);
		break;
	}
	case 0: {
		SIGINT_action.sa_handler = SIG_DFL;
		sigaction(SIGINT, &SIGINT_action, NULL);
		SIGTSTP_action.sa_handler = SIG_IGN;
		sigfillset(&SIGTSTP_action.sa_mask);
		int fd;
		int fd_o;
		// Processes and I/O
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
		if (input_file) {
			fcntl(fd, F_SETFD, FD_CLOEXEC);
		}
		if (output_file) {
			fcntl(fd_o, F_SETFD, FD_CLOEXEC);
		}
		perror("Command not found");
		fflush(stdout);
		exit(1);
		break;
	}
	default: {
		pid_t actualPid = waitpid(spawnPid, &childExitStatus, 0);

		// https://stackoverflow.com/questions/5288910/sigprocmask-blocking-signals-in-unix

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


		if (WIFEXITED(childExitStatus)) {
			child_status = WEXITSTATUS(childExitStatus);
			*exit_status = child_status;
		}
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

void exit_processes(struct background_process* head) {
	head = head->next;
	while (head != NULL) {
		kill(head->pid, SIGKILL);
		head = head->next;
	}
}

void background_commands(char** args, struct background_process** head, char* input_file, char* output_file) {
	// https://www.youtube.com/watch?v=1R9h-H2UnLs
	pid_t spawnPid = -5;
	int childExitStatus;
	int child_status = -5;

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
		else {
			int null_out = open("/dev/null", O_WRONLY);
			int result_2 = dup2(null_out, 1);
			if (result_2 == -1) {
				perror("Error occurred, background dup() null output_file.\n");
				exit(1);
			}
		}
		execvp(args[0], args);
		if (input_file) {
			fcntl(fd, F_SETFD, FD_CLOEXEC);
		}
		if (output_file) {
			fcntl(fd_o, F_SETFD, FD_CLOEXEC);
		}
		perror("Command not found");
		exit(1); break;
	}
	default: {
		pid_t actualPid = waitpid(spawnPid, &childExitStatus, WNOHANG);
		// exit normally
		bool state = WIFEXITED(childExitStatus);
		int state_value = WEXITSTATUS(childExitStatus);

		// exit not normally
		bool state_terminated = WIFSIGNALED(childExitStatus);
		int state_termination_value = WTERMSIG(childExitStatus);

		// slow-running background process to keep track of
		if (actualPid != spawnPid) {
			child_status = WEXITSTATUS(childExitStatus);
			add_process(spawnPid, head);
			printf("background pid is %d\n", spawnPid);
			fflush(stdout);
		}

		else if (actualPid == spawnPid && state) {
			// fast background process
			if (state_value == 0) {
				child_status = WEXITSTATUS(childExitStatus);
				printf("background pid is %d\n", spawnPid);
				printf("background pid %d is done: exit value %d\n", spawnPid, child_status);
				fflush(stdout);
			}
			// command error
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

void direction(struct user_input* input, bool* continue_sh, bool* child_processed_bool, int* exit_status_int, struct background_process** head, bool* terminated_status) {
	struct user_input* input_2 = input;
	char* cmd = calloc(strlen(input->command) + 1, sizeof(char));
	strcpy(cmd, input_2->command);

	char dest_array[512][200];
	int arg_counter = 0;

	bool child_processed = *child_processed_bool;
	int exit_status = *exit_status_int;
	bool terminated_process = *terminated_status;

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

	bool background = input_2->ampersand;

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
		*continue_sh = false;
		exit_processes(*head);
	}

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

	else if (background) {
		background_commands(input_2->args, head, input_2->input_file, input_2->output_file);
	}

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

	// Signals - Exploration: Signal Handling API
	struct sigaction SIGINT_action = { 0 }, SIGTSTP_action = { 0 };
	SIGINT_action.sa_handler = SIG_IGN;
	sigfillset(&SIGINT_action.sa_mask);
	SIGINT_action.sa_flags = SA_RESTART;
	sigaction(SIGINT, &SIGINT_action, NULL);

	// https://stackoverflow.com/questions/5288910/sigprocmask-blocking-signals-in-unix
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
	// Prompt
	while (continue_sh) {
		background_check(head);
		fflush(stdout);
		sigprocmask(SIG_UNBLOCK, &signal_set, NULL);
		printf(": ");
		fflush(stdout);
		// if fgets is not null
		if (strlen(fgets(user_input, 2049, stdin)) > 1) {

			// if fgets is not a comment (pound sign)
			if (user_input[0] != pound_sign) {
				variable_expansion(user_input, expanded_var);
				if (is_var_expansion) {
					input = process_user_input(expanded_var);
					is_var_expansion = false;

				}
				else {
					input = process_user_input(user_input);
				}
				//input = process_user_input(user_input);
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