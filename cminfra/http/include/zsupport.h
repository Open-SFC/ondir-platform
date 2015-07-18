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
/*  File        : zsupport.h                                              */
/*                                                                        */
/*  Description : This file contains zone related declarations and        */
/*                function prototypes used in the https callback          */
/*                functions.                                              */
/*                                                                        */
/*  Version      Date      Author      Change Description                 */
/*  -------    --------    ------    ----------------------               */
/*    1.1      05.14.04    DRAM      Modified during the code review.     */
/**************************************************************************/

#ifndef _ZONE_SUPPORT_H_
#define _ZONE_SUPPORT_H_

/****************************************************
 * * * *  M a c r o    D e f i n i t i o n s  * * * *
 ****************************************************/

#define  HTTP_SELECT_ZONE             (9)

/****************************************************
 * * * *  T y p e    D e c l a r a t i o n s  * * * *
 ****************************************************/

/**
 *
 */
typedef struct  Httpzoneinfo_s
{
#define  HTTP_MAX_INDEX               (40)
#define  HTTP_MAX_DATA                (70)

  char  index[HTTP_MAX_INDEX];
  char  data[HTTP_MAX_DATA];
} Httpzoneinfo_t;

/********************************************************
 * * * *  F u n c t i o n    P r o t o t y p e s  * * * *
 ********************************************************/

int32_t HTTPSetzone( char  *str );

int32_t HTTPGetzone( char  *str );

#endif /* _ZONE_SUPPORT_H_ */

