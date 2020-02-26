//*******************************************************************
//Author: Cory Mckiel
//Date Created: Feb 20, 2020
//Last Modified: Feb 26, 2020
//Program Name: prime
//Associated Files: prime.c oss.c
//Compiler: gcc
//Options:  The following numbers are REQUIRED for proper operation.
//          logical_id      (The assignment number for the child.)
//          number          (number to be tested for primality.)
//          shm_size        (The size of the shm seg this child
//                              will attach to.)
//Usage:    ./prime logical_id number shm_size (Exact order matters)
//
//Program Description: prime is designed to be a child process to 
//oss. It's main function is test a number to see if it is prime.
//To communicate with oss it attaches to shared memory created
//by oss.
//*******************************************************************

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>

//Store the name the program was called with.
char calling_name[200];

int main(int argc, char *argv[])
{
    //Store the executable name in a global
    //variable to use for error reporting.
    strncpy(calling_name, argv[0], 200);

    //Block ctrl-c  
    signal(SIGINT, SIG_IGN);

    //Check to make the the correct number
    //of arguments is satisfied.
    if (argc != 4) {
        fprintf(stderr, "%s: Usage: ./%s logical_child_id_int potential_prime_int sizeof_shm\n", calling_name, calling_name);
        exit(-1);
    }
    
    //Made it this far? Store the numbers.
    int logical_child_id = atoi(argv[1]);
    int potential_prime = atoi(argv[2]);
    int size_of_shm = atoi(argv[3]);

    //Declare the shared mem variables.
    key_t key;
    int shm_id;
    int *shm_ptr;

    //Generate the same key as oss/master. (Hardcoded)
    if ((key = ftok("./", 432)) == -1) {
        fprintf(stderr, "%s: Error: ftok() failed to generate key.\n%s\n", calling_name, strerror(errno));
        exit(-1);
    }
    //Get the shared memory created by oss/master using same key.
    if ((shm_id = shmget(key, size_of_shm*sizeof(int), 0666)) < 0) {
        fprintf(stderr, "%s: Error: Failed to get shared memory.\n%s\n", calling_name, strerror(errno));
        exit(-1);
    }
    //Attach to the shm.
    if ((shm_ptr = (int*)shmat(shm_id, NULL, 0)) < 0) {
        fprintf(stderr, "%s: Error: Failed to attach to shared memory.\n%s\n", calling_name, strerror(errno));
        exit(-1);
    }

    int sieve_size = potential_prime+1;
    int sieve[sieve_size];
    int i, j;

	//Fill the prime sieve.
	for (i = 0; i < sieve_size; i++) {
        if (*shm_ptr) {
            break;
        }
		sieve[i] = i;
	}

    //Start counting simulated milliseconds.
    int starttime = *(shm_ptr+2);
    int times_up = 0;

	//In all honesty, I got this algorithm from codesdope.com.
	//I understand the algorithm and the comments are my own.
	i = 2;
	//Loop through each element in sieve and remove all 
	//multiples of prime numbers. By the time i*i > arraySize
	//all non primes have been removed.
	while ((i*i) <= sieve_size) {
        //Check that the calculation hasn't taken
        //more than 1 ms simulated time.
        if (*(shm_ptr+2) - starttime > 1000000) {
            times_up = 1;
            break;
        }
        //Uncomment to test killing computation.
        //sleep(1);
        if (*shm_ptr) {
            //Parent sent kill signal
            break;
        }
        //check for even numbers besides 2.
        if (potential_prime != 2 && (potential_prime % 2) == 0) {
            sieve[potential_prime] = 0;
            break;
        }
        
      	//If sieve[i] == 0, then the value is not prime.
		if (sieve[i] != 0) {
			//Loop through sieve and erase multiples
			//of sieve[i], which at this point is prime.
			for (j = 2; j < sieve_size; j++) {
                if (*shm_ptr) {
                    break;
                }
				//First ensure we're in bounds.
				if (sieve[i]*j > sieve_size) {
					break;
				} else {
					//Then set each multiple
					//of sieve[i] to 0.
					sieve[sieve[i]*j] = 0;
				}
			}
		}
		i++;
	}

    //Now the sieve contains only prime numbers and
    //zeros up to potential_prime. Look in the last index
    //to see if potential_prime is still there or if it is 
    //a zero. If kill flag, didn't finish, store -1.
    if (*shm_ptr == 1 || times_up) {
        *(shm_ptr+3+logical_child_id) = -1;
    }
    else if (sieve[potential_prime] == 0) {
        *(shm_ptr+3+logical_child_id) = potential_prime - (2*potential_prime);
    }
    else {
        *(shm_ptr+3+logical_child_id) = potential_prime;
    }
    
    //Detach and remove shm
    shmdt(shm_ptr);
     
    return 0;
}
