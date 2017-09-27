#ifndef PMC_HELPER_H_
#define PMC_HELPER_H_

#define EVENT_ARRAY_LEN 10

 void init_pmcs();
 void set_selected_counter(int counter);
 void set_event_for_selected_counter(int event_id) ;
 int get_event_for_selected_counter() ;
 uint32_t get_count_for_selected_counter();
 uint32_t get_cycle_count() ;

 void get_six_event_ids(int *ids) ; // old
 void get_six_counts(int *counts) ; // old
 void set_six_event_ids(int events[6]) ; // old

 uint32_t get_cpu_id_code();
 uint32_t get_no_counters() ;


 void set_events(int *events, int num_events);
 void get_events(int *events);
 void get_counts(uint32_t *counts);

#endif
