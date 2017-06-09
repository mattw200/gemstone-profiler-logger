#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>  
#include <stdlib.h>
#include <inttypes.h>
#include "pmc-helper-64.h"

#include <sched.h>

#define HEADER_LEN 4096

#define PLATFORM_ODROID_C2 1

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
    //TODO remove previous online/offline check and put a per-core one
    struct timespec milt;
    clock_gettime(CLOCK_REALTIME, &milt);
    int64_t millitime = milt.tv_sec * INT64_C(1000) + milt.tv_nsec / 1000000;
    //printf("Millitime: %f\n", (double)millitime);
    //printf("Millitime int: %"PRId64"\n", millitime);
    printf("%"PRId64"",millitime);
    //char header[HEADER_LEN];
    int i = 0;
    for (i = 0; i < num_cpus; i++) {
        unsigned long mask = 0 | (1<<i); //cpu 0
        unsigned int len = sizeof(mask);
        int result = sched_setaffinity(0, len, &mask);
        //printf("Core: %d, Result: %d\n", i, result);
        uint32_t cpuid = get_cpu_id_code();
        uint32_t num_counters = get_no_counters();
        num_counters = get_no_counters();
        uint32_t counts[num_counters];
        get_counts(&counts[0]);
        //char field[512];
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
    //printf("%s\n", header);
    return 0;
}
