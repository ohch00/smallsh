To compile:
On the os1.engr.oregonstate.edu server, type "gcc --std=gnu99 -o smallsh main.c" 
in the terminal without the quotation marks.

To run:
// Citation for running the testing script with smallsh.
// Date Accessed: 10/31/2021
// Instructions adapted from Assignment 3 Page.

On the os1.engr.oregonstate.edu server, place the testing script in the same directory as smallsh and chmod the testing script using "chmod +x ./p3testscript"
Run the following command(s) from a bash prompt:
$ ./p3testscript 2>&1
or
$ ./p3testscript 2>&1 | more
or
$ ./p3testscript > mytestresults 2>&1 