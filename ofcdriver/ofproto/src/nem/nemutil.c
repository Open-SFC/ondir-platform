/**
 * Copyright (c) 2012, 2013  Freescale.
 *  
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 * 
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 **/

/* File  :      nemutil.c
 * Author:      Atmaram G <atmaram.g@freescale.com>
 * Date:   08/23/2013
 * Description: A network map_entry is logical mapping between network elements. Network Element Mapper module maintains mappings between different entities.This file provides utility functions used in different Mapping databases.

 */
#ifdef CNTRL_NETWORK_ELEMENT_MAPPER_SUPPORT
#include "controller.h"
#include "dprm.h"
#include "cntlr_epoll.h"
#include "cntrl_queue.h"
#include "of_utilities.h"
#include "of_msgapi.h"
#include "of_multipart.h"
#include "cntlr_tls.h"
#include "cntlr_transprt.h"
#include "cntlr_event.h"
#include "dprmldef.h"
#include "of_nem.h"
#include "nemldef.h"

/******************************************************************************
 * * * * * * * * * utility function prototypes * * * * * * * * * * * * * * * * * 
 *******************************************************************************/
uint32_t nem_get_hashval_by_32bitvalue(uint32_t value_32,uint32_t no_of_buckets);
uint32_t nem_get_hashval_by_64bitvalue_and_32bitvalue(uint64_t value_64, uint32_t value_32,uint32_t no_of_buckets);
uint32_t nem_get_hashval_by_64bitvalue_and_string(uint64_t value_64, char *string_p,uint32_t no_of_buckets);
uint32_t nem_get_hashval_by_32bitvalue_and_string(uint32_t value_32, char *string_p,uint32_t no_of_buckets);
uint32_t nem_get_hashval_by_32bitvalue_and_32bitvalue(uint32_t value1_32, uint32_t value2_32,uint32_t no_of_buckets);


/******************************************************************************
 * * * * * * * * * * * * HASH utilities * * * * * * * * * * * * * * * * * * *
 *******************************************************************************/

uint32_t nem_get_hashval_by_32bitvalue(uint32_t value_32,uint32_t no_of_buckets)
{
   uint32_t hashkey,param1,param2,param3,param4;

   param1 = (uint32_t)(value_32);
   param2 = 1;
   param3 = 1;
   param4 = 1;
   DPRM_COMPUTE_HASH(param1,param2,param3,param4,no_of_buckets,hashkey);
   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "hashkey %d", hashkey);
   return hashkey;
}


uint32_t nem_get_hashval_by_64bitvalue_and_32bitvalue(uint64_t value_64, uint32_t value_32,uint32_t no_of_buckets)
{
   uint32_t hashkey,param1,param2,param3,param4;

   param1 = (uint32_t)(value_64 >>32);
   param2 = value_64;
   param3 = value_32;
   param4 = 1;
   DPRM_COMPUTE_HASH(param1,param2,param3,param4,no_of_buckets,hashkey);
   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "hashkey %d", hashkey);
   return hashkey;
}

uint32_t nem_get_hashval_by_64bitvalue_and_string(uint64_t value_64, char *string_p,uint32_t no_of_buckets)
{
   uint32_t hashkey,param1,param2,param3,param4;
   char name_hash[8];
   memset(name_hash,0,8);
   strncpy(name_hash,string_p,8);

   param1 = (uint32_t)(value_64 >>32);
   param2 = value_64;
   param3 =  *(uint32_t *)(name_hash +0);
   param4 =  *(uint32_t *)(name_hash +4);
   DPRM_COMPUTE_HASH(param1,param2,param3,param4,no_of_buckets,hashkey);
   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "hashkey %d", hashkey);
   return hashkey;
}

uint32_t nem_get_hashval_by_32bitvalue_and_string(uint32_t value_32, char *string_p,uint32_t no_of_buckets)
{
   uint32_t hashkey,param1,param2,param3,param4;
   char name_hash[8];
   memset(name_hash,0,8);
   strncpy(name_hash,string_p,8);

   param1 = (uint32_t)(value_32 );
   param2 = 1;
   param3 =  *(uint32_t *)(name_hash +0);
   param4 =  *(uint32_t *)(name_hash +4);
   DPRM_COMPUTE_HASH(param1,param2,param3,param4,no_of_buckets,hashkey);
   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "hashkey %d", hashkey);
   return hashkey;
}

uint32_t nem_get_hashval_by_32bitvalue_and_32bitvalue(uint32_t value1_32, uint32_t value2_32,uint32_t no_of_buckets)
{
   uint32_t hashkey,param1,param2,param3,param4;

   param1 = (uint32_t)(value1_32 );
   param2 = (uint32_t)(value2_32 );
   param3 =  1;
   param4 = 1;
   DPRM_COMPUTE_HASH(param1,param2,param3,param4,no_of_buckets,hashkey);
   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "hashkey %d", hashkey);
   return hashkey;
}


uint32_t nem_get_hashval_by_dpid_and_nsname(uint64_t dpid, char *name_p,uint32_t no_of_buckets)
{
   return (nem_get_hashval_by_64bitvalue_and_string(dpid, name_p, no_of_buckets));
}

uint32_t nem_get_hashval_by_dpid_and_portname(uint64_t dpid, char *name_p,uint32_t no_of_buckets)
{
   return (nem_get_hashval_by_64bitvalue_and_string(dpid, name_p, no_of_buckets));
}

uint32_t nem_get_hashval_by_dpid_and_portid(uint64_t dpid, uint32_t port_id,uint32_t no_of_buckets)
{
   return (nem_get_hashval_by_64bitvalue_and_32bitvalue(dpid, port_id, no_of_buckets));
}

uint32_t nem_get_hashval_by_nsid_and_peth(uint32_t nsid, char *name_p,uint32_t no_of_buckets)
{
   return (nem_get_hashval_by_32bitvalue_and_string(nsid, name_p, no_of_buckets));
}

uint32_t nem_get_hashval_by_nsid_and_peth_id(uint32_t nsid, uint32_t peth_id,uint32_t no_of_buckets)
{
   return (nem_get_hashval_by_32bitvalue_and_32bitvalue(nsid, peth_id, no_of_buckets));
}

uint32_t nem_get_hashval_by_dpid_and_nsid(uint64_t dpid, uint32_t nsid,uint32_t no_of_buckets)
{
   return (nem_get_hashval_by_64bitvalue_and_32bitvalue( dpid, nsid, no_of_buckets));
}

uint32_t nem_get_hashval_by_nsid(uint32_t nsid,uint32_t no_of_buckets)
{
   return (nem_get_hashval_by_32bitvalue(nsid, no_of_buckets));
}

int32_t nem_set_cur_task_to_name_space (char *ns_name, int32_t *ns_fd_p)
{
	char            ns_file[80];
	int32_t         ns_fd;
	int32_t status=OF_SUCCESS;

	do
	{
		snprintf (ns_file, sizeof(ns_file), "/var/run/netns/%s", ns_name);
		ns_fd = open (ns_file, O_RDONLY); /* open fd belongs to netns */
		if (ns_fd < 0) {
			OF_LOG_MSG (OF_NEM_LOG, OF_LOG_ERROR,
					"Failed to open file with name :%s", ns_file);
			status=OF_FAILURE;
		        break;
		}

		/* temporarily associate current thread to target netns */
		if (setns (ns_fd, CLONE_NEWNET) < 0) {
			OF_LOG_MSG (OF_NEM_LOG, OF_LOG_ERROR,
					"Failed to set ns with name :%s", ns_file);
			close (ns_fd);
			status=OF_FAILURE;
		        break;
		}
		*ns_fd_p = ns_fd;

	}while(0);

	return status;
}


int32_t nem_set_cur_task_to_root_name_space (int32_t root_ns_fd, int32_t ns_fd)
{
	int32_t status=OF_SUCCESS;

	do
	{
		if (setns (root_ns_fd, CLONE_NEWNET) < 0)
		 {
			OF_LOG_MSG (OF_NEM_LOG, OF_LOG_ERROR,
					"Failed to set the current task to root ns");
			close(ns_fd);
		}
	}while(0);

	return status;
}
#endif /* CNTRL_NETWORK_ELEMENT_MAPPER_SUPPORT*/
