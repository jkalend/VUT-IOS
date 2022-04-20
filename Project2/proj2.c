//
// Created by kalen on 20.4.22.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>


int main (int argc, char *argv[]) {
    //check for correct number of arguments
    if (argc != 5) {
		printf("Incorrect number of arguments.\n");
		return 1;
	}
	//check for correct arguments
	char *error = "";

	//create a variable to hold the number of hydrogen atoms
	long nh = strtol(argv[2], &error, 10);
	if (strcmp(error, "") != 0) {
		printf("Invalid number of hydrogen atoms.\n");
		return 1;
	}
	//create a variable to hold the number of oxygen atoms
	long no = strtol(argv[1], &error, 10);
	if (strcmp(error, "") != 0) {
		printf("Invalid number of oxygen atoms.\n");
		return 1;
	}
	//create a variable to hold the number of milliseconds spent waiting
	long ti = strtol(argv[3], &error, 10);
	if (strcmp(error, "") != 0) {
		printf("Invalid number of milliseconds.\n");
		return 1;
	}
	//create a variable to hold the maximum number of milliseconds required to finish the job
	long tb = strtol(argv[4], &error, 10);
	if (strcmp(error, "") != 0) {
		printf("Invalid number of milliseconds.\n");
		return 1;
	}

    //check for correct arguments
    if (nh < 0 || no < 0 || ti < 0 || tb < 0) {
		printf("Negative number of atoms.\n");
		return 1;
	}
	if (ti > 1000 || tb > 1000) {
		fprintf(stderr, "Too many milliseconds.\n");
		return 1;
	}

	u_int8_t *shm = mmap(NULL, sizeof(u_int8_t) * (nh + no), PROT_READ | PROT_WRITE,
								   MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	*shm = 0;

	long size;
	if (nh > no) {
		size = nh;
	} else {
		size = no;
	}

	//if semaphore == 0, process have to wait
	//if semaphore == 1 and higher, process can run
	//wait(x); semaphore decremented by x
	//post(y); semaphore set to y

	//fork the process in loop to create the number of oxygen atoms and hydrogen atoms
	printf("Forking %ld oxygen atoms and %ld hydrogen atoms.\n", no, nh);
	for (long i = 0; i < size; i++) {
		//pid_t pid = fork();
		if (i < no) {
			if (fork() == 0) { //child process
				*shm += 1;
				//exit the child process
				exit(0);
			} else { //parent process
				//wait for the child process to finish
				wait(NULL);
				//printf("%d\n", *shm);
			}
		}
		if (i < nh) {
			if (fork() == 0) { //child process
				*shm += 1;
				//exit the child process
				exit(0);
			} else { //parent process
				//wait for the child process to finish
				wait(NULL);
				//printf("%d\n", *shm);
			}
		}
	}
	printf("%d\n", *shm);
	//free(shm);
}
