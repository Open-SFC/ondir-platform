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
/*  File        : httpcfg.h                                               */
/*                                                                        */
/*  Description : This file contains global configuration for http server */
/*                related to lxpc system.                                 */
/*                                                                        */
/*  Version      Date      Author      Change Description                 */
/*  -------    --------    ------    ----------------------               */
/**************************************************************************/

#ifndef __HTTPCFG_H
#define __HTTPCFG_H

/****************************************************
 * * * *  M a c r o    D e f i n i t i o n s  * * * *
 ****************************************************/
#ifdef INTOTO_UCM_SUPPORT
extern
int  IGWGetSubscriberInfoOnIP(IGWSubscriberInfo_t	*pInfo);
#endif


#endif /* __HTTPCFG_H */
