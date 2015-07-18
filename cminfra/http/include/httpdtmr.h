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
/*  File        : httpdtmr.h                                              */
/*                                                                        */
/*  Description : This file contains timer related declarations.          */
/*                                                                        */
/*  Version      Date      Author      Change Description                 */
/*  -------    --------    ------    ----------------------               */
/*    1.0      00.00.00    kmdhar    Initial implementation               */
/*    1.1      05.11.04    DRAM      Modified during the code review.     */
/**************************************************************************/

#ifndef __HTTPDTMR_H_
#define __HTTPDTMR_H_

/****************************************************
 * * * *  M a c r o    D e f i n i t i o n s  * * * *
 ****************************************************/

#define  HTTPD_TIMER_PERIODIC         ( 0 )
#define  HTTPD_TIMER_MSEC             ( 1 )
#define  HTTPD_TIMER_SEC              ( 2 )
#define  HTTPD_TIMER_MIN              ( 3 )

/* #define  MAX_TIMER_LISTS              ( 4 )  moved to system directory */

#define  Httpd_Timer_IsValidType(a)   (( (a) >= HTTPD_TIMER_PERIODIC ) && \
                                       ( (a) <= HTTPD_TIMER_MIN))

#define  SYS_VERSION_ID_LENGTH         9
#define  SYS_DISPLAY_STRING_LENGTH     255
#define  SYS_HOST_NAME_LEN             63
#define  SYS_DOMAIN_NAME_LEN           255

/****************************************************
 * * * *  T y p e    D e c l a r a t i o n s  * * * *
 ****************************************************/
typedef struct SysConf_s
{
  unsigned char  ucDescr[SYS_DISPLAY_STRING_LENGTH + 1];
  uint32_t  VerId[SYS_VERSION_ID_LENGTH];
  uint32_t  uiTimeTicks;
  unsigned char  ucContact[SYS_DISPLAY_STRING_LENGTH + 1];
  unsigned char  ucName[SYS_HOST_NAME_LEN + 1];
  unsigned char  ucLocation[SYS_DISPLAY_STRING_LENGTH + 1];
  uint32_t  uiServices;
  char   DomainName[SYS_DOMAIN_NAME_LEN + 1];
}SysConf_t;
 
/********************************************************
 * * * *  F u n c t i o n    P r o t o t y p e s  * * * *
 ********************************************************/

/**
 * T i m e r    R e l a t e d    S t u f f.
 */

/**
 *
 */
int32_t  Httpd_Timer_Init(HttpGlobals_t *pHttpGlobals);

/**
 *
 */
void  Httpd_Timer_Task(HttpGlobals_t *pHttpGlobals);

/**
 *
 */
int32_t  Httpd_Timer_Create( Httpd_Timer_t  **ptimerId );

/**
 *
 */
int32_t  Httpd_Timer_Start( Httpd_Timer_t      *ptimerId,
                            HTTPD_TIMER_CBFUN  pRoutine,
                            void             *pArg,
                            uint32_t           uitimerType,
                            uint32_t           uitimerValue,
                            uint32_t           uidebugInfo );

/**
 *
 */
int32_t  Httpd_Timer_Stop( Httpd_Timer_t  *ptimerId );

/**
 *
 */
int32_t  Httpd_Timer_Delete( Httpd_Timer_t  *ptimerId );

/**
 *
 */
int32_t  Httpd_Timer_Restart( Httpd_Timer_t  *ptimerId );

/**
 *
 */
int32_t  Httpd_Timer_RestartWithNewValue( Httpd_Timer_t  *ptimerId,
                                          uint32_t       uinewValue );

/**
 *
 */
bool  Httpd_Timer_IsActive( Httpd_Timer_t  *ptimerId,
                               uint8_t        uitimerType );

/**
 *
 */
int32_t  Httpd_Timer_GetRemainingTime( Httpd_Timer_t  *ptimerId,
                                       uint32_t       *pgetRemngTime );

/**
 *
 */
uint32_t  Httpd_Timer_GetDuration( Httpd_Timer_t  *ptimerId );

/**
 *
 */
void  *Httpd_Timer_GetCallbackArg( Httpd_Timer_t  *ptimerId );

/**
 *
 */
void  Httpd_Timer_SetCallbackArg( Httpd_Timer_t  *ptimerId,
                                    void         *parg );

/**
 *
 */
void  Httpd_Timer_CallOut( Httpd_Timer_t  *pTimerId );

/**
 *
 */
uint32_t  Httpd_Timer_FirstNodeVal(HttpGlobals_t *pHttpGlobals);

/**
 *
 */
HTTPD_TIMER_CBFUN  Httpd_Timer_GetCbFun( Httpd_Timer_t  *ptimerId );

#endif /* __HTTPDTMR_H_ */

