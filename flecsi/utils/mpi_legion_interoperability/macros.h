#ifndef PTHREADS_MACROS_H
#define PTHREADS_MACROS_H

#ifdef DEBUG_THREAD
#define THREAD_TRACE(fmt, ...) fprintf(stderr, "[pone thread] [%lx] " fmt "\n", pthread_self(), ##__VA_ARGS__)
#else
#define THREAD_TRACE(fmt, ...)
#endif

#define CHECK_PTHREAD(code)                                     \
    do {                                                        \
        int r;                                                  \
        THREAD_TRACE("%s %s at %d", #code, __FILE__, __LINE__); \
        if ((r = (code)) != 0) {                                \
            errno = r;                                          \
            perror("pthread error: " #code);                    \
            abort();                                            \
        }                                                       \
    } while (0)
#endif 
