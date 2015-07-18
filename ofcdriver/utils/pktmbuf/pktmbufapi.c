/**************************************************************************
 * Copyright 2011-2012, Freescale Semiconductor, Inc. All rights reserved.
 ***************************************************************************/
/*
 * File:	pbufapi.c
 *
 * Description: PSP buffer module APIs for PSP.
 *
 * Authors:	Venu Maya <venu_maya@freescale.com>
 * Modifier:
 *
 */
/*
 * History
 * 20 Sep 2011 - Venu Maya - Initial Implementation.
 *
 */
/******************************************************************************/

/*************** Macros ***************/
/*#include <psp/moduleid.h>*/
#include "controller.h"
#include "cntrlappcmn.h"
#include "dll.h"
#include "pktmbuf.h"
#include "pktmbufldef.h"
#include "sll.h"
#include "memldef.h"
#include "oflog.h"

int32_t of_pktmbuf_init();
extern struct pkt_mbuf_pool *pPSPBufPoolList_g;
extern struct pkt_mbuf *pkt_mbuf_heap_alloc(uint32_t uiDataSize);
extern int32_t pkt_mbuf_heap_free( struct pkt_mbuf *pBuf);

static inline int32_t pspBufIsCloned(struct pkt_mbuf *pBuf);



#define NUM_PKT_MBUF_POOLS      (sizeof(pkt_mbuf_pool_config_g) / sizeof(struct pkt_mbuf_pool_conf))
struct pkt_mbuf_pool PSP_PKTPROC_DATA *pPSPBufPoolList_g = NULL;

struct pkt_mbuf_pool_conf pkt_mbuf_pool_config_g[] = {
                                                        {128, 500},
                                                        {256, 500},
                                                        {512, 500},
                                                        {1024, 1000},
                                                        {1536, 1900},
                                                       // {1600, 1900},
                                                        {2048, 1000},
                                                        {4096, 500},
                                                        {8192, 50},
                                                        {16384, 50},
                                                        {32768, 50}
                                                     };
extern struct pkt_mbuf_conf pkt_mbuf_conf_g;
extern cntlr_instance_t cntlr_instance;

struct pktbuf_callbacks pktmbuf_cbks=
{
		__pkt_mbuf_alloc,
		_pkt_mbuf_free
};

struct pkt_mbuf * PSP_PKTPROC_FUNC __pkt_mbuf_alloc(uint32_t uiDataSize, uint32_t uiFlags)
{
  struct pkt_mbuf *pBuf = NULL;

  if(!(uiFlags & PKT_MBUF_MEMFLAG_HEAP))
  {
    struct pkt_mbuf_pool *pBufPool = pPSPBufPoolList_g;
    struct pkt_mbuf_pool *pPrevBufPool = NULL;
    uint8_t	single_pkt_buf = 0;

    while (pBufPool != NULL) {

	if (uiDataSize <= pBufPool->data_size)
	{
		pBuf = pkt_mbuf_alloc_by_pool (pBufPool);
		if (PSP_LIKELY(pBuf != NULL)) {
			single_pkt_buf = 1;
			break;
		}
	}
	pPrevBufPool = pBufPool;
	pBufPool = pBufPool->next;
    }

    /* Use the last buffer pool, if all the buffer pools are not fitting. */
    if (pBufPool == NULL)
	pBufPool = pPrevBufPool;

    /* Single PSPBuf case */
    if (PSP_LIKELY(single_pkt_buf))
    {
#ifdef CONFIG_PKT_MBUF_DEBUG_PKTPATH
      pBuf->pkt_path[0] = '\0';
      pBuf->pkt_path_p = pBuf->pkt_path;
#endif
      pBuf->end.buf =  pBuf;
      pBuf->end.ptr =  pBuf->data.ptr + pBufPool->data_size;
      pBuf->mac_hdr = pBuf->network_hdr = pBuf->transport_hdr = pBuf->data;
      pkt_mbuf_debug("PSPBUF::ALLOC %p (thread %d)\n", pBuf, PSP_SELF_THREAD_INDEX());
      CNTLR_LOCK_INIT(pBuf->data_ref_lock);
      pBuf->data_ref_cnt = 1;
      OF_PKT_MBUF_INC_ALLOC_CNT(uiDataSize, pBuf);
      return pBuf;
    }
    else if((uiFlags & PKT_MBUF_SG_ACCEPTED)) /* SG case */
    {
      uint32_t uiNumBufs,uiRemLen;
      int32_t ii;
      struct pkt_mbuf *pHeadBuf=NULL,*pPspBuf=NULL,*pLastBuf=NULL;

      // Now get number of PSPBufs required from this Pool
      pkt_mbuf_debug("%s:%d uiDataSize = %u  Max Data Size = %u !!! \r\n",__FUNCTION__,__LINE__,uiDataSize,pBufPool->data_size);
      uiNumBufs = uiDataSize/(pBufPool->data_size);
      uiRemLen  = uiDataSize - (uiNumBufs * pBufPool->data_size);
      if(uiRemLen != 0)
         uiNumBufs++;
      pkt_mbuf_debug("%s:%d  Number of Buffers required = %u , Remaining Len = %u !!! \r\n",__FUNCTION__,__LINE__,uiNumBufs,uiRemLen);

      for(ii=0;ii<(uiNumBufs);ii++)
      {
         pkt_mbuf_debug("%s:%d Allocating Node No # %d  \r\n",__FUNCTION__,__LINE__,ii);
         pPspBuf = pkt_mbuf_alloc_by_pool(pBufPool);
         if(!pPspBuf)
         {
           printf("%s:%d Failure to get PSPBuf !!! \r\n",__FUNCTION__,__LINE__);
           if(pHeadBuf)
             pkt_mbuf_free(pHeadBuf);
      OF_LOG_MSG(OF_LOG_MOD, OF_LOG_ERROR, "pkt_mbuf_alloc() size %d failed", uiDataSize);
           return NULL;
         }
         pPspBuf->max_data_size =  pBufPool->data_size;
         pPspBuf->buf_pool = pBufPool;
         pPspBuf->start_of_data     =  pkt_mbuf_sod_ptr(pPspBuf);

         pPspBuf->data.buf =  pPspBuf->end.buf = pPspBuf;
         pPspBuf->data.ptr =  pPspBuf->start_of_data +  pkt_mbuf_prepend_size();
         pPspBuf->end.ptr =  pPspBuf->data.ptr + pBufPool->data_size;

         pkt_mbuf_debug("pSoD             = %p  \r\n",pPspBuf->start_of_data);
         pkt_mbuf_debug(" Data.ptr          = %p  \r\n",pPspBuf->data.ptr);
         pkt_mbuf_debug(" Count           = %u  \r\n",pPspBuf->count);


         if(!pHeadBuf)
         {
           pkt_mbuf_debug("%s:%d Head PSPBuf = %p \r\n",__FUNCTION__,__LINE__,pPspBuf);
           pHeadBuf = pPspBuf;
           pPspBuf->sg.prev = pPspBuf->sg.next = pPspBuf;
           pPspBuf->sg.rem_len = 0;
           pHeadBuf->sg.rem_nodes = 0;
           pPspBuf->sg.head   =   pHeadBuf;
           pLastBuf = pHeadBuf;
           pHeadBuf->freertn = NULL;
           pHeadBuf->mac_hdr = pHeadBuf->network_hdr = pHeadBuf->transport_hdr= pHeadBuf->data;
         }
         else
         {
           pLastBuf->sg.next  =   pPspBuf;
           pPspBuf->sg.prev   =   pLastBuf;
           pLastBuf = pPspBuf;
           pPspBuf->sg.next   =   pHeadBuf;
           pPspBuf->sg.head   =   pHeadBuf;

           /* In SG Node, we can use Prepend size for data */
           pPspBuf->data.ptr =  pPspBuf->start_of_data;
           //pPspBuf->count  +=  pkt_mbuf_prepend_size();

           pHeadBuf->sg.rem_len +=  pPspBuf->count;
           pHeadBuf->sg.rem_nodes++;
           pkt_mbuf_debug("%s:%d Number of Nodes = %u  \r\n",__FUNCTION__,__LINE__,pHeadBuf->sg.rem_nodes);
           pkt_mbuf_debug("%s:%d Node PSPBuf = %p \r\n",__FUNCTION__,__LINE__,pPspBuf);
         }
      }
      pkt_mbuf_debug("PSPBUF::ALLOC (SG) %p (thread %d)\n", pHeadBuf, PSP_SELF_THREAD_INDEX());
      CNTLR_LOCK_INIT(pHeadBuf->data_ref_lock);
      pHeadBuf->data_ref_cnt = 1;
      OF_PKT_MBUF_INC_ALLOC_CNT(uiDataSize, pHeadBuf);
      return pHeadBuf;
    }
    else
    {
      printf("%s:%d PSPBuf with data size = %u cannot be allocated without SG nodes \r\n",__FUNCTION__,__LINE__, uiDataSize);
      OF_LOG_MSG(OF_LOG_MOD, OF_LOG_ERROR, "pkt_mbuf_alloc() size %d failed", uiDataSize);
      return NULL;
    }
  }
  else
  {
    return pkt_mbuf_heap_alloc(uiDataSize);
  }
      OF_LOG_MSG(OF_LOG_MOD, OF_LOG_ERROR, "pkt_mbuf_alloc() size %d failed", uiDataSize);
  return NULL;
}


//int32_t PSP_PKTPROC_FUNC _pkt_mbuf_free (struct pkt_mbuf *pBuf)
int32_t _pkt_mbuf_free (struct pkt_mbuf *pBuf)
{
	struct pkt_mbuf_pool	*pPspBufPool;
	void			*pMemPool;
	uint64_t		uiPhyAddr;
	uchar8_t		*buf_start;

        OF_PKT_MBUF_INC_FREE_CNT(pBuf);
	pkt_mbuf_debug("%s:%d PKT_MBUF_MEMFLAG_HEAP = 0x%x uiFlags = 0x%x\r\n",
		__FUNCTION__, __LINE__, PKT_MBUF_MEMFLAG_HEAP, pBuf->flags);
	/* Get 'bHeap' flag status */
	if (!(pBuf->flags & PKT_MBUF_MEMFLAG_HEAP)) {
		pkt_mbuf_debug("mapped memory... Releasing to Pool\n");
		pPspBufPool = (struct pkt_mbuf_pool *)(pBuf->buf_pool);
		pMemPool = pPspBufPool->mem_pool;

		/* Adjust Hardware area */
		buf_start = ((uchar8_t *)pBuf) - pkt_mbuf_hw_area_size();
		uiPhyAddr = pBuf->buf_phy_addr;
		/* Restore data buffer physical address */
		((struct mempool_mmap_link_node *)buf_start)->phys_addr = uiPhyAddr;
		//    pkt_mmap_link->phys_addr = uiPhyAddr;
		return mempool_release_mem_block (pMemPool, buf_start,
				(uchar8_t)FALSE);
	} else {
		pkt_mbuf_debug ("%s:%d bHeap is TRUE, Freeing the memory...\r\n",
			__FUNCTION__, __LINE__);
		return pkt_mbuf_heap_free(pBuf);
	}
}

static inline int32_t pspBufIsCloned(struct pkt_mbuf *pBuf)
{
  bool8_t bCloned=FALSE;

  if(pBuf->cloned)
  {
    CNTLR_LOCK_TAKE(pBuf->data_ref_lock);
    if( pBuf->data_ref_cnt != 1 )
      bCloned =  TRUE;
    CNTLR_LOCK_RELEASE(pBuf->data_ref_lock);
  }
  return bCloned;
}

uint32_t pkt_mbuf_pool_get_mem_size (void)
{
	struct pkt_mbuf_pool *pkt_buf_pool_info;
	uint32_t	pool_mem_size = 0;

	for (pkt_buf_pool_info = pPSPBufPoolList_g; (pkt_buf_pool_info);
		pkt_buf_pool_info = pkt_buf_pool_info->next)
	{
		pool_mem_size += pkt_buf_pool_info->buf_size *
					pkt_buf_pool_info->num_bufs;
	}

	return pool_mem_size;
}

/* This should be called from an init function */
int32_t pspBufCreateBufPools(void)
{
  struct pkt_mbuf_pool *pBufPoolInfo;

  pBufPoolInfo = pPSPBufPoolList_g;
  while(1)
  {
    if(!pBufPoolInfo)
      break;
    /* Create the PSPBuf memory pools */
    pBufPoolInfo->mem_pool = pkt_mbuf_pool_create(pBufPoolInfo->buf_size,
                                               pBufPoolInfo->num_bufs);
    if(!pBufPoolInfo->mem_pool)
    {
      printf("%s:%d   pkt_mbuf_pool_create failed !!!   \r\n",__FUNCTION__,__LINE__);
      return PSP_FAILURE;
    }
    pkt_mbuf_debug("%s::created pool with size %d\n", __FUNCTION__, pBufPoolInfo->buf_size);
    pBufPoolInfo = pBufPoolInfo->next;
  }

  pkt_mbuf_conf_g.pool_created = TRUE;
  return PSP_SUCCESS;
}

void pkt_mbuf_delete_buf_pools (void)
{
	struct pkt_mbuf_pool *buf_pool_info = pPSPBufPoolList_g;

	while (buf_pool_info) {

		mempool_delete_pool (buf_pool_info->mem_pool);
		buf_pool_info = buf_pool_info->next;
	} 

	return;
}





#ifdef CNTRL_INFRA_LIB_SUPPORT
int32_t cntlr_shared_lib_init()
{
	int32_t ret_val;
        int32_t status=OF_SUCCESS;
	OF_LOG_MSG(OF_LOG_UTIL, OF_LOG_DEBUG,"Loading  pktmbuf Infra");
    printf("\r\n ****  NSM DANGER  ****pktmuf_init is called");
	do
	{
		/** Initialize NEM Infrastructure */
		ret_val=of_pktmbuf_init();
		if(ret_val != OF_SUCCESS)
		{
			OF_LOG_MSG(OF_LOG_UTIL, OF_LOG_DEBUG, "Pkt mbuf initialization failed");
			status=OF_FAILURE;
			break;
		}

	}while(0);

	return status;
}
#endif


int32_t of_pktmbuf_init()
{
	int32_t status=OF_SUCCESS;
	int32_t ret_val;

	OF_LOG_MSG( OF_LOG_UTIL, OF_LOG_DEBUG, "entered");

	do
	{
		ret_val= of_pkt_buf_init();
		if(ret_val != OF_SUCCESS)
		{
			OF_LOG_MSG(OF_LOG_UTIL, OF_LOG_ERROR, "of_pkt_buf_init failed");
			status=OF_FAILURE;
			break;
		}

		ret_val=cntlr_register_pktbuf_callbacks(&pktmbuf_cbks);
		if(ret_val != OF_SUCCESS)
		{
			OF_LOG_MSG(OF_LOG_UTIL, OF_LOG_ERROR, "pktbuf callback registration failed");
			status=OF_FAILURE;
			break;
		}
	}while(0);

	return status;

}


int32_t of_pkt_buf_init ()
{
  uint32_t buf_size = 0, count = NUM_PKT_MBUF_POOLS;
  uint32_t /* buf_region_size, */ buf_count, num_bufs;
  int32_t ii;
  struct pkt_mbuf_pool *buf_pool_info = NULL;

  pPSPBufPoolList_g = NULL;

  pkt_mbuf_debug ("%s:%d:: Number of Pools configured = %d\r\n",
      __FUNCTION__, __LINE__, count);

  for (ii = 0; ii < count; ++ii) {
    /* Calculate memory size required for pkt_mbuf */
    buf_size = pkt_mbuf_conf_g.hw_area_size + sizeof(struct pkt_mbuf) +
		pkt_mbuf_conf_g.pkt_prepend_size +
		pkt_mbuf_pool_config_g[ii].data_size +
		pkt_mbuf_conf_g.pkt_append_size +
		pkt_mbuf_conf_g.appl_scratch_area_size +
		pkt_mbuf_conf_g.third_party_app_buf_area_size;

    buf_count = pkt_mbuf_pool_config_g[ii].pool_size;

    while (buf_count > 0) {

#if 0
	buf_region_size = buf_size * buf_count;

	/* Kernel mapping is not working if the buf_region_size is more than
	 * hugepage size. Creating one buffer pool for every hugepage size memory.
	 */
	if (buf_region_size > CNTLR_DEFAULT_HUGEPAGE_SIZE) {
		num_bufs = CNTLR_DEFAULT_HUGEPAGE_SIZE / buf_size;
	} else
#endif
	{

		num_bufs = buf_count;
	}
	
	/* Create a new pkt_mbuf pool entry */
	buf_pool_info = (struct pkt_mbuf_pool*) of_mem_calloc (1,
					sizeof(struct pkt_mbuf_pool));
	if(!buf_pool_info) {
		printf("%s:%d  memory allocation failed !!!\r\n",
		  __FUNCTION__, __LINE__);
		return PSP_FAILURE;
	}
	buf_pool_info->data_size = pkt_mbuf_pool_config_g[ii].data_size;
	buf_pool_info->num_bufs = num_bufs;
	buf_pool_info->next = NULL;
	buf_pool_info->buf_size = buf_size;

	pkt_mbuf_debug ("%s:%d:: Buffer size = %u\r\n",
	__FUNCTION__, __LINE__, buf_pool_info->buf_size);
	pkt_mbuf_debug ("%s:%d:: Number of Buffers = %u\r\n",
	__FUNCTION__, __LINE__, buf_pool_info->num_bufs);

	/* Add the entry to the linked list 'pPSPBufPoolList_g'
	* This list is ordered on 'data size' of pkt_mbuf.
	*/
	if(pPSPBufPoolList_g) {
		struct pkt_mbuf_pool *pTempPoolInfo = pPSPBufPoolList_g;
		struct pkt_mbuf_pool *pPrevPoolInfo = NULL;

		pkt_mbuf_debug ("%s:%d:: Inserting PSPBufPool = %p, size = %u\r\n",
				__FUNCTION__, __LINE__, buf_pool_info,
				buf_pool_info->data_size);
		while(1) {
			pkt_mbuf_debug("%s:%d:: Node size = %u\r\n", __FUNCTION__,
					__LINE__, pTempPoolInfo->data_size);

			if(buf_pool_info->data_size < pTempPoolInfo->data_size) {
				pkt_mbuf_debug("%s:%d:: Inserting PSPBufPool = %p\r\n",
					__FUNCTION__, __LINE__, buf_pool_info);

				buf_pool_info->next = pTempPoolInfo;
				if(pPrevPoolInfo)
					pPrevPoolInfo->next = buf_pool_info;
				else
					pPSPBufPoolList_g = buf_pool_info;
				break;
			} else {
				pkt_mbuf_debug("%s:%d:: Inserting PSPBufPool = %p\r\n",
					__FUNCTION__, __LINE__, buf_pool_info);

				pPrevPoolInfo = pTempPoolInfo;
				pTempPoolInfo = pTempPoolInfo->next;
				if (!pTempPoolInfo) {
					pPrevPoolInfo->next = buf_pool_info;
					break;
				}
			}
		} /* end of while loop */
	} else {
		pkt_mbuf_debug("%s:%d:: Adding at head PSPBufPool = %p\r\n",
				__FUNCTION__, __LINE__, buf_pool_info);
		pPSPBufPoolList_g = buf_pool_info;
	}

    	buf_count -= num_bufs;
    } // while loop to allocate all buffers.
  } /* end of for loop */

  /* Create pools for the pkt_mbufs. */
  pspBufCreateBufPools(); //- moved to shared library

  return 0;
}
#if 0
inline void pkt_mbuf_update_pkt_hdrs(struct pkt_mbuf *pkt_mbuf_p)
{
  uint8_t iphdr_len=0;

  pkt_mbuf_p->network_hdr = pkt_mbuf_p->data;
  pkt_mbuf_p->network_hdr.ptr = pkt_mbuf_p->data.ptr + 14;
  /* updating transport hdr pointers */
  pkt_mbuf_p->transport_hdr = pkt_mbuf_p->data;
  iphdr_len = ((pkt_mbuf_p->network_hdr.ptr[0] & 0xf)*4);
  pkt_mbuf_p->transport_hdr.ptr = pkt_mbuf_p->network_hdr.ptr + iphdr_len;

  //pkt_mbuf_p->l3_proto = pkt_mbuf_p->data.ptr[9];
  pkt_mbuf_p->l3_proto = pkt_mbuf_ipv4_proto(pkt_mbuf_p);
  pkt_mbuf_p->nw_tuple.ip4.dst = pkt_mbuf_ipv4_dst_ip(pkt_mbuf_p);
  pkt_mbuf_p->nw_tuple.ip4.src = pkt_mbuf_ipv4_src_ip(pkt_mbuf_p);

  if((pkt_mbuf_p->l3_proto == OF_CNTLR_IP_PROTO_UDP)||
		  (pkt_mbuf_p->l3_proto == OF_CNTLR_IP_PROTO_TCP))
  {
    OF_LOG_MSG(OF_LOG_MOD,OF_LOG_DEBUG, "transport protocol");  
    pkt_mbuf_p->xport_tuple.src_port = pkt_mbuf_get_16bits(pkt_mbuf_xport_hdr_ptr(pkt_mbuf_p));
    pkt_mbuf_p->xport_tuple.dst_port = pkt_mbuf_get_16bits(pkt_mbuf_xport_hdr_ptr(pkt_mbuf_p) + 2);

  }
  else if(pkt_mbuf_p->l3_proto == OF_CNTLR_IP_PROTO_ICMP)
  {
    OF_LOG_MSG(OF_LOG_MOD,OF_LOG_DEBUG, "icmp protocol");
    pkt_mbuf_p->xport_tuple.icmp_type = *(pkt_mbuf_xport_hdr_ptr(pkt_mbuf_p));
    pkt_mbuf_p->xport_tuple.icmp_code = *(pkt_mbuf_xport_hdr_ptr(pkt_mbuf_p) + 1);  
    OF_LOG_MSG(OF_LOG_MOD,OF_LOG_DEBUG, "icmp type:%d and code:%d",pkt_mbuf_p->xport_tuple.icmp_type, pkt_mbuf_p->xport_tuple.icmp_code);
  }
  else
  {
    OF_LOG_MSG(OF_LOG_MOD, OF_LOG_DEBUG, "Invalid l3 proto ");
  }
  return;
}
#endif

#ifdef CNTRL_INFRA_LIB_SUPPORT
int32_t cntlr_shared_lib_deinit()
{
	int32_t status=OF_SUCCESS;
	int32_t ret_val;
	OF_LOG_MSG(OF_LOG_UTIL, OF_LOG_DEBUG,"UnLoading  pktmbuf Infra");

	do
	{
		pkt_mbuf_delete_buf_pools();
		ret_val=cntlr_deregister_pktbuf_callbacks(&pktmbuf_cbks);
		if(ret_val != OF_SUCCESS)
		{
			OF_LOG_MSG(OF_LOG_UTIL, OF_LOG_ERROR, "pktmbuf deregistration failed");
			status=OF_FAILURE;
			break;
		}
	}while(0);

	return status;
}
#endif
