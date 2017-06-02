
#include <stdio.h>
#include <stdint.h>
#include "pmc-helper-64.h"

inline void 
initialisePerformanceCounters(int32_t do_reset, int32_t enable_divider)
{
	// in general enable all counters
	int32_t value = 1;
	
	//perform reset
	if (do_reset)
	{
		value |= 2; //reset all counters to zero
		value |= 4; //reset cycle counter to zero
	}

	if (enable_divider)
	{
		value |= 8; //enable "divide by 64" for CCNT
	}

	value |= 16; 

	// program the performace-counter control-register
	//asm volatile("MCR p15, 0, %0, c9, c12, 0\t\n" :: "r"(value));
    asm volatile("msr PMCR_EL0, %0" : : "r" (value));

    asm volatile("msr PMCNTENSET_EL0, %0" : : "r" (0x8000003f));
    asm volatile("msr PMOVSCLR_EL0, %0" : : "r" (0x8000003f));
	
	//enable all counters
	//Write to Count Enable Set (CNTENS) Register
	//asm volatile("MCR p15, 0, %0, c9, c12, 1\t\n" :: "r"(0x8000000f));
	//asm volatile("MCR p15, 0, %0, c9, c12, 1\t\n" :: "r"(0x8000003f));//32-bit

	// clear overflows
	//asm volatile("MCR p15, 0, %0, c9, c12, 3\t\n" :: "r"(0x8000000f)); //32 bit
	
}

inline void set_selected_counter(int counter)
{
	unsigned long pmselr;
    asm volatile("mrs %0, PMSELR_EL0" : "=r" (pmselr));   
	pmselr &= ~(0x1F);
	pmselr |= (counter & 0x1F); 
    asm volatile("msr PMSELR_EL0, %0" : : "r" (pmselr));
}

inline void set_event_for_selected_counter(int event_id) 
{
    asm volatile("msr PMXEVTYPER_EL0, %0" : : "r" (event_id));
}

inline int get_event_for_selected_counter() 
{
    int event = 0;
    asm volatile("mrs %0, PMXEVTYPER_EL0" : "=r" (event));
    return event;
}

inline uint32_t get_count_for_selected_counter()
{
    // Warning: it is a 32-bit register
    uint32_t value = 0;
    asm volatile("mrs %0, PMXEVCNTR_EL0" : "=r" (value));
    return value;
}


inline void get_six_event_ids(int *ids) 
{
    int i;
    for (i = 0; i <6; i++) {
        set_selected_counter(i);
        ids[i] = get_event_for_selected_counter();
    }
}

inline void get_six_counts(int *counts) 
{
    int i;
    for (i = 0; i <6; i++) {
        set_selected_counter(i);
        counts[i] = get_count_for_selected_counter();
    }
}

inline void set_six_event_ids(int events[6]) 
{
    int i;
    for (i = 0; i < 6; i++) {
        set_selected_counter(i);
        set_event_for_selected_counter(events[i]);
    }
}
