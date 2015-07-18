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
/*  File        : htmlpars.h                                              */
/*                                                                        */
/*  Description : This file contains local declarations and definitions   */
/*                related to the HTML parsing engine.                     */
/*                                                                        */
/*  Version      Date      Author      Change Description                 */
/*  -------    --------    ------    ----------------------               */
/*    1.0      00.00.00    kmdhar    Initial implementation               */
/*    1.1      05.05.04    DRAM      Modified after the code review.      */
/**************************************************************************/

#ifndef  __HTMLPARS_H_
#define  __HTMLPARS_H_

/****************************************************
 * * * *  M a c r o    D e f i n i t i o n s  * * * *
 ****************************************************/

#define  HTTPD_TAG_BEGIN_CHR          '<'
#define  HTTPD_TAG_END_CHR            '>'
#define  HTTPD_TAG_SEP_CHR            '_'
#define  HTTPD_TAG_KEY_STR            "IGW"

#define  HTTPD_VAR_BEGIN_CHR          '['
#define  HTTPD_VAR_END_CHR            ']'
#define  HTTPD_VAR_SEP_CHR            '_'
#define  HTTPD_VAR_KEY_STR            "IGW"

#define  HTTPD_TAG_MAX_LEN            (64)

#define  HTTPD_HASH_SIZE              (64)
#define  HTTPD_ENTRIES_PER_TABLE      (600)

/****************************************************
 * * * *  T y p e    D e c l a r a t i o n s  * * * *
 ****************************************************/

/**
 *
 */
typedef int32_t (*HTTPD_HASH_FUNCTION)( char  * );

/**
 *
 */
typedef struct Httpd_HashEntry_s
{
  char                   *pTagName;
  void                    *pCbFunct;
  int32_t                   VarCount;
  Httpd_Variable_t          *pVarArray;
  struct Httpd_HashEntry_s  *pNext;
  unsigned char                   ucPrivilage;
} Httpd_HashEntry_t;

/**
 *
 */
typedef struct Httpd_HashTable_s
{
  HTTPD_HASH_FUNCTION  HashFunct;
  int32_t              HashSize;
  int32_t              MaxEntries;
  int32_t              nCount;
  Httpd_HashEntry_t    *pTable[HTTPD_HASH_SIZE];
} Httpd_HashTable_t;

/**
 * Experimental-Advanced Feature
 * Registration Of Custom Tag Type( like Table, General etc )
 */
/* Don't Dare To Pass HashEntry to the callback function */
typedef struct Httpd_TTEntry_s
{
  char           *pTagName;
  void            *pCbFunct;
  int32_t           VarCount;
  Httpd_Variable_t  *pVarArray;
} Httpd_TTEntry_t;

/**
 *
 */
typedef int32_t (*HTTPD_TAG_TYPE_HANDLER)( HttpRequest      *,
                                           Httpd_TTEntry_t  *,
                                           char          * );

/**
 *
 */
typedef struct Httpd_TagType_s
{
  int32_t                 Value;
  char                 *pBeginKey;
  char                 *pEndKey;
  Httpd_HashTable_t       HTable;
  HTTPD_TAG_TYPE_HANDLER  HandlerFn;
  struct Httpd_TagType_s  *pNext;
} Httpd_TagType_t;

struct FdBuf;

/********************************************************
 * * * *  F u n c t i o n    P r o t o t y p e s  * * * *
 ********************************************************/

int32_t Httpd_TagList_Init( void );

int32_t HttpdSup_Gen_ErrInfo( HttpRequest    *pHttpReq,
                              Httpd_Value_t  *ValArray,
                              uint32_t       uiValCount,
                              void         *pAppData );

int32_t Httpd_SendBinFile( HttpRequest  *pHttpReq );

int32_t Httpd_HRefTag_Process( unsigned char     *pTagName,
                               HttpRequest  *pRequest );

int32_t Httpd_StoreBinFile( HttpRequest  *pHttpReq,
                            uint32_t     ulBufLen,
                            uint32_t     ulDatasz );
int32_t Httpd_PostString_Process( HttpRequest  *pHttpReq );

int32_t Httpd_TagType_GetMax( void );


/**
 * Experimental-Advanced Feature- Generic Way Of Adding New Tag-Type
 *        like Table Tag, General Tag
 *
 * Ex:
 * Httpd_TagType_Register( 1, "TABSTART", "TABEND", MyFunction );
 *
 * For The Above registration, Whenever Server Finds  <IGW_TABSTART_*****>
 * ... <IGW_TABEND_***> combination,  'MyFunction' will be called with
 * appropriate parameters and it is the responsibility of that function to
 * transmit that buffer.
 */


#endif /*  __HTMLPARS_H_ */
