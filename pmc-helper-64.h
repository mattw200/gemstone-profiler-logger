#ifndef PMC_HELPER_H_
#define PMC_HELPER_H_

inline void init_pmcs();
inline void set_selected_counter(int counter);
inline void set_event_for_selected_counter(int event_id) ;
inline int get_event_for_selected_counter() ;
inline uint32_t get_count_for_selected_counter();
inline void get_six_event_ids(int *ids) ;
inline void get_six_counts(int *counts) ;
inline void set_six_event_ids(int events[6]) ;
inline uint32_t get_cpu_id_code();
inline uint32_t get_no_counters() ;

#endif
