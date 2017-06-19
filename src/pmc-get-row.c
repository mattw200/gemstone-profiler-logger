#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>  
#include <stdlib.h>
#include <inttypes.h>
#include "../include/pmc-helper-64.h"

#include <sched.h>

#define HEADER_LEN 4096

//#define PLATFORM_ODROID_C2 1

int is_cpu_online(int cpu_num) 
{
    char filename[128];
    sprintf(filename, "/sys/devices/system/cpu/cpu%d/online", cpu_num);
    FILE *online_check = fopen(filename, "r");  
    if (online_check == NULL) {	
         //error 
         printf("FATAL ERROR: could not open cpu online file\n");
         exit(-1);
    }
    char online_buffer [8];
    fread(online_buffer, 1, 8, online_check);
    fclose(online_check);		
    return atoi(online_buffer);  
}

/* 
 * If the cpu is offline, then the software can't access the 
 * registers of the cpu to determine the number of counters
 * (needed to print the correct number of columns). 
 * Therefore, it uses a file with them recorded in 
 * (file created by pmc-setup
 */
int get_num_counters_from_file(int cpu_num)
{
    FILE *cpu_data = fopen("cpu-data.csv", "r");  
    if (cpu_data == NULL) {
        printf("FATAL ERROR: could not open cpu-data.csv\n");
        exit(-1);
    }
    char line[8];
    int count = 0;
    int num_counters = 0;
    while (fgets(line, sizeof(line), cpu_data)) {
        if (count == cpu_num) {
            return atoi(line); 
        }
        count++;
    }
    printf("FATAL ERROR: cpu not found in cpu-data.csv file\n");
    exit(-1);
}

void pmc_get_row(char* row_label) {
    // get number of CPUs (both offline and online)
    unsigned long num_cpus = sysconf (_SC_NPROCESSORS_CONF);
    unsigned long online_cpus = sysconf (_SC_NPROCESSORS_ONLN);
    //printf("number of CPUs configured: %lu\n", num_cpus);
    //printf("number of CPUs online: %lu\n", num_cpus);
    if (num_cpus != online_cpus) {
        fprintf(stderr, "FATAL ERROR: Some CPUs are offline. All must be online for setup.\n");
        exit(-1);
    }
	// millisecond int
    struct timespec milt;
    clock_gettime(CLOCK_REALTIME, &milt);
    int64_t millitime = milt.tv_sec * INT64_C(1000) + milt.tv_nsec / 1000000;
    printf("%"PRId64"",millitime);
    // date stamp
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char timeStamp[64];
    printf("\t%d-%d-%d %d:%d:%d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    printf("\t%s", row_label);
    //printf("Millitime: %f\n", (double)millitime);
    //printf("Millitime int: %"PRId64"\n", millitime);
    //char header[HEADER_LEN];
    int i = 0;
    for (i = 0; i < num_cpus; i++) {
        // test is cpu is online or offline
        if (!is_cpu_online(i)) {
            // get cpu stats from setup
            unsigned int counters = get_num_counters_from_file(i);
            int j = 0;
            for (j = 0; j < counters; j++) {
                printf("\t-1");
            }
            continue;
        }
        unsigned long mask = 0 | (1<<i); //cpu 0
        unsigned int len = sizeof(mask);
        int result = sched_setaffinity(0, len, &mask);
        //printf("Core: %d, Result: %d\n", i, result);
        uint32_t cpuid = get_cpu_id_code();
        uint32_t num_counters = get_no_counters();
        num_counters = get_no_counters();
        uint32_t counts[num_counters];
        uint32_t cycle_count = get_cycle_count();
        get_counts(&counts[0]);
        printf("\t%"PRIu32"", cycle_count);
        //char field[512];
        // get cycle count
        int p;
        for (p = 0; p < num_counters; p++) {
            //sprintf(field, "\t%sCPU %d cntr %d (0x%02X)", field, i, p, counts[p]);
            //printf("\tCPU %d cntr %d (0x%02X)", i, p, counts[p]);
            printf("\t%"PRIu32"",  counts[p]);
        }
        //strcpy(header, field); 
    }
#ifdef PLATFORM_ODROID_C2
    // add temperature
    int c2_temperature = -1;
    FILE *temp_file = fopen("/sys/devices/virtual/thermal/thermal_zone0/temp", "r");  
    if (temp_file == NULL) {	
        //error 
	} else {
	    char buffer [32];
        fread(buffer, 1, 32, temp_file);
        c2_temperature =atoi(buffer);
        fclose(temp_file);		
    }
    //sprintf(header, "%s\t%f", header, c2_temperature);
    printf("\t%d", c2_temperature/1000);
#endif
    printf("\n");
}
