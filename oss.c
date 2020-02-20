#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

void help();


int main(int argc, char *argv[]) 
{
    //Initialize these to acceptable default values.
    int child_proc_alltime_max = 5;
    int child_proc_realtime_max = 2;
    int start_of_prime_sequence = 43;
    int increment_sequence_by = 1;
    char output_file_name[200];
    int use_output_file = 0;

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

    printf("total child proc: %d\n", child_proc_alltime_max);
    printf("sys child proc: %d\n", child_proc_realtime_max);
    printf("start prime: %d\n", start_of_prime_sequence);
    printf("increment by: %d\n", increment_sequence_by);

    if (use_output_file) {
        printf("out file: %s\n", output_file_name);
    }

    return 0;
}

void help()
{
    printf("help\n");
}
