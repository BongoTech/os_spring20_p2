//*******************************************************************
//Author: Cory Mckiel
//Date Created: Feb 20, 2020
//Last Modified: Feb 23, 2020
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

//Declare this as volatile so the compiler
//knows that it can be asynchronously changed.
//Declare as atomic so that nothing tries
//to change at the same time.
static volatile sig_atomic_t kill_flag = 0;

//Store the name the program was called with.
char calling_name[200];

//Kill signal handler. Sets kill signal.
//kill_flag will be checked during
//program running to see if we should
//terminate gracefully.
static void handler(int sig)
{
    kill_flag = 1;   
}

//Set up the interrupt to catch SIGUSR1
//which doesn't have any default action.
//Allows me to end the program on my terms.
//Returns -1 on error.
static int setupinterrupt()
{
    struct sigaction act;
    act.sa_handler = handler;
    act.sa_flags = 0;
    return (sigemptyset(&act.sa_mask) || sigaction(SIGUSR1, &act, NULL));
}

int main(int argc, char *argv[])
{
    //Store the executable name in a global
    //variable to use for error reporting.
    strncpy(calling_name, argv[0], 200);

    //Set up the interrupt so the parent
    //can kill this process gracefully upon
    //termination.
    if (setupinterrupt() == -1) {
        fprintf(stderr, "%s: Error: Failed to set up interrupt.\n%s\n", calling_name, strerror(errno));
        exit(-1);
    }
    
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

    //Print them to stdout to confirm reciept.
    fprintf(stderr, "%s: logical ID: %d\n", calling_name, logical_child_id);
    //printf("%s: potential prime: %d\n", calling_name, potential_prime);
    //printf("%s: size of shm: %d\n", calling_name, size_of_shm);

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

    //Print out the shared memory.
    //printf("%s: Seconds: %d\n", calling_name, *shm_ptr);
    //printf("%s: %d: Milliseconds: %d\n", calling_name, logical_child_id, *(shm_ptr+1));

    //Do some fake work.
    int i = 0;
    while(i < 1000) {
        i += 1;
    }
    
    //Store the potential_prime in shared memory
    //for testing purposes to let oss/master
    //know the process finished.
    *(shm_ptr+(2+logical_child_id)) = potential_prime;   
   
    //Detach and remove shm
    shmdt(shm_ptr);
    //Not sure if the child should remove the shared memory
    //or let the parent do it.
    //shmctl(shm_id, IPC_RMID, NULL);
      
    return 0;
}
