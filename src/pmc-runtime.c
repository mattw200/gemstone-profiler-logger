#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <time.h>
#include <math.h>
#include "../include/pmc-get-row.h"


#define CHECK_FILENAME "PMC_RUN_CHECK"

/* Author: Matthew J. Walker */
/* Date created: 14 June 2017 */

/* This is a simple wrapper that uses pmc-get-header and pmc-get-row to 
 * log PMCs continuously. It stops if the file "PMC_RUN_CHECK" doesn't 
 * exist or reads anything other than "1"
 * ASSUMES pmc-setup has already been run!
 */

//The largest possible number of fields/columns in the csv
// file created by pmc-get-headers and pmc-get-pmcs
// (can also include power sensor data, frequency, temperature etc.)
#define MAX_FIELDS 128

int check_stopfile() 
{
    FILE *check = fopen(CHECK_FILENAME, "r");  
    if (check == NULL) {	
        return 0;
    }
    char check_buffer[1];
    fread(check_buffer, 1, 1, check);
    fclose(check);
    if (check_buffer[0] = '1')
        return 1;
    return 0;
}

double read_pmc_data(int* pmc_indexes, int* is_offline, uint32_t* pmc_values)
{
    double time_ms = -1.0f;
    char line[4096];
    int line_count = 0;
    int field_count = 0;
    FILE *file = fopen("temp-pmc-row.csv", "r");
    if (file == NULL) {
       printf("FATAL ERROR: could not open temp-pmc-row.csv file\n");
       exit(-1);
    }
    while (fgets(line, sizeof(line), file)) {
        char * pch;
        pch = strtok (line,"\t");
        while (pch != NULL) {
            //printf ("Delim %d: %s\n",count,pch);
            if (field_count == 0) { // fiend of time_ms
                time_ms = atof(pch); //atof actually returns a double
            }
            if (field_count > 0 && pmc_indexes[field_count]) {
                // This field is a cycle count/pmc - needs to be converted to 'rate'
                if (strcmp(pch,"-1") == 0) {
                    // cpu is offline, therefore this field is 'NA'
                    is_offline[field_count] = 1;
                } else {
                    //cpu is offline and this field has a counter value
                    pmc_values[field_count] = strtoul(pch, NULL, 0);
                }
            }
            pch = strtok (NULL, "\t");
            field_count++;
        }
    }
    return time_ms;
}


int main(int argc, char *argv[])
{
    int sample_period = 0;
    if( argc != 2 ) {
        printf("Expects one argument!\n");
        printf("USAGE: pmc-runtime [sample period in ms]\n");
        printf("E.g.: pmc-runtime 500000\n");
        exit(0);
    } else {
	sample_period = atoi(argv[1]); 
    } 
    system("bin/pmc-get-header > temp-pmc-header.csv");
    system("bin/pmc-get-pmcs > temp-pmc-row.csv");
    // find PMC column indexes
    int pmc_indexes[MAX_FIELDS]; // we know index 0 is not valid
    FILE *temp = fopen("temp-pmc-header.csv", "r");
    if (temp == NULL) {
         printf("FATAL ERROR: could not open temp-pmc-header.csv file\n");
         exit(-1);
    }
    char line[4096];
    int count = 0;
    int h = 0;
    char old_headers[2048]; //the existing headers from 'pmc-get-pmcs'
    char new_headers[2048]; //the new headers (e.g. the pmc rates)
    while (fgets(line, sizeof(line), temp)) {
        char * pch;
        pch = strtok (line,"\t");
        unsigned int current_id = -1;
        while (pch != NULL) {
            //printf ("Delim %d: %s\n",count,pch);
            sprintf(old_headers, "%s%s\t", old_headers, pch);
            if ( (strstr(pch, "cycle count") != NULL) ||  (strstr(pch, "cntr") != NULL)) {
                // found a heading that is a PMC or cycle count 
                sprintf(new_headers, "%s\t%s rate", new_headers, pch);
                pmc_indexes[count] = 1;
                h++;
            } else {
                pmc_indexes[count] = 0;
            }
            pch = strtok (NULL, "\t");
            count++;
        }
    }
    fclose(temp);
    int i = 0;
    for (i = 0; i < h; i++) {
        printf("%d,",pmc_indexes[i]);
    }
    //remove '\n' from old headers
    //old_headers[strlen(old_headers) -2] = 0;
    old_headers[strlen(old_headers) -2] = 0;
    printf("\n%s%s\n", old_headers, new_headers); //print all headers (old + new)
    system("echo '1' > "CHECK_FILENAME"");
    char label[64] = "log";
    while(1) {
        /* Steps in this bit:
         * 1) Read last saved PMCs from file
         * 2) Use the already created pmc_indexes to find pmcs/cycle count
         *  (these need to be converted to 'rate' from the raw values)
         * 3) Get the values from file and save into array of uint32 (from str) 'pre_pmc_values'
         *     (the file reads '-1' if CPU offline)
         *     (therfore, an array 'pre_is_offline' created. 0=online, 1=offline)
         * 4) Get the current PMCs and save to file
         */
        // 'pre' used to identify variables for previously saved PMCs (i.e. last sample)
        // 'cur' used to identify variables for current PMCs (i.e. current sample)
        uint32_t pre_pmc_values[MAX_FIELDS]; //used to save numerical pmc_values
        int pre_is_offline[MAX_FIELDS];
        // read the temp-pmc-row.csv' file
        double pre_time_ms = read_pmc_data(pmc_indexes, pre_is_offline, pre_pmc_values);
        for (i = 0; i < 64; i++) { //should be MAX_FIELDS (not 64) but easier to see
            printf("%d: %"PRIu32" %d\n", i, pre_pmc_values[i], pre_is_offline[i]);
        }
        system("bin/pmc-get-pmcs log > temp-pmc-row.csv");
        uint32_t cur_pmc_values[MAX_FIELDS]; //used to save numerical pmc_values
        int cur_is_offline[MAX_FIELDS];
        double cur_time_ms = read_pmc_data(pmc_indexes, cur_is_offline, cur_pmc_values);
        for (i = 0; i < 64; i++) {
            printf("%d: %"PRIu32" %d\n", i, cur_pmc_values[i], cur_is_offline[i]);
        }
        double delta_time = (cur_time_ms - pre_time_ms)/1000000.0f;
        printf("Time 1: %f, Time 2: %f, delta_s: %f\n", pre_time_ms, cur_time_ms, delta_time);
        // now go through each pmc and find the rate
        char new_columns[2048];
        for (i = 0; i < MAX_FIELDS; i++) {
            if (pmc_indexes[i] ) {
                // this field is a pmc/cycle count field
                if (!(pre_is_offline[i] || cur_is_offline[i])) {
                    // cpu is online in both samples
                    double delta_pmcs = 0.0f;
                    if (cur_pmc_values[i] >= pre_pmc_values[i]) {
                        // normal scenario, no overflow
                        delta_pmcs = (double)cur_pmc_values[i] - (double)pre_pmc_values[i];
                    } else {
                        // overflow. 
                        delta_pmcs = (double)cur_pmc_values[i] + (pow(2, 32) - (double)pre_pmc_values[i]);
                    }
                    sprintf(new_columns, "%s%f\t", new_columns, delta_pmcs);
                } else {
                    sprintf(new_columns, "%soffline\t", new_columns);
                }
            }
        }
        printf("%s\n",new_columns);
        break;
        usleep(sample_period);            
        if (!check_stopfile())
            break;
    }
    return 0;
}
