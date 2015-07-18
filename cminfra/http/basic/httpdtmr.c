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
/*  File        : httpdtmr.c                                              */
/*                                                                        */
/*  Description : This file contains http timer related functions.        */
/*                                                                        */
/*  Version      Date      Author      Change Description                 */
/*  -------    --------    ------    ----------------------               */
/*    1.0      00.00.00    kmdhar    Initial implementation               */
/*    1.1      05.11.04    DRAM      Modified during the code review.     */
/*    1.2      22.02.06    Harishp   Modified during MultipleInstances	  */	
/*                                   and Multithreading	                  */
/**************************************************************************/

#ifdef OF_HTTPD_SUPPORT

/***********************************************************
 * * * *  I n c l u d e    H e a d e r    f i l e s  * * * *
 ***********************************************************/

#include "cmincld.h"
#include "httpdinc.h"
#include "htpdglb.h"
#include "tmrlgdef.h"
#include "tmrlgif.h"
#include "httpdtmr.h"
#include "httpd.h"
#ifdef INTOTO_CMS_SUPPORT
#include "sysgdef.h"
#endif  /* INTOTO_CMS_SUPPORT */

#define  HTTPD_MILLI_SECS_PER_SEC     (1000)
#define  HTTPD_SECS_PER_MINUTE        (60)

/************************************************************
 * * * *  V a r i a b l e    D e c l a r a t i o n s  * * * *
 ************************************************************/

/********************************************************
 * * * *  F u n c t i o n    P r o t o t y p e s  * * * *
 ********************************************************/

/**
 *
 */

int32_t Httpd_Timer_NodeInsert(HttpGlobals_t *pHttpGlobals,Httpd_Timer_t  *ptimerId );

/**
 *
 */

int32_t Httpd_Timer_NodeDelete( Httpd_Timer_t  *ptimerId );

/**
 *
 */

void Httpd_Timer_NodeUpdate(HttpGlobals_t *pHttpGlobals,uint32_t  uiTimerType,
                               int32_t   lDiff );

/**
 *
 */

uint32_t Httpd_Timer_GetSysUptime( uint32_t  uiTimerType );

#ifdef HTTPD_DEBUG
/**
 *
 */

void Httpd_Timer_PrintNodeInfo( Httpd_Timer_t  *pTimerId );

/**
 *
 */

void Httpd_Timer_ListByType(HttpGlobals_t *pHttpGlobals,int32_t  nType );

/**
 *
 */

void Httpd_Timer_ListAll(HttpGlobals_t *pHttpGlobals);
#endif /* HTTPD_DEBUG */

/****************************************************************
 * * * *  F u n c t i o n    I m p l e m e n t a t i o n  * * * *
 ****************************************************************/

/***********************************************************************
 * Function Name : Httpd_Timer_Create                                  *
 * Description   : Allocates a new timer structure                     *
 * Input         :                                                     *
 * Output        : ptimerId - Contains a valid pointer on success     *
 * Return Value  :  OF_SUCCESS on success, otherwise OF_FAILURE.         *
 ***********************************************************************/

int32_t Httpd_Timer_Create( Httpd_Timer_t  **ptimerId )
{
  void *pVoid;

  if ((pVoid = (void *)of_calloc(1, sizeof(Httpd_Timer_t))) == NULL)
  {
    Httpd_Error("Httpd_TimerCreate: Unable to allocate memory");
    return OF_FAILURE;
  }

  *ptimerId = pVoid;
  return OF_SUCCESS;
} /* Httpd_Timer_Create() */

/***********************************************************************
 * Function Name : Httpd_Timer_Start                                   *
 * Description   : This starts a new timer with the given values       *
 * Input         : ptimerId - Pointer to the timer id                  *
 *                 proutine - Timeout Routine                          *
 *                 parg     - Argument To That Routine                 *
 *                 uitimertype - type of timer ( sec / min )           *
 *                 uitimerValue- timeout value                         *
 *                 uidebugInfo -                                       *
 * Output        :                                                     *
 * Return Value  :  OF_SUCCESS on success, otherwise OF_FAILURE.         *
 ***********************************************************************/

int32_t Httpd_Timer_Start( Httpd_Timer_t      *ptimerId,
                           HTTPD_TIMER_CBFUN  proutine,
                           void             *parg,
                           uint32_t           uiTimerType,
                           uint32_t           uitimerValue,
                           uint32_t           uidebugInfo )
{
 HttpGlobals_t *pHttpg=NULL;
 
 HTTPD_ASSERT(!ptimerId); 
 pHttpg=pHTTPG;
 if(!pHttpg)
 {
     Httpd_Error("pHttpg is NULL\r\n");
     return OF_FAILURE;
 }
  if (!Httpd_Timer_IsValidType(uiTimerType))
  {
    Httpd_Error("Invalid Timer Type.\n");
    return OF_FAILURE;
  }

  if (uitimerValue == 0)
  {
  return OF_SUCCESS;
  }

  if (uiTimerType != HTTPD_TIMER_SEC)
  {
    Httpd_Error("This Type of Timer is Not Supported\n");
    Httpd_Print("Type = %u\n", uiTimerType);
    return OF_FAILURE;
  }

  if ( Httpd_Timer_IsActive(ptimerId, uiTimerType) )
  {
    Httpd_Debug("Httpd_Timer_Start :: Deleting Existing Timer Node...\n");

    if (Httpd_Timer_NodeDelete(ptimerId) != 0)
    {
      return OF_FAILURE;
    }
  }

  ptimerId->pRoutine = proutine;
  ptimerId->pArg     = parg;
  ptimerId->Type     = uiTimerType;
  ptimerId->Value    = uitimerValue;
  ptimerId->iActive  = 0;

#ifdef HTTPD_DEBUG
  Httpd_Print("New Timer is Started with count %ld\n", uitimerValue);
#endif /* HTTPD_DEBUG */

  /**
   * Insert the current node.
   */
  if (Httpd_Timer_NodeInsert(pHttpg,ptimerId) < 0)
  {
    Httpd_Error("Httpd_Timer_Start :: Unable To Insert Node\n");
    return OF_FAILURE;
  }

  return OF_SUCCESS;
} /* Httpd_Timer_Start() */

/***********************************************************************
 * Function Name : Httpd_Timer_NodeInsert                              *
 * Description   : This inserts a timer node in the local timer list   *
 * Input         : ptimerId - Pointer to the timer id(Node)            *
 * Output        :                                                     *
 * Return Value  :  OF_SUCCESS on success, otherwise OF_FAILURE.         *
 ***********************************************************************/


int32_t Httpd_Timer_NodeInsert(HttpGlobals_t *pHttpGlobals,Httpd_Timer_t  *ptimerId )
{
  Httpd_Timer_t  *pcurrent;
  uint32_t        uicumuTime;

  uint8_t         uiType;

  uicumuTime = 0;


  HTTPD_ASSERT(!ptimerId);

  /**
   * Get the type of timer you are dealing with and use it as a key for
   * pointer manipulations on all the four types of linked lists.
   */
  uiType = ptimerId->Type;

  ptimerId->iActive = 1;

  /**
   * If this happens to be the first node.
   */
  if (!pHttpGlobals->pHttpdTmrLst [uiType])
  {
    pHttpGlobals->pHttpdTmrLst [uiType]         = ptimerId;
    pHttpGlobals->pHttpdTmrLst [uiType]->pNext  = NULL;
    pHttpGlobals->pHttpdTmrLst [uiType]->pPrev  = NULL;
    ptimerId->Count               = ptimerId->Value;

    return OF_SUCCESS;
  }
  else
  {
    pcurrent = pHttpGlobals->pHttpdTmrLst[uiType];

    while (1)
    {
      /**
       * If the total time is less than or equal to the cumulative time
       * then insert the node before the current node.
       */
      uicumuTime = uicumuTime + pcurrent -> Count;

      if ((ptimerId -> Value) <= uicumuTime)
      {
        ptimerId->pNext = pcurrent;
        ptimerId->pPrev = pcurrent->pPrev;

        if (pcurrent->pPrev)
        {
          pcurrent->pPrev->pNext = ptimerId;
        }
        else
        {
          pHttpGlobals->pHttpdTmrLst [uiType] = ptimerId;
        }

        pcurrent->pPrev = ptimerId;
        ptimerId->Count = ptimerId->Value - (uicumuTime - (pcurrent->Count));
        pcurrent->Count = pcurrent->Count - ptimerId->Count;
        break;
      }

      /**
       * Put the node as the last one, if the current node is the last one
       * and the cumulative count is still less than the total count.
       */

      if (pcurrent->pNext == NULL)
      {
        pcurrent->pNext = ptimerId;
        ptimerId->pNext = NULL;
        ptimerId->pPrev = pcurrent;
        ptimerId->Count = ptimerId->Value - uicumuTime;
        break;
      }
      pcurrent = pcurrent->pNext;
    }
    return OF_SUCCESS;
  }
} /* Httpd_Timer_NodeInsert() */

/***********************************************************************
 * Function Name : Httpd_Timer_NodeDelete                              *
 * Description   : Deletes a timer node from the local timer list      *
 * Input         : ptimerId - Pointer to the timer id(Node)            *
 * Output        :                                                     *
 * Return Value  :  OF_SUCCESS on success, otherwise OF_FAILURE.         *
 ***********************************************************************/


int32_t Httpd_Timer_NodeDelete( Httpd_Timer_t  *ptimerId )
{
  uint8_t   uiType;

  HttpGlobals_t *pHttpg=NULL;
  
  HTTPD_ASSERT(!ptimerId);
  pHttpg=pHTTPG;
  if(!pHttpg)
 {
    Httpd_Error("pHttpg is NULL\r\n");
    return OF_FAILURE;
 }
  uiType            = ptimerId->Type;

  ptimerId->iActive = 0;

  if (ptimerId->pNext)
  {
    ptimerId->pNext->Count = ptimerId->pNext->Count + ptimerId->Count;
    ptimerId->pNext->pPrev = ptimerId->pPrev;
  }

  if (ptimerId->pPrev)
  {
    ptimerId->pPrev->pNext = ptimerId->pNext;
  }
  else
  {
    pHttpg->pHttpdTmrLst [uiType] = ptimerId->pNext;
  }

  return OF_SUCCESS;
} /* Httpd_Timer_NodeDelete() */

/***********************************************************************
 * Function Name : Httpd_Timer_Stop                                    *
 * Description   : Stops an active timer node                          *
 * Input         : ptimerId - Pointer to the timer id(Node)            *
 * Output        :                                                     *
 * Return Value  :  OF_SUCCESS on success, otherwise OF_FAILURE.         *
 ***********************************************************************/

int32_t Httpd_Timer_Stop( Httpd_Timer_t  *ptimerId )
{
  HTTPD_ASSERT(!ptimerId);

  if (Httpd_Timer_IsActive(ptimerId, ptimerId->Type))
  {
    if (Httpd_Timer_NodeDelete(ptimerId) < 0)
    {
      return OF_FAILURE;
    }
  }

  return OF_SUCCESS;
} /* Httpd_Timer_Stop() */

/***********************************************************************
 * Function Name : Httpd_Timer_Delete                                  *
 * Description   : Deletes the timer from the list                     *
 * Input         : ptimerId - Pointer to the timer id(Node)            *
 * Output        :                                                     *
 * Return Value  :  OF_SUCCESS on success, otherwise OF_FAILURE.         *
 ***********************************************************************/

int32_t Httpd_Timer_Delete( Httpd_Timer_t  *ptimerId )
{
  HTTPD_ASSERT(!ptimerId);
#ifdef HTTPD_DEBUG
  Httpd_Print("Time Node Delete Is Called!!\r\n");
#endif /* HTTPD_DEBUG */

  Httpd_Timer_Stop(ptimerId);
 
  of_free(ptimerId);
  return OF_SUCCESS;
} /* Httpd_Timer_Delete() */

/***********************************************************************
 * Function Name : Httpd_Timer_Restart                                 *
 * Description   : Restarts the active timer with same value           *
 * Input         : ptimerId - Pointer to the timer id(Node)            *
 * Output        :                                                     *
 * Return Value  :  OF_SUCCESS on success, otherwise OF_FAILURE.         *
 ***********************************************************************/

int32_t Httpd_Timer_Restart( Httpd_Timer_t  *ptimerId )
{
  HTTPD_ASSERT(!ptimerId);

  return Httpd_Timer_Start(ptimerId, ptimerId->pRoutine, ptimerId->pArg,
                           ptimerId->Type, ptimerId->Value,
                           ptimerId->uidebugInfo);
} /* Httpd_Timer_Restart() */

/***********************************************************************
 * Function Name : Httpd_Timer_RestartWithNewValue                     *
 * Description   : Restarts the active timer with new value            *
 * Input         : ptimerId   - Pointer to the timer id(Node)          *
 *                 uinewValue - new value with which to restart        *
 * Output        :                                                     *
 * Return Value  :  OF_SUCCESS on success, otherwise OF_FAILURE.         *
 ***********************************************************************/

int32_t Httpd_Timer_RestartWithNewValue( Httpd_Timer_t  *ptimerId,
                                         uint32_t       uinewValue )
{
  int32_t istatus;

  HTTPD_ASSERT(!ptimerId);

  if ((Httpd_Timer_IsActive(ptimerId, ptimerId->Type)) &&
      (Httpd_Timer_Stop(ptimerId)))
  {
    return OF_FAILURE;
  }

  /**
   * Initialize the duration of the timer to the new value passed and
   * re-start the timer.
   */
  ptimerId -> Value = uinewValue;

  istatus = Httpd_Timer_Start(ptimerId, ptimerId->pRoutine, ptimerId->pArg,
                              ptimerId->Type, ptimerId->Value,
                              ptimerId->uidebugInfo);

  return istatus;
} /* Httpd_Timer_RestartWithNewValue() */

/***********************************************************************
 * Function Name : Httpd_Timer_Init                                    *
 * Description   : Initializes the internal timer structures           *
 * Input         :                                                     *
 * Output        :                                                     *
 * Return Value  :  OF_SUCCESS on success, otherwise OF_FAILURE.         *
 ***********************************************************************/

int32_t Httpd_Timer_Init( HttpGlobals_t  *pHttpGlobals)
{
 int32_t uitmr;
 
 int8_t  MaxTimerLists; 
 
 MaxTimerLists=Httpd_GetMaxTimerLists(pHttpGlobals);/*pHttpGlobals->pHttpGlobalConfig->iMaxTimerLists; */
 pHttpGlobals->pHttpdTmrLst=(Httpd_Timer_t **)of_calloc(MaxTimerLists,sizeof(Httpd_Timer_t *)); 
 
 if(!pHttpGlobals->pHttpdTmrLst)
 {
	  printf("allocation  failed %d\n\r",MaxTimerLists);
	 return OF_FAILURE;
 }
  for (uitmr = 0; uitmr < MaxTimerLists; uitmr++)  /* MAX_TIMER_LISTS */
  {
    pHttpGlobals->pHttpdTmrLst [uitmr] = NULL;
  }

  /**
   * Get the current system up time.
   */
  pHttpGlobals->HttpdLastUpdate = Httpd_Timer_GetSysUptime(HTTPD_TIMER_SEC);

  return OF_SUCCESS;
} /* Httpd_Timer_Init() */

/***********************************************************************
 * Function Name : Httpd_Timer_GetRemainingTime                        *
 * Description   : Gives the remaining time of a timer                 *
 * Input         : ptimerId      - Pointer to the timer id(Node)       *
 * Output        : pgetRemngTime - holds remaining time on success     *
 * Return Value  :  OF_SUCCESS on success, otherwise OF_FAILURE.         *
 ***********************************************************************/

int32_t Httpd_Timer_GetRemainingTime( Httpd_Timer_t  *ptimerId,
                                      uint32_t       *pgetRemngTime )
{
  Httpd_Timer_t  *pcurrent;
  uint32_t       uicumuTime;

  HTTPD_ASSERT(!ptimerId);

  if (!ptimerId->iActive)
  {
    *pgetRemngTime = 0;
    return OF_FAILURE;
  }

  uicumuTime = 0;
  pcurrent = ptimerId;

  while (1)
  {
    uicumuTime = uicumuTime + pcurrent->Count;

    if (!pcurrent->pPrev)
    {
      *pgetRemngTime = uicumuTime;
      break;
    }
    pcurrent = pcurrent -> pPrev;
  }
  return OF_SUCCESS;
} /* Httpd_Timer_GetRemainingTime() */

/***********************************************************************
 * Function Name : Httpd_Timer_IsActive                                *
 * Description   : Tells whether given timer is active or not          *
 * Input         : ptimerId      - Pointer to the timer id(Node)       *
 *                 uiTimerType   - Type of timer                       *
 * Output        :                                                     *
 * Return Value  :  TRUE if it is active                               *
 *                  FALSE if it is not                                 *
 ***********************************************************************/

bool Httpd_Timer_IsActive( Httpd_Timer_t  *ptimerId,
                              uint8_t        uiTimerType )
{
 HttpGlobals_t *pHttpg=NULL;
 Httpd_Timer_t  *pCurrent=NULL;

 pHttpg=pHTTPG;
 if(!pHttpg)
 {
    Httpd_Error("pHttpg is NULL\r\n");
    return OF_FAILURE;
 }
 pCurrent = pHttpg->pHttpdTmrLst[uiTimerType];

  HTTPD_ASSERT(!ptimerId);

  while (pCurrent)
  {
    if (pCurrent == ptimerId)
    {
      break;
    }

    pCurrent = pCurrent->pNext;  /** SRINI - SEP 19 **/
  }

  if (!pCurrent)
  {
    return FALSE;
  }
  else
  {
    return TRUE;
  }
} /* Httpd_Timer_IsActive() */

/***********************************************************************
 * Function Name : Httpd_Timer_GetDuration                             *
 * Description   : Tells initial count with which timer was started    *
 * Input         : ptimerId  - Pointer to the timer id(Node)           *
 * Output        :                                                     *
 * Return Value  : Initial timeout value                                *
 ***********************************************************************/

uint32_t Httpd_Timer_GetDuration( Httpd_Timer_t  *ptimerId )
{
  return ptimerId->Value;
} /* Httpd_Timer_GetDuration() */

/***********************************************************************
 * Function Name : Httpd_Timer_GetCbFun                                *
 * Description   : Gives the callback function of a timer              *
 * Input         : ptimerId  - Pointer to the timer id(Node)           *
 * Output        :                                                     *
 * Return Value  : Returns timeout function                            *
 ***********************************************************************/

HTTPD_TIMER_CBFUN Httpd_Timer_GetCbFun( Httpd_Timer_t  *ptimerId )
{
  return ptimerId->pRoutine;
} /* Httpd_Timer_GetCbFun() */

/***********************************************************************
 * Function Name : Httpd_Timer_GetCallbackArg                          *
 * Description   : Gives the callback argument of a timer              *
 * Input         : ptimerId  - Pointer to the timer id(Node)           *
 * Output        :                                                     *
 * Return Value  : Returns callback argument                           *
 ***********************************************************************/

void *Httpd_Timer_GetCallbackArg( Httpd_Timer_t  *ptimerId )
{
  return ptimerId->pArg;
} /* Httpd_Timer_GetCallbackArg() */

/***********************************************************************
 * Function Name : Httpd_Timer_SetCallbackArg                          *
 * Description   : Sets the callback argument of a timer               *
 * Input         : ptimerId  - Pointer to the timer id(Node)           *
 *                 parg      - New argument                            *
 * Output        :                                                     *
 * Return Value  : NONE                                                *
 ***********************************************************************/

void Httpd_Timer_SetCallbackArg( Httpd_Timer_t  *ptimerId,
                                   void         *parg )
{
  ptimerId->pArg = parg;
} /* Httpd_Timer_SetCallbackArg() */

/***********************************************************************
 * Function Name : Httpd_Timer_NodeUpdate                              *
 * Description   : Updates the timer list with the given elapsed time  *
 *                 Callback functions are called for timeout timers    *
 * Input         : uiTimerType  - Type of the timer list to update     *
 *                 lDiff        - Elapsed time                         *
 * Output        :                                                     *
 * Return Value  : NONE                                                *
 ***********************************************************************/


void Httpd_Timer_NodeUpdate(HttpGlobals_t *pHttpGlobals,uint32_t  uiTimerType,
                               int32_t   lDiff )
{
  Httpd_Timer_t *headPtr;

#ifdef HTTPD_DEBUG
  Httpd_Print("Httpd Timer Node Update With %d seconds\r\n", lDiff);
#endif /* HTTPD_DEBUG */
  if (uiTimerType != HTTPD_TIMER_SEC)
  {
    Httpd_Error("This Type of Timer is Not Supported\n");
    return;
  }

  headPtr = pHttpGlobals->pHttpdTmrLst[uiTimerType];

  if (headPtr)
  {
    headPtr->Count -= lDiff;

    if (headPtr->Count < 0)
    {
      headPtr->Count = 0;
    }
  }

  while (headPtr && headPtr->Count <= 0)
  {
    Httpd_Timer_NodeDelete(headPtr);
    headPtr->pRoutine (headPtr->pArg);
    headPtr = pHttpGlobals->pHttpdTmrLst[uiTimerType];
  }
} /* Httpd_Timer_NodeUpdate() */

/***********************************************************************
 * Function Name : Httpd_Timer_Task                                    *
 * Description   : This checks the elapsed time and calls timer node   *
 *                 update on the required lists                        *
 *                 This functions has to be called periodically        *
 * Input         : NONE                                                *
 * Output        :                                                     *
 * Return Value  : NONE                                                *
 ***********************************************************************/

void Httpd_Timer_Task(HttpGlobals_t *pHttpGlobals)
{
  time_t   CurTime;
  int32_t  lDiff = 0;

  /**
   * Get the current system up time.
   */
  CurTime = Httpd_Timer_GetSysUptime(HTTPD_TIMER_SEC);

  lDiff = CurTime - pHttpGlobals->HttpdLastUpdate;

  /**
   * System time might have been updated to past by admin.
   */
  if (lDiff < 0)
  {
    /**
     * Just reset and return.
     */
    Httpd_Debug("Httpd_Timer_Task:: System Time Might Have Been Modified. Resetting...");

    /**
     * Get the current system up time.
     */
    pHttpGlobals->HttpdLastUpdate = Httpd_Timer_GetSysUptime(HTTPD_TIMER_SEC);
    return;
  }

  if (lDiff > 1)
  {
    Httpd_Timer_NodeUpdate(pHttpGlobals,HTTPD_TIMER_SEC, lDiff);
    pHttpGlobals->HttpdLastUpdate = CurTime;
  }
  /**
   * Difference may be less than a second, so just ignore this time.
   */
} /* Httpd_Timer_Task() */

/***********************************************************************
 * Function Name : Httpd_Timer_CallOut                                 *
 * Description   : Calls the callback function of the timer            *
 * Input         : ptimerId  - Pointer to the timer id(Node)           *
 * Output        :                                                     *
 * Return Value  : NONE                                                *
 ***********************************************************************/

void Httpd_Timer_CallOut( Httpd_Timer_t  *pTimerId )
{
  pTimerId->pRoutine(pTimerId->pArg);
} /* Httpd_Timer_CallOut() */

/***********************************************************************
 * Function Name : Httpd_Timer_CallOut                                 *
 * Description   : Calls the callback function of the timer            *
 * Input         : ptimerId  - Pointer to the timer id(Node)           *
 * Output        :                                                     *
 * Return Value  : NONE                                                *
 ***********************************************************************/

uint32_t Httpd_Timer_FirstNodeVal(HttpGlobals_t *pHttpGlobals)
{
  Httpd_Timer_t *headPtr;

  headPtr = pHttpGlobals->pHttpdTmrLst[HTTPD_TIMER_SEC];

  if (!headPtr)
  {
    return 0;
  }
  else
  {
    return headPtr->Count;
  }
} /* Httpd_Timer_FirstNodeVal() */

/***********************************************************************
 * Function Name : Httpd_TimerUpdate                                   *
 * Description   : This function is used to set the current system up  *
 *                 time in the global variable that is used by the     *
 *                 functionality.                                      *
 * Input         : pHttpGlobals  - Global pointer                      *
 * Output        : None                                                *
 * Return Value  : None                                                *
 ***********************************************************************/

void Httpd_TimerUpdate(HttpGlobals_t *pHttpGlobals)
{
  /**
   * Get the current system up time.
   */
   if(!pHttpGlobals)
  {
    Httpd_Error("pHttpGlobals is NULL\r\n");	
    return;
  }	 
  pHttpGlobals->HttpdLastUpdate = Httpd_Timer_GetSysUptime(HTTPD_TIMER_SEC);
}

/***********************************************************************
 * Function Name : Httpd_Timer_GetSysUptime                            *
 * Description   : This function returns the current system time for   *
 *                 the given timer type.                               *
 * Input         : uiTimerType (i) - type of the timer.                *
 * Output        : None                                                *
 * Return Value  : Current timeout value based on the given timer type.*
 ***********************************************************************/


uint32_t Httpd_Timer_GetSysUptime( uint32_t  uiTimerType )
{
  SysConf_t  sysConfig;
  uint32_t   uiSysUpTime = 0;
  uint32_t   ulTicks2Sec = 0;
  /**
   * Intialize the sysConfig variable.
   */
  of_memset(&sysConfig, 0, sizeof(SysConf_t));
  of_memset(&ulTicks2Sec, 0, sizeof(ulTicks2Sec));
  /**
   * Get the current system uptime.
   */
   cm_lib_timer_get_time_stamp(&sysConfig.uiTimeTicks);
  
#ifdef INTOTO_CMS_SUPPORT
  if(Getjiffies(&sysConfig.uiTimeTicks) == OF_FAILURE) 
  {
   return OF_FAILURE;
  }
  
  if(Httpdget_ticks_per_sec(&ulTicks2Sec) == OF_FAILURE)
  {
   return OF_FAILURE;
  }
#else
  Httpdget_ticks_per_sec(&ulTicks2Sec);
#endif  /* INTOTO_CMS_SUPPORT */

  switch (uiTimerType)
  {
  case HTTPD_TIMER_PERIODIC:
    uiSysUpTime = sysConfig.uiTimeTicks;
    break;
  case HTTPD_TIMER_MSEC:
    /**
     * Get the time in seconds and multiply by 1000 to get time in milli
     * seconds.
     */
    uiSysUpTime = (sysConfig.uiTimeTicks / ulTicks2Sec) * HTTPD_MILLI_SECS_PER_SEC;
    break;
  case HTTPD_TIMER_SEC:
    /**
     * Get the time in seconds.
     */
    uiSysUpTime = sysConfig.uiTimeTicks / ulTicks2Sec;
    break;
  case HTTPD_TIMER_MIN:
    /**
     * Get the time in seconds and divide it by 60 second to get the time
     * in minutes.
     */
    uiSysUpTime = (sysConfig.uiTimeTicks / ulTicks2Sec) / HTTPD_SECS_PER_MINUTE;
    break;
  }
  return uiSysUpTime;
} /* Httpd_Timer_GetSysUptime() */

#ifdef HTTPD_DEBUG

/***********************************************************************
 * Function Name : Httpd_Timer_CallOut                                 *
 * Description   : Calls the callback function of the timer            *
 * Input         : ptimerId  - Pointer to the timer id(Node)           *
 * Output        :                                                     *
 * Return Value  : NONE                                                *
 ***********************************************************************/


void Httpd_Timer_PrintNodeInfo( Httpd_Timer_t  *pTimerId )
{
  HTTPD_ASSERT(!pTimerId);
  Httpd_Print("Info of Timer-Node at %p\n", pTimerId);
  Httpd_Print("   pRoutine     =  %lx\n", (uint32_t)pTimerId->pRoutine);
  Httpd_Print("   pArg         =  %lx\n", (uint32_t)pTimerId->pArg);
  Httpd_Print("   Count        =  %ld\n", pTimerId->Count);
  Httpd_Print("   Value        =  %d\n", pTimerId->Value);
  Httpd_Print("   Type         =  %ld\n", pTimerId->Type);
  Httpd_Print("   bDynamic     =  %d\n", pTimerId->bDynamic);
  Httpd_Print("   uidebuginfo  =  %ld\n", pTimerId->uidebugInfo);
  Httpd_Print("   iActive      =  %ld\n", pTimerId->iActive);
  Httpd_Print("   pNext        =  %lx\n", (uint32_t)pTimerId->pNext);
  Httpd_Print("   pPrev        =  %lx\n", (uint32_t)pTimerId->pPrev);
}

/***********************************************************************
 * Function Name : Httpd_Timer_CallOut                                 *
 * Description   : Calls the callback function of the timer            *
 * Input         : ptimerId  - Pointer to the timer id(Node)           *
 * Output        :                                                     *
 * Return Value  : NONE                                                *
 ***********************************************************************/


void Httpd_Timer_ListByType(HttpGlobals_t *pHttpGlobals,int32_t  nType )
{
  Httpd_Timer_t  *temp;

  HTTPD_ASSERT(!Httpd_Timer_IsValidType(nType));

  temp = pHttpGlobals->pHttpdTmrLst[nType];

  while (temp)
  {
    Httpd_Timer_PrintNodeInfo( temp );
    temp = temp->pNext;
  }
  return;
} /* Httpd_Timer_ListByType() */

/***********************************************************************
 * Function Name : Httpd_Timer_CallOut                                 *
 * Description   : Calls the callback function of the timer            *
 * Input         : ptimerId  - Pointer to the timer id(Node)           *
 * Output        :                                                     *
 * Return Value  : NONE                                                *
 ***********************************************************************/


void Httpd_Timer_ListAll(HttpGlobals_t *pHttpGlobals)
{
  int32_t i;
  int8_t  MaxTimerLists;    /* MAX_TIMER_LISTS */
  MaxTimerLists=Httpd_GetMaxTimerLists(pHttpGlobals);/*pHttpGlobals->pHttpGlobalConfig->iMaxTimerLists; */
 for (i = 0; i < MaxTimerLists; i++)  
  {
    if (pHttpGlobals->pHttpdTmrLst[i])
    {
      Httpd_Timer_ListByType(pHttpGlobals,i);
    }
  }
} /* Httpd_Timer_ListAll() */

#endif /* HTTPD_DEBUG */

#endif /* OF_HTTPD_SUPPORT */


