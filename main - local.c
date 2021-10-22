/*
Name: Christina Oh
Date: 10/22/2021
Project: Assignment 3
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>

// Prompt, Comments & Blank Lines
void process_user_input(char* user_input) {
	char args[512][2048];

	char* token = strtok_r(user_input, " ");
	char* saveptr = calloc(strlen(token) + 1, sizeof(char));

	int counter = 1;
	bool special_symbol = false;
	char ampersand = '&';
	char lesser_than = '<';
	char greater_than = '>';

	while (token != NULL && not special_symbol) {
		strcpy(saveptr, token);
		if (strcmp(saveptr, ampersand) == 0 || strcmp(saveptr, lesser_than) == 0 || strcmp(saveptr, greater_than) == 0) {
			special_symbol = true;
			break;
		}
		token = strtok(NULL, " ");
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

// Built-in commands

// Exec

// Forking



int main() {
	bool continue_sh = true;
	char user_input[2049];
	printf("$ smallsh\n");

	// Prompt
	while (continue_sh) {
		fprintf(stdout, ": ");
		fflush(stdout);
		fgets(user_input, 2049, stdin);
		process_user_input(user_input);

	}
}