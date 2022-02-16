#include <stdio.h>
#include <stdlib.h>
/*
 * int main takes a command line input of an int x, and uses it solve the x philosophers problem. Default input is 5. main uses semaphores to synchronize the philosophers in a for loop, printing how much the philosphers eat or think. After all the philosophers are done, the array is cleaned up   
 * */
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <errno.h>
#include <math.h>
#include <sys/wait.h>
int randomGaussian(int,int);
int main(int argc, char **argv){
	//x is the number of philosophers
	int x;
	//if a command line arguement exists, set x to argv[2], the command line arguement
	if(argc == 2){
		sscanf(argv[1],"%d",&x);
	}
	else{
		//default value x is the default number of dining philosophers
		x = 5;
	}
	//create a semaphore array size x and save it in chopsticks. The chopsticks variable (aka the semaphore array) represents the chopsticks
	int chopsticks = semget(IPC_PRIVATE,x,IPC_CREAT | IPC_EXCL | 0600);
	//error check if chopsticks is -1, if so print error message
	if(chopsticks  == -1){
		int error = errno;
		fprintf(stderr,"error code %d = %s\n",error,strerror(error));
	}
	//create a pick sembuf pick that activates eating by picking up a semaphore chopstick. Pick decrements the semaphore's semval
	struct sembuf pick[1] = { { 0,-1,0} };
	//create a drop sembuff drop by finishing using a chopstick semaphore, or dropping it. Drop increments the semaphore's semval
	struct sembuf drop[1] = { {0,1,0} };
	//create array of process ids x. it will store each process id representing a philosopher
	int pid[x];
	//food represnts the total amount of food eaten by a philospher. The variable food is a global that each process keeps its own independet count after forking
	int food = 0;
	//drop all the chopsticks on the table by having semctl setting all the chopsticks elements' semval to 1, the sem_op of drop 
	for(int i = 0; i < x; i++){
		int check = semctl(chopsticks,i,SETVAL,1);
		//store the output of semctl in check. If check is -1, print the error message
		if(check == -1){
			int error = errno;
			fprintf(stderr,"error code %d = %s",error,strerror(error));
		}
	}
	//i is the increment counter in the for loop that keeps track of philosphers and chopsticks
	int i = 0;
	//initialize each process(philosopher) and fork x amount of processes in the for loop. There will be x amount of children processes that eat food until food reaches a 100. Parent does nothing
	for(i = 0;i < x;i++){
		//fork a process into pid[i],philospher is the child process
		pid[i] = fork();
		//parent does nothing
		if(pid[i]){
		}
		//child is the philospher process. The philospher process eats until it full or food is 100
		else{
			//srand prepares seed for random. Actual value inside srand is something I got from stackOverflow. It wasn't explained 
			srand(time(0) ^ getpid());
			//while food is less than 100, the philosopher will try to eat. If he can't access one of the 2 semaphores representing the chopsticks, he waits, after a philospher has eaten for a random amount of time, he thinks for a random amount of time 
			while(food <= 100){
				//i is the left-most chopstick on the table. j is the rightmost chopstick and is essentailly i incremented by 1. if i is x-1, or the largest element in chopsticks array, j is 0 so that the philosphers will continue to eat until they are full
				int j;
				if(i == x-1){
					j = 0;
				}
				else{
					j = i+1;
				}
				//attempt to pick up the i chopstick, or decrease the semval for the ith chopsticks elemenet by by 1. Store the output of semop in left
				pick->sem_num = i;
				//^^^ IMPORTANT  ^^^ pick/drop->sem_num changes the chopsticks element number pick/drop will work on.Here on the line before,pick's sem_num is set to i so that pick can work on the ith element in chopsticks. THIS WILL NOT BE REPEATED AGAIN
				int left = semop(chopsticks,pick,1);
				//if left returns -1, print error message
				if(left == -1){
					int error = errno;
					fprintf(stderr,"Chopstick 1 pick up error: error code %d and error code %d: %s\n",i,error,strerror(error));
				}
				//k is the amount of food eaten in an instance by a philosopher
				int k;
				//otherwise, if left is 0, continue doing other operations
				if(left == 0){
					//attempt to pick up the j chopstick by attempting decrease the semval for the jth chopsticks semval by 1 
					pick->sem_num = j;
					int right = semop(chopsticks,pick,1);
					//if process can't pick up chopstick j, drop chopstick i(decrementing the chopsticks ith element's semval by 1), or increase the semval for the semaphore chopsticks element by 1
					if(right == -1){
						drop->sem_num = i;
						semop(chopsticks,drop,1);
					}
					//otherwise have the process eat the food, drop the chopsticks and think
					else{
						//k is set to a random number from the randomGaussian function with standard mean 9 and standard deviation 3. This will be the time that a philospher wull wat
						k = randomGaussian(9,3);
						//if k is less than 0, set it to 0
						if(k < 0){
							k = 0;
						}
						//incrmenent the amount of food eaten by k
						food = food + k;
						//print the amount of food used by philosopjer i, and the type of chopsticks he used
						printf("%d amount eaten in size of %d by philosopher %d using chopsticks %d and %d\n",food,k,i,i,j);
						//sleep for k seconds to simualte eating
						sleep(k);
						//drop the jth chopstick, or increment the semval of the jth chopsticks elelement by 1
						drop->sem_num = j;
						int drop_right = semop(chopsticks,drop,1);
						//check to see if drop_right is -1, if so print error message
						if(drop_right == -1){
							int num = errno;
							fprintf(stderr,"chopstick 1 = error code %d = %s\n",num,strerror(num));
						}
						//change the drop's sem_num to i to drop the ith chopstick element's semval to increment by 1
						drop->sem_num = i;
						int drop_left = semop(chopsticks,drop,1);
						//if drop_left is -1, print error message
						if(drop_left == -1){
							int error2 = errno;
							fprintf(stderr,"chopstick 2 error code %d = %s\n",error2,strerror(error2));
						}
						//int l is the variable for the thinking period. It is a random Gaussian number with standard mean 11 and standard deviation 7
						int l = randomGaussian(11,7);
						if(l < 0){
							l = 0;
						}
						//sleep for k seconds to simulate the philosophers thinking and print the philosopher's name and his time of thinking period
						printf("Philosopher %d is thinking %d seconds\n",i,l);
						sleep(l);
						//if food is greater than 100, philosopher is full and is not hungry anymore. Exit the process
						if(food >= 100){
							exit(0);
						}
					}
				}
			}
		}
	}
	//while the children processes are running, wait until all the child processes are done 
	for(int i = 0;i < x;x++){
		//infinite wait
		int stop = wait(NULL);
		//if no child processes exist stop waiting
		if(stop == -1){
			break;
		}
	}
	//delete the chopstick semaphore set with semctl and save the ouput in delete
	int delete = semctl(chopsticks,i,IPC_RMID);
	//if delete produes -1, produce an error message
	if(delete == -1){
		int error = errno;
		fprintf(stderr,"error code %d = %s\n",error,strerror(error));
	}
}
/* successive calls to randomGaussian produce integer return values */
/* having a gaussian distribution with the given mean and standard  */
/* deviation.  Return values may be negative.                       */

int randomGaussian(int mean, int stddev) {
	double mu = 0.5 + (double) mean;
	double sigma = fabs((double) stddev);
	double f1 = sqrt(-2.0 * log((double) rand() / (double) RAND_MAX));
	double f2 = 2.0 * 3.14159265359 * (double) rand() / (double) RAND_MAX;
	if (rand() & (1 << 5))
		return (int) floor(mu + sigma * cos(f2) * f1);
	else
		return (int) floor(mu + sigma * sin(f2) * f1);
}	

