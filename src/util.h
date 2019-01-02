#ifndef UTIL_H
#define UTIL_H 

#define debug_print(...) \
            do { if (DEBUG) fprintf(stderr, __VA_ARGS__); } while (0)

#endif /* UTIL_H */
