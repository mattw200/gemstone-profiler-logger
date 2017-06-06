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

#define NUM_CORES 4


int main(int argc, char *argv[])
{
    int i;
    for (i = 0; i < NUM_CORES; i++) {
        unsigned long mask = 0 | (1<<i); //cpu 0
        unsigned int len = sizeof(mask);
        int result = sched_setaffinity(0, len, &mask);
        printf("Core: %d, Result: %d\n", i, result);
        // 1) Read PMCR_EL0 register (Performance Monitor Control Register)
        // (32-bit register) 
        uint32_t r = 0;
        asm volatile("mrs %0, PMCR_EL0" : "=r" (r));
        // print implementation code:
        printf("Implementation code: 0x%0x\n", r>>24);  
        // print identification code
        printf("Identification code: 0x%0x\n", (r>>16)&0xFF);  
        // print number of counters
        printf("Number of counters: %d\n", (r>>11)&0x1F);  
        init_pmcs(0);
        asm volatile("mrs %0, PMCR_EL0" : "=r" (r)); // read again
        printf("32 or 64 overflow: %"PRIu32"\n", r);
        uint64_t cycle_count = 0;
        //while (1) {
        //    asm volatile("mrs %0, PMCCNTR_EL0" : "=r" (cycle_count));
        //    printf("Cycle count: %"PRIu64"\n", cycle_count);
        //}
        //set events
        int events[6] = {0x11, 0x01, 0x02, 0x03, 0x04, 0x11};
        set_six_event_ids(events);
        int events_check[6];
        get_six_event_ids(&events_check[0]);
        printf("Event IDs check: ");
        int n = 0;
        for (n = 0; n < 6; n++) {
            printf("0x%0x  ", events_check[n]);
        }
        printf("\n");
    }	
    while (1) {
        int i;
        for (i = 0; i < NUM_CORES; i++) {
            unsigned long mask = 0 | (1<<i); //cpu 0
            unsigned int len = sizeof(mask);
            int result = sched_setaffinity(0, len, &mask);
            uint32_t event_counts[6];
            get_six_counts(&event_counts[0]) ;
            int n = 0;
            for (n = 0; n < 6; n++) {
                printf("0x%0x  \t", event_counts[n]);
            }
            printf("\n");
        }
        printf("\n\n");

    }
    sleep(1);
    return 0;
}


