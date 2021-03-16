/*******************************************************************\
*                                                                   *
* Author:		Dustin                                      *
* Date:			05/03/19                                    *
* Project ID:		restinghelper.c		                    *
* Problem Description:	Multithreaded lock example with queue       *
*                                                                   *
\********************************************************************/

// compile with gcc -std=c99 restinghelper.c -lpthread -D _BSD_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <semaphore.h>

pthread_mutex_t seatMutex;
pthread_mutex_t enterMutex;
pthread_mutex_t checkMutex;
pthread_mutex_t nMutex;
sem_t helperSleepingSemaphore;
sem_t learnerSemaphore;
int seatsTaken = 0;
int n = 0;

typedef struct v {
	int t_id;
} a_new_type;

void *learner(void *);
void *helper(void *);

int main(int argc, char ** argv) {
	
	// Seed randomizer
	srand(time(NULL));	

	// Prepare ID data to be passed to learner threads
	struct v *data1 = (struct v *) malloc(sizeof(struct v));
	struct v *data2 = (struct v *) malloc(sizeof(struct v));
	struct v *data3 = (struct v *) malloc(sizeof(struct v));
	struct v *data4 = (struct v *) malloc(sizeof(struct v));
	struct v *data5 = (struct v *) malloc(sizeof(struct v));
	data1->t_id = 11;
	data2->t_id = 22;
	data3->t_id = 33;
	data4->t_id = 44;
	data5->t_id = 55;

	// Initialize necessary mutex and semaphores
	pthread_mutex_init(&seatMutex, NULL);
	pthread_mutex_init(&enterMutex, NULL);
	pthread_mutex_init(&checkMutex, NULL);
	pthread_mutex_init(&nMutex, NULL);
	sem_init(&helperSleepingSemaphore,0,0);
	sem_init(&learnerSemaphore,0,0);

	// Prepare parameters for pthread
	pthread_t tid[6];
	pthread_attr_t attr;
	pthread_attr_init(&attr);

	// Create all threads
	pthread_create(&tid[0],&attr,helper,(void *)NULL);
	pthread_create(&tid[1],&attr,learner,(void *)data1);	
	pthread_create(&tid[2],&attr,learner,(void *)data2);
	pthread_create(&tid[3],&attr,learner,(void *)data3);
	pthread_create(&tid[4],&attr,learner,(void *)data4);
	pthread_create(&tid[5],&attr,learner,(void *)data5);

	// Wait for all threads to close before ending execution
	for (int i = 0; i<6; i++){
		pthread_join(tid[i],NULL);
	}

	// Useless test case
	printf("Parent thread shouldn't print this until children done (i.e. never)\n");
	
	exit(0);	
}

void *learner(void *param){
	
	// Convert void parameter to struct v type and pull learner id into local variable id
	struct v * data  = (struct v *) param;
	int id = data->t_id;		
	sleep(3);
	// Loop indefinitely
	while(1){
		
		// Randomize an amount of time to program for
		int temp = (int)(rand()%(10)+5);

		// Print out time going to program for, sleep that amount of time, then print out going to look for helper
		printf("Learner %d is going to program (sleep) for %d seconds!\n",id,temp);
		sleep(temp);
		printf("Learner %d needs help with their program! Going to see helper!\n",id);	
	
		// Use a mutex lock to quickly determine the number of seats currently available without error or locking up the variable
		pthread_mutex_lock(&seatMutex);

		//pthread_mutex_unlock(&c

		// There are three total chairs, but we're using the initial iteration as an indication that the helper is sleeping
		if (seatsTaken < 3){
			seatsTaken++;
			int currentSeats = seatsTaken;
			pthread_mutex_unlock(&seatMutex);
			
			pthread_mutex_lock(&checkMutex);
			sem_post(&helperSleepingSemaphore);
			pthread_mutex_unlock(&checkMutex);
		
			printf("Learner %d is taking a seat waiting for the helper to become available.\n",id);
			
			
			// Lock and find the seat the learner is taking.
			// Chair #0 means going straight to helper
			printf("Learner %d is taking a seat. There are %d chair(s) still open.\n",id,(3-currentSeats));
			
			// Enter the office when available
			
			pthread_mutex_lock(&enterMutex);	
			printf("***LEARNER %d is ENTERING the HELPER's Office.***\n",id);
			pthread_mutex_lock(&nMutex);
			n = id;
			//pthread_mutex_unlock(&nMutex);
			sem_wait(&learnerSemaphore);
			pthread_mutex_unlock(&enterMutex);

			printf("***LEARNER %d is EXITING the HELPER'S Office.***\n",id);
		}	
		else{ 
			printf("No seats are available. Learner %d will come back later!\n",id);
			pthread_mutex_unlock(&seatMutex);
		}
	};

	printf("\nLearner %d thread exiting now\n",id);
	pthread_exit(0);
}

void *helper(void *param){
	int value = 0;
	int newvalue = 0;
	// Loop indefinitely
	// Ensure message displayed 
	while(1){
		// Set the helper to sleep
		// time before and after execution!
		// movde downprintf("No one is here. The helper is going to sleep.\n");
		// get clock ehre
		pthread_mutex_lock(&checkMutex);
		sem_getvalue(&helperSleepingSemaphore,&value);
		
		if (value <= 0){
			printf("No one is here. The helper is going to sleep.\n");
			pthread_mutex_unlock(&checkMutex);
		}

		sem_wait(&helperSleepingSemaphore);
		
		sem_getvalue(&helperSleepingSemaphore,&newvalue);
		
		printf("semvalue: %d %d\n",value,newvalue);
		if ((newvalue == 0) && (value == 0)) printf("Someone woke the helper up!\n");
		pthread_mutex_unlock(&checkMutex);	
			// Check if anyone is here
			
		pthread_mutex_unlock(&enterMutex);
				
		int sec = (int)(rand()%(3)+2);
		//pthread_mutex_lock(&nMutex);
		printf("***Helping LEARNER %d for %d minutes***\n",n,sec);
		pthread_mutex_unlock(&nMutex);
		sleep(sec);
		seatsTaken--;
		sem_post(&learnerSemaphore);
		pthread_mutex_unlock(&enterMutex);
		pthread_mutex_unlock(&seatMutex);
		sleep(1);
		
	};
	printf("\nhelper thread exiting now\n");
	pthread_exit(0);
}
