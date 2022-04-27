//
// IOS - Project 2
// Author: Jan Kalenda
// Date: 26.4.2022
// VUT FIT
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
#define MOLECULE_MAX 6


#define SEM_COUNT 7

/// Initialization of shared memory
/// \param shm shared memory
/// \param nh number of hydrogen atoms
/// \param no number of oxygen atoms
void init_mem(int *shm, const int nh, const int no) {
	shm[COUNTER] = 1; // counts the number of lines
	shm[HYDROGEN_ID] = 1; // counts the number of hydrogen atoms
	shm[OXYGEN_ID] = 1; // counts the number of oxygen atoms
	shm[MOLECULE_COUNT] = 0; // counts the number of molecules
	shm[NH_OREQ] = 2*no; // number of oxygen atoms required to make one molecule (doubled)
	shm[NO_HREQ] = nh; // number of hydrogen atoms required for one oxygen atom to make one molecule
	shm[MOLECULE_MAX] = 0; // maximum number of molecules
}

/// Checks whether the semaphores are initialized correctly
/// \param check return value of sem_init
/// \return 0 on success, 1 on failure of initialization
int sem_check(int check) {
	if (check == -1) {
		fprintf(stderr, "Error: semaphore creation failed\n");
		return 1;
	}
	return 0;
}

/// Initializes and deletes semaphores
/// \param sem shared memory for semaphores
/// \param count number of semaphores
/// \param values array of initial semaphore values
/// \param clear 0 to initialize semaphores, 1 to delete them
/// \return 0 on success, 1 on failure of initialization
int init_sems(sem_t **sem, int count, int *values, char clear) {
	int fail = 0;

	// clears the semaphores
	if (clear == 1 || values == NULL) {
		goto init_clear;
	}
	// initializes the semaphores
	for (int i = 0; i < count; i++) {
		if (sem_check(sem_init(*sem + i, 1, values[i])) == 1) {
			fail = 1;
			goto init_clear;
		}
	}
	return EXIT_SUCCESS;
init_clear:
	for (int i = 0; i < count; i++) {
		sem_destroy(*sem + i);
	}
	return fail;
}

/// Forks the hydrogen processes
/// \param sem shared memory for semaphores
/// \param shm general shared memory
/// \param fp stream for output
/// \param ti time to wait between before going to queue
/// \param pids array of pids needed to be freed in the child process
/// \return pid of the child process on success, -1 on failure
pid_t hydrogen(sem_t *sem, int *shm, FILE *fp, const long ti, pid_t *pids) {
	pid_t control = fork();
	if (control == 0) { //child process
		sem_wait(&sem[3]);
		int id = shm[HYDROGEN_ID];

		//seeds random number generator
		srandom(time(NULL) * getpid());

		fprintf(fp,"%d: H %d: started\n", shm[COUNTER], id);
		fflush(fp);
		INCH //increment counter and hydrogen id

		sem_post(&sem[3]);

		usleep(random()%(ti+1) * 1000);

		sem_wait(&sem[3]);
		fprintf(fp, "%d: H %d: going to queue\n", shm[COUNTER], id);
		fflush(fp);
		INCC
		//shm[NH_OREQ]--;

		sem_post(&sem[3]);

		sem_post(&sem[1]); //signals arrival of hydrogen for the molecule creation
		sem_wait(&sem[2]); //waits for oxygen to start binding

		int mol = shm[MOLECULE_COUNT]+1; //sets the molecule id for the hydrogen

		sem_wait(&sem[3]);
		if (shm[MOLECULE_COUNT] == shm[MOLECULE_MAX]) {

			sem_post(&sem[3]);
			sem_wait(&sem[6]); // waits for the last molecule to be created
			sem_wait(&sem[3]);
			fprintf(fp, "%d: H %d: not enough O or H\n", shm[COUNTER], id);
			fflush(fp);
			fclose(fp);
			free(pids);
			INCC
			sem_post(&sem[6]);
			sem_post(&sem[2]);
			sem_post(&sem[3]);
			exit(1);
		}
		sem_post(&sem[3]);


		sem_wait(&sem[3]);
		fprintf(fp, "%d: H %d: creating molecule %d\n", shm[COUNTER], id, mol);
		fflush(fp);
		INCC
		sem_post(&sem[3]);
		sem_post(&sem[5]);


		sem_wait(&sem[4]); //waits for the oxygen to bind
		sem_wait(&sem[3]);
		fprintf(fp, "%d: H %d: molecule %d created\n", shm[COUNTER], id, mol);
		fflush(fp);
		INCC
		sem_post(&sem[3]);
		sem_post(&sem[5]); //signals the oxygen that the hydrogen is done

		//exit the child process
		fclose(fp);
		free(pids);
		exit(0);
	} else if (control == -1) {
		fprintf(stderr, "Forking failed\n");
		return -1;
	}
	else { //parent process
		return control;
	}
}

/// Forks the oxygen processes
/// \param sem shared memory for semaphores
/// \param shm general shared memory
/// \param fp stream for output
/// \param ti time to wait between before going to queue
/// \param tb time to wait to simulate bind
/// \param pids array of pids needed to be freed in the child process
/// \return pid of the child process on success, -1 on failure
pid_t oxygen(sem_t *sem, int *shm, FILE *fp, const long ti, const long tb, pid_t *pids) {
	pid_t control = fork();
	if (control == 0) { //child process
		sem_wait(&sem[3]);
		int id = shm[OXYGEN_ID];

		srandom(time(NULL) * getpid()); // seeds random number generator

		fprintf(fp, "%d: O %d: started\n", shm[COUNTER], id);
		fflush(fp);
		INCO
		sem_post(&sem[3]);

		usleep(random()%(ti+1) * 1000);

		sem_wait(&sem[3]);
		fprintf(fp, "%d: O %d: going to queue\n", shm[COUNTER], id);
		fflush(fp);
		INCC

		shm[NO_HREQ] -= 2;
		if (shm[NO_HREQ] < 0) {
			sem_post(&sem[3]);
			sem_wait(&sem[6]); //waits for the last molecule
			sem_wait(&sem[3]);
			fprintf(fp, "%d: O %d: not enough H\n", shm[COUNTER], id);
			fflush(fp);
			fclose(fp);
			free(pids);
			INCC
			sem_post(&sem[6]);
			sem_post(&sem[2]);
			sem_post(&sem[3]);
			exit(1);
		}
		sem_post(&sem[3]);

		//making molecule

		sem_wait(&sem[0]); // only one oxygen
		sem_wait(&sem[1]); // waits for 2 hydrogen atoms
		sem_wait(&sem[1]);


		sem_post(&sem[2]); // release 2 hydrogen atoms
		sem_post(&sem[2]);

		int mol = shm[MOLECULE_COUNT]+1;

		sem_wait(&sem[3]);
		fprintf(fp, "%d: O %d: creating molecule %d\n", shm[COUNTER], id, mol);
		fflush(fp);
		INCC
		sem_post(&sem[3]);

		usleep(random()%(tb+1) * 1000);

		sem_wait(&sem[5]);
		sem_wait(&sem[5]);

		sem_wait(&sem[3]);
		sem_post(&sem[4]); // signals the hydrogen to print the molecule is created
		sem_post(&sem[4]); // 2 hydrogen atoms are signaled
		fprintf(fp, "%d: O %d: molecule %d created\n", shm[COUNTER], id, mol);
		fflush(fp);
		INCC
		sem_post(&sem[3]);

		sem_wait(&sem[3]);
		shm[MOLECULE_COUNT]++;
		sem_post(&sem[3]);

		sem_wait(&sem[5]); // holds the oxygen until the molecule is finished
		sem_wait(&sem[5]); // waits for the hydrogen atoms to finish

		if (shm[MOLECULE_COUNT] == shm[MOLECULE_MAX]) {
			sem_post(&sem[6]); // signals the leftover atoms to exit
			sem_post(&sem[2]); // frees any leftover hydrogen atoms
		}

		sem_post(&sem[0]);
		// exit the child process
		fclose(fp);
		free(pids);
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
    // check for correct number of arguments
    if (argc != 5) {
		fprintf(stderr, "Incorrect number of arguments.\n");
		return 1;
	}
	// check for correct arguments
	char *error = "";

	// create a variable to hold the number of hydrogen atoms
	long nh = strtol(argv[2], &error, 10);
	if (strcmp(error, "") != 0 || strcmp(argv[2], "") == 0) {
		fprintf(stderr, "Invalid number of hydrogen atoms.\n");
		return 1;
	}
	// create a variable to hold the number of oxygen atoms
	long no = strtol(argv[1], &error, 10);
	if (strcmp(error, "") != 0 || strcmp(argv[1], "") == 0) {
		fprintf(stderr, "Invalid number of oxygen atoms.\n");
		return 1;
	}
	// create a variable to hold the number of milliseconds spent waiting
	long ti = strtol(argv[3], &error, 10);
	if (strcmp(error, "") != 0 || strcmp(argv[3], "") == 0) {
		fprintf(stderr, "Invalid number of milliseconds.\n");
		return 1;
	}
	// create a variable to hold the maximum number of milliseconds required to finish the job
	long tb = strtol(argv[4], &error, 10);
	if (strcmp(error, "") != 0 || strcmp(argv[4], "") == 0) {
		fprintf(stderr, "Invalid number of milliseconds.\n");
		return 1;
	}

    // check for correct arguments
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

	// create a file to write to
	FILE *fp = fopen("proj2.out", "w");

	// create a general shared memory segment
	int *shm = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	if (shm == MAP_FAILED) {
		fprintf(stderr, "Error: mmap failed\n");
		fclose(fp);
		exit(1);
	}

	// memory used to store semaphores
	sem_t *sem = mmap(NULL, 4096 , PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	if (sem == MAP_FAILED) {
		fprintf(stderr, "Error: mmap failed\n");
		munmap(shm, 4086);
		fclose(fp);
		exit(1);
	}

	// initialize the general shared memory segment
	init_mem(shm, (int)nh, (int)no);

	// allocates the memory for pids of the processes
	pid_t *pids = malloc(sizeof(pid_t) * (nh + no));
	if (pids == NULL) {
		fprintf(stderr, "Error: malloc failed\n");
		goto exit;
	}
	// [0] for single oxygen, starts at 1,
	// [1] signals the oxygen atom of available hydrogen atoms, starts at 0,
	// [2] for signal from oxygen to hydrogen atoms to start binding, starts at 0,
	// [3] to protect the critical sections, starts at 1,
	// [4] for order of output, starts at 0,
	// [5] for oxygen atom to wait for hydrogen atoms to finish, starts at 0,
	// [6] for "not enough" messages, holds all overflowing atoms, starts at 0,
	int check = init_sems(&sem, SEM_COUNT, (int[]){1, 0, 0, 1, 0, 0, 0}, 0);
	if (check == 1)
		goto exit;

	long size;
	if (nh > no) {
		size = nh;
	} else {
		size = no;
	}

	long h = nh;
	long o = no;
	for (long i = 0; i < no; i++) {
		if (h <= 1 || o < 0) {
			break;
		} else {
			shm[MOLECULE_MAX]++;
			h -= 2;
			o -= 1;
		}
	}

	if (shm[MOLECULE_MAX] == 0) {
		sem_post(&sem[6]);
	}
	// fork the process in loop to create the number of oxygen atoms and hydrogen atoms
	// stores the pid of the process in the pids array
	// cnt is used to keep track of the number of processes created
	int cnt = 0;
	for (long i = 0; i < size; i++) {
		if (i < no) {
			pids[cnt] = oxygen(sem, shm, fp, ti, tb, pids);
			if (pids[cnt] == -1) {
				cnt--; // to prevent -1 in kill()
				goto exit;
			}
			cnt++;
		}
		if (i < nh) {
			pids[cnt] = hydrogen(sem, shm, fp, ti, pids);
			if (pids[cnt] == -1) {
				cnt--; // to prevent -1 in kill()
				goto exit;
			}
			cnt++;
		}
	}

	// wait for all processes to finish
	for (int i = 0; i < cnt; i++) {
		waitpid(pids[i],NULL,0);
	}

	// destroys the semaphores, shared memory, the pids array, and closes the file
	init_sems(&sem, SEM_COUNT, NULL, 1);
	munmap(shm, 4096);
	munmap(sem, 4096);
	free(pids);
	fclose(fp);
	return 0;
exit:
	// signals all processes to stop
	for (int i = 0; i < cnt; i++) {
		kill(pids[i], SIGKILL);
	}
	init_sems(&sem, SEM_COUNT, NULL, 1);
	munmap(shm, 4096);
	munmap(sem, 4096);
	if (pids != NULL)
		free(pids);
	fclose(fp);
	return 1;
}
