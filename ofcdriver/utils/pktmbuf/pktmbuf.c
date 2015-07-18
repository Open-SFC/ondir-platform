
/**  \addtogroup PBUFAPI  PSP Buffer manager APIs
 */

/**************************************************************************
 * Copyright 2011-2012, Freescale Semiconductor, Inc. All rights reserved.
 ***************************************************************************/
/*
 * File:	pktmbuf.c
 *
 * Description: Functions on pkt_mbuf structrues and pools.
 *
 * Authors:
 * Modifier:
 *
 */
/*
 * History
 * 20 Sep 2011 - Mallik - Initial Implementation.
 *
 */
/******************************************************************************/

#include "controller.h"
#include "dll.h"
#include "pktmbuf.h"
#include "pktmbufldef.h"
#include "sll.h"
#include "memldef.h"
#include "oflog.h"


uint32_t pkt_mbuf_dbg=0;

#if 0
// Move it to pspcore.h
extern void *pPSPCoreConfigDOM_g;
#endif

int32_t mempool_show(void *mempool );

struct pkt_mbuf *pkt_mbuf_copy(struct pkt_mbuf *mbuf, uint32_t flags);

#if 0
void *pPSPBufConfigDOM_g;
#endif

extern struct pkt_mbuf_conf  pkt_mbuf_conf_g;

extern struct pkt_mbuf_pool *pPSPBufPoolList_g;

#if 0
int32_t pkt_mbuf_conf_get_buf_pool_list(void *buf_pool_node, IXML_NodeList **info_list,uint32_t *list_count);
int32_t pkt_mbuf_conf_get_buf_pool_info_at_index(IXML_NodeList  *info_list, int32_t index,uint32_t *data_size, uint32_t *pool_size);
#endif
int32_t pkt_mbuf_process_init_hook(void *arg_p,int32_t init);
int32_t pkt_mbuf_get_data_ref_at_offset(struct pkt_mbuf *mbuf, uint32_t offset,struct pkt_mbuf_data_ref *pDataPtr,uint32_t *bytes_valid);
void pspShowPSPBuf( struct pkt_mbuf *mbuf);
int32_t PSPBufPushData(struct pkt_mbuf *mbuf, uint32_t ulNoOfBytes);

int32_t  pkt_mbuf_show_stats();

void pkt_mbuf_list_pools();
int32_t pbuftest(int32_t cmd);

extern cntlr_instance_t cntlr_instance;

void *pkt_mbuf_pool_create (uint32_t buf_size, uint32_t num_bufs)
{
	char		pool_name[MEMPOOL_MAX_POOL_NAME_LEN+1];
	void		*mempool = NULL;
	uint8_t		do_not_memset = FALSE;
	int		ret;
        struct mempool_params mempool_info={};

	/*  Derive a name to buffer pool using ‘buffer size’ */
	snprintf (pool_name, MEMPOOL_MAX_POOL_NAME_LEN+1, "pkt_mbuf_pool_%u",
			buf_size);

	/* Create the buffer pool from kernel memory using ‘mempool’ API */
	mempool_info.mempool_type = MEMPOOL_TYPE_MMAP;  
	mempool_info.block_size = buf_size;
	mempool_info.max_blocks = num_bufs;
	mempool_info.b_memset_not_required =  do_not_memset;
	mempool_info.b_list_per_thread_required =  TRUE;
	mempool_info.noof_blocks_to_move_to_thread_list = num_bufs / (CNTLR_NO_OF_THREADS_IN_SYSTEM + 1);

	ret = mempool_create_pool (pool_name, &mempool_info, 
			&mempool);
	/* How to decide per thread count for PSPBufs ??? uiBlockMoveCountToThreadList
	 * For now equally distributing among main thread and worker threads...need to revisit .
	 */
	if ((ret != MEMPOOL_SUCCESS) || (mempool == NULL)) {
		printf ("%s:%d:: create_mempool() Failed\r\n", __FUNCTION__, __LINE__);
		return NULL;
	}

	return mempool;
}

/** \ingroup PBUFAPI

\n <b>Description:</b>\n

  <b>Input Parameters:</b>\n
  <i>  :</i>
  <i>  :</i>

  <b>Output Parameters</b>\n
  <i> None </i>

  <b>Returns</b>\n
  <i> NULL :</i> Returns NULL in case of failure.

 */

/** \ingroup PBUFAPI

\n <b>Description:</b>\n
  This function is used by PSP applications to reserve scratch area in PSPBuf.
  This API should be called only in Process Level 1 Init hooks.

\n <b>Input Parameters:</b>\n
  <i> scratch_area_size : </i> Scratch area size to be reserved.

  <b>Output Parameters</b>\n
  <i> None </i>

  <b>Returns</b>\n
  <i> None:</i>

 */
int pkt_mbuf_reserve_app_scratch_area (uint32_t scratch_area_size)
{
	// Calling this function after buffer pools are created is useless.
	if( pkt_mbuf_conf_g.pool_created == TRUE) {
		// Trying to set 'Scratch Area' after pools created !!!
		printf("%s:%d Trying to set 'Scratch Area' after pools created !!!!\r\n",
			__FUNCTION__, __LINE__);
		return -1;
	}

	if(pkt_mbuf_conf_g.appl_scratch_area_size < scratch_area_size)
		pkt_mbuf_conf_g.appl_scratch_area_size = scratch_area_size;

	return 0;
}

/** \ingroup PBUFAPI

\n <b>Description:</b>\n
  This function is used by 3rd party PSP applications to reserve some area in PSPBuf,

\n <b>Input Parameters:</b>\n
  <i> third_party_app_area_size: </i> Third party application area size to be reserved.

  <b>Output Parameters</b>\n
  <i> None </i>

  <b>Returns</b>\n
  <i> None:</i>

 */
void pkt_mbuf_reserve_third_party_area (uint32_t third_party_app_area_size)
{
	if (pkt_mbuf_conf_g.third_party_app_buf_area_size <
					third_party_app_area_size) {
		pkt_mbuf_conf_g.third_party_app_buf_area_size =
					third_party_app_area_size;
	}
	return;
}

struct pkt_mbuf* PSP_PKTPROC_FUNC pkt_mbuf_alloc_by_pool (struct pkt_mbuf_pool *buf_pool)
{
	struct pkt_mbuf *buf;
	uint8_t  heap = FALSE;
	int32_t  ret_value = PSP_FAILURE;
	uint64_t phy_addr;
//	int32_t  nc_lines;

	/* Get a free block from PSPBuf mempool */
	ret_value = mempool_get_mem_block (buf_pool->mem_pool, (uchar8_t **)&buf,
				&heap);
	if (ret_value != MEMPOOL_SUCCESS) {
		pkt_mbuf_debug("%s:%d  mempool_get_block Failed !!!  \r\n",__FUNCTION__,__LINE__);
		return NULL;
	}

	/* Store physical address in PSPBuf TBD: Check if this is data area physical address ??*/
	phy_addr = ((struct mempool_mmap_link_node *)buf)->phys_addr;
	pkt_mbuf_debug("%s:%d ulPhyAddr = 0x%x\r\n",__FUNCTION__,__LINE__,phy_addr);
	buf = (struct pkt_mbuf *) (((char*)buf) + pkt_mbuf_hw_area_size());

#if 0
#ifndef CONFIG_PSP_BOARD_X86
  nc_lines = sizeof(struct pkt_mbuf)/PSP_CACHE_LINE_SIZE;
#if 0
  {
    uint32_t ii;
    /* if(sizeof(struct pkt_mbuf)%PSP_CACHE_LINE_SIZE) nc_lines++; */
    for(ii = 0; ii < 64; ii++)
    {
      *((uchar8_t*)buf+ ii) = 0x80;
    }
  }
#endif
  PSP_DATA_CACHE_BLOCK_CLEARn(buf, nc_lines);
#if 0
  // for testing
  for(ii = 0; ii < 64; ii++)
  {
    if(*((uchar8_t*)buf + ii) != 0)
    {
      PSPPrint("%s::data not cleared at offset %d\n", __FUNCTION__, ii);
      PSPPrint("%s:: sizeof PSPBuf = %d, rem bytes = %d  \n", __FUNCTION__,sizeof(struct pkt_mbuf),(sizeof(struct pkt_mbuf)%PSP_CACHE_LINE_SIZE)) ;
      //break;
    }
  }
#endif
#endif
#endif

	buf->buf_phy_addr = phy_addr;
	pkt_mbuf_set_sod_phy_addr(buf);
	/* Append size and prepend size is not considered in uiMaxDataSize
	this field may be used to add received packet data to the pspbuf
	structure */
	buf->max_data_size =  buf_pool->data_size;
	buf->buf_pool = buf_pool;
	buf->start_of_data     =  pkt_mbuf_sod_ptr(buf);
	pkt_mbuf_debug("pSoD             = %p  \r\n",buf->start_of_data);
	buf->data.buf =  buf;
	buf->data.ptr =  buf->start_of_data +  pkt_mbuf_prepend_size();
	buf->mac_hdr = buf->network_hdr = buf->transport_hdr= buf->data;
	pkt_mbuf_debug(" data.ptr          = %p  \r\n",buf->data.ptr);

	/* align pointer to 4 byte boundary */

	buf->sg.next = buf->sg.prev = buf->sg.head = buf;

	/*  'data' and 'count' fields must be set by caller of this function
	*/

	/* Store ‘bHeap’ flag */
	if(heap == TRUE) {
		buf->flags |= PKT_MBUF_MEMFLAG_HEAP;
		pkt_mbuf_debug("%s:%d heap is TRUE  \r\n",__FUNCTION__,__LINE__);
	} else {
		buf->flags &= ~PKT_MBUF_MEMFLAG_HEAP;
	}
	return buf;
}


struct pkt_mbuf *pkt_mbuf_heap_alloc(uint32_t data_size)
{
  struct pkt_mbuf *buf=NULL;
  uint32_t buf_size;
  // For now assuming that for heap allocated buffers, following buffer areas are NOT required:
  // H/w area
  // Skip Area
  // If Appln scratch area and 3rd party appln area are NOT needed, it should be passed in Flags.
  buf_size = sizeof(struct pkt_mbuf);
  buf_size +=  data_size;
  buf_size +=  pkt_mbuf_appl_scratch_area_size() ;
  buf_size +=  pkt_mbuf_third_party_appl_area_size();
  buf_size +=  pkt_mbuf_prepend_size();
  buf_size +=  pkt_mbuf_append_size();

  pkt_mbuf_debug("%s:%d Allocating PSPBuf from Heap !!!! \r\n",__FUNCTION__,__LINE__);
  buf = (struct pkt_mbuf *) of_mem_calloc(1,buf_size);
  if(buf == NULL)
  {
    printf("%s:%d Failed to allocate memory !!!! \r\n",__FUNCTION__,__LINE__);
    return NULL;
  }
  buf->flags |= PKT_MBUF_MEMFLAG_HEAP;
  buf->data_ref_cnt = 1;
  CNTLR_LOCK_INIT(buf->data_ref_lock);
  buf->max_data_size =  data_size;
  buf->start_of_data     =  pkt_mbuf_sod_ptr(buf);

  buf->data.ptr =  buf->start_of_data +  pkt_mbuf_prepend_size();
  buf->data.buf =  buf->end.buf = buf;

  buf->count =  data_size;
  buf->end.ptr =  buf->data.ptr + buf->count;
  buf->mac_hdr = buf->network_hdr = buf->transport_hdr= buf->data;

  pkt_mbuf_debug("pSoD   =  %p  \r\n",buf->start_of_data);

  buf->sg.prev = buf->sg.next = buf;
  buf->sg.rem_len = 0;
  buf->sg.rem_nodes = 0;
  buf->sg.head   =   buf;
  buf->freertn = NULL;

  return buf;
}

int32_t pkt_mbuf_heap_free( struct pkt_mbuf *buf)
{
  free(buf);
  return PSP_SUCCESS;
}

int32_t __pkt_mbuf_free( struct pkt_mbuf *mbuf)
{
  struct pkt_mbuf *temp,*buf;

  pkt_mbuf_debug("PSPBUF::FREE %p (thread %d) - sg_nodes %d\n", mbuf, PSP_SELF_THREAD_INDEX(), mbuf->sg.rem_nodes);


  //If it is a clone, get parent buffer and free the clone.
  pkt_mbuf_debug("%s:%d  PKT_MBUF_CLONE = 0x%x   flags = 0x%x  \r\n",__FUNCTION__,__LINE__,PKT_MBUF_CLONE,mbuf->flags);
  if(PSP_UNLIKELY(mbuf->flags & PKT_MBUF_CLONE))
  {
     //Get the Parent
     temp = mbuf->parent;

     pkt_mbuf_debug(" Free called on a Clone %p .... Parent = %p \r\n",mbuf,temp);
    _pkt_mbuf_free(mbuf);

     //Now check the parent
     mbuf = temp;
  }

  pkt_mbuf_debug("%s:%d DataRefCnt = %d  bCloned = %d  \r\n",__FUNCTION__,__LINE__, mbuf->data_ref_cnt,mbuf->cloned);

  // If Buf is cloned, decrement data Ref count
  if(PSP_UNLIKELY(mbuf->cloned))
  {
    pkt_mbuf_debug("Buf is cloned....Ref cnt = %d \r\n",mbuf->data_ref_cnt);
    CNTLR_LOCK_TAKE(mbuf->data_ref_lock);
    mbuf->data_ref_cnt--;
    if(mbuf->data_ref_cnt)
    {
      pkt_mbuf_debug("Clones exist... RefCn = %d \r\n",mbuf->data_ref_cnt);
      if(mbuf->data_ref_cnt == 1)
        mbuf->cloned = FALSE;
      CNTLR_LOCK_RELEASE(mbuf->data_ref_lock);
      return PSP_SUCCESS;
    }
    CNTLR_LOCK_RELEASE(mbuf->data_ref_lock);
  }

  /* Free the mempools in SG nodes */
  if(PSP_UNLIKELY(pkt_mbuf_is_sg_valid(mbuf)))
  {
    /* Here ensure that passed mbuf is Head PSPBuf */
    if(!pkt_mbuf_sg_is_head(mbuf))
    {
      printf("%s:%d ERROR!!! Given PSPBuf is NOT a Head Buffer !!! \r\n",__FUNCTION__,__LINE__);
      return PSP_FAILURE;
    }
    buf = mbuf->sg.next;
    while(buf != mbuf)
    {
      pkt_mbuf_debug("%s:%d Freeing next SG node ....  \r\n",__FUNCTION__,__LINE__);
      temp = buf->sg.next;
      _pkt_mbuf_free(buf);
      buf = temp;
    }
  }

  if(PSP_UNLIKELY(mbuf->freertn))
  {
    pkt_mbuf_debug("%s:%d Invoking freertn ...  \r\n",__FUNCTION__,__LINE__);
    mbuf->freertn(mbuf->arg);
  }

  pkt_mbuf_debug("Freeing Head PSPBuf ...  \r\n");
  /* Now free the Head PSPBuf */
  return _pkt_mbuf_free(mbuf);
}

int32_t  __pkt_mbuf_free_chain(struct pkt_mbuf *mbuf)
{
  struct pkt_mbuf *temp;
  /*Check if mbuf is fragmented, if not free the buffer*/
  if(!pkt_mbuf_is_fragmented_pkt(mbuf))
  {
    pkt_mbuf_debug("%s:%d Freeing non fragment buf \r\n",__FUNCTION__,__LINE__);
    return __pkt_mbuf_free(mbuf);
  }
  /*Store the next frag in temp, free the current buffer.*/
  while(mbuf)
  {
    pkt_mbuf_debug("%s:%d Freeing next fragment node ....  \r\n",__FUNCTION__,__LINE__);
    temp=mbuf->frag.next;
    __pkt_mbuf_free(mbuf);
    mbuf = temp;
  }
  return PSP_SUCCESS;
}



#ifdef PKT_MBUF_XML_CONFIG
/* This should be called from level 2 process init hook */
int32_t pkt_mbuf_read_buf_pool_config(void)
{
  uchar8_t out_buf[PSP_MAX_DIR_PATH_LEN+1],pParamName[128];
  uint32_t data_size = 0,count=0,pool_size=0;
  int32_t ret_value,ii;
  struct pkt_mbuf_pool *buf_pool_info=NULL;
  void *root_node = NULL,*mbuf_info_node=NULL;
  IXML_NodeList *info_list=NULL;

  // Get XML Node for PSPBuf configuration
  buf_pool_info = NULL;
  PSPXMLGetRootNode(pPSPCoreConfigDOM_g, &root_node);
  if(!root_node)
  {
    printf("%s:  PSPXMLGetRootNode failed \r\n",__FUNCTION__);
    return PSP_FAILURE;
  }
  /* Venu -- Need to fix this API, it should return NULL in case of failure
   */
  mbuf_info_node=NULL;
  PSPXMLGetFirstChildByTagName((uchar8_t *)"PSPBufPoolInfo",
                                root_node,
                                &mbuf_info_node);
  if(!mbuf_info_node)
  {
    printf("%s: PSPXMLGetFirstChildByTagName  failed \r\n",__FUNCTION__);
    return PSP_FAILURE;
  }

  /* HW Area Size */
  t_strcpy((char *)pParamName,"HW_Area_Size");
  t_memset(out_buf,0,PSP_MAX_DIR_PATH_LEN+1);
  if(PSPXMLGetFirstChildValueByTagName((char *)pParamName,
                                    mbuf_info_node,
                                    out_buf,
                                    64))
  {
    printf("%s: PSPXMLGetFirstChildValueByTagName for HW_Area_Size failed \r\n",__FUNCTION__);
    return PSP_FAILURE;
  }

  pkt_mbuf_conf_g.hw_area_size = t_atoi((char *)out_buf);

/* Pkt Prepend Size */
  t_memset(out_buf,0,PSP_MAX_DIR_PATH_LEN+1);
  t_strcpy((char*)pParamName,"Pkt_Prepend_Size");
  if(PSPXMLGetFirstChildValueByTagName((char*)pParamName,
                                    mbuf_info_node,
                                    out_buf,
                                    64))
  {
    printf("%s: PSPXMLGetFirstChildValueByTagName for Pkt_Prepend_Size failed \r\n",__FUNCTION__);
    return PSP_FAILURE;
  }
  pkt_mbuf_conf_g.pkt_prepend_size  = t_atoi((char*)out_buf);
  pkt_mbuf_debug("\r\n%s: Pkt Prepend Size = %d \r\n",__FUNCTION__,pkt_mbuf_conf_g.pkt_prepend_size );

  /* Pkt Append Size */
  t_memset(out_buf,0,PSP_MAX_DIR_PATH_LEN+1);
  t_strcpy((char*)pParamName,"Pkt_Append_Size");
  if(PSPXMLGetFirstChildValueByTagName((char*)pParamName,
                                    mbuf_info_node,
                                    out_buf,
                                    64))
  {
    printf("%s: PSPXMLGetFirstChildValueByTagName for Pkt_Append_Size failed \r\n",__FUNCTION__);
    return PSP_FAILURE;
  }
  pkt_mbuf_conf_g.pkt_append_size  = t_atoi((char*)out_buf);
  pkt_mbuf_debug("\r\n%s: Pkt Append Size = %d \r\n",__FUNCTION__,pkt_mbuf_conf_g.pkt_append_size);


  ret_value = pkt_mbuf_conf_get_buf_pool_list(mbuf_info_node, &info_list,&count);
  if((ret_value != PSP_SUCCESS) || (info_list == NULL))
  {
    printf("%s:%d pkt_mbuf_conf_get_buf_pool_list  failed \r\n",__FUNCTION__,__LINE__);
    return PSP_FAILURE;
  }

  pkt_mbuf_debug("%s:%d  Number of Pools configured = %d  \r\n",__FUNCTION__,__LINE__,count);
  for(ii=0;ii<count;ii++)
  {
    /* Get next 'data size' and 'number of buffers' from configuration file */
     ret_value = pkt_mbuf_conf_get_buf_pool_info_at_index(info_list,ii,&data_size, &pool_size);
     if(ret_value == PSP_FAILURE)
     {
       printf("%s:%d  pkt_mbuf_conf_get_buf_pool_info_at_index failed for index = %d   \r\n",__FUNCTION__,__LINE__,ii);
       return PSP_FAILURE;
     }

   pkt_mbuf_debug("%s:%d  Creating BufPool...   \r\n",__FUNCTION__,__LINE__);
     /* Create a new PSPBuf pool entry */
     buf_pool_info = (struct pkt_mbuf_pool*)of_mem_calloc(1,sizeof(struct pkt_mbuf_pool));
     if(!buf_pool_info)
     {
       printf("%s:%d  memory allocation failed !!!   \r\n",__FUNCTION__,__LINE__);
       return PSP_FAILURE;
     }
     buf_pool_info->data_size  = data_size;
     buf_pool_info->num_bufs   = pool_size;
     buf_pool_info->next   = NULL;

   /* Calculate memory size required for PSPBuf.*/
     buf_pool_info->buf_size =   pkt_mbuf_conf_g.hw_area_size;
     buf_pool_info->buf_size +=  sizeof(struct pkt_mbuf);

     buf_pool_info->buf_size +=  pkt_mbuf_prepend_size();
     buf_pool_info->buf_size +=  data_size;
     buf_pool_info->buf_size +=  pkt_mbuf_append_size();
     buf_pool_info->buf_size +=  pkt_mbuf_conf_g.appl_scratch_area_size ;
     buf_pool_info->buf_size +=  pkt_mbuf_conf_g.third_party_app_buf_area_size ;

    pkt_mbuf_debug("%s:%d   Buffer size  =  %u  \r\n",__FUNCTION__,__LINE__,buf_pool_info->buf_size);
    pkt_mbuf_debug("%s:%d   Number of Buffers  =  %u  \r\n",__FUNCTION__,__LINE__,buf_pool_info->num_bufs);

   /* Add the entry to the linked list 'pPSPBufPoolList_g'
    * This list is ordered on 'data size' of PSPBuf.
    */
     if(pPSPBufPoolList_g)
     {
       struct pkt_mbuf_pool *pTempPoolInfo = pPSPBufPoolList_g;
       struct pkt_mbuf_pool *pPrevPoolInfo = NULL;
       pkt_mbuf_debug("%s:%d   Inserting  PSPBufPool  =  %p , size = %u \r\n",__FUNCTION__,__LINE__,buf_pool_info,buf_pool_info->data_size);
       while(1)
       {
          pkt_mbuf_debug("%s:%d  Node size =  %u  \r\n",__FUNCTION__,__LINE__,pTempPoolInfo->data_size);
          if(buf_pool_info->data_size <= pTempPoolInfo->data_size)
          {
            pkt_mbuf_debug("%s:%d   Inserting  PSPBufPool  =  %p  \r\n",__FUNCTION__,__LINE__,buf_pool_info);
            buf_pool_info->next = pTempPoolInfo;
            if(pPrevPoolInfo)
              pPrevPoolInfo->next = buf_pool_info;
            else
              pPSPBufPoolList_g = buf_pool_info;
            break;
          }
          else
          {
            pkt_mbuf_debug("%s:%d   Inserting  PSPBufPool  =  %p  \r\n",__FUNCTION__,__LINE__,buf_pool_info);
            pPrevPoolInfo = pTempPoolInfo;
            pTempPoolInfo = pTempPoolInfo->next;
            if(!pTempPoolInfo )
            {
              pPrevPoolInfo->next = buf_pool_info;
              break;
            }
          }
       }

     }
     else
     {
       pkt_mbuf_debug("%s:%d   Adding at head PSPBufPool  =  %p  \r\n",__FUNCTION__,__LINE__,buf_pool_info);
       pPSPBufPoolList_g = buf_pool_info;
     }
  }
 return PSP_SUCCESS;
}
#endif /* PKT_MBUF_XML_CONFIG */

void pkt_mbuf_list_pools()
{
  int32_t ii=1;
  struct pkt_mbuf_pool *buf_pool_info=NULL;

  buf_pool_info = pPSPBufPoolList_g;

  while(buf_pool_info)
  {
    printf("\r\n Pool No# %d  \r\n",ii++);
    //printf(" buf_pool_info   = %p   \r\n",buf_pool_info);
    printf("data Size         = %u    \r\n",buf_pool_info->data_size);
    printf("Buffer Size       = %u   \r\n",buf_pool_info->buf_size);
    printf("Number of Buffers = %u   \r\n",buf_pool_info->num_bufs);
    buf_pool_info = buf_pool_info->next;
  }
}

/* Returns offset to SoD pointed by given data reference */
int32_t pkt_mbuf_get_offset_to_sod(struct pkt_mbuf *mbuf, struct pkt_mbuf_data_ref data_ref)
{
  unsigned long addr,data_start,data_end ;\
  unsigned long prev_count,max_data;
  struct pkt_mbuf *buf;

  addr = (unsigned long)data_ref.ptr;
  data_start = (unsigned long)(mbuf->start_of_data);\
  data_end   = (unsigned long)mbuf->start_of_data + pkt_mbuf_max_data_size(mbuf) - 1;\


  if((pkt_mbuf_is_sg_valid(mbuf) != TRUE))\
  {
    if((addr >= data_start) &&
	   (addr <=  data_end ))
      return (addr - (unsigned long)(mbuf->data.ptr));
    else
    {
      printf("%s:%d Panic!!! data reference going beyond valid data!!!   \r\n",__FUNCTION__,__LINE__);\
      printf(" Addr = 0x%lx, data Begin = 0x%lx, data End = 0x%lx    \r\n",addr,data_start,data_end);\
      return PSP_FAILURE;
    }
  }

  /* Scatter Gather list */
  prev_count = 0;
  buf = mbuf;
  while(1)
  {
    max_data = pkt_mbuf_max_data_size(buf);\
    if((addr >= (unsigned long)buf->start_of_data) &&
	 (addr <=  (unsigned long)(buf->start_of_data + max_data - 1)))
    {
      return(prev_count + (addr - ((unsigned long)buf->data.ptr)));
    }
    else
    {
      prev_count += max_data;
    }
    if(buf->sg.next == buf->sg.head)
    {
      printf("%s:%d data_ref going beyond valid data !!!\r\n",__FUNCTION__,__LINE__);
      break;
    }
    buf = buf->sg.next;
  }
  return PSP_FAILURE;
}

/* Returns offset to data pointed by given data reference */
int32_t pkt_mbuf_get_offset_to_data_ptr(struct pkt_mbuf *mbuf, struct pkt_mbuf_data_ref data_ref)
{
  unsigned long addr;
  unsigned long prev_count;
  struct pkt_mbuf *buf;

  addr = (unsigned long)data_ref.ptr;

  if((pkt_mbuf_is_sg_valid(mbuf) != TRUE))\
  {
    if((addr >= (unsigned long)mbuf->data.ptr) &&
	   (addr <=  (unsigned long)(mbuf->data.ptr +  mbuf->count-1)))
      return (addr - (unsigned long)(mbuf->data.ptr));
    else
      return PSP_FAILURE;
  }

  /* Scatter Gather list */
  prev_count = 0;
  buf = mbuf;
  while(1)
  {
    if((addr >= (unsigned long)buf->data.ptr) &&
	 (addr <=  (unsigned long)(buf->data.ptr + buf->count -1)))
    {
      return(prev_count + (addr - ((unsigned long)buf->data.ptr)));
    }
    else
    {
      prev_count += buf->count;
    }
    if(buf->sg.next == buf->sg.head)
    {
      printf("%s:%d data_ref going beyond valid data !!!\r\n",__FUNCTION__,__LINE__);
      break;
    }
    buf = buf->sg.next;
  }
  return PSP_FAILURE;
}


/** \ingroup PBUFAPI

 <b>Description:</b>\n
 This API advances the given data reference by specified number of bytes and
 returns the actual number of bytes pulled.

  <b>Input Parameters:</b>\n
  <i> mbuf :</i>  pointer to PSPBuf \n
  <i> data_ref :</i>  pointer to data reference \n
  <i> num_bytes :</i> Number of bytes to pull

  <b>Output Parameters</b>\n
  <i> None </i>

  <b>Returns</b>\n
  <i> bytes_pulled :</i> Number of bytes pulled.
 */
int32_t pkt_mbuf_pull_data_ref(struct pkt_mbuf *mbuf, struct pkt_mbuf_data_ref *data_ref,uint32_t num_bytes)
{
  struct pkt_mbuf_data_ref orig_data;
  uint32_t bytes_pulled=0;

  orig_data  =  *data_ref;
__pkt_mbuf_pull_data_ref(mbuf,orig_data,num_bytes,bytes_pulled);
  *data_ref = orig_data;

  return bytes_pulled;
}

/** \ingroup PBUFAPI

 <b>Description:</b>\n
 This API advances the data reference in PSPBuf by specified number of bytes and returns updated data reference.
 It returns the actual number of bytes pulled.

  <b>Input Parameters:</b>\n
  <i> mbuf :</i>  pointer to PSPBuf \n
  <i> num_bytes :</i> Number of bytes to pull

  <b>Output Parameters</b>\n
  <i> None </i>

  <b>Returns</b>\n
  <i> bytes_pulled :</i> Number of bytes pulled.
 */
int32_t pkt_mbuf_pull_data(struct pkt_mbuf *mbuf, uint32_t num_bytes)
{
  uint32_t bytes_pulled=0;

  __pkt_mbuf_pull_data(mbuf,num_bytes,bytes_pulled);
  return bytes_pulled;
}


/* This API reads specified number of bytes from the given data pointer reference.
 * It returns the actual number of bytes read from packet.
 */
int32_t pkt_mbuf_read_data_ext(struct pkt_mbuf   *mbuf, struct pkt_mbuf_data_ref *data_ref,
                           uchar8_t   *user_data, uint32_t        num_bytes)
{\
   uint32_t bytes_read;
   struct pkt_mbuf_data_ref temp_data;

  temp_data  =  *data_ref;
  __pkt_mbuf_read_data_ext(mbuf, temp_data,num_bytes,user_data, bytes_read);
  *data_ref = temp_data;

  return bytes_read;
}

/** \ingroup PBUFAPI

 <b>Description:</b>\n
  This API reads specified number of bytes from the current data reference in PSPBuf.
  Current data reference in PSPBuf is advanced by number of bytes read.

  <b>Input Parameters:</b>\n
  <i> mbuf :</i>  pointer to PSPBuf \n
  <i> user_data :</i>  destination memory pointer where data is copied \n
  <i> num_bytes :</i> Number of bytes to read

  <b>Output Parameters</b>\n
  <i> None </i>

  <b>Returns</b>\n
  <i> bytes_read :</i> Number of bytes read.
 */
int32_t pkt_mbuf_read_data(struct pkt_mbuf   *mbuf, uchar8_t   *user_data, uint32_t  num_bytes)
{\
  uint32_t        bytes_read;
  __pkt_mbuf_read_data(mbuf,num_bytes,user_data,bytes_read);

  return bytes_read;
}

int32_t pkt_mbuf_get_data(struct pkt_mbuf *dest_buf,struct pkt_mbuf *src_buf, uint32_t bytes_to_copy)
{
  int32_t ret_value;
  struct pkt_mbuf_data_ref orig_src_data = src_buf->data;
  uint32_t orig_count = src_buf->count;


  ret_value = pkt_mbuf_copy_data(dest_buf,src_buf,bytes_to_copy);
  src_buf->data  = orig_src_data;
  src_buf->count = orig_count;
  return ret_value;
}
/** \ingroup PBUFAPI

 <b>Description:</b>\n
  This API copies specified number of bytes from the source PSPBuf to destination PSPBuf.
  Current data reference in Source and destination buffers is advanced by number of bytes read.

  <b>Input Parameters:</b>\n
  <i> dest_buf :</i>  pointer to destination PSPBuf \n
  <i> src_buf :</i>  pointer to source PSPBuf \n
  <i> bytes_to_copy :</i> Number of bytes to copy

  <b>Output Parameters</b>\n
  <i> None </i>

  <b>Returns</b>\n
  <i> PSP_SUCCESS or PSP_FAILURE</i> Success or Failure.
 */
int32_t  pkt_mbuf_copy_data(struct pkt_mbuf *dest_buf,struct pkt_mbuf *src_buf, uint32_t bytes_to_copy)
{
  uint32_t src_valid_bytes,valid_bytes;
  uint32_t num_bytes = bytes_to_copy;
  struct pkt_mbuf *temp_src = src_buf->data.buf;
  struct pkt_mbuf_data_ref src_data, orig_src_data;

  src_data =  orig_src_data = src_buf->data;

  while(num_bytes)
  {\
    __pkt_mbuf_get_data_count( src_data ,src_valid_bytes);
    pkt_mbuf_debug("%s:%d SrcValidBytes = %u \r\n",__FUNCTION__,__LINE__,src_valid_bytes);

    valid_bytes = (num_bytes < src_valid_bytes) ? num_bytes:src_valid_bytes;
    pkt_mbuf_debug("%s:%d NumBytes = %u,  ValidBytes = %u Writing data:\r\n",__FUNCTION__,__LINE__,num_bytes,valid_bytes);
    //__printchar(src_data.ptr,valid_bytes);
    __pkt_mbuf_write_data(dest_buf,src_data.ptr,valid_bytes);

    num_bytes -= valid_bytes;

    src_data.ptr += valid_bytes;
    if(num_bytes)
    {
      if( (temp_src->sg.next == temp_src->sg.head) )
      {
        printf("%s:%d Source PSPBuf has insufficient valid data !!! \r\n",__FUNCTION__,__LINE__);
        return PSP_FAILURE;
      }
      temp_src = temp_src->sg.next;
      src_data = temp_src->data;
    }
  }
  src_buf->data = src_data;
  src_buf->count -= bytes_to_copy;
  //src_buf->data = orig_src_data;
  return PSP_SUCCESS;
}

__inline__  void pkt_mbuf_copy_header(struct pkt_mbuf *src, struct pkt_mbuf *dest)
{
  uint32_t offset;

#define PCL(x) dest->x = src->x
  PCL(orig_rx_iface_id);
  PCL(rx_iface_id);
  PCL(tx_iface_id);
  PCL(ns_id);
  PCL(time_stamp);
  PCL(flags);
  PCL(l2_proto);
  PCL(l3_proto);
  PCL(pkt_type);
  PCL(rx_vlan_info) ;
  PCL(tx_vlan_info) ;
  PCL(qos_prio);

  if(src->l2_proto == PSP_L2PROTO_IP4)
    PCL(nw_tuple.ip4) ;
#ifdef PSP_IPV6_SUPPORT
  else if(src->l2_proto == PSP_L2PROTO_IP6)
    PCL(nw_tuple.ip6);
#endif

  PCL(xport_tuple.val);
  /* Since xportTuple is a union, the following need not be copied
  PCL(xport_tuple.spi);
  PCL(-xport_tuple.icmp_code);
  PCL(xport_tuple.id);
  PCL(xport_tuple.src_port);
  PCL(xport_tuple.dst_port);
  */

  // Set data count TBD: shud we copy 'count' ???
  //PCL(count) ;
  pkt_mbuf_debug("%s:%d  ORIG: count =  %u  \r\n",__FUNCTION__,__LINE__,src->count);
  pkt_mbuf_debug("%s:%d  Copy: count =  %u  \r\n",__FUNCTION__,__LINE__,dest->count);


  __pkt_mbuf_get_sod_offset(src,src->mac_hdr,offset); \
  pkt_mbuf_debug("%s:%d  SOURCE MAC Hdr Offset =  %u  \r\n",__FUNCTION__,__LINE__,offset);
  pkt_mbuf_get_data_data_ref_ad_sod_offset( dest, offset, dest->mac_hdr); \

  __pkt_mbuf_get_sod_offset(src,src->network_hdr,offset); \
  pkt_mbuf_debug("%s:%d  SOURCE Network Hdr Offset =  %u  \r\n",__FUNCTION__,__LINE__,offset);
  pkt_mbuf_get_data_data_ref_ad_sod_offset( dest, offset, dest->network_hdr); \

  __pkt_mbuf_get_sod_offset(src,src->transport_hdr,offset); \
  pkt_mbuf_debug("%s:%d  SOURCE Transport Hdr Offset =  %u  \r\n",__FUNCTION__,__LINE__,offset);
  pkt_mbuf_get_data_data_ref_ad_sod_offset( dest, offset, dest->transport_hdr); \

  __pkt_mbuf_get_sod_offset(dest,dest->mac_hdr,offset); \
  pkt_mbuf_debug("%s:%d  DEST MAC Hdr Offset =  %u  \r\n",__FUNCTION__,__LINE__,offset);
  __pkt_mbuf_get_sod_offset(dest,dest->network_hdr,offset); \
  pkt_mbuf_debug("%s:%d  DEST Network Hdr Offset =  %u  \r\n",__FUNCTION__,__LINE__,offset);
  __pkt_mbuf_get_sod_offset(dest,dest->transport_hdr,offset); \
  pkt_mbuf_debug("%s:%d  DEST Transport Hdr Offset =  %u  \r\n",__FUNCTION__,__LINE__,offset);

  return;
#undef PCL
}

/** \ingroup PBUFAPI

 <b>Description:</b>\n
  This API preapres a single flat PSPBuf from a chain of PSPBufs which are IP Fragments and/or Scatter Gather buffers.
  If requested, the flattened buffer will have specified amount of extra size.
  By default memory for Flat buffer is allocated from heap.

  <b>Input Parameters:</b>\n
  <i> mbuf :</i>  pointer to PSPBuf which is a chain of SG nodes or/and IP fragments. \n
  <i> extra_buf_size :</i> Number of extra bytes to allocate \n
  <i> flags :</i>  Flags

  <b>Output Parameters</b>\n
  <i> None </i>

  <b>Returns</b>\n
  <i> struct pkt_mbuf* </i> pointer to Flattened Buffer.\n
  <i> NULL      </i> In case of failure.

*/
struct pkt_mbuf *pkt_mbuf_flatten(struct pkt_mbuf *mbuf, uint32_t extra_buf_size, uint32_t flags)
{
  uint32_t data_size,offset;
  bool8_t heap=TRUE;
  struct pkt_mbuf *fat_buf = NULL,*buf;
  uchar8_t *src,*dest;


  // Get total data size required.
  data_size = pkt_mbuf_pkt_len(mbuf);
  // Add extra buf size
  data_size += extra_buf_size;

  // Check flags to see if kernel mapped or heap memory is required
  if(!(flags & PKT_MBUF_MEMFLAG_HEAP))
   heap = FALSE;

  if(heap == TRUE)
  {
    pkt_mbuf_debug("%s:%d Allocate memory from Heap   \r\n",__FUNCTION__,__LINE__);
  }
  else
  {
    pkt_mbuf_debug("%s:%d Allocate memory from Kernel mapped memory   \r\n",__FUNCTION__,__LINE__);
  }

  // Allocate the new PSPBuf with this size
  fat_buf = pkt_mbuf_alloc(data_size,flags);
  if(!fat_buf)
  {
    printf("%s:%d pkt_mbuf_alloc failed  \r\n",__FUNCTION__,__LINE__);
    return NULL;
  }

#ifdef CNTLR_IP_REASM_SUPPORT
  /*
   * If it is a fragmented packet, convert the chain of fragments into a
   * single SG buffer
   */
  if(pkt_mbuf_is_fragmented_pkt(mbuf))
  {
    PSP_IPReasmLinearize(mbuf);
  }
#endif

  // Check flags to find out which components of PSPBuf need to be copied
  if( flags & PKT_MBUF_HW_AREA_REQD) /* HW area required*/
  {
    dest = pkt_mbuf_hw_area_ptr(fat_buf);
    src  = pkt_mbuf_hw_area_ptr(mbuf);
    memcpy(dest,src,pkt_mbuf_hw_area_size());
  }

  /* SKIP area required*/
  if( flags & PKT_MBUF_SKIP_AREA_REQD)
  {
    dest = pkt_mbuf_skip_area_ptr(fat_buf);
    src  = pkt_mbuf_skip_area_ptr(mbuf);
    memcpy(dest,src,pkt_mbuf_prepend_size());
  }

  /* Application Scratch area required*/
  if( flags & PKT_MBUF_SCRATCH_AREA_REQD)
  {
    dest = pkt_mbuf_appl_scratch_area_ptr(fat_buf);
    src  = pkt_mbuf_appl_scratch_area_ptr(mbuf);
    memcpy(dest,src, pkt_mbuf_appl_scratch_area_size());
  }

  // Copy Headers
  fat_buf->count =  data_size;
  pkt_mbuf_copy_header(mbuf,fat_buf);
  fat_buf->max_data_size =  data_size;

  /* Keep current data ref in copy at same offset as in original buffer */
  __pkt_mbuf_get_sod_offset(mbuf, mbuf->data,offset);
  pkt_mbuf_debug("%s:%d  SOURCE data  Offset =  %u  \r\n",__FUNCTION__,__LINE__,offset);
  pkt_mbuf_get_data_data_ref_ad_sod_offset( fat_buf, offset, fat_buf->data);
  __pkt_mbuf_get_sod_offset(fat_buf, fat_buf->data,offset);
  pkt_mbuf_debug("%s:%d  FAT BUF data  Offset =  %u  \r\n",__FUNCTION__,__LINE__,offset);

  // Copy data from 'Head PSPBuf' + 'SG nodes'
  dest = fat_buf->data.ptr;
  src  = mbuf->data.ptr;
  memcpy(dest,src,mbuf->count);
  fat_buf->count = mbuf->count;
  dest += mbuf->count;

  if(mbuf->sg.rem_nodes)
  {
    buf  = mbuf->sg.next;
    while(buf != mbuf)
    {
      src  = buf->data.ptr;
      memcpy(dest,src,buf->count);
      dest += buf->count;
      fat_buf->count += buf->count;
      buf  = buf->sg.next;
    }
  }

  if(heap == TRUE)
    fat_buf->flags |= PKT_MBUF_MEMFLAG_HEAP;


  return fat_buf;
}



/** \ingroup PBUFAPI

\n <b>Description:</b>\n
  This API creates a new PSPBuf and copies both header fileds and data from source PSPBuf.
  Since the copy buffer has its own data region, its data area can be modified independently.

  <b>Input Parameters:</b>\n
  <i> mbuf :</i>  pointer to source PSPBuf. \n
  <i> flags :</i>  Flags

  <b>Output Parameters</b>\n
  <i> None </i>

  <b>Returns</b>\n
  <i> struct pkt_mbuf* </i> pointer to copy buffer.\n
  <i> NULL      </i> In case of failure.

 */
struct pkt_mbuf *pkt_mbuf_copy(struct pkt_mbuf *mbuf, uint32_t flags)
{
  uint32_t data_size,offset;
  bool8_t heap=TRUE;
  struct pkt_mbuf *copy = NULL;

  struct pkt_mbuf_data_ref temp_data;

  int32_t ret_value;


  // Get total data size required.
  data_size = pkt_mbuf_pkt_len(mbuf);
  // OR should this be Max data size of Original buffer
  //data_size = pkt_mbuf_max_data_size(mbuf);

  // Check flags to see if kernel mapped or heap memory is required
  if(!(flags & PKT_MBUF_MEMFLAG_HEAP))
    heap = FALSE;

  if(heap == TRUE)
  {
    pkt_mbuf_debug("%s:%d Allocate memory from Heap  !!!!!  \r\n",__FUNCTION__,__LINE__);
  }
  else
  {
    pkt_mbuf_debug("%s:%d Kernel mapped memory is requested for Copy \r\n",__FUNCTION__,__LINE__);
  }

  // Allocate the new PSPBuf with this size
  copy = pkt_mbuf_alloc(data_size,flags);
  if(!copy)
  {
    printf("%s:%d pkt_mbuf_alloc failed  \r\n",__FUNCTION__,__LINE__);
    return NULL;
  }
  pkt_mbuf_debug("%s:%d PSPBuf Allocated with data size = %u \r\n",__FUNCTION__,__LINE__,data_size);

  /* Copy PSPBuf meta data from original to copy */
  copy->count = mbuf->count;
  pkt_mbuf_copy_header(mbuf, copy);

  if(heap == TRUE)
    copy->flags |= PKT_MBUF_MEMFLAG_HEAP;

  /* Keep current data ref in copy at same offset as in original buffer */
  __pkt_mbuf_get_sod_offset(mbuf, mbuf->data,offset);
  pkt_mbuf_debug("%s:%d  SOURCE data  Offset =  %u  \r\n",__FUNCTION__,__LINE__,offset);
  pkt_mbuf_get_data_data_ref_ad_sod_offset( copy, offset, copy->data);


  __pkt_mbuf_get_sod_offset(copy, copy->data,offset);
  pkt_mbuf_debug("%s:%d Before Copy, DEST  data  Offset =  %u  \r\n",__FUNCTION__,__LINE__,offset);

  /* Now Copy the data */
  temp_data = copy->data;
  ret_value = pkt_mbuf_get_data(copy, mbuf, mbuf->count);
  if(ret_value != PSP_SUCCESS)
  {
    printf("%s:%d  data Copy failed !!! returning NULL \r\n",__FUNCTION__,__LINE__);
    pkt_mbuf_free(copy);
    return NULL;
  }
  // restore current data to original place
  copy->data = temp_data;

  __pkt_mbuf_get_sod_offset(copy, copy->data,offset);
  pkt_mbuf_debug("%s:%d  After COPY, DEST  data  Offset =  %u  \r\n",__FUNCTION__,__LINE__,offset);

  return copy;
}

/** \ingroup PBUFAPI

\n <b>Description:</b>\n
  This API creates a new PSPBuf and copies header fileds from source PSPBuf.
  Clone buffer uses the same data area as in master.
  Since the clone buffer shares its data region with original buffer, its data area cannot be modified independently.
  Only read operations are allowed on data of cloned buffers.
  When data needs to be modified, 'pkt_mbuf_copy' needs to be used.

  <b>Input Parameters:</b>\n
  <i> mbuf :</i>  pointer to source PSPBuf. \n
  <i> flags :</i>  Flags

  <b>Output Parameters</b>\n
  <i> None </i>

  <b>Returns</b>\n
  <i> struct pkt_mbuf* </i> pointer to clone buffer.\n
  <i> NULL      </i> In case of failure.

 */
struct pkt_mbuf *pkt_mbuf_clone(struct pkt_mbuf *mbuf, uint32_t flags)
{
#define PCL(x) clone_buf->x = mbuf->x
  uint32_t data_size;
  bool8_t heap=TRUE;
  struct pkt_mbuf *clone_buf = NULL;
  //uchar8_t *src,*dest;


  // For clone, no data area is allocated. Clone data points to original buffer's data.
  data_size = 0;
   flags = flags | PKT_MBUF_MEMFLAG_HEAP;

  // Check flags to see if kernel mapped or heap memory is required
  if(!(flags & PKT_MBUF_MEMFLAG_HEAP))
    heap = FALSE;

  if(heap == TRUE)
  {
    pkt_mbuf_debug("%s:%d Allocate memory from Heap   \r\n",__FUNCTION__,__LINE__);
  }
  else
  {
    printf("%s:%d Kernel mapped memory is NOT required for Clone !!!!, returning FAILURE  \r\n",__FUNCTION__,__LINE__);
    return NULL;
  }

  // Allocate the new PSPBuf with this size
  clone_buf = pkt_mbuf_alloc(data_size,flags);
  if(!clone_buf)
  {
    printf("%s:%d pkt_mbuf_alloc failed  \r\n",__FUNCTION__,__LINE__);
    return NULL;
  }
  pkt_mbuf_debug("%s:%d flags = 0x%x  \r\n",__FUNCTION__,__LINE__,clone_buf->flags);
  clone_buf->start_of_data = mbuf->start_of_data;
  clone_buf->sod_phy_addr = mbuf->sod_phy_addr;
  clone_buf->data = mbuf->data;
  clone_buf->count = mbuf->count;
  clone_buf->max_data_size = mbuf->max_data_size;

  /* Copy PSPBuf meta data from original to clone */
  pkt_mbuf_copy_header(mbuf, clone_buf);

  if(heap == TRUE)
    clone_buf->flags |= PKT_MBUF_MEMFLAG_HEAP;

  // Increment Ref count on Original
  CNTLR_LOCK_TAKE(mbuf->data_ref_lock);
  mbuf->data_ref_cnt++;
  CNTLR_LOCK_RELEASE(mbuf->data_ref_lock);
  mbuf->cloned = TRUE;

  // Store pointer to Original Buffer
  clone_buf->parent = mbuf;
  // Set the flag 'bClone'
  clone_buf->flags |=  PKT_MBUF_CLONE;


#undef PCL
 return clone_buf;
}


/*****************************************************************************/
/* Function        : pkt_mbuf_show_stats                                         */
/* Description     : This API is used to show the statistics of the           */
/*                   PSPBuf module                                           */
/* Input Arguments :                                                         */
/* Output Arguments:                                                         */
/*   pPSPBufStats  - pointer to the memory where the PSPBuf stats info has   */
/*                   to be filled.                                           */
/* Return Value    : PSP_SUCCESS - on success.                               */
/*                   Appropriate Error - on failure.                         */
/*****************************************************************************/
int32_t  pkt_mbuf_show_stats()
{
  struct mempool_stats  mempool_stats;
  struct pkt_mbuf_stats      stats;
  struct pkt_mbuf_stats      *temp_stats;
  struct pkt_mbuf_pool       *buf_pool_info=NULL;

  buf_pool_info = pPSPBufPoolList_g;
  temp_stats   = &stats;

  while(buf_pool_info)
  {
    mempool_get_stats(buf_pool_info->mem_pool, &mempool_stats);

    temp_stats->block_size        = mempool_stats.block_size;
    temp_stats->max_static_bufs   = mempool_stats.static_blocks;
    temp_stats->available_static_bufs = mempool_stats.free_blocks_count;
    temp_stats->max_heap_bufs     = mempool_stats.heap_allowed_blocks_count;
    temp_stats->available_heap_bufs   = mempool_stats.heap_free_blocks_count;

    temp_stats->alloc_hits       = mempool_stats.allocated_blocks_count +
                                    mempool_stats.allocation_fail_blocks_count +
                                    mempool_stats.heap_allocated_blocks_count +
                                    mempool_stats.heap_allocation_fail_blocks_count;

    temp_stats->release_hits     = mempool_stats.released_blocks_count +
                                    mempool_stats.released_fail_blocks_count +
                                    mempool_stats.heap_released_blocks_count;

    temp_stats->alloc_fails      = mempool_stats.allocation_fail_blocks_count +
                                    mempool_stats.heap_allocation_fail_blocks_count;

    temp_stats->release_fails    = mempool_stats.released_fail_blocks_count;


    printf("************ PSPBuf Stats  **************\r\n");
    printf("PSPBuf Block Size       =          %u\r\n",temp_stats->block_size);

    printf("Max Static Bufs         =          %u\r\n",temp_stats->max_static_bufs);
    printf("Available Static Bufs   =          %u\r\n",temp_stats->available_static_bufs);
    printf("Available Heap   Bufs   =          %u\r\n",temp_stats->available_heap_bufs);
    printf("Allocation Hits         =          %u\r\n",temp_stats->alloc_hits);
    printf("Release    Hits         =          %u\r\n",temp_stats->release_hits);
    printf("Allocation Fails        =          %u\r\n",temp_stats->alloc_fails);
    printf("Release Fails           =          %u\r\n",temp_stats->release_fails);

    buf_pool_info = buf_pool_info->next;
  }
  return PSP_SUCCESS;
}



#define __pkt_mbuf_show_data(mbuf,num_bytes)\
{\
  int32_t valid_bytes,bytes_count;\
  uchar8_t *data;\
  struct pkt_mbuf *buf = mbuf;\
  bytes_count = num_bytes;\
  printf("\r\n ******** PSPBuf data  STARTS ************\r\n");\
  data = buf->data.ptr;\
  while(bytes_count)\
  {\
    /*valid_bytes = buf->max_data_size - ( data - buf->start_of_data);*/\
    valid_bytes = buf->count;\
    printf(" bytes_count = %u, valid_bytes = %d \r\n",bytes_count,valid_bytes);\
    if(bytes_count < valid_bytes)\
    {\
      printf("\r\n DATA is :\r\n");\
      __printchar(data,bytes_count)\
      break;\
    }\
    else\
    {\
      __printchar(data,valid_bytes)\
      bytes_count -= valid_bytes; \
      buf = buf->sg.next;\
      if(buf == mbuf)\
      {\
        printf("\r\n No more bytes available !!! \r\n");\
        break;\
      } \
      data = buf->data.ptr;\
      /*data = buf->start_of_data;*/\
      printf("\r\n NEXT SG NODE \r\n");\
    }\
  }\
  printf(" ******** PSPBuf data  ENDS ************\r\n");\
}
#define __pkt_mbuf_queue_show_data(buf_queue,num_bytes)\
{\
  int32_t valid_bytes,bytes_count;\
  uchar8_t *data;\
  struct pkt_mbuf *buf,*mbuf;\
  buf =  mbuf = (struct pkt_mbuf*)DLL_QUEUE_FIRST(buf_queue) ;\
  bytes_count = num_bytes;\
  printf("\r\n ******** PSPBuf data  STARTS  Head = %p ************\r\n",mbuf);\
  data = buf->data.ptr;\
  while(bytes_count)\
  {\
    /*valid_bytes = buf->max_data_size - ( data - buf->start_of_data);*/\
    valid_bytes = buf->count;\
    /*printf(" bytes_count = %u, valid_bytes = %d \r\n",bytes_count,valid_bytes);*/\
    /*printf(" pPrev = %p buf = %p  pNext = %p \r\n", ((struct dll_queue_node*)buf)->pPrev,buf,((struct dll_queue_node*)buf)->pNext);*/\
    /*printf(" pPrev = %p buf = %p  pNext = %p \r\n",buf->pPrev,buf,buf->pNext);*/\
    if(bytes_count <= valid_bytes)\
    {\
      __printchar(data,bytes_count)\
      break;\
    }\
    else\
    {\
      __printchar(data,valid_bytes)\
      bytes_count -= valid_bytes; \
      /*buf = DLL_QUEUE_NEXT(buf_queue,(struct dll_queue_node*)buf);*/\
      buf = DLL_QUEUE_NEXT(buf_queue,buf);\
      if( !buf )\
      {\
        printf("\r\n Next Buf is NULL...No more bytes available !!! \r\n");\
        break;\
      } \
      if( buf == mbuf)\
      {\
        printf("\r\n No more bytes available buf = %p  \r\n",buf);\
        break;\
      } \
      data = buf->data.ptr;\
      /*printf("\r\n Next PSPBuf in Q \r\n");*/\
    }\
  }\
  printf("\r\n ******** PSPBuf data  ENDS ************\r\n");\
}


#ifdef NOT_USED
/* This API advances the given data reference by specified number of bytes
 * It returns the actual number of bytes pulled.
-- duplicate definition.
 */
int32_t pkt_mbuf_pull_data_ref(struct pkt_mbuf *mbuf, struct pkt_mbuf_data_ref *data_ref,uint32_t num_bytes)
{
  struct pkt_mbuf_data_ref orig_data;
  uint32_t bytes_pulled=0;

  orig_data  =  *data_ref;
__pkt_mbuf_pull_data_ref(mbuf,orig_data,num_bytes,bytes_pulled);
  *data_ref = orig_data;

  return bytes_pulled;
}
#endif

/* This API returns PSP data pointer to the data at given offset.
 * This API can be used by the users of PSP.
 */
int32_t pkt_mbuf_get_data_ref_at_offset(struct pkt_mbuf *mbuf, uint32_t offset,struct pkt_mbuf_data_ref *data_ref,uint32_t *bytes_valid)
{
  struct pkt_mbuf_data_ref data;
  uint32_t        valid_bytes = 0;

  __pkt_mbuf_get_data_ref_at_offset(mbuf,offset,data,valid_bytes);

  if(valid_bytes)
  {
    *data_ref  = data;
    *bytes_valid     = valid_bytes;
    return PSP_SUCCESS;
  }
  return PSP_FAILURE;
}


/*
 * This API is used to update the Network and Transport header pointers and
 * filling the Network and Transport Layer Information in given psp buffer.
 * NOTE: this API assumes that current data pointer is pointing to start of IP Header.
 */
void pkt_mbuf_fill_hdrs_info_in_pkt_mbuf(struct pkt_mbuf *mbuf)
{
  /* Set Network header pointer to the start of data ptr */
  pkt_mbuf_nw_hdr(mbuf) = mbuf->data;
  /* Fill IP Header Information in PSP Buffer */
  pkt_mbuf_parse_ipv4_tuple_ex(mbuf);

  /* Set the buf ptr */
  mbuf->transport_hdr.buf = mbuf->data.buf;
  /* Set Transport header pointer to the start of data ptr + IP Hdr Length */
  if(pkt_mbuf_is_sg_valid(mbuf) != TRUE)
  {
    pkt_mbuf_xport_hdr_ptr(mbuf) = \
          (pkt_mbuf_nw_hdr_ptr(mbuf) +  pkt_mbuf_ipv4_hdr_len(mbuf));
    /* Fill Transport Header Information in PSP Buffer */
    pkt_mbuf_parse_xport_tuple_ex(mbuf);
  }
  else
  {
    /* Need to Take Care of SG Buffers */
    if(pkt_mbuf_count_pkt_bytes(mbuf) >= (pkt_mbuf_ipv4_hdr_len(mbuf) + 4))
    {
      pkt_mbuf_xport_hdr_ptr(mbuf) = \
          (pkt_mbuf_nw_hdr_ptr(mbuf) +  pkt_mbuf_ipv4_hdr_len(mbuf));
      /* Fill Transport Header Information in PSP Buffer */
      pkt_mbuf_parse_xport_tuple_ex(mbuf);
    }
    else
    {
      printf("%s:: SG buffer, no transport header in first head node !!! case not handled !!! \n", __FUNCTION__);
    }
  }
  return;
}

#ifdef PKT_MBUF_TESTING
/************ Test Functions -  START     *******************************/
/****** TEST funcions START  *****/
extern int32_t mempool_show_stats (uchar8_t  *pool_name);
void pbufs()
{
  struct pkt_mbuf_pool *buf_pool_info=NULL;

  buf_pool_info = pPSPBufPoolList_g;

  while(buf_pool_info)
  {
   mempool_show(buf_pool_info->mem_pool);
   //mempool_show_stats("PSPBufPool-3144");
   buf_pool_info = buf_pool_info->next;
  }
}



int32_t PSPBufPushData(struct pkt_mbuf *mbuf, uint32_t ulNoOfBytes)
{
  struct pkt_mbuf_data_ref temp_data;
  uint32_t ulBytesPushed=0;

  temp_data = mbuf->data;
  __pkt_mbuf_push_data_ref(mbuf, temp_data,ulNoOfBytes,ulBytesPushed);
  mbuf->data = temp_data;
  mbuf->count += ulBytesPushed;
  if(ulNoOfBytes != ulBytesPushed)
    return PSP_FAILURE;
  return PSP_SUCCESS;
}

void pspShowPSPBuf( struct pkt_mbuf *mbuf)
{
  struct pkt_mbuf_pool *buf_pool=NULL;

  if(pkt_mbuf_dbg)
  {
    printf("%s:%d   HW Area Size = %u \r\n",__FUNCTION__,__LINE__,pkt_mbuf_hw_area_size());
    printf("%s:%d   SKIP Area Size = %u \r\n",__FUNCTION__,__LINE__,pkt_mbuf_prepend_size());
    printf("%s:%d   PREPEND Area Size = %u \r\n",__FUNCTION__,__LINE__,pkt_mbuf_prepend_size());
    printf("%s:%d   APPEND Area Size = %u \r\n",__FUNCTION__,__LINE__,pkt_mbuf_append_size());
    printf("%s:%d   Appl Scratch Area Size = %u \r\n",__FUNCTION__,__LINE__,pkt_mbuf_appl_scratch_area_size());
    printf("%s:%d   Third Party Appl Area Size = %u \r\n",__FUNCTION__,__LINE__,pkt_mbuf_third_party_appl_area_size());

    printf("%s:%d   PSPBuf Pointer  = %p \r\n",__FUNCTION__,__LINE__,mbuf);
    printf("%s:%d   HW Area Pointer  = %p \r\n",__FUNCTION__,__LINE__,pkt_mbuf_hw_area_ptr(mbuf));

    printf("        DataBufPhyAddr   = %x  \r\n",(uint32_t)mbuf->buf_phy_addr);
    printf("%s:%d   HW Area PHY ADDR  = %u \r\n",__FUNCTION__,__LINE__,pkt_mbuf_hw_area_phy_addr(mbuf));


    printf("%s:%d   SKIP Area Pointer  = %p \r\n",__FUNCTION__,__LINE__,pkt_mbuf_skip_area_ptr(mbuf));
    printf("%s:%d   SoD  Pointer  = %p \r\n",__FUNCTION__,__LINE__,pkt_mbuf_sod_ptr(mbuf));

    mbuf->start_of_data  =  pkt_mbuf_sod_ptr(mbuf);
    printf("pSoD             = %p  \r\n",mbuf->start_of_data);
    printf("data            = %p  \r\n",mbuf->data.ptr);

    printf("sg Next           = %p  \r\n",mbuf->sg.next);
    printf("sg Prev           = %p  \r\n",mbuf->sg.prev);
    printf("sg Head           = %p  \r\n",mbuf->sg.head);
    printf("Sg Num Nodes      = %u  \r\n",mbuf->sg.rem_nodes);
    printf("Count            = %d  \r\n",mbuf->count);
    printf("SrcNsid          = %u  \r\n",mbuf->ns_id);
    printf("TimeStamp        = %lld  \r\n",mbuf->time_stamp);

    printf("Flags            = 0x%x \r\n",mbuf->flags);
    printf("PktType          = 0x%x \r\n",mbuf->pkt_type);
    printf("L2Proto          = %d  \r\n",mbuf->l2_proto);
    printf("L3Proto          = %d  \r\n",mbuf->l3_proto);

    printf("pMacHdr          = %p  \r\n",mbuf->mac_hdr.ptr);
    printf("pNetworkHdr      = %p  \r\n",mbuf->network_hdr.ptr);
    printf("ptransport_hdr    = %p  \r\n",mbuf->transport_hdr.ptr);

    printf("pMacHdr          = %p  \r\n",pkt_mbuf_mac_hdr_ptr(mbuf));
    printf("pNetworkHdr      = %p  \r\n",pkt_mbuf_nw_hdr_ptr(mbuf));
    printf("ptransport_hdr    = %p  \r\n",pkt_mbuf_xport_hdr_ptr(mbuf));

    printf("DataLen          = %u  \r\n",mbuf->count);
    printf("SgLen            = %u  \r\n",mbuf->sg.rem_len);
    printf("PktLen           = %u  \r\n", pkt_mbuf_pkt_len(mbuf));

    buf_pool = (struct pkt_mbuf_pool *) mbuf->buf_pool;
    if(buf_pool)
    {
      printf("buf_pool         = %p  \r\n",mbuf->buf_pool);
      printf("mempool         = %p  \r\n",buf_pool->mem_pool);
    }

    printf("Neighbour pPrev  = %p  \r\n",mbuf->pPrev);
    printf("Neighbour pNext  = %p  \r\n",mbuf->pNext);

    printf("freertn          = %p  \r\n",mbuf->freertn);
    printf("arg_p             = %p  \r\n",mbuf->arg);
  }
}

void test_fn()
{
  uint32_t     size;
  struct safe_ref rxIfaceId, txIfaceId;
  uchar8_t *ptr;

  struct pkt_mbuf  PspBuf;
  struct pkt_mbuf  *mbuf = &PspBuf;


  size = pkt_mbuf_hw_area_size();
  size = pkt_mbuf_prepend_size();
  size = pkt_mbuf_appl_scratch_area_size();
  size = pkt_mbuf_third_party_appl_area_size();

  ptr = pkt_mbuf_hw_area_ptr(mbuf);
  ptr = pkt_mbuf_skip_area_ptr(mbuf) ;
  ptr = pkt_mbuf_sod_ptr(mbuf);
  ptr = pkt_mbuf_appl_scratch_area_ptr(mbuf)  ;
  ptr = pkt_mbuf_third_party_appl_area_ptr(mbuf);

  ptr = pkt_mbuf_mac_hdr_ptr(mbuf);
  ptr = pkt_mbuf_xport_hdr_ptr(mbuf)  ;
  ptr = pkt_mbuf_nw_hdr_ptr(mbuf);

  size = pkt_mbuf_count_pkt_bytes(mbuf) ;
  rxIfaceId = pkt_mbuf_rxiface_id(mbuf);
  txIfaceId = pkt_mbuf_txiface_id(mbuf);
  size = pkt_mbuf_ns_id(mbuf);
//  size = PSPBUF_DST_NSID(mbuf);

  size = pkt_mbuf_src_ip(mbuf);
  size = pkt_mbuf_dst_ip(mbuf);

  size = pkt_mbuf_src_port(mbuf);
  size = pkt_mbuf_dst_port(mbuf);
  size = pkt_mbuf_spi(mbuf);
  size = pkt_mbuf_icmp_code(mbuf);
  size =  pkt_mbuf_icmp_id (mbuf);

/* Following macros define PSPBuf flags stored in 'flags' member */

  size =  pkt_mbuf_sg_valid(mbuf);
  size =  pkt_mbuf_nwhdr_ptr_valid(mbuf);
  size =  pkt_mbuf_xporthdr_ptr_valid(mbuf);


/* The following macros can be used to access different fields of IP header in the packet
 * WARNING !!! We need to make sure that we can read sufficient number of bytes starting from IP header.
 */
 ptr  = pkt_mbuf_ipv4_hdr_ptr(mbuf);
 ptr  =  pkt_mbuf_ipv4_tos_ptr(mbuf);
 ptr  = PSPBUF_IPV4_TOTLEN_PTR(mbuf);
 ptr  = pkt_mbuf_ipv4_id_ptr(mbuf);
 ptr  = pkt_mbuf_ipv4_fragoff_ptr(mbuf);
 ptr  = pkt_mbuf_ipv4_ttl_ptr(mbuf);
 ptr  = pkt_mbuf_ipv4_proto_ptr(mbuf);
 ptr  = pkt_mbuf_ipv4_cksum_ptr(mbuf);
 ptr  = pkt_mbuf_ipv4_src_ip_ptr(mbuf);
 ptr  = pkt_mbuf_ipv4_dst_ip_ptr(mbuf);

// ___PSPBufWriteData(mbuf,DataPtr,pWrData,num_bytes,ulBytesWritten);
//  size = PSPBUF_IPV4_PAYLOAD_PTR(mbuf);
}
void test_alloc();
void test_copy();
void test_clone();
void test_ReadWrite();
void test_Flatten();
void test_sgcopy();
void test_dcbz();

void pbtst();
void testAppQs();

void pbtst()
{
  pbuftest(0);
}

int32_t pbuftest(int32_t cmd)
{
  pkt_mbuf_dbg=1;
  switch(cmd)
  {
    case 1:
      test_alloc();
      break;

    case 2:
   test_copy();
      break;

    case 3:
   test_clone();
      break;

    case 4:
    test_ReadWrite();
      break;

    case 5:
    test_Flatten();
      break;

    case 6:
   test_sgcopy();
      break;

    case 7:
      test_dcbz();
      break;

    case 8:
       testAppQs();
      break;

   default:
    printf(" Call pbuftest() with one of the following options: \n") ;
    printf(" AllocFree: 1 ") ;
    printf(" Copy     : 2 ") ;
    printf(" Clone    : 3 ") ;
    printf("\n") ;
    printf(" ReadWrite: 4 ") ;
    printf(" Flatten  : 5 ") ;
    printf(" SG Copy  : 6 ") ;
    printf("\n") ;
    printf(" DCBZ: 7 ") ;
    printf(" Test Application Queue API : 8 ") ;
    printf("\n") ;
    break;
  }

  pkt_mbuf_dbg=0;
  return PSP_SUCCESS;
}


void testAppQs()
{
  uchar8_t data1[32],data2[32],data3[32];
  //uint32_t bytes_read=0,ulBytesPushed=0;
  //uint32_t ulBytesWritten=0;
  //struct pkt_mbuf_data_ref DataPtr;
  int32_t ii=0;


  struct pkt_mbuf *buf,*pPSPBuf1, *pPSPBuf2, *pPSPBuf3;
  uint32_t data_size,flags=0;

  struct dll_queue  BufQ,*buf_queue;
  buf_queue = &BufQ;

  //Allocation data size
  data_size = 1024;
  flags |= PKT_MBUF_HW_AREA_REQD;

  pPSPBuf1 = pPSPBuf2 = pPSPBuf3 = NULL;
  // Fill input and output data
  for(ii = 0;ii<10;ii++)
  {
    data1[ii] = 'a'+ii;
    data2[ii] = 'A'+ii;
    data3[ii] = '0'+ii;
  }

  /* Buffer 1 */
  pPSPBuf1 = pkt_mbuf_alloc( data_size,flags);
  if(!pPSPBuf1)
  {
    printf("%s:%d pkt_mbuf_alloc() failed !!  \r\n",__FUNCTION__,__LINE__);
    return ;
  }
  printf("\r\n pPSPBuf1 = %p \r\n",pPSPBuf1);
  //printf("pSoD  = %p \r\n",pPSPBuf1->start_of_data);
  //printf("data.ptr  = %p \r\n",pPSPBuf1->data.ptr);
  pPSPBuf1->count = 10;
  // Fill input data
  for(ii = 0;ii<10;ii++)
  {
    pPSPBuf1->data.ptr[ii]   = data1[ii];
  }
  printf(" data in Buffer 1 : \r\n");
  __printchar(pPSPBuf1->data.ptr,10);\

#if 1 //def TEST_BUFDUP
  pkt_mbuf_dup_p( &buf,
              pPSPBuf1,
              0/* uiOffset*/,
              2/*Count*/,
              0/*ExtraHdrSize*/);
  printf("\r\n two bytes data in Duped Buffer @offset = 0 \r\n");
  __printchar(buf->data.ptr,2);\

  pkt_mbuf_dup_p( &buf,
              pPSPBuf1,
              4/* uiOffset*/,
              4/*Count*/,
              0/*ExtraHdrSize*/);
  printf("\r\n four bytes data in Duped Buffer @offset = 4 \r\n");
  __printchar(buf->data.ptr,4);
#endif

  /* Buffer 2 */
  pPSPBuf2 = pkt_mbuf_alloc( data_size,flags);
  if(!pPSPBuf2)
  {
    printf("%s:%d pkt_mbuf_alloc() failed !!  \r\n",__FUNCTION__,__LINE__);
    return ;
  }
  printf(" \r\n pPSPBuf2 = %p \r\n",pPSPBuf2);
  pPSPBuf2->count = 10;
  for(ii = 0;ii<10;ii++)
  {
    pPSPBuf2->data.ptr[ii]   = data2[ii];
  }
  printf("data in Buffer 2 :\r\n");
  __printchar(pPSPBuf2->data.ptr,10)\

  /* Buffer 3 */
  pPSPBuf3 = pkt_mbuf_alloc( data_size,flags);
  if(!pPSPBuf3)
  {
    printf("%d pkt_mbuf_alloc() failed !!  \r\n",__LINE__);
    return ;
  }
  printf("\r\n pPSPBuf3 = %p \r\n",pPSPBuf3);
  pPSPBuf3->count = 10;
  for(ii = 0;ii<10;ii++)
  {
    pPSPBuf3->data.ptr[ii]   = data3[ii];
  }
  printf(" data in Buffer 3 :\r\n");
  __printchar(pPSPBuf3->data.ptr,10)\

  // Create DLL Q of PSPBufs
  printf("%d  Initializing BufQ \r\n",__LINE__);
  pkt_mbuf_queue_init(buf_queue);

  // Append next Buf
  printf("%d  Appending Buffer 1 \r\n",__LINE__);
  pkt_mbuf_queue_append(buf_queue,pPSPBuf1);
  printf("%s:%d  Number of Buffers in Q = %d \r\n",__FUNCTION__,__LINE__, PSP_DLLQ_COUNT(buf_queue) );
  // Show data from Queued PSPBufs
  __pkt_mbuf_queue_show_data(buf_queue, 10);

  // Append next Buf
  printf("%d  Appending Buffer 2 \r\n",__LINE__);
  pkt_mbuf_queue_append(buf_queue,pPSPBuf2);
  printf("%d  Number of Buffers in Q = %d \r\n",__LINE__, PSP_DLLQ_COUNT(buf_queue) );
  __pkt_mbuf_queue_show_data(buf_queue, 20);

  // Append next Buf
  printf("%d  Appending Buffer 3 \r\n",__LINE__);
  //pkt_mbuf_queue_append(buf_queue,pPSPBuf3);
  printf("%d  Number of Buffers in Q = %d \r\n",__LINE__, PSP_DLLQ_COUNT(buf_queue) );
  __pkt_mbuf_queue_show_data(buf_queue, 30);


#ifdef TEST_PULLUP
  // Test Pullup
  t_memset(data1,0,sizeof(data1));
  printf("%d  Pulling up 5 bytes \r\n",__LINE__);
  pkt_mbuf_pullup(buf_queue, data1,5);
  printf("%d  Pulled up data = %s \r\n",__LINE__, data1 );
#endif

#ifdef TEST_TRIM
  // Test Trim
  buf = DLL_QUEUE_FIRST(buf_queue);
  pkt_mbuf_trim( buf_queue,&buf,3);
  printf("Trimmed  3 bytes...Number of Buffers in Q = %d \r\n", PSP_DLLQ_COUNT(buf_queue) );
  __pkt_mbuf_queue_show_data(buf_queue, 3);

  pkt_mbuf_trim( buf_queue,&buf,1);
  printf("%d  Trimmed 1 byte...Number of Buffers in Q = %d \r\n",__LINE__, PSP_DLLQ_COUNT(buf_queue) );
  __pkt_mbuf_queue_show_data(buf_queue, 1);
#endif


}

void test_alloc()
{
  struct pkt_mbuf *mbuf = NULL;
  uint32_t data_size,flags=0,offset,bytes_valid=0;
  struct pkt_mbuf_data_ref DataPtr;


  //Allocation data size
  data_size = 1024;

  flags |= PKT_MBUF_SG_ACCEPTED;
  mbuf = pkt_mbuf_alloc( data_size,flags);
  if(!mbuf)
  {
    printf("%s:%d pkt_mbuf_alloc() failed !!  \r\n",__FUNCTION__,__LINE__);
    return ;
  }
  printf("mbuf = %p \r\n",mbuf);
  printf("pSoD  = %p \r\n",mbuf->start_of_data);
  printf("data.ptr  = %p \r\n",mbuf->data.ptr);
  printf("Count   = %d  \r\n",mbuf->count);
  mbuf->network_hdr.ptr += 14;
  mbuf->transport_hdr.ptr += 34;

  DataPtr.buf = NULL;
  DataPtr.ptr = NULL;

  pspShowPSPBuf(mbuf);

  __pkt_mbuf_get_data_offset(mbuf,mbuf->data,offset);
   printf("\r\n  Offset to curr data pointer   =  %u \r\n",offset);

   // Get data Ref using Offset
   __pkt_mbuf_get_data_ref_at_offset( mbuf, offset, DataPtr, bytes_valid) ;
   printf("\r\n  User data pointer after 'GetDatRefAOffset'  =  %p\r\n",DataPtr.ptr);
   printf("\r\n  Bytes valid in the PSPBuf  =  %u\r\n",bytes_valid);

   printf("\r\n  Advancing user dataref by 5 bytes \r\n");
   DataPtr.ptr = mbuf->data.ptr+5;
   DataPtr.buf = mbuf;
   printf(" Offset to user data from curr data is %u\r\n", pkt_mbuf_get_offset_to_data_ptr(mbuf, DataPtr));
   __pkt_mbuf_get_data_offset(mbuf, mbuf->data, offset);
   printf(" Offset to user data is %u\r\n", offset);

  pkt_mbuf_free(mbuf);
}


void test_copy()
{
  struct pkt_mbuf *pCopy1;
  struct pkt_mbuf *mbuf = NULL;
  uint32_t data_size,flags=0;
  int32_t ii;

  //Allocation data size
  data_size = 1024;

  flags |= PKT_MBUF_SG_ACCEPTED;
  mbuf = pkt_mbuf_alloc( data_size,flags);
  if(!mbuf)
  {
    printf("%s:%d pkt_mbuf_alloc() failed !!  \r\n",__FUNCTION__,__LINE__);
    return ;
  }
  printf("pkt_mbuf_alloc() success  mbuf = %p \r\n",mbuf);
  printf("pSoD  = %p \r\n",mbuf->start_of_data);
  printf("data.ptr  = %p \r\n",mbuf->data.ptr);
  printf("Count   = %d  \r\n",mbuf->count);
  mbuf->network_hdr.ptr += 14;
  mbuf->transport_hdr.ptr += 34;

  // Fill  data
  for(ii = 0;ii<26;ii++)
  {
    mbuf->data.ptr[ii]  = 'a'+ii;
  }
  pspShowPSPBuf(mbuf);
  printf("\r\n**** PSPBuf data in Original:\r\n");
  __pkt_mbuf_show_data(mbuf,26);

  printf("%s:%d Allocating first copy  \r\n",__FUNCTION__,__LINE__);
  pCopy1 = pkt_mbuf_copy(mbuf,flags);
  if(!pCopy1)
  {
    printf("%s:%d pkt_mbuf_copy() failed !!  \r\n",__FUNCTION__,__LINE__);
    return ;
  }

  printf("%s:%d Show Copy1 =  %p  \r\n",__FUNCTION__,__LINE__,pCopy1);
  pspShowPSPBuf(pCopy1);
  printf("\r\n**** PSPBuf data in Copy:\r\n");
  __pkt_mbuf_show_data(pCopy1,26);

  // Free the first copy
  printf("%s:%d Free Copy1 = %p  \r\n",__FUNCTION__,__LINE__,pCopy1);
  pkt_mbuf_free(pCopy1);

  // Now free the Original Buf
  printf("%s:%d Free Parent Buf = %p \r\n",__FUNCTION__,__LINE__,mbuf);
  pkt_mbuf_free(mbuf);
}


void test_clone()
{
  uint32_t flags = 0;
  struct pkt_mbuf *pClone1,*pClone2;

  struct pkt_mbuf *mbuf = NULL;
  //Allocation data size
  uint32_t data_size = 1024;
  int32_t ii;

  flags |= PKT_MBUF_SG_ACCEPTED;
  mbuf = pkt_mbuf_alloc( data_size,flags);
  if(!mbuf)
  {
    printf("%s:%d pkt_mbuf_alloc() failed !!  \r\n",__FUNCTION__,__LINE__);
    return ;
  }
  printf("pSoD  = %p \r\n",mbuf->start_of_data);
  printf("data.ptr  = %p \r\n",mbuf->data.ptr);
  printf("Count   = %d  \r\n",mbuf->count);
  mbuf->network_hdr.ptr += 14;
  mbuf->transport_hdr.ptr += 34;

  // Fill  data
  for(ii = 0;ii<26;ii++)
  {
    mbuf->data.ptr[ii]  = 'a'+ii;
  }

  pspShowPSPBuf(mbuf);
  printf("\r\n**** PSPBuf data in Original:\r\n");
  __pkt_mbuf_show_data(mbuf,26);

  printf("%s:%d Initial DataRefCnt = %d   \r\n",__FUNCTION__,__LINE__, mbuf->data_ref_cnt);
  printf("%s:%d Allocating first clone   \r\n",__FUNCTION__,__LINE__);
  pClone1 = pkt_mbuf_clone(mbuf,0);
  if(!pClone1)
  {
    printf("%s:%d pkt_mbuf_clone() failed !!  \r\n",__FUNCTION__,__LINE__);
    return;
  }
  printf("%s:%d DataRefCnt = %d   \r\n",__FUNCTION__,__LINE__, mbuf->data_ref_cnt);
  printf("%s:%d Clone1 = %p  flags = 0x%x  \r\n",__FUNCTION__,__LINE__,pClone1,pClone1->flags);
  printf("\r\n**** PSPBuf data in Clone1:\r\n");
  __pkt_mbuf_show_data(pClone1,26);

  printf("%s:%d Allocating second clone   \r\n",__FUNCTION__,__LINE__);
  pClone2 = pkt_mbuf_clone(mbuf,0);
  if(!pClone2)
  {
    printf("%s:%d pkt_mbuf_clone() failed !!  \r\n",__FUNCTION__,__LINE__);
    return;
  }
  printf("%s:%d DataRefCnt = %d  bCloned = %d  \r\n",__FUNCTION__,__LINE__, mbuf->data_ref_cnt,mbuf->cloned);
  printf("%s:%d Clone2 = %p flags = 0x%x  \r\n",__FUNCTION__,__LINE__,pClone2,pClone2->flags);
  printf("\r\n**** PSPBuf data in Clone2:\r\n");
  __pkt_mbuf_show_data(pClone2,26);


  // Free the first clone
  printf("%s:%d Free Clone1 flags = 0x%x  \r\n",__FUNCTION__,__LINE__,pClone1->flags);
  pkt_mbuf_free(pClone1);
  printf("%s:%d DataRefCnt = %d   \r\n",__FUNCTION__,__LINE__, mbuf->data_ref_cnt);

  // Free the second clone
  printf("%s:%d Free Clone2 flags = 0x%x  \r\n",__FUNCTION__,__LINE__,pClone2->flags);
  pkt_mbuf_free(pClone2);
  printf("%s:%d DataRefCnt = %d  bCloned = %d  \r\n",__FUNCTION__,__LINE__, mbuf->data_ref_cnt,mbuf->cloned);

  // Now free the Original Buf
  printf("%s:%d Free Parent Buf \r\n",__FUNCTION__,__LINE__);
  pkt_mbuf_free(mbuf);
}

void test_ReadWrite()
{
  uchar8_t mydata[32],outdata[32],*data=NULL;
  uint32_t bytes_read=0,ulBytesPushed=0;
  //uint32_t ulBytesWritten=0;
  struct pkt_mbuf_data_ref DataPtr;
  int32_t ii=0;

  struct pkt_mbuf *mbuf = NULL;
  uint32_t data_size,flags=0;
  //Allocation data size
  data_size = 1024;
  flags |= PKT_MBUF_SG_ACCEPTED;
  mbuf = pkt_mbuf_alloc( data_size,flags);
  if(!mbuf)
  {
    printf("%s:%d pkt_mbuf_alloc() failed !!  \r\n",__FUNCTION__,__LINE__);
    return ;
  }
  printf("mbuf = %p \r\n",mbuf);
  printf("pSoD  = %p \r\n",mbuf->start_of_data);
  printf("data.ptr  = %p \r\n",mbuf->data.ptr);
  printf("Count   = %d  \r\n",mbuf->count);

  // use macro
  mbuf->network_hdr.ptr += 14;
  mbuf->transport_hdr.ptr += 34;

  pspShowPSPBuf(mbuf);

  // Fill input and output data
  for(ii = 0;ii<26;ii++)
  {
    mydata[ii] = 'a'+ii;
    outdata[ii] = 'A';
  }

  // Test Writing
   printf("\r\n**** PSPBuf data before Write:\r\n");
   __pkt_mbuf_show_data(mbuf,26);
   DataPtr = mbuf->data;
   printf("\r\n Before WRITE User data Ptr = %p , buf = %p \r\n",DataPtr.ptr,DataPtr.buf);
   printf("\r\n Before WRITE PSPBuf data Ptr = %p , buf = %p \r\n",mbuf->data.ptr,DataPtr.buf);
   printf("\r\n Before WRITE PSPBuf SoD      = %p \r\n",DataPtr.buf->start_of_data);
   //__pkt_mbuf_put_data(mbuf,DataPtr,mydata,26,ulBytesWritten);
   // __pkt_mbuf_write_data_ext(mbuf,DataPtr,mydata,26);
    __pkt_mbuf_write_data(mbuf,mydata,26);
   DataPtr = mbuf->data;
   printf(" Offset to data is %u\r\n", pkt_mbuf_get_offset_to_data_ptr(mbuf, DataPtr));
   //printf("\r\n Number of bytes written =  %u\r\n",ulBytesWritten);
   printf("\r\n After WRITE User data Ptr = %p , buf = %p \r\n",DataPtr.ptr,DataPtr.buf);
   printf("\r\n After WRITE PSPBuf data Ptr = %p , buf = %p \r\n",mbuf->data.ptr,DataPtr.buf);
   printf("\r\n After WRITE PSPBuf SoD      = %p  \r\n",DataPtr.buf->start_of_data);

   __pkt_mbuf_push_data(mbuf,26,DataPtr,ulBytesPushed); \
    printf("\r\n Bytes pushed = %u  ** packet data after pushed:\r\n",ulBytesPushed);
   __pkt_mbuf_show_data(mbuf,26);


   /* Read the data from PSPBuf */
   printf("\r\n*** user data before Read:\r\n");
   data = outdata;
   for(ii = 0;ii<26;ii++)
     printf(" %c",data[ii]  );

   bytes_read =0;
   //DataPtr = mbuf->data;
   //bytes_read = pkt_mbuf_read_data_ext(mbuf,&DataPtr,outdata,26) ;
   bytes_read = pkt_mbuf_read_data(mbuf,outdata,26) ;
   printf("\r\n*** Bytes read = %u \r\n",bytes_read);
   printf("\r\n*** user data after Read:\r\n");
   for(ii = 0;ii<26;ii++)
     printf(" %c",data[ii]  );

   DataPtr = mbuf->data;
    __pkt_mbuf_push_data(mbuf,26,DataPtr,ulBytesPushed); \
   printf("\r\n*** Buffer data after Read:\r\n");
   __pkt_mbuf_show_data(mbuf,26);

}


void test_Flatten()
{
  struct pkt_mbuf *mbuf = NULL,*fat_buf=NULL;
  uint32_t data_size,flags=0;
  uint32_t bytes_valid=0,offset;
  int32_t ii;

  //Allocation data size
  data_size = 40;
  flags |= PKT_MBUF_SG_ACCEPTED;
  mbuf = pkt_mbuf_alloc( data_size,flags);
  if(!mbuf)
  {
    printf("%s:%d pkt_mbuf_alloc() failed !!  \r\n",__FUNCTION__,__LINE__);
    return ;
  }
  printf("mbuf = %p \r\n",mbuf);
  printf("pSoD  = %p \r\n",mbuf->start_of_data);
  printf("data.ptr  = %p \r\n",mbuf->data.ptr);
  printf("Count   = %d  \r\n",mbuf->count);

  // Fill input and output data
  for(ii = 0;ii<26;ii++)
  {
   if(ii<16)
     mbuf->data.ptr[ii]   = 'a'+ii;
   else
   {
     (mbuf->sg.next)->data.ptr[ii-16]   = 'a'+ii;
     printf(" DATAPOINTER = %p, data = \r\n", (mbuf->sg.next)->data.ptr);
   }
  }
  pspShowPSPBuf(mbuf);
  printf("\r\n ** packet data in SG buffer before Flattening:\r\n");
  __pkt_mbuf_show_data(mbuf,26);


  pkt_mbuf_debug("%s:%d   data Ref:  buf = %p  Ptr =  %p  \r\n",__FUNCTION__,__LINE__,mbuf,mbuf->data.ptr );

  __pkt_mbuf_get_data_ref_at_offset( mbuf, 0, mbuf->mac_hdr, bytes_valid);
  pkt_mbuf_debug("%s:%d   MAC data Ref:  buf = %p  Ptr =  %p  \r\n",__FUNCTION__,__LINE__,mbuf->mac_hdr.buf,mbuf->mac_hdr.ptr );
  pkt_mbuf_debug("%s:%d   Valid Bytes = %u \r\n",__FUNCTION__,__LINE__,bytes_valid);

  __pkt_mbuf_get_data_ref_at_offset( mbuf, 14, mbuf->network_hdr, bytes_valid);
  pkt_mbuf_debug("%s:%d   NETWORK data Ref:  buf = %p  Ptr =  %p  \r\n",__FUNCTION__,__LINE__,mbuf->network_hdr.buf,mbuf->network_hdr.ptr );
  pkt_mbuf_debug("%s:%d   Valid Bytes = %u \r\n",__FUNCTION__,__LINE__,bytes_valid);

  __pkt_mbuf_get_data_ref_at_offset( mbuf, 34, mbuf->transport_hdr, bytes_valid);
  pkt_mbuf_debug("%s:%d   Transport data Ref:  buf = %p  Ptr =  %p  \r\n",__FUNCTION__,__LINE__,mbuf->transport_hdr.buf,mbuf->transport_hdr.ptr );
  pkt_mbuf_debug("%s:%d   Valid Bytes = %u \r\n",__FUNCTION__,__LINE__,bytes_valid);


  flags |= PKT_MBUF_MEMFLAG_HEAP;
  fat_buf = pkt_mbuf_flatten(mbuf,0,flags);
  if(!fat_buf)
  {
    printf("%s:%d pkt_mbuf_flatten() failed !!  \r\n",__FUNCTION__,__LINE__);
    return ;
  }
  printf("%s:%d pkt_mbuf_flatten() success  pFlatBuf = %p \r\n",__FUNCTION__,__LINE__,fat_buf);
  pspShowPSPBuf(fat_buf);

  __pkt_mbuf_get_data_offset(fat_buf,fat_buf->data,offset) \
  printf("%s:%d Offset of data = %u \r\n",__FUNCTION__,__LINE__,offset);

  printf("\r\n ** packet data after Flattening:\r\n");
  __pkt_mbuf_show_data(fat_buf,26);
}


void test_sgcopy()
{
  struct pkt_mbuf *mbuf = NULL;
  struct pkt_mbuf *pPSPBuf2 = NULL;
  uint32_t data_size,flags=0;
  uint32_t ulBytesPushed=0;
  int32_t ii;

  // Allocate a Buffer so that it will be with SGs --- make only small size pool available
  data_size = 60;
  flags |= PKT_MBUF_SG_ACCEPTED;
  mbuf = pkt_mbuf_alloc( data_size,flags);
  if(!mbuf)
  {
    printf("%s:%d pkt_mbuf_alloc() failed !!  \r\n",__FUNCTION__,__LINE__);
    return ;
  }
  printf("mbuf = %p \r\n",mbuf);
  printf("pSoD  = %p \r\n",mbuf->start_of_data);
  printf("data.ptr  = %p \r\n",mbuf->data.ptr);
  printf("Count   = %d  \r\n",mbuf->count);

  // Fill input and output data
  for(ii = 0;ii<26;ii++)
  {
   if(ii<16)
     mbuf->data.ptr[ii]   = 'a'+ii;
   else
   {
     (mbuf->sg.next)->data.ptr[ii-16]   = 'a'+ii;
     printf(" DATAPOINTER = %p, data = \r\n", (mbuf->sg.next)->data.ptr);
   }
  }
  printf("%s:%d data in  PSPBuf: \r\n",__FUNCTION__,__LINE__);
   __pkt_mbuf_show_data(mbuf,26);

  // Now copy to another PSPBuf and see output
  pPSPBuf2 = pkt_mbuf_alloc( data_size,flags);
  if(!pPSPBuf2)
  {
    printf("%s:%d pkt_mbuf_alloc() failed !!  \r\n",__FUNCTION__,__LINE__);
    return ;
  }
  printf("%s:%d pkt_mbuf_alloc() success  pPSPBuf22 = %p \r\n",__FUNCTION__,__LINE__,pPSPBuf2);
  //printf("%s:%d PSPBuf2: data ptr 1 = %p \r\n",__FUNCTION__,__LINE__,pPSPBuf2->start_of_data);
  //printf("%s:%d PSPBuf2: data ptr 2 = %p \r\n",__FUNCTION__,__LINE__,pPSPBuf2->sg.next->start_of_data);

  pkt_mbuf_copy_data(pPSPBuf2, mbuf, 26);

  __pkt_mbuf_push_data_ref(pPSPBuf2, pPSPBuf2->data ,26,ulBytesPushed);

  printf("%s:%d After Push, data in  PSPBuf2: \r\n",__FUNCTION__,__LINE__);
   __pkt_mbuf_show_data(pPSPBuf2,26);
}

#if 0
void test_dcbz()
{
  uchar8_t *data=NULL;

  struct pkt_mbuf *mbuf = NULL;
  uint32_t data_size,flags=0;
  int32_t ii, nc_lines;

#ifndef CONFIG_PSP_BOARD_X86

  //Allocation data size
  data_size = 1024;
  flags |= PKT_MBUF_SG_ACCEPTED;
  mbuf = pkt_mbuf_alloc( data_size,flags);
  if(!mbuf)
  {
    printf("%s:%d pkt_mbuf_alloc() failed !!  \r\n",__FUNCTION__,__LINE__);
    return ;
  }
  printf("mbuf = %p \r\n",mbuf);
  printf("pSoD  = %p \r\n",mbuf->start_of_data);
  printf("data.ptr  = %p \r\n",mbuf->data.ptr);
  printf("Count   = %d  \r\n",mbuf->count);

  data = (uchar8_t*)mbuf;

  printf("sizeof PSPBuf = %d, Cache line size = %d  \n",sizeof(struct pkt_mbuf),PSP_CACHE_LINE_SIZE) ;
  nc_lines = sizeof(struct pkt_mbuf)/PSP_CACHE_LINE_SIZE;
  printf(" Number of Cache lines = %d ,   remaining bytes = %d \r\n",nc_lines, (sizeof(struct pkt_mbuf)%PSP_CACHE_LINE_SIZE) );


  for(ii = 0; ii < 64; ii++)
  {
    *(data + ii) = 0x80;
  }
  PSP_DATA_CACHE_BLOCK_CLEARn(data, nc_lines);
  // for testing
  for(ii = 0; ii < 64; ii++)
  {
    if(*(data + ii) != 0)
    {
      printf("%s::data not cleared at offset %d\n", __FUNCTION__, ii);
      break;
    }
  }
#else
      printf(" NOT SUPPORTED FOR X86 \n");

#endif
}
#endif


/****** TEST funcions END  *****/
#endif

