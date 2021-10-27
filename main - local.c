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
	char* args[512];
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


void direction(struct user_input* input, bool* continue_sh) {
	char* cmd = calloc(strlen(input->command) + 1, sizeof(char));
	strcpy(cmd, input->command);

	char args_array[512][500];
	strcpy(args_array, input->args);
	char* filename = args_array[0];
	char* home;

	if (strcmp(cmd, "cd") == 0) {
		if (filename) {
			chdir(filename);
		}
		else {
			home = getenv("HOME");
			chdir(home);
		}
	}

	else if (strcmp(cmd, "exit") == 0) {
		*continue_sh = true;
	}

	else if (strcmp(cmd, "status") == 0) {

	}

	else {

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


	printf("$ smallsh");
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
}