//WARNING! only compatible with compilers that support __VA_OPT__()
#ifndef DEBUG_H_INCLUDED
#define DEBUG_H_INCLUDED
#include <stdio.h>  //for stderr and printf

//OPTIONS
#define DEBUG 1       //Defined with codeblocks configuration depending on build type
//#define DEBUG_FILEINFO
//OPTIONS END


#define DBGT_ERROR "[ERROR]"
#define DBGT_INFO  "[INFO]"
//do{}while is used to "swallow" the semicolon when using dprintf(...);
#ifdef DEBUG_FILEINFO
    #define dprintf(severity,inpstrg, ...) \
        do { if(DEBUG) fprintf(stderr, "%s\t fi: %s,fc: %s,ln: %d\n" inpstrg "\n\n",severity,__FILE__,__func__,__LINE__ __VA_OPT__(,) __VA_ARGS__); } while(0)
#else
    #define dprintf(severity,inpstrg, ...) \
        do { if(DEBUG) fprintf(stderr, "%s\t fc: %s,ln: %d\n" inpstrg "\n\n",severity,__func__,__LINE__ __VA_OPT__(,) __VA_ARGS__); } while(0)
#endif //
#endif // DEBUG_H_INCLUDED
