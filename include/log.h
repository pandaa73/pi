#ifndef __PANDAA73_LOG_H
#define __PANDAA73_LOG_H

#include <stdio.h>
#include <stdlib.h>

#if defined(PLG_LOGLEVEL_INFO)
    #define PLG_INFO(...) do {\
        fprintf(\
            stderr,\
            "INFO: %s@%d: %s\n",\
            __FILE__, __LINE__,\
            ##__VA_ARGS__\
        );\
    } while(0)
#else
    #define PLG_INFO(...) do {} while(0)
#endif

#if defined(PLG_LOGLEVEL_INFO) || defined(PLG_LOGLEVEL_WARN)
    #define PLG_WARN(...) do {\
        fprintf(\
            stderr,\
            "WARN: %s@%d: %s\n",\
            __FILE__, __LINE__,\
            ##__VA_ARGS__\
        );\
    } while(0)
#else
    #define PLG_WARN(...) do {} while(0)
#endif

#if defined(PLG_LOGLEVEL_INFO) || defined(PLG_LOGLEVEL_WARN)\
        || defined(PLG_LOGLEVEL_ERROR)
    #define PLG_ERROR(...) do {\
        fprintf(\
            stderr,\
            "ERROR: %s@%d: %s\n",\
            __FILE__, __LINE__,\
            ##__VA_ARGS__\
        );\
    } while(0)
#else
    #define PLG_ERROR(...) do {} while(0)
#endif

#if defined(PLG_LOGLEVEL_INFO) || defined(PLG_LOGLEVEL_WARN)\
        || defined(PLG_LOGLEVEL_ERROR) || defined(PLG_LOGLEVEL_FATAL)
    #define PLG_FATAL(...) do {\
        fprintf(\
            stderr,\
            "FATAL: %s@%d: %s\n",\
            __FILE__, __LINE__,\
            ##__VA_ARGS__\
        );\
        exit(EXIT_FAILURE);\
    } while(0)
#else
    #define PLG_FATAL(...) do { exit(EXIT_FAILURE); } while(0)
#endif

#endif /* __PANDAA73_LOG_H */
