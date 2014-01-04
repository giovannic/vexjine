/*
 * perfdirect.h
 *
 *  Created on: 26 Jul 2010
 *      Author: nb605
 */

#ifndef PERFDIRECT_H_
#define PERFDIRECT_H_

/*
 * perfdirect.c
 *
 *  Created on: 26 Jul 2010
 *      Author: nb605
 */

#include "libperfctr.h"
#include "arch.h"

void PERFCTR_initialize();
struct vperfctr * PERFCTR_register_thread();
void PERFCTR_unregister_thread();
long long PERFCTR_getVT();
long long PERFCTR_getVT_for(struct vperfctr* perfdirect_threadglobalref);
long long PERFCTR_getRT();
#endif /* PERFDIRECT_H_ */
