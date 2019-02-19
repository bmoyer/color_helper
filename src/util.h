#ifndef UTIL_H
#define UTIL_H 

#define debug_print(...) \
            do { if (DEBUG) fprintf(stderr, __VA_ARGS__); } while (0)

#define LOG_TIMING(a, msg) \
    do { \
        struct timeval tval_before, tval_after, tval_result; \
        gettimeofday(&tval_before, NULL); \
        a; \
        gettimeofday(&tval_after, NULL); \
        timersub(&tval_after, &tval_before, &tval_result); \
        debug_print("%s/%s: %ld.%06ld sec\n", __FILE__, msg, (long int)tval_result.tv_sec, (long int)tval_result.tv_usec); \
    } while (0)

#define LOG_TID() debug_print("%s::%s\tTID = %ld\n", __FILE__, __func__, syscall(SYS_gettid));

struct timeval get_time() {
    struct timeval tval;
    gettimeofday(&tval, NULL);
    return tval;
}

long get_usec_elapsed(struct timeval* after, struct timeval* before) {
    struct timeval result;
    timersub(after, before, &result);
    return result.tv_sec * 1000000L + result.tv_usec;
}

#endif /* UTIL_H */
