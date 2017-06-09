
#include <stdio.h>
#include <stdint.h>
#include "pmc-helper-64.h"

inline void init_pmcs()
{
	// in general enable all counters
	int32_t value = 1;
	uint32_t div_ccnt_64 = 0; //don't divide cycle count
    value |= 2; //reset all counters to zero
    value |= 4; //reset cycle counter to zero
	if (div_ccnt_64)
	{
		value |= 8; //enable "divide by 64" for CCNT
	}
	value |= 16; 

#if __aarch64__
	// program the performace-counter control-register
    asm volatile("msr PMCR_EL0, %0" : : "r" (value));
    asm volatile("msr PMCNTENSET_EL0, %0" : : "r" (0x8000003f));
    asm volatile("msr PMOVSCLR_EL0, %0" : : "r" (0x8000003f));
#elif defined(__ARM_ARCH_7A__)
	// program the performace-counter control-register
	asm volatile("MCR p15, 0, %0, c9, c12, 0\t\n" :: "r"(value));
	asm volatile("MCR p15, 0, %0, c9, c12, 1\t\n" :: "r"(0x8000003f));
	asm volatile("MCR p15, 0, %0, c9, c12, 3\t\n" :: "r"(0x8000003f));
#else
#error This platform is not supported
#endif
}

inline void set_selected_counter(int counter)
{
#if __aarch64__
	unsigned long pmselr;
    asm volatile("mrs %0, PMSELR_EL0" : "=r" (pmselr));   
	pmselr &= ~(0x1F);
	pmselr |= (counter & 0x1F); 
    asm volatile("msr PMSELR_EL0, %0" : : "r" (pmselr));
#elif defined(__ARM_ARCH_7A__)
    unsigned long pmnxsel;
	asm volatile("MRC p15, 0, %0, c9, c12, 5\t\n" : "=r"(pmnxsel));
    //pmnxsel &= ~(0x03);
    //pmnxsel |= (counter & 0x03); 
    pmnxsel &= ~(0x1F);
    pmnxsel |= (counter & 0x1F); 
    writePMNXSEL(pmnxsel);
	asm volatile("MCR p15, 0, %0, c9, c12, 5\t\n" :: "r"(pmnxsel));

#else
#error This platform is not supported
#endif
}

inline void set_event_for_selected_counter(int event_id) 
{
#if __aarch64__
    asm volatile("msr PMXEVTYPER_EL0, %0" : : "r" (event_id));
#elif defined(__ARM_ARCH_7A__)
    asm volatile("MCR p15, 0, %0, c9, c13, 1\t\n" :: "r"(event_id));
#else
#error This platform is not supported
#endif
}

inline int get_event_for_selected_counter() 
{
    int event = 0;
#if __aarch64__
    asm volatile("mrs %0, PMXEVTYPER_EL0" : "=r" (event));
#elif defined(__ARM_ARCH_7A__)
	asm volatile ("MRC p15, 0, %0, c9, c13, 1\t\n" : "=r"(event));
#else
#error This platform is not supported
#endif
    return event;
}

inline uint32_t get_count_for_selected_counter()
{
    // Warning: it is a 32-bit register
    uint32_t value = 0;
#if __aarch64__
    asm volatile("mrs %0, PMXEVCNTR_EL0" : "=r" (value));
#elif defined(__ARM_ARCH_7A__)
	asm volatile("MRC p15, 0, %0, c9, c13, 2\t\n" : "=r"(value));
#else
#error This platform is not supported
#endif
    return value;
}

inline uint32_t get_cpu_id_code()
{
    uint32_t value = 0;
#if __aarch64__
    asm volatile("mrs %0, PMCR_EL0" : "=r" (value));
#elif defined(__ARM_ARCH_7A__)
	asm volatile("MRC p15, 0, %0, c9, c12, 0\t\n" : "=r"(value));
#else
#error This platform is not supported
#endif
    return (value>>16)&0xFF;
}

inline uint32_t get_no_counters() 
{
    uint32_t value = 0;
#if __aarch64__
    asm volatile("mrs %0, PMCR_EL0" : "=r" (value));
#elif defined(__ARM_ARCH_7A__)
	asm volatile("MRC p15, 0, %0, c9, c12, 0\t\n" : "=r"(value));
#else
#error This platform is not supported
#endif
    return (value>>11)&0x1F;
}

inline void set_events(int *events, int num_events)
{
    int num_counters = get_no_counters();
    int i;
    for (i = 0; i < num_events; i++) {
        if (i>=num_counters) {
            printf("WARNING: more events specified than counters!");
            break;
        }
        set_selected_counter(i);
        set_event_for_selected_counter(events[i]);
    } 
}

inline void get_events(int *events)
{
    int num_counters = get_no_counters();
    int i;
    for (i = 0; i < num_counters; i++) {
       set_selected_counter(i);
       events[i] = get_event_for_selected_counter();
    }
}

inline void get_counts(uint32_t *counts)
{
    int num_counters = get_no_counters();
    int i;
    for (i = 0; i < num_counters; i++) {
       set_selected_counter(i);
       counts[i] = get_count_for_selected_counter();
    }
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
