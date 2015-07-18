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
/*  File        : hcmgrmbl.h                                              */
/*                                                                        */
/*  Description : This file contains the definitions for all the buffer   */
/*                management related data and the prototypes for all the  */
/*                functions of the buffer management module.              */
/*                                                                        */
/*  Version      Date      Author      Change Description                 */
/*  -------    --------    ------    ----------------------               */
/*    1.1      05.17.04    DRAM      Modified during the code review.     */
/**************************************************************************/

#ifndef HTTPSMBLKLDEF_H
#define HTTPSMBLKLDEF_H

/****************************************************
 * * * *  M a c r o    D e f i n i t i o n s  * * * *
 ****************************************************/

/**
 * Definition of local constants.
 */
#define  HTTPS_M_DATA                 (1)
#define  HTTPS_MAX_POOL_NAME          (50)

/**
 * The maximum number of pools of data buffers that can be created.
 */
#define  HTTPS_MAX_MBLK_POOLS         (1)

#define  HTTPS_MBLK_ERROR_BASE        (50000)

#define Trace(id, level, format,args...)
/**
 * Macro for determining the minimum of two numbers.
 */
#ifndef min
#define min(x, y) ((x < y) ? x : y)
#endif

enum HTTPS_MbllkError
{
  HTTPS_MBLK_INVALID_INPUT = HTTPS_MBLK_ERROR_BASE + 1,
  HTTPS_MBLK_OUT_OF_MSG_BLKS,
  HTTPS_MBLK_DUPB_FAILED,
  HTTPS_MBLK_DUPMSG_FAILED,
  HTTPS_MBLK_COPYB_FAILED,
  HTTPS_MBLK_COPYMSG_FAILED,
  HTTPS_MBLK_ESBALLOC_FAILED,
  HTTPS_MBLK_PULL_LEN_EXCEEDS_SZ,
  HTTPS_MBLK_NOTHING_TO_UNLINK,
  HTTPS_MBLK_NOTHING_TO_REMOVE
};
typedef enum HTTPS_MblkError  HTTPS_MblkError_t;

/****************************************************
 * * * *  T y p e    D e c l a r a t i o n s  * * * *
 ****************************************************/

/**
 * Structure for the free function and the arguments to it.
 */
struct HTTPS_FreeRtn
{
  void (*FreeFunc)(void*);
  void *FreeArg;
};

typedef struct HTTPS_FreeRtn  HTTPS_frtn_t;

/*-------------------------------------------------------------------*/
/* HTTPS_datab structure definition                                  */
/*-------------------------------------------------------------------*/

struct HTTPS_datab
{
  int32_t             lPoolId;   /* Pool to which data buffer belongs    */
  struct HTTPS_datab  *pDbFreep; /* used internally                      */
  unsigned char*           pDbBase;   /* first byte of buffer                 */
  unsigned char*           pDbLim;    /* last byte+1 of buffer                */
  unsigned char            ucDbRef;   /* count of msgs pointing to this block */
  unsigned char            ucDbType;  /* msg type                             */
  HTTPS_frtn_t        DbFrtn;    /* Pointer to function to free data buf */
};
typedef struct HTTPS_datab  HTTPS_dblk_t;

/*----------------------------------------------------------------------*/
/* T_msgb structure definition                                          */
/*----------------------------------------------------------------------*/

struct HTTPS_msgb
{
  int32_t             lPoolId; /* Pool to which mblk belongs          */
  struct HTTPS_msgb   *pNext;  /* next msg on queue                   */
  struct HTTPS_msgb   *pPrev;  /* previous msg on queue               */
  struct HTTPS_msgb   *pCont;  /* next msg block of msg               */
  unsigned char            *pRptr;  /* first unread data byte in buffer    */
  unsigned char            *pWptr;  /* first unwritten data byte in buffer */
  struct HTTPS_datab  *pData;  /* data block                          */
};
typedef struct HTTPS_msgb HTTPS_mblk_t;

/*----------------------------------------------------------------------*/
/* Definition of the buffer configuration table                         */
/*----------------------------------------------------------------------*/

typedef struct HTTPS_MblkBufConfig
{
  uint32_t  ulBuffers;   /* Number of buffers */
  uint32_t  ulBufSize;   /* Buffer size */
} HTTPS_MblkBufConfig_t;

/*--------------------------------------------------------------------*/
/* Definition for Data block pool header link                         */
/*--------------------------------------------------------------------*/
typedef struct HTTPS_DblkLink
{
  struct HTTPS_DblkLink  *pNext; /* Next data block link         */
  int32_t                lTwait; /* Wait flag for the data block */
  HTTPS_dblk_t           *pDblk; /* Pointer to the data block    */

  /* Statistics of data blocks */

  int32_t   lSize;       /* Buffer size                            */
  int32_t   lDblks;      /* Number of data blocks                  */
  int32_t   lFree;       /* Number of free data blocks             */
  int32_t   lWait;       /* Number of tasks waited for data block  */
  int32_t   lDrops;      /* Number of times failed to get data blk */
  uint32_t  ulHits;
  uint32_t  ulSuccess;
  uint32_t  ulMisses;
} HTTPS_DblkLink_t;

/*----------------------------------------------------------------------*/
/* Message block statistics                                             */
/*----------------------------------------------------------------------*/

typedef struct HTTPS_MblkStat
{
  int32_t   lMblks;      /* Number of  mblks configured        */
  int32_t   lFree;       /* Number of available mblks          */
  int32_t   lWait;       /* Number of times waited for mblk    */
  int32_t   lDrops;      /* Number of times failed to get mblk */
  uint32_t  ulHits;
  uint32_t  ulSuccess;
  uint32_t  ulMisses;
} HTTPS_MblkStat_t;

/*----------------------------------------------------------------------*/
/* Data blocks statistics                                               */
/*----------------------------------------------------------------------*/
typedef struct HTTPS_DbStat
{
  int32_t  lSize;   /* Size of data block                 */
  int32_t  lDblks;  /* Number of data blocks              */
  int32_t  lFree;   /* number of free data blocks         */
  int32_t  lWait;   /* Number of times waited to get dblk */
  int32_t  lDrops;  /* Number of times failed to get dblk */
} HTTPS_DbStat_t;

/**
 * Definition for the data pool structure.
 */
typedef struct HTTPS_DataPool
{
  char           cPoolName[HTTPS_MAX_POOL_NAME];
  bool           bInUse;
  HTTPS_DblkLink_t  *pDblkLink;
  HTTPS_DblkLink_t  *pZdlLink;
  HTTPS_mblk_t      *pMblkFreelist;
  HTTPS_MblkStat_t  MblkStats;
  void(*pFreeFn)  (void *);
  void*           pFreeFnArg;
} HTTPS_DataPool_t;

/*----------------------------------------------------------------------*/
/* Data block structure for esballoc call. It need to store all the     */
/* data blocks allocated thru' esballoc in a linked to be freed at      */
/* pSOS restart time. Esballoc can be used by the driver attach a data  */
/* buffer. This can be done before pSOS is initialized                  */
/*----------------------------------------------------------------------*/
struct HTTPS_esbdatab
{
  HTTPS_dblk_t           EsbDblk;  /* Data block */
  struct HTTPS_esbdatab  *pNext;   /* Next link */
  struct HTTPS_esbdatab  *pPrev;   /* Previous link */
};
typedef struct HTTPS_esbdatab  HTTPS_EsbDblk_t;

/**
 *  Structure to map pool names to pool IDs
typedef struct HTTPS_PoolHash
{
  int16_t PoolId;
  char PoolName[50];
} HTTPS_PoolHash_t;
 */

/********************************************************
 * * * *  F u n c t i o n    P r o t o t y p e s  * * * *
 ********************************************************/

/**
 * This function is used to initialize the mblocks.
 */
void HttpsInitMblks( void );

/**
 * This function is used to print the statistics of a pool to the console.
 */
void HttpsMblkPrintStats( void );



/**
 * This function is used allocate a data buffer of given size from the
 * default pool.
 */
HTTPS_mblk_t *HttpsAllocb( int32_t  lSize );

/**
 * This function is used to allocate data buffers of given size from the
 * given pool.
 */
HTTPS_mblk_t *HttpsAllocbFromPool( int32_t  lPoolId,
                                   int32_t  lSize );

/**
 * This function is used allocate message and data block headers from the
 * default pool and link them to the user allocated data buffer passed as
 * input. Registers the user specified free function, if any.
 */
HTTPS_mblk_t *HttpsEsbAlloc( unsigned char      *pBase,
                             int32_t       lSize,
                             HTTPS_frtn_t  *pFrtn );

/**
 * This function is used allocate message and data block headers from the
 * specified pool and link them to the user allocated data buffer passed as
 * input. Registers the user specified free function, if any.
 */
HTTPS_mblk_t *HttpsEsbAllocFromPool( int32_t       lPoolId,
                                     unsigned char      *pBase,
                                     int32_t       lSize,
                                     HTTPS_frtn_t  *pFrtn );

/**
 * This function is used to free the given message block and the data
 * block headers.
 */
void HttpsFreeb( HTTPS_mblk_t  *bp );

/**
 * This function is used to free the entire message given by the following
 * link through the pCont pointer and using Freeb function to free each
 * block.
 */
void HttpsFreeMsg( HTTPS_mblk_t  *mp );

#endif

