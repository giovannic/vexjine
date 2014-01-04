/*
 * DebugUtil.h: Various debugging methods for printing stack-traces, errors etc
 *
 *  Created on: 28 Jul 2010
 *      Author: nb605
 */

#ifndef DEBUGUTIL_H_
#define DEBUGUTIL_H_

#if defined(__x86_64__)
#define LIBUNWIND_CONSTANT_RAX UNW_X86_64_RAX
#define LIBUNWIND_CONSTANT_RDX UNW_X86_64_RDX
#define LIBUNWIND_CONSTANT_RCX UNW_X86_64_RCX
#define LIBUNWIND_CONSTANT_RBX UNW_X86_64_RBX
#else
#define LIBUNWIND_CONSTANT_RAX UNW_X86_EAX
#define LIBUNWIND_CONSTANT_RDX UNW_X86_EDX
#define LIBUNWIND_CONSTANT_RCX UNW_X86_ECX
#define LIBUNWIND_CONSTANT_RBX UNW_X86_EBX
#endif


#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <string>

#include <execinfo.h>
#include <syscall.h>


void vtfstacktrace(bool condition, FILE *log_file, const char *text);
int vtflog(bool condition, FILE *log_file, const char *fmt, ...);

std::string getCallingMethod();
std::string getAllCallingMethodsUntil(short maxStack);

void printError(const char *msg);

double vtf_drand();
int random_normal();
double busyWaiting(const float &durationInSec);

inline int mypow2(int exp) {
	return (1 << exp);
};

#endif /* DEBUGUTIL_H_ */
