#ifndef UTIL_H
#define UTIL_H 

#define debug_print(...) \
            do { if (DEBUG) fprintf(stderr, __VA_ARGS__); } while (0)

#define LOG_TIMING(a) \
    do { \
        clock_t start = clock(); \
        a; \
        clock_t stop = clock(); \
        debug_print("%s/%s: %f ms\n", __FILE__, __func__, (double)(stop - start) / (1000*CLOCKS_PER_SEC)); \
    } while (0)

#define LOG_TID() debug_print("%s::%s\tTID = %ld\n", __FILE__, __func__, syscall(SYS_gettid));

#endif /* UTIL_H */
