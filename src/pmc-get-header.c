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

int main(int argc, char *argv[])
{
    // get number of CPUs (both offline and online)
    unsigned long num_cpus = sysconf (_SC_NPROCESSORS_CONF);
    unsigned long online_cpus = sysconf (_SC_NPROCESSORS_ONLN);
    //printf("number of CPUs configured: %lu\n", num_cpus);
    //printf("number of CPUs online: %lu\n", num_cpus);
    if (num_cpus != online_cpus) {
        fprintf(stderr, "FATAL ERROR: Some CPUs are offline. All must be online for setup.\n");
        exit(-1);
    }
	printf("milliseconds\tdatetime\tlabel");
    int i = 0;
    for (i = 0; i < num_cpus; i++) {
        unsigned long mask = 0 | (1<<i); //cpu 0
        unsigned int len = sizeof(mask);
        int result = sched_setaffinity(0, len, &mask);
        //printf("Core: %d, Result: %d\n", i, result);
        uint32_t cpuid = get_cpu_id_code();
        uint32_t num_counters = get_no_counters();
        //printf("Core: %d, id: %u, no. counters: %u\n", i, cpuid, num_counters);
        num_counters = get_no_counters();
        int events[num_counters];
        get_events(&events[0]);
        printf("\tCPU %d (id:0x%02X) cycle count", i, cpuid);
        int p;
        for (p = 0; p < num_counters; p++) {
            printf("\tCPU %d (id:0x%02X) cntr %d (0x%02X)", i, cpuid, p, events[p]);
        }
    }
#ifdef PLATFORM_ODROID_C2
    printf("\tcluster temperature");
#endif

    printf("\n");
    return 0;
}


