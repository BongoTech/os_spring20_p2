//*******************************************************************
//Author: Cory Mckiel
//Date Created: Feb 20, 2020
//Last Modified: Feb 21, 2020
//Program Name: oss
//Associated Files: oss.c prime.c
//Compiler: gcc
//Options:  -h      (Displays a usage message)
//          -n x    (Indicate the maximum total of child processes,
//                      x, oss will ever create. Default 4)
//          -s x    (Indicate the number of children, x, allowed to
//                      exist in the system at the same time. Default 2)
//          -b B    (Start of the sequence of numbers to be tested
//                      for primality. Default 43)
//          -i I    (Increment between numbers that we test. Default 1)
//          -o filename     (Output file. Default is no output file.)
//
//Program Description: oss is a controller program that solves
//the problem of determining if a number is prime by launching
//multiple child processes suited towards this task, prime.c. At
//any given time there are a fixed number of child processes within
//the system. There is also a total number of children that oss will
//ever create.
//*******************************************************************

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>

void help();
char calling_name[200];

int main(int argc, char *argv[]) 
{
    //Initialize these to acceptable default values.
    int child_proc_alltime_max = 4;
    int child_proc_realtime_max = 2;
    int start_of_prime_sequence = 43;
    int increment_sequence_by = 1;
    char output_file_name[200];
    int use_output_file = 0;

    //Store the name of the executable to a global variable
    //for error reporting.
    strncpy(calling_name, argv[0], 200);

    int option_index = 0;
    while ((option_index = getopt(argc, argv, "hn:s:b:i:o:")) != -1) {
        switch (option_index) {
            case 'h':
                help();
                exit(0);
                break;
            case 'n':
                if (atoi(optarg) <= 20 && atoi(optarg) >=0)
                    child_proc_alltime_max = atoi(optarg);
                break;
            case 's':
                if (atoi(optarg) <= 20 && atoi(optarg) >=0)
                    child_proc_realtime_max = atoi(optarg);
                break;
            case 'b':
                start_of_prime_sequence = atoi(optarg);
                break;
            case 'i':
                increment_sequence_by = atoi(optarg);
                break;
            case 'o':
                use_output_file = 1;
                strncpy(output_file_name, optarg, 200);
                break;
            default:
                fprintf(stderr, "Oops\n");
                exit(-1);
        }
    }


    /* Test print for option detection
    printf("total child proc: %d\n", child_proc_alltime_max);
    printf("sys child proc: %d\n", child_proc_realtime_max);
    printf("start prime: %d\n", start_of_prime_sequence);
    printf("increment by: %d\n", increment_sequence_by);
    if (use_output_file) {
        printf("out file: %s\n", output_file_name);
    }
    */

    //Shared memory variables.
    key_t key;
    int shm_id;
    int *shm_ptr;

    //Use ftok to deterministically generate key so that
    //child process may do the same to access shared memory.
    if((key  = ftok("./", 432)) == -1) {
        fprintf(stderr, "%s: Error: ftok() failed to get shm key.\n%s\n", calling_name, strerror(errno));
        exit(-1);
    }
    //Create the shared memory segment to include the 2 integers
    //seconds and milliseconds, as well as an integer for each child
    //process in alltime_max.
    if ((shm_id = shmget(key, (2+child_proc_alltime_max)*sizeof(int), IPC_CREAT|0666)) < 0) {
        fprintf(stderr, "%s: Error: Failed to allocate shared memory.\n%s\n", calling_name, strerror(errno));
        exit(-1);
    }
    //Attach the memory segment to shm_ptr. Seconds is accessed
    //by *shm_ptr, milliseconds is accessed by *(shm_ptr+1), etc.
    if ((shm_ptr = (int*)shmat(shm_id, NULL, 0)) < 0) {
        fprintf(stderr, "%s: Error: Failed to attach shared memory.\n%s\n", calling_name, strerror(errno));
        exit(-1);
    }

    //Assign seconds and milliseconds as a test.
    *shm_ptr = 10;
    *(shm_ptr+1) = 20;

    //Print their values to confirm.
    printf("Seconds are: %d\n", *shm_ptr);
    printf("Milli are: %d\n", *(shm_ptr+1));


    //Detach and remove shared memory segment.
    shmdt(shm_ptr);
    shmctl(shm_id, IPC_RMID, 0);
    

    return 0;
}

void help()
{
    printf("help\n");
}
