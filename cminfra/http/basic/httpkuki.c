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
/*  File        : httpkuki.c                                              */
/*                                                                        */
/*  Description : This file contains http cookie related functions.       */
/*                                                                        */
/*  Version      Date      Author      Change Description                 */
/*  -------    --------    ------    ----------------------               */
/*    1.0      00.00.00    kmdhar    Initial implementation               */
/*    1.1      03.31.04    DRAM      Modified during the code review.     */
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
#include "httpgif.h"
#include "htpdglb.h"
#include "httpdtmr.h"
#include "httpd.h"

/****************************************************
 * * * *  M a c r o    D e f i n i t i o n s  * * * *
 ****************************************************/

#define  COOKIE_SEM_CREATE   Httpd_Cookie_SemCreate
#define  COOKIE_SEM_GET      Httpd_Cookie_SemGet
#define  COOKIE_SEM_RELEASE  Httpd_Cookie_SemRelease

/* #define  HTTPCOOKIE_MAX      (120) moved to system directory */

#define  HTTPCOOKIE_ISVALID(a) (a >= 0 && a < ulCookieMax)

#define HTTPCOOKIE_TBL_SIZE	64 /* should be a power of two */

#define HTTP_COOKIE_FINDBYVAL	0
#define HTTP_COOKIE_FINDBYNAME	1
#define HTTP_COOKIE_FINDBYIP	2
#define HTTP_COOKIE_FINDBYPORT	3
/************************************************************
 * * * *  V a r i a b l e    D e c l a r a t i o n s  * * * *
 ************************************************************/

/********************************************************
 * * * *  F u n c t i o n    P r o t o t y p e s  * * * *
 ********************************************************/

/**
 * This is cookie timer callback function.
 */

void HttpCookie_TmrCbk( void  *pVal );

 
uint32_t GetCookieIndex(unsigned char *value_p);


HttpCookie_t *HttpCookie_Search(void *pParam, uint32_t  ulType);

extern int32_t igwHTTPDUpdateUserloginTimeInKuki(char *pUserName, uint32_t ulLoginTime);
extern int32_t igwHTTPDGetUserloginTimeFromKuki(char *pUserName, uint32_t *pulLoginTime);
/***********************************************************************
 * Function Name : GetCookieIndex                                       *
 * Description   : Gets index into hash table from last two bytes of the*
 *                 cookie string                                        *
 * Input         : value_p - Pointer To Cookie Value                     *
 * Output        : None                                                 *
 * Return value  : Table Index                                          *
 ***********************************************************************/
 uint32_t GetCookieIndex(unsigned char *value_p)
{
  uint32_t ii=0,iTemp=0;	
  uint32_t jj=0;	
  uint32_t ulVal=0;	
  
  if((iTemp = of_strlen((char *) value_p)) < 2)
  {
     ii=0; 
  }
  else
  {
      ii = ( iTemp - 2);
  }
  value_p += ii;
  for(jj = 0; jj < 2; jj++)
  {	  
    ii = *value_p++;
    ulVal <<= 4;
    if(ii > '9') 
    {
      ii &= 0xDF;	 /* to lower */ 
      ulVal += (ii - 0x37); /* to hex Value */
    }
    else
     ulVal |= (ii & 0xf);      	  
  }    
  return (ulVal & (HTTPCOOKIE_TBL_SIZE - 1));
}
/***********************************************************************
 * Function Name : HttpCookie_TmrCbk                                      *
 * Description   : This is cookie timer callback function.                *
 *                 This deletes the cookie from the list                  *
 * Input         : pVal  - Pointer To Cookie Value                        *
 * Output        : None                                                   *
 * Return value  : None                                                   *
 ***********************************************************************/


void HttpCookie_TmrCbk( void  *pVal )
{
 HttpGlobals_t  *pHttpg=NULL;
#ifndef OF_CM_SUPPORT
 HttpCookie_t  *pKuki = NULL; 
#endif
 /*UserDbTemp_t  UserInfo;
 uint32_t SnetId = 0;*/

  if(!pVal)
  {
	return;
  }
  pHttpg=pHTTPG; 

  if (!pHttpg)
  {
     Httpd_Error("pHttpg is NULL\r\n");
     return ;
  }
  /**
   * Increment the timeout statistics field by 1.
   */
  pHttpg->HttpCookieStats.ulTimeOuts++;

  /**
   * Delete the cookie for the given cookie value.
   */
  /*FETCH the user record*/

#ifndef OF_CM_SUPPORT
  pKuki = HttpCookie_Search(pVal, HTTP_COOKIE_FINDBYVAL);
  
/*
 * FIXME: Need to validate through UCM PAM
 */
  if(pKuki) /*Validate the user*/
  {
     SnetId = pKuki->uiSNetId;
     if (GetExactUsrDbRec(SnetId, 
                          pKuki->ucUserName, &UserInfo) == OF_SUCCESS)
     {
        /* User is present without Admin privilages */
        if(of_strcmp(UserInfo.aUserName,"") &&      
          (UserInfo.uiPrivileges ) != ADMIN_PRIV) 
        {    
           Httpd_Timer_Restart(pKuki->pTimer);
           return;
        }
     }
   }
#endif /*OF_CM_SUPPORT*/

   HttpCookie_Del((unsigned char*) pVal);
} /* HttpCookie_TmrCbk() */
/**************************************************************************
 * Function Name : HttpCookie_Init                                        *
 * Description   : Initializes the data structures related to cookies     *
 * Input         : ulMax  - Max. no of cookies                            *
 * Output        : None                                                   *
 * Return value  : OF_SUCCESS on success                                   *
 *                 OF_FAILURE on failure.                                  *
 **************************************************************************/

int32_t HttpCookie_Init( HttpGlobals_t  *pHttpGlobals,uint32_t  ulMax )
{
  ulMax = HTTPCOOKIE_TBL_SIZE;
  /**
   * Allocate array of pointers and verify if it is allocated.
   */
  pHttpGlobals->pHttpCookieList = (HttpCookie_t**) of_calloc(ulMax,
                                              sizeof(HttpCookie_t*));

  if (!pHttpGlobals->pHttpCookieList)
  {
    return OF_FAILURE;
  }

  /**
   * Create the semaphore.
   */
  COOKIE_SEM_CREATE();

  return OF_SUCCESS;
} /* HttpCookie_Init() */

/**************************************************************************
 * Function Name : HttpCookie_Add                                         *
 * Description   : Adds a new cookie to the list                          *
 * Input         : value_p    - Cookie Value                               *
 *                 pUserName - User Name                                  *
 *                 pChallenge- Challenge String                           *
 *                 pPath     - Path String                                *
 *                 ulVersion - Cookie Version Number                      *
 *                 ulMaxAge  - Max. Age of Cookie                         *
 * Output        : None                                                   *
 * Return value  : OF_SUCCESS on success                                   *
 *                 OF_FAILURE on failure.                                  *
 **************************************************************************/

int32_t HttpCookie_Add(HttpGlobals_t *pHttpGlobals,
			   unsigned char  *value_p,
                        unsigned char  *pUserName,
                        unsigned char  *PeerIp,
                        unsigned char  *pChallenge,
                        unsigned char  *pPath,
                        uint32_t  ulVersion,
                        uint32_t  ulMaxAge )
{
  int32_t       lIndex;
  HttpCookie_t  *pCookie;
  int32_t       iMaxCookieValueLen; 
  /**
   * Validate the parameters.
   */
  if ((!pUserName)  ||
      (!value_p)     ||
      (!pChallenge) ||
      (!pPath))
  {
    return OF_FAILURE;
  }

  iMaxCookieValueLen=Httpd_GetMaxCookieValueLen(pHttpGlobals);/*pHttpGlobals->pHttpGlobalConfig->ulHttpdMaxCookieValueLen;*/
  /**
   * Validate the parameters lengths.
   */
  if ((of_strlen((char *)pUserName) > HTTP_UNAME_LEN)      ||
      (of_strlen((char *)value_p) > iMaxCookieValueLen)    ||
      (of_strlen((char *)pChallenge) > HTTP_CHALLENGE_LEN) ||
      (of_strlen((char *)pPath) > HTTP_PATH_LEN ))
  {
    return OF_FAILURE;
  }

  /**
   * Get the semaphore.
   */
  COOKIE_SEM_GET();

  /**
   * Allocate the memory for the cookie entry and verify whether allocation
   * is successful.
   */
  pCookie = (HttpCookie_t*)(of_calloc(1, sizeof(HttpCookie_t)));

  if (!pCookie)
  {
    /**
     * Release the semaphore.
     */
    COOKIE_SEM_RELEASE();
    return OF_FAILURE;
  }
  /* Find the index into table from cookie value */
  lIndex = GetCookieIndex(value_p);
  pCookie->ucValue=of_calloc(1,iMaxCookieValueLen +1);
  if(!pCookie->ucValue)
  {
    /**
     * Release the semaphore.
     */
     of_free(pCookie);
     COOKIE_SEM_RELEASE();
     return OF_FAILURE;
  }
  /**
   * Create the timer and validate it.
   */
  if (Httpd_Timer_Create(&pCookie->pTimer) < 0)
  {
    /**
     * Free the memory.
     */
     if(pCookie->ucValue)
     {
      of_free(pCookie->ucValue); 
     }
      of_free(pCookie);
  
      /**
       * Release the semaphore.
       */
      COOKIE_SEM_RELEASE();
      return OF_FAILURE;
  }

  /**
   * Copy the passed parameters to allocated cookie entry.
   */
  of_strcpy((char *)pCookie->ucValue, (char *)value_p);
  of_strcpy((char *)pCookie->ucUserName, (char *)pUserName);
  of_strcpy((char *)pCookie->ucIp, (char *)PeerIp);
  of_strcpy((char *)pCookie->ucChallenge, (char *)pChallenge);
  of_strcpy((char *)pCookie->ucPath, (char *)pPath);

  /**
   * Start the timer.
   */
  Httpd_Timer_Start(pCookie->pTimer,
                    (HTTPD_TIMER_CBFUN)(HttpCookie_TmrCbk),
                    (void*) pCookie->ucValue, HTTPD_TIMER_SEC,
                    ulMaxAge, 0);

  pCookie->ulVersion = ulVersion;
  pCookie->ulMaxAge  = ulMaxAge;
  pCookie->bLoggedIn = FALSE;

  /* Add entry at the head of the list at this index */ 
  pCookie->pNext = pHttpGlobals->pHttpCookieList[lIndex];

  pHttpGlobals->pHttpCookieList[lIndex] = pCookie;

  pHttpGlobals->HttpCookieStats.ulAdditions++;

  /**
   * Release the semaphore.
   */
  COOKIE_SEM_RELEASE();
  #ifdef HTTPD_DEBUG
  printf("%s::Cookie Added\n", __FUNCTION__);
  #endif /*HTTPD_DEBUG*/
  return OF_SUCCESS;
} /* HttpCookie_Add() */

/***********************************************************************
 * Function Name : HttpCookie_Del                                      *
 * Description   : This deletes the cookie node using cookie value     *
 * Input         : pCookieVal - cookie value                           *
 * Output        : None                                                *
 * Return value  : OF_SUCCESS on success                                *
 *                 OF_FAILURE on failure.                               *
 ***********************************************************************/

int32_t HttpCookie_Del(unsigned char  *pCookieVal)
{
  HttpCookie_t  *pPrevCookie=NULL;
  HttpCookie_t  *pCookie=NULL;
  uint32_t lIndex=0;
  HttpGlobals_t *pHttpg=NULL;
#ifdef INTOTO_UAM_SUPPORT
  IGWUAMRedirAPIInfo_t RedirInfo = {};

  of_memset(&RedirInfo,0,sizeof(IGWUAMRedirAPIInfo_t));
#endif


  if(!pCookieVal)
    return OF_FAILURE;
 /* COOKIE_SEM_GET();*/
  pHttpg=pHTTPG;
 if(!pHttpg)
 {
    Httpd_Error("pHttpg is NULL\r\n");
    return OF_FAILURE;
 }
  /* get index into table from cookie bits */  
  lIndex = GetCookieIndex(pCookieVal);

  /* get list head at this index */  
  pCookie = pHttpg->pHttpCookieList[lIndex];
  if((!pCookie)  ||(!pCookie->ucValue))
 {
     /*COOKIE_SEM_RELEASE();*/
     return OF_FAILURE;
 }
  /**
   * Get the semaphore.
   */
  pPrevCookie = (HttpCookie_t  *)0;
  
  while(1)
  {	
    if(!pCookie)
    {	    
      /*COOKIE_SEM_RELEASE();*/
      return OF_FAILURE;
    }

    if((pCookie->ucValue)&&( !of_strcmp((char *)pCookie->ucValue, (char *)pCookieVal)))
    {
      /* Found Cookie */
      if(pPrevCookie)
        pPrevCookie->pNext = pCookie->pNext;
      else
        pHttpg->pHttpCookieList[lIndex] = pCookie->pNext;
      break; 
    }
    pPrevCookie = pCookie;
    pCookie = pCookie->pNext; 
  }
  /**
   * Delete the timer.
   */
  if(pCookie->pTimer)
    Httpd_Timer_Delete(pCookie->pTimer);

  /**
   * Increment the cookie deletions count by 1.
   */
  pHttpg->HttpCookieStats.ulDeletions++;

#ifdef INTOTO_UAM_SUPPORT
  of_strncpy(RedirInfo.cCookie, pCookie->ucValue, 50);
  IGWUAMDeleteRedirInfoAPI(&RedirInfo);
#endif

  /**
   * Free the memory.
   */
  if(pCookie->ucValue)
    of_free(pCookie->ucValue); 
  of_free(pCookie);

  /**
   * Release the semaphore.
   */
  /*COOKIE_SEM_RELEASE();*/
  return OF_SUCCESS;

} /* HttpCookie_Del() */


/**************************************************************************
 * Function Name : HttpCookie_Search                                      *
 * Description   : Finds Cookie Node Using Cookie Valuei, name, ip, port  *
 * Input         : pParam - Cookie Value, Name, Ip, Port                  *
 * Output        : pIndex - Not used                                      *
 * Return value  : Cookie Node Pointer on success                         *
 *                 NULL on failure.                                       *
 **************************************************************************/
 HttpCookie_t *HttpCookie_Search(void *pParam,
                               uint32_t  ulType)
{
  uint32_t ii=0;
  uint32_t jj=0;
  uint32_t lIndex=0;
  HttpCookie_t *pCookie=NULL;
  HttpGlobals_t *pHttpg=NULL;

  /**
   * Validate the passed parameter.
   */
  if(!pParam)
    return NULL;
   pHttpg=pHTTPG;
   if(!pHttpg)
   {
     Httpd_Error("pHttpg is NULL\r\n");
     return NULL;
   }
  if(ulType == HTTP_COOKIE_FINDBYVAL)
  {	  
    /* get hash table index from cookie value */
    lIndex = GetCookieIndex((unsigned char *)pParam);
    jj = (lIndex + 1);
  }    
  else
  {
    lIndex = 0;
    jj = HTTPCOOKIE_TBL_SIZE;
  }

  for(; lIndex < jj; lIndex++)
  {	  
    pCookie = pHttpg->pHttpCookieList[lIndex];
    while(1)  
    {
      if(!pCookie)
        break;	    
      switch(ulType)
      {
        case HTTP_COOKIE_FINDBYVAL:
          if(!of_strcmp((char *)pCookie->ucValue, (char *)pParam))
            return(pCookie);		
          break;	      
	
        case HTTP_COOKIE_FINDBYNAME:
          if(!of_strcmp((char *)pCookie->ucUserName, (char *)pParam))
            return(pCookie);		
          break;	      
	
        case HTTP_COOKIE_FINDBYIP:
          if(!of_strcmp((char *)pCookie->ucIp, (char *)pParam))	    
            return(pCookie);		
          break;	      
	
        case HTTP_COOKIE_FINDBYPORT:
          ii = (uint32_t)pParam;	  
          if(pCookie->usPort == ii)
            return(pCookie);		
          break;	      
      }	    
      pCookie = pCookie->pNext;   
    }
  }    
  return NULL;	    
} /* HttpCookie_Search() */

/**************************************************************************
 * Function Name : HttpCookie_Find                                        *
 * Description   : Finds Cookie Node Using Cookie Value                   *
 * Input         : pCookieVal - Cookie Value                              *
 * Output        : pIndex     - Index of the node                         *
 * Return value  : Cookie Node Pointer on success                         *
 *                 NULL on failure.                                       *
 **************************************************************************/
HttpCookie_t *HttpCookie_Find( unsigned char  *pCookieVal,
                               uint32_t  *pIndex)
{
  return(HttpCookie_Search(pCookieVal, HTTP_COOKIE_FINDBYVAL));

} /* HttpCookie_Find() */

/***********************************************************************
 * Function Name : HttpCookie_FindByName                                  *
 * Description   : Finds Cookie Node Using Name                           *
 * Input         : name_p - User Name                                      *
 * Output        : pIndex     - Index of the node                         *
 * Return value  : Cookie Node Pointer on success                         *
 *                 NULL on failure.                                       *
 ***********************************************************************/

HttpCookie_t *HttpCookie_FindByName(unsigned char  *name_p, uint32_t  *pIndex)
{
  return(HttpCookie_Search(name_p, HTTP_COOKIE_FINDBYNAME));
} /* HttpCookie_FindByName() */

/**************************************************************************
 * Function Name : HttpCookie_DelAllByPort                                *
 * Description   : This function is used to delete all the cookie entries *
 *                 with the given port number.                            *
 * Input         : usPort - port number for which entries to be deleted.  *
 * Output        : None                                                   *
 * Return value  : None.                                                  *
 **************************************************************************/

void HttpCookie_DelAllByPort( uint16_t  usPort )
{
  HttpCookie_t  *pKuki = NULL;
  uint32_t         ii=0;

  /**
   * Get the semaphore.
   */
  COOKIE_SEM_GET();

  /**
   * Delete all the cookie entries with the given port.
   */
  ii=usPort;
  while (1)
  {
    /**
     * Get the index of the cookie entry to be deleted using the passed
     * user name parameter and validate the operation.
     */
    pKuki = HttpCookie_Search((void *)ii, HTTP_COOKIE_FINDBYPORT);
    if (!pKuki)   
      break;
    HttpCookie_Del(pKuki->ucValue);
    
   }    

  /**
   * Release the semaphore.
   */
  COOKIE_SEM_RELEASE();

} /* HttpCookie_DelAllByPort() */

/**************************************************************************
 * Function Name : HttpCookie_GetByVal                                    *
 * Description   : This can be used get info about cookie                 *
 * Input         : pCookieVal - cookie value                              *
 * Output        : pLoggedIn  - login flag                                *
 *                 pChallenge - Challenge String                          *
 *                 pUserName  - User Name                                 *
 * Return value  : OF_SUCCESS on success                                   *
 *                 OF_FAILURE on failure.                                  *
 **************************************************************************/

int32_t HttpCookie_GetByVal( unsigned char  *pCookieVal,
                             bool   *pLoggedIn,
                             unsigned char  *pChallenge,
                             unsigned char  *pUserName)
{
  HttpCookie_t  *pKuki=NULL;
  
  if(!pCookieVal)
  {
	  return OF_FAILURE;
  }
  /**
   * Get the semaphore.
   */
  COOKIE_SEM_GET();
  
  pKuki = HttpCookie_Search(pCookieVal, HTTP_COOKIE_FINDBYVAL);
  if (!pKuki)
  {
    /**
     * Release the semaphore.
     */
    COOKIE_SEM_RELEASE();
    return OF_FAILURE;
  }

  /**
   * Validate the parameter and fill the respective value using the obtained
   * cookie entry.
   */
  if (pLoggedIn)
  {
    *pLoggedIn = pKuki->bLoggedIn;
  }

  /**
   * Validate the parameter and fill the respective value using the obtained
   * cookie entry.
   */
  if (pChallenge)
  {
    of_strcpy((char *)pChallenge, (char *)pKuki->ucChallenge);
  }

  /**
   * Validate the parameter and fill the respective value using the obtained
   * cookie entry.
   */
  if (pUserName)
  {
    of_strcpy((char *)pUserName, (char *)pKuki->ucUserName);
  }

  /**
   * Release the semaphore.
   */
  COOKIE_SEM_RELEASE();
  return OF_SUCCESS;
} /* HttpCookie_GetByVal() */

/**************************************************************************
 * Function Name : HttpCookie_GetSNetId                                   *
 * Description   : This function is used to get the SNet id for the given *
 *                 cookie value.                                          *
 * Input         : pCookieVal - cookie value                              *
 * Output        : puiSNetIde - SNet identification.                      *
 * Return value  : OF_SUCCESS on success                                   *
 *                 OF_FAILURE on failure.                                  *
 **************************************************************************/

int32_t HttpCookie_GetSNetId( unsigned char  *pCookieVal,
                              uint32_t  *puiSNetId )
{
  HttpCookie_t  *pKuki=NULL;
  
   if(!pCookieVal)
   {
	  return OF_FAILURE;
   }
    
  /**
   * Get the semaphore.
   */
  COOKIE_SEM_GET();

  /**
   * Get the index of the cookie entry to be deleted using the passed
   * cookie value parameter and validate the operation.
   */
  pKuki = HttpCookie_Search(pCookieVal, HTTP_COOKIE_FINDBYVAL);
  if (!pKuki)
  {
    /**
     * Release the semaphore.
     */
    COOKIE_SEM_RELEASE();
    return OF_FAILURE;
  }

  /**
   * Validate the parameter and fill the respective value using the obtained
   * cookie entry.
   */
  if (puiSNetId)
  {
    *puiSNetId = pKuki->uiSNetId;
  }

  /**
   * Release the semaphore.
   */
  COOKIE_SEM_RELEASE();
  return OF_SUCCESS;
} /* HttpCookie_GetSNetId() */

/**************************************************************************
 * Function Name : HttpCookie_GetBytesWritten                             *
 * Description   : This function is used to get the bytes written.        *
 * Input         : pCookieVal - cookie value                              *
 * Output        : puiBytesWritten (O) - Number of bytes written.         *
 * Return value  : OF_SUCCESS on success                                   *
 *                 OF_FAILURE on failure.                                  *
 **************************************************************************/

int32_t HttpCookie_GetBytesWritten( unsigned char  *pCookieVal,
                                    uint32_t  *puiBytesWritten )
{
  HttpCookie_t  *pKuki=NULL;

  if(!pCookieVal)
  {
	  return OF_FAILURE;
  }
  /**
   * Get the semaphore.
   */
  COOKIE_SEM_GET();

  /**
   * Get the index of the cookie entry to be deleted using the passed
   * cookie value parameter and validate the operation.
   */
  pKuki = HttpCookie_Search(pCookieVal, HTTP_COOKIE_FINDBYVAL);
  if (!pKuki)
  {
    /**
     * Release the semaphore.
     */
    COOKIE_SEM_RELEASE();
    return OF_FAILURE;
  }

  /**
   * Validate the parameter and fill the respective value using the obtained
   * cookie entry.
   */
  if (puiBytesWritten)
  {
    *puiBytesWritten = pKuki->uiBytesWritten;
  }

  /**
   * Release the semaphore.
   */
  COOKIE_SEM_RELEASE();
  return OF_SUCCESS;
} /* HttpCookie_GetBytesWritten() */

/**************************************************************************
 * Function Name : HttpCookie_GetHashEntry                                *
 * Description   : This function is used to get the hash entry.           *
 * Input         : pCookieVal - cookie value                              *
 * Output        : ppBinFileEntry (O) - hash entry used by file transfer. *
 * Return value  : OF_SUCCESS on success                                   *
 *                 OF_FAILURE on failure.                                  *
 **************************************************************************/

int32_t HttpCookie_GetHashEntry( unsigned char  *pCookieVal,
                                 void    **ppBinFileEntry )
{
  HttpCookie_t  *pKuki=NULL;
  
  if(!pCookieVal)
  { 
	  return OF_FAILURE;
  }
  /**
   * Get the semaphore.
   */
  COOKIE_SEM_GET();

  /**
   * Get the index of the cookie entry to be deleted using the passed
   * cookie value parameter and validate the operation.
   */
  pKuki = HttpCookie_Search(pCookieVal, HTTP_COOKIE_FINDBYVAL);
  if (!pKuki)
  {
    /**
     * Release the semaphore.
     */
    COOKIE_SEM_RELEASE();
    return OF_FAILURE;
  }

  /**
   * Validate the parameter and fill the respective value using the obtained
   * cookie entry.
   */
  if (pKuki->pBinFileEntry)
  {
    *ppBinFileEntry = pKuki->pBinFileEntry;
  }
  else
  {
    *ppBinFileEntry = NULL;
  }

  /**
   * Release the semaphore.
   */
  COOKIE_SEM_RELEASE();
  return OF_SUCCESS;
} /* HttpCookie_GetHashEntry() */

/**************************************************************************
 * Function Name : HttpCookie_LoggedIn                                    *
 * Description   : This function should be called after successful        *
 *                 authentication of user                                 *
 * Input         : pCookieVal - cookie value                              *
 *                 pUserName  - new user name                             *
 *                 ulNewTime  - new time out                              *
 * Output        : None                                                   *
 * Return value  : OF_SUCCESS on success                                   *
 *                 OF_FAILURE on failure.                                  *
 **************************************************************************/

int32_t HttpCookie_LoggedIn( unsigned char      *pCookieVal,
                             unsigned char      *pUserName,
                             uint32_t      ulNewTime )
{
  HttpCookie_t  *pKuki=NULL;
  HttpGlobals_t  *pHttpg=NULL;
  
  if(!pCookieVal)
  {
	  return OF_FAILURE;
  }
  pHttpg=pHTTPG;
  if(!pHttpg)
 {
    Httpd_Error("pHttpg is NULL\r\n");	
    return OF_FAILURE;
 }
  /**
   * Get the semaphore.
   */
  COOKIE_SEM_GET();

  /**
   * Get the index of the cookie entry to be deleted using the passed
   * cookie value parameter and validate the operation.
   */
  pKuki = HttpCookie_Search(pCookieVal, HTTP_COOKIE_FINDBYVAL);
  if (!pKuki)
  {
    /**
     * Release the semaphore.
     */
    COOKIE_SEM_RELEASE();

    return OF_FAILURE;
  }

  /**
   * Copy the user name and other information to the obtained cookie
   * structure.
   */
  of_strcpy((char *)pKuki->ucUserName, (char *)pUserName);
  pKuki->ulMaxAge  = ulNewTime;
  pKuki->bLoggedIn = TRUE;

  /**
   * Timaer needs to be maintained only for config admin users.
   * For other users packet processing modules will maintain the timer.
   */
  if(HttpUserHasAdminPriv((char *)pUserName) == TRUE)
  {
    /**
     * Restart the timer with the new value.
     */
    if (Httpd_Timer_RestartWithNewValue(pKuki->pTimer, ulNewTime) < 0)
    {
      Httpd_Error("Unable To Restart the timer\n");

      /**
       * Release the semaphore.
       */
      COOKIE_SEM_RELEASE();
      return OF_FAILURE;
    }
  }

  /**
   * Increment the logins count by 1.
   */
  pHttpg->HttpCookieStats.ulLogIns++;

  /**
   * Release the semaphore.
   */
  COOKIE_SEM_RELEASE();

  return OF_SUCCESS;
} /* HttpCookie_LoggedIn() */

/**************************************************************************
 * Function Name : HttpCookie_AddPortInfo                                 *
 * Description   : This function is used to add the port information      *
 *                 to the given cookie after successful login.            *
 * Input         : pCookieVal - cookie value to be used to find the entry.*
 *                 usPort     - port number to be added.                  *
 * Output        : None                                                   *
 * Return value  : OF_SUCCESS on success                                   *
 *                 OF_FAILURE on failure.                                  *
 **************************************************************************/

int32_t HttpCookie_AddPortInfo( unsigned char  *pCookieVal,
                                uint16_t  usPort )
{
 HttpCookie_t  *pKuki=NULL;
  
 if(!pCookieVal)
  {
	  return OF_FAILURE;
  }
  /**
   * Get the semaphore.
   */
  COOKIE_SEM_GET();

  /**
   * Get the index of the cookie entry to be deleted using the passed
   * cookie value parameter and validate the operation.
   */
  pKuki = HttpCookie_Search(pCookieVal, HTTP_COOKIE_FINDBYVAL);
  if (!pKuki)
  {
    /**
     * Release the semaphore.
     */
    COOKIE_SEM_RELEASE();

    return OF_FAILURE;
  }

  /**
   * Copy the port identification to the obtained cookie structure.
    */
  pKuki->usPort = usPort;

  /**
   * Release the semaphore.
   */
  COOKIE_SEM_RELEASE();

  return OF_SUCCESS;
} /* HttpCookie_AddPortInfo() */

/**************************************************************************
 * Function Name : HttpCookie_AddSNetId                                   *
 * Description   : This function is used to add the SNet identification   *
 *                 to the given cookie after successful login.            *
 * Input         : pCookieVal - cookie value to be used to find the entry.*
 *                 uiSNetId   - SNet identification to be added.          *
 * Output        : None                                                   *
 * Return value  : OF_SUCCESS on success                                   *
 *                 OF_FAILURE on failure.                                  *
 **************************************************************************/

int32_t HttpCookie_AddSNetId( unsigned char  *pCookieVal,
                              uint32_t  uiSNetId )
{
 HttpCookie_t  *pKuki=NULL;

 if(!pCookieVal)
  {
	  return OF_FAILURE;
  }
  /**
   * Get the semaphore.
   */
  COOKIE_SEM_GET();

  /**
   * Get the index of the cookie entry to be deleted using the passed
   * cookie value parameter and validate the operation.
   */
  pKuki = HttpCookie_Search(pCookieVal, HTTP_COOKIE_FINDBYVAL);
  if (!pKuki)
  {
    /**
     * Release the semaphore.
     */
    COOKIE_SEM_RELEASE();

    return OF_FAILURE;
  }

  /**
   * Copy the SNet identification to the obtained cookie structure.
   */
  pKuki->uiSNetId = uiSNetId;

  /**
   * Release the semaphore.
   */
  COOKIE_SEM_RELEASE();

  return OF_SUCCESS;
} /* HttpCookie_AddSNetId() */

/**************************************************************************
 * Function Name : HttpCookie_SetBytesWritten                             *
 * Description   : This function is used to set the bytes written variable*
 *                 of the cookie entry.                                   *
 * Input         : pCookieVal - cookie value to be used to find the entry.*
 *                 uiBytesWritten (I) - number of bytes written to be set.*
 * Output        : None                                                   *
 * Return value  : OF_SUCCESS on success                                   *
 *                 OF_FAILURE on failure.                                  *
 **************************************************************************/

int32_t HttpCookie_SetBytesWritten( unsigned char  *pCookieVal,
                                    uint32_t  uiBytesWritten )
{
  HttpCookie_t  *pKuki=NULL;

   if(!pCookieVal)
  {
	  return OF_FAILURE;
  }

  /**
   * Get the semaphore.
   */
  COOKIE_SEM_GET();

  /**
   * Get the index of the cookie entry to be deleted using the passed
   * cookie value parameter and validate the operation.
   */
  pKuki = HttpCookie_Search(pCookieVal, HTTP_COOKIE_FINDBYVAL);
  if (!pKuki)
  {
    /**
     * Release the semaphore.
     */
    COOKIE_SEM_RELEASE();

    return OF_FAILURE;
  }

  /**
   * Copy the bytes written to the obtained cookie structure.
   */
  pKuki->uiBytesWritten = uiBytesWritten;

  /**
   * Release the semaphore.
   */
  COOKIE_SEM_RELEASE();

  return OF_SUCCESS;
} /* HttpCookie_SetBytesWritten() */

/**************************************************************************
 * Function Name : HttpCookie_SetHashEntry                                *
 * Description   : This function is used to set the hash entry of the     *
 *                 cookie entry.                                          *
 * Input         : pCookieVal - cookie value to be used to find the entry.*
 *                 ppBinFileEntry (I) - hash entry to be set.             *
 * Output        : None                                                   *
 * Return value  : OF_SUCCESS on success                                   *
 *                 OF_FAILURE on failure.                                  *
 **************************************************************************/

int32_t HttpCookie_SetHashEntry( unsigned char  *pCookieVal,
                                 void    **ppBinFileEntry )
{
 HttpCookie_t  *pKuki=NULL;

 if(!pCookieVal)
  {
	  return OF_FAILURE;
  }

  /**
   * Get the semaphore.
   */
  COOKIE_SEM_GET();

  /**
   * Get the index of the cookie entry to be deleted using the passed
   * cookie value parameter and validate the operation.
   */
  pKuki = HttpCookie_Search(pCookieVal, HTTP_COOKIE_FINDBYVAL);
  if (!pKuki)
  {
    /**
     * Release the semaphore.
     */
    COOKIE_SEM_RELEASE();

    return OF_FAILURE;
  }

  /**
   * Copy the hash entry to the obtained cookie structure.
   */
  pKuki->pBinFileEntry = *ppBinFileEntry;

  /**
   * Release the semaphore.
   */
  COOKIE_SEM_RELEASE();

  return OF_SUCCESS;
} /* HttpCookie_SetHashEntry() */

/**************************************************************************
 * Function Name : HttpCookie_ResetInacTimeout                            *
 * Description   : This function is used to reset the inactivity          *
 *                 timeout value for the cookie entry with the given      *
 *                 cookie value.                                          *
 * Input         : pCookieVal - cookie value                              *
 *                 pHttpGlobals  - Global pointer                         *
 * Output        : None                                                   *
 * Return value  : None                                                   *
 **************************************************************************/

void HttpCookie_ResetInacTimeout(HttpGlobals_t *pHttpGlobals,unsigned char  *pCookieVal)
{
 HttpCookie_t  *pKuki=NULL;

  if(!pHttpGlobals)
  {
    Httpd_Error("pHttpGlobals is NULL\r\n");	
    return;
  }
 if(!pCookieVal)
  {
	  return;
  }
 
  /**
   * Get the semaphore.
   */
  COOKIE_SEM_GET();

  /**
   * Get the index of the cookie entry to be deleted using the passed
   * cookie value parameter and validate the operation.
   */
  pKuki = HttpCookie_Search(pCookieVal, HTTP_COOKIE_FINDBYVAL);
  if (!pKuki)
  {
    /**
     * Release the semaphore.
     */
    COOKIE_SEM_RELEASE();
    return;
  }
#if 0 /*for UCM*/
  /**
   * Restart the timer if the login is TRUE.
   */
  if ((HttpUserIsConfigType(pKuki->ucUserName)) &&
      (pKuki->bLoggedIn))
  {
    Httpd_Timer_Restart(pKuki->pTimer);
  }
#endif /* for UCM*/

  /**
   * Release the semaphore.
   */
  COOKIE_SEM_RELEASE();
} /* HttpCookie_ResetInacTimeout() */

/**************************************************************************
 * Function Name : SetUserInactivityTimeOut                               *
 * Description   : This function is used to set the inactivity timeout    *
 *                 value in the cookie entry using the parameters.        *
 * Input         : pCookieVal     - Cookie value                          *
 *                 uiAdminTimeOut - Admin inactivity timeout value.       
 *                 pHttpGlobals  - Global pointer                         *
 * Output        : None                                                   *
 * Return value  : OF_SUCCESS on success                                   *
 *                 OF_FAILURE on failure.                                  *
 **************************************************************************/

int32_t SetUserInactivityTimeOut(HttpGlobals_t *pHttpGlobals,unsigned char  *pCookieVal,
                                  uint32_t  uiAdminTimeOut )
{
 HttpCookie_t  *pKuki=NULL;
 
  if(!pHttpGlobals)
  {
    Httpd_Error("pHttpGlobals is NULL\r\n");	
    return OF_FAILURE;
  }
  
 if(!pCookieVal)
  {
	  return OF_FAILURE;
  }
 
  
  /**
   * Get the index of the cookie entry to be deleted using the passed
   * cookie value parameter and validate the operation.
   */
  pKuki = HttpCookie_Search(pCookieVal, HTTP_COOKIE_FINDBYVAL);
  if (!pKuki)
  {
    return OF_FAILURE;
  }

  /**
   * Set the user name and inactivity time out values.
   */
  pKuki->ulMaxAge  = uiAdminTimeOut;

  /**
   * Restart the timer with the new values.
   */
  if (Httpd_Timer_RestartWithNewValue(pKuki->pTimer,
                                      uiAdminTimeOut) < 0)
  {
    return OF_FAILURE;
  }
  return OF_SUCCESS;
} /* SetUserInactivityTimeOut() */
/***********************************************************************
 * Function Name : HttpCookie_DelByName                                   *
 * Description   : This deletes the cookie node using user name           *
 * Input         : name_p - user name                                      *
 * Output        : None                                                   *
 * Return value  : OF_SUCCESS on success                                   *
 *                 OF_FAILURE on failure.                                  *
 ***********************************************************************/

int32_t HttpCookie_DelByName( unsigned char  *name_p )
{
  HttpCookie_t  *pKuki=NULL;

  /**
   * Validate the parameters.
   */
  if ((!name_p) ||
      (!of_strcmp((char *)name_p, "")))
  {
    return OF_FAILURE;
  }

  /**
   * Get the semaphore.
   */
  COOKIE_SEM_GET();

  /**
   * Get the index of the cookie entry to be deleted using the passed
   * user name parameter and validate the operation.
   */
  pKuki = HttpCookie_Search(name_p, HTTP_COOKIE_FINDBYNAME);
  if (!pKuki)
  {
    /**
     * Release the semaphore.
     */
    COOKIE_SEM_RELEASE();
    return OF_FAILURE;
  }
  HttpCookie_Del(pKuki->ucValue);
 
  /**
   * Release the semaphore.
   */
  COOKIE_SEM_RELEASE();

  return OF_SUCCESS;
} /* HttpCookie_DelByName() */

/***********************************************************************
 * Function Name : HttpSNetCookie_Del                                     *
 * Description   : This deletes the SNet cookie using cookie value        *
 * Input         : pReq - HttpRequest                                     *
 * Output        : None                                                   *
 * Return value  : OF_SUCCESS on success                                   *
 *                 OF_FAILURE on failure.                                  *
 ***********************************************************************/

int32_t HttpSNetCookie_Del( HttpRequest  *pReq )
{
  HttpClientReq_t  *pClntReq = NULL;

  if(!pReq)
  {
      return OF_FAILURE;
  }

  /**
   * Get the handle for http client request private data.
   */
  pClntReq = (HttpClientReq_t*) pReq->pPvtData;

  /**
   * Validate the private data.
   */
  if (!pClntReq)
  {
    return OF_FAILURE;
  }

  /**
   * Delete the cookie of the private data.
   */
   return HttpCookie_Del(pClntReq->ucCookieVal);
} /* HttpSNetCookie_Del() */

/***********************************************************************
 * Function Name : HttpCookie_IsUserLoggedIn                           *
 * Description   : This function is used to verify whether the given   *
 *                 user is logged in or not.                           *
 * Input         : pUserName (I) - user name to be checked in the      * 			
 *                                 information table.                  *
 *               : peerIp        - Current Request IP  		       *
 *               : ucCookieValue - Current Request CookieValue         *
 * Output        : None                                                *
 * Return value  : TRUE if the user is already logged in, otherwise    *
 *                 FALSE.                                              *
 ***********************************************************************/

bool HttpCookie_IsUserLoggedIn( unsigned char  *pUserName ,unsigned char *PeerIp,unsigned char *ucCookieValue)
{
  HttpCookie_t  *pKuki=NULL;
  bool       bUserLoggedIn = FALSE;
  unsigned char      *SrcIp=NULL;
  /**
   * Validate the passed parameter.
   */
  if ((!pUserName) ||
      (!of_strcmp((char *)pUserName, "")))
  {
    return OF_FAILURE;
  }

   /**
   * Get the index of the cookie entry to be deleted using the passed
   * user name parameter and validate the operation.
   */
  pKuki = HttpCookie_Search(pUserName, HTTP_COOKIE_FINDBYNAME);
  if (pKuki)
  {
     SrcIp=pKuki->ucIp;
     if(of_strcmp((char *)SrcIp,(char *)PeerIp)==0)   
     {
    	 if(of_strcmp((char *)pKuki->ucValue,(char *)ucCookieValue)==0)
        {
            /**
              *  This senario occures only user is tring to click signin button more than once.
              */
	      bUserLoggedIn = FALSE;
        }
        else
      	 {
		/*close the session forcefully*/
		HttpCookie_DelByName(pUserName);    
		 bUserLoggedIn = FALSE;
        }    
	     
    }	    
    else
    {
    /**
     * Set the flag representing user has already been logged in.
     */
     bUserLoggedIn = TRUE;	  
    }
 }
   return bUserLoggedIn;
} /* HttpCookie_IsUserLoggedIn() */
/***********************************************************************
 *Function Name : Httpd_GetSrcIpAddress					*
 *Description   :This function is used to get Ip address of user who    *		
 *		already logged in.					*
 *Input		:user name.    						*
 *Output 	:SrcIp address						*  
 *Return Value	:Ip address 									*	
***********************************************************************/
unsigned char *Httpd_GetSrcIpAddress(unsigned char  *pUserName)
{
	HttpCookie_t  *pKuki=NULL;	
	static unsigned char  ucSrc[HTTP_UNAME_LEN];
	
	if(!pUserName)
	{
  	      return NULL;
	}
        pKuki = HttpCookie_Search(pUserName, HTTP_COOKIE_FINDBYNAME); 
	if(!pKuki)
	{
		return NULL;
	}
	of_strcpy((char *)ucSrc,(char *)pKuki->ucIp);       
	return ucSrc;
}/*Httpd_GetSrcIpAddress*/

/***********************************************************************
 * Function Name : HttpCookie_DelByIpAddress                              *
 * Description   : deletes Cookie Node Using IpAddress                    *
 * Input         : PeerIp                                                 *
 * Output        : OF_SUCCESS			                          *
 * Return value  : OF_SUCCESS			                          *
 * ***********************************************************************/

int32_t HttpCookie_DelByIpAddress( unsigned char  *Peer )
{
 HttpCookie_t  *pKuki=NULL;

 /**
  * Validate the parameters.
  */
   if ((!Peer) ||
           (!of_strcmp((char *)Peer, "")))
       {
    		return OF_FAILURE;
       }

  /**
   * Get the semaphore.
   */
   COOKIE_SEM_GET();

  /**
   * Get the index of the cookie entry to be deleted using the passed
   * user Ip address parameter and validate the operation.
   */
  pKuki = HttpCookie_Search(Peer, HTTP_COOKIE_FINDBYIP);
  if (!pKuki)
  {
        /**
         * Release the semaphore.
         */
         COOKIE_SEM_RELEASE();
         return OF_FAILURE;
  }
  HttpCookie_Del(pKuki->ucValue);    
  
 /**
  * Release the semaphore.
  */
  COOKIE_SEM_RELEASE();
  return OF_SUCCESS;
} /* HttpCookie_DelByIpAddress() */

int32_t igwHTTPDUpdateUserloginTimeInKuki(char *pUserName, uint32_t ulLoginTime)
{
  HttpCookie_t  *pKuki=NULL;

  COOKIE_SEM_GET();
  pKuki = HttpCookie_Search(pUserName, HTTP_COOKIE_FINDBYNAME);
  if (!pKuki)
  {
    COOKIE_SEM_RELEASE();
    return OF_FAILURE;
  }
  pKuki->ulLoginTime = ulLoginTime;
  COOKIE_SEM_RELEASE();
  return OF_SUCCESS;
}

int32_t igwHTTPDGetUserloginTimeFromKuki(char *pUserName, uint32_t *pulLoginTime)
{
  HttpCookie_t  *pKuki=NULL;

  COOKIE_SEM_GET();
  pKuki = HttpCookie_Search(pUserName, HTTP_COOKIE_FINDBYNAME);
  if (!pKuki)
  {
    COOKIE_SEM_RELEASE();
    return OF_FAILURE;
  }
  *pulLoginTime = pKuki->ulLoginTime;
  COOKIE_SEM_RELEASE();
  return OF_SUCCESS;
}


#endif /* OF_HTTPD_SUPPORT */

