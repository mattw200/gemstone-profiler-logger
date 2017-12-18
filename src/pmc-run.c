#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <time.h>
#include "../include/pmc-get-row.h"

#define CHECK_FILENAME "PMC_RUN_CHECK"

/* Author: Matthew J. Walker */
/* Date created: 14 June 2017 */

/* This is a simple wrapper that uses pmc-get-header and pmc-get-row to 
 * log PMCs continuously. It stops if the file "PMC_RUN_CHECK" doesn't 
 * exist or reads anything other than "1"
 * ASSUMES pmc-setup has already been run!
 */

int check_stopfile() 
{
    FILE *check = fopen(CHECK_FILENAME, "r");  
    if (check == NULL) {	
        printf("NULL\n");
        return 0;
    }
    char check_buffer[1];
    fread(check_buffer, 1, 1, check);
    fclose(check);
    check = NULL;
    if (check_buffer[0] == '1')
        return 1;
    //printf("NOT 1\n");
    return 0;
}

int main(int argc, char *argv[])
{
    long sample_period = 0;
    if( argc != 2 ) {
        printf("Expects one argument!\n");
        printf("USAGE: pmc-run [sample period in us]\n");
        printf("E.g.: pmc-run 500000\n");
        exit(0);
    } else {
	sample_period = atol(argv[1]); 
    } 
    system("./bin/pmc-get-header");
    system("echo '1' > "CHECK_FILENAME"");
    char label[64] = "log";
    while(1) {
        pmc_get_row(&label[0]);
        if (!check_stopfile())
            break;
        //usleep(sample_period);            
	struct timespec tim;
        // sample period is in us
        if (sample_period > 999999) {
            tim.tv_sec = (int)(sample_period / 1000000); 
            tim.tv_nsec = (sample_period - ((long)tim.tv_sec * 1000000)) * 1000;
        }
        else {
            tim.tv_sec = 0;
            tim.tv_nsec = sample_period * 1000;
        }
	nanosleep(&tim , NULL);
    }
    return 0;
}

