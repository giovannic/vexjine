/*
 * DebugUtil.cpp
 *
 *  Created on: 28 Jul 2010
 *      Author: nb605
 */

#include "DebugUtil.h"

#include <sstream>

#ifdef USING_LIBUNWIND
#include <libunwind.h>
#endif

#define MAX_STACKDEPTH_SIZE 20
#define _USE_MATH_DEFINES
#include <math.h>


/*
 * Return a random double
 */
double vtf_drand() {
	return (rand() + 1.0) / (RAND_MAX + 1.0);

}

/*
 * Generate an integer with a normal distribution (0, 1)
 */
int random_normal() {
	return (2 * sqrt(-2 * log(vtf_drand())) * cos(2 * M_PI * vtf_drand())) / 1;
}


/*
 * Keep the CPU busy for an approximate number of seconds (clearly differs according to CPU)
 */
double busyWaiting(const float &durationInSec) {
	long iterations = 108840000 * durationInSec;
	double temp = 0.0;
	for (long i  =0 ; i<iterations; i++) {
		temp += 1;
	}
	return temp;
}

/*************************************************************************
 ****
 **** LOGGING FUNCTIONS
 ****
 ************************************************************************/

/*
 * Print to stderr
 */
void printError(const char *msg) {
	fprintf(stderr, "ERROR: %s\n", msg);
	fflush(stderr);
}

/*
 * Conditional logging function in the format of fprintf
 */
int vtflog(bool condition, FILE *log_file, const char *fmt, ...) {
	if (condition) {
		va_list ap;                                /* special type for variable    */
		char format[256];                /* argument lists               */
		int count = 0;
		int i, j;                                  /* Need all these to store      */
		char c;                                    /* values below in switch       */
		double d;
		unsigned u;
		char *s;
		void *v;
		long long l;

		va_start(ap, fmt);                         /* must be called before work   */
		while (*fmt) {
			for (j = 0; fmt[j] && fmt[j] != '%'; j++)
				format[j] = fmt[j];                    /* not a format string          */
			if (j) {
				format[j] = '\0';
				count += fputs(format, log_file);		/* it verbatim              */
				fflush(log_file);
				fmt += j;
			} else {
				for (j = 0; !isalpha(fmt[j]); j++) {   /* find end of format specifier */
					format[j] = fmt[j];
					if (j && fmt[j] == '%')              /* special case printing '%'    */
						break;
				}
				format[j] = fmt[j];                    /* finish writing specifier     */
				format[j + 1] = '\0';                  /* don't forget NULL terminator */
				fmt += j + 1;

				switch (format[j]) {                   /* cases for all specifiers     */
				case 'l':
					format[j + 1] = 'l';
					format[j + 2] = 'd';
					format[j + 3] = '\0';
					fmt += 2;
					l = va_arg(ap, long long);                 /* process the argument         */
					count += fprintf(log_file, format, l); 		/* and log it                 */
					fflush(log_file);
					break;
				case 'd':
					i = va_arg(ap, int);                 /* process the argument         */
					count += fprintf(log_file, format, i); /* and log it                 */
					fflush(log_file);
					break;
				case 'i':                              /* many use identical actions   */
					i = va_arg(ap, int);                 /* process the argument         */
					count += fprintf(log_file, format, i); /* and log it                 */
					fflush(log_file);
					break;
				case 'o':
				case 'x':
				case 'X':
				case 'u':
					u = va_arg(ap, unsigned);
					count += fprintf(log_file, format, u);
					fflush(log_file);
					break;
				case 'c':
					c = (char) va_arg(ap, int);          /* must cast!                   */
					count += fprintf(log_file, format, c);
					fflush(log_file);
					break;
				case 's':
					s = va_arg(ap, char *);
					count += fprintf(log_file, format, s);
					fflush(log_file);
					break;
				case 'f':
				case 'e':
				case 'E':
				case 'g':
				case 'G':
					d = va_arg(ap, double);
					count += fprintf(log_file, format, d);
					fflush(log_file);
					break;
				case 'p':
					v = va_arg(ap, void *);
					count += fprintf(log_file, format, v);
					fflush(log_file);
					break;
				case 'n':
					count += fprintf(log_file, "%d", count);
					fflush(log_file);
					break;
				case '%':
					count += fprintf(log_file, "%%");
					fflush(log_file);
					break;
				default:
					fprintf(stderr, "Invalid format specifier in log().\n");
				}
			}
		}

		va_end(ap);                                /* clean up                     */


		return count;
	} else {
		return 0;
	}
}


/*
 * Print the calling method
 */
std::string getCallingMethod() {
#ifdef USING_LIBUNWIND
	std::stringstream stemp;

	unw_cursor_t    cursor;
	unw_context_t   context;

	unw_getcontext(&context);
	unw_init_local(&cursor, &context);
	const short maxStack = 3;
	short current_stack_depth = 0;
	while (unw_step(&cursor) > 0 && current_stack_depth++ < maxStack) {
		unw_word_t  offset;
		unw_word_t  pc, eax, ebx, ecx, edx;
		char        fname[64];

		unw_get_reg(&cursor, UNW_REG_IP,  &pc);
		unw_get_reg(&cursor, LIBUNWIND_CONSTANT_RAX, &eax);
		unw_get_reg(&cursor, LIBUNWIND_CONSTANT_RDX, &edx);
		unw_get_reg(&cursor, LIBUNWIND_CONSTANT_RCX, &ecx);
		unw_get_reg(&cursor, LIBUNWIND_CONSTANT_RBX, &ebx);

		fname[0] = '\0';
		unw_get_proc_name(&cursor, fname, sizeof(fname), &offset);
		if (current_stack_depth > 2) {
			stemp << fname;
		}

	}
	return stemp.str().replace(0,26," ");
#else
	return std::string("Cannot return stack-trace without libunwind");
#endif
}


/*
 * Print all methods higher on the stack trace using libunwind
 */
std::string getAllCallingMethodsUntil(short maxStack) {
#ifdef USING_LIBUNWIND

	std::stringstream stemp;

	unw_cursor_t    cursor;
	unw_context_t   context;

	unw_getcontext(&context);
	unw_init_local(&cursor, &context);
	short current_stack_depth = 0;
	while (current_stack_depth++ < maxStack && unw_step(&cursor) > 0) {
		unw_word_t  offset;
		unw_word_t  pc, eax, ebx, ecx, edx;
		char        fname[128];

		unw_get_reg(&cursor, UNW_REG_IP,  &pc);
		unw_get_reg(&cursor, LIBUNWIND_CONSTANT_RAX, &eax);
		unw_get_reg(&cursor, LIBUNWIND_CONSTANT_RDX, &edx);
		unw_get_reg(&cursor, LIBUNWIND_CONSTANT_RCX, &ecx);
		unw_get_reg(&cursor, LIBUNWIND_CONSTANT_RBX, &ebx);

		fname[0] = '\0';
		unw_get_proc_name(&cursor, fname, sizeof(fname), &offset);
		if (current_stack_depth > 1) {
			stemp << fname << "#";
		}

	}
	return stemp.str();
#else
	return std::string("Cannot return stack-trace without libunwind");
#endif
}


/*
 * Print all methods higher on the stack trace using backtrace
 */
void vtfstacktrace(bool condition, FILE *log_file, const char *text) {
#ifdef GDB_USAGE

#ifdef USING_LIBUNWIND
	unw_cursor_t    cursor;
	unw_context_t   context;

	unw_getcontext(&context);
	unw_init_local(&cursor, &context);
	const short maxStack = 20;
	short current_stack_depth = 0;
	printf("************ %s **************\n", text);
	while (unw_step(&cursor) > 0 && current_stack_depth++ < maxStack) {
		unw_word_t  offset;
		unw_word_t  pc, eax, ebx, ecx, edx;
		char        fname[64];

		unw_get_reg(&cursor, UNW_REG_IP,  &pc);
		unw_get_reg(&cursor, LIBUNWIND_CONSTANT_RAX, &eax);
		unw_get_reg(&cursor, LIBUNWIND_CONSTANT_RDX, &edx);
		unw_get_reg(&cursor, LIBUNWIND_CONSTANT_RCX, &ecx);
		unw_get_reg(&cursor, LIBUNWIND_CONSTANT_RBX, &ebx);

		fname[0] = '\0';
		unw_get_proc_name(&cursor, fname, sizeof(fname), &offset);
		printf ("%p : (%s+0x%x) [%p]\n", (void *)pc, fname, (unsigned int)offset, (void *)pc);

	}
	printf("**************************\n\n");
#endif

#else
	if (condition) {
		int j, nptrs;

		void *buffer[MAX_STACKDEPTH_SIZE];
		char **strings;

		nptrs = backtrace(buffer, MAX_STACKDEPTH_SIZE);
		fprintf(log_file,"\n**************************\n%s in (%d)\n",text, nptrs);

		strings = backtrace_symbols(buffer, nptrs);
		if (strings == NULL) {
			perror("ERROR acquiring backtrace_symbols");
			return;
		}

		// starting from 1, we don't care about vtfstacktrace function
		for (j = 1; j < nptrs; j++) {
			fprintf(log_file, "%s\n", strings[j]);
		}

		fprintf(log_file, "**************************\n");
		fflush(log_file);
		free(strings);
	}

#endif
}
