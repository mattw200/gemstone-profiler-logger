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

#define EVENTS_CONFIG_FILENAME "events.config"

void get_user_events(unsigned int cpu_id, int *events, char* events_config_filename)
{
    FILE *config_file;
    config_file = fopen(events_config_filename, "r");
    if (config_file == NULL) {
        fprintf(stderr, "FATAL ERROR: Could not open file: %s\n", EVENTS_CONFIG_FILENAME);
        exit(-1);
    }
    char line[256];
    int count = 0;
    while (fgets(line, sizeof(line), config_file)) {
        int i = 0;
        for (i = 0; i < EVENT_ARRAY_LEN; i++) {
            events[i] = -1;
        }
        //printf("Line %d: %s\n", count, line); 
        char * pch;
        pch = strtok (line,",");
        int delim_count = 0;
        unsigned int current_id = -1;
        while (pch != NULL) {
            if (delim_count == 0) {
              current_id = strtol(pch, NULL, 0); 
            } else if (delim_count > 1) {
                events[delim_count-2] = strtol(pch, NULL, 0);
            }
            //printf ("%d: %s\n",delim_count,pch);
            pch = strtok (NULL, ",");
            delim_count++;
        }
        if (current_id == cpu_id) {
            //printf("CPU ID Match\n");
            break;
        }
        count++;
    }
    fclose(config_file);
}

int main(int argc, char *argv[])
{
    // get number of CPUs (both offline and online)
    unsigned long num_cpus = sysconf (_SC_NPROCESSORS_CONF);
    unsigned long online_cpus = sysconf (_SC_NPROCESSORS_ONLN);
    printf("number of CPUs configured: %lu\n", num_cpus);
    printf("number of CPUs online: %lu\n", num_cpus);
    if (num_cpus != online_cpus) {
        fprintf(stderr, "FATAL ERROR: Some CPUs are offline. All must be online for setup.\n");
        exit(-1);
    }
    FILE *cpu_data = fopen("cpu-data.csv", "w");  
    if (cpu_data == NULL) {
        fprintf(stderr, "FATAL ERROR: Could not open cpu-data file\n");
        exit(-1);
    }
    int i = 0;
    for (i = 0; i < num_cpus; i++) {
        unsigned long mask = 0 | (1<<i); //cpu 0
        unsigned int len = sizeof(mask);
        int result = sched_setaffinity(0, len, &mask);
        //printf("Core: %d, Result: %d\n", i, result);
        init_pmcs();
        uint32_t cpuid = get_cpu_id_code();
        uint32_t num_counters = get_no_counters();
		fprintf(cpu_data, "%d\n", num_counters);
        //printf("Core: %d, id: %u, no. counters: %u\n", i, cpuid, num_counters);
        int user_events[EVENT_ARRAY_LEN];
        if (argc == 2) {
            get_user_events(cpuid, &user_events[0], argv[1]);
        } else {
            get_user_events(cpuid, &user_events[0], EVENTS_CONFIG_FILENAME);
        }
        int specified_events = 0;
        int p = 0;
        //printf("Events: \n");
        for (p = 0; p < EVENT_ARRAY_LEN; p++) {
            if (user_events[p] >= 0) {
                specified_events++;
            }
            //printf("EVENT: %d\n", user_events[p]);
        }
        //printf("Specified events: %d\n", specified_events);
        // check number of events
        if (specified_events != num_counters) {
            printf("WARNING: events specified != number of counters\n");
        }
        set_events(&user_events[0], specified_events);
        int events_check[EVENT_ARRAY_LEN];
        for (p = 0; p < EVENT_ARRAY_LEN; p++) {
            events_check[p] = -1;
        }
        get_events(events_check);
        printf("CPU %d (0x%02X) Check (%d counters): ", i, cpuid, num_counters);
        for (p = 0; p < num_counters; p++) {
            printf("0x%02X, ", events_check[p]);
        }
        printf("\n");
    }
    fclose(cpu_data);
    return 0;
}


