/**************************************************************************
 * Copyright 2011-2012, Freescale Semiconductor, Inc. All rights reserved.
 ***************************************************************************/
/*
 * File:	pktmbufldef.h
 *
 * Description: pkt_mbuf local include file.
 *
 * Authors:	malik.mlr@freescale.com
 * Modifier:
 *
 */
/*
 * History
 * 08 Oct 2013 - Mallik - Initial Implementation.
 *
 */
/******************************************************************************/
#ifndef __PKTMBUFLDEF_H
#define __PKTMBUFLDEF_H

#define PSP_BUF_POOL_NAME_MAX  32

#if 0
#define PSP_BUFPOOL_THREAD_THRESHOLD 5 /* TBD: How to decide this ??? */
#endif


/*
 * structure to store pkt_mbuf statistics
 */
struct pkt_mbuf_stats
{
        uint32_t  block_size;
        uint32_t  current_active_bufs;
        uint32_t  max_static_bufs;
        uint32_t  max_heap_bufs;

        uint32_t  available_static_bufs;
        uint32_t  available_heap_bufs;

        uint32_t  alloc_hits;
        uint32_t  release_hits;
        uint32_t  alloc_fails;
        uint32_t  release_fails;
};



struct pkt_mbuf*  pkt_mbuf_alloc_by_pool (struct pkt_mbuf_pool *pPool);
int32_t cntlr_register_pktbuf_callbacks(
		struct pktbuf_callbacks *pktbuf_callbacks_p);
int32_t cntlr_deregister_pktbuf_callbacks(
		struct pktbuf_callbacks *pktbuf_callbacks_p);

#define PSP_DATA_CACHE_BLOCK_CLEARn

#if 0
t_int32 pkt_mbuf_conf_get_first_pkt_mbuf_pool_info(void *pDOMDoc, uint32_t *ulDataSize, uint32_t *ulPoolSize);
t_int32 pkt_mbuf_conf_get_next_pkt_mbuf_pool_info(void *pConfigDOM, uint32_t *ulDataSize, uint32_t *ulNumBufs);
#endif


#endif
