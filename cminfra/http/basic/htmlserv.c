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
/*  File        : htmlserv.c                                              */
/*                                                                        */
/*  Description : This file contains the parsing utilities and hash tables*/
/*                of callback registrations.                              */
/*                                                                        */
/*  Version      Date      Author      Change Description                 */
/*  -------    --------    ------    ----------------------               */
/*    1.0      00.00.00    kmdhar    Initial implementation               */
/*    1.1      05.10.04    DRAM      Modified during the code review.     */
/*    1.2      22.02.06    Harishp   Modified during MultipleInstances	  */	
/*                                   and Multithreading	                  */
/**************************************************************************/

#ifdef OF_HTTPD_SUPPORT

/***********************************************************
 * * * *  I n c l u d e    H e a d e r    f i l e s  * * * *
 ***********************************************************/

#include "cmincld.h"
#include "httpdinc.h"
#include "httpwrp.h"
#include "httpgdef.h"
#include "httpgif.h"
#include "htpdglb.h"
#include "httpdtmr.h"
#include "httpd.h"
#include "htmlpars.h"

/****************************************************
 * * * *  M a c r o    D e f i n i t i o n s  * * * *
 ****************************************************/

#define  HTTPD_LOCAL_NAME_LEN         (65)
#define  HTTPD_LOCAL_BIND_STRING_LEN  (80)
#define HTTP_COMPLETE_IF_TAG_LEN        (80+1)

extern char  aPrevXmlUrlData[HTTP_FILENAME_LEN + 1];

/************************************************************
 * * * *  V a r i a b l e    D e c l a r a t i o n s  * * * *
 ************************************************************/

/********************************************************
 * * * *  F u n c t i o n    P r o t o t y p e s  * * * *
 ********************************************************/

 int32_t HexToInt( unsigned char  *HexString,
                  unsigned char  *pOutput );

 int32_t Httpd_ValAry_FromBinFileData( unsigned char          *pMpBuf,
                                      int32_t           lMpBytes,
                                      unsigned char          *pBoundary,
                                      Httpd_Variable_t  *pVarAry,
                                      uint32_t          VarCount,
                                      Httpd_Value_t     *pValAry );

 Httpd_Value_t *Httpd_ValAry_Alloc( Httpd_Variable_t  *pVarArray,
                                   uint32_t          nCount,
                                   bool           bNoVals );
 int32_t Httpd_ValAry_Free( Httpd_Value_t  *pVarArray );

 int32_t Httpd_ValAry_FromPostString( char           *pPostString,
                                     Httpd_Variable_t  *pVarAry,
                                     uint32_t          VarCount,
                                     Httpd_Value_t     *pValAry );

 int32_t Httpd_ValAry_FromMultiPart( unsigned char          *pMpBuf,
                                    int32_t           lMpBytes,
                                    unsigned char          *pBoundary,
                                    Httpd_Variable_t  *pVarAry,
                                    uint32_t          VarCount,
                                    Httpd_Value_t     *pValAry );

 char *Httpd_Variable_FromBuffer( char   *pBuf,
                                    uint32_t  uiLength,
                                    char   *cVariable,
                                    uint32_t  VarLen,
                                    char   **cVarStart );

 Httpd_TagType_t *Httpd_TagType_SearchByValue( int32_t  Value );

 Httpd_TagType_t *Httpd_TagType_SearchByBeginKey( char  *pBeginKey );

 char *Httpd_TagRegion_Capture( char          *pBuf,
                                  char          *cTagName,
                                  uint32_t         TagLen,
                                  Httpd_TagType_t  **pTagType,
                                  int32_t          *PreBufLen,
                                  int32_t          *TagBufLen,
                                  char          **TagBufStart );

#ifdef HTTPD_DEBUG
 void Httpd_HashTable_Dump( Httpd_HashTable_t  *pThis );
#endif /* HTTPD_DEBUG */

 void *Httpd_MemMem( void   *pSrc,
                                 int32_t  lSrcBytes,
                                 void   *pDst,
                                 int32_t  lDstBytes );

 Httpd_HashEntry_t *Httpd_Search_Tag( int32_t  Value,
                                     char  *pTagName );

 unsigned char *Httpd_MPart_GetSubmitName( unsigned char  *pInLine,
                                                unsigned char  *name_p,
                                                uint32_t  ulLen );


 Httpd_HashEntry_t *Httpd_HashEntry_New( HttpTagArgs_t TagArgs);

 int32_t Httpd_HashTable_DeleteEntry( Httpd_HashTable_t  *pHTable,
                                     const char      *pTagName );
 int32_t Httpd_HashTable_AddEntry( Httpd_HashTable_t  *pHTable,
                                  Httpd_HashEntry_t  *pThis );

 int32_t Httpd_HashTable_AddEntryEx( Httpd_HashTable_t  *pHTable,
                                               HttpTagArgs_t       TagArgs);

 int32_t Httpd_HashTable_DeleteEntryEx( Httpd_HashTable_t  *pHTable,
                                       const char      *pTagName );

 Httpd_HashEntry_t *Httpd_Search_ActionTag_New( char *pTagName );


 Httpd_HashEntry_t *Httpd_Search_FileStoreTag( char  *pTagName );

 Httpd_HashEntry_t *Httpd_Search_FileSendTag( char  *pTagName );

 Httpd_HashEntry_t *Httpd_HashTable_Search( Httpd_HashTable_t  *pHTable,
                                           char            *pTagName );

 Httpd_HashEntry_t *Httpd_Search_GenTag( char  *pTagName );

 Httpd_HashEntry_t *Httpd_Search_ActionTag( char  *pTagName );

 int32_t Httpd_TagType_Handler_Table( HttpRequest      *pRequest,
                                     Httpd_TTEntry_t  *pTTEntry,
                                     char          *pBuf );

 int32_t Httpd_TagType_Register( int32_t                 Value,
                                           char                 *pBeginKey,
                                           char                 *pEndKey,
                                           HTTPD_TAG_TYPE_HANDLER  HandlerFn );


 int32_t Httpd_HashFunction( char *name_p );

 int32_t Httpd_HashTable_Init( Httpd_HashTable_t  *pHTable );

 int32_t Httpd_TagList_Add( Httpd_TagType_t  *pThis );

 Httpd_TagType_t *Httpd_TagType_Alloc( int32_t                 Value,
                                      char                 *pBeginKey,
                                      char                 *pEndKey,
                                      HTTPD_TAG_TYPE_HANDLER  HandlerFn );

 int32_t Httpd_TagType_Free( Httpd_TagType_t  *pThis );

#if 0
 Httpd_HashEntry_t *Httpd_Search_TableTag( char  *pTagName );
#endif /* 0 */



int32_t Httpd_TagType_Handler_General( HttpRequest      *pRequest,
                                       Httpd_TTEntry_t  *pTTEntry,
                                       char          *pBuf );
 
int32_t Httpd_PostString_Convert( unsigned char  *cInString );


int32_t Httpd_If_Process( HttpRequest  *pRequest, unsigned char	*pHtmlBuf);

/****************************************************************
 * * * *  F u n c t i o n    I m p l e m e n t a t i o n  * * * *
 ****************************************************************/

/**************************************************************************
 * Function Name : Httpd_TagType_SearchByValue                            *
 * Description   : Search in the registered tag types using the value     *
 * Input         : Value - value of the tag type                          *
 * Output        :                                                        *
 * Return value  : pointer to the tag type if found                       *
 *                 NULL if not found                                      *
 **************************************************************************/

 Httpd_TagType_t *Httpd_TagType_SearchByValue( int32_t  Value )
{
  HttpGlobals_t  *pHttpg=NULL;
  Httpd_TagType_t  *pTemp=NULL;

  pHttpg=pHTTPG;  
  if(!pHttpg)
  {
    Httpd_Error("pHttpg is NULL\r\n");
    return NULL;
  }
  pTemp = pHttpg->pHttpdTagList;

  while (pTemp)
  {
    if (pTemp->Value == Value)
    {
      return pTemp;
    }
    pTemp = pTemp->pNext;
  }
  return NULL;
} /* Httpd_TagType_SearchByValue() */

/**************************************************************************
 * Function Name : Httpd_TagType_SearchByBeginKey                         *
 * Description   : Search in the registered tag types using begin key     *
 * Input         : pBeginKey - begin key of the tag type                  *
 * Output        :                                                        *
 * Return value  : OF_SUCCESS on success                                   *
 *                 OF_FAILURE on failure                                   *
 **************************************************************************/

 Httpd_TagType_t *Httpd_TagType_SearchByBeginKey( char  *pBeginKey )
{
 HttpGlobals_t *pHttpg=NULL;
 Httpd_TagType_t  *pTemp=NULL;
 
 pHttpg=pHTTPG; 
 if(!pHttpg)
 {
    Httpd_Error("pHttpg is NULL\r\n");
    return NULL;
 }
 pTemp = pHttpg->pHttpdTagList;

  while (pTemp)
  {
    if (!of_strcmp(pTemp->pBeginKey, pBeginKey))
    {
      return pTemp;
    }
    pTemp = pTemp->pNext;
  }
  return NULL;
} /* Httpd_TagType_SearchByBeginKey() */

/**************************************************************************
 * Function Name : Httpd_HashEntry_New                                    *
 * Description   : Allocates a new hash entry and initializes with the    *
 *                 arguments supplied.                                    *
 * Input         : pTagName - Name of the tag. ex: login.igw, LOGIN       *
 *                 pCbFunct - Callback function for this tag              *
 *                 VarCount - Count of variables present within           *
 *                            the tag                                     *
 *                 pVarArray- Array of variables(name, size)              *
 * Output        :                                                        *
 * Return value  : Pointer to newly allocated hash entry on success       *
 *                 NULL on failure                                        *
 **************************************************************************/

 Httpd_HashEntry_t *Httpd_HashEntry_New( HttpTagArgs_t TagArgs)
{
  Httpd_HashEntry_t  *pTemp=NULL;

  HTTPD_ASSERT(!TagArgs.pTagName);
  HTTPD_ASSERT(!TagArgs.pCbFunct);

  pTemp = of_malloc(sizeof(Httpd_HashEntry_t));

  if (!pTemp)
  {
    return NULL;
  }

  pTemp->pTagName = of_strdup((char *)TagArgs.pTagName);

  if (!pTemp->pTagName)
  {
    of_free(pTemp);
    return NULL;
  }

  pTemp->pCbFunct  = TagArgs.pCbFunct;
  pTemp->VarCount  = TagArgs.VarCount;
  pTemp->pVarArray = TagArgs.pVarArray;
  pTemp->ucPrivilage = TagArgs.ucPrivilage;
  pTemp->pNext     = NULL;

  return pTemp;
} /* Httpd_HashEntry_New() */

/**************************************************************************
 * Function Name : Httpd_HashTable_AddEntry                               *
 * Description   : Adds a hash entry to the hash table and rejects any    *
 *                 duplicate tag name.                                    *
 * Input         : pHTable - Pointer to the hash table                    *
 *                 pThis   - Pointer to the hash entry                    *
 * Output        :                                                        *
 * Return value  : OF_SUCCESS on success, OF_FAILURE on failure             *
 **************************************************************************/

 int32_t Httpd_HashTable_AddEntry( Httpd_HashTable_t  *pHTable,
                                  Httpd_HashEntry_t  *pThis )
{
  int32_t            HashVal=0;
  Httpd_HashEntry_t  *pTemp=NULL;

  HTTPD_ASSERT(!pHTable); /* As this is local function, we believe args */
  HTTPD_ASSERT(!pThis);
  HTTPD_ASSERT(!pThis->pTagName);

  if (pHTable->nCount+1 > pHTable->MaxEntries)
  {
    Httpd_Error("Reached Maximum Entries. Rejecting...\n");
    return OF_FAILURE;
  }

  HashVal  =  pHTable->HashFunct((char*) pThis->pTagName);

  /* Check For Duplicate */
  pTemp = pHTable->pTable[HashVal];

  while (pTemp)
  {
    if (!of_strcmp(pTemp->pTagName, pThis->pTagName))
    {
#ifdef HTTPD_DEBUG
      Httpd_Error("Duplicate Tag Found! Rejecting..\n");
      Httpd_Print("Tag '%s' is Already Been Registered To Somebody\n",
                  pThis->pTagName);
#endif /* HTTPD_DEBUG */
      return OF_FAILURE;
    }
    pTemp = pTemp->pNext;
  }

  pThis->pNext             = pHTable->pTable[HashVal];
  pHTable->pTable[HashVal] = pThis;
  pHTable->nCount++;

  return OF_SUCCESS;
} /* Httpd_HashTable_AddEntry() */

/************************************************************************
 * Function Name : Httpd_HashTable_DeleteEntry                          *
 * Description   : Adds a hash entry to the hash table                  *
 *                 Rejects any duplicate tag name                       *
 * Input         : pHTable - Pointer to the hash table                  *
 *                 pThis   - Pointer to the hash entry                  *
 * Output        :                                                      *
 * Return value  : OF_SUCCESS on success                                 *
 *                 OF_FAILURE on failure                                 *
 ************************************************************************/

 int32_t Httpd_HashTable_DeleteEntry( Httpd_HashTable_t  *pHTable,
                                     const char      *pTagName )
{
  int32_t            HashVal=0;
  Httpd_HashEntry_t  *pTemp = NULL;
  Httpd_HashEntry_t  *pPrev = NULL;
  Httpd_HashEntry_t  *pNext = NULL;

  HTTPD_ASSERT(!pHTable);  /* As this is local function, we believe args*/
  HTTPD_ASSERT(!pTagName);

  HashVal  =  pHTable->HashFunct((char*)pTagName);

  /* Check For Duplicate */
  pTemp = pHTable->pTable[HashVal];
  pPrev = NULL;

  while (pTemp)
  {
    if (!of_strcmp(pTemp->pTagName, pTagName))
    {
      if (pPrev == NULL)
      {
	pNext = pTemp->pNext;      
        of_free(pTemp);
        pHTable->pTable[HashVal] = pNext;
      }
      else
      {
        pPrev->pNext = pTemp->pNext;
        of_free(pTemp);
      }
      pHTable->nCount--;

      return OF_SUCCESS;
    }

    pPrev = pTemp;
    pTemp = pTemp->pNext;
  }

  /* Tag not found */
  return OF_FAILURE;
} /* Httpd_HashTable_DeleteEntry() */

/**************************************************************************
 * Function Name : Httpd_HashTable_AddEntryEx                             *
 * Description   : Same as above function except it takes raw args        *
 * Input         : pHTable  - Pointer to the hash table                   *
 *                 pTagName - Name of the Tag                             *
 *                 pCbFunct - Callback function for this tag              *
 *                 VarCount - Count of variables within the tag           *
 *                 pVarArray- Array of variables                          *
 * Output        :                                                        *
 * Return value  : OF_SUCCESS on success, OF_FAILURE on failure             *
 **************************************************************************/

 int32_t Httpd_HashTable_AddEntryEx( Httpd_HashTable_t  *pHTable,
                                               HttpTagArgs_t       TagArgs)
{
  Httpd_HashEntry_t  *pTemp=NULL;

  HTTPD_ASSERT(!pHTable);
  HTTPD_ASSERT(!TagArgs.pTagName);
  HTTPD_ASSERT(!TagArgs.pCbFunct);

  pTemp = Httpd_HashEntry_New(TagArgs);

  if (!pTemp)
  {
    Httpd_Error("Unable To Allocate Memory!\n");
    return OF_FAILURE;
  }

  return Httpd_HashTable_AddEntry(pHTable, pTemp);
} /* Httpd_HashTable_AddEntryEx() */

/***********************************************************************
* Function Name : Httpd_HashTable_DeleteEntryEx                        *
* Description   : delete the registered                                *
* Input         : pHTable  - Pointer to the hash table                 *
*                 pTagName - Name of the Tag                           *
*                 pCbFunct - Callback function for this tag            *
*                 VarCount - Count of variables within the tag         *
*                 pVarArray- Array of variables                        *
* Output        :                                                      *
* Return value  : OF_SUCCESS on success                                 *
*                 OF_FAILURE on failure                                 *
************************************************************************/

 int32_t Httpd_HashTable_DeleteEntryEx( Httpd_HashTable_t  *pHTable,
                                       const char      *pTagName )
{
  return Httpd_HashTable_DeleteEntry(pHTable, pTagName);
} /* Httpd_HashTable_DeleteEntryEx() */

/***********************************************************************
 * Function Name : Httpd_HashTable_Search                              *
 * Description   : Searches the hash table with the given tag name     *
 * Input         : pHTable  - Pointer to the hash table                *
 *                 pTagName - Name of the tag                          *
 * Output        :                                                     *
 * Return value  : Pointer to the hash entry if match found            *
 *                 NULL is match is not found                          *
 ***********************************************************************/

 Httpd_HashEntry_t *Httpd_HashTable_Search( Httpd_HashTable_t  *pHTable,
                                           char            *pTagName )
{
  int32_t            HashVal=0;
  Httpd_HashEntry_t  *pTemp=NULL;

  HTTPD_ASSERT(!pHTable);
  HTTPD_ASSERT(!pTagName);

  HashVal = pHTable->HashFunct((char*)pTagName);
  pTemp   = pHTable->pTable[HashVal];

  while (pTemp)
  {
    if (!of_strcmp(pTemp->pTagName, pTagName))
    {
      return pTemp;
    }

    pTemp = pTemp->pNext;
  }
  return NULL;
} /* Httpd_HashTable_Search() */

/***********************************************************************
 * Function Name : Httpd_Search_Tag                                    *
 * Description   : Searches the hash table of the tag type for tag name *
 * Input         : Value  - Value of tag type whose hash table is used  *
 *                          for searching the given tag                *
 *                 pTagName - Name of the tag                          *
 * Output        :                                                     *
 * Return value  : Pointer to the hash entry if match found            *
 *                 NULL is match is not found                          *
 ***********************************************************************/

 Httpd_HashEntry_t *Httpd_Search_Tag( int32_t  Value,
                                     char  *pTagName )
{
  Httpd_TagType_t  *pThis=NULL;

  pThis = Httpd_TagType_SearchByValue(Value);

  if (!pThis)
  {
    return NULL;
  }

  return  Httpd_HashTable_Search(&pThis->HTable, pTagName);
} /* Httpd_Search_Tag() */

/***********************************************************************
 * Function Name : Httpd_Search_GenTag                                 *
 * Description   : Searches for the given general tag name in the       *
 *                 general tag type's hash table                        *
 * Input         : pTagName - Name of the general tag                  *
 * Output        :                                                     *
 * Return value  : Pointer to the hash entry if match found            *
 *                 NULL is match is not found                          *
 ***********************************************************************/

 Httpd_HashEntry_t *Httpd_Search_GenTag( char  *pTagName )
{
  return Httpd_Search_Tag(HTTPD_TAG_TYPE_GENERAL, pTagName);
} /* Httpd_Search_GenTag() */

/***********************************************************************
 * Function Name : Httpd_Search_ActionTag                              *
 * Description   : Searches for the given action tag name in the       *
 *                 action tag type's hash table                         *
 * Input         : pTagName - Name of the action tag                   *
 * Output        :                                                     *
 * Return value  : Pointer to the hash entry if match found            *
 *                 NULL is match is not found                          *
 ***********************************************************************/

 Httpd_HashEntry_t *Httpd_Search_ActionTag( char  *pTagName )
{
  return Httpd_Search_Tag(HTTPD_TAG_TYPE_ACTION, pTagName);
} /* Httpd_Search_ActionTag() */

/***********************************************************************
* Function Name : Httpd_Search_ActionTag_New                           *
* Description   : Searches for the given action tag name in the        *
*                 New action tag type's hash table                      *
* Input         : pTagName - Name of the action tag                    *
* Output        :                                                      *
* Return value  : Pointer to the hash entry if match found             *
*                 NULL is match is not found                           *
************************************************************************/

 Httpd_HashEntry_t *Httpd_Search_ActionTag_New( char  *pTagName )
{
  return Httpd_Search_Tag(HTTPD_TAG_TYPE_ACTION_NEW, pTagName);
} /* Httpd_Search_ActionTag_New() */

/***********************************************************************
 * Function Name : Httpd_Search_FileStoreTag                           *
 * Description   : Searches for the given action(for image upload)     *
 *                 tag name in the                       *
 *                 action tag type's hash table                         *
 * Input         : pTagName - Name of the action tag                   *
 * Output        :                                                     *
 * Return value  : Pointer to the hash entry if match found            *
 *                 NULL is match is not found                          *
 ***********************************************************************/

 Httpd_HashEntry_t *Httpd_Search_FileStoreTag( char  *pTagName )
{
  return Httpd_Search_Tag(HTTPD_TAG_TYPE_FILE_STORE, pTagName);
} /* Httpd_Search_FileStoreTag() */

/***********************************************************************
 * Function Name : Httpd_Search_FileSendTag                           *
 * Description   : Searches for the given action(for image upload)     *
 *                 tag name in the                       *
 *                 action tag type's hash table                         *
 * Input         : pTagName - Name of the action tag                   *
 * Output        :                                                     *
 * Return value  : Pointer to the hash entry if match found            *
 *                 NULL is match is not found                          *
 ***********************************************************************/

 Httpd_HashEntry_t *Httpd_Search_FileSendTag( char *pTagName )
{
  return Httpd_Search_Tag(HTTPD_TAG_TYPE_FILE_SEND, pTagName);
} /* Httpd_Search_FileSendTag() */
/***********************************************************************
 * Function Name : Httpd_Register_IfTag                               *
 * Description   : Registers a If  tag to the HTML parse engine    *
 * Input         : pTagName - Name of the general tag                  *
 *                 Callback-Function, Variable-Count, Variable Array   *
 * Output        :                                                     *
 * Return value  : OF_SUCCESS on success                                *
 *                 OF_FAILURE on failure                                *
 ***********************************************************************/

int32_t Httpd_Register_IfTag( const char     *pTagName,
                               void            *pCbFunct,
                               int32_t           VarCount,
                               Httpd_Variable_t  *pVarArray )
{
  HttpTagArgs_t TagArgs = {};
  TagArgs.pTagName = of_strdup((char *)pTagName);
  if(!TagArgs.pTagName)
  {
      HttpDebug((" TagArgs.pTagName is NULL \r\n"));
      return OF_FAILURE;
  }
  TagArgs.pCbFunct = pCbFunct;
  TagArgs.VarCount =  1  ;                                /*VarCount Should be 1*/
  TagArgs.pVarArray = pVarArray;		
  if(Httpd_Register_Tag(HTTPD_TAG_TYPE_IF,TagArgs)!=OF_SUCCESS)
  {
       of_free(TagArgs.pTagName);
       HttpDebug((" Httpd_Register_Tag is failed \r\n"));
       return OF_FAILURE;
  }
  of_free(TagArgs.pTagName);  
  return OF_SUCCESS;
} /* Httpd_Register_IfTag() */
/***********************************************************************
 * Function Name : Httpd_Register_GenTag                               *
 * Description   : Registers a general tag to the HTML parse engine    *
 * Input         : pTagName - Name of the general tag                  *
 *                 Callback-Function, Variable-Count, Variable Array   *
 * Output        :                                                     *
 * Return value  : OF_SUCCESS on success                                *
 *                 OF_FAILURE on failure                                *
 ***********************************************************************/

int32_t Httpd_Register_GenTag( const char     *pTagName,
                               void            *pCbFunct,
                               int32_t           VarCount,
                               Httpd_Variable_t  *pVarArray )
{
  HttpTagArgs_t TagArgs = {};
  TagArgs.pTagName = of_strdup((char *)pTagName);
  if(!TagArgs.pTagName)
  {
      HttpDebug((" TagArgs.pTagName is NULL \r\n"));
      return OF_FAILURE;
  }
  
  TagArgs.pCbFunct = pCbFunct;
  TagArgs.VarCount = VarCount;
  TagArgs.pVarArray = pVarArray;
  if(Httpd_Register_Tag(HTTPD_TAG_TYPE_GENERAL,TagArgs)!=OF_SUCCESS)
  {
      of_free(TagArgs.pTagName);
      HttpDebug((" Httpd_Register_Tag is failed \r\n"));
      return OF_FAILURE;
  }
  of_free(TagArgs.pTagName);
  return OF_SUCCESS;
} /* Httpd_Register_GenTag() */

/**************************************************************************
 * Function Name : Httpd_TagList_Init                                     *
 * Description   : Registers default tag types supported by server        *
 *                 general, table, action tag types                       *
 * Input         :                                                        *
 * Output        : None                                                   *
 * Return value  : OF_SUCCESS on success                                   *
 *                 OF_FAILURE on failure                                   *
 **************************************************************************/

int32_t Httpd_TagList_Init( void )
{
  /* Register GeneralTag, TableTag  Handlers*/
  Httpd_TagType_Register(HTTPD_TAG_TYPE_GENERAL, "START", "END",
                         Httpd_TagType_Handler_General);
  Httpd_TagType_Register(HTTPD_TAG_TYPE_TABLE, "TABSTART", "TABEND",
                         Httpd_TagType_Handler_Table);

  /**
   * This is different from above registrations.
   * This For Post .igw tags. (Action Tags)
   */
  Httpd_TagType_Register(HTTPD_TAG_TYPE_ACTION, "", "", NULL);
  /*HTTP2.0*/

  Httpd_TagType_Register(HTTPD_TAG_TYPE_FILE_STORE, "", "", NULL);
  Httpd_TagType_Register(HTTPD_TAG_TYPE_FILE_SEND, "", "", NULL);
  Httpd_TagType_Register(HTTPD_TAG_TYPE_ACTION_NEW, "", "", NULL);
 Httpd_TagType_Register(HTTPD_TAG_TYPE_IF, "STARTIF", "ENDIF", NULL);
  return OF_SUCCESS;
} /* Httpd_TagList_Init() */

/**************************************************************************
 * Function Name : Httpd_HashFunction                                     *
 * Description   : Default hash function for hash tables. The Xors all the*
 *                 bytes in the string and returns a masked value (0 -    *
 *                 HTTPD_HASH_SIZE) HTTPD_HASH_SIZE must be less than 255 *
 * Input         : name_p - string to hash                                 *
 * Output        :                                                        *
 * Return value  : Hash Value on success                                  *
 *                 255 on failure as default                              *
 **************************************************************************/

 int32_t  Httpd_HashFunction( char  *name_p )
{
  unsigned char  RetVal=0;
  HTTPD_ASSERT(!name_p);

  if (HTTPD_HASH_SIZE > 255)
  {
#ifdef HTTPD_DEBUG
    Httpd_Print("Hash Size > 255. Re write the default hash function\n");
#endif /* HTTPD_DEBUG */
    return 255;
  }
  RetVal = *name_p++;

  while (*name_p)
  {
    RetVal ^= *name_p++;
  }

  return (int32_t) (RetVal & (HTTPD_HASH_SIZE-1));
} /* Httpd_HashFunction() */

/**************************************************************************
 * Function Name : Httpd_HashTable_Init                                   *
 * Description   : Initializes the hash table with default values.        *
 * Input         : pHTable - pointer to the hash table                    *
 * Output        :                                                        *
 * Return value  : OF_SUCCESS on success                                   *
 *                 OF_FAILURE on failure                                   *
 **************************************************************************/

 int32_t Httpd_HashTable_Init( Httpd_HashTable_t  *pHTable )
{
  int32_t  iTemp=0;

  HTTPD_ASSERT(!pHTable);

  pHTable->HashFunct  = Httpd_HashFunction;
  pHTable->HashSize   = HTTPD_HASH_SIZE;
  pHTable->MaxEntries = HTTPD_ENTRIES_PER_TABLE;
  pHTable->nCount     = 0;

  for (iTemp = 0; iTemp < pHTable->HashSize; iTemp++)
  {
    pHTable->pTable[iTemp] = NULL;
  }

  return OF_SUCCESS;
} /* Httpd_HashTable_Init() */

/**************************************************************************
 * Function Name : Httpd_TagList_Add                                      *
 * Description   : This adds the given tag type to the list rejects       *
 *                 duplicate value and duplicate begin key.                *
 * Input         : pThis   pointer to the TagType structure               *
 * Output        : None                                                   *
 * Return value  : OF_SUCCESS on successful addition                       *
 *                 OF_FAILURE on failure.                                  *
 **************************************************************************/

 int32_t Httpd_TagList_Add( Httpd_TagType_t  *pThis )
{
  Httpd_TagType_t  *pTemp=NULL;
  HttpGlobals_t   *pHttpg=NULL;
  
  HTTPD_ASSERT(!pThis);
  
  pHttpg=pHTTPG;
  if(!pHttpg)
 {
    Httpd_Error("pHttpg is NULL\r\n");
    return OF_FAILURE;
 }
  /**
   * Find For Duplicate Value.
   */
  pTemp = Httpd_TagType_SearchByValue(pThis->Value);

  if (pTemp)
  {
#ifdef HTTPD_DEBUG
    Httpd_Error("Duplicate TagType Value Found While Registering\n");
    Httpd_Print("That Was Registered For %s\n", pTemp->pBeginKey);
#endif /* HTTPD_DEBUG */
    return OF_FAILURE;
  }

  /**
   * Find For Duplicate pBeginKey.
   */
  /**
   * Added If the pBeginKey is NULL also they are not adding.
   */
  if (of_strcmp(pThis->pBeginKey, "") != 0)
  {
    pTemp = Httpd_TagType_SearchByBeginKey(pThis->pBeginKey);

    if (pTemp)
    {
#ifdef HTTPD_DEBUG
      Httpd_Error("Duplicate BeginKey Found While Registering\n");
      Httpd_Print("The Begin-Key '%s' is Already Registered For Someone else\n",
                  pTemp->pBeginKey);
#endif /* HTTPD_DEBUG */
      return OF_FAILURE;
    }
  }

  /**
   * So, This is Not a Duplicate Entry, Insert At Head.
   */
  pThis->pNext  = pHttpg->pHttpdTagList;
  pHttpg->pHttpdTagList = pThis;

  return OF_SUCCESS;
} /* Httpd_TagList_Add() */

/**************************************************************************
 * Function Name : Httpd_TagType_Alloc                                    *
 * Description   : This allocates a tag type and initializes with the     *
 *                 given arguments.                                       *
 * Input         : Value       unique value to the tag type.              *
 *                 pBeginKey   Begin Key Of Tag Type. Ex: START           *
 *                 pEndKey     End   Key Of Tag Type. Ex: END             *
 *                 HandlerFn   Handler Of This Tag Type                   *
 * Output        : None                                                   *
 * Return value  : Pointer to the allocated structure on success.         *
 *                 NULL on error                                          *
 **************************************************************************/

 Httpd_TagType_t *Httpd_TagType_Alloc( int32_t                 Value,
                                      char                 *pBeginKey,
                                      char                 *pEndKey,
                                      HTTPD_TAG_TYPE_HANDLER  HandlerFn)
{
  Httpd_TagType_t  *pNew=NULL;

  /**
   *
   */
  pNew = (Httpd_TagType_t *) of_malloc(sizeof(Httpd_TagType_t));

  if (!pNew)
  {
    return NULL;
  }

  pNew->pBeginKey = of_strdup(pBeginKey);

  if (!pNew->pBeginKey)
  {
    of_free(pNew);
    return NULL;
  }

  pNew->pEndKey = of_strdup(pEndKey);

  if (!pNew->pEndKey)
  {
    of_free(pNew->pBeginKey);
    of_free(pNew);
    return NULL;
  }

  pNew->Value     = Value;
  pNew->HandlerFn = HandlerFn;
  pNew->pNext     = NULL;
  Httpd_HashTable_Init(&pNew->HTable);

  return pNew;
} /* Httpd_TagType_Alloc() */


/**************************************************************************
 * Function Name : Httpd_TagType_Free                                     *
 * Description   : This frees the tag type structure                      *
 * Input         : pThis   pointer to the TagType structure               *
 * Output        : None                                                   *
 * Return value  : returns OF_SUCCESS                                      *
 **************************************************************************/

 int32_t Httpd_TagType_Free( Httpd_TagType_t  *pThis )
{
  HTTPD_ASSERT(!pThis);
  HTTPD_ASSERT(!pThis->pBeginKey);
  HTTPD_ASSERT(!pThis->pEndKey);

  of_free(pThis->pEndKey);
  of_free(pThis->pBeginKey);
  of_free(pThis);

  return OF_SUCCESS;
} /* Httpd_TagType_Free() */

/***********************************************************************
 * Function Name : Httpd_TagType_Handler_Table                         *
 * Description   : This is the table tag type handler.                  *
 * Input         : pHttpreq - Pointer To The Request                   *
 *                 pTTEntry - Private Tag-Type Entry                   *
 *                 pBuf     - Dynamic buffer to send                   *
 * Output        :                                                     *
 * Return value  : OF_SUCCESS on success                                *
 *                 OF_FAILURE on failure                                *
 ***********************************************************************/

 int32_t Httpd_TagType_Handler_Table( HttpRequest      *pRequest,
                                     Httpd_TTEntry_t  *pTTEntry,
                                     char          *pBuf )
{
  Httpd_Value_t      *pValAry=NULL;
  HTTPD_CBFUN_TABLE  CbFun;
  int32_t            RetVal=0;

  HTTPD_ASSERT(!pRequest);
  HTTPD_ASSERT(!pTTEntry);

  CbFun = (HTTPD_CBFUN_TABLE) pTTEntry->pCbFunct;

  if (pTTEntry->VarCount > 0)
  {
    pValAry = Httpd_ValAry_Alloc(pTTEntry->pVarArray, pTTEntry->VarCount,
                                 FALSE);

    if (!pValAry)
    {
      return OF_FAILURE;
    }
  }
  else
  {
    pValAry = NULL;
  }

  RetVal = CbFun(pRequest, pBuf, of_strlen(pBuf), pTTEntry->VarCount,
                 pTTEntry->pVarArray, pValAry);

  if(Httpd_ValAry_Free(pValAry)!=OF_SUCCESS)
  {
        HttpDebug((" Httpd_ValAry_Free returned Failure \r\n"));
        return OF_FAILURE;
  }
  return RetVal;
} /* Httpd_TagType_Handler_Table() */

/***********************************************************************
 * Function Name : Httpd_TagType_Handler_General                       *
 * Description   : This is the general tag type handler.                *
 * Input         : pHttpreq - Pointer To The Request                   *
 *                 pTTEntry - Private Tag-Type Entry                   *
 *                 pBuf     - Dynamic buffer to send                   *
 * Output        :                                                     *
 * Return value  : OF_SUCCESS on success                                *
 *                 OF_FAILURE on failure                                *
 ***********************************************************************/

int32_t Httpd_TagType_Handler_General( HttpRequest      *pRequest,
                                       Httpd_TTEntry_t  *pTTEntry,
                                       char          *pBuf )
{
  HTTPD_CBFUN_GENERAL  CbFun;
  Httpd_Value_t        *pValAry=NULL;
  int32_t              RetVal=0;
  char              *pAppData=NULL;

  HTTPD_ASSERT(!pRequest);
  HTTPD_ASSERT(!pTTEntry);
  HTTPD_ASSERT(!pBuf);

  if (pRequest->pAppData[0] != '\0')
  {
    pAppData = (char *) pRequest->pAppData;
  }
  else
  {
    pAppData = NULL;
  }

  CbFun = (HTTPD_CBFUN_GENERAL) pTTEntry->pCbFunct;

  /**
   * Some pages have blank tags, no variable inside.
   */
  if (pTTEntry->VarCount > 0)
  {
    pValAry = Httpd_ValAry_Alloc(pTTEntry->pVarArray, pTTEntry->VarCount,
                                 FALSE);

    if (!pValAry)
    {
      return OF_FAILURE;
    }
  }
  else
  {
    pValAry = NULL;
  }

  if (CbFun(pRequest, pValAry, pTTEntry->VarCount, pAppData) != OF_SUCCESS)
  {
#ifdef HTTPD_DEBUG
    Httpd_Error("Callback Function For a General Tag Failed!\n");
    Httpd_Print("General Tag Handler For '%s' is Failed!\n",
                pTTEntry->pTagName);
#endif /* HTTPD_DEBUG */
    if(Httpd_ValAry_Free(pValAry)!=OF_SUCCESS)
    {
      	  HttpDebug((" Httpd_ValAry_Free returned Failure \r\n"));      	  
    }
    return OF_FAILURE;
  }

  RetVal = Httpd_Send_DynamicBuf( pRequest, pBuf, of_strlen(pBuf),
                                  pTTEntry->VarCount, pTTEntry->pVarArray,
                                  pValAry);
   if(Httpd_ValAry_Free(pValAry)!=OF_SUCCESS)
  {
        HttpDebug((" Httpd_ValAry_Free returned Failure \r\n"));
        return OF_FAILURE;
  }
  return RetVal;
} /* Httpd_TagType_Handler_General() */

/***********************************************************************
 * Function Name : Httpd_Register_FileStoreTag                         *
 * Description   : Registers a action tag to the HTML parse engine     *
 * Input         : pTagName - Name of the action tag                   *
 *                 Callback-Function, Variable-Count, Variable Array   *
 * Output        :                                                     *
 * Return value  : OF_SUCCESS on success                                *
 *                 OF_FAILURE on failure                                *
 ***********************************************************************/

int32_t Httpd_Register_FileStoreTag( const char     *pTagName,
                                     void            *pCbFunct,
                                     int32_t           VarCount,
                                     Httpd_Variable_t  *pVarArray )
{
  HttpTagArgs_t   TagArgs = {};
  TagArgs.pTagName = of_strdup((char *)pTagName);
  if(!TagArgs.pTagName)
  {
       HttpDebug((" TagArgs.pTagName is NULL\r\n"));
       return OF_FAILURE;
  }
  TagArgs.pCbFunct = pCbFunct;
  TagArgs.VarCount = VarCount;
  TagArgs.pVarArray = pVarArray;

  /**
   * Register the file store tag.
   */
  if(Httpd_Register_Tag(HTTPD_TAG_TYPE_ACTION, TagArgs)!=OF_SUCCESS)
  {
       HttpDebug((" Httpd_Register_Tag is failed \r\n"));
       of_free(TagArgs.pTagName);  
       return OF_FAILURE;
  }
 of_free(TagArgs.pTagName);
 return OF_SUCCESS;
} /* Httpd_Register_FileStoreTag() */

#if 0
/***********************************************************************
 * Function Name : Httpd_Search_TableTag                               *
 * Description   : Searches for the given table tag name in the        *
 *                 table tag type's hash table                          *
 * Input         : pTagName - Name of the table tag                    *
 * Output        :                                                     *
 * Return value  : Pointer to the hash entry if match found            *
 *                 NULL is match is not found                          *
 ***********************************************************************/

 Httpd_HashEntry_t *Httpd_Search_TableTag( char  *pTagName )
{
  return Httpd_Search_Tag(HTTPD_TAG_TYPE_TABLE, pTagName);
} /* Httpd_Search_TableTag() */
#endif /* 0 */

/***********************************************************************
 * Function Name : Httpd_Register_TableTag                             *
 * Description   : Registers a table tag to the HTML parse engine      *
 * Input         : pTagName - Name of the table tag                    *
 *                 Callback-Function, Variable-Count, Variable Array   *
 * Output        :                                                     *
 * Return value  : OF_SUCCESS on success                                *
 *                 OF_FAILURE on failure                                *
 ***********************************************************************/

int32_t Httpd_Register_TableTag( const char     *pTagName,
                                 void            *pCbFunct,
                                 int32_t           VarCount,
                                 Httpd_Variable_t  *pVarArray )
{
  HttpTagArgs_t   TagArgs = {};
  TagArgs.pTagName = of_strdup((char *)pTagName);
  if(!TagArgs.pTagName)
  { 
      HttpDebug((" TagArgs.pTagName is NULL\r\n"));
      return OF_FAILURE;
  }
  TagArgs.pCbFunct = pCbFunct;
  TagArgs.VarCount = VarCount;
  TagArgs.pVarArray = pVarArray;

  /**
   * Register the table tag.
   */
   if(Httpd_Register_Tag(HTTPD_TAG_TYPE_GENERAL,TagArgs)!=OF_SUCCESS)
   {
        HttpDebug((" Httpd_Register_Tag is failed \r\n"));
        of_free(TagArgs.pTagName);  
        return OF_FAILURE;
   }
   of_free(TagArgs.pTagName);  
   return OF_SUCCESS;
} /* Httpd_Register_TableTag() */

/**************************************************************************
 * Function Name : Httpd_TagType_Register                                 *
 * Description   : Registers a tag type handler to the parsing engine     *
 * Input         : Value, Begin Key, End Key and Handler.                 *
 * Output        : None                                                   *
 * Return value  : OF_SUCCESS on successful addition                       *
 *                 OF_FAILURE on failure.                                  *
 **************************************************************************/

 int32_t Httpd_TagType_Register( int32_t                 Value,
                                           char                 *pBeginKey,
                                           char                 *pEndKey,
                                           HTTPD_TAG_TYPE_HANDLER  HandlerFn )
{
  Httpd_TagType_t  *pTemp=NULL;

  pTemp = Httpd_TagType_Alloc(Value, pBeginKey, pEndKey, HandlerFn);

  if (!pTemp)
  {
    Httpd_Error("Unable to Allocate Memory For New TagType\n");
    return OF_FAILURE;
  }

  if (Httpd_TagList_Add(pTemp) != OF_SUCCESS)
  {
    Httpd_TagType_Free(pTemp);
    return OF_FAILURE;
  }

  return OF_SUCCESS;
} /* Httpd_TagType_Register() */

/***********************************************************************
 * Function Name : Httpd_Register_ActionTag                            *
 * Description   : Registers a action tag to the HTML parse engine     *
 * Input         : pTagName - Name of the action tag                   *
 *                 Callback-Function, Variable-Count, Variable Array   *
 * Output        :                                                     *
 * Return value  : OF_SUCCESS on success                                *
 *                 OF_FAILURE on failure                                *
 ***********************************************************************/

int32_t Httpd_Register_ActionTag( const char     *pTagName,
                                  void            *pCbFunct,
                                  int32_t            VarCount,
                                  Httpd_Variable_t  *pVarArray )
{
  HttpTagArgs_t   TagArgs = {};
  TagArgs.pTagName = of_strdup((char *)pTagName);
  if(!TagArgs.pTagName)
  { 
      HttpDebug((" TagArgs.pTagName is NULL\r\n"));
      return OF_FAILURE;
  }
  TagArgs.pCbFunct = pCbFunct;
  TagArgs.VarCount = VarCount;
  TagArgs.pVarArray = pVarArray;
  if(Httpd_Register_Tag(HTTPD_TAG_TYPE_ACTION, TagArgs)!=OF_SUCCESS)
  {    
        HttpDebug((" Httpd_Register_Tag is failed \r\n"));
        of_free(TagArgs.pTagName);  
        return OF_FAILURE;
   }
   of_free(TagArgs.pTagName);  
   return OF_SUCCESS;
} /* Httpd_Register_ActionTag() */

/***********************************************************************
 * Function Name : Httpd_Register_FileSendTag                          *
 * Description   : Registers the file tag to the HTML parse engine.    *
 * Input         : pTagName - Name of the action tag                   *
 *                 Callback-Function, Variable-Count, Variable Array   *
 * Output        :                                                     *
 * Return value  : OF_SUCCESS on success                                *
 *                 OF_FAILURE on failure                                *
 ***********************************************************************/

int32_t Httpd_Register_FileSendTag( const char     *pTagName,
                                    void            *pCbFunct,
                                    int32_t            VarCount,
                                    Httpd_Variable_t  *pVarArray )
{
  HttpTagArgs_t TagArgs = {};
  TagArgs.pTagName = of_strdup((char *)pTagName);
  if(!TagArgs.pTagName)
  { 
      HttpDebug((" TagArgs.pTagName is NULL\r\n"));
      return OF_FAILURE;
  }
  TagArgs.pCbFunct = pCbFunct;
  TagArgs.VarCount = VarCount;
  TagArgs.pVarArray = pVarArray;
  if( Httpd_Register_Tag(HTTPD_TAG_TYPE_FILE_SEND, TagArgs)!=OF_SUCCESS)
  {    
        HttpDebug((" Httpd_Register_Tag is failed \r\n"));
        of_free(TagArgs.pTagName);  
        return OF_FAILURE;
   }
   of_free(TagArgs.pTagName);  
   return OF_SUCCESS;
} /* Httpd_Register_FileSendTag() */

/***********************************************************************
 * Function Name : Httpd_Register_Tag                                  *
 * Description   : general function to register any kind of tag using  *
 *                 the value of tag type                                *
 *                 Control always comes here while registering any tag *
 * Input         : pTagName - Name of the action tag                   *
 *                 Callback-Function, Variable-Count, Variable Array   *
 * Output        :                                                     *
 * Return value  : OF_SUCCESS on success                                *
 *                 OF_FAILURE on failure                                *
 ***********************************************************************/

int32_t Httpd_Register_Tag( int32_t           Value,
                            HttpTagArgs_t     TagArgs)
{
  Httpd_TagType_t   *pThis=NULL;

  pThis = Httpd_TagType_SearchByValue(Value);

  if (!pThis)
  {
#ifdef HTTPD_DEBUG
    Httpd_Print("Httpd:: Bad Tag-Type %d. Check if Http is initialized by now\n",
                Value);
#endif /* HTTPD_DEBUG */
    return OF_FAILURE;
  }
  return Httpd_HashTable_AddEntryEx(&pThis->HTable, TagArgs);
} /* Httpd_Register_Tag() */

/************************************************************************
* Function Name : Httpd_DeRegister_Tag                                  *
* Description   : general function to register any kind of tag using    *
*                 the value of tag type                                  *
*                 Control always comes here while registering any tag   *
* Input         : pTagName - Name of the action tag                     *
*                 Callback-Function, Variable-Count, Variable Array     *
* Output        :                                                       *
* Return value  : OF_SUCCESS on success                                  *
*                  OF_FAILURE on failure                                 *
*************************************************************************/

int32_t Httpd_DeRegister_Tag( int32_t        Value,
                              const char  *pTagName )
{
  Httpd_TagType_t   *pThis=NULL;

  pThis = Httpd_TagType_SearchByValue(Value);

  if (!pThis)
  {
#ifdef HTTPD_DEBUG
    Httpd_Print("Httpd:: Bad Tag-Type %d. Check if Http is initialized by now\n",
                Value);
#endif /* HTTPD_DEBUG */
    return OF_FAILURE;
  }

  return Httpd_HashTable_DeleteEntryEx(&pThis->HTable, pTagName);
} /* Httpd_DeRegister_Tag() */

/***********************************************************************
 * Function Name : Httpd_Register_TagEx                                *
 * Description   : same as above function, but takes pointer to        *
 *                 registry entry structure                            *
 * Input         : pThis - pointer to registry entry                   *
 * Output        :                                                     *
 * Return value  : OF_SUCCESS on success                                *
 *                 OF_FAILURE on failure                                *
 ***********************************************************************/

int32_t Httpd_Register_TagEx( Httpd_RegEntry_t  *pThis )
{
  HttpTagArgs_t TagArgs = {};
  TagArgs.pTagName = of_strdup((char *)pThis->pTagName);
  if(!TagArgs.pTagName)
  { 
      HttpDebug((" TagArgs.pTagName is NULL\r\n"));
      return OF_FAILURE;
  }
  TagArgs.pCbFunct = pThis->pCbFunct;
  TagArgs.VarCount = pThis->VarCount;
  TagArgs.pVarArray = pThis->pVarArray;
  TagArgs.ucPrivilage = pThis->ucPrivilage;

  if( Httpd_Register_Tag(pThis->Type, TagArgs)!=OF_SUCCESS)
    {    
        HttpDebug((" Httpd_Register_Tag is failed \r\n"));
        of_free(TagArgs.pTagName);  
        return OF_FAILURE;
   }
   of_free(TagArgs.pTagName);  
   return OF_SUCCESS;
} /* Httpd_Register_TagEx() */

/***************************************************************************
 * Function Name : Httpd_Register_TagArray                                 *
 * Description   : Registers all the entries by going through the array of *
 *                 registry entries. Last entry in the array MUST end with *
 *                 having HTTPD_TAG_TYPE_NONE in the value field.          *
 * Input         : pAry (I) - Pointer to the first element in the array    *
 * Output        : None.                                                   *
 * Return value  : No. of registered entries in the array on success       *
 *                 Negative value of failed entry on FAILURE               *
 ***************************************************************************/

int32_t Httpd_Register_TagArray( Httpd_RegEntry_t  *pAry )
{
  int32_t  nCount = 0;

  HTTPD_ASSERT(!pAry);

  while (pAry)
  {
    if (pAry->Type == HTTPD_TAG_TYPE_NONE)
    {
      return nCount;
    }

    if (Httpd_Register_TagEx(pAry) < 0)
    {
      Httpd_Error("Unable to register tag\n");
      return  -(nCount+1);
    }
    nCount++;
    pAry++;
  }
  Httpd_Error("The Array is not ending with HTTPD_TAG_TYPE_NONE\n");

  return -(nCount);
} /* Httpd_Register_TagArray() */

/***********************************************************************
 * Function Name : Httpd_Value_CopyStr                                 *
 * Description   : This copies a string to the httpd-value structure   *
 *                 This function is used by callback function          *
 *                 It strips the string if it exceeds its size         *
 * Input         : pVal - Pointer to the httpd-value                   *
 *                 pStr - String to copy                               *
 * Output        :                                                     *
 * Return value  : OF_SUCCESS on success                                *
 *                 OF_FAILURE on failure                                *
 ***********************************************************************/
int32_t Httpd_Value_CopyStr( Httpd_Value_t  *pVal,
                             char        *pStr )
{
  HTTPD_ASSERT(!pVal);
  HTTPD_ASSERT(!pVal->value_p);

  if ((pStr) &&
      (of_strlen(pStr) >= pVal->Size))
  {
    Httpd_Warning("Httpd_Value_CopyStr :: Len >= Max. Stripping");
  }

  if (pStr)
  {
    of_strncpy(pVal->value_p, pStr, pVal->Size-1);
  }
  else
  {
    of_strcpy(pVal->value_p, "");
  }

  pVal->value_p[pVal->Size-1] = '\0';
  return OF_SUCCESS;
} /* Httpd_Register_TagArray() */

/***********************************************************************
 * Function Name : Httpd_Value_CopyInt                                 *
 * Description   : This copies an integer to the httpd-value structure *
 * Input         : pVal - Pointer to the httpd-value                   *
 *                 iMyVal - Value to be copied                         *
 * Output        :                                                     *
 * Return value  : OF_SUCCESS on success                                *
 *                 OF_FAILURE on failure                                *
 ***********************************************************************/

int32_t Httpd_Value_CopyInt( Httpd_Value_t  *pVal,
                             int32_t        iMyVal )
{
  if (sprintf(pVal->value_p, "%d", iMyVal) != 1)
  {
    return OF_FAILURE;
  }
  else
  {
    return OF_SUCCESS;
  }
} /* Httpd_Value_CopyInt() */

/***********************************************************************
 * Function Name : Httpd_Value_CopyUInt                                *
 * Description   : This copies an unsigned integer to the httpd-value  *
 * Input         : pVal - Pointer to the httpd-value                   *
 *                 uiMyVal - Value to be copied                        *
 * Output        :                                                     *
 * Return value  : OF_SUCCESS on success                                *
 *                 OF_FAILURE on failure                                *
 ***********************************************************************/

int32_t Httpd_Value_CopyUInt( Httpd_Value_t  *pVal,
                              uint32_t       uiMyVal )
{
  if (sprintf(pVal->value_p, "%u", uiMyVal) != 1)
  {
    return OF_FAILURE;
  }
  else
  {
    return OF_SUCCESS;
  }
} /* Httpd_Value_CopyUInt() */

/***********************************************************************
 * Function Name : Httpd_Value_AppendStr                               *
 * Description   : This appends the given string to the httpd-value    *
 * Input         : pVal - Pointer to the httpd-value                   *
 *                 pStr - string to be appended                        *
 * Output        :                                                     *
 * Return value  : OF_SUCCESS on success                                *
 *                 OF_FAILURE on failure                                *
 ***********************************************************************/

int32_t Httpd_Value_AppendStr( Httpd_Value_t  *pVal,
                               char        *pStr )
{
  int32_t  lMyLen = 0;
  int32_t  lThatLen = 0;

  lMyLen   = of_strlen(pVal->value_p);
  lThatLen = of_strlen(pStr);

  if (lMyLen >= pVal->Size-1)
  {
    return OF_FAILURE;
  }

  if ((lMyLen+lThatLen) < pVal->Size)
  {
    of_strcpy(pVal->value_p+lMyLen, pStr);
  }
  else
  {
    of_strncpy(pVal->value_p+lMyLen, pStr, pVal->Size - lMyLen -1);
    pVal->value_p[pVal->Size-1] = '\0';
  }
  return OF_SUCCESS;
} /* Httpd_Value_AppendStr() */

/***********************************************************************
 * Function Name : Httpd_Value_StrCmp                                  *
 * Description   : This compares the http-value with given string      *
 *                 Does case-sensitive comparison                     *
 * Input         : pVal - Pointer to the httpd-value                   *
 *                 pThisStr - string to be compared with               *
 * Output        :                                                     *
 * Return value  : same as strcmp of standard libc function            *
 ***********************************************************************/

int32_t Httpd_Value_StrCmp( Httpd_Value_t  *pVal,
                            char        *pThisStr )
{
  return of_strcmp(pVal->value_p, pThisStr);
} /* Httpd_Value_StrCmp() */

/***********************************************************************
 * Function Name : Httpd_Value_StrCaseCmp                              *
 * Description   : This compares the http-value with given string      *
 *                 Does case-insensitive comparison                   *
 * Input         : pVal - Pointer to the httpd-value                   *
 *                 pThisStr - string to be compared with               *
 * Output        :                                                     *
 * Return value  : same as strcasecmp of standard libc function        *
 ***********************************************************************/

int32_t Httpd_Value_StrCaseCmp( Httpd_Value_t  *pVal,
                                char        *pThisStr )
{
  return strcasecmp(pVal->value_p, pThisStr);
} /* Httpd_Value_StrCaseCmp() */

/***********************************************************************
 * Function Name : Httpd_PostString_Convert                            *
 * Description   : This converts the post string received from browser *
 *                 normal form. This writes output string back on to   *
 *                 input. Replaces '+' with ' '                        *
 *                 It is guaranteed that length of output is less than  *
 *                 that of input                                       *
 * Input         : cInString - Pointer to the post string              *
 * Output        : writes converted string back to input parameter     *
 * Return value  : OF_SUCCESS on success                                *
 *                 OF_FAILURE on failure                                *
 ***********************************************************************/
 
int32_t Httpd_PostString_Convert( unsigned char  *cInString )
{
  unsigned char  *pBack=NULL;
  unsigned char  *pFront=NULL;
  unsigned char  nVal=0;

  if (!cInString)
  {
    return -1;
  }

  pBack = pFront = cInString;

  while (*pFront)
  {
    if (*pFront == '+')
    {
      *pBack = ' ';
    }
    else if (*pFront == '%')
    {
      if (HexToInt(pFront, &nVal) != OF_SUCCESS)
      {
        *pBack = '\0';  /* Parse Error*/
        return OF_FAILURE;
      }
      else
      {
        *pBack = nVal;
        /* This Should Be Increased By 3. to skip something like  %20*/
        /* But There is an increment down there.*/
        pFront += 2;
      }
    }
    else
    {
      *pBack = *pFront;
    }

    pBack++;
    pFront++;
  }
  *pBack = '\0';

  return OF_SUCCESS;
} /* Httpd_PostString_Convert() */

/***********************************************************************
 * Function Name : HexToInt                                            *
 * Description   : Converts a two digit hex into decimal               *
 * Input         : HexString - Pointer to the two digit hex string     *
 * Output        : pOutput - pointer to output character(byte)         *
 * Return value  : OF_SUCCESS on success                                *
 *                 OF_FAILURE on failure                                *
 ***********************************************************************/

 int32_t HexToInt( unsigned char  *HexString,
                  unsigned char  *pOutput )
{
  uint16_t  uiInteger = 0;
  uint16_t  uiCnt=0;
  uint16_t  uiInt=0;

  HexString++;

  for (uiCnt = 0; uiCnt < 2; uiCnt++)
  {
    switch (*HexString)
    {
    case '0':
      uiInt = 0;
      break;
    case '1':
      uiInt = 1;
      break;
    case '2':
      uiInt = 2;
      break;
    case '3':
      uiInt = 3;
      break;
    case '4':
      uiInt = 4;
      break;
    case '5':
      uiInt = 5;
      break;
    case '6':
      uiInt = 6;
      break;
    case '7':
      uiInt = 7;
      break;
    case '8':
      uiInt = 8;
      break;
    case '9':
      uiInt = 9;
      break;
    case 'A':
      uiInt = 10;
      break;
    case 'B':
      uiInt = 11;
      break;
    case 'C':
      uiInt = 12;
      break;
    case 'D':
      uiInt = 13;
      break;
    case 'E':
      uiInt = 14;
      break;
    case 'F':
      uiInt = 15;
      break;
    default :
#ifdef HTTPD_DEBUG
      Httpd_Print("HexToInt :: Invalid Character ASCII = %d\n",
                  *HexString);
#endif /* HTTPD_DEBUG */
      return OF_FAILURE;
    }
    if (uiCnt == 0)
    {
      uiInteger = uiInteger + uiInt * 16;
    }
    else
    {
      uiInteger = uiInteger + uiInt;
    }

    HexString++;
  }
  *pOutput = uiInteger;

  return OF_SUCCESS;
} /* HexToInt() */

/***********************************************************************
 * Function Name : Httpd_ValAry_Alloc                                  *
 * Description   : Allocates a new value array depending on the given  *
 *                 variable array and count                            *
 * Input         : pVarArray - contains sizes of each variable         *
 *                 nCount    - variable count                          *
 * Output        :                                                     *
 * Return value  : Pointer to the Value Array on success               *
 *                 NULL on failure                                     *
 ***********************************************************************/

 Httpd_Value_t *Httpd_ValAry_Alloc( Httpd_Variable_t  *pVarArray,
                                   uint32_t          nCount,
                                   bool           bNoVals )
{
  static  unsigned char  *pNullStr;
  Httpd_Value_t        *pTemp=NULL;
  char              *pData=NULL;
  uint32_t             nTotal = 0, ii=0;

  HTTPD_ASSERT(!pVarArray);

  if (!bNoVals)
  {
    for (ii = 0; ii < nCount; ii++)
    {
      nTotal += pVarArray[ii].Size;
    }
  }

  nTotal += (nCount)*sizeof(Httpd_Value_t);

  pData = (char *)  of_malloc(nTotal);

  if (!pData)
  {
    return NULL;
  }

  pTemp = (Httpd_Value_t *)(pData);

  pData += nCount*sizeof(Httpd_Value_t);

  for (ii = 0; ii < nCount; ii ++)
  {
    pTemp[ii].Size    = pVarArray[ii].Size;

    if (!bNoVals)
    {
      pTemp[ii].value_p  = pData;
      Httpd_Value_CopyStr(&pTemp[ii], "");
      pData += pVarArray[ii].Size;
    }
    else
    {
      pTemp[ii].value_p = (char *) pNullStr;
    }
  }

  return pTemp;
} /* Httpd_ValAry_Alloc() */

/***********************************************************************
 * Function Name : Httpd_ValAry_Free                                   *
 * Description   : Frees the value array allocated by above function   *
 * Input         : pVarArray - pointer to value array                  *
 * Output        :                                                     *
 * Return value  : OF_SUCCESS on success                                *
 *                 OF_FAILURE on failure                                *
 ***********************************************************************/

 int32_t Httpd_ValAry_Free( Httpd_Value_t  *pVarArray )
{
 if(!pVarArray )
 {
      return OF_FAILURE;
 }
       of_free(pVarArray);
       return OF_SUCCESS;
} /* Httpd_ValAry_Free() */

/***********************************************************************
 * Function Name : Httpd_SendBinFile                                   *
 * Description   : To Send requested binary file to the Browser(PC)    *
 * Input         : HttpRequest - pointer to HttpReuest                 *
 * Output        :                                                     *
 * Return value  : OF_SUCCESS on success                                *
 *                 OF_FAILURE on failure                                *
 ***********************************************************************/

int32_t Httpd_SendBinFile( HttpRequest  *pHttpReq)
{
  Httpd_HashEntry_t      *pThis=NULL;
  HTTPD_CBFUN_FILE_SEND  CbFun;

  HTTPD_ASSERT(!pHttpReq);

  /* Process for uploading image file in to flash */
  pThis = Httpd_Search_FileSendTag((char *) pHttpReq->filename);

  if (!pThis)
  {
#ifdef HTTPD_DEBUG
    Httpd_Warning("Httpd_Send_BinaryFile: Nobody Registered For File Tag '%s'\n",
                  pHttpReq->filename);
#endif /* HTTPD_DEBUG */
    HttpSendHtmlFile(pHttpReq, (unsigned char *) "errscrn.htm",
                    (unsigned char *)  "This feature is not available in this release");
    return OF_SUCCESS;
  }

  CbFun = (HTTPD_CBFUN_FILE_SEND) pThis->pCbFunct;
  HTTPD_ASSERT(!CbFun);

  if (CbFun(pHttpReq) != OF_SUCCESS)
  {
#ifdef HTTPD_DEBUG
    Httpd_Warning("ActionSendBinaryFile-Tag Callback Function Failed For '%s'\n",
                  pHttpReq->filename);
#endif /* HTTPD_DEBUG */
    return OF_FAILURE;
  }

  return OF_SUCCESS;
} /* Httpd_SendBinFile() */
/***********************************************************************
 * Function Name : Httpd_If_Process                               *
 * Description   : Passing  pHttpReq  and PHtmlBuf  *
 *                                     *
 * Input         : pHttpReq,pHtmlBuf     *
 * Output        :                                                     *
 * Return value  : OF_SUCCESS on success                                *
 *                 OF_FAILURE on failure                                *
 ***********************************************************************/
 
int32_t Httpd_If_Process( HttpRequest  *pRequest, unsigned char  *pHtmlBuf)
  {
  Httpd_HashEntry_t  *pThis=NULL;
  HTTPD_CBFUN_IF     CbFun;
  Httpd_Value_t      *pValAryFirstOut=NULL,*pValArySecondOut=NULL;
  char            *pAppData=NULL;
  bool            bCheck=FALSE;
  unsigned char           *pCur=NULL;
  unsigned char           *pStartIfBegin=NULL,*pStartIfEnd=NULL;
  unsigned char             pCompleteTag[HTTP_COMPLETE_IF_TAG_LEN]={};
  unsigned char           *pEndIfBegin=NULL,*pEndIfEnd=NULL;
  unsigned char           *FirstOperand=NULL,*SecondOperand=NULL,*TempString=NULL,*Condition=NULL;
  unsigned char           *FirstArgument=NULL,*SecondArgument=NULL,*FirstResult=NULL,*SecondResult=NULL;
  int32_t   	     Increment=1,TagLength=0;
pCur=pHtmlBuf;
while(1)
{
         pStartIfBegin=NULL;
         pStartIfEnd=NULL;
         pEndIfBegin=NULL;
         pEndIfEnd=NULL;
         pStartIfBegin=(unsigned char *) strstr((char *) pCur,"<IGW_STARTIF");
         if ( pStartIfBegin== NULL )
         {
              HttpDebug("There is no IGW_STARTIF");
         	return OF_SUCCESS;
         }
          pStartIfEnd = (unsigned char *) of_strstr((char *) pStartIfBegin, ">");
         if ( pStartIfEnd == NULL )
           {
              	Httpd_Debug("There is no end seperator for IGW_STARTIF");
              	return 	OF_SUCCESS;
           }
         /* pEndIfBegin= of_strstr(pCur,"<IGW_ENDIF"); */
         pEndIfBegin= (unsigned char *) of_strstr((char *) pStartIfBegin,"<IGW_ENDIF");
         if ( pEndIfBegin == NULL )
         {
         	Httpd_Debug("indicating that there is no corresponding IGW_ENDIF");
         	return OF_SUCCESS;
         } 
         pEndIfEnd = (unsigned char *) of_strstr((char *) pEndIfBegin, ">");
         if ( pEndIfEnd == NULL )
         {
         	Httpd_Debug(" indicating that there is no > seperator in IGW_ENDIF tag type");
         	return  OF_SUCCESS;
         }
         /*<IGW_STARTIF_CHECKMODULE("dhcp")_==_RETURNARUGMENT("true")>*/
         TagLength=pStartIfEnd - pStartIfBegin + 1  ;         
         of_strncpy((char *) pCompleteTag, (char *) pStartIfBegin ,TagLength);                 
         
         
         TempString=(unsigned char *) strtok((char *) pCompleteTag, "()_");
         if(!TempString)
         {
            HttpDebug(("TempString is NULL"));
            return OF_FAILURE;
         }
         
         Increment=1;
         while(TempString!='\0' && TempString)
         {
               TempString=(unsigned char *) strtok(NULL, "()_>\"");
               if(Increment==2)  FirstOperand=TempString;
               if(Increment==3)  FirstArgument=TempString;
               if(Increment==4)  Condition=TempString;
               if(Increment==5)  SecondOperand=TempString;
               if(Increment==6)  SecondArgument=TempString;
               Increment++;
         }
         pThis=Httpd_Search_Tag(HTTPD_TAG_TYPE_IF, (char *) FirstOperand);
         if(!pThis)
         {
              HttpDebug(("No one registered FirstOperand\r\n"));
            	return OF_SUCCESS;
         }
         /*check call back function registered or not */
         CbFun = (HTTPD_CBFUN_IF)pThis->pCbFunct;
         HTTPD_ASSERT(!CbFun);
         
         pValAryFirstOut=Httpd_ValAry_Alloc(pThis->pVarArray,pThis->VarCount, FALSE);
         if(!pValAryFirstOut)
         {
            HttpDebug(("pValAryFirstOut is NULL"));
            return OF_FAILURE;
         }
         
         if(CbFun(pRequest,FirstArgument, pValAryFirstOut,pThis->VarCount, pAppData) != OF_SUCCESS)
            {
                Httpd_Error("Callback Function For a httpd_if_process Failed!\n");
                if(Httpd_ValAry_Free(pValAryFirstOut)!=OF_SUCCESS)
                {
         	       HttpDebug((" Httpd_ValAry_Free returned Failure \r\n"));
         	       return OF_FAILURE;
                }
                return OF_SUCCESS;
             }
         FirstResult=(unsigned char *) pValAryFirstOut->value_p;
         
         pThis=Httpd_Search_Tag(HTTPD_TAG_TYPE_IF, (char *) SecondOperand);
         if(!pThis)
         {
              HttpDebug(("No one registered SecondOperand\r\n"));
         	return OF_SUCCESS;
         }
         /*Allocate Httvalue_p_t for storing the output string.;*/
         CbFun = (HTTPD_CBFUN_IF)pThis->pCbFunct;
         HTTPD_ASSERT(!CbFun);
         
         pValArySecondOut=Httpd_ValAry_Alloc(pThis->pVarArray,pThis->VarCount, FALSE);
         if(!pValArySecondOut)
         {
            HttpDebug((" is NULL"));
            return OF_FAILURE;
         } 
          if(CbFun(pRequest,SecondArgument, pValArySecondOut, pThis->VarCount, pAppData) != OF_SUCCESS)
          {
                 Httpd_Error("Callback Function For a httpd_if_process Failed!\n");
                 if(Httpd_ValAry_Free(pValArySecondOut)!=OF_SUCCESS)
                 {
             	     HttpDebug((" Httpd_ValAry_Free returned Failure \r\n"));
             	     return OF_FAILURE;
                 }
                 return OF_SUCCESS;
          }         
         SecondResult=(unsigned char *) pValArySecondOut->value_p;  
	  if(Condition != NULL)
	  {
    	      if (of_strcmp((char *) Condition,"==")==0)
             {
                    bCheck=TRUE;
             }
             else
             {
                    bCheck=FALSE;
             }
	 }
         if (pRequest->pAppData[0] != '\0')
          {
             pAppData = (char * )pRequest->pAppData;
          }
           else
          {
             pAppData = NULL;
          }      
         if(bCheck==TRUE)
         {
            if (of_strcmp((char *) FirstResult,(char *) SecondResult)==0)
            {
         	of_memcpy(pStartIfBegin, pStartIfEnd+1, pEndIfBegin - pStartIfEnd +1);
         	pCur = pStartIfBegin;
         	if(pValAryFirstOut )
         	{
                      of_free(pValAryFirstOut);
         	}
         	if(pValArySecondOut)
          	{
         		of_free(pValArySecondOut);
          	}
         	
            }
            else
            {
              of_memcpy(pStartIfBegin, pEndIfEnd+1, of_strlen((char *) pEndIfEnd )+1);
              pCur = pStartIfBegin;
              if(pValAryFirstOut )
              {
                     of_free(pValAryFirstOut);
               }
               if(pValArySecondOut)
               {
         	     of_free(pValArySecondOut);
                }		      
            }
         }
         else  
         {
            if (of_strcmp((char *) FirstResult,(char *) SecondResult)!=0)
            {
         	of_memcpy(pStartIfBegin, pStartIfEnd+1, pEndIfBegin - pStartIfEnd +1);
         	pCur = pStartIfBegin;
                if(pValAryFirstOut )
         	{
                      of_free(pValAryFirstOut);
         	}
         	if(pValArySecondOut)
          	{
         		of_free(pValArySecondOut);
          	} 
            }
            else
            {
              of_memcpy(pStartIfBegin, pEndIfEnd+1, of_strlen((char *) pEndIfEnd)+1);
              pCur = pStartIfBegin;
              if(pValAryFirstOut )
              {
                  of_free(pValAryFirstOut);
              }
              if(pValArySecondOut)
              {
         	  of_free(pValArySecondOut);
              }         
            }
         }
  }  /* while end */
  return OF_SUCCESS;
}


/***********************************************************************
 * Function Name : Httpd_HRefTag_Process                               *
 * Description   : Passing HREF ApplicationData to the CallbackFn    *
 *                 registered for HREF Tag name                         *
 * Input         : pTagName - Application Tag name;   HttpRequest     *
 * Output        :                                                     *
 * Return value  : OF_SUCCESS on success                                *
 *                 OF_FAILURE on failure                                *
 ***********************************************************************/

int32_t Httpd_HRefTag_Process( unsigned char     *pTagName,
                               HttpRequest  *pRequest )
{
  Httpd_HashEntry_t    *pThis=NULL;
  HTTPD_CBFUN_GENERAL  CbFun;
  Httpd_Value_t        *pValAry=NULL;
  char              *pAppData=NULL;
  bool              bConfig = FALSE;

  HTTPD_ASSERT(!pRequest);
  HTTPD_ASSERT(!pTagName);

  /**
   * Get the user privilages
   */
  bConfig = HttpUserHasAdminPriv((char *) pRequest->pUserName);
  /**
   * Search for callback registered for given HRef Tag.
   */
  pThis = Httpd_Search_GenTag((char *) pTagName);

  if (!pThis)
  {
#ifdef HTTPD_DEBUG
    Httpd_Warning("Httpd_HRefTag_Process: nobody registered for tag '%s'\n",
                  pTagName);
#endif /* HTTPD_DEBUG */
    return OF_FAILURE;
  }

  CbFun = (HTTPD_CBFUN_GENERAL) pThis->pCbFunct;

  HTTPD_ASSERT(!CbFun);

  if(pThis->ucPrivilage == HTTP_CBKFN_PRIV_ADMIN)
  {
    if(bConfig == FALSE)
    {
      HttpSendHtmlFile(pRequest,(unsigned char *)  "errscrn.htm",
                  (unsigned char *)  "You dont have permission to submit the changes");
      return OF_SUCCESS;
    }
  }

  if (pRequest->pAppData[0] != '\0')
  {
    pAppData = (char *) pRequest->pAppData;
  }
  else
  {
    pAppData = NULL;
  }

  if (pThis->VarCount > 0)
  {
    pValAry = Httpd_ValAry_Alloc(pThis->pVarArray, pThis->VarCount, FALSE);

    if (!pValAry)
    {
      return OF_FAILURE;
    }
  }
  else
  {
    pValAry = NULL;
  }

  if (CbFun(pRequest, pValAry, pThis->VarCount, pAppData) != OF_SUCCESS)
  {
#ifdef HTTPD_DEBUG
    Httpd_Error("Callback Function For a Httpd_HRefTag Failed!\n");
#endif /* HTTPD_DEBUG */
    if(Httpd_ValAry_Free(pValAry)!=OF_SUCCESS)
    {
	    HttpDebug((" Httpd_ValAry_Free returned Failure \r\n"));	   
    }
    return OF_FAILURE;
  }

  /**
   * Note: In CbFun it should send appropriate HTML file after its
   * processing in case of any failure....It has to send errscrn.htm page
   * with error message in case of success...It has to send appropriate
   * page...like iaptab.htm for iap* Href
   */
  return OF_SUCCESS;
} /* Httpd_HRefTag_Process() */

/***********************************************************************
 * Function Name : Httpd_ValAry_FromBinFileData                        *
 * Description   : Read the ValAry from RequestData (Read Bin file from*
 *                 Request Data                                        *
 * Input         :                                                     *
 * Output        :                                                     *
 * Return value  : OF_SUCCESS on success                                *
 *                 OF_FAILURE on failure                                *
 ***********************************************************************/

 int32_t Httpd_ValAry_FromBinFileData( unsigned char          *pMpBuf,
                                      int32_t           lMpBytes,
                                      unsigned char          *pBoundary,
                                      Httpd_Variable_t  *pVarAry,
                                      uint32_t          VarCount,
                                      Httpd_Value_t     *pValAry )
{
  unsigned char  aName[HTTPD_LOCAL_NAME_LEN];
  unsigned char  ucBndStr[HTTPD_LOCAL_BIND_STRING_LEN];
  unsigned char  *pCur=NULL;
  unsigned char  *pBack=NULL;
  uint32_t  ii=0;

  if ((!pMpBuf) ||
      (!pBoundary))
  {
    return OF_FAILURE;
  }

  if (VarCount == 0)
  {
    return OF_SUCCESS;
  }

  if ((!pVarAry) ||
      (!pValAry))
  {
    return OF_FAILURE;
  }

  if (of_strlen((char *) pBoundary) > 76)
  {
    return OF_FAILURE;
  }

  sprintf((char *) ucBndStr, "--%s", pBoundary);

  pCur = pMpBuf;

  pBack = pCur;
  pCur = (unsigned char *) of_strstr((char *) pCur, CRLF);

  if (!pCur)
  {
    return OF_FAILURE;
  }

  *pCur = '\0';
  pCur += of_strlen(CRLF);

  if (of_strcmp((char *) pBack, (char *) ucBndStr))
  {
    return OF_FAILURE;
  }

  sprintf((char *) ucBndStr, "\r\n--%s", pBoundary);

  while (pCur < pMpBuf+lMpBytes)
  {
    pBack = pCur;
    pCur = Httpd_MPart_GetSubmitName(pBack, aName, HTTPD_LOCAL_NAME_LEN - 1);

    if (!pCur)
    {
      return OF_FAILURE;
    }

    if (pCur >= pMpBuf+lMpBytes)
    {
      return OF_FAILURE;
    }

    for (ii = 0; ii < VarCount; ii ++)
    {
      if (!strcasecmp((char*)pVarAry[ii].name_p, (char *) aName))
      {
        pValAry[ii].value_p = (char *) pCur;
        break;
      }
    }

    pBack = pCur;
    if (ii < VarCount)
    {
      if (pValAry[ii].Size > (pMpBuf+lMpBytes)- pCur)
      {
        pValAry[ii].Size = (pMpBuf+lMpBytes)- pCur;
      }
    }

    pValAry[ii].value_p[pValAry[ii].Size] = '\0';
    pCur = pMpBuf+lMpBytes;
  }

  return OF_SUCCESS;
} /* Httpd_ValAry_FromBinFileData() */

/***********************************************************************
 * Function Name : Httpd_StoreBinFile                                  *
 * Description   : To Store the received Binary file. Reads the received*
 * Bin file and passes it to ApplCallBk to store it whenever data      *
 * receives it passes to application...its not wait for complete Data  *
 * Input         : pHttpReq, ulBufLen=RecievedDataLen, ulDatasz=TotalLen *
 * Output        :                                                     *
 * Return value  : OF_SUCCESS on success                                *
 *                 OF_FAILURE on failure                                *
 ***********************************************************************/

int32_t Httpd_StoreBinFile( HttpRequest  *pHttpReq,
                            uint32_t     ulBufLen,
                            uint32_t     ulDatasz )
{
  Httpd_Value_t           *pVals=NULL;
  unsigned char                *pTemp=NULL, *pBoundary=NULL, *pEnd=NULL ,*pCur =NULL;
  int32_t                 ulBndLen=0, ulEndFlag = 0;
  unsigned char                ucBndStr[HTTPD_LOCAL_BIND_STRING_LEN];
  HTTPD_CBFUN_FILE_STORE  FileCbFun;
  bool                 bConfig = FALSE;
  uint32_t                uiBytesWritten = 0, ii;
  Httpd_HashEntry_t       *pBinFileEntry = NULL;
  unsigned char  aName[HTTPD_LOCAL_NAME_LEN];

  HTTPD_ASSERT(!pHttpReq);

  /**
   * Get the bytes written from the cookie record.
   */
  if (HttpCookie_GetBytesWritten(pHttpReq->pCookieVal,
                                 &uiBytesWritten) != OF_SUCCESS)
  {
    return OF_FAILURE;
  }

  /**
   * Process for uploading image file in to flash.
   */
  if (uiBytesWritten == 0)
  {
    pBinFileEntry = Httpd_Search_FileStoreTag((char *) pHttpReq->filename);

    /**
     * Set the hash entry from the cookie record.
     */
    if (HttpCookie_SetHashEntry(pHttpReq->pCookieVal,
                                (void **) &pBinFileEntry) != OF_SUCCESS)
    {
      return OF_FAILURE;
    }
  }
  else
  {
    /**
     * Get the hash entry from the cookie record.
     */
    if (HttpCookie_GetHashEntry(pHttpReq->pCookieVal,
                                (void **) &pBinFileEntry) != OF_SUCCESS)
    {
      return OF_FAILURE;
    }
  }

  if (!pBinFileEntry)
  {
#ifdef HTTPD_DEBUG
    Httpd_Warning("Httpd_StoreBinFile: Nobody registered for action tag '%s'\n",
                  pHttpReq->filename);
#endif /* HTTPD_DEBUG */
    return OF_FAILURE;
  }

  bConfig = HttpUserHasAdminPriv((char *) pHttpReq->pUserName);

  if(pBinFileEntry->ucPrivilage == 0)
  {
    if(bConfig == FALSE)
    {
      HttpSendHtmlFile(pHttpReq,(unsigned char *)  "errscrn.htm",
                  (unsigned char *)  "You dont have authorisation to view this page");
      return OF_SUCCESS;
    }
  }

  FileCbFun = (HTTPD_CBFUN_FILE_STORE) pBinFileEntry->pCbFunct;
  HTTPD_ASSERT(!FileCbFun);

  pVals = Httpd_ValAry_Alloc(pBinFileEntry->pVarArray,
                             pBinFileEntry->VarCount, TRUE);
  
  if (!pVals)
  {
#ifdef HTTPD_DEBUG
    Httpd_Warning("Httpd_StoreBinFile: Unable To Allocate Memory For Value-Array\n");
#endif /* HTTPD_DEBUG */
    return OF_FAILURE;
  }

  /* Put CRLF at end of MPartBuf */
  pTemp = HttpRequest_GetMPartData(pHttpReq);

  pBoundary = HttpRequest_GetMPartBoundary(pHttpReq);
  if(!pBoundary)
  {
      #ifdef HTTPD_DEBUG
          Httpd_Error("\n\rHttpd_StoreBinFile() : Err, pBoundary is NULL\n\r");
      #endif /* HTTPD_DEBUG */
     if(Httpd_ValAry_Free(pVals)!=OF_SUCCESS)
     {
       HttpDebug((" Httpd_ValAry_Free returned Failure \r\n"));	      
     }
      return OF_FAILURE;
  }
  
  if (of_strlen((char *) pBoundary) > 76)
  {
#ifdef HTTPD_DEBUG
    Httpd_Error("\n\rHttpd_StoreBinFile() : Err, boundary > 76\n\r");
#endif /* HTTPD_DEBUG */
    if(Httpd_ValAry_Free(pVals)!=OF_SUCCESS)
    {
       HttpDebug((" Httpd_ValAry_Free returned Failure \r\n"));	      
    }
    return OF_FAILURE;
  }
  sprintf((char *) ucBndStr, "--%s", pBoundary);
  ulBndLen = of_strlen((char *) ucBndStr);

  /**
   * Received in single buffer.. Do old process.
   */
  if (ulDatasz == ulBufLen)
  {
    if (Httpd_ValAry_FromMultiPart(pTemp, ulBufLen, pBoundary,
                                   pBinFileEntry->pVarArray,
                                   pBinFileEntry->VarCount,
                                   pVals) != OF_SUCCESS)
    {
#ifdef HTTPD_DEBUG
      Httpd_Error("Httpd_ImageUpload_Process: Unable to parse multi-part ");
      Httpd_Error("post string\n");
#endif /* HTTPD_DEBUG */
      if(Httpd_ValAry_Free(pVals)!=OF_SUCCESS)
      {
	      HttpDebug((" Httpd_ValAry_Free returned Failure \r\n"));	      
      }
      return OF_FAILURE;
    }
  }
  else
  {
    if (uiBytesWritten == 0)
    {
      if (Httpd_ValAry_FromBinFileData(pTemp, ulBufLen, pBoundary,
                                       pBinFileEntry->pVarArray,
                                       pBinFileEntry->VarCount,
                                       pVals) != OF_SUCCESS)
      {
#ifdef HTTPD_DEBUG
        Httpd_Error("Httpd_ImageUpload_Process: Unable to parse ");
        Httpd_Error("multi-part post string\n");
#endif /* HTTPD_DEBUG */
        if(Httpd_ValAry_Free(pVals)!=OF_SUCCESS)
        {
             HttpDebug((" Httpd_ValAry_Free returned Failure \r\n"));	      
        }
        return OF_FAILURE;
      }
    }
    else
    {
      pVals[0].value_p = (char *) pTemp;
      pEnd = Httpd_MemMem(pTemp, ulBufLen, ucBndStr, ulBndLen);

      if (!pEnd)
      {
        pVals[0].Size = ulBufLen;
      }
      else
      { 
        if (pVals[0].Size > pEnd - pTemp)
        {
          pVals[0].Size = pEnd- pTemp-2;  /* 2?? */
        }
        ulEndFlag = 1; /* Bin file received completely */

       pCur = Httpd_MPart_GetSubmitName(pEnd, aName, HTTPD_LOCAL_NAME_LEN - 1);
        
       if(pCur)
       {
         for (ii = 0; ii < pBinFileEntry->VarCount; ii ++)
         {
      	   if (!strcasecmp((char*)pBinFileEntry->pVarArray[ii].name_p, (char *) aName))
           {
              pVals[ii].value_p = (char *) pCur;
              break;
           }
         }
           
         pCur = Httpd_MemMem(pCur, (pTemp+ulBufLen - pCur), ucBndStr, ulBndLen);
           
         if(!pCur)
         {
            return OF_FAILURE;
         }
  
         if (ii < pBinFileEntry->VarCount)
         {
            if (pVals[ii].Size > (pCur - (unsigned char*)pVals[ii].value_p))
            {
              pVals[ii].Size = pCur - (unsigned char*)pVals[ii].value_p;
            }

           pVals[ii].value_p[pVals[ii].Size] = '\0';
         }
       }      
      }
      pVals[0].value_p[pVals[0].Size] = '\0';
    }
  }

  if (uiBytesWritten == 0)
  {
    if (FileCbFun(pHttpReq, NULL, 0, HTTPD_FILE_CREATE) != OF_SUCCESS)
    {
      Httpd_Warning("Action upload-tag callback function(create) failed for '%s'\n",
                    pHttpReq->filename);
      if(Httpd_ValAry_Free(pVals)!=OF_SUCCESS)
      {
	    HttpDebug((" Httpd_ValAry_Free returned Failure \r\n"));
      }      
      return OF_FAILURE;
    }
    
    if (FileCbFun(pHttpReq, pVals, pBinFileEntry->VarCount,
                  HTTPD_FILE_VALIDATE) != OF_SUCCESS)
    {
      Httpd_Warning("Action upload-tag callback function(create) failed for '%s'\n",
                    pHttpReq->filename);
      if(Httpd_ValAry_Free(pVals)!=OF_SUCCESS)
      {
	    HttpDebug((" Httpd_ValAry_Free returned Failure \r\n"));
      }
      return OF_FAILURE;
    }

  }
  if (FileCbFun(pHttpReq, pVals, pBinFileEntry->VarCount,
                HTTPD_FILE_DATA) != OF_SUCCESS)
  {
    Httpd_Warning("Action upload-tag callback function(file data) failed for '%s'\n",
                  pHttpReq->filename);
    if(Httpd_ValAry_Free(pVals)!=OF_SUCCESS)
    {
	    HttpDebug((" Httpd_ValAry_Free returned Failure \r\n"));
    }
    return OF_FAILURE;
  }

  if ((ulEndFlag) ||
      (ulBufLen + uiBytesWritten >= ulDatasz))
  {
    if (FileCbFun(pHttpReq, NULL, 0, HTTPD_FILE_CLOSE) != OF_SUCCESS)
    {
      Httpd_Warning("Action upload-tag callback function(close) failed for '%s'\n",
                    pHttpReq->filename);
     if(Httpd_ValAry_Free(pVals)!=OF_SUCCESS)
     {
	    HttpDebug((" Httpd_ValAry_Free returned Failure \r\n"));	
      }
      return OF_FAILURE;
    }
    pBinFileEntry = NULL;

    /**
     * Set the hash entry from the cookie record.
     */
    if (HttpCookie_SetHashEntry(pHttpReq->pCookieVal,
                             (void **) &pBinFileEntry) != OF_SUCCESS)
    {
      return OF_FAILURE;
    }
  }
  if(Httpd_ValAry_Free(pVals)!=OF_SUCCESS)
  {
	    HttpDebug((" Httpd_ValAry_Free returned Failure \r\n"));
	    return OF_FAILURE;
  }
  return OF_SUCCESS;
} /* Httpd_StoreBinFile() */

/***********************************************************************
 * Function Name : Httpd_PostString_Process                            *
 * Description   : This is the action tag type handler                  *
 *                 this calls the registered function of the           *
 *                 target igw tag.                                     *
 * Input         : pHttpReq - pointer to http request                  *
 * Output        :                                                     *
 * Return value  : OF_SUCCESS on success                                *
 *                 OF_FAILURE on failure                                *
 ***********************************************************************/

int32_t Httpd_PostString_Process( HttpRequest  *pHttpReq )
{
  Httpd_HashEntry_t   *pThis=NULL;
  HTTPD_CBFUN_ACTION  CbFun;
  Httpd_Value_t       *pVals=NULL;
  bool             bConfig = FALSE;
  int32_t             Perm = 0;
  int32_t             iRet = -1;
  HttpClientReq_t     *pClientReq=NULL;

  HTTPD_ASSERT(!pHttpReq);

  Httpd_GetPermission((char *) pHttpReq->filename,&Perm);
  if(Perm == HTTPD_PRIV_USER)
  {
    bConfig = TRUE;
  }
  else
  {
    bConfig = HttpUserHasAdminPriv((char *) pHttpReq->pUserName);
  }
  pClientReq = (HttpClientReq_t *)pHttpReq->pPvtData;
  pThis = Httpd_Search_ActionTag_New((char *) pHttpReq->filename);

  if (pThis)
  {
    /**
     * Set the bytes written in the cookie record.
     */
    if(!((pClientReq->ulPageType == LOGINPAGEREQUEST) && 
        (pClientReq->ulConnType)))		         
    {	    
      if (HttpCookie_SetBytesWritten(pHttpReq->pCookieVal,
                                   0) != OF_SUCCESS)
      {
        HTTPDFREE(pHttpReq);
        return OF_FAILURE;
      }
    }
    CbFun = (HTTPD_CBFUN_ACTION) pThis->pCbFunct;
    HTTPD_ASSERT(!CbFun);

    if(pThis->ucPrivilage == HTTP_CBKFN_PRIV_ADMIN)
    {
      if(bConfig == FALSE)
      {
        HttpSendHtmlFile(pHttpReq,(unsigned char *)  "errscrn.htm",
                    (unsigned char *)  "You dont have authorisation to view this page");
        HTTPDFREE(pHttpReq);
	 return OF_SUCCESS;
      }
    }

    iRet = ((HTTPD_CBFUN_ACTION_NEW)CbFun)(pHttpReq, pHttpReq->cPostString);

    if (iRet == OF_SUCCESS)
    {
#ifndef INTOTO_CMS_SUPPORT
     HTTPDFREE(pHttpReq);
#endif  /* INTOTO_CMS_SUPPORT */
      return OF_SUCCESS;
    }
    else if(iRet == HTTP_DONOT_REMOVE_REQUEST)
    {
        return HTTP_DONOT_SEND_RESPONCE;
    }
    else
    {
      Httpd_Warning("Action-Tag callback function(new) failed for '%s'\n",
                    pHttpReq->filename);
      HTTPDFREE(pHttpReq);
      return OF_FAILURE;
    }
  }

  pThis = Httpd_Search_ActionTag((char *) pHttpReq->filename);

  if (!pThis)
  {
#ifdef HTTPD_DEBUG
    Httpd_Warning("Nobody registered for action tag '%s'\n",
                  pHttpReq->filename);
#endif
    HttpSendHtmlFile(pHttpReq,(unsigned char *)  "errscrn.htm",
                    (unsigned char *)  "This feature is not available in this release");
    HTTPDFREE(pHttpReq);
    return OF_SUCCESS;
  }

  CbFun = (HTTPD_CBFUN_ACTION) pThis->pCbFunct;
  HTTPD_ASSERT(!CbFun);
  if(pThis->ucPrivilage == HTTP_CBKFN_PRIV_ADMIN)
  {
    if(bConfig == FALSE)
    {

      /*For Last accessed xml url */
      if(!of_strcmp((char *) pHttpReq->filename,"xmlreq.igw"))
      {
        char aXMLBuf[HTTP_HTML_FILE_INBUFF_LEN +1];
        uint32_t ulBuflen=0;
        if(aPrevXmlUrlData[0] != '\0')
        {
          sprintf(aXMLBuf, "<?xml version=\"1.0\" encoding=\"UTF-8\"?><?xml-stylesheet type=\"text/xsl\" href=\"errscrn_en.xsl\"?> <body><Error_String>You do not have permission to submit the changes</Error_String><PrevXml>%s</PrevXml></body>",aPrevXmlUrlData);
          ulBuflen = of_strlen(aXMLBuf);
          of_strcpy((char *) pHttpReq->filename, "errscrn.xml");
          Httpd_SendXMLBuff(pHttpReq,aXMLBuf,ulBuflen);
        }
        else
        {
          HttpSendHtmlFile(pHttpReq, (unsigned char *) "errscrn.htm",
             (unsigned char *)  "You do not have permission to submit the changes");
        }
        of_memset(aXMLBuf,0,HTTP_HTML_FILE_INBUFF_LEN +1);
        of_memset(aPrevXmlUrlData,0,HTTP_FILENAME_LEN +1);
      }
      else
      {
        HttpSendHtmlFile(pHttpReq,(unsigned char *)  "errscrn.htm",
                  (unsigned char *)  "You dont have permission to submit the changes");
      }
      HTTPDFREE(pHttpReq);	  
      return OF_SUCCESS;
    }
  }

  if (HttpRequest_IsMultiPart(pHttpReq))
  {
    pVals = Httpd_ValAry_Alloc(pThis->pVarArray, pThis->VarCount, TRUE);

    if (!pVals)
    {
      Httpd_Warning("Unable To Allocate Memory For Value-Array\n");
      HTTPDFREE(pHttpReq);	  
      return OF_FAILURE;
    }

    if (Httpd_ValAry_FromMultiPart(HttpRequest_GetMPartData(pHttpReq),
                                   HttpRequest_GetMPartDataLen(pHttpReq),
                                   HttpRequest_GetMPartBoundary(pHttpReq),
                                   pThis->pVarArray, pThis->VarCount,
                                   pVals) != OF_SUCCESS)
    {
      Httpd_Error("Unable To Parse Multi-Part Post String\n");
     if( Httpd_ValAry_Free(pVals)!=OF_SUCCESS)
     {
          HttpDebug((" Httpd_ValAry_Free returned Failure \r\n"));	      
      }
      HTTPDFREE(pHttpReq);	  
      return OF_FAILURE;
    }
  }
  else
  {
    /**
     * Set the bytes written in the cookie record.
     */

    if(!((pClientReq->ulPageType == LOGINPAGEREQUEST) && 
        (pClientReq->ulConnType)))		         
    {	    
      if (HttpCookie_SetBytesWritten(pHttpReq->pCookieVal,
                                   0) != OF_SUCCESS)
      {
        HTTPDFREE(pHttpReq);
        return OF_FAILURE;
      }
    }      
    pVals = Httpd_ValAry_Alloc(pThis->pVarArray, pThis->VarCount, FALSE);

    if (!pVals)
    {
      Httpd_Warning("Unable To Allocate Memory For Value-Array\n");
      HTTPDFREE(pHttpReq);  	  
      return OF_FAILURE;
    }

    if (Httpd_ValAry_FromPostString((char *) pHttpReq->cPostString,
                                    pThis->pVarArray, pThis->VarCount,
                                    pVals) != OF_SUCCESS)
    {
      Httpd_Error("Unable To Parse PostString\n");
      if(Httpd_ValAry_Free(pVals)!=OF_SUCCESS)
      {
	    HttpDebug((" Httpd_ValAry_Free returned Failure \r\n"));
      }
      HTTPDFREE(pHttpReq); 	  
      return OF_FAILURE;
    }
  }

  iRet = CbFun(pHttpReq, pVals, pThis->VarCount);
  if(Httpd_ValAry_Free(pVals)!=OF_SUCCESS)
  {
    HttpDebug((" Httpd_ValAry_Free returned Failure \r\n"));
    HTTPDFREE(pHttpReq);
    return OF_FAILURE;
  }
  if (iRet == OF_SUCCESS)
  {
    HTTPDFREE(pHttpReq);
    return OF_SUCCESS;
  }
  else if(iRet == HTTP_DONOT_REMOVE_REQUEST)
  {
   		return HTTP_DONOT_SEND_RESPONCE;
  }
  else
  {
    Httpd_Warning("Action-Tag callback function(new) failed for '%s'\n",
                  pHttpReq->filename);
    HTTPDFREE(pHttpReq);
    return OF_FAILURE;
  }
  HTTPDFREE(pHttpReq);
  return OF_SUCCESS;
} /* Httpd_PostString_Process() */

/***********************************************************************
 * Function Name : Httpd_MPart_GetSubmitName                           *
 * Description   : Gives The next submit name in the multi-part data.  *
 * Input         : pInBuf      - Input Buffer                          *
 *                 ulLen       - Max. Len of submit(output) name       *
 * Output        : name_p       - Parsed Submit Name                    *
 * Return value  : Pointer To Start Of Data on success                 *
 *                 NULL on failure                                     *
 ***********************************************************************/


unsigned char *Httpd_MPart_GetSubmitName( unsigned char  *pInBuf,
                                     unsigned char  *name_p,
                                     uint32_t  ulLen )
{
  unsigned char  *pTmp=NULL, *pTmp1=NULL, *pCur=NULL, *pLine=NULL;
  bool   bGotFlag = FALSE;

  pCur = pInBuf;

  while (*pCur)
  {
    pLine = pCur;

    pCur = (unsigned char * )of_strstr((char *) pCur, (char *) CRLF);

    if (!pCur)
    {
      return NULL;
    }

    *pCur = '\0';

    pCur += of_strlen(CRLF);

    if (!of_strcmp((char *) pLine, "")) /* End-Of-Header */
    {
      if (bGotFlag)
      {
        return pCur;
      }
      else
      {
        return NULL;
      }
    }
    if (bGotFlag)
    {
      continue;
    }

    if (!of_strstr((char *) pLine, "Content-Disposition"))
    {
      continue;
    }

    pTmp = (unsigned char *) of_strstr((char *) pLine, "name=\"");

    if (!pTmp)
    {
      return NULL;
    }

    pTmp += of_strlen("name=\"");

    pTmp1 = (unsigned char *) of_strchr((char *) pTmp, '"');

    if (!pTmp1)
    {
      return NULL;
    }

    if ((pTmp1-pTmp) < ulLen)
    {
      ulLen = pTmp1-pTmp;
    }

    of_strncpy((char *) name_p, (char * )pTmp, ulLen);
    name_p[ulLen] = '\0';
    bGotFlag = TRUE;
  }

  return NULL;
} /* Httpd_MPart_GetSubmitName() */

/***********************************************************************
 * Function Name : Httpd_MemMem                                        *
 * Description   :                                                     *
 * Input         :                                                     *
 * Output        :                                                     *
 * Return value  :                                                     *
 ***********************************************************************/


void *Httpd_MemMem( void   *pSrc,
                      int32_t  lSrcBytes,
                      void   *pDst,
                      int32_t  lDstBytes )
{
  int32_t  ii=0;

  for (ii = 0; ii <= (lSrcBytes-lDstBytes); ii++)
  {
    if (memcmp((void*)((char*)pSrc+ii), pDst, lDstBytes) == 0)
    {
      return (void*)((char*)pSrc+ii);
    }
  }
  return NULL;
} /* Httpd_MemMem() */

/***********************************************************************
 * Function Name : Httpd_ValAry_FromMultiPart                          *
 * Description   : This is initializes the value array depending on    *
 *                 the posted values in multi-part string              *
 * Input         : pMpBuf      - Multi-Part Buffer                     *
 *                 pBoundary   - Boundary String The multi-Part String *
 *                 pVarAry     - Variable Array containing names        *
 *                 VarCount    - Count of variables                    *
 * Output        : pValAry     - Value Array                           *
 * Return value  : OF_SUCCESS on success                                *
 *                 OF_FAILURE on failure                                *
 ***********************************************************************/

 int32_t Httpd_ValAry_FromMultiPart( unsigned char          *pMpBuf,
                                    int32_t           lMpBytes,
                                    unsigned char          *pBoundary,
                                    Httpd_Variable_t  *pVarAry,
                                    uint32_t          VarCount,
                                    Httpd_Value_t     *pValAry )
{
  unsigned char   aName[HTTPD_LOCAL_NAME_LEN];
  unsigned char   ucBndStr[HTTPD_LOCAL_BIND_STRING_LEN];
  unsigned char   *pCur=NULL, *pBack=NULL;
  uint32_t   ulBndLen=0, ii=0;

  if ((!pMpBuf) ||
      (!pBoundary))
  {
    return OF_FAILURE;
  }

  if (VarCount == 0)
  {
    return OF_SUCCESS;
  }

  if ((!pVarAry) ||
      (!pValAry))
  {
    return OF_FAILURE;
  }

  if (of_strlen((char *) pBoundary) > 76)
  {
    return OF_FAILURE;
  }

  sprintf((char *) ucBndStr, "--%s", pBoundary);
  ulBndLen = of_strlen((char *) ucBndStr);

  pCur = pMpBuf;

  pBack = pCur;
  pCur = (unsigned char *) of_strstr((char *) pCur, CRLF);

  if (!pCur)
  {
    return OF_FAILURE;
  }

  *pCur = '\0';
  pCur += of_strlen(CRLF);

  if (of_strcmp((char *) pBack, (char *) ucBndStr))
  {
    return OF_FAILURE;
  }

  sprintf((char *) ucBndStr, "\r\n--%s", pBoundary);
  ulBndLen = of_strlen((char *) ucBndStr);

  while (pCur < pMpBuf+lMpBytes)
  {
    pBack = pCur;
    pCur = Httpd_MPart_GetSubmitName(pBack, aName, HTTPD_LOCAL_NAME_LEN - 1);

    if (!pCur)
    {
      return OF_FAILURE;
    }

    if (pCur >= pMpBuf+lMpBytes)
    {
      return OF_FAILURE;
    }

    for (ii = 0; ii < VarCount; ii ++)
    {
      if (!strcasecmp((char*)pVarAry[ii].name_p, (char *) aName))
      {
        pValAry[ii].value_p = (char *) pCur;
        break;
      }
    }

    pBack = pCur;

    pCur = Httpd_MemMem(pCur, (pMpBuf+lMpBytes-pCur), ucBndStr, ulBndLen);

    if (!pCur)
    {
      return OF_FAILURE;
    }

    if (ii < VarCount)
    {
      if (pValAry[ii].Size > (pCur - (unsigned char*)pValAry[ii].value_p))
      {
        pValAry[ii].Size = pCur - (unsigned char*)pValAry[ii].value_p;
      }

      pValAry[ii].value_p[pValAry[ii].Size] = '\0';
    }

    *(pCur) = '\0';

    pCur += of_strlen((char *) ucBndStr);

    if (!of_strncmp((char *) pCur, "--\r\n", 4))
    {
      break;
    }
    else if (!of_strncmp((char *) pCur, "\r\n", 2))
    {
      pCur += 2;
      continue;
    }
    else
    {
      return OF_FAILURE;
    }
  }
  return OF_SUCCESS;
} /* Httpd_ValAry_FromMultiPart() */

/***********************************************************************
 * Function Name : Httpd_ValAry_FromPostString                         *
 * Description   : This is initializes the value array depending on    *
 *                 the posted values                                   *
 * Input         : pPostString - Post String                           *
 *                 pVarAry     - Variable Array containing names        *
 *                 VarCount    - Count of variables                    *
 * Output        : pValAry     - Value Array                           *
 * Return value  : OF_SUCCESS on success                                *
 *                 OF_FAILURE on failure                                *
 ***********************************************************************/

 int32_t Httpd_ValAry_FromPostString( char           *pPostString,
                                                Httpd_Variable_t  *pVarAry,
                                                uint32_t          VarCount,
                                                Httpd_Value_t     *pValAry )
{
  char   *name_p=NULL, *value_p=NULL, *pStart=NULL;
  char   *pTemp=NULL, *pCopy=NULL, *pOpCode=NULL;
  uint32_t  ii=0;
  int16_t   RetVal = 0;


  HTTPD_ASSERT(!pPostString);
  HTTPD_ASSERT(!pVarAry);
  HTTPD_ASSERT(!pValAry);

#ifdef HTTPD_DEBUG
  Httpd_Print("(New)Asked For %ld Elements\n", VarCount);
#endif /* HTTPD_DEBUG */

  pCopy = of_strdup(pPostString); /* We Should Not Corrupt pPostString.*/
  pStart = pCopy;

  while (TRUE)
  {
    name_p = pStart;  /* Points To Start Of Name*/

    pTemp = of_strchr(pStart, '=');

    if (!pTemp)
    {
      Httpd_Error("Invalid Post String\n");
      RetVal = -1; /* return Failure.*/
      break;
    }
    /* Found  '='*/
    *pTemp = '\0';
    value_p = pTemp+1;  /* Points To Start of Value*/
    pStart = value_p;
    pTemp = of_strchr(pStart, '&');

    if (pTemp)
    {
      *pTemp = '\0';
      pStart = pTemp+1;
    }

    /* So We Have a Set Here.*/
    if (Httpd_PostString_Convert((unsigned char *) name_p) < 0)
    {
      Httpd_Error("Httpd_PostString_Convert Failed For Name\n");
      RetVal = -1;
      break;
    }

    if((of_strcmp(name_p, "ucmreq") == 0) &&  (pOpCode && ((of_strcmp(pOpCode, "ADD") == 0)||(of_strcmp(pOpCode, "EDIT") == 0))) )
    {
	/*skip convertion on add/edit. Will be taken up while creating NV pairs.*/
    }
    else
    {
      if (Httpd_PostString_Convert((unsigned char *) value_p) < 0)
      {
        Httpd_Error("Httpd_PostString_Convert Failed For Value\n");
        RetVal = -1;
        break;
      }
    }
    for (ii = 0; ii < VarCount; ii ++)
    {
      if (!strcasecmp((char*)pVarAry[ii].name_p, name_p))
      {
        break;
      }
      if(strcasecmp(name_p, "opcode") == 0)
      {
	pOpCode = value_p;
      }
    }

    if (ii < VarCount)
    {
      HTTPD_ASSERT(pVarAry[ii].Size != pValAry[ii].Size);
      Httpd_Value_CopyStr(&pValAry[ii], value_p);
#ifdef HTTPD_DEBUG
      Httpd_Print("'%s'\t\t'%s'\n", pVarAry[ii].name_p,
                  pValAry[ii].value_p);
#endif /* HTTPD_DEBUG */
    }
    else
    {
      /**
       * Discard This Name=Value Pair as user didn't ask 4 it. But Notify.
       */
      /* placed debug statements under macro -- syam */
#ifdef HTTPD_DEBUG
      Httpd_Print("Unknown Name '%s' is Encountered in the Post String\n",
                  name_p);
      Httpd_Print("Check the Variable Array, That doesn't contain the above name\n");
#endif /* HTTPD_DEBUG */
    }

    if (!pTemp)
    {
      RetVal = 0;
      break; /* If This Was the Last element.*/
    }
  }

  of_free(pCopy);
  return RetVal;
} /* Httpd_ValAry_FromPostString() */

/***********************************************************************
 * Function Name : Httpd_PostString_Compile                            *
 * Description   : This is utility function which can be used by new
                   action handlers to get the values, as per data type,
                   of registered tags from post string                 *
 * Input         : pPostString - Post String                           *
 *                 pVarAry     - Variable Value field Array, each
                                 element containing name, length, value     *
 *                 pArrLen    - Length of array pointed by pVarAry     *
 * Output        : pArrLen     - Number of elements in pVarAry filled
                                 properly                             *
 * Return value  : OF_SUCCESS on success                                *
 *                 OF_FAILURE on failure                                *
 ***********************************************************************/

int32_t Httpd_PostString_Compile( char                 *pPostString,
                                  Httpd_Tag_Var_values_t  *pVarAry,
                                  uint32_t                *pArrLen )
{
  char                 *name_p=NULL, *value_p=NULL, *pStart=NULL;
  char                 *pTemp=NULL, *pCopy=NULL;
  uint32_t                ii=0;
  int16_t                 RetVal = 0;
  Httpd_Tag_Var_values_t  *pTempVarAry=NULL;

  HTTPD_ASSERT(!pPostString);
  HTTPD_ASSERT(!pVarAry);

  pCopy = of_strdup(pPostString); /* We Should Not Corrupt pPostString.*/
  if(!pCopy)
  {
      Httpd_Error("Httpd_PostString_Compile::NULL value assigned to  pCopy\n");
      return OF_FAILURE;
  }
  
  pStart = pCopy;

  /**
   * Initialize all elements to zero.
   */
  for (ii = 0; ii < *pArrLen; ii++)
  {
    of_memset(pVarAry[ii].value_p, 0, pVarAry[ii].ulLen);
  }

  while (TRUE)
  {
    name_p = pStart;  /* Points To Start Of Name*/

    pTemp = of_strchr(pStart, '=');

    if (!pTemp)
    {
      Httpd_Error("Invalid Post String\n");
      RetVal = -1; /* return Failure.*/
      break;
    }
    /* Found  '='*/
    *pTemp = '\0';
    value_p = pTemp+1;  /* Points To Start of Value*/
    pStart = value_p;
    pTemp = of_strchr(pStart, '&');

    if (pTemp)
    {
      *pTemp = '\0';
      pStart = pTemp+1;
    }

    /* So We Have a Set Here.*/
    if (Httpd_PostString_Convert((unsigned char *) name_p) < 0)
    {
      Httpd_Error("Httpd_PostString_Compile Failed For Name\n");
      RetVal = -1;
      break;
    }

    if (Httpd_PostString_Convert((unsigned char *) value_p) < 0)
    {
      Httpd_Error("Httpd_PostString_Compile Failed For Value\n");
      RetVal = -1;
      break;
    }

    pTempVarAry = pVarAry;

    for (ii = 0; ii < *pArrLen; ii ++)
    {
      if (!strcasecmp((char*)pTempVarAry->name_p, name_p))
      {
        break;
      }

      pTempVarAry++;
    }

    if (ii < *pArrLen)
    {
      /**
       * If match found, then check whether buffer length is sufficient or
       * not depending on type.
       */
      switch (pTempVarAry->ulDataType)
      {
      case HTTP_DATA_TYPE_CHAR:
        {
          if (pTempVarAry->ulLen < sizeof(unsigned char))
          {
            Httpd_Error("Httpd_PostString_Compile: Insufficient buffer len\n");
          }
          else
          {
            pTempVarAry->value_p[0] = *value_p;
          }
          break;
        }

      case HTTP_DATA_TYPE_STRING:
        {
          if (pTempVarAry->ulLen < of_strlen(value_p))
          {
            Httpd_Error("Httpd_PostString_Compile: Insufficient buffer len\n");
          }
          else
          {
            of_strncpy((char *) pTempVarAry->value_p, value_p, of_strlen((char *) value_p));
          }
          break;
        }

      case HTTP_DATA_TYPE_BYTE:
        {
          if (pTempVarAry->ulLen < sizeof(uint8_t))
          {
            Httpd_Error("Httpd_PostString_Compile: Insufficient buffer len\n");
          }
          else
          {
            *(uint8_t*)(pTempVarAry->value_p) = (uint8_t)(cm_atoi(value_p) & 0xff);
          }
          break;
        }

      case HTTP_DATA_TYPE_SHORT:
        {
          if (pTempVarAry->ulLen < sizeof(uint16_t))
          {
            Httpd_Error("Httpd_PostString_Compile: Insufficient buffer len\n");
          }
          else
          {
            *(uint16_t*)(pTempVarAry->value_p) = (uint16_t)(cm_atoi(value_p) & 0xffff);
          }
          break;
        }

      case HTTP_DATA_TYPE_INT:
        {
          if (pTempVarAry->ulLen < sizeof(uint32_t))
          {
            Httpd_Error("Httpd_PostString_Compile: Insufficient buffer len\n");
          }
          else
          {
            *(uint32_t*)(pTempVarAry->value_p) = (uint32_t)cm_atoi(value_p);
          }

          break;
        }

      default:
        {
          Httpd_Error("Httpd_PostString_Compile: Unknown data type\n");
          break;
        }
      }
    }
    else
    {
      /**
       * Discard This Name=Value Pair as user didn't ask 4 it. But Notify.
       */
      /**
       * Placed debug statements under macro -- syam.
       */
#ifdef HTTPD_DEBUG
      Httpd_Print("Unknown Name '%s' is Encountered in the Post String\n",
                  name_p);
      Httpd_Print("Check The Variable Array, that doesn't contain the ");
      Httpd_Print("above name\n");
#endif /* HTTPD_DEBUG */
    }

    if (!pTemp)
    {
      RetVal = 0;
      break; /* If This Was the Last element.*/
    }
  }

  of_free(pCopy);
  return RetVal;
} /* Httpd_PostString_Compile() */

/***********************************************************************
 * Function Name : Httpd_FillGetValArray                               *
 * Description   : This is utility function to fill the general GET tags
                   with the specified values. This function can be used
                   by any general HTTP GET handler.
 * Input         : pReq -  HTTP REquest Info                          *
 *                 pVarArray - Array, where each element contains
                                   Name of General GET tags registered
                                   for HTML page
                   value_ps - Array, where each element contains the
                             Name and corresponding Value to be filled
                             of GET tag
 *                 ulValCount   - Length of pVarArray, value_ps and
                                  pValArray arrays.*

 * Output        : pValArray     - Value array filled and ready to be
                                   displayed                           *
 * Return value  : OF_SUCCESS on success                                *
 *                 OF_FAILURE on failure                                *
 ***********************************************************************/
 
int32_t Httpd_FillGetValArray( HttpRequest             *pReq,
                               Httpd_Variable_t        *pVarArray,
                               Httpd_Tag_Var_values_t  *value_ps,
                               uint32_t                ulValCount,
                               Httpd_Value_t           *pValArray )
{
  uint32_t  ii=0, jj=0;
  char   *pTemp1=NULL, *pTemp2=NULL, *pTemp3=NULL, bSelected = FALSE;

  HTTPD_ASSERT(!pVarArray);
  HTTPD_ASSERT(!pValArray);
  HTTPD_ASSERT(!value_ps);

  for (ii = 0; ii < ulValCount; ii++)
  {
    /**
     * Find the location of get tag in registration.
     */
    for (jj = 0; jj < ulValCount; jj++)
    {
      if ((of_strcmp(pVarArray[jj].name_p, (char *) value_ps[ii].name_p)) == 0)
      {
        break;
      }
    }

    if (jj >= ulValCount)
    {
      continue;
    }

    switch (value_ps[ii].ulDataType)
    {
    case HTTP_DATA_TYPE_CHAR:

      if (value_ps[ii].value_p == NULL)
      {
        of_strcpy(pValArray[jj].value_p, "");
        break;
      }
      *((char *)(pValArray[jj].value_p)) = *((unsigned char*)(value_ps[ii].value_p));

      break;

    case HTTP_DATA_TYPE_STRING:
      {
        if (value_ps[ii].ulHtmlType == HTTP_HTML_TYPE_STRING)
        {
          if (value_ps[ii].value_p)
          {
            of_strncpy(pValArray[jj].value_p, (char *) value_ps[ii].value_p,
                      pValArray[jj].Size);
          }
          else
          {
            of_strcpy(pValArray[jj].value_p,"");
          }
        }
        else if (value_ps[ii].ulHtmlType == HTTP_HTML_TYPE_OPTION)
        {
          pTemp1 = (char *) pValArray[jj].value_p;
          pTemp2 = (char *) value_ps[ii].value_p;

          if (pTemp2 == NULL)
          {
            break;
          }
          pTemp3 = of_strstr(pTemp2, ",");

          /**
           * Null terminate the string, i.e., Initialize it.
           */
          *pTemp1 = 0;

          do
          {
            /**
             * Replace ',' with null character.
             */
            if (pTemp3)
            {
              *pTemp3 = 0;
              pTemp3++;
            }

            /**
             * Check whether the length in buffer pointed by pTemp1 can
             * hold this option value.
             */
            if (bSelected)
            {
              if ((of_strlen(pTemp1) + of_strlen(pTemp2) +
                   of_strlen("<option></option>")) >= pValArray[jj].Size)
              {
                /**
                 * Instead of returning error, should we skip filling option
                 * values TBD ??.
                 */
                return OF_FAILURE;
              }
            }
            else
            {
              if ((of_strlen(pTemp1) + of_strlen(pTemp2) +
                   of_strlen("<option selected></option>")) >=
                       pValArray[jj].Size)
              {
                return OF_FAILURE;
              }
            }

            if ((!value_ps[ii].pVarSpecificOption) ||
                (bSelected) ||
                (of_strcmp(pTemp2, value_ps[ii].pVarSpecificOption)))
            {
              of_strcat(pTemp1, "<option>");
            }
            else
            {
              of_strcat(pTemp1, "<option selected>");
              bSelected = TRUE;
            }
            of_strcat(pTemp1, pTemp2);
            of_strcat(pTemp1, "</option>");

            /**
             * Move pTemp2 to next field.
             */
            pTemp2 = pTemp3;

            /**
             * Move pTemp3 to next delimiter.
             */
            if (pTemp2)
            {
              pTemp3 = of_strstr(pTemp2, ",");
            }
          } while (pTemp2);
        }
        else
        {
          /**
           * This type of data type is not valid for HTML option type,
           * caller need to pass only string data type, with ',' as
           * delimiter.
           */
          return OF_FAILURE;
        }

        break;
      }

    case HTTP_DATA_TYPE_BYTE:
    case HTTP_DATA_TYPE_SHORT:
    case HTTP_DATA_TYPE_INT:
      {
        uint32_t  ulVal = 0;

        if (value_ps[ii].value_p == NULL)
        {
          of_strcpy(pValArray[jj].value_p, "");
          break;
        }

        /**
         * check the lengths.
         */
        if ((value_ps[ii].ulDataType == HTTP_DATA_TYPE_BYTE) &&
            (pValArray[jj].Size < sizeof(uint8_t)))
        {
          return OF_FAILURE;
        }

        if ((value_ps[ii].ulDataType == HTTP_DATA_TYPE_SHORT) &&
            (pValArray[jj].Size < sizeof(uint16_t)))
        {
          return OF_FAILURE;
        }

        if ((value_ps[ii].ulDataType == HTTP_DATA_TYPE_INT) &&
            (pValArray[jj].Size < sizeof(uint32_t)))
        {
          return OF_FAILURE;
        }

        if (value_ps[ii].ulDataType == HTTP_DATA_TYPE_BYTE)
        {
          ulVal = *(value_ps[ii].value_p);
        }
        else if (value_ps[ii].ulDataType == HTTP_DATA_TYPE_SHORT)
        {
          ulVal = *((uint16_t*)(value_ps[ii].value_p));
        }
        else
        {
          ulVal = *((uint32_t*)(value_ps[ii].value_p));
        }

        if ((value_ps[ii].ulHtmlType == HTTP_HTML_TYPE_RADIO) ||
            (value_ps[ii].ulHtmlType == HTTP_HTML_TYPE_CHKBOX))
        {
          if (ulVal == FALSE)
          {
            of_strcpy(pValArray[jj].value_p, "");
          }
          else
          {
            of_strcpy(pValArray[jj].value_p, "checked");
          }

          if ((value_ps[ii].pVarSpecificOption) &&
              (*(value_ps[ii].pVarSpecificOption) == FALSE))
          {
            of_strcat(pValArray[jj].value_p, " disabled");
          }
        }
        else if (value_ps[ii].ulHtmlType == HTTP_HTML_TYPE_DISPLAY)
        {
          if (ulVal == FALSE)
          {
            of_strcpy(pValArray[jj].value_p, "display:none");
          }
          else
          {
            of_strcpy(pValArray[jj].value_p, "display:block");
          }
        }
        else if (value_ps[ii].ulHtmlType == HTTP_HTML_TYPE_STRING)
        {
          sprintf(pValArray[jj].value_p, "%u", ulVal);
        }
        else
        {
          /**
           * This type of data type is not valid for HTML option type,
           * caller need to pass only string data type, with ',' as
           * delimiter.
           */
          return OF_FAILURE;
        }
        break;
      }

      default:
      {
        break;
      }
    }
  }
  return OF_SUCCESS;
} /* Httpd_FillGetValArray() */

/***********************************************************************
 * Function Name : HttpSendHtmlFile                                    *
 * Description   : This sends the given HTML page to the browser       *
 * Input         : pHttpReq  - Http Request Pointer                    *
 *                 pFileName - Name of the HTML file                   *
 *                 pPostValue- Application Data to be sent to callbacks*
 * Output        :                                                     *
 * Return value  : OF_SUCCESS on success                                *
 *                 OF_FAILURE on failure                                *
 ***********************************************************************/

int16_t HttpSendHtmlFile( HttpRequest  *pHttpReq,
                          unsigned char     *pFileName,
                          unsigned char     *pPostValue )
{
  uint32_t  ulAppDataLen=0;
  HttpGlobals_t *pHttpg=NULL;

   if(!pHttpReq)
  {
      HttpDebug(("pHttpReq is NULL\r\n"));
      return OF_FAILURE;
  }  
  if(!pFileName)
  {
       HttpDebug(("pFileName is NULL\r\n"));
       return OF_FAILURE;
  }
  pHttpg=(HttpGlobals_t *)pHttpReq->pGlobalData;
  if(!pHttpg)
 {
    Httpd_Error("pHttpg is NULL\r\n");
    return OF_FAILURE;
 }
  ulAppDataLen=pHttpg->pHttpGlobalConfig->ulAppDataLength;
  if (pPostValue != NULL)
  {
    if (of_strlen((char *) pPostValue) >= ulAppDataLen)
    {
      return OF_FAILURE;
    }

    of_strcpy((char *) pHttpReq->pAppData, (char *) pPostValue);
  }
  else
  {
    pHttpReq->pAppData[0] = '\0';
  }

  of_strcpy((char *) pHttpReq->filename, (char *) pFileName);

  return HttpRequest_SendFile(pHttpg,pHttpReq);
} /* HttpSendHtmlFile() */

/***********************************************************************
 * Function Name : Httpd_Send_PlainString                              *
 * Description   : This sends a plain string to the browser            *
 * Input         : pRequest  - Http Request Pointer                    *
 *                 pString   - String to be sent                       *
 * Output        :                                                     *
 * Return value  : OF_SUCCESS on success                                *
 *                 OF_FAILURE on failure                                *
 ***********************************************************************/

int32_t Httpd_Send_PlainString( HttpRequest  *pRequest,
                                char      *pString )
{
  uint32_t   uiLength=0;

  HTTPD_ASSERT(!pRequest);
  HTTPD_ASSERT(!pString);

  uiLength = of_strlen(pString);

  return Httpd_Send_PlainBuf(pRequest, pString, uiLength);
} /* Httpd_Send_PlainString() */

/***********************************************************************
 * Function Name : Httpd_Send_PlainBuf                                 *
 * Description   : This sends a plain buffer to the browser            *
 * Input         : pRequest  - Http Request Pointer                    *
 *                 pHtmlBuf  - Buffer to be sent                       *
 *                 uiLength  - Number of bytes to send                 *
 * Output        :                                                     *
 * Return value  : OF_SUCCESS on success                                *
 *                 OF_FAILURE on failure                                *
 ***********************************************************************/

int32_t Httpd_Send_PlainBuf( HttpRequest  *pRequest,
                             char      *pHtmlBuf,
                             uint32_t     uiLength )
{
  HttpGlobals_t *pHttpg=NULL;

  pHttpg=(HttpGlobals_t *)pRequest->pGlobalData;
  if(!pHtmlBuf)
  {
    Httpd_Error("pHtmlBuf is NULL\r\n");
    return OF_FAILURE;
  }
  if(!pHttpg)
 {
    Httpd_Error("pHttpg is NULL\r\n");
    return OF_FAILURE;
 }
  if (Httpd_Send(pHttpg,pRequest, pHtmlBuf, uiLength) == uiLength)
  {
    return OF_SUCCESS;
  }
  else
  {
    return OF_FAILURE;
  }
} /* Httpd_Send_PlainBuf() */

/***********************************************************************
 * Function Name : Httpd_Send_DynamicBuf                               *
 * Description   : This sends a dynamic buffer to the browser          *
 *                 A dynamic buffer can contain http-variables but not *
 *                 tags. All variables are replaced with their values  *
 *                 Typically this function is called from table tag    *
 *                 callback function                                   *
 * Input         : pRequest  - Http Request Pointer                    *
 *                 pHtmlBuf  - Buffer to be sent                       *
 *                 uiLength  - Number of bytes to send                 *
 *                 VarCount  - Count of variables                      *
 *                 pVarAry   - Variable array                          *
 *                 pValAry   - Value array                             *
 * Output        :                                                     *
 * Return value  : OF_SUCCESS on success                                *
 *                 OF_FAILURE on failure                                *
 ***********************************************************************/

int32_t Httpd_Send_DynamicBuf( HttpRequest       *pRequest,
                               char           *pHtmlBuf,
                               uint32_t          uiLength,
                               uint32_t          VarCount,
                               Httpd_Variable_t  *pVarAry,
                               Httpd_Value_t     *pValAry )
{
  char  *pCur=NULL, *pPrev=NULL, *pTemp=NULL;
  int32_t  RetVal=0, ii=0;
  char  cVariable[HTTPD_VARIABLE_MAX_LEN+1];

  HTTPD_ASSERT(!pRequest);
  HTTPD_ASSERT(!pHtmlBuf);

  if ((VarCount == 0) ||
      (!pVarAry) ||
      (!pValAry))
  {
    return Httpd_Send_PlainBuf(pRequest, pHtmlBuf, uiLength);
  }

  HTTPD_ASSERT(!pVarAry);
  HTTPD_ASSERT(!pValAry);

  pCur    = pHtmlBuf;

  while (pCur < (pHtmlBuf+uiLength))
  {
    pPrev = pCur;
    pCur  = Httpd_Variable_FromBuffer(pCur, (pHtmlBuf - pCur + uiLength),
                                      cVariable, HTTPD_VARIABLE_MAX_LEN,
                                      &pTemp);

    RetVal = Httpd_Send_PlainBuf(pRequest, pPrev, (pTemp - pPrev));

    if (RetVal < 0)
    {
      Httpd_Error("Unable To Send The Data\n");
      return OF_FAILURE;
    }
    if (!pCur)  /* End Of Parsing For Data Tags */
    {
      return OF_SUCCESS;
    }

    /**
     * We always receive  MY_NAME  when that finds [IGW_MY_NAME].
     */

    for (ii = 0; ii < VarCount; ii++)
    {
      if (!of_strcmp(pVarAry[ii].name_p, cVariable))
      {
        break;
      }
    }

    if (ii >= 0 && ii < VarCount)
    {
      RetVal = Httpd_Send_PlainString(pRequest, pValAry[ii].value_p);

      if (RetVal < 0)
      {
        return OF_FAILURE;
      }
    }
    else
    {
#ifdef HTTPD_DEBUG
      Httpd_Print("Unknown Variable '%s', Ignoring...\n", cVariable);
      Httpd_Error("Bad Registered Variable Array.\n");
#endif /* HTTPD_DEBUG */
    }
  }
  return OF_FAILURE; /* Never Reached */
} /* Httpd_Send_DynamicBuf() */

/***********************************************************************
 * Function Name : Httpd_Send_DynamicString                            *
 * Description   : See Httpd_Send_DynamicBuf                           *
 * Input         : pRequest  - Http Request Pointer                    *
 *                 pHtmlBuf  - Buffer to be sent                       *
 *                 uiLength  - Number of bytes to send                 *
 *                 pVarAry   - Variable array                          *
 *                 pValAry   - Value array                             *
 * Output        :                                                     *
 * Return value  : OF_SUCCESS on success                                *
 *                 OF_FAILURE on failure                                *
 ***********************************************************************/

int32_t Httpd_Send_DynamicString( HttpRequest       *pRequest,
                                  char           *pHtmlBuf,
                                  uint32_t          VarCount,
                                  Httpd_Variable_t  *pVarAry,
                                  Httpd_Value_t     *pValAry )
{
  return Httpd_Send_DynamicBuf(pRequest, pHtmlBuf, of_strlen(pHtmlBuf),
                               VarCount, pVarAry, pValAry);
} /* Httpd_Send_DynamicString() */

/***********************************************************************
 * Function Name : Httpd_Send_HyperString                              *
 * Description   : This sends a hyper string to the browser.           *
 *                 A hyper string can contain multiple tag regions.    *
 *                 This calls all callback functions of the tags       *
 *                 present in this buffer and send the values          *
 * Input         : pRequest  - Http Request Pointer                    *
 *                 pHtmlBuf  - Hyper String to be sent                 *
 * Output        :                                                     *
 * Return value  : OF_SUCCESS on success                                *
 *                 OF_FAILURE on failure                                *
 ***********************************************************************/

int32_t Httpd_Send_HyperString( HttpRequest  *pRequest,
                                char      *pHtmlBuf )
{
  return Httpd_Send_HyperStringEx(pRequest, pHtmlBuf, TRUE);
} /* Httpd_Send_HyperString() */

/***********************************************************************
 * Function Name : Httpd_Send_HyperString                              *
 * Description   : This sends a hyper string to the browser.           *
 *                 A hyper string can contain multiple tag regions.    *
 *                 This calls all callback functions of the tags       *
 *                 present in this buffer and send the values.         *
 *                 This manipulates the input string for better        *
 *                 performance if bReadOnly id not true                *
 * Input         : pRequest  - Http Request Pointer                    *
 *                 pHtmlBuf  - Hyper String to be sent                 *
 *                 bReadOnly - Is this a read-only buffer              *
 * Output        :                                                     *
 * Return value  : OF_SUCCESS on success                                *
 *                 OF_FAILURE on failure                                *
 ***********************************************************************/

int32_t Httpd_Send_HyperStringEx( HttpRequest  *pRequest,
                                  char      *pHtmlBuf,
                                  bool      bReadOnly )
{
  Httpd_TagType_t    *pThis=NULL;
  Httpd_HashEntry_t  *pHEntry=NULL;
  Httpd_TTEntry_t    TTEntry;
  char            TagName[ HTTPD_TAG_MAX_LEN + 1 ];
  int32_t            PreBufLen=0, TagBufLen=0;
  char            *TagBufStart=NULL;
  char            *pPrev=NULL, *pCur=NULL;
  char            *pTagBuffer = NULL;

  pCur = pHtmlBuf; 
  Httpd_If_Process(pRequest, (unsigned char *) pHtmlBuf);
 			 /*If Condition support*/
  while (1)
  {
    pPrev = pCur;
    pCur  = Httpd_TagRegion_Capture(pCur, TagName, HTTPD_TAG_MAX_LEN + 1,
                                    &pThis, &PreBufLen, &TagBufLen,
                                    &TagBufStart);
    if (!pCur)
    {
      return Httpd_Send_PlainBuf(pRequest, pPrev, PreBufLen);
    }

    if (Httpd_Send_PlainBuf(pRequest, pPrev, PreBufLen) < 0)
    {
      return OF_FAILURE;
    }

    HTTPD_ASSERT(!pThis);

    if (bReadOnly)
    {
      pTagBuffer = (char *) of_malloc(TagBufLen+1);

      if (!pTagBuffer)
      {
        return OF_FAILURE;
      }

      of_strncpy(pTagBuffer, TagBufStart, TagBufLen);
      pTagBuffer[TagBufLen] = '\0';
    }
    else
    {
      /**
       * Set '\0' in place of '<' in  "<IGW_END_...>..."
       */
      TagBufStart[TagBufLen] = '\0';
      pTagBuffer = TagBufStart;
    }

    pHEntry = Httpd_HashTable_Search(&pThis->HTable, TagName);

    if (!pHEntry)
    {
#ifdef HTTPD_DEBUG
      Httpd_Print("'%s' is not registered by anyone\n", TagName);
#endif /* HTTPD_DEBUG */
       /* Just Ignore This, and Proceed With Next Tag*/
    }
    else
    {
      /**
       * Don't dare to send HashEntry pointer.
       */
      TTEntry.pTagName  = pHEntry->pTagName;
      TTEntry.pCbFunct  = pHEntry->pCbFunct;
      TTEntry.VarCount  = pHEntry->VarCount;
      TTEntry.pVarArray = pHEntry->pVarArray;

      if (pThis->HandlerFn(pRequest, &TTEntry, pTagBuffer) != OF_SUCCESS)
      {
        return OF_FAILURE;
      }
    }
    if (bReadOnly)
    {
      /**
       * Free the previously allocated buffer.
       */
      of_free(pTagBuffer);
    }
  }

  return OF_SUCCESS; /* Never Reached */
} /* Httpd_Send_HyperStringEx() */

/***********************************************************************
 * Function Name : Httpd_TagRegion_Capture                             *
 * Description   : This marks the first tag region and initializes the  *
 *                 output pointers with the boundaries.                *
 * Input         : pBuf   - Input buffer to parse                      *
 * Output        : cTagName - Tag Name is copied to this               *
 *                 TagLen   -                                          *
 *                 pTagType - Initialized with the tag type             *
 *                 PreBufLen - Length from start of string to start of *
 *                             tag name.                               *
 *                 TagBufLen - Buffer length in tag-region             *
 *                 TagBufStart - start of tag-buffer(dynamic buffer)   *
 * Return value  : pointer to (end of tag+1) on success                *
 *                 NULL on error                                       *
 ***********************************************************************/

 char *Httpd_TagRegion_Capture( char          *pBuf,
                                  char          *cTagName,
                                  uint32_t         TagLen,
                                  Httpd_TagType_t  **pTagType,
                                  int32_t          *PreBufLen,
                                  int32_t          *TagBufLen,
                                  char          **TagBufStart )
{
  char          *pTemp=NULL, *pTemp1=NULL, *pTemp2=NULL;
  char          tmpBuf[120], tmpBuf1[32];
  int32_t          tmpLen=0;
  Httpd_TagType_t  *pThis=NULL;

  HTTPD_ASSERT(!pBuf);
  HTTPD_ASSERT(!cTagName);
  HTTPD_ASSERT(!pTagType);

  /*  <IGW_*/
  sprintf(tmpBuf, "%c%s%c", HTTPD_TAG_BEGIN_CHR, HTTPD_TAG_KEY_STR,
            HTTPD_TAG_SEP_CHR);

  tmpLen = of_strlen(tmpBuf);

  pTemp =  of_strstr(pBuf, tmpBuf);

  if (!pTemp)
  {
    Httpd_Heavy_Debug("No Tags Found\n");
    goto ErrFailed;
  }

  pTemp1 = of_strchr(pTemp + tmpLen, HTTPD_TAG_SEP_CHR);
  if (!pTemp1)
  {
    Httpd_Error("Ouch! Unable To Find Second Separator\n");
    goto ErrFailed;
  }

  pTemp2 = of_strchr(pTemp1, HTTPD_TAG_END_CHR);  /* Search For '>'*/
  if (!pTemp2)
  {
    Httpd_Error("Ouch! Unable To Find End '>' ??\n");
    goto ErrFailed;
  }

  /**
   *  Now We Have All The Three Pointers
   *
   *     pTemp         pTemp1  ___  pTemp2
   *        |            |    |
   *  ......<IGW_TABSTART_JUNK>.....
   *     -->|   |<--
   *         tmpLen
   *
   *  Comments are written referring to the above example.
   *  But it can be  for general tag also
   */

  /**
   * Copy  TABSTART to tmpBuf1.
   */
  of_strncpy(tmpBuf1, pTemp+tmpLen, pTemp1-pTemp-tmpLen);
  tmpBuf1[ pTemp1-pTemp-tmpLen ] = '\0';

  pThis = Httpd_TagType_SearchByBeginKey(tmpBuf1);
  if (!pThis)
  {
#ifdef HTTPD_DEBUG
    Httpd_Error("Unknown Tag Type\n");
    Httpd_Print("Nobody is registered to handle '%s' Tag Type\n",
                tmpBuf1);
#endif /* HTTPD_DEBUG */
    goto ErrFailed;
  }

  *PreBufLen = (pTemp - pBuf);

  /* Copy  JUNK to cTagName */
  of_strncpy(cTagName, pTemp1+1, pTemp2-pTemp1-1);
  cTagName[ pTemp2-pTemp1-1 ] = '\0';
  *pTagType  = pThis;
  *TagBufStart = pTemp2+1;

  /* Construct Second Search String,  <IGW_TABEND_JUNK>*/
  sprintf(tmpBuf, "%c%s%c%s%c%s%c", HTTPD_TAG_BEGIN_CHR,
            HTTPD_TAG_KEY_STR, HTTPD_TAG_SEP_CHR, pThis->pEndKey,
            HTTPD_TAG_SEP_CHR, cTagName, HTTPD_TAG_END_CHR);

  tmpLen = of_strlen(tmpBuf);

  pTemp = of_strstr(pTemp2, tmpBuf);

  if (!pTemp)
  {
#ifdef HTTPD_DEBUG
    Httpd_Error("There is no End For a Tag-Type\n");
    Httpd_Print("Bad Buffer/HTML File.  '%s' has no end\n", cTagName);
#endif /* HTTPD_DEBUG */
    goto ErrFailed;
  }

  *TagBufLen = (pTemp-pTemp2-1);
  return  (pTemp + tmpLen);

ErrFailed:

  if (TagLen > 0)
  {
    cTagName[0] = '\0';
  }

  *pTagType    = NULL;
  *PreBufLen   = of_strlen(pBuf);
  *TagBufLen   = 0;
  *TagBufStart = NULL;

  return NULL;
} /* Httpd_TagRegion_Capture() */

/***********************************************************************
 * Function Name : Httpd_Variable_FromBuffer                           *
 * Description   : Reads a Variable(Data-Tag) From the pBuf(dyn.buf)   *
 *                 into cVariable.                                     *
 * Input         : pBuf     - Input buffer to parse for variable       *
 *                 uiLength - Length of the buffer                     *
 * Output        : cVariable - Variable Name                           *
 *                 VarLen    -                                         *
 *                 cVarStart - Initialized with the tag type            *
 * Return value  : If Successful, It Copies Variable Name in to        *
 *                 cVariable, Sets VarStart to Point. to The Start of  *
 *                 Variable Name in pBuf, and Returns a pointer that   *
 *                 points to the next character after the              *
 *                 end-of-variable name in pBuf. If it can't find any  *
 *                 Variable Name, cVarStart will be pointing the end   *
 *                 of buffer and returns NULL                          *
 ***********************************************************************/

 char *Httpd_Variable_FromBuffer( char   *pBuf,
                                    uint32_t  uiLength,
                                    char   *cVariable,
                                    uint32_t  VarLen,
                                    char   **cVarStart )
{
  char  *pCur = NULL, *pTemp=NULL;
  int32_t  tmpLen=0;

  HTTPD_ASSERT(!pBuf);
  HTTPD_ASSERT(!cVariable);
  HTTPD_ASSERT(!cVarStart);

  pCur = pBuf;

  tmpLen = of_strlen(HTTPD_VAR_KEY_STR);

  while (pCur < (pBuf+uiLength))
  {
    if ((*pCur == HTTPD_VAR_BEGIN_CHR) &&
        (pCur+tmpLen+1 < (pBuf+uiLength)) &&
        !of_strncmp(pCur+1, HTTPD_VAR_KEY_STR, tmpLen))
    {
      *cVarStart = pCur;

      /**
       * Check for Separator '_'
       */
      if (*(pCur+tmpLen+1) != HTTPD_VAR_SEP_CHR)
      {
        pCur = pCur+tmpLen+2;
        continue;
      }

      /*       <    IGW     _    */
      pCur +=  1 + tmpLen + 1;
      pTemp = pCur;

      while ((pTemp < (pBuf+uiLength)) &&
             (*pTemp != HTTPD_VAR_END_CHR))
      {
        pTemp++;
      }

      if (pTemp >= (pBuf+uiLength))
      {
        Httpd_Error("Oops! Bad HTML File, Variable Doesn't End With ']'\n");

        /**
         * Reset cVarStart to the end of buffer and return NULL.
         */
        *cVarStart = pBuf+uiLength;
        return NULL;
      }

      tmpLen = (((pTemp-pCur) > VarLen)? VarLen : (pTemp-pCur));

      of_strncpy(cVariable, pCur, tmpLen);
      cVariable[ tmpLen ] = '\0';
      return pTemp + 1;
    }
    pCur++;
  }
  *cVarStart = pBuf+uiLength;
  return NULL;
} /* Httpd_Variable_FromBuffer() */

/***********************************************************************
 * Function Name : HttpGetAppData                                      *
 * Description   : Gives the current application data of that request  *
 * Input         : pHttpreq - Pointer To The Request                   *
 * Output        :                                                     *
 * Return value  : Returns pAppData of the request                     *
 ***********************************************************************/

char *HttpGetAppData( HttpRequest  *pHttpReq )
{
  return((char *) pHttpReq->pAppData);
} /* HttpGetAppData() */



/***********************************************************************
 * Function Name : Httpd_TagType_Handler_Table                         *
 * Description   : This is the table tag type handler.                  *
 * Input         : pHttpreq - Pointer To The Request                   *
 *                 pTTEntry - Private Tag-Type Entry                   *
 *                 pBuf     - Dynamic buffer to send                   *
 * Output        :                                                     *
 * Return value  : OF_SUCCESS on success                                *
 *                 OF_FAILURE on failure                                *
 ***********************************************************************/

int32_t HttpRetriveIpAddrFromValArray( Httpd_Value_t  *ValArray,
                                       uint32_t       *pIpAddr,
                                       int32_t        iIndex )
{
  uint32_t  uiAddrByte1=0, uiAddrByte2=0, uiAddrByte3=0, uiAddrByte4=0;
  char   c, *s=NULL;
  int32_t   ii=0;

  if (ValArray[iIndex].value_p)
  {
    for (ii = iIndex; ii <= iIndex + 3; ii++)
    {
      s =  ValArray[ii].value_p;

      while ((c = *s++) != '\0')
      {
        if (!isdigit(c))
        {
          return OF_FAILURE;
        }
      }
    }

    uiAddrByte1 = of_atoi(ValArray[iIndex].value_p);
    uiAddrByte2 = of_atoi(ValArray[iIndex+1].value_p);
    uiAddrByte3 = of_atoi(ValArray[iIndex+2].value_p);
    uiAddrByte4 = of_atoi(ValArray[iIndex+3].value_p);

    if ((uiAddrByte1 > 255) ||
        (uiAddrByte2 > 255) ||
        (uiAddrByte3 > 255) ||
        (uiAddrByte4 > 255))
    {
      Httpd_Debug("Invalid address");
      return 1;
    }

    *pIpAddr = (uint32_t)(uiAddrByte1 << 24) | (uiAddrByte2 << 16) |
                         (uiAddrByte3 << 8) | (uiAddrByte4);

    if (*pIpAddr == 0)
    {
      return 2;
    }
    return 0;
  }

  return 2;
} /* HttpRetriveIpAddrFromValArray() */

/***
 ***  Changes made by vamsi starts
 ***/

/***
 *** The following functions are for maintaining the opaque variables
 *** in the Http request structure.
 *** Reason: Today we don't have any provision to pass some relvant
 *** information from a post operation to a get operation,so that we
 *** can come out of this problem with this approach.
 *** Let us take an exmple 'HttpSendHtmlFile' it accept three inputs
 *** HttpReq,HtmlFileName & Application data,if we want to pass
 *** more number of entities we don't have any provision and our Http
 *** module need a mechanism to store the opaque varaibles.
 ***/

/***
 *** HttdReqOpaqueStatus maintains the available or not available status
 *** if any of the callback modules request for the opaque index then
 ***  we traverse through the array and return the available index,there after
 *** that module will write/read the data from the specified index of
 *** 'HttpRequest' structure.
 ***/
int32_t HttpReqOpaqueInit(HttpGlobals_t *pHttpGlobals)
{
  int32_t  iCount=0;

  if(!pHttpGlobals)
  {
    Httpd_Error("pHttpGlobals is NULL\r\n");	
    return OF_FAILURE;
  }
  
  for(iCount=0;iCount <  IGW_HTTPD_MAX_OPAQUE_VARIABLES;iCount++)
  {
   pHttpGlobals->HttpReqOpaqueStatus[iCount].bAvailable =TRUE;/**means avialable **/
  }
  return OF_SUCCESS;
}

int32_t IGWHttpRegisterforOpaqueIndex(HttpGlobals_t *pHttpGlobals,uint32_t      *pIndex,
                                       FREE_FUN_PTR  pFreeFunPtr )
{
  int32_t  iCount=0;

  if(!pHttpGlobals)
  {
    Httpd_Error("pHttpGlobals is NULL\r\n");	
    return OF_FAILURE;
  }
  
  if(pIndex == NULL)
  {
    return OF_FAILURE;
  }
  for(iCount=0;iCount<IGW_HTTPD_MAX_OPAQUE_VARIABLES;iCount++)
  {
    if(pHttpGlobals->HttpReqOpaqueStatus[iCount].bAvailable==TRUE)/**means avialable **/
    {
      pHttpGlobals->HttpReqOpaqueStatus[iCount].bAvailable=FALSE;/**means reserved **/
      pHttpGlobals->HttpReqOpaqueStatus[iCount].pFreeFun=pFreeFunPtr;

      *pIndex = iCount;
      return OF_SUCCESS;
    }
  }
  return OF_FAILURE;
}

int32_t IGWHttpDeRegisteOpaqueIndex(HttpGlobals_t *pHttpGlobals,uint32_t  uiIndex )
{
  if(!pHttpGlobals)
  {
    Httpd_Error("pHttpGlobals is NULL\r\n");	
    return OF_FAILURE;
  }
  
  if(uiIndex >= IGW_HTTPD_MAX_OPAQUE_VARIABLES)
  {
    /**Some one gave a wrong index ***/
    return OF_FAILURE;
  }

  if(pHttpGlobals->HttpReqOpaqueStatus[uiIndex].bAvailable== FALSE)
  {
    pHttpGlobals->HttpReqOpaqueStatus[uiIndex].bAvailable=TRUE;/**means available**/
    pHttpGlobals->HttpReqOpaqueStatus[uiIndex].pFreeFun=NULL;
  }
  else
  {
    /*trying to deregister which is not registered so return
     *error
     */
     return OF_FAILURE;
  }
  return OF_SUCCESS;
}

int32_t IGWHttpSetOpaqueData( HttpRequest  *pHttpReq,
                              uint32_t     uiIndex,
                          void       *pData )
{
  /*void          *pTmp;*/
  HttpClientReq_t *pTemp=NULL;
  if( (uiIndex >= IGW_HTTPD_MAX_OPAQUE_VARIABLES) ||
      ( pHttpReq == NULL) ||
      (pData == NULL )
    )
  {
#ifdef HTTPD_DEBUG
      Httpd_Error("Invalid inputs for IGWHttpSetOpaqueData !\n");
#endif /* HTTPD_DEBUG */
    return OF_FAILURE;
  }
 /**Set the Opaque data in Http Request strcuture ***/
  /*pTmp = (void *)*(pHttpReq->pHttpReqOpaqueNode);
  pTmp += uiIndex;
  pTmp = (void *)pData;*/
  pTemp = (HttpClientReq_t *)pHttpReq->pPvtData;
  /*pHttpReq->pPvtData->pHttpReqOpaqueNode[uiIndex]=pData; */
  pTemp->pHttpReqOpaqueNode[uiIndex]=pData;
  return OF_SUCCESS;
}
void  *IGWHttpGetOpaqueData( HttpRequest  *pHttpReq,
                           uint32_t     uiIndex )
{
  HttpClientReq_t *pTemp=NULL;
  if( (uiIndex >= IGW_HTTPD_MAX_OPAQUE_VARIABLES) ||
      ( pHttpReq == NULL)
    )
  {
#ifdef HTTPD_DEBUG
      Httpd_Error("Invalid inputs for IGWHttpGetOpaqueData !\n");
#endif /* HTTPD_DEBUG */
    return NULL;
  }
  pTemp = (HttpClientReq_t *)pHttpReq->pPvtData;

  return (void *)(pTemp->pHttpReqOpaqueNode[uiIndex]);
}
/***Changes made by Vamsi ends****/
#ifdef HTTPD_DEBUG

/**
 * Dumps the contents of the hash table.
 */
 void Httpd_HashTable_Dump( Httpd_HashTable_t  *pThis )
{
  Httpd_HashEntry_t  *pTemp=NULL;
  Httpd_Variable_t   *pVar=NULL;
  int32_t            ii=0, jj=0, nCount=0;

  HTTPD_ASSERT(!pThis);

  Httpd_Print("Hash Function = %p\r\n", pThis->HashFunct);
  Httpd_Print("Hash Size     = %d\r\n", pThis->HashSize);
  Httpd_Print("Max. Entries  = %d\r\n", pThis->MaxEntries);
  Httpd_Print("nCount        = %d\r\n", pThis->nCount);

  nCount = 0;

  for (ii = 0; ii < pThis->HashSize; ii++)
  {
    pTemp = pThis->pTable[ii];

    if (!pTemp)
    {
      continue;
    }

    while (pTemp)
    {
      nCount++;
      Httpd_Print("%d. HashPosition %d\r\n", nCount, ii);
      Httpd_Print("\tTag Name   = %s\r\n", pTemp->pTagName);
      Httpd_Print("\tCbFunct    = %p\r\n", pTemp->pCbFunct);
      Httpd_Print("\tVarCount   = %d\r\n", pTemp->VarCount);
      Httpd_Print("\tVarArray   = %p\r\n", pTemp->pVarArray);
      pVar = (Httpd_Variable_t *) pTemp->pVarArray;

      for (jj = 0; jj < pTemp->VarCount; jj++)
      {
        if (pVar && pVar->name_p)
        {
          Httpd_Print("\t\t%s\t\t%ld\r\n", pVar->name_p, pVar->Size);
          pVar++;
        }
        else
        {
          break;
        }
      }
      pTemp = pTemp->pNext;
    }
  }
} /* Httpd_HashTable_Dump() */

#endif /* HTTPD_DEBUG */

#endif /* OF_HTTPD_SUPPORT */

