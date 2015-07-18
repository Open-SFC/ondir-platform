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

/**************************************************************************/
/*  File        : httpsmbl.c                                              */
/*                                                                        */
/*  Description : This file contains the definitions of all the functions */
/*                that form mblk buffer management. It has a table of     */
/*                pointers to data blocks indexed on the pool ID.         */
/*                                                                        */
/*                Each of these pools can configure the sizes of the data */
/*                blocks and the number of buffers that each size should  */
/*                have.                                                   */
/*                                                                        */
/*                Allocation is done from a pool if data buffers are      */
/*                available. A list of HTTPS_mblk_t structures are also   */
/*                held in the pool from which the message block headers   */
/*                are allocated for that pool.                            */
/*                                                                        */
/*                The data buffers are maintained in a linked list in     */
/*                increasing order of the size and the best-fit algorithm */
/*                is used to allocate buffers.                            */
/*                                                                        */
/*  Version      Date      Author      Change Description                 */
/*  -------    --------    ------    ----------------------               */
/*    1.1      03.31.04    DRAM      Modified during the code review.     */
/**************************************************************************/

#if defined(OF_HTTPD_SUPPORT) && defined(OF_HTTPS_SUPPORT)

/***********************************************************
 * * * *  I n c l u d e    H e a d e r    f i l e s  * * * *
 ***********************************************************/

#include "httpdinc.h"
#include "hcmgrmbl.h"
#include "hcmgrgdf.h"
#include "hcmgrgdf.h"

/****************************************************
 * * * *  M a c r o    D e f i n i t i o n s  * * * *
 ****************************************************/

#define  HTTPS_MBLK_MAX_SETS          (4)
#define  HTTPS_MBLK_COUNT_SIZE        (50)
#define  HTTPS_MBLK_ENTRY_SIZE        (1024)

/****************************************************
 * * * *  T y p e    D e c l a r a t i o n s  * * * *
 ****************************************************/

typedef void (*voidFUNPTR) (void *);

/************************************************************
 * * * *  V a r i a b l e    D e c l a r a t i o n s  * * * *
 ************************************************************/

/**
 * An array of pools from which message and data blocks are allocated
 * The pool from which the allocation should occur can be given or
 * they are allocated from the pool id.
 */
 HTTPS_DataPool_t  HttpsdataPool[HTTPS_MAX_MBLK_POOLS];


/**
 * Default Pool ID.
 */
 int32_t  T_lMblkDefaultPool;

extern uint32_t HttpsPoolId;

/**
 * Contains the Pool names and the corresponding pool IDs
 *  HTTPS_PoolHash_t MapPoolName[HTTPS_MAX_MBLK_POOLS];
 */
uint32_t  ulNumOfHttpsDbAllocs_g = 0;

/**
 * This Mblock Configuration Structure can be tuned. The
 * HTTPS_MBLK_MAX_SETS can also be changed to increase mblock sets.
 *
 * NOTE : The last set must be set to NULL.
 */
HTTPS_MblkBufConfig_t HTTPS_bufcfg[HTTPS_MBLK_MAX_SETS] =
{
 /* count,  size */
  { HTTPS_MBLK_COUNT_SIZE,     HTTPS_MBLK_ENTRY_SIZE },
  { 0,                         0                     }
};

/**
 * #define HTTPS_DEBUG  1
 */

/********************************************************
 * * * *  F u n c t i o n    P r o t o t y p e s  * * * *
 ********************************************************/

/**
 * Function to initialize the mblocks in a pool.
 */

unsigned char *HttpsInitMblocks( int32_t   lPoolId,
                            unsigned char  *pBuffer,
                            int32_t   *pBufLength,
                            int32_t   lNoMblks );

/**
 * Function to initialize the data blocks in a pool.
 */

unsigned char *HttpsInitDblocks( int32_t                lPoolId,
                            unsigned char               *pBuffer,
                            int32_t                *pBufLength,
                            HTTPS_MblkBufConfig_t  *pBufCfg,
                            int16_t                iAlignment );

/**
 * Function to allocate data blocks of the given size from the given pool.
 */

HTTPS_dblk_t *HttpsDbAlloc( int32_t  lPoolId,
                            int32_t  lSize );

/**
 * Function to free the data block and the data buffer to the pool.
 */

void HttpsDbFree( HTTPS_dblk_t  *dp );

/**
 * Function to allocate the message block header
 */

HTTPS_mblk_t *HttpsMbAlloc( int32_t  lPoolId );

/**
 * Function to free the message block header.
 */
void HttpsMbFree( HTTPS_mblk_t  *mp );

/**
 * Function to calculate the number of bytes in the message.
 * /

int32_t HttpsMsgSize( HTTPS_mblk_t  *mp );*/

/**
 * This function is used to initialize the buffer management.
 */

unsigned char *HttpsMblkInit( int32_t                *lPoolId,
                         char                *pPoolName,
                         unsigned char               *pBuffer,
                         int32_t                lBufLength,
                         int32_t                lNoMblks,
                         HTTPS_MblkBufConfig_t  *pBufCfg,
                         int16_t                iAlignment,
                         bool                bDefault,
                         void                 (*pFreeFn)(void*),
                         void                 *pFreeArg );

/**
 * This function is used to initialize and install the CLI command leaves.
 */

void HttpsMblkCmdInit( void );

/****************************************************************
 * * * *  F u n c t i o n    I m p l e m e n t a t i o n  * * * *
 ****************************************************************/

/*************************************************************************
 * Function Name : HttpsUploadCert                                       *
 * Description   : This function will actually write the                 *
 *                 self-certificate into the Mblocks.                    *
 * Input         : pSelfBuf is buffer                                    *
 * Output        :                                                       *
 * Return value  : OF_SUCCESS or OF_FAILURE.                               *
 *************************************************************************/

void HttpsInitMblks(void)
{
  uint32_t  ii;
  uint32_t  ulBufSize = 0;
  uint32_t  ulMaxBufs = 0;
  unsigned char  *buffer   = NULL;
  unsigned char  *ptr      = NULL;
  int32_t   PoolId    = 0;

  /**
   * Calculate total size of memory required.
   */
  for (ii = 0; ii < HTTPS_MBLK_MAX_SETS; ii++)
  {
    ulBufSize += ((HTTPS_bufcfg[ii].ulBuffers * HTTPS_bufcfg[ii].ulBufSize) +
                  (HTTPS_bufcfg[ii].ulBuffers * sizeof(HTTPS_dblk_t))) +
                  (HTTPS_bufcfg[ii].ulBuffers * 3);

    ulBufSize += sizeof(HTTPS_DblkLink_t);
    ulMaxBufs += HTTPS_bufcfg[ii].ulBuffers;
  }

  ulBufSize += (ulMaxBufs * sizeof(HTTPS_mblk_t));

  /**
   * Allocate memory for this much size.
   */
  buffer = (unsigned char*) calloc(1, ulBufSize);

  if (buffer == NULL)
  {
#ifdef HTTPS_DEBUG
    Trace(HTTPS_ID, TRACE_SEVERE,
          "Memory Allocation for HTTPS_mblk_t s failed\n");
#endif /*HTTPS_DEBUG*/
    return;
  }

  /**
   * Initialize the default pool id with ulMaxBufs mblock headers, and the
   * configuration given in HTTPS_bufcfg for data buffers.
   */
  HttpsMblkCmdInit();

  ptr = HttpsMblkInit(&PoolId, "HTTPS", buffer, ulBufSize, ulMaxBufs,
                      HTTPS_bufcfg, 2, TRUE, NULL, NULL);

  if (ptr == NULL)
  {
#ifdef HTTPS_DEBUG
    Trace(HTTPS_ID, TRACE_SEVERE,
          "Error in initializing the mblk pools for ePPTP\n");
#endif /*HTTPS_DEBUG*/
  }
  else
  {
    HttpsPoolId = PoolId;
  }
} /* HttpsInitMblks() */

/***************************************************************************
 * Function Name : HttpsMblkInit
 * Description   : Initializes the mlock header and data buffers
 *                 for a pool. It calls the routines HttpsInitMblocks and
 *                 HttpsInitDblocks to initialize the pool.
 * Input         : char*          -- Pool Name
                   unsigned char*         -- Data Segment
                   int32_t           -- Number of message block headers
                   HTTPS_MblkBufConfig_t*  -- Pointer to buffer configuration
                   int16_t           -- Alignment for data buffer
                   bool           -- Whether this is the default pool
 * Output        : int32_t*          -- Pool ID
 * Return value  : unsigned char*  -- Updated Data Segment
 ***************************************************************************/

unsigned char *HttpsMblkInit( int32_t                *lPoolId,
                         char                *pPoolName,
                         unsigned char               *pBuffer,
                         int32_t                lBufLength,
                         int32_t                lNoMblks,
                         HTTPS_MblkBufConfig_t  *pBufCfg,
                         int16_t                iAlignment,
                         bool                bDefault,
                         voidFUNPTR           pFreeFn,
                         void                 *pFreeFnArg )
{
  int32_t  ii;

  for (ii = 0; ii < HTTPS_MAX_MBLK_POOLS; ii++)
  {
    if (!HttpsdataPool[ii].bInUse)
    {
      break;
    }
  }

  if (ii == HTTPS_MAX_MBLK_POOLS)
  {
    *lPoolId = -1;
    return (NULL);
  }

  *lPoolId = ii;

  /*---------------------------------------------------------------------*/
  /* Configure the buffers specified in the table                        */
  /*---------------------------------------------------------------------*/
#ifdef HTTPS_DEBUG
  Trace(HTTPS_ID, TRACE_SEVERE, "Initializing Dblocks\n");
#endif

  pBuffer = (unsigned char*) HttpsInitDblocks(*lPoolId, pBuffer, &lBufLength,
                                         pBufCfg, iAlignment);

  if (pBuffer == NULL)
  {
    HttpsdataPool[*lPoolId].pDblkLink = NULL;
    HttpsdataPool[*lPoolId].pZdlLink  = NULL;
    return NULL;
  }

  /*---------------------------------------------------------------------*/
  /* Configure the mblks specified in the configuration table            */
  /*---------------------------------------------------------------------*/
#ifdef HTTPS_DEBUG
  Trace(HTTPS_ID, TRACE_SEVERE, "Initializing Mblocks\n");

  Trace(HTTPS_ID, TRACE_SEVERE,
        "HttpsMblkInit : After allocating Dblocks...Bufsize : %d\n",
        lBufLength);
  Trace(HTTPS_ID, TRACE_SEVERE,
        "HttpsMblkInit : Memory requested to alloc mblks : %d\n",
        (lNoMblks * sizeof(HTTPS_mblk_t)));
#endif

  pBuffer = (unsigned char*) HttpsInitMblocks(*lPoolId, pBuffer, &lBufLength,
                                         lNoMblks);
  if (pBuffer == NULL)
  {
    HttpsdataPool[*lPoolId].pDblkLink     = NULL;
    HttpsdataPool[*lPoolId].pZdlLink      = NULL;
    HttpsdataPool[*lPoolId].pMblkFreelist = NULL;
    of_memset(&HttpsdataPool[*lPoolId].MblkStats, 0,
             sizeof(HTTPS_MblkStat_t));
    return NULL;
  }

  if (bDefault)
  {
    T_lMblkDefaultPool = *lPoolId;
  }

  HttpsdataPool[*lPoolId].bInUse = TRUE;
  strcpy(HttpsdataPool[*lPoolId].cPoolName, pPoolName);
  HttpsdataPool[*lPoolId].pFreeFn    = pFreeFn;
  HttpsdataPool[*lPoolId].pFreeFnArg = pFreeFnArg;

  return (pBuffer);
} /* HttpsMblkInit() */

/***************************************************************************
 * Function Name : HttpsMblkCmdInit
 * Description   :
 * Installs the CLI command leaves in the path for the mblk statistics
 * Input         : None
 * Output        : None
 * Return value  : None
 ***************************************************************************/

void HttpsMblkCmdInit( void )
{
  int32_t  ii;

  /**
   * Initialize the data pools and the buffer of mappings from pool names
   * to pool IDs.
   */
  for (ii = 0; ii < HTTPS_MAX_MBLK_POOLS; ii++)
  {
    of_memset(&HttpsdataPool[ii], 0, sizeof(HTTPS_DataPool_t));
  }

  /**
   * Initialize the default Pool.
   */
  T_lMblkDefaultPool = -1;

  return;
} /* HttpsMblkCmdInit() */

/***************************************************************************
 * Function Name : HttpsAllocb
 * Description   : This method allocates a message block of type HTTPS_M_DATA
 *                 and a data block. It allocates a buffer of size equal
 *                 to or greater than the size specified. It uses the
 *                 default pool ID buffers for allocation.
 * Input         : int32_t   -- Size of the data buffer required
 * Output        : None
 * Return value  : HTTPS_mblk_t*   -- pointer to the message block allocated
 ***************************************************************************/

HTTPS_mblk_t *HttpsAllocb( int32_t  lSize )
{
#ifdef HTTPS_DEBUG
  HTTPS_mblk_t  *pDebug;
  uint32_t      mysize = 0;
#endif

  if (T_lMblkDefaultPool == -1)
  {
    return NULL;
  }

#ifdef HTTPS_DEBUG
  pDebug =  HttpsAllocbFromPool(T_lMblkDefaultPool, lSize);

  mysize = (uint32_t)(pDebug->pData->pDbLim - pDebug->pData->pDbBase);

  if ((mysize > 1024) &&
      (mysize <= 2048))
  {
    Trace(HTTPS_ID, TRACE_SEVERE, "\n\r 2k : Alloc mblk:0x%x", pDebug);
  }

  return pDebug;
#else
  return HttpsAllocbFromPool(T_lMblkDefaultPool, lSize);
#endif
} /* HttpsAllocb() */

/***************************************************************************
 * Function Name : HttpsAllocbFromPool
 * Description   : This method allocates a message block of type HTTPS_M_DATA
 *                 and a data block. It allocates a buffer of size equal
 *                 to or greater than the size specified. It uses the
 *                 given pool ID's buffers for allocation.
 * Input         : int32_t   -- Pool ID
 *                 int32_t   -- Size of the data buffer required
 * Output        : None
 * Return value  : HTTPS_mblk_t*   -- pointer to the message block allocated
 ***************************************************************************/

HTTPS_mblk_t *HttpsAllocbFromPool( int32_t  lPoolId,
                                   int32_t  lSize )
{
  register HTTPS_dblk_t  *pdblk;
  register HTTPS_mblk_t  *pmblk = NULL;

  /*---------------------------------------------------------------------*/
  /* First, allocate a data block of the specified size at the           */
  /* specified priority.                                                 */
  /*---------------------------------------------------------------------*/
  if ((pdblk = (HTTPS_dblk_t *)HttpsDbAlloc(lPoolId, lSize)) != NULL)
  {
#ifdef HTTPS_DEBUG
    Trace(HTTPS_ID, TRACE_SEVERE,
          "HttpsDbAlloc returned a valid data block\n");
#endif
    pdblk->ucDbType = HTTPS_M_DATA; /* default as HTTPS_DATA block */
    pdblk->ucDbRef  = 1;      /* set reference count to 1 */
/*
    pdblk->ucDbDebug = 0;
*/
    pdblk->DbFrtn.FreeFunc = NULL;

    /*-----------------------------------------------------------------*/
    /* Allocate a message block.                                       */
    /*-----------------------------------------------------------------*/
    if ((pmblk = (HTTPS_mblk_t *)HttpsMbAlloc(lPoolId)) != NULL)
    {
#ifdef HTTPS_DEBUG
      Trace(HTTPS_ID, TRACE_SEVERE,
            "HttpsMbAlloc returned a valid message block\n");
#endif
      /*-------------------------------------------------------------*/
      /* link the data block to the message block                    */
      /*-------------------------------------------------------------*/
      pmblk->pData = pdblk;
      pmblk->pRptr = pdblk->pDbBase;
      pmblk->pWptr = pdblk->pDbBase;
    }
    else
    {
      /*-------------------------------------------------------------*/
      /* out of message blocks, free the just allocated data block.  */
      /*-------------------------------------------------------------*/
#ifdef HTTPS_DEBUG
      Trace(HTTPS_ID, TRACE_SEVERE, "Out of message blocks\n");
#endif
      pdblk->ucDbRef = 0;
      HttpsDbFree(pdblk);
    }
  }

  return (pmblk);
} /* HttpsAllocbFromPool() */

/***************************************************************************
 * Function Name : HttpsFreeb
 * Description   :
     * Frees a message block and the associated data block if the
     * reference count for the data block is 0. Otherwise, it frees
     * only the message block.
 * Input         : HTTPS_mblk_t*   -- Pointer to the message block to be freed
 * Output        : None
 * Return value  : None
 ***************************************************************************/

void HttpsFreeb( HTTPS_mblk_t  *bp )
{
#ifdef NOSPL
  uint32_t  ulom;
#endif

#ifdef HTTPS_DEBUG
  uint32_t  lSize = 0;

  lSize = (uint32_t)(bp->pData->pDbLim - bp->pData->pDbBase);

  if ((lSize > 1024) &&
      (lSize <= 2048))
  {
    Trace(HTTPS_ID, TRACE_SEVERE, " --  Free mblk:0x%x", bp);
  }
#endif

  if (HttpsdataPool[bp->lPoolId].pFreeFn != NULL)
  {
    HttpsdataPool[bp->lPoolId].pFreeFn(
        HttpsdataPool[bp->lPoolId].pFreeFnArg);
  }
  else
  {
    if (bp->pData)
    {
      /* first, decrement the reference count */
#ifdef NOSPL
      ulom = T_splhigh();
#endif
      bp->pData->ucDbRef--;

      /*------------------------------------------------------------------*/
      /* Free the data block only if the reference count is 0.  We should */
      /* not free a data block that is still pointed at by another message*/
      /* block.                                                           */
      /*------------------------------------------------------------------*/
      if (bp->pData->ucDbRef < 1)
      {
#ifdef NOSPL
        T_splx(ulom);
#endif
        HttpsDbFree(bp->pData);
      }
      else
      {
#ifdef NOSPL
        T_splx(ulom);
#endif
      }
    }

    /*-------------------------------------------------------------------*/
    /* Free the message block.                                           */
    /*-------------------------------------------------------------------*/
    HttpsMbFree(bp);
  }
} /* HttpsFreeb() */

/***************************************************************************
 * Function Name : HttpsFreeMsg
 * Description   :
     * Frees the entire message with all its associated data buffers
 * Input         : HTTPS_mblk_t*    -- Pointer to the message to be freed
 * Output        : None
 * Return value  : None
 ***************************************************************************/

void HttpsFreeMsg( HTTPS_mblk_t  *mp )
{
  register HTTPS_mblk_t  *pmblk;

  /*---------------------------------------------------------------------*/
  /* Traverse through the message and free all the message blocks        */
  /* using the HttpsFreeb utility.                                       */
  /*---------------------------------------------------------------------*/
  while (mp != NULL)
  {
    pmblk = mp->pCont;
    HttpsFreeb(mp);
    mp = pmblk;
  }
} /* HttpsFreeMsg() */

/***************************************************************************
 * Function Name : HttpsInitMblocks
 * Description   :
     * Initializes the mblock headers in a pool. It takes as input the no
     * of mblock headers to be initialized. It returns a pointer to the
     * updated data segment which was passed in as a parameter
 * Input         : int32_t   -- Pool ID
                   unsigned char* -- pointer to the data segment
                   int32_t   -- number of mblock headers to be initialized
 * Output        : int32_t*  -- Buffer Length

 * Return value  : unsigned char* -- pointer to updated data segment
 ***************************************************************************/


unsigned char *HttpsInitMblocks( int32_t   lPoolId,
                            unsigned char  *pBuffer,
                            int32_t   *pBufLength,
                            int32_t   lNoMblks )
{
  HTTPS_mblk_t  *mp;

  HttpsdataPool[lPoolId].pMblkFreelist = (HTTPS_mblk_t*)NULL;
  HttpsdataPool[lPoolId].MblkStats.lMblks = 0;

  /*---------------------------------------------------------------------*/
  /* Initialize the memory blocks in the system                          */
  /*---------------------------------------------------------------------*/
  for (;lNoMblks > 0; lNoMblks--)
  {
    /**
     * If the buffer provided is not sufficient to support the number of
     * mblks requested, return error.
     */
    if (*pBufLength < sizeof(HTTPS_mblk_t))
    {
      return NULL;
    }

    /*
    of_memset(pBuffer, 0, sizeof(HTTPS_mblk_t));
    */
    mp = (HTTPS_mblk_t *) pBuffer;
    mp->lPoolId = lPoolId;

    /**
     * Add to the free list.
     */
    mp->pCont = HttpsdataPool[lPoolId].pMblkFreelist;
    HttpsdataPool[lPoolId].pMblkFreelist = mp;
    pBuffer = pBuffer + sizeof(HTTPS_mblk_t);
    *pBufLength -= sizeof(HTTPS_mblk_t);
    HttpsdataPool[lPoolId].MblkStats.lMblks++;
  }

  HttpsdataPool[lPoolId].MblkStats.lFree = HttpsdataPool[lPoolId].MblkStats.lMblks;
  HttpsdataPool[lPoolId].MblkStats.lWait = 0;
  HttpsdataPool[lPoolId].MblkStats.lDrops = 0;

  return(pBuffer);
} /* HttpsInitMblocks() */

/***************************************************************************
 * Function Name : HttpsInitDblocks
 * Description   :
     * This function initializes the dblock header and data buffer part
     * for each buffer size for a particular pool. It initializes as many
     * buffers for each size as requested in the buffer configuration table
     * passed as the input parameter.
 * Input         : int32_t          -- Pool ID
                   unsigned char*        -- pointer to the data segment
                   HTTPS_MblkBufConfig_t* -- pointer to the buffer config structure
                                       which gives the sizes and the number
                                       of buffers per size
                   int16_t          -- Alignment needed for the data buffer
 * Output        : int32_t*         -- Length of the buffer
 * Return value  : unsigned char* -- pointer to updated data segment
 ***************************************************************************/


unsigned char *HttpsInitDblocks( int32_t                lPoolId,
                            unsigned char               *pBuffer,
                            int32_t                *pBufLength,
                            HTTPS_MblkBufConfig_t  *pBufCfg,
                            int16_t                iAlignment )
{
  int32_t           li = 0, lj = 0;
  int32_t           lsize, iAlignOffset;
  HTTPS_dblk_t      *dp;
  HTTPS_DblkLink_t  *dlp = 0, *dlnext = 0, *dlprev = 0, **dlfree;

  HttpsdataPool[lPoolId].pDblkLink = (HTTPS_DblkLink_t *)NULL;
  dlfree = &HttpsdataPool[lPoolId].pDblkLink;

  /*---------------------------------------------------------------------*/
  /* Initialize the data blocks and buffers. Carve the memory from cp    */
  /* and form the linked list of data block headers and associated       */
  /* buffers. The linked list is organized in the increasing order of    */
  /* buffer size                                                         */
  /*---------------------------------------------------------------------*/
  while (*((int32_t *) pBufCfg))
  {
    dlnext = *dlfree;

    /*-----------------------------------------------------------------*/
    /* Find the place in the linked list where the buffer size fits in */
    /*-----------------------------------------------------------------*/
    while (1)
    {
      if ((dlnext == 0) ||
          (dlnext->lSize > pBufCfg->ulBufSize))
      {
        /**
         * If the buffer provided is not sufficient to hold the dblk return
         * error.
         */
        if (*pBufLength < sizeof(HTTPS_DblkLink_t))
        {
          return NULL;
        }

        dlp = (HTTPS_DblkLink_t *) pBuffer;

        if (dlprev)
        {
          dlprev->pNext = dlp;
        }
        else
        {
          *dlfree = dlp;
        }

        pBuffer += sizeof(HTTPS_DblkLink_t);
        *pBufLength -= sizeof(HTTPS_DblkLink_t);
        dlp->pNext = dlnext;
        dlp->lSize = pBufCfg->ulBufSize;
        dlp->pDblk = 0;
        break;
      }

      if (dlnext->lSize == pBufCfg->ulBufSize)
      {
        dlp = dlnext;
        break;
      }

      dlprev = dlnext;
      dlnext = dlnext->pNext;
    }

    /*-----------------------------------------------------------------*/
    /*   Initialize the link list with the size specified in the       */
    /*  configuration table                                            */
    /*-----------------------------------------------------------------*/
    dp = (HTTPS_dblk_t *) pBuffer;

    /*
    if (dlp->lSize == MLEN)
      mdl_link = dlp;
    */

    if (dlp->lSize == 0)
    {
      HttpsdataPool[lPoolId].pZdlLink = dlp;
      lsize = sizeof(HTTPS_dblk_t);
    }
    else
    {
      lsize = sizeof(HTTPS_dblk_t);
    }

    for (li = 0; li < pBufCfg->ulBuffers; li++)
    {
      /*
      of_memset(pBuffer, 0, sizeof(HTTPS_dblk_t));
      */
      /**
       * If the size of the buffer is less than the size of HTTPS_dblk_t +
       * BufSize + Alignment requirements, return an error
       */
      if (*pBufLength < (lsize + pBufCfg->ulBufSize + iAlignment - 1))
      {
        return NULL;
      }

      dp->lPoolId = lPoolId;
      dp->pDbFreep = dlp->pDblk;
      dlp->pDblk = dp;

      /**
       * Check that the buffer base satisfies the alignment requirement.
       */
      dp->pDbBase = (unsigned char *) dp + lsize;
      iAlignOffset = 0;

      switch (iAlignment)
      {
      case 1 :
        break;
      case 2 :
        if (((int32_t)dp->pDbBase) & 1)
        {
          iAlignOffset = iAlignment - ((int32_t)dp->pDbBase % iAlignment);
          dp->pDbBase += iAlignOffset;
        }
        break;
      case 4 :
        if ((int32_t)(dp->pDbBase) & 3)
        {
          iAlignOffset = iAlignment - ((int32_t)dp->pDbBase % iAlignment);
          dp->pDbBase += iAlignOffset;
        }
        break;
      case 8 :
        if ((int32_t)(dp->pDbBase) & 7)
        {
          iAlignOffset = iAlignment - ((int32_t)dp->pDbBase % iAlignment);
          dp->pDbBase += iAlignOffset;
        }
        break;
      default :
#ifdef HTTPS_DEBUG
        Trace(HTTPS_ID, TRACE_SEVERE,
              "Initializing data blocks...unknown alignment\n");
#endif
        break;
      }

      dp->pDbLim   =  dp->pDbBase + pBufCfg->ulBufSize;
      dp->ucDbRef  = 0;
      dp->ucDbType = 0;

      /*
      dp->ucDbClass = lj;
      */

      pBuffer = (unsigned char*) dp->pDbLim;
      *pBufLength = *pBufLength - (lsize + pBufCfg->ulBufSize + iAlignOffset);
      dp = (HTTPS_dblk_t *) pBuffer;
      dlp->lFree++;
    }
    dlp->lDblks = dlp->lFree;
    dlp->lWait = dlp->lDrops = 0;
    pBufCfg++;
    lj++;
  }

  dlp->pNext = 0;

#ifdef HTTPS_DEBUG
  Trace(HTTPS_ID, TRACE_SEVERE, "********* POOL INFO *********\n");
  dlp = HttpsdataPool[lPoolId].pDblkLink;
  while (dlp)
  {
    Trace(HTTPS_ID, TRACE_SEVERE,
          "Size : %d, Buffers Free : %d\n", dlp->lSize, dlp->lFree);
    dlp = dlp->pNext;
  }
#endif

  return(pBuffer);
} /* HttpsInitDblocks() */

/***************************************************************************
 * Function Name : HttpsDbAlloc
 * Description   :
     * Allocates a data block of a particular size from the default pool
     * of buffers. The data block linked list is searched for the best-fit
     * buffer to allocate. If no data block is found to be free, it returns
     * NULL; otherwise, it returns the pointer to the buffer found.
 * Input         : int32_t    -- Pool ID
                   int32_t    -- size of the data buffer requested
 * Output        : None
 * Return value  : HTTPS_dblk_t*    -- Pointer to the data block allocated
 ***************************************************************************/


HTTPS_dblk_t *HttpsDbAlloc( int32_t  lPoolId,
                            int32_t  lSize )
{
  HTTPS_DblkLink_t  *dlp;
#ifdef NOSPL
  uint32_t          ulos;
#endif
  HTTPS_dblk_t      *dp;

  dlp = HttpsdataPool[lPoolId].pDblkLink;

  ulNumOfHttpsDbAllocs_g++;

  /*---------------------------------------------------------------------*/
  /* Check to see if it is 0 size buffer.                                */
  /*---------------------------------------------------------------------*/
  if ((lSize == 0) &&
      (dlp->lSize))
  {
    return((HTTPS_dblk_t *) 0);
  }

  /*---------------------------------------------------------------------*/
  /* Search for the closest size buffer                                  */
  /*---------------------------------------------------------------------*/
#ifdef HTTPS_DEBUG
  Trace(HTTPS_ID, TRACE_SEVERE, "Searching for nearest size buffer\n");
#endif
#ifdef NOSPL
  ulos = T_splhigh();
#endif

  dp = NULL;

  while (dlp)
  {
    if (lSize <= dlp->lSize)
    {
      /*----------------------------------------------------------------*/
      /* Find to see if a buffer is available                           */
      /*----------------------------------------------------------------*/
      dlp->ulHits++;

      if ((dp = dlp->pDblk) != NULL)
      {
        dlp->pDblk = dp->pDbFreep;
        dp->pDbFreep = (HTTPS_dblk_t *) dlp;
        dlp->lFree--;
        dlp->ulSuccess++;
        break;
      }
      else
      {
        dlp->lDrops++;
        dlp->ulMisses++;
      }
    }
    dlp = dlp->pNext;
  }

#ifdef NOSPL
  T_splx(ulos);
#endif

#ifdef HTTPS_DEBUG
  Trace(HTTPS_ID, TRACE_SEVERE, "HttpsDbAlloc : Free : %d, Drops : %d\n",
        dlp->lFree, dlp->lDrops);
#endif

 return(dp);
} /* HttpsDbAlloc() */

/***************************************************************************
 * Function Name : HttpsDbFree
 * Description   :
     * This function frees the data block and the associated buffer to
     * the pool to which this block belongs. It gets the pool ID from
     * the HTTPS_dblk_t structure.

     * NOTE:
     * The HttpsDbFreep pointer in the HTTPS_dblk_t structure is used to hold the
     * address of the next free block when the block is in the linked
     * list for its size. When it is allocated, it is used to hold the
     * address of the HTTPS_DblkLink_t structure pointer which is the link
     * structure pointer for its size. While freeing, this pointer is
     * recovered and the data block it points to is recovered, to push
     * this block to the head of the list. Thus, there is no need to
     * store the size with the HTTPS_dblk_t structure and search for it.

 * Input         : HTTPS_dblk_t*    -- pointer to the data block
 * Output        : None
 * Return value  : None
 ***************************************************************************/


void HttpsDbFree( HTTPS_dblk_t  *dp )
{
#ifdef NOSPL
  uint32_t          ulos;
#endif
  int32_t           lfrtn = 0;
  HTTPS_DblkLink_t  *dlp;
  HTTPS_frtn_t      fp;


  /*---------------------------------------------------------------------*/
  /* Check to see if external buffer. Call free routine if it is so.     */
  /*---------------------------------------------------------------------*/
  if (dp->DbFrtn.FreeFunc)
  {
#ifdef HTTPS_DEBUG
    Trace(HTTPS_ID, TRACE_SEVERE, "Calling the free function provided\n");
#endif
    lfrtn = 1;
    fp    = dp->DbFrtn;
    dp->DbFrtn.FreeFunc = 0;
  }

  /*---------------------------------------------------------------------*/
  /* Return the data block to its link list                              */
  /*---------------------------------------------------------------------*/
#ifdef NOSPL
  ulos =  T_splhigh();
#endif
  dlp          = (HTTPS_DblkLink_t *) dp->pDbFreep;
  dp->pDbFreep = dlp->pDblk;
  dlp->pDblk   = dp;
  dlp->lFree++;
#ifdef NOSPL
  T_splx(ulos);
#endif
  if (lfrtn)
  {
    (*fp.FreeFunc)(fp.FreeArg);
  }
} /* HttpsDbFree() */

/***************************************************************************
 * Function Name : HttpsMbAlloc
 * Description   :
     * Allocates a message block structure from the pool given as input
 * Input         : int32_t    -- Pool ID
 * Output        : None
 * Return value  : HTTPS_mblk_t*    -- Pointer to the allocated message block
 ***************************************************************************/


HTTPS_mblk_t *HttpsMbAlloc( int32_t  lPoolId )
{
#ifdef NOSPL
  uint32_t      ulos;
#endif
  HTTPS_mblk_t  *mp;

  /*---------------------------------------------------------------------*/
  /* Check to see if a message block is available                        */
  /*---------------------------------------------------------------------*/
#ifdef NOSPL
  ulos = T_splhigh();
#endif

  HttpsdataPool[lPoolId].MblkStats.ulHits++;

  if ((mp = HttpsdataPool[lPoolId].pMblkFreelist))
  {
    HttpsdataPool[lPoolId].pMblkFreelist = mp->pCont;
    mp->pNext = mp->pCont = 0;
/*
    mp->pPrev = 0;
*/
    HttpsdataPool[lPoolId].MblkStats.lFree--;
    HttpsdataPool[lPoolId].MblkStats.ulSuccess++;
  }
  else
  {
    HttpsdataPool[lPoolId].MblkStats.lDrops++;
    HttpsdataPool[lPoolId].MblkStats.ulMisses++;
  }

#ifdef NOSPL
  T_splx(ulos);
#endif

  return(mp);
} /* HttpsMbAlloc() */

/***************************************************************************
 * Function Name : HttpsMbFree
 * Description   :
     * Frees the message block to the pool it came from by extracting
     * the pool ID from the mblock structure
 * Input         : HTTPS_mblk_t*    -- Pointer to the message block to be freed
 * Output        : None
 * Return value  : None
 ***************************************************************************/

void HttpsMbFree( HTTPS_mblk_t  *mp )
{
#ifdef NOSPL
  uint32_t ulos;
#endif

#ifdef NOSPL
  ulos = T_splhigh();
#endif

  mp->pNext = 0;
/*
  mp->pPrev = 0;
*/
  mp->pCont = HttpsdataPool[mp->lPoolId].pMblkFreelist;
  HttpsdataPool[mp->lPoolId].pMblkFreelist = mp;
  HttpsdataPool[mp->lPoolId].MblkStats.lFree++;

#ifdef NOSPL
  T_splx(ulos);
#endif
} /* HttpsMbFree() */

/***************************************************************************
 * Function Name : HttpsMblkPrintStats
 * Description   :
     *  Prints the statistics for a particular pool.
 * Input         : int16_t               --  Pool ID
                   CliSessionContext_t*  --  Session Context Pointer
 * Output        : None
 * Return value  : None
 ***************************************************************************/

void HttpsMblkPrintStats( void )
{
  uint32_t          iPoolId = 0; /* We have only one pool */
  char           buff[100];
  HTTPS_DblkLink_t  *dlp;

  sprintf(buff, "Pool  : %s\n\r", HttpsdataPool[iPoolId].cPoolName);
  printf("%s \r\n",buff);

  sprintf(buff, "Number of HttpsDbAlloc Hits so far %u\r\n",
          ulNumOfHttpsDbAllocs_g);
  printf(" %s \r\n",buff);

  printf("Mblk Statistics\n\r");
  sprintf(buff, "Total : %d\t Free : %d\t Drops : %d\t Hits: %-10u\t Success:%-10u\t Misses:%-10u\n\r",
          HttpsdataPool[iPoolId].MblkStats.lMblks,
          HttpsdataPool[iPoolId].MblkStats.lFree,
          HttpsdataPool[iPoolId].MblkStats.lDrops,
          HttpsdataPool[iPoolId].MblkStats.ulHits,
          HttpsdataPool[iPoolId].MblkStats.ulSuccess,
          HttpsdataPool[iPoolId].MblkStats.ulMisses);
  printf("%s \r\n",buff);

  printf("Dblk Statistics\n\r");

  printf("Size       Total      Free       Drops      Hits       Success    Misses\n\r");
  dlp = HttpsdataPool[iPoolId].pDblkLink;
  while (dlp != NULL)
  {
    sprintf(buff, "%-10d %-10d %-10d %-10d %-10u %-10u %-10u\n\r",
            dlp->lSize, dlp->lDblks, dlp->lFree, dlp->lDrops,
            dlp->ulHits, dlp->ulSuccess, dlp->ulMisses);
    printf( "%s \r\n",buff);
    dlp = dlp->pNext;
  }
} /* HttpsMblkPrintStats() */

#endif /* OF_HTTPD_SUPPORT && OF_HTTPS_SUPPORT */


