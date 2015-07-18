/*
 *
 * Copyright  2012, 2013  Freescale Semiconductor
 *
 *
 * Licensed under the Apache License, version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *     Unless required by applicable law or agreed to in writing, software
 *     distributed under the License is distributed on an "AS IS" BASIS,
 *     WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *     See the License for the specific language governing permissions and
 *     limitations under the License.
*/

#include <pthread.h>
#include <stdint.h>
#include <urcu.h>
#include <string.h>
#include "cntlr_lock.h"
#include "cntlr_list.h"
#include "cntlr_system.h"
#include "mchash.h"
#include "crmapi.h"
#include "crmldef.h"
#include "viewldef.h"
#include "memapi.h"
#include "oflog.h"


int32_t of_show_stack_trace();

#define OFP_ETH_ALEN 6

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif


#ifndef __KERNEL__
#ifdef __GNUC__
#ifndef likely
#define likely(a)       __builtin_expect(!!(a), 1)
#endif
#ifndef unlikely
#define unlikely(a)     __builtin_expect(!!(a), 0)
#endif
#else
#ifndef likely
#define likely(a)       (a)
#endif
#ifndef unlikely
#define unlikely(a)     (a)  
#endif
#endif
#endif




#define cntlr_assert(cond)  do { \
                                if(!(cond)) \
                                { \
                                  OF_LOG_MSG(OF_LOG_MOD, OF_LOG_CRITICAL,"ASSERT: (%s) failed @ %s:%d\n",#cond,__FILE__,__LINE__);\
                                  of_show_stack_trace();\
                                }\
                             } while(0);

