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

// Process user input
void process_user_input(char* user_input) {
	char args[512][2049];
	char copy_user_input[2049];
	char var_expansion = "$$";
	char* var_found;
	char* expanded_var;
	strcpy(copy_user_input, user_input);

	// remove /n character at input
	copy_user_input[strlen(copy_user_input) - 1] = 0;

	// check for variable expansion ($$)
	var_found = strstr(copy_user_input, var_expansion);
	if (var_found) {
		variable_expansion(copy_user_input, expanded_var);
	}

	char* token = strtok(copy_user_input, " ");
	char* saveptr = calloc(strlen(token) + 1, sizeof(char));

	int counter = 0;
	bool special_symbol = false;
	char ampersand = '&';
	bool ampersand_present = false;
	char lesser_than = '<';
	char greater_than = '>';
	int conversion;

	while (token != NULL && !special_symbol) {
		strcpy(saveptr, token);
		conversion = saveptr[0];
		if (conversion == lesser_than || conversion == greater_than) {
			// @ need to check if file name is present after input/output
			special_symbol = true;
			break;
		}

		token = strtok(NULL, " ");
		if (conversion == ampersand && token == NULL) {
			ampersand_present = true;
		}

		strcpy(args[counter], saveptr);
		counter = counter + 1;
	}

	free(saveptr);

	if (special_symbol) {

	}
}

// Signal Handling

// Input Parser

// $$ Expansion
void variable_expansion(char* user_input, char* expanded_var) {
	char copy_user_input[2049];
	strcpy(copy_user_input, user_input);

}

// Built-in commands

// Exec

// Forking



int main() {
	bool continue_sh = true;
	char user_input[2049];
	char pound_sign = '#';

	printf("$ smallsh");

	// Prompt
	while (continue_sh) {
		printf("\n: ");
		fflush(stdout);
		// if fgets is not null
		if (strlen(fgets(user_input, 2049, stdin)) > 1) {

			// if fgets is not a comment (pound sign)
			if (user_input[0] != pound_sign) {
				process_user_input(user_input);
			}
		}
		// if fgets is null
		else {
			continue;
		}
	}
}