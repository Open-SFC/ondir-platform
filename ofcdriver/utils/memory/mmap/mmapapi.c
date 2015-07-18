
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


#define MAP_HUGETLB     0x40000

/******************************************************************************
 * * * * * * * * * * * * Function Prototypes * * * * * * * * * * * * * * * * * *
 *******************************************************************************/
int mempool_create_mmap_pool (struct mempool_params *pool_params_p,
      void*    mempool_p_p);

int32_t mempool_delete_mmap_pool(void* pool_p);

int32_t PSP_PKTPROC_FUNC mempool_mmap_get_mem_block(void* pool_p,
      uchar8_t **mem_block_p_p,
      uint8_t* heap_b_p);

int32_t mempool_mmap_release_mem_block(void*     pool_p,
      uchar8_t* mem_block_p,
      uchar8_t  heap_b);

uint8_t *mempool_alloc_chain_mmap_region (struct memory_pool *mempool_p,
      uint32_t max_blocks, uint32_t block_size,
      uint32_t *alloc_count_p, uint8_t **tailp_p);


/******************************************** **********************************
 * * * * * * * F U N C T I O N   D E F I N I T I O N S* * * * * * * * * * * * *
 ******************************************************************************/

/***************************************************************************
 * Function Name : mempool_cerate_mmap_pool
 * Description   : Used to create memory pool using 'mmap' memory.
 *                 Allocates 'mmap' memory, divides the memory into blocks of
 *                 block size and maintains the memory blocks in a mempool.
 * Input         :
 *                 block_size - block size
 *                 max_blocks    - Number of blocks to be allocated.
 * Output        : None.
 * Return value  : MEMPOOL_SUCCESS/MEMPOOL_FAILURE
 ***************************************************************************/


int mempool_create_mmap_pool (struct mempool_params *pool_params_p,
      void*    mempool)
{
   struct memory_pool   *mempool_p = (struct memory_pool *)mempool;
   uint32_t      alloc_count = 0;
   uchar8_t      *tail_p = NULL;
   int32_t status=MEMPOOL_SUCCESS;


   do
   {
      if ( pool_params_p == NULL )
      {
         OF_LOG_MSG (OF_LOG_UTIL, OF_LOG_ERROR, "invalid input");
         free(mempool_p);
         status=MEMPOOL_FAILURE;   
         break;
      }

      /* sizeof the block should be atleast 8 bytes.
       * First word (4 bytes) is used to maintain the linked list of free blocks.
       * Second word is used to store physical address of mapped memory
       */
      if(pool_params_p->block_size <= 8)
      {
         OF_LOG_MSG (OF_LOG_UTIL, OF_LOG_ERROR, "BLOCKSIZE is less than 8 bytes ");
         free(mempool_p);
         status= MEMPOOL_ERROR_BLOCKSIZE_TOO_SMALL;
         break;
      }

      if (peth_char_dev_inst_g == NULL) 
      {
         OF_LOG_MSG (OF_LOG_UTIL, OF_LOG_ERROR, "char dev instance not yet iniialized");
         free(mempool_p);
         status=MEMPOOL_FAILURE;
         break;
      }

      mempool_p->mempool_type= MEMPOOL_TYPE_MMAP;
      mempool_p->total_blocks_count = pool_params_p->max_blocks;
      mempool_p->block_size  = pool_params_p->block_size;

      mempool_p->mem_region.mem_size = pool_params_p->block_size * pool_params_p->max_blocks;

      OF_LOG_MSG (OF_LOG_UTIL, OF_LOG_DEBUG, "mmap region size = 0x%0x",
            mempool_p->mem_region.mem_size);

      if (create_mmap_region (&(mempool_p->mem_region),
               peth_char_dev_inst_g->fd) < 0) 
      {
         OF_LOG_MSG (OF_LOG_UTIL, OF_LOG_ERROR,
               "Unable to create mmap memory region.");
         free(mempool_p);
         status=MEMPOOL_FAILURE;
         break;
      }

      mempool_p->kern_to_user_offset = (unsigned long) (mempool_p->mem_region.start_user_va_p -
            mempool_p->mem_region.start_kern_va_p);

      OF_LOG_MSG (OF_LOG_UTIL, OF_LOG_DEBUG, "block size %d kern_to_user_offset %d",
            pool_params_p->block_size, mempool_p->kern_to_user_offset);

      mempool_p->head_p =  mempool_alloc_chain_mmap_region (mempool_p, pool_params_p->max_blocks,
            pool_params_p->block_size, &alloc_count,
            &tail_p);
      if (mempool_p->head_p == NULL)
      {
         OF_LOG_MSG (OF_LOG_UTIL, OF_LOG_ERROR,"Unable to Allocate Memory Chunk for mempool",mempool_p->mempool_name);
         mempool_delete_pool (mempool_p);
         status=MEMPOOL_ERROR_UNABLE_TO_ALLOCATE_MEMORY_CHUNK;
         break;
      }
      mempool_p->tail_p = tail_p;
      /* Update the counters with real alloc_count */
      mempool_p->free_blocks_count += alloc_count;
      mempool_p->static_allocated_blocks_count += alloc_count;

      CNTLR_LOCK_INIT(mempool_p->mempool_lock);
      mempool_p->lock_b = TRUE;
      //mempool_p->lock_b = TRUE;
      if(mempool_set_threshold_values(mempool_p, 0, pool_params_p->max_blocks, 0) != MEMPOOL_SUCCESS)
         //if(mempool_set_threshold_values(mempool_p,threshold_min,static_blocks,replenish_count) != MEMPOOL_SUCCESS)
      {
         mempool_delete_pool(mempool_p);
         status=MEMPOOL_ERROR_SETTING_THRESHOLD_VALUES;
         break;
      }
      if(set_mempool_attributes(mempool_p,
               pool_params_p->b_list_per_thread_required,
               pool_params_p->noof_blocks_to_move_to_thread_list,
               pool_params_p->b_memset_not_required) != MEMPOOL_SUCCESS)
      {
         mempool_delete_pool(mempool_p);
         status=MEMPOOL_FAILURE;
         break;
      }

      status= MEMPOOL_SUCCESS;
   }while(0);

   return status;
}


int32_t mempool_delete_mmap_pool(void* pool_p)
{
   struct  memory_pool*  mempool_p = NULL;

   OF_LOG_MSG (OF_LOG_UTIL, OF_LOG_DEBUG, "entered");

   if(pool_p == NULL)
      return MEMPOOL_ERROR_NULL_MEMPOOL_PASSED;

   mempool_p = (struct memory_pool *)pool_p;

#if 0
   CNTLR_LOCK_TAKE(mempool_lock_g);
   CNTLR_SLLQ_DELETE_NODE (&mempool_list_g, (struct cntlr_sll_q_node *) mempool_p);
   CNTLR_LOCK_RELEASE(mempool_lock_g);
#endif

   if (peth_char_dev_inst_g == NULL) {
      OF_LOG_MSG (OF_LOG_UTIL, OF_LOG_ERROR,
            "char dev instance not yet iniialized.");
      return MEMPOOL_FAILURE;
   }

   delete_mmap_region (&(mempool_p->mem_region), peth_char_dev_inst_g->fd);
   free(mempool_p);
   return (MEMPOOL_SUCCESS);
}


int32_t PSP_PKTPROC_FUNC mempool_mmap_get_mem_block(void* pool_p,
      uchar8_t **mem_block_p_p,
      uint8_t* heap_b_p)

{
   struct memory_pool* mempool_p = (struct memory_pool *)pool_p;
   uchar8_t  *block_p = NULL, *temp_block_p = NULL;
   uchar8_t  thread_num = 0;
   uint32_t  ii,count;

   *mem_block_p_p = NULL;

   /* Check if mempool is valid */
   if(CNTLR_UNLIKELY(!mempool_p))
      return MEMPOOL_ERROR_NULL_MEMPOOL_PASSED;

   thread_num = cntlr_thread_index;

   /* Check per thread mempool*/
   if(CNTLR_LIKELY(thread_num && mempool_p->per_thread_list_p)) 
   {
      struct per_thread_mem_node* th_list_p;

      thread_num--;
      th_list_p = &mempool_p->per_thread_list_p[thread_num];

      if(CNTLR_LIKELY(th_list_p->free_count))
      {
         /* Free blocks are available in thread list. 
          * Allocate a node to the application.
          */
         *mem_block_p_p = th_list_p->per_thread_head_p;
         th_list_p->per_thread_head_p = 
            ((struct mem_link_node *)(*mem_block_p_p))->next_p;
         th_list_p->free_count--;
         th_list_p->alloc_count++;
         ((struct mem_link_node *)(*mem_block_p_p))->next_p = NULL;
         MEMP_INSERT_DEBUG_INFO_IN_STATIC_BLOCK((*mem_block_p_p));
         *heap_b_p = FALSE;
         return MEMPOOL_SUCCESS;
      }
      else
      {
         /* No free blocks in thread list. Move static blocks to the thread list.
          * And allocate a node from the thread list to the application.
          */
         CNTLR_LOCK_TAKE(mempool_p->mempool_lock);
         //mempool_dbg("\r\n %s: Thread Num = %u, Per Thread free count = %u   ",__FUNCTION__,ucThreadNum+1,pMemPool->pPerThreadList[ucThreadNum].uiFreeCount);
         if(mempool_p->per_thread_list_p[thread_num].free_count)
         {
            *mem_block_p_p = mempool_p->per_thread_list_p[thread_num].per_thread_head_p;
            mempool_p->per_thread_list_p[thread_num].per_thread_head_p = 
               ((struct mem_link_node *)(*mem_block_p_p))->next_p;
            mempool_p->per_thread_list_p[thread_num].free_count--;
            mempool_p->per_thread_list_p[thread_num].alloc_count++;
            CNTLR_LOCK_RELEASE(mempool_p->mempool_lock);
            ((struct mem_link_node *)(*mem_block_p_p))->next_p = NULL;
            MEMP_INSERT_DEBUG_INFO_IN_STATIC_BLOCK((*mem_block_p_p));
            *heap_b_p = FALSE;
            return MEMPOOL_SUCCESS;
         }


         temp_block_p = mempool_p->head_p;
#if 0
         if(!pucTempBlock)
            printf("\r\n %s:No more static allocations possible...",__FUNCTION__);
#endif
         if(temp_block_p && mempool_p->block_count_for_moving_to_threadlist)
         {
            /*mempool_dbg("Thread Num = %u, BMV cnt = %u, Global pool Free Count = %u\n",
              ucThreadNum+1,pMemPool->uiBlockMoveCountToThreadList,pMemPool->MemPoolStats.ulFreeCnt);*/
            if(mempool_p->block_count_for_moving_to_threadlist < 
                  mempool_p->free_blocks_count)
            {
               /*mempool_dbg("\r\n %s: Thread Num = %u,  ",__FUNCTION__,thread_num); */
               count = mempool_p->block_count_for_moving_to_threadlist;
               block_p = temp_block_p;
               ii = count - 1;
               while(ii > 0)
               {
                  block_p = ((struct mem_link_node *)block_p)->next_p;
                  ii--;
               }
               mempool_p->head_p = ((struct mem_link_node *)block_p)->next_p;
               ((struct mem_link_node *)block_p)->next_p = NULL;
            }
            else
            {
               count = mempool_p->free_blocks_count;
               mempool_p->head_p = NULL;
               mempool_p->tail_p = NULL;
            }

            mempool_p->allocated_blocks_count += count;
            mempool_p->free_blocks_count -= count;
            // mempool_dbg("\r\n %s: Thread Num = %u, Global Pool Free Count = %u   ",__FUNCTION__,ucThreadNum+1,pMemPool->MemPoolStats.ulFreeCnt); 

            mempool_p->per_thread_list_p[thread_num].per_thread_head_p = 
               ((struct mem_link_node *)temp_block_p)->next_p;

            mempool_p->per_thread_list_p[thread_num].free_count = (count - 1);
            //mempool_dbg("\r\n %s: Thread Num = %u, Per Thread free count = %u   ",__FUNCTION__,ucThreadNum+1,pMemPool->pPerThreadList[ucThreadNum].uiFreeCount);
            CNTLR_LOCK_RELEASE(mempool_p->mempool_lock);
            ((struct mem_link_node *)temp_block_p)->next_p = NULL;
            MEMP_INSERT_DEBUG_INFO_IN_STATIC_BLOCK(temp_block_p);
            *mem_block_p_p  = temp_block_p;
            *heap_b_p = FALSE;
            return MEMPOOL_SUCCESS;
         }
      }
   }
   else
   {
      //mempool_dbg("\r\n %s: Get Block from Global Pool .....",__FUNCTION__);
      /*Check free list */
      CNTLR_LOCK_TAKE(mempool_p->mempool_lock);


      temp_block_p = mempool_p->head_p;
      if(CNTLR_LIKELY(temp_block_p))
      {
         mempool_p->head_p = ((struct mem_link_node *)temp_block_p)->next_p;
         if(CNTLR_UNLIKELY(!(mempool_p->head_p)))
         {
            mempool_p->tail_p = NULL;
         }
         mempool_p->allocated_blocks_count++;
         mempool_p->free_blocks_count--;
         ((struct mem_link_node *)temp_block_p)->next_p = NULL;
         CNTLR_LOCK_RELEASE(mempool_p->mempool_lock);
         MEMP_INSERT_DEBUG_INFO_IN_STATIC_BLOCK(temp_block_p);
         *heap_b_p = FALSE;
         *mem_block_p_p = temp_block_p;
         return MEMPOOL_SUCCESS;
      }
      else
      {
         /*mempool_dbg("\r\n %s: No Blocks in Global Pool !!!!! \r\n",__FUNCTION__);*/
      }
   }

   mempool_p->allocation_fail_blocks_count++;
   CNTLR_LOCK_RELEASE(mempool_p->mempool_lock);
   /* mempool_dbg("%s: No static or heap blocks are available for Mempool %s ..\n",__FUNCTION__,mempool_p->mempool_name);*/
   return MEMPOOL_FAILURE;
}

int32_t mempool_mmap_release_mem_block(void*     pool_p,
      uchar8_t* mem_block_p,
      uchar8_t  heap_b)

{
   struct memory_pool* mempool_p = (struct memory_pool *)pool_p;
   uchar8_t thread_num = 0;


   if(CNTLR_LIKELY(heap_b != TRUE))
   {
      MEMP_CHECK_DEBUG_INFO_IN_STATIC_BLOCK(mem_block_p);

#ifndef CNTLR_DSMEMPOOL_STATIC_DEBUG
      if(CNTLR_UNLIKELY(!(mempool_p->flags & CNTLR_MEMPOOL_DONOT_MEMSET)))
         memset(mem_block_p, 0, mempool_p->block_size);
#endif

      thread_num = cntlr_thread_index;

      if(CNTLR_LIKELY(thread_num  &&   
               (mempool_p->per_thread_list_p) &&
               (mempool_p->per_thread_list_p[thread_num-1].free_count <
                mempool_p->block_count_for_moving_to_threadlist)))
      {
         struct per_thread_mem_node* th_list_p;

         thread_num--; 
         th_list_p = &mempool_p->per_thread_list_p[thread_num];
         if(mem_block_p == th_list_p->per_thread_head_p)
         {
            //of_ShowStackTrace();
         }
         ((struct mem_link_node *)mem_block_p)->next_p = th_list_p->per_thread_head_p;
         th_list_p->per_thread_head_p = mem_block_p;
         th_list_p->free_count++;
         th_list_p->release_count++;
         return MEMPOOL_SUCCESS;
      }
      else
      {
         //mempool_dbg("\r\n %s: Release Block to Global Pool .....",__FUNCTION__);
         CNTLR_LOCK_TAKE(mempool_p->mempool_lock);

         ((struct mem_link_node *)mem_block_p)->next_p = NULL; 
         if(CNTLR_LIKELY(mempool_p->tail_p))
         {
            ((struct mem_link_node *)(mempool_p->tail_p))->next_p = mem_block_p;
         }
         else
         {
            mempool_p->head_p = mem_block_p;
         }
         mempool_p->tail_p = mem_block_p;
         mempool_p->released_blocks_count++;
         mempool_p->free_blocks_count++;
         CNTLR_LOCK_RELEASE(mempool_p->mempool_lock);
         return MEMPOOL_SUCCESS;
      }
   }
   else
   {
      OF_LOG_MSG (OF_LOG_UTIL, OF_LOG_ERROR,
            "Should not happen!! mmap-based memory pool should not have heap blocks.");
      return MEMPOOL_FAILURE;

   }
   return MEMPOOL_FAILURE;
}


int create_mmap_region (struct mem_region_info *mem_region_p,
      int32_t char_drv_fd)
{
   int             fd, page_size = getpagesize();
   unsigned long      virt_pfn, offset;
   unsigned long long   page;
   uint32_t      hugepage_mask = CNTLR_DEFAULT_HUGEPAGE_SIZE - 1;

   if ((!mem_region_p) || (mem_region_p->mem_size == 0)) {
      OF_LOG_MSG (OF_LOG_UTIL, OF_LOG_ERROR,
            "Invalid mem_region_p");
      return MEMPOOL_FAILURE;
   }

   /* Align the memory size to hugepage size boundary. */
   mem_region_p->mem_size = (mem_region_p->mem_size + hugepage_mask) &
      ~hugepage_mask;

   OF_LOG_MSG (OF_LOG_UTIL, OF_LOG_DEBUG, "mmap region size = 0x%0x",
         mem_region_p->mem_size);

   mem_region_p->start_user_va_p = mmap (NULL,
         mem_region_p->mem_size,
         PROT_READ | PROT_WRITE,
         MAP_SHARED | MAP_LOCKED | MAP_POPULATE |
         MAP_HUGETLB | MAP_ANONYMOUS,
         -1, 0);
   if (mem_region_p->start_user_va_p == MAP_FAILED) {
      OF_LOG_MSG (OF_LOG_UTIL, OF_LOG_ERROR, "memory map failed");
      return MEMPOOL_FAILURE;
   }

   fd = open("/proc/self/pagemap", O_RDONLY);
   if (fd < 0) {
      OF_LOG_MSG (OF_LOG_UTIL, OF_LOG_ERROR,
            "Unable to open /proc/self/pagemap file");
      munmap (mem_region_p->start_user_va_p, mem_region_p->mem_size);
      return MEMPOOL_FAILURE;
   }

   virt_pfn = (unsigned long)(mem_region_p->start_user_va_p) / page_size;
   offset = sizeof(unsigned long long) * virt_pfn;

   if (lseek (fd, offset, SEEK_SET) == -1) {
      OF_LOG_MSG (OF_LOG_UTIL, OF_LOG_ERROR,
            "seek error in /proc/self/pagemap file");
      munmap (mem_region_p->start_user_va_p, mem_region_p->mem_size);
      close(fd);
      return MEMPOOL_FAILURE;
   }

   if (read (fd, &page, sizeof(unsigned long long)) < 0) {
      OF_LOG_MSG (OF_LOG_UTIL, OF_LOG_ERROR,
            "cannot read /proc/self/pagemap");
      munmap (mem_region_p->start_user_va_p, mem_region_p->mem_size);
      close(fd);
      return MEMPOOL_FAILURE;
   }
   close(fd);

   OF_LOG_MSG (OF_LOG_UTIL, OF_LOG_DEBUG,
         "Mem region page map = 0x%0llx", page);
   /*
    * 0-54 bits of each virtual page info contains the
    * PFN (Page Frame Number).  Refer to pagemap.txt
    * in Linux Documentation
    */
   mem_region_p->start_phys_addr = ((page & 0x7fffffffffffffULL) * page_size);

   OF_LOG_MSG (OF_LOG_UTIL, OF_LOG_DEBUG,
         "Mem region phys_addr = 0x%0llx\n", mem_region_p->start_phys_addr);

   /* Send ioctl command to map this memory region in kernel. */
   if (ioctl (char_drv_fd, PETH_IOCTL_MAP_KERN_MEMORY,
            mem_region_p) < 0) {
      OF_LOG_MSG (OF_LOG_UTIL, OF_LOG_ERROR,
            "ioctl command to map the memory region in kernel failed");
      munmap (mem_region_p->start_user_va_p, mem_region_p->mem_size);
      return MEMPOOL_FAILURE;
   }

   OF_LOG_MSG (OF_LOG_UTIL, OF_LOG_DEBUG,
         "Mem region start_kern_va_p = 0x%0lx",
         mem_region_p->start_kern_va_p);

   return MEMPOOL_SUCCESS;
}

int delete_mmap_region (struct mem_region_info *mem_region_p,
      int32_t char_drv_fd)
{
   if ((!mem_region_p) || (mem_region_p->mem_size == 0)) {
      OF_LOG_MSG (OF_LOG_UTIL, OF_LOG_ERROR,
            "Invalid mem_region_p\n");
      return MEMPOOL_FAILURE;
   }

   if (mem_region_p->start_user_va_p == MAP_FAILED) {
      OF_LOG_MSG (OF_LOG_UTIL, OF_LOG_ERROR,
            "memory not mapped.\n");
      return MEMPOOL_FAILURE;
   }

   /* Send ioctl command to unmap this memory region in kernel. */
   if (ioctl (char_drv_fd, PETH_IOCTL_UNMAP_KERN_MEMORY,
            mem_region_p) < 0) {
      OF_LOG_MSG (OF_LOG_UTIL, OF_LOG_ERROR,
            "ioctl command to unmap the memory region in kernel failed.");
      return MEMPOOL_FAILURE;
   }

   munmap (mem_region_p->start_user_va_p, mem_region_p->mem_size);
   return MEMPOOL_SUCCESS;
}

uint8_t *mempool_alloc_chain_mmap_region (struct memory_pool *mempool_p,
      uint32_t max_blocks, uint32_t block_size,
      uint32_t *alloc_count_p, uint8_t **tailp_p)
{
   uint8_t      *pucHead = NULL, *pucBlock = NULL, *pucPrev = NULL;
   uint32_t   alloc_count = 0;
   int32_t      ii = 0;
   struct mem_region_info   *mem_region_p = &(mempool_p->mem_region);

   for (ii = 0; ii < max_blocks; ++ii) {

      pucBlock = (uint8_t *)(mem_region_p->start_user_va_p + (ii * block_size));
#ifdef PSP_DSMEMPOOL_STATIC_DEBUG
      pucBlock += MEMP_STATIC_PRE_SIGNATURE_BYTES;
      MEMP_SET_PRE_SIGNATURE(pucBlock, MEMP_INACTIVE_PRE_SIGNATURE);
      MEMP_SET_POST_SIGNATURE(pucBlock, MEMP_INACTIVE_POST_SIGNATURE,
            mempool_p);
      /* memset(pucBlock+8,MEMP_DATA_EXP,(pMemPool->MemPoolStats.block_size - 8)) */;
#endif
      alloc_count++;
      ((struct mempool_mmap_link_node *)pucBlock)->phys_addr =
         mem_region_p->start_phys_addr + (ii * block_size);
#ifdef PSP_DSMEMPOOL_STATIC_DEBUG
      ((struct mempool_mmap_link_node *)pucBlock)->phys_addr +=
         MEMP_STATIC_PRE_SIGNATURE_BYTES;
#endif
      if (pucPrev == NULL) {
         pucHead = pucPrev = pucBlock;
      } else {
         ((struct mempool_mmap_link_node *)pucPrev)->link_node.next_p = pucBlock;
         pucPrev = pucBlock;
      }
   }

   ((struct mempool_mmap_link_node *)pucBlock)->link_node.next_p = NULL;
   *alloc_count_p = alloc_count;
   *tailp_p = pucPrev;
   return (pucHead);
}
