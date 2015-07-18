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
/**************************************************************************/
/*  File        : httpdinc.h                                              */
/*                                                                        */
/*  Description : This file contains include entries used by the http     */
/*                server for the lx flavor.                               */
/*                                                                        */
/*  Version      Date      Author      Change Description                 */
/*  -------    --------    ------    ----------------------               */
/*    1.1      05.14.04    DRAM      Modified during the code review.     */
/**************************************************************************/

#ifndef _HTTPDINC_H_
#define _HTTPDINC_H_

/***********************************************************
 * * * *  I n c l u d e    H e a d e r    f i l e s  * * * *
 ***********************************************************/

/*#include "ucmcmn.h"
#include "cmincld.h"
#include "ucmnet.h"*/
#include "httpwrp.h"

#ifndef __KERNEL__
//#include "uncomp.h"   /* for Unzip Library */
#include "zsupport.h"
#include "cmfile.h"
#include "certcfg.h"
#endif
#ifndef TR_IMPORT
#define TR_IMPORT extern
#endif /* TR_IMPORT */

#ifdef OF_HTTPS_SUPPORT
/*#include "openssl/openssl.h"*/
#include "openssl/ssl.h"
#include "hcmgrmbl.h"
#include "hcmgrgdf.h"
#include "hcmgrgif.h"
#include "httpswrp.h"
#endif /* OF_HTTPS_SUPPORT */

/****************************************************
 * * * *  M a c r o    D e f i n i t i o n s  * * * *
 ****************************************************/

#define  HTTP_MPART_LEN  (13*1024)

/********************************************************
 * * * *  F u n c t i o n    P r o t o t y p e s  * * * *
 ********************************************************/
#ifndef OF_CM_SUPPORT
extern
int32_t  IGWGetSubscriberInfoOnIP(IGWSubscriberInfo_t	*pInfo);

extern
void IGWSetSockReuseOpt( IGWSOCKHANDLE  iSocketFd );

extern
int32_t IGWSocketGetSockName( IGWSOCKHANDLE   iSocketFd,
                              ipaddr          *pIPAddress,
                              uint16_t        *pPort );
#endif /*OF_CM_SUPPORT*/

#endif /* _HTTPDINC_H_ */

