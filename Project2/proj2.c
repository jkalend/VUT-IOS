//
// Created by kalen on 20.4.22.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <time.h>

#define INCH shm[COUNTER]++; shm[HYDROGEN_ID]++;
#define INCO shm[COUNTER]++; shm[OXYGEN_ID]++;
#define INCC shm[COUNTER]++;

#define COUNTER 0
#define HYDROGEN_ID 1
#define OXYGEN_ID 2
#define MOLECULE_COUNT 3
#define NH_OREQ 4
#define NO_HREQ 5


#define SEM_COUNT 5

void init_mem(int *shm, const int nh, const int no) {
	shm[COUNTER] = 1;
	shm[HYDROGEN_ID] = 1;
	shm[OXYGEN_ID] = 1;
	shm[MOLECULE_COUNT] = 0;
	shm[NH_OREQ] = 2*no;
	shm[NO_HREQ] = nh;
}

int sem_check(int check) {
	if (check == -1) {
		fprintf(stderr, "Error: semaphore creation failed\n");
		return 1;
	}
	return 0;
}

int init_sems(sem_t **sem, int count, int *values, char clear) {
	if (clear == 1 || values == NULL) {
		goto init_clear;
	}
	for (int i = 0; i < count; i++) {
		if (sem_check(sem_init(*sem + i, 1, values[i])) == 1) {
			goto init_clear;
		}
	}
	return EXIT_SUCCESS;
init_clear:
	for (int i = 0; i < count; i++) {
		sem_destroy(*sem + i);
	}
	return EXIT_FAILURE;
}

pid_t hydrogen(sem_t *sem, int *shm, FILE *fp, const long ti) {
	pid_t control = fork();
	if (control == 0) { //child process
		sem_wait(&sem[3]);
		int id = shm[HYDROGEN_ID];
		srandom(time(NULL) * getpid());

		fprintf(fp,"%d: H %d: started\n", shm[COUNTER], id);
		fflush(fp);
		INCH
		sem_post(&sem[3]);

		usleep(random()%(ti+1) * 1000);

		sem_wait(&sem[3]);
		fprintf(fp, "%d: H %d: going to queue\n", shm[COUNTER], id);
		fflush(fp);
		INCC
		shm[NH_OREQ]--;
		if (shm[NH_OREQ] < 0 || (shm[NH_OREQ] <= 1 && shm[NO_HREQ] < 0) || (shm[NO_HREQ] == 1 &&
																			shm[NH_OREQ] <= 1)) {
			//sem_wait(&sem[3]);
			fprintf(fp, "%d: H %d: not enough O or H\n", shm[COUNTER], id);
			fflush(fp);
			INCC
			sem_post(&sem[3]);
			exit(1);
		}
		sem_post(&sem[3]);

		sem_post(&sem[1]);
		sem_wait(&sem[2]);
		int mol = shm[MOLECULE_COUNT];

		sem_wait(&sem[3]);
		fprintf(fp, "%d: H %d: creating molecule %d\n", shm[COUNTER], id, mol);
		fflush(fp);
		INCC
		sem_post(&sem[3]);

		//usleep(random()%(tb+1) * 1000);

		sem_wait(&sem[4]);
		sem_wait(&sem[3]);
		fprintf(fp, "%d: H %d: molecule %d created\n", shm[COUNTER], id, mol);
		fflush(fp);
		INCC
		sem_post(&sem[3]);

		//exit the child process
		exit(0);
	} else if (control == -1) {
		fprintf(stderr, "Forking failed\n");
		return -1;
	}
	else { //parent process
		return control;
	}
}

pid_t oxygen(sem_t *sem, int *shm, FILE *fp, const long ti, const long tb) {
	pid_t control = fork();
	if (control == 0) { //child process
		sem_wait(&sem[3]);
		int id = shm[OXYGEN_ID];
		srandom(time(NULL) * getpid());

		fprintf(fp, "%d: O %d: started\n", shm[COUNTER], id);
		fflush(fp);
		INCO
		sem_post(&sem[3]);

		usleep(random()%(ti+1) * 1000);

		sem_wait(&sem[3]);
		fprintf(fp, "%d: O %d: going to queue\n", shm[COUNTER], id);
		fflush(fp);
		INCC
		//sem_post(&sem[3]);

		//sem_wait(&sem[3]);
		shm[NO_HREQ] -= 2;
		if (shm[NO_HREQ] < 0) {
			//sem_wait(&sem[3]);
			fprintf(fp, "%d: O %d: not enough H\n", shm[COUNTER], id);
			fflush(fp);
			INCC
			sem_post(&sem[3]);
			exit(1);
		}
		sem_post(&sem[3]);

		//making molecule

		sem_wait(&sem[0]); // only one oxygen
		sem_wait(&sem[1]); // waits for 2 hydrogen atoms
		sem_wait(&sem[1]);

		shm[MOLECULE_COUNT]++;
		sem_post(&sem[2]); // release 2 hydrogen atoms
		sem_post(&sem[2]);

		int mol = shm[MOLECULE_COUNT];

		sem_wait(&sem[3]);
		fprintf(fp, "%d: O %d: creating molecule %d\n", shm[COUNTER], id, mol);
		fflush(fp);
		INCC
		sem_post(&sem[3]);

		usleep(random()%(tb+1) * 1000);

		sem_wait(&sem[3]);
		sem_post(&sem[4]);
		sem_post(&sem[4]);
		fprintf(fp, "%d: O %d: molecule %d created\n", shm[COUNTER], id, mol);
		fflush(fp);
		INCC
		sem_post(&sem[3]);

		sem_post(&sem[0]);
		//exit the child process
		exit(0);
	} else if (control == -1) {
		fprintf(stderr, "Forking failed\n");
		return -1;
	}
	else { //parent process
		return control;
	}
}


int main (int argc, char *argv[]) {
    //check for correct number of arguments
    if (argc != 5) {
		fprintf(stderr, "Incorrect number of arguments.\n");
		return 1;
	}
	//check for correct arguments
	char *error = "";

	//create a variable to hold the number of hydrogen atoms
	long nh = strtol(argv[2], &error, 10);
	if (strcmp(error, "") != 0 || strcmp(argv[2], "") == 0) {
		fprintf(stderr, "Invalid number of hydrogen atoms.\n");
		return 1;
	}
	//create a variable to hold the number of oxygen atoms
	long no = strtol(argv[1], &error, 10);
	if (strcmp(error, "") != 0 || strcmp(argv[1], "") == 0) {
		fprintf(stderr, "Invalid number of oxygen atoms.\n");
		return 1;
	}
	//create a variable to hold the number of milliseconds spent waiting
	long ti = strtol(argv[3], &error, 10);
	if (strcmp(error, "") != 0 || strcmp(argv[3], "") == 0) {
		fprintf(stderr, "Invalid number of milliseconds.\n");
		return 1;
	}
	//create a variable to hold the maximum number of milliseconds required to finish the job
	long tb = strtol(argv[4], &error, 10);
	if (strcmp(error, "") != 0 || strcmp(argv[4], "") == 0) {
		fprintf(stderr, "Invalid number of milliseconds.\n");
		return 1;
	}

    //check for correct arguments
    if (nh < 0 || no < 0 || ti < 0 || tb < 0) {
		fprintf(stderr, "Negative number of atoms.\n");
		return 1;
	}
	if (nh == 0 && no == 0) {
		fprintf(stderr, "No atoms.\n");
		return 1;
	}
	if (ti > 1000 || tb > 1000) {
		fprintf(stderr, "Too many milliseconds.\n");
		return 1;
	}


	FILE *fp = fopen("proj2.out", "w");

	setbuf(fp, NULL);

	int *shm = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	if (shm == MAP_FAILED) {
		fprintf(stderr, "Error: mmap failed\n");
		fclose(fp);
		exit(1);
	}
	//memory used to store semaphores
	sem_t *sem = mmap(NULL, 4096 , PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	if (sem == MAP_FAILED) {
		fprintf(stderr, "Error: mmap failed\n");
		munmap(shm, 4086);
		fclose(fp);
		exit(1);
	}

	init_mem(shm, (int)nh, (int)no);
	// [0] for single oxygen, [1] for hydrogen atoms, [2] for creating of molecule
	// [3] to prevent parallel writes to memory, [4] for order of output
	int check = init_sems(&sem, SEM_COUNT, (int[]){1, 0, 0, 1, 0}, 0);
	if (check == 1)
		goto exit;

	long size;
	if (nh > no) {
		size = nh;
	} else {
		size = no;
	}

	//fork the process in loop to create the number of oxygen atoms and hydrogen atoms
	pid_t *last = malloc(sizeof(pid_t) * (nh + no));
	int cnt = 0;
	for (long i = 0; i < size; i++) {
		if (i < no) {
			last[cnt] = oxygen(sem, shm, fp, ti, tb);
			if (last[cnt] == -1) {
				free(last);
				goto exit;
			}
			cnt++;
		}
		if (i < nh) {
			last[cnt] = hydrogen(sem, shm, fp, ti);
			if (last[cnt] == -1) {
				free(last);
				goto exit;
			}
			cnt++;

		}
	}
	//while (wait(NULL) > 0);

	for (int i = 0; i < cnt; i++) {
		waitpid(last[i],NULL,0);
	}

	init_sems(&sem, SEM_COUNT, NULL, 1);
	munmap(shm, 4096);
	munmap(sem, 4096);
	free(last);
	fclose(fp);
	return 0;
exit:
	init_sems(&sem, SEM_COUNT, NULL, 1);
	munmap(shm, 4096);
	munmap(sem, 4096);
	fclose(fp);
	return 1;
}
