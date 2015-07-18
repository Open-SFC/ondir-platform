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

/* File  :      tcp_api.c
 * Author:      Atmaram G <atmaram.g@freescale.com>
 * Date:   08/23/2013
 * Description: TCP plugin APIs.
 */

/******************************************************************************
 * * * * * * * * * * * * Include Files * * * * * * * * * * * * * * * * * *
 *******************************************************************************/


#include "controller.h"
#include <sys/mman.h>
#include <sys/ioctl.h>
#include "sll.h"
#include "memapi.h"
#include "memldef.h"
#include "dll.h"
#include "pktmbuf.h"
#include "pethapi.h"


/******************************************************************************
 * * * * * * * * * * * * Global Variables * * * * * * * * * * * * * * * * * *
 *******************************************************************************/

extern struct peth_char_dev_info *peth_char_dev_inst_g;
/******************************************************************************
 * * * * * * * * * * * * Function Prototypes * * * * * * * * * * * * * * * * * *
 *******************************************************************************/
int32_t peth_init();
int32_t register_mmap_with_cntlr();
int32_t of_mmap_init( );
extern int mempool_create_mmap_pool (struct mempool_params *pool_params_p,
      void*    mempool_p);
extern int32_t cntlr_register_memory_pool(int32_t mempool_type,
      struct of_mempool_callbacks *mempool_callbacks_p);

extern int32_t mempool_delete_mmap_pool(void* pool_p);

extern int32_t PSP_PKTPROC_FUNC mempool_mmap_get_mem_block(void* pool_p,
      uchar8_t **mem_block_p_p,
      uint8_t* heap_b_p);

extern int32_t mempool_mmap_release_mem_block(void*     pool_p,
      uchar8_t* mem_block_p,
      uchar8_t  heap_b);

struct of_mempool_callbacks  mmap_callbacks=
{
   mempool_create_mmap_pool,
   mempool_delete_mmap_pool,
   mempool_mmap_get_mem_block,
   mempool_mmap_release_mem_block
};


/******************************************** **********************************
 * * * * * * * F U N C T I O N   D E F I N I T I O N S* * * * * * * * * * * * *
 ******************************************************************************/
#ifdef CNTRL_INFRA_LIB_SUPPORT
void cntlr_shared_lib_init()
{
   int32_t ret_val=OF_SUCCESS;

   OF_LOG_MSG(OF_LOG_UTIL, OF_LOG_DEBUG,"Loading MMap Infra Library");

   do
   {
      /** Initialize NEM Infrastructure */
      ret_val=of_mmap_init();
      if(ret_val != OF_SUCCESS)
      {
         OF_LOG_MSG(OF_LOG_UTIL, OF_LOG_DEBUG, "MMAP initialization failed");
         break;
      }

   }while(0);

   return;
}
#endif


int32_t of_mmap_init( )
{
   int32_t status=OF_SUCCESS;
   int32_t ret_val=OF_SUCCESS;

   OF_LOG_MSG (OF_LOG_UTIL, OF_LOG_DEBUG,"entered");

   do
   {
      ret_val = register_mmap_with_cntlr();
      if(ret_val != OF_SUCCESS)
      {
         OF_LOG_MSG(OF_LOG_UTIL, OF_LOG_ERROR,"registartion with mmp failed");
         status = OF_FAILURE;
         break;
      }

   }while(0);
   return status;
}

int32_t register_mmap_with_cntlr()
{
   int32_t ret_val = OF_SUCCESS;
   int32_t status = OF_SUCCESS;

   OF_LOG_MSG(OF_LOG_UTIL, OF_LOG_DEBUG,"entered");
   do
   {
      ret_val = cntlr_register_memory_pool( MEMPOOL_TYPE_MMAP, &mmap_callbacks);
      if(ret_val != OF_SUCCESS)
      {
         OF_LOG_MSG(OF_LOG_UTIL, OF_LOG_ERROR,"registration memory pool - Failed");
         status = OF_FAILURE;
         break;
      }

#if 0
      ret_val = peth_init();
      if(ret_val != OF_SUCCESS)
      {
         OF_LOG_MSG(OF_LOG_UTIL, OF_LOG_ERROR,"peth initialization Failed");
         status = OF_FAILURE;
         break;
      }
#endif
   }while(0);

   return status;
}

int32_t peth_init()
{
   int32_t status = OF_SUCCESS;
    char                    peth_char_dev_cmd_str[80];


   OF_LOG_MSG(OF_LOG_UTIL, OF_LOG_DEBUG,"entered");

   do
   {
	   snprintf (peth_char_dev_cmd_str, sizeof(peth_char_dev_cmd_str),
			   "mknod %s c %d 0", PETH_CHAR_DRIVER,
			   PSEUDO_ETH_CHAR_DEV_MAJOR_NUM);
            OF_LOG_MSG(OF_LOG_UTIL, OF_LOG_DEBUG,"Command is |%s|", peth_char_dev_cmd_str);
	   if (system (peth_char_dev_cmd_str) == -1) {
		   OF_LOG_MSG (OF_LOG_UTIL, OF_LOG_ERROR,
				   "mknod command failed");
		   exit(0);
	   }

	   peth_char_dev_inst_g = calloc (1, sizeof(struct peth_char_dev_info));
	   if (peth_char_dev_inst_g == NULL) {
		   OF_LOG_MSG (OF_LOG_UTIL, OF_LOG_ERROR,
				   "Unable to allocate memory");
		   exit(0);
	   }
	   peth_char_dev_inst_g->fd = -1;

	   if ((peth_char_dev_inst_g->fd = open (PETH_CHAR_DRIVER, O_RDWR)) < 0) {
		   OF_LOG_MSG (OF_LOG_UTIL, OF_LOG_ERROR,
				   "Unable to open char driver");
		   free (peth_char_dev_inst_g);
		   exit(0);
	   }


   }while(0);

   return status;
}

#ifdef CNTRL_INFRA_LIB_SUPPORT
void cntlr_shared_lib_deinit()
{

   OF_LOG_MSG(OF_LOG_UTIL, OF_LOG_DEBUG,"UnLoading MMap Infra Library");

   do
   {
#if 0
      /** De-Initialize MMAP Infrastructure */
      ret_val=of_mmap_deinit();
      if(ret_val != OF_SUCCESS)
      {
         OF_LOG_MSG(OF_LOG_UTIL, OF_LOG_DEBUG, "MMAP De-initialization failed");
         status= OF_FAILURE;
         break;
      }
#endif
   }while(0);

   return;
}
#endif
