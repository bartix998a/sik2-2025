#ifndef MIM_ERR_H
#define MIM_ERR_H

#include <stdnoreturn.h>

#ifdef __cplusplus
#define  noreturn [[noreturn]]
extern "C" {
#endif

// Print information about a system error and quits.
noreturn void syserr(const char *fmt, ...);

// Print information about an error and quits.
noreturn void fatal(const char *fmt, ...);

// Print information about an error and return.
void error(const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif
