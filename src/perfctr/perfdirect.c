/*
 * perfdirect.c
 *
 *  Created on: 26 Jul 2010
 *      Author: nb605
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "libperfctr.h"
#include "arch.h"

__thread struct vperfctr* __perfdirect_threadglobalref = NULL;
static long long __perfdirect_mhz = 0.0;
char *search_cpu_info(FILE * f, char *search_str, char *line)
{
	/* This function courtesy of Rudolph Berrendorf! */
	/* See the home page for the German version of PAPI. */
	char *s;
	while (fgets (line, 256, f) != NULL)
	{
		if (strstr (line, search_str) != NULL)
		{
			/* ignore all characters in line up to : */
			for (s = line; *s && (*s != ':'); ++s);
			if (*s)
				return s;
		}
	}
	return NULL;
}

long long get_cpu_info()
{
	char maxargs[256], *s;
	float lmhz = 0.0;
	FILE *f;

	if ((f = fopen ("/proc/cpuinfo", "r")) == NULL)
	{
		return -1;
	}

	/* All of this information maybe overwritten by the substrate */

	/* MHZ */
	rewind(f);
	s = search_cpu_info(f, "clock", maxargs);
	if (!s) {
		rewind(f);
		s = search_cpu_info(f, "cpu MHz", maxargs);
	}
	if (s)
		sscanf(s + 1, "%f", &lmhz);

//	printf("ta mhz diavastikan se %f\n", lmhz);
	return lmhz;
}


void PERFCTR_initialize() {
	__perfdirect_mhz = get_cpu_info();
}

struct vperfctr *PERFCTR_register_thread() {
	struct perfctr_info info;
	if (__perfdirect_threadglobalref == NULL) {

		__perfdirect_threadglobalref = vperfctr_open();
		if( !__perfdirect_threadglobalref ) {
			perror("vperfctr_open in register_thread");
			exit(1);
		}
		if( vperfctr_info(__perfdirect_threadglobalref, &info) < 0 ) {
			perror("vperfctr_info in register_thread");
			exit(1);
		}

		struct vperfctr_control control;
		memset(&control, 0, sizeof control);
		do_setup(&info, &control.cpu_control);
		if( vperfctr_control(__perfdirect_threadglobalref, &control) < 0 ) {
			perror("vperfctr_control in register_thread");
			exit(1);
		}
	}
	return __perfdirect_threadglobalref;
}


void PERFCTR_unregister_thread() {
	if (__perfdirect_threadglobalref != NULL) {
		vperfctr_close(__perfdirect_threadglobalref);
		__perfdirect_threadglobalref = NULL;
	}
}

long long PERFCTR_getVT() {
	struct vperfctr* perfdirect_threadglobalref = __perfdirect_threadglobalref;
	if (perfdirect_threadglobalref == NULL) {
		perfdirect_threadglobalref = PERFCTR_register_thread();
		if (__perfdirect_mhz == 0.0) {
			PERFCTR_initialize();
		}
	}
	return ((vperfctr_read_tsc(perfdirect_threadglobalref)*1000LL)/(long long)__perfdirect_mhz);
}


long long PERFCTR_getVT_for(struct vperfctr* perfdirect_threadglobalref) {
	return ((vperfctr_read_tsc(perfdirect_threadglobalref)*1000LL)/(long long)__perfdirect_mhz);
}


inline long long get_cycles() {
   long long ret = 0;
#ifdef __x86_64__
   do {
      unsigned int a,d;
      asm volatile("rdtsc" : "=a" (a), "=d" (d));
      (ret) = ((long long)a) | (((long long)d)<<32);
   } while(0);
#else
   __asm__ __volatile__("rdtsc"
                       : "=A" (ret)
                       : );
#endif
   return ret;
}

long long PERFCTR_getRT() {
    return ((get_cycles()*1000LL)/(long long)__perfdirect_mhz);
}
/*
int main(void)
{

	struct vperfctr *__perfdirect_threadglobalref = registerCurrentThread();
	long long start, end;

	start = getVT(__perfdirect_threadglobalref);
	int i ;
	double temp = 0.0;
	for (i = 0; i<217680000; i++) {
		temp += i;
	}
	end= getVT(__perfdirect_threadglobalref);

	printf("tell me what it takes %lld\n", (end-start));
	long long lmhz = (((end-start)*1000LL)/(long long)__perfdirect_mhz);//mhz;
	printf("time (?) is %lld\n",  lmhz);

	return 0;
}
*/
