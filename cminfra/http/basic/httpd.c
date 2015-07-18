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
/*  File        : httpd.c                                                 */
/*                                                                        */
/*  Description : This file contains core functions related to the http   */
/*                server module, which includes, protocol implementation, */
/*                session management, etc.                                */
/*                                                                        */
/*  Version      Date      Author      Change Description                 */
/*  -------    --------    ------    ----------------------               */
/*    1.0      00.00.00    kmdhar    Initial implementation               */
/*    1.1      05.05.04    DRAM      Modified during the code review.     */
/*    1.2      01.11.04    RamaRao   Modified during concurrent behvior   */
/*                                   implementation     		          */
/*    1.3      17.08.05    Harishp   Modified during Persistence And 	  */	
/*				                     Pipelining HTTP/1.1 behvior       	  */
/*				                     implementation 	  		          */
/*    1.4      22.02.06    Harishp   Modified during MultipleInstances	  */
/*                                   and Multithreading	                  */
/**************************************************************************/

#ifdef OF_HTTPD_SUPPORT

/***********************************************************
 * * * *  I n c l u d e    H e a d e r    f i l e s  * * * *
 ***********************************************************/

#include "cmincld.h"
#include "httpdinc.h"
#include "fcntl.h"
#include "errno.h"
#include "basic_tunable.h"
#include "htpdglb.h"
#include "httpdtmr.h"
#include "httpd.h"
#include "httpwrp.h"
#include "httpgif.h"
#include "htmlpars.h"
#include "htpercod.h"
#include "lxoswrp.h"

#ifdef OF_HTTPS_SUPPORT
#include "httpswrp.h"
#include "hcmgrgif.h"
#endif /*OF_HTTPS_SUPPORT*/

#include "openssl/ssl.h"
/*#include "hcrtgif.h"*/

#ifdef OF_CM_SUPPORT
#include "ucmglue.h"
#endif /*OF_CM_SUPPORT*/


#ifdef INTOTO_CMS_SUPPORT
#include "htpcnfrc.h"
#include "httpswrp.h"
#include "confver.h"

#define CMS_ENABLE_LOGIN
#endif  /* INTOTO_CMS_SUPPORT */

/****************************************************
 * * * *  M a c r o    D e f i n i t i o n s  * * * *
 ****************************************************/ 
/****************************************************
 * * * *  T y p e    D e c l a r a t i o n s  * * * *
 ****************************************************/
 
#define NULL_MIMETYPE ((hMimeType_t *)NULL)
#define MIME_HASH(iIndex) (isalpha(iIndex) ? (tolower(iIndex))- 'a': 26)
#define MBB_TEMPORARY_MSG_LENGTH (1024*13) /* 512*/ /* 60 */
#define MBB_ATA_DIRECTORY_LENGTH 128
//#define JNLP_SIZE 1024
#define JNLP_SIZE 2048
#define JNLP_FILE "/igateway/html/lv.jnlp"
extern bool bIsLogoFile;

#define LOGO_ERROR_STRING  "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<?xml-stylesheet type=\"text/xsl\" href=\"errscrn_en.xsl\"?>\n<body><Error_String>Logo is Not Used and Please restore the Original HTML files</Error_String></body>"


#define MASK_IP_SIZE 15
HttpdListenPorts_t HttpdListenPorts;

/********************************************************
 * * * * * * V a r i a b l e     A r r a y s * * * * * *
 ********************************************************/

 char month_snames[HTTP_NUMBER_OF_MONTHS][HTTP_MONTH_STRING_LEN] =
{
  "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct",
  "Nov", "Dec"
};

 char day_snames[HTTP_NUMBER_OF_DAYS][HTTP_DAY_STRING_LEN] =
{
 "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};

uint32_t ulNonLogoFileReq_g = 1;
uint32_t ulLogoFileReq_g = 1;
/*Last accessed file application GET Params */
char   aPrevXmlUrlData[HTTP_FILENAME_LEN + 1];

 unsigned char LogoBuff_g[]= {
 0x47, 0x49, 0x46, 0x38, 0x39, 0x61, 0x3A, 0x00,
 0x2B, 0x00, 0xF7, 0x00, 0x00, 0xFF, 0xFF, 0xFF,
 0x00, 0x00, 0x00, 0xC8, 0x17, 0x1F, 0x1F, 0x04,
 0x05, 0xCA, 0x1C, 0x24, 0xCA, 0x1D, 0x25, 0xCF,
 0x20, 0x28, 0xCC, 0x26, 0x2D, 0xCB, 0x27, 0x2E,
 0xD6, 0x2A, 0x32, 0xCD, 0x2A, 0x31, 0xE0, 0x30,
 0x38, 0xDA, 0x2F, 0x36, 0xCE, 0x2E, 0x35, 0xCF,
 0x34, 0x3B, 0x22, 0x09, 0x0A, 0xD3, 0x4A, 0x50,
 0xD4, 0x4C, 0x52, 0xD3, 0x4F, 0x55, 0xD5, 0x50,
 0x56, 0xD6, 0x55, 0x5B, 0xD7, 0x59, 0x5E, 0xD9,
 0x62, 0x67, 0xDB, 0x6E, 0x73, 0xDE, 0x7A, 0x7E,
 0xE1, 0x86, 0x8A, 0xE3, 0x8C, 0x90, 0xE3, 0x8F,
 0x92, 0xCF, 0x8E, 0x91, 0xE7, 0xA0, 0xA3, 0xEA,
 0xAE, 0xB0, 0xEC, 0xB1, 0xB3, 0xED, 0xB9, 0xBB,
 0xEB, 0xB7, 0xB9, 0xEF, 0xC2, 0xC4, 0xF0, 0xC4,
 0xC6, 0xDA, 0xB2, 0xB3, 0xF0, 0xC7, 0xC8, 0xF6,
 0xD5, 0xD6, 0xF2, 0xD1, 0xD2, 0xF5, 0xD9, 0xDA,
 0xF3, 0xD8, 0xD9, 0xF7, 0xDC, 0xDD, 0xF5, 0xDB,
 0xDC, 0xF6, 0xDF, 0xE0, 0xC9, 0x17, 0x20, 0xCA,
 0x18, 0x21, 0xCA, 0x1D, 0x26, 0xCA, 0x1E, 0x27,
 0xC9, 0x1E, 0x27, 0xCA, 0x1F, 0x28, 0xC7, 0x1F,
 0x27, 0xCB, 0x20, 0x29, 0xCB, 0x21, 0x2A, 0xCB,
 0x22, 0x2B, 0xCC, 0x23, 0x2C, 0xCC, 0x23, 0x2B,
 0xCC, 0x24, 0x2C, 0xD5, 0x27, 0x30, 0xD3, 0x26,
 0x2F, 0xCC, 0x25, 0x2E, 0xCC, 0x27, 0x30, 0xCC,
 0x27, 0x2F, 0x83, 0x19, 0x20, 0x9F, 0x1F, 0x27,
 0xD0, 0x29, 0x31, 0xCE, 0x29, 0x31, 0xC7, 0x27,
 0x2F, 0xCD, 0x29, 0x31, 0xCD, 0x2A, 0x33, 0xCB,
 0x2A, 0x32, 0x9C, 0x20, 0x26, 0xD3, 0x2C, 0x35,
 0xCE, 0x2B, 0x33, 0x6E, 0x17, 0x1C, 0x18, 0x05,
 0x06, 0xA3, 0x23, 0x2A, 0x91, 0x1F, 0x25, 0xD7,
 0x2F, 0x37, 0xCE, 0x2D, 0x35, 0xBA, 0x28, 0x2F,
 0xD2, 0x2E, 0x37, 0xCD, 0x2D, 0x37, 0xCD, 0x2D,
 0x35, 0xC3, 0x2B, 0x33, 0xBA, 0x2A, 0x31, 0xD8,
 0x31, 0x3A, 0xCE, 0x2F, 0x37, 0xCD, 0x2F, 0x37,
 0x42, 0x0F, 0x12, 0xCD, 0x32, 0x3B, 0xCE, 0x33,
 0x3C, 0xA3, 0x2A, 0x32, 0xCF, 0x36, 0x3E, 0xD0,
 0x3A, 0x42, 0xD1, 0x3F, 0x46, 0xD2, 0x40, 0x48,
 0xE2, 0x46, 0x4E, 0xD4, 0x44, 0x4B, 0xD2, 0x44,
 0x4B, 0xD4, 0x45, 0x4C, 0xD2, 0x45, 0x4C, 0xD4,
 0x49, 0x50, 0x39, 0x14, 0x16, 0xD5, 0x4D, 0x54,
 0xDA, 0x52, 0x59, 0xD5, 0x52, 0x59, 0xD6, 0x56,
 0x5C, 0xD8, 0x5D, 0x64, 0xB0, 0x4D, 0x53, 0x96,
 0x42, 0x47, 0xDA, 0x64, 0x6A, 0xE1, 0x69, 0x6F,
 0xDA, 0x69, 0x6F, 0xDA, 0x71, 0x77, 0xDD, 0x73,
 0x78, 0xDE, 0x75, 0x7A, 0xDE, 0x77, 0x7C, 0xC6,
 0x6C, 0x71, 0xDF, 0x7D, 0x82, 0xE1, 0x80, 0x85,
 0xC8, 0x75, 0x79, 0xE4, 0x90, 0x94, 0xE4, 0x95,
 0x99, 0xE6, 0x99, 0x9D, 0xE7, 0x9D, 0xA1, 0xE8,
 0xA3, 0xA7, 0xE9, 0xA6, 0xAA, 0xBB, 0x87, 0x8A,
 0xE9, 0xAA, 0xAD, 0xEE, 0xBC, 0xBF, 0xEF, 0xBF,
 0xC2, 0xF2, 0xC9, 0xCB, 0xBE, 0x9E, 0xA0, 0xF3,
 0xCD, 0xCF, 0xF4, 0xD3, 0xD5, 0xF3, 0xD4, 0xD6,
 0xF7, 0xE2, 0xE3, 0xBE, 0xAE, 0xAF, 0xF8, 0xE5,
 0xE6, 0x54, 0x12, 0x17, 0x26, 0x1B, 0x1C, 0xED,
 0xB9, 0xBD, 0x43, 0x37, 0x38, 0x33, 0x29, 0x2A,
 0xF3, 0xD0, 0xD4, 0xFB, 0xF1, 0xF2, 0xFE, 0xF5,
 0xF6, 0xFC, 0xF3, 0xF4, 0x0F, 0x03, 0x05, 0x28,
 0x22, 0x23, 0xF6, 0xF0, 0xF1, 0xFE, 0xFB, 0xFC,
 0xF7, 0xF4, 0xF5, 0x20, 0x1F, 0x20, 0x00, 0x04,
 0x04, 0x0A, 0x0E, 0x0E, 0x05, 0x07, 0x07, 0x13,
 0x18, 0x18, 0x31, 0x3A, 0x3A, 0x22, 0x27, 0x27,
 0x3E, 0x42, 0x42, 0x2C, 0x2E, 0x2E, 0xE0, 0xE5,
 0xE5, 0xEF, 0xF3, 0xF3, 0x54, 0x55, 0x55, 0xFD,
 0xFF, 0xFF, 0x5A, 0x5B, 0x5B, 0xFE, 0xFF, 0xFF,
 0xE4, 0xE5, 0xE5, 0xC5, 0xC6, 0xC6, 0xBF, 0xC0,
 0xC0, 0xB1, 0xB2, 0xB2, 0xB2, 0xBC, 0xBB, 0xF3,
 0xFC, 0xFB, 0x0B, 0x13, 0x12, 0xE5, 0xEB, 0xEA,
 0xD6, 0xD9, 0xD8, 0x83, 0x85, 0x84, 0x0B, 0x0C,
 0x0B, 0xFC, 0xF8, 0xF7, 0xFA, 0xEF, 0xEE, 0x09,
 0x01, 0x01, 0x06, 0x01, 0x01, 0x09, 0x02, 0x02,
 0xF8, 0xE8, 0xE8, 0xFA, 0xED, 0xED, 0xD0, 0xC5,
 0xC5, 0xFC, 0xF4, 0xF4, 0xFB, 0xF4, 0xF4, 0xFC,
 0xF7, 0xF7, 0xE4, 0xDF, 0xDF, 0xFD, 0xF9, 0xF9,
 0xFD, 0xFB, 0xFB, 0xFE, 0xFD, 0xFD, 0xFD, 0xFC,
 0xFC, 0xF5, 0xF4, 0xF4, 0xFA, 0xFA, 0xFA, 0xF9,
 0xF9, 0xF9, 0xF6, 0xF6, 0xF6, 0xF0, 0xF0, 0xF0,
 0xEA, 0xEA, 0xEA, 0xE2, 0xE2, 0xE2, 0xDF, 0xDF,
 0xDF, 0xDC, 0xDC, 0xDC, 0xD9, 0xD9, 0xD9, 0xD6,
 0xD6, 0xD6, 0xD1, 0xD1, 0xD1, 0xCC, 0xCC, 0xCC,
 0xBB, 0xBB, 0xBB, 0xB6, 0xB6, 0xB6, 0xAF, 0xAF,
 0xAF, 0xAA, 0xAA, 0xAA, 0xA6, 0xA6, 0xA6, 0xA0,
 0xA0, 0xA0, 0x99, 0x99, 0x99, 0x93, 0x93, 0x93,
 0x8E, 0x8E, 0x8E, 0x89, 0x89, 0x89, 0x87, 0x87,
 0x87, 0x84, 0x84, 0x84, 0x83, 0x83, 0x83, 0x80,
 0x80, 0x80, 0x7D, 0x7D, 0x7D, 0x78, 0x78, 0x78,
 0x74, 0x74, 0x74, 0x6D, 0x6D, 0x6D, 0x66, 0x66,
 0x66, 0x51, 0x51, 0x51, 0x4F, 0x4F, 0x4F, 0x4B,
 0x4B, 0x4B, 0x48, 0x48, 0x48, 0x44, 0x44, 0x44,
 0x43, 0x43, 0x43, 0x40, 0x40, 0x40, 0x3E, 0x3E,
 0x3E, 0x39, 0x39, 0x39, 0x33, 0x33, 0x33, 0x29,
 0x29, 0x29, 0x26, 0x26, 0x26, 0x25, 0x25, 0x25,
 0x24, 0x24, 0x24, 0x22, 0x22, 0x22, 0x1E, 0x1E,
 0x1E, 0x1A, 0x1A, 0x1A, 0x15, 0x15, 0x15, 0x13,
 0x13, 0x13, 0x0F, 0x0F, 0x0F, 0x0D, 0x0D, 0x0D,
 0x0B, 0x0B, 0x0B, 0x09, 0x09, 0x09, 0x06, 0x06,
 0x06, 0x05, 0x05, 0x05, 0x03, 0x03, 0x03, 0x01,
 0x01, 0x01, 0xFF, 0xFF, 0xFF, 0x21, 0xF9, 0x04,
 0x01, 0x00, 0x00, 0xFF, 0x00, 0x2C, 0x00, 0x00,
 0x00, 0x00, 0x3A, 0x00, 0x2B, 0x00, 0x00, 0x08,
 0xFF, 0x00, 0x01, 0x08, 0x1C, 0x48, 0xB0, 0xA0,
 0xC1, 0x83, 0x07, 0x83, 0x25, 0x72, 0xC4, 0x07,
 0x4F, 0x9C, 0x0A, 0x6A, 0xD4, 0x4C, 0x80, 0x30,
 0xE1, 0x4D, 0x06, 0x41, 0x92, 0x10, 0x6A, 0xDC,
 0xC8, 0x11, 0x00, 0x2F, 0x42, 0x7C, 0xEA, 0xCC,
 0xC9, 0xD0, 0x07, 0x84, 0x89, 0x5C, 0xB5, 0x24,
 0xE9, 0x62, 0x41, 0xC8, 0xCF, 0x9C, 0x31, 0x60,
 0xF4, 0x64, 0xEC, 0x48, 0x53, 0xA3, 0x0A, 0x3F,
 0x7A, 0xFA, 0x0C, 0xCA, 0x35, 0xAC, 0x23, 0x25,
 0x10, 0x15, 0x20, 0x10, 0xAA, 0x49, 0x54, 0x20,
 0xB0, 0x43, 0x81, 0x02, 0x1D, 0xF2, 0x55, 0x54,
 0x20, 0xAA, 0x0E, 0x5E, 0x44, 0x34, 0xE5, 0x48,
 0x0B, 0x11, 0x88, 0x11, 0xBA, 0xA6, 0x16, 0x74,
 0x04, 0x66, 0x85, 0xD6, 0x81, 0xA8, 0x04, 0x06,
 0x5B, 0x51, 0x02, 0x05, 0xB0, 0xAF, 0x06, 0xF7,
 0xBC, 0x11, 0xA6, 0x35, 0xD1, 0x9D, 0x14, 0x00,
 0x72, 0x9D, 0x60, 0x81, 0x09, 0xED, 0x41, 0x5F,
 0x11, 0x86, 0x16, 0x45, 0x15, 0xE8, 0x4B, 0x8B,
 0x0B, 0x8B, 0x58, 0x9C, 0xB5, 0x8B, 0x10, 0xC3,
 0x9D, 0xA2, 0xB9, 0xE6, 0xE4, 0xF0, 0x91, 0xE4,
 0x40, 0x08, 0xC2, 0x1B, 0xFB, 0x50, 0x60, 0x4B,
 0x93, 0x10, 0x04, 0x1A, 0x49, 0x9E, 0x3C, 0xC1,
 0x31, 0xA6, 0x16, 0x64, 0x84, 0x7F, 0xBE, 0x30,
 0xED, 0xF8, 0x07, 0x4B, 0x0E, 0xCD, 0x4F, 0x0E,
 0x10, 0xC1, 0xE0, 0xF9, 0x73, 0x5A, 0x07, 0xBC,
 0x00, 0x18, 0x53, 0xD6, 0xD3, 0xE0, 0x30, 0x0D,
 0x3C, 0x88, 0x68, 0x2E, 0x42, 0x03, 0xC2, 0x20,
 0xD7, 0x08, 0xE9, 0x78, 0xF1, 0xE5, 0xAC, 0xDD,
 0x2C, 0x73, 0xC8, 0x0A, 0xD2, 0xA2, 0x43, 0xA3,
 0x88, 0x66, 0x1F, 0x07, 0xF0, 0x64, 0x05, 0x6E,
 0x90, 0x56, 0x19, 0x35, 0x98, 0xCE, 0x05, 0xD8,
 0x8E, 0x8D, 0x60, 0xAF, 0x0B, 0x98, 0x35, 0xDB,
 0xFF, 0xF0, 0xF2, 0x81, 0x3A, 0x42, 0x43, 0x31,
 0xEA, 0x00, 0x80, 0xB7, 0x3D, 0xC0, 0xB7, 0x81,
 0xBE, 0x2E, 0xC8, 0xD8, 0x0D, 0x63, 0x0D, 0x8A,
 0x82, 0xCE, 0x8A, 0x4D, 0x5D, 0x96, 0x1C, 0x21,
 0xAA, 0x3B, 0x02, 0x04, 0x02, 0x00, 0x35, 0xF5,
 0xF8, 0x13, 0xCF, 0x33, 0x02, 0x4D, 0x32, 0x07,
 0x0D, 0x4F, 0x44, 0x81, 0x04, 0x12, 0x77, 0xCC,
 0x24, 0x50, 0x32, 0xE3, 0x90, 0x33, 0x95, 0x35,
 0xEF, 0x2C, 0xA3, 0x51, 0x22, 0x0E, 0x6C, 0x91,
 0x88, 0x40, 0xCF, 0x50, 0xC3, 0x8C, 0x6C, 0x97,
 0xC8, 0x31, 0x43, 0x14, 0x51, 0x40, 0xC1, 0x04,
 0x07, 0x02, 0x15, 0x73, 0xCC, 0x31, 0xD1, 0xB8,
 0x13, 0xC0, 0x39, 0xC7, 0x10, 0x63, 0x4C, 0x58,
 0xB2, 0x2D, 0xF3, 0x4C, 0x2A, 0x94, 0x09, 0x63,
 0xCC, 0x31, 0xCC, 0x94, 0x12, 0x00, 0x3F, 0xCC,
 0xBC, 0x48, 0x50, 0x32, 0xCE, 0xC4, 0x82, 0x81,
 0x0B, 0x77, 0x84, 0x75, 0x8D, 0x28, 0xED, 0xB0,
 0x03, 0x8D, 0x39, 0x96, 0x0C, 0xA0, 0x04, 0x15,
 0x8C, 0x54, 0x82, 0x8B, 0x27, 0xA5, 0x28, 0xA3,
 0x4A, 0x3C, 0xF1, 0xF0, 0xB3, 0x5D, 0x3E, 0xF0,
 0xC4, 0xA3, 0x0D, 0x00, 0xC7, 0x64, 0xD3, 0x4E,
 0x3E, 0xFB, 0xDC, 0xB3, 0x8E, 0x35, 0x00, 0x14,
 0xA3, 0xCE, 0x3B, 0xF5, 0x6C, 0xD7, 0x4F, 0x3C,
 0xF0, 0x9C, 0xA3, 0x9F, 0x34, 0xE6, 0xD8, 0xD3,
 0xC9, 0x19, 0x46, 0x1C, 0x40, 0x42, 0x58, 0xDE,
 0xD8, 0xA9, 0x8A, 0x3C, 0xDB, 0x65, 0x91, 0x45,
 0x7B, 0xDB, 0xA5, 0x43, 0x0D, 0xA3, 0x8C, 0x86,
 0x73, 0x8C, 0x39, 0x90, 0x6E, 0x37, 0x0E, 0x31,
 0xF4, 0x54, 0x1A, 0x80, 0x3C, 0xA8, 0x68, 0xD3,
 0x4F, 0x7B, 0x47, 0x2C, 0xD0, 0xC4, 0x3C, 0xAB,
 0x00, 0xC0, 0xCD, 0x76, 0x9C, 0x28, 0xD2, 0xC8,
 0x76, 0x95, 0x04, 0x50, 0x8F, 0x3D, 0xED, 0xF1,
 0xFF, 0x23, 0xCE, 0x3D, 0x9F, 0x6E, 0xE7, 0x0F,
 0x3F, 0xFC, 0x7C, 0x13, 0x4E, 0x7B, 0xA5, 0x60,
 0x43, 0xE9, 0x76, 0xE0, 0x88, 0xD2, 0x8F, 0x3F,
 0xED, 0xF5, 0xD3, 0x0F, 0x3B, 0xD4, 0x10, 0x1B,
 0x80, 0x28, 0x6E, 0x24, 0x80, 0xC0, 0x03, 0x01,
 0xD0, 0x93, 0xCA, 0xA9, 0x01, 0xCC, 0xD2, 0xC6,
 0x00, 0xED, 0x9D, 0x92, 0x8C, 0x32, 0xEB, 0xB4,
 0xF7, 0x4D, 0x88, 0xFA, 0x6C, 0xA7, 0x0E, 0x34,
 0xAA, 0x4C, 0x13, 0x6E, 0x00, 0xEC, 0xF4, 0x74,
 0x8C, 0x8C, 0x01, 0xDC, 0xC3, 0x8A, 0x2A, 0xED,
 0x8C, 0xF9, 0x2E, 0x34, 0xF1, 0x06, 0xF0, 0x4A,
 0x21, 0x61, 0xE8, 0xE0, 0xC6, 0xB0, 0x01, 0x6C,
 0x43, 0xAD, 0x2D, 0x47, 0x60, 0x5B, 0xED, 0x88,
 0x00, 0x5C, 0xD3, 0x5E, 0x37, 0x00, 0x28, 0x73,
 0xAE, 0x85, 0x00, 0x74, 0xD3, 0xDE, 0x36, 0x03,
 0x89, 0xD3, 0x1E, 0x9C, 0xE8, 0x6C, 0x87, 0x4F,
 0x32, 0x00, 0x3C, 0x53, 0x2B, 0x24, 0x69, 0x18,
 0x90, 0xC6, 0x2E, 0x9C, 0x6C, 0x77, 0x0E, 0xB5,
 0xB8, 0x30, 0x01, 0xED, 0xA6, 0xC7, 0x08, 0xF4,
 0xE8, 0x76, 0xDE, 0x00, 0xD0, 0xCC, 0xB9, 0xE5,
 0x08, 0x44, 0xCE, 0xC4, 0x03, 0x69, 0xD3, 0xDE,
 0x38, 0x00, 0x54, 0x1C, 0x00, 0x3E, 0xA9, 0x00,
 0x60, 0x8D, 0x9D, 0x4D, 0xEC, 0x90, 0x04, 0x21,
 0xAE, 0x64, 0x1A, 0x80, 0x3B, 0x24, 0x9B, 0xBC,
 0xDD, 0x3B, 0x29, 0x0F, 0xD8, 0x5E, 0xCB, 0x2F,
 0x6F, 0x17, 0x33, 0x00, 0xE9, 0xB4, 0x57, 0xCD,
 0x40, 0xD9, 0xF0, 0x9A, 0xB3, 0xC5, 0x3D, 0x67,
 0xED, 0x8F, 0x12, 0x0C, 0xD4, 0xC0, 0x07, 0x00,
 0xC4, 0xCC, 0xB3, 0xDD, 0x3C, 0x49, 0x0B, 0xCC,
 0xB4, 0xCA, 0x4F, 0xBB, 0x0C, 0xB3, 0x40, 0xDA,
 0x6D, 0x07, 0xA7, 0x40, 0x59, 0x6F, 0x77, 0xCA,
 0xD6, 0x3B, 0xF7, 0x8C, 0x4D, 0x00, 0x4A, 0x38,
 0xFF, 0xB1, 0x03, 0x06, 0x6C, 0x95, 0xBD, 0x9D,
 0x26, 0x49, 0x9F, 0xBC, 0xB6, 0xD3, 0x2C, 0xBB,
 0xBD, 0x1D, 0xC3, 0xE3, 0xB4, 0x97, 0xCD, 0x40,
 0x85, 0x02, 0x4B, 0x35, 0xD7, 0x00, 0xB4, 0xC2,
 0x08, 0x12, 0x09, 0xB4, 0x31, 0x53, 0x32, 0xB0,
 0x06, 0xD0, 0x4E, 0xDA, 0x4B, 0x37, 0xBD, 0x72,
 0x00, 0x50, 0x9F, 0xAB, 0x8E, 0x40, 0xAC, 0x68,
 0x2D, 0xD0, 0xAF, 0x01, 0x44, 0x33, 0x79, 0x00,
 0xFA, 0x34, 0x83, 0xCA, 0x1E, 0x41, 0x30, 0x00,
 0x44, 0x23, 0xB0, 0x08, 0xB4, 0x4A, 0xAD, 0xE3,
 0xFC, 0xCB, 0x85, 0xE1, 0xA2, 0xB7, 0xAD, 0x4C,
 0x3E, 0xDB, 0xED, 0x73, 0x0A, 0x38, 0xD9, 0xB0,
 0xAB, 0x0F, 0x35, 0xC8, 0x5C, 0xB3, 0x8F, 0xB8,
 0xFA, 0xB1, 0x1E, 0x4A, 0x1E, 0x48, 0xEC, 0x70,
 0xC4, 0x12, 0x01, 0x8C, 0x93, 0x0C, 0x33, 0xEA,
 0xB4, 0x17, 0x8D, 0x2C, 0xDB, 0xDD, 0xD2, 0x06,
 0x28, 0xDB, 0xC1, 0x13, 0x7C, 0xE2, 0xC6, 0xD4,
 0xDB, 0x1E, 0x3A, 0xD1, 0x10, 0x1F, 0x40, 0x3F,
 0x46, 0x47, 0x8B, 0x20, 0x00, 0xDF, 0x6C, 0x87,
 0x8B, 0x12, 0x34, 0x24, 0xF0, 0xC6, 0x23, 0xED,
 0xD5, 0x73, 0x4F, 0x7B, 0xE2, 0x00, 0x00, 0xF8,
 0x02, 0xE0, 0x0F, 0x40, 0x90, 0x2F, 0x00, 0x9A,
 0x68, 0x5A, 0x35, 0xBC, 0x85, 0xBA, 0xCE, 0x6D,
 0x67, 0x1D, 0x00, 0x90, 0x06, 0xBB, 0xDA, 0xB3,
 0x0E, 0x68, 0x0C, 0x24, 0x15, 0x9F, 0x58, 0x02,
 0x10, 0x16, 0x20, 0x83, 0x1F, 0xFC, 0x62, 0x14,
 0xE4, 0xA8, 0xD5, 0x76, 0xEC, 0xE1, 0x0D, 0xB6,
 0x00, 0xE2, 0x07, 0x28, 0x24, 0x01, 0x2B, 0xAC,
 0x61, 0x8D, 0x69, 0xE8, 0x07, 0x00, 0xCB, 0x60,
 0xA1, 0x35, 0xE6, 0xE7, 0x32, 0x6C, 0x88, 0x63,
 0x1C, 0xE0, 0x60, 0x85, 0x40, 0x90, 0x51, 0x8D,
 0x70, 0x8C, 0xE3, 0x1B, 0xD2, 0x78, 0xE1, 0x40,
 0xFF, 0x48, 0x50, 0x05, 0x1D, 0x04, 0xC1, 0x0E,
 0xD8, 0x20, 0x85, 0xEE, 0xBC, 0x31, 0x0E, 0x71,
 0x60, 0x83, 0x60, 0x00, 0x80, 0x83, 0x0E, 0x9C,
 0x60, 0x05, 0x13, 0xEC, 0xA5, 0x28, 0xBD, 0xD0,
 0x40, 0x14, 0x0C, 0xD0, 0x05, 0x0F, 0xD4, 0x84,
 0x0D, 0x0A, 0x48, 0xC2, 0x15, 0x4E, 0x60, 0x9E,
 0x81, 0x94, 0x40, 0x0D, 0x30, 0x78, 0x81, 0x1A,
 0xC8, 0x58, 0x93, 0x37, 0xF4, 0x20, 0x09, 0x3D,
 0x00, 0x41, 0x19, 0x59, 0x70, 0x87, 0x29, 0x10,
 0xC0, 0x08, 0x19, 0x90, 0x10, 0x4D, 0x32, 0x50,
 0x83, 0x27, 0xD0, 0x40, 0x0F, 0xD4, 0xC9, 0x85,
 0x1E, 0xBC, 0xF0, 0x82, 0x17, 0x48, 0x40, 0x2A,
 0x4D, 0xF1, 0x80, 0x0D, 0x9E, 0xE0, 0x03, 0x08,
 0xF4, 0xE2, 0x33, 0x89, 0xD0, 0xC3, 0x17, 0x60,
 0x50, 0x00, 0x07, 0x6C, 0x40, 0x8F, 0x44, 0xE1,
 0x90, 0x02, 0x9E, 0x60, 0x83, 0x3D, 0xD8, 0x05,
 0x15, 0x2C, 0xD0, 0x80, 0x17, 0x64, 0xF0, 0x02,
 0x23, 0xC8, 0xE1, 0x3E, 0x5F, 0x41, 0x05, 0x73,
 0x9E, 0xA0, 0x00, 0x29, 0x08, 0x42, 0x2B, 0xB4,
 0x10, 0x41, 0x1D, 0xBA, 0x40, 0x03, 0x18, 0xF4,
 0xC0, 0x02, 0x23, 0x80, 0x0C, 0x0B, 0xBC, 0x70,
 0x80, 0x06, 0xF4, 0xC0, 0x01, 0x81, 0xC0, 0x51,
 0x47, 0x80, 0x61, 0x88, 0x0D, 0x4C, 0x80, 0x08,
 0x69, 0x24, 0x42, 0x1C, 0x44, 0x20, 0x4C, 0xC2,
 0x08, 0x42, 0x0A, 0xBD, 0x24, 0x02, 0x02, 0xE2,
 0x30, 0x82, 0xD1, 0x18, 0x04, 0x18, 0x8B, 0x00,
 0x41, 0x06, 0x26, 0x30, 0x85, 0x34, 0xCA, 0x40,
 0x0B, 0x17, 0x18, 0x81, 0x29, 0xCC, 0xE3, 0x08,
 0x5A, 0x2A, 0x60, 0x0A, 0x37, 0x28, 0x02, 0x1A,
 0x30, 0xC0, 0x07, 0x0F, 0x7C, 0xE0, 0x03, 0x38,
 0xA1, 0x83, 0x1A, 0xBA, 0xE0, 0x83, 0x42, 0xC2,
 0x40, 0x0A, 0x6B, 0xD8, 0x83, 0x57, 0xCA, 0x28,
 0x3B, 0x10, 0x14, 0xD0, 0x41, 0x0B, 0x34, 0xC0,
 0x01, 0x74, 0x6A, 0x40, 0x03, 0x1E, 0xF8, 0xC0,
 0x07, 0x35, 0x78, 0x01, 0x01, 0x08, 0x50, 0x00,
 0x04, 0x74, 0x81, 0x0D, 0x7B, 0x88, 0x44, 0x5D,
 0xF8, 0x59, 0x10, 0x16, 0xFC, 0xE1, 0x0E, 0x16,
 0x40, 0x03, 0x19, 0xC8, 0x20, 0x86, 0x8E, 0x9A,
 0x61, 0x0D, 0x71, 0xC0, 0x83, 0x4E, 0x72, 0xD1,
 0xCC, 0x32, 0x06, 0x04, 0x00, 0x3B};

 HtErrorMsgInfo errInfo_f[HTERR_ELEMENTS] =
{
  { 200,  "OK"                                   },
  { 201,  "Created"                              },
  { 202,  "Accepted"                             },
  { 203,  "Partial Information"                  },
  { 204,  "No Response"                          },
  { 301,  "Moved"                                },
  { 302,  "Found"                                },
  { 303,  "Method"                               },
  { 304,  "Not Modified Since"                   },
  { 400,  "Invalid Request"                      },
  { 401,  "Unauthorized Access Denied"           },
  { 402,  "Payment Required"                     },
  { 403,  "Access forbidden"                     },
  { 404,  "Not Found"                            },
  { 500,  "Can't Access Document"                },
  { 501,  "Command not Implemented"              },
  { 502,  "Invalid response from gateway"        },
  { 503,  "file temporarily not available"       },
  { 8001, "-------------------------"            },
  { 8002, "Server timed out"                     },
  { 8003, "Data transfer interrupted"            },
  { 8004, "Connection establishment interrupted" },
  { 8005, "Bad Incomplete, or Unknown  Response" },
  { 8006, "Too many redirections"                },
  { 8007, "System call `%s' failed:"             },
  { 8008, "Out of Memory"                        }
};

/**
 * Http methods list
 */
 HttpMethodInfo httpMethods_f[] =
{
  { hMethodGet,  "GET"  },
  { hMethodHead, "HEAD" },
  { hMethodPost, "POST" },
  { hMethodBad,  ""     },
  { hEventTimeOut, "" },
  { hEventClose, "" }
};

/**
 * Http versions list.
 */
 HttpVersionInfo httpVersions_f[] =
{
  { hVersion09,  "HTTP/0.9" },
  { hVersion10,  "HTTP/1.0" },
  { hVersion11,  "HTTP/1.1" },
  { hVersionBad, "OTHER"    }
};

 hMimeType_t  HttpMimeTypes[] =
{
  {"application/download",      "bin",     NULL_MIMETYPE },
  {"application/download",      "cfg",     NULL_MIMETYPE },
  {"application/download",      "dll",     NULL_MIMETYPE },
  {"application/oda",           "oda",     NULL_MIMETYPE },
  {"application/pdf",           "pdf",     NULL_MIMETYPE },
  {"application/postscript",    "ai",      NULL_MIMETYPE },
  {"application/postscript",    "eps",     NULL_MIMETYPE },
  {"application/postscript",    "ps",      NULL_MIMETYPE },
  {"application/rtf",           "rtf",     NULL_MIMETYPE },
  {"application/x-mif",         "mif",     NULL_MIMETYPE },
  {"application/x-csh",         "csh",     NULL_MIMETYPE },
  {"application/x-dvi",         "dvi",     NULL_MIMETYPE },
  {"application/x-hdf",         "hdf",     NULL_MIMETYPE },
  {"application/x-javascript",  "js",      NULL_MIMETYPE },
  {"application/x-latex",       "latex",   NULL_MIMETYPE },
  {"application/x-netcdf",      "cdf",     NULL_MIMETYPE },
  {"application/x-netcdf",      "nc",      NULL_MIMETYPE },
  {"application/x-sh",          "sh",      NULL_MIMETYPE },
  {"application/x-tcl",         "tcl",     NULL_MIMETYPE },
  {"application/x-tex",         "tex",     NULL_MIMETYPE },
  {"application/x-texinfo",     "texinfo", NULL_MIMETYPE },
  {"application/x-texinfo",     "texi",    NULL_MIMETYPE },
  {"application/x-troff",       "t",       NULL_MIMETYPE },
  {"application/x-troff",       "tr",      NULL_MIMETYPE },
  {"application/x-troff",       "roff",    NULL_MIMETYPE },
  {"application/x-troff-man",   "man",     NULL_MIMETYPE },
  {"application/x-troff-me",    "me",      NULL_MIMETYPE },
  {"application/x-troff-ms",    "ms",      NULL_MIMETYPE },
  {"application/x-wais-source", "src",     NULL_MIMETYPE },
  {"application/zip",           "zip",     NULL_MIMETYPE },
  {"application/x-bcpio",       "bcpio",   NULL_MIMETYPE },
  {"application/x-cpio",        "cpio",    NULL_MIMETYPE },
  {"application/x-gtar",        "gtar",    NULL_MIMETYPE },
  {"application/x-shar",        "shar",    NULL_MIMETYPE },
  {"application/x-sv4cpio",     "sv4cpio", NULL_MIMETYPE },
  {"application/x-sv4crc",      "sv4crc",  NULL_MIMETYPE },
  {"application/x-tar",         "tar",     NULL_MIMETYPE },
  {"application/download",      "tgz",     NULL_MIMETYPE },
  {"application/x-ustar",       "ustar",   NULL_MIMETYPE },
  {"audio/basic",               "au",      NULL_MIMETYPE },
  {"audio/basic",               "snd",     NULL_MIMETYPE },
  {"audio/x-aiff",              "aif",     NULL_MIMETYPE },
  {"audio/x-aiff",              "aiff",    NULL_MIMETYPE },
  {"audio/x-aiff",              "aifc",    NULL_MIMETYPE },
  {"audio/x-wav",               "wav",     NULL_MIMETYPE },
  {"image/gif",                 "gif",     NULL_MIMETYPE },
  {"image/ief",                 "ief",     NULL_MIMETYPE },
  {"image/jpeg",                "jpe",     NULL_MIMETYPE },
  {"image/jpeg",                "jpg",     NULL_MIMETYPE },
  {"image/jpeg",                "jpeg",    NULL_MIMETYPE },
  {"image/tiff",                "tiff",    NULL_MIMETYPE },
  {"image/tiff",                "tif",     NULL_MIMETYPE },
  {"image/x-cmu-raster",        "ras",     NULL_MIMETYPE },
  {"image/x-portable-anymap",   "pnm",     NULL_MIMETYPE },
  {"image/x-portable-bitmap",   "pbm",     NULL_MIMETYPE },
  {"image/x-portable-graymap",  "pgm",     NULL_MIMETYPE },
  {"image/x-portable-pixmap",   "ppm",     NULL_MIMETYPE },
  {"image/x-rgb",               "rgb",     NULL_MIMETYPE },
  {"image/x-xbitmap",           "xbm",     NULL_MIMETYPE },
  {"image/x-xpixmap",           "xpm",     NULL_MIMETYPE },
  {"image/x-xwindowdump",       "xwd",     NULL_MIMETYPE },
  {"text/html",                 "dat",     NULL_MIMETYPE },
  {"text/html",                 "html",    NULL_MIMETYPE },
  {"text/plain",                 "jar",    NULL_MIMETYPE },
  {"application/x-java-jnlp-file",     "jnlp",    NULL_MIMETYPE },
  {"application/octet-stream",         	"class",     	NULL_MIMETYPE },     /* SSL-VPN  */
  {"application/octet-stream",         	"exe",     	NULL_MIMETYPE },     /* SSL-VPN  */
  {"application/x-java-archive-diff",   	"jardiff",     	NULL_MIMETYPE },
  {"text/html",                 "htm",     NULL_MIMETYPE },
  {"text/x-sgml",               "sgml",    NULL_MIMETYPE },
  {"text/x-sgml",               "sgm",     NULL_MIMETYPE },
  {"text/xml",                  "xml",     NULL_MIMETYPE }, /* test */
  {"text/xsl",                  "xsl",     NULL_MIMETYPE }, /* SSL-VPN */
  {"text/plain",                "txt",     NULL_MIMETYPE },
  {"text/richtext",             "rtx",     NULL_MIMETYPE },
  {"text/tab-separated-values", "tsv",     NULL_MIMETYPE },
  {"text/x-setext",             "etx",     NULL_MIMETYPE },
  {"text/css",                  "css",     NULL_MIMETYPE },
  {"video/mpeg",                "mpeg",    NULL_MIMETYPE },
  {"video/mpeg",                "mpe",     NULL_MIMETYPE },
  {"video/mpeg",                "mpg",     NULL_MIMETYPE },
  {"video/quicktime",           "qt",      NULL_MIMETYPE },
  {"video/quicktime",           "mov",     NULL_MIMETYPE },
  {"video/x-fli",               "fli",     NULL_MIMETYPE },
  {"video/x-msvideo",           "avi",     NULL_MIMETYPE },
  {"video/x-sgi-movie",         "movie",   NULL_MIMETYPE },
  {"image/png",                 "png",     NULL_MIMETYPE },
  { NULL,                       NULL,      NULL_MIMETYPE }
};
/********************************************************
 * * * *  F u n c t i o n    P r o t o t y p e s  * * * *
 ********************************************************/
int32_t LaunchLogViewer(HttpGlobals_t *pHttpGlobals,HttpRequest *pReq);
int32_t PostJarfile( HttpRequest  *pReq);
 int32_t IGWHttpdGetEphermalPort(
                                           HttpGlobals_t  *pHttpGlobals,
                                           HttpPortInfo_t *PortInfo,
                                           uint32_t       ulIPAddr,
                                           cm_sock_handle  *pSockFd,
                                           uint16_t       *pucPortNo
                                          );
/*
 * HTTP FILE SYSTEM RELATED FUNCTIONS
 */

/**
 * This function is used to open the given file from the flash in the given
 * mode.
 */

int32_t HtmlFs_OpenFile(HttpGlobals_t *pHttpGlobals,char  *pFileName,
                         int32_t  iMode );

/**
 * This function is used to read the information from the respective file
 * using the given parameters.
 */

int32_t HtmlFs_ReadFile (HttpGlobals_t *pHttpGlobals,int32_t  iHandle,
                          char  *pBuf,
                          int32_t  iCount );

/**
 * This function is used to write the information to the respective file
 * using the given parameters.
 */
/**/
int32_t HtmlFs_WriteFile(HttpGlobals_t *pHttpGlobals,int32_t  iHandle,
                          char  *pBuf,
                          int32_t  iCount );

/**
 * This function is used to seek the information from the respective file
 * using the given parameters.
 */

int32_t HtmlFs_SeekFile (HttpGlobals_t *pHttpGlobals,int32_t  iHandle,
                          int32_t  iOffset,
                          int32_t  iWhence );

/**
 * This function is used to close the file in the flash. The file name can
 * be obtained from the global variable.
 */

int32_t HtmlFs_CloseFile(HttpGlobals_t *pHttpGlobals,int32_t  iHandle );

/**
 * This function is used to
 */

int32_t Httpd_ReadFile(HttpGlobals_t *pHttpGlobals,unsigned char  *pFileName );

/**
 * This function is used to enable the server using the given type.
 */
/**/
int32_t Httpd_Enable(HttpGlobals_t *pHttpGlobals,hServerType_e  Type );

/**
 * This function is used to disable the server using the given type.
 */
/**/
int32_t Httpd_Disable( HttpGlobals_t *pHttpGlobals,hServerType_e  Type );


int32_t HttpWriteToTransmitBuffer (
                                 HttpGlobals_t *pHttpGlobals,  
                                 HttpClientReq_t  *pReq,
                                 unsigned char         *pBuf,
                                 uint32_t         uiTxBufLen);


int32_t Http_ProcessRegModuleRequest(HttpGlobals_t *pHttpGlobals,HttpClientReq_t  *pClientReq);


int32_t HttpClient_MultiPartProcess(HttpGlobals_t *pHttpGlobals,HttpClientReq_t  *pClientReq );


 int32_t HttpProcessLoopBackData(HttpGlobals_t *pHttpGlobals,uint32_t SFd);

/**
 * This function is used to get the enumerated value for the passed method
 * name.
 */

int32_t HttpClient_MethStr2Enum( unsigned char  *pMethName );

/**
 * This function is used to get the enumerated value for the passed version
 * name.
 */

int32_t HttpClient_VersionStr2Enum( unsigned char  *pVersionName );

/**
 * This function is used
 */

char Httpd_x2c( unsigned char  *what );

/**
 * This function is used
 */

void  Httpd_UnEscapeUrl( unsigned char  *url );



/**
 * This function is used
 */

int32_t Httpd_AcceptClient(HttpGlobals_t  *pHttpGlobals,uint32_t  ulIndex ,uint32_t ip_type_ul);

/**
 * This function is used
 */

int32_t HttpClient_SendFile(HttpGlobals_t  *pHttpGlobals,HttpClientReq_t  *pClientReq );


/**
 * This function is used
 */

int32_t HttpClient_SetHdrFields(HttpGlobals_t  *pHttpGlobals,HttpClientReq_t  *pClientReq );

/**
 * This function is used
 */

void HttpClient_SetDummyReq( HttpClientReq_t  *pClientReq,
                               HttpRequest      *pDummy );

/**
 * This function is used
 */

int32_t HttpClient_EvaluateAccess(HttpGlobals_t  *pHttpGlobals,HttpClientReq_t  *pClientReq );



/**
 * This function is used
 */
int32_t HttpClient_SendHeader(HttpGlobals_t  *pHttpGlobals,HttpClientReq_t  *pClientReq );

/**
 * This is called to set the request url in the context. This parses it and
 * removes escape sequences and sets plain url in the context.
 */

int32_t HttpClient_SetUrl( HttpClientReq_t  *pClientReq,
                           unsigned char         *pUrl );

/**
 * This parses first line of the request for method, URL and version.
 */

int32_t HttpClient_ParseRequestLine( HttpClientReq_t  *pClientReq,
                                     unsigned char         *pFirstLine );

/**
 * Gives the cookie value.
 */

int32_t HttpClient_ParseCookieValue( char  *PtraFieldVal,
                                     char  *PtrCookieValue );

/**
 * This parses a line of request header and sets the appropriate field in
 * the client context. This is for each line in the request header.
 */

int32_t HttpClient_ParseHeaderLine(HttpGlobals_t *pHttpGlobals,HttpClientReq_t  *pClientReq,
                                    unsigned char         *pHdrLine );

/**
 * This checks the validity of cookie received from client and takes
 * appropriate action.
 */

int32_t HttpClient_ProcessCookie(HttpGlobals_t *pHttpGlobals,HttpClientReq_t  *pClientReq,
                                  unsigned char         *aFieldVal );

/**
 * This parses the request string(first line + header)
 */

int32_t HttpClient_ParseRequest(HttpGlobals_t *pHttpGlobals,HttpClientReq_t  *pClientReq );



int32_t HttpClient_Process(HttpGlobals_t *pHttpGlobals,HttpClientReq_t  *pClientReq );

/**
 * This function is used
 */

int32_t HttpDef_ClientAccpet( HttpServer_t     *pSer,
                              HttpClientReq_t  *pClientReq );

/**
 * This function is used
 */

int32_t HttpDef_ClientRead( HttpClientReq_t  *pClientReq,
                            unsigned char         *pBuf,
                            uint32_t         ulLen,
                            int32_t          *pBytesRead);

/**
 * This function is used
 */

int32_t HttpDef_ClientWrite( HttpClientReq_t  *pClientReq,
                             unsigned char         *pBuf,
                             uint32_t         ulLen );

/**
 * This function is used
 */

int32_t HttpDef_ClientClose( HttpClientReq_t  *pClientReq );

/**
 * This function is used
 */

void HttpClient_SetReadInfo( HttpClientReq_t  *pClientReq,
                               unsigned char         *pBuf,
                               uint32_t         ulMaxLen,
                               unsigned char         *pCur );

/**
 * This function tells whether the current posting is of multi-part or not.
 */

bool HttpClient_IsMultiPart( HttpClientReq_t  *pClientReq );

/**
 * This returns multi-part boundary string pointer. Boundary comes in the
 * value of request header field 'Content-Type' as
 * Content-Type=multi-part/form-data; boundary=----------herf34u2
 */

unsigned char* HttpClient_GetMPartBoundary( HttpClientReq_t  *pClientReq );


/**
 * This function is used
 */
/**/
int32_t HttpReadIntoUnzipBuffer( unsigned char  *pInputBuf,
                                 uint32_t  ulCnt,
                                 void    *pIGWInputArg );

/**
 * This function is used
 */
/**/
int32_t HttpReadFromUnzipBuffer( unsigned char  *pOutputBuf,
                                 uint32_t  ulCnt,
                                 void    *pIGWOutputArg );


 int32_t HttpCreateLoopBackSocket(void);

 int32_t HttpSendToClient (HttpGlobals_t *pHttpGlobals,HttpClientReq_t *pClientReq,int32_t cProcessRetVal);
 HttpTransmitBuffer_t* HttpAllocTxBuf(HttpGlobals_t  *pHttpGlobals);

/* This wrapper function  is  provided for  SSL_VPN to parse the raw buffer */

void Httpd_ParseBuffer(unsigned char *pBuf, HttpClientReq_t *pClientReq);

#ifdef INTOTO_CMS_SUPPORT


int32_t HttpCMSDevProcessLoopBackData(uint32_t SFd);

 
int32_t HttpCMSDevCreateLoopBackSocket(void);

#endif  /* INTOTO_CMS_SUPPORT */

/*HTTP/1.1 Persistent and Pipelinig*/

 
int32_t HttpClient_CreateSubRequest(HttpGlobals_t  *pHttpGlobals,HttpClientReq_t *pClientReq);

 
int32_t HttpClient_PipeLineRequest(HttpGlobals_t  *pHttpGlobals,HttpClientReq_t *pClientReq);


int32_t HttpStateMachineFunc(HttpGlobals_t *pHttpGlobals,HttpClientReq_t *pClientReq );


int32_t   HttpClient_ReadGet(HttpGlobals_t  *pHttpGlobals,HttpClientReq_t  *pClientReq);	


int32_t   HttpClient_ReadPost(HttpGlobals_t  *pHttpGlobals,HttpClientReq_t  *pClientReq);	


int32_t   HttpClient_ReadHead(HttpGlobals_t  *pHttpGlobals,HttpClientReq_t  *pClientReq);	


int32_t   HttpClient_ReadBad(HttpGlobals_t  *pHttpGlobals,HttpClientReq_t  *pClientReq);	

int32_t   HttpClient_StartProcessing(HttpGlobals_t *pHttpGlobals,HttpClientReq_t  *pClientReq);	


void HttpClient_CleanUp(HttpGlobals_t *pHttpGlobals);


char* HttpStrTok(char *cInputStr,char *cTokenStr);

 
char* HttpStrpBrk(char* cInputStr,char* cTokenStr);

/************************* STATE MACHINE **************************/
 HttpHandleEvent_t
          HttpStateTransTable[HTTP_MAX_STATES][HTTP_MAX_EVENTS] =
          {
               /* STATE_READ_REQUEST */
               {  
	             {HTTP_GET_METHOD_EVENT,HttpClient_ReadGet },
	             {HTTP_HEAD_METHOD_EVENT,HttpClient_ReadHead},
	             {HTTP_POST_METHOD_EVENT,HttpClient_ReadPost},
	             {HTTP_BAD_METHOD_EVENT,HttpClient_ReadBad},
	             {HTTP_TIMEOUT_EVENT,HttpClient_Delete},
	             {HTTP_CLOSE_EVENT,HttpClient_Delete}
               },
               /* STATE_READ_POST_STRING */
               {
	              {HTTP_GET_METHOD_EVENT,HttpClient_Delete},
	              {HTTP_HEAD_METHOD_EVENT,HttpClient_Delete},
	              {HTTP_POST_METHOD_EVENT,HttpClient_ReadPStr_ReadEvent},
	              {HTTP_BAD_METHOD_EVENT,HttpClient_Delete},
	              {HTTP_TIMEOUT_EVENT,HttpClient_Delete},
	              {HTTP_CLOSE_EVENT,HttpClient_Delete }
	       },
	       /* STATE_MULTIPART_READ */
	       {
			 {HTTP_GET_METHOD_EVENT, HttpClient_Delete},
			 {HTTP_HEAD_METHOD_EVENT,HttpClient_Delete},
			 {HTTP_POST_METHOD_EVENT,HttpClient_MultiPartProcess },
			 {HTTP_BAD_METHOD_EVENT, HttpClient_Delete},
			 {HTTP_TIMEOUT_EVENT,HttpClient_Delete},
			 {HTTP_CLOSE_EVENT,HttpClient_Delete }
	       },
	       /* STATE_SEND_RESPONSE*/
	       {
		       {HTTP_GET_METHOD_EVENT, HttpClient_SendResponse},
		       {HTTP_HEAD_METHOD_EVENT, HttpClient_SendResponse},
		       {HTTP_POST_METHOD_EVENT, HttpClient_SendResponse},
		       {HTTP_BAD_METHOD_EVENT, HttpClient_SendResponse},
		       {HTTP_TIMEOUT_EVENT,HttpClient_Delete },
		       {HTTP_CLOSE_EVENT,HttpClient_Delete }
	       },
	       /* STATE_PIPELINE_REQUEST */
	       {
		       {HTTP_GET_METHOD_EVENT,HttpClient_PipeLineRequest},
		       {HTTP_HEAD_METHOD_EVENT,HttpClient_PipeLineRequest},
		       {HTTP_POST_METHOD_EVENT, HttpClient_Delete},
		       {HTTP_BAD_METHOD_EVENT,HttpClient_Delete},
		       {HTTP_TIMEOUT_EVENT, HttpClient_Delete},
		       {HTTP_CLOSE_EVENT,HttpClient_Delete }
	       },
	       /* STATE_FINISHED */
	       {
		       {HTTP_GET_METHOD_EVENT,HttpClient_Delete},
		       {HTTP_HEAD_METHOD_EVENT,HttpClient_Delete},
		       {HTTP_POST_METHOD_EVENT,HttpClient_Delete},
		       {HTTP_BAD_METHOD_EVENT,HttpClient_Delete},
		       {HTTP_TIMEOUT_EVENT,HttpClient_Delete},
		       {HTTP_CLOSE_EVENT,HttpClient_Delete}
	       }
	  };

/**
 *A strtok which returns empty strings, too
 **/
/**********************************************************************************
 * Function Name:HttpStrTok														  *						
 * Description -A strtok which returns empty strings, too 						  *	
 * input:@s: The string to be searched                                            *   
 *       @ct: The characters to search for                                        *   
 * output:1.)Null if not match found                                              * 
 *        2.)Pointer pointing to the matching position in @cs                     *  
 *********************************************************************************/

 char* HttpStrTok(char *s,char *ct)
{
  char *sbegin=NULL;
  char *send=NULL;
  static char *ssave = NULL;

  sbegin  = s ? s : ssave;
  if (!sbegin)
  {
     return NULL;
  }
  if (*sbegin == '\0')
  {
    ssave = NULL;
    return NULL;
  }
  send =HttpStrpBrk(sbegin, ct);
  if (send && *send != '\0')
  {
    *send++ = '\0';
  }
  ssave = send;
  return sbegin;
}
/**********************************************************************************
 * Function Name:HttpStrpbrk													  *	
 * Description - Find the first occurrence of a set of characters				  *	
 * input:@cs: The string to be searched											  *				
 *       @ct: The characters to search for										  *		
 * output:1.)Null if not match found											  *		
 *        2.)Pointer pointing to the matching position in @cs 					  *	
 *********************************************************************************/
 char* HttpStrpBrk(char* cs,char* ct)
{
  char *sc1=NULL;
  char *sc2=NULL;
  for( sc1 = cs; *sc1 != '\0'; ++sc1) 
  {
    for( sc2 = ct; *sc2 != '\0'; ++sc2)
    {
      if (*sc1 == *sc2)
      {		
        return (char *) sc1;
	  }
    }
  }
  return NULL;
}


/****************************************************************************************
 * Function Name    : HttpStateMachineFunc											    *	
 * Description      : This function searches through the state transition table looking *	
 *		 	          for the event type corresponding to the state. It, then, calls the*
 * 		              Function associated with this state and event or returns error    *	
 * Input            : HttpClientReq_t -- message containing event, context		        *	 
 *			          and any other required parameters			                        *  
 *                    pHttpGlobals  - Global pointer                                    *	
 * Output           : None														        *
 * Return value     : OF_SUCCESS on success and OF_FAILURE on error					    *	
 ****************************************************************************************/

int32_t HttpStateMachineFunc(HttpGlobals_t *pHttpGlobals,HttpClientReq_t *pClientReq)
{
  int32_t           iResult=0;
  int32_t          iState=0;
  int32_t          iEvent=0;    


  /**
   * Validate the passed parameter(s).
   */
  if(!pHttpGlobals)
  {
    Httpd_Error("pHttpGlobals is NULL\r\n");	
    return OF_FAILURE;
  }
  
  if (pClientReq==NULL)
  {
      HttpDebug("pClientRequest is NULL");
      return OF_FAILURE;
  }
  if(pClientReq->eState==hStateReadReq)
  {        
        if (HttpClient_ParseRequest(pHttpGlobals,pClientReq) != OF_SUCCESS)
        {
            HttpDebug("HttpClient_ParseRequest is returned failure");
            return OF_FAILURE;
        }
       /*start of code for SSL-VPN*/
        if(!Httpd_IsPartOfLoginPage(pHttpGlobals,(char *) pClientReq->ucFileName))
        {
#if 0
            bool bLoggedIn=FALSE;
        
            if(HttpCookie_GetByVal(pClientReq->ucCookieVal, &bLoggedIn,
                                NULL, NULL)!=OF_SUCCESS)
            {
                HttpDebug("HttpCookie_GetByVal failed");
                return OF_FAILURE; 
            }
            if (!bLoggedIn)
            {
              pClientReq->ulPageType = LOGINPAGEREQUEST;
            }
#endif
            pClientReq->ulPageType = LOGINPAGEREQUEST;
        }
        /*End of code for SSL-VPN*/

        /**
         * Only GET method allowed for version 0.9.
         */           
        if ((pClientReq->eVersion == hVersion09) &&
            (pClientReq->eMethod != hMethodGet))
        {          
               HttpDebug("Only GET method allowed for version 0.9");
               return OF_FAILURE;
        }
        if(pClientReq->pTimer) 
        {
          if(Httpd_Timer_Stop(pClientReq->pTimer)!=OF_SUCCESS)
          {
               HttpDebug("Httpd_Timer_Stop is returned failure");
               return OF_FAILURE;
          }
        }
 }
  /*Found the state index*/
  iState=pClientReq->eState; 
  iEvent=(pClientReq->eMethod) -HTTP_STATE_EVENT_BASE;
  
   /*we call the appropiate function*/  
  pHttpGlobals->MutexSockFd = -1;     
  iResult = HttpStateTransTable[iState][iEvent].FuncPtr(pHttpGlobals,pClientReq);  
  if (iResult==OF_FAILURE)
   {              
              if(HttpClient_Delete(pHttpGlobals,pClientReq)!=OF_SUCCESS)
               {
                    HttpDebug("Client Delete is returned failure");                    
               }
              HttpDebug("State trasaction is returned failure\r\n ");
              return OF_FAILURE;
   }
   else if( iResult== HTTP_DONOT_SEND_RESPONCE)
   {
           HttpDebug("HTTP_DONOT_SEND_RESPONCE");
           return HTTP_DONOT_SEND_RESPONCE;
   }   	
   return OF_SUCCESS;
}/*HttpStateMachineFunc*/

/**********************************************************************************
 * Function Name : HttpClient_PipeLineRequest									  *	
 * Description   : This is the handler for  pipelined request from client	      *	 
 * Input         : pClientReq 											          * 	 
 *			       pHttpGlobals  - Global pointer								  *								  *	
 * Output        : None											        		  *
 * Return value  : OF_SUCCESS on success and OF_FAILURE on error			    	  *	
 **********************************************************************************/
 
int32_t HttpClient_PipeLineRequest(HttpGlobals_t  *pHttpGlobals,HttpClientReq_t *pClientReq)
{
   int16_t   iReqTableSize=0;   
   int16_t   iiLoop=0;                        
   int16_t    ijLoop=0;                       
   int16_t    iIndex=-1;                                 
   bool   bIsFound=FALSE;   
   int16_t     iValue=-1;                
   HttpKeepAliveFd_t *pKeepAliveConn=NULL;
   
   
    if(pClientReq==NULL)
    {
      HttpDebug("pclientrequest is NULL\r\n");
      return OF_FAILURE;
    }    

    if(!pHttpGlobals)
    {
      Httpd_Error("pHttpGlobals is NULL\r\n");	
      pClientReq->bPersistentConn=FALSE;
      return OF_FAILURE;
    }
   
    iReqTableSize= Httpd_GetMaxReqTableSize(pHttpGlobals); /*pHttpGlobals->pHttpGlobalConfig->iReqtablesize;*/
    if(iReqTableSize<0)
    {
      HttpDebug("iReqTableSize  is <0 \r\n");
      pClientReq->bPersistentConn=FALSE;
      return OF_FAILURE;
    }  
   for (iiLoop = 0;iiLoop < iReqTableSize; iiLoop++)    
   {             
       if(pHttpGlobals->pHttpClientReqTable[iiLoop].iSockFd==pClientReq->SockFd)
      	{  
      	   iValue=iiLoop;
      	   bIsFound=TRUE;
      	    break;      	    
      	}
       if(pHttpGlobals->pHttpClientReqTable[iiLoop].iSockFd<=0)
       { 
           if(iValue<0) 
           {
               iValue=iiLoop;
           }
           break;
       }
   }
   if(iValue == -1)
   {
     Httpd_Error("Invalid value for iValue\r\n");
     pClientReq->bPersistentConn=FALSE;
     return OF_FAILURE;
   }
  if(bIsFound==FALSE)
  {
     pHttpGlobals->pHttpClientReqTable[iValue].iSockFd=pClientReq->SockFd;
  }
     for (ijLoop = 0;ijLoop <= HTTP_MAX_PIPELINED_REQUESTS;ijLoop++)    
     {    
            if (!(pHttpGlobals->pHttpClientReqTable[iValue].pHttpPipeline[ijLoop]))
            {
                iIndex = ijLoop;
                break;
            } 
      }
    if ((iIndex < 0) ||(iIndex==HTTP_MAX_PIPELINED_REQUESTS))
    {
      HttpDebug("iIndex  is <0  or Maximum requests reached for pipelining\r\n");
      pClientReq->bPersistentConn=FALSE;
      return OF_FAILURE;
    } 
    if(HttpClient_SendResponse(pHttpGlobals,pClientReq)!=OF_SUCCESS)
    {
      HttpDebug("HttpClient_SendResponce failed\r\n");
      pClientReq->bPersistentConn=FALSE;
 	    return OF_FAILURE;
    }
    if(pClientReq->bIsPipeLined==FALSE)
    {
           pHttpGlobals->pHttpClientReqTable[iValue].pHttpPipeline[iIndex]=pClientReq;
           pHttpGlobals->pHttpClientReqTable[iValue].bStartResponce=TRUE;
           pClientReq->bIsPipeLined=TRUE;
           pClientReq->iPipeLineTableIndex=iValue;
           pClientReq->iPipeLineSubIndex=iIndex;
    }
    if(HttpClient_CreateSubRequest(pHttpGlobals,pClientReq)!=OF_SUCCESS)
    {
      HttpDebug("HttpClient_CreateSubRequest failed\r\n");
      pClientReq->bPersistentConn=FALSE;
 	    return OF_FAILURE;
    }
     
    /**
   * Start the  Keep-Alive Timer 
   **/
     if(pClientReq->bIsAcceptFd==TRUE)
     {
	      pKeepAliveConn=of_calloc(1,sizeof(HttpKeepAliveFd_t));
	       if(!pKeepAliveConn)
		{
		   HttpDebug(("of_calloc is failed for  pKeepAliveConn\r\n"));
  		   return OF_FAILURE;
		}
	      pKeepAliveConn->pKeepAliveTimer=NULL;
	      pKeepAliveConn->iSockFd=pClientReq->SockFd;
	      
	       if (Httpd_Timer_Create(& pKeepAliveConn->pKeepAliveTimer) !=OF_SUCCESS)
	       {
	              of_free(pKeepAliveConn);
		       HttpDebug("Unable create Timer");	   	       	       
		       return OF_FAILURE;
	      }	     
             if(Httpd_Timer_Start(pKeepAliveConn->pKeepAliveTimer,
                           (HTTPD_TIMER_CBFUN)(pHttpGlobals->pCbkTimers->HttpdKeepAliveTimeoutCbk),
                           (void*) pKeepAliveConn, HTTPD_TIMER_SEC,
                            HTTP_MAX_KEEPALIVE_TIMEOUT, 0)==OF_FAILURE)
              {
                    of_free(pKeepAliveConn->pKeepAliveTimer);
                    of_free(pKeepAliveConn);
                    HttpDebug("Unable start the Timer");	   	       	       
                    return OF_FAILURE;
              }
             pHttpGlobals->pHttpClientReqTable[iValue].pKeepAliveInfo=pKeepAliveConn;
     }
     return OF_SUCCESS;
}/*HttpClient_PipeLineRequest*/

/*******************************************************************************
 * Function Name : HttpClient_KeepAliveTimeOut                                 *
 * Description   : This timer-handler for timeout of keep alive connection     *
 *                 This updates the poll table and   deletes the client request*
 * Input         : pVal - Client Request                                       *
 * Output        : None                                                        *
 * Return value  : None                                                        *
 *******************************************************************************/
void HttpClient_KeepAliveTimeOut( void  *pVal )
{
  HttpKeepAliveFd_t  *pKeepAliveFD=NULL;
  uint32_t         	    uiIndex=0;
  int32_t           	    fd_iCount=0;
  bool        	    bFound=FALSE;
  int32_t          	    iLoop=0;
  HttpGlobals_t          *pHttpg=NULL;

  fd_iCount=Httpd_GetMaxFds(pHttpg); /*pHttpg->pHttpGlobalConfig->iMaxfds;*/
  if(fd_iCount<=0)
  {
       HttpDebug(" fd_iCount is invalid");
       return;
  }
  pKeepAliveFD = (HttpKeepAliveFd_t *) pVal;
  if (!pKeepAliveFD)
  {
       HttpDebug("pKeepAlivefd is NULL");
       return;
  }

 pHttpg=pHTTPG;
 if(!pHttpg)
 {
    Httpd_Error("pHttpg is NULL\r\n");
    return;
 }
 fd_iCount=Httpd_GetMaxFds(pHttpg); /*pHttpg->pHttpGlobalConfig->iMaxfds;*/
  /**
   * Reset This Reference If Present In Http Client Request Table.
   */
  for (uiIndex = pHttpg->iMaxServers; uiIndex < fd_iCount; uiIndex++)    
  {
     if (pHttpg->pHttpClientReqTable[uiIndex].iSockFd == pKeepAliveFD->iSockFd)
     {  
         for(iLoop= 0;iLoop < HTTP_MAX_PIPELINED_REQUESTS; iLoop++)
      	 {
       	    HttpClientReq_t *pClientReq=NULL;
	           pClientReq = pHttpg->pHttpClientReqTable[uiIndex].pHttpPipeline[iLoop];
	           if(pClientReq == NULL ||(pClientReq->SockFd <=0))
	           { 
	                continue;
	           }
	           else
	           {       	    		            
	                   if(HttpClient_Delete(pHttpg,pClientReq)!=OF_SUCCESS)
		            {
		               HttpDebug("Client Delete is returned failure");		              
		            }             			      	    		             	    		        
	           } 
         }         
         pHttpg->pHttpClientReqTable[uiIndex].iSockFd=-10;
         of_memset(pHttpg->pHttpClientReqTable[uiIndex].pHttpPipeline,0,4*HTTP_MAX_PIPELINED_REQUESTS);  
	  pHttpg->pHttpClientReqTable[uiIndex].pKeepAliveInfo=NULL;
         bFound=TRUE;
     }
  }  
  if(bFound==TRUE)
  {
         cm_socket_close(pKeepAliveFD->iSockFd);
         Httpd_Timer_Delete(pKeepAliveFD->pKeepAliveTimer);
         of_free(pKeepAliveFD);
  }
  return;
} /* HttpClient_KeepAliveTimeOut() */

/**************************************************************************
 * Function Name : HttpClient_ReadGet                                     * 
 * Description   : This the GET event handler for client request          *
 * Input         : pClientReq (I) - pointer to the ClientReq structure.   *
 *                 pHttpGlobals  - Global pointer                         *
 * Output        : None.                                                  *
 * Return value  : OF_SUCCESS on success and OF_FAILURE, otherwise.         *
 **************************************************************************/

int32_t   HttpClient_ReadGet(HttpGlobals_t  *pHttpGlobals,HttpClientReq_t  *pClientReq)
{
  
  if(!pHttpGlobals)
  {
    Httpd_Error("pHttpGlobals is NULL\r\n");	
    return OF_FAILURE;
  }
  
  if(pClientReq==NULL)
  {
     HttpDebug("pClientRequest is NULL");
     return OF_FAILURE;
  }
  /*HTTP/1.1 Implimetation*/ 
 /* Added the below line for BR 39811 */
 pClientReq->lOutContentLen = -1;/*Reset the reference*/
 if((pClientReq->bPersistentConn==TRUE)    && /*Keep-Alive found*/		 
		      (pClientReq->bHost==TRUE)     && /*RFC HTTP/1.1*/
                    (pClientReq->eVersion==hVersion11 ))/*HTTP/1.1 Request*/
  {
     pClientReq->eState = hStatePipeline;
     return OF_SUCCESS;
     
   }   
  else /*Normal Connection HTTP/1.0*/
   {
      pClientReq->eState = hStateSendResp;      
      return OF_SUCCESS;
   }
 return OF_SUCCESS;
}

/**************************************************************************
 * Function Name : HttpClient_ReadPost                                    *
 * Description   : This the POST event handler for client request         *
 * Input         : pClientReq (I) - pointer to the ClientReq structure.   *
 *                 pHttpGlobals  - Global pointer 						  *
 * Output        : None.                                                  *
 * Return value  : OF_SUCCESS on success and OF_FAILURE, otherwise.         *
 **************************************************************************/

int32_t   HttpClient_ReadPost(HttpGlobals_t  *pHttpGlobals,HttpClientReq_t   *pClientReq)
{
    int32_t   XtraBytes=0;

     if(!pHttpGlobals)
     {
       Httpd_Error("pHttpGlobals is NULL\r\n");	
       return OF_FAILURE;
     }
	
     if(pClientReq==NULL)
     {
        HttpDebug("pClient Request is NULL");
        return OF_FAILURE;
     }
    pClientReq->pEndOfHeader += of_strlen(CRLF_CRLF); /*  4 bytes.\r\n\r\n */
    XtraBytes = pClientReq->ReadInfo.pCur -pClientReq->pEndOfHeader;
    if (HttpClient_IsMultiPart(pClientReq))
    {
      /**
       * Content length is not supplied in header.
       */
      if (pClientReq->lInContentLen < 0)
      {
        return OF_FAILURE;
      }

      pClientReq->pMPart = (HttpMPartInfo_t *)
                                of_calloc(1, sizeof(HttpMPartInfo_t));

      if (!pClientReq->pMPart)
      {
        return OF_FAILURE;
      }

      HttpClient_SetReadInfo(pClientReq, pClientReq->pMPart->ucBuf,
                             HTTP_MPART_LEN, pClientReq->pMPart->ucBuf);

      pClientReq->eState = hStateReadMPart;
    }
    else  /* Normal Post String */
    {
      HttpClient_SetReadInfo(pClientReq, pClientReq->ucPostString,
                             HTTP_POSTSTR_LEN, pClientReq->ucPostString);
      pClientReq->eState = hStateReadPStr;
    }

    if (pClientReq->ReadInfo.ulMaxLen < XtraBytes)
    {
      XtraBytes = pClientReq->ReadInfo.ulMaxLen;
    }

    /**
     * Copy the extra bytes read.
     */
    of_memcpy(pClientReq->ReadInfo.pCur,pClientReq->pEndOfHeader, XtraBytes);
    pClientReq->ReadInfo.pCur[XtraBytes]  = '\0';
    pClientReq->ReadInfo.pCur            += XtraBytes;
    pClientReq->lOutContentLen   = XtraBytes + 1;
    
    return OF_SUCCESS;
}/*HttpClient_ReadPost*/

/**************************************************************************
 * Function Name : HttpClient_ReadHead                                    *
 * Description   : This the HEAD event handler for client request         *
 * Input         : pClientReq (I) - pointer to the ClientReq structure.   *
 *                 pHttpGlobals  - Global pointer                         *
 * Output        : None.                                                  *
 * Return value  : OF_SUCCESS on success and OF_FAILURE, otherwise.         *
 **************************************************************************/

int32_t   HttpClient_ReadHead(HttpGlobals_t  *pHttpGlobals,HttpClientReq_t   *pClientReq)
{
    if(!pHttpGlobals)
   {
     Httpd_Error("pHttpGlobals is NULL\r\n");	
     return OF_FAILURE;
   }

    if(pClientReq==NULL)
    {
       HttpDebug("pClientRequest is NULL");
       return OF_FAILURE;
    }
    if (HttpClient_SetHdrFields(pHttpGlobals,pClientReq) != OF_SUCCESS)
    {
        HttpDebug("HttpClient_SetHdrFields is return  failure"); 
        return OF_FAILURE;
    }

    if (HttpClient_SendHeader(pHttpGlobals,pClientReq)!= OF_SUCCESS)
    {
        HttpDebug("HttpClient_SendHeader is return  failure"); 
        return OF_FAILURE;
    }
   // pClientReq->eState = hStatePipeline; BugID:43776
    pClientReq->eState = hStateSendResp;
    return OF_SUCCESS;
}/*HttpClient_ReadHead*/
/**************************************************************************
 * Function Name : HttpClient_ReadHead                                    *
 * Description   : This the BAD event handler for client request          *
 * Input         : pClientReq (I) - pointer to the ClientReq structure.   *
 *                 pHttpGlobals  - Global pointer                         * 
 * Output        : None.                                                  *
 * Return value  : OF_SUCCESS on success or   OF_FAILURE on failure         *
 **************************************************************************/

int32_t   HttpClient_ReadBad(HttpGlobals_t  *pHttpGlobals,HttpClientReq_t   *pClientReq) 
{
      if(!pHttpGlobals)
      {
        Httpd_Error("pHttpGlobals is NULL\r\n");	
        return OF_FAILURE;
      }

     if(!pClientReq)
     {
        HttpDebug("pClientrequest is NULL");
        return OF_FAILURE;
     }
     pClientReq->eMethod=hEventClose;
     return  OF_SUCCESS;
}/*HttpClient_ReadBad*/
/**************************************************************************
 * Function Name : HttpClient_StartResponse                               *
 * Description   :                                                        *
 * Input         : pVal (I) - pointer to the ClientReq structure.         *
 * Output        : None.                                                  *
 * Return value  : None   											      *
 **************************************************************************/
void  HttpClient_StartResponse( void  *pVal)
{
  HttpResponceTable_t  *pRespond=NULL;  
  pRespond = (HttpResponceTable_t  *) pVal;
  if (!pRespond)
  {
       HttpDebug("pVal is NULL");
       return;
  }  
  /* Allowing the Server to Start sending responce to client */
  pRespond->bStartResponce=TRUE;       
  return;
}/* HttpClient_StartResponse*/
/**********************************************************************************
 * Function Name : HttpClient_CreateSubRequest								      *	
 * Description   : this routine used to crate sub Client request       		      *	 
 * Input         : HttpClientReq -- pClientReq 									  * 
 *                 pHttpGlobals  - Global pointer   	                          *
 * Output        :None	   												  		  *
 * Return value  :OF_SUCCESS on success and OF_FAILURE on error				      *	
 **********************************************************************************/

int32_t HttpClient_CreateSubRequest(HttpGlobals_t  *pHttpGlobals,HttpClientReq_t *pClientReq)
{
  int32_t   iIndex=0; 
  HttpClientReq_t *pNewClientReq=NULL;   
  uint32_t ulAppDataLen=0;  
  uint32_t ulUrlLength=0;

  if(!pHttpGlobals)
  {
    Httpd_Error("pHttpGlobals is NULL\r\n");	
    return OF_FAILURE;
  }
  
  if(pClientReq==NULL)
  {
      HttpDebug(" pClientRequest is NULL");
      return OF_FAILURE;
  }
  pNewClientReq = (HttpClientReq_t *) of_calloc(1, sizeof(HttpClientReq_t));
  if (!pNewClientReq)
  {
         HttpDebug("Unable To Allocate Client Request");
         pHttpGlobals->HttpServers[pClientReq->lServId].Stats.ulClientAllocFails++;
         return OF_FAILURE;
  }
  ulAppDataLen=Httpd_GetAppDataLength(pHttpGlobals); /*pHttpGlobals->pHttpGlobalConfig->ulAppDataLength;*/
  ulUrlLength=Httpd_GetUrlSize(pHttpGlobals);/*pHttpGlobals->pHttpGlobalConfig->ulUrlLength;*/
  pNewClientReq->ucCookieVal=of_calloc(1,Httpd_GetMaxCookieValueLen(pHttpGlobals)+1);
  if (!pNewClientReq->ucCookieVal)
  {
     of_free(pNewClientReq);
     HttpDebug("Unable To Allocate Client Request");
     return OF_FAILURE;
  }
  of_strcpy((char *) pNewClientReq->ucCookieVal,(char *) pClientReq->ucCookieVal);
  pNewClientReq->pTimer = NULL;     
  if (Httpd_Timer_Create(&pNewClientReq->pTimer) !=OF_SUCCESS)
  {
      HttpDebug("Unable To start the timer For ReqStr");
      if(HttpClient_Free(pHttpGlobals,pNewClientReq)!=OF_SUCCESS)
      {
         HttpDebug("client free failed");
         return OF_FAILURE;
      }
      return OF_FAILURE;
  } 

  /*we initialise the required fields*/
  pNewClientReq->lServId  = pClientReq->lServId;  
  pNewClientReq->PeerIp   = pClientReq->PeerIp;   
  pNewClientReq->PeerPort = pClientReq->PeerPort ;
  pNewClientReq->SockFd   = pClientReq->SockFd;  
  pNewClientReq->SrcPort  = pClientReq->SrcPort;
  pNewClientReq->pSpData  = pClientReq->pSpData;
  pNewClientReq->lInContentLen  = -1;
  pNewClientReq->lOutContentLen = -1;
  pNewClientReq->eMethod=hMethodBad;
  pNewClientReq->eState   = hStateReadReq;        
  pNewClientReq->bIsHalfPacketRecv=FALSE;
  pNewClientReq->bPersistentConn=FALSE; 
  pNewClientReq->bIsPipeLined=FALSE; 
  pNewClientReq->iPipeLineSubIndex=-1;
  pNewClientReq->bRefererFound = FALSE;
  pNewClientReq->iPipeLineTableIndex = pClientReq->iPipeLineTableIndex;
  pNewClientReq->ulTotalFileSize=0;
  pNewClientReq->ucRemainBuffer = NULL;
#ifdef INTOTO_CMS_SUPPORT
  pNewClientReq->bDoNotRemoveRequest = FALSE;
#endif  /* INTOTO_CMS_SUPPORT */
  pNewClientReq->bISPgUserLoginReq  = TRUE;
  pNewClientReq->ucAppData=of_calloc(1,ulAppDataLen+1);
  if(!pNewClientReq->ucAppData)
  {
     if(HttpClient_Free(pHttpGlobals,pNewClientReq) != OF_SUCCESS)
     {
        HttpDebug(("client free failed"));        
     }
     HttpDebug(("of_calloc failed for pNewClientReq->ucAppData\r\n"));
     return OF_FAILURE;
  }
  pNewClientReq->ucUrl=of_calloc(1,ulUrlLength+1);
  if(!pNewClientReq->ucUrl)
  {
     if(HttpClient_Free(pHttpGlobals,pNewClientReq) != OF_SUCCESS)
     {
        HttpDebug(("client free failed"));       
     }
     HttpDebug(("of_calloc failed for pNewClientReq->ucUrl \r\n"));
     return OF_FAILURE;
  }
  pNewClientReq->pGlobalData     = (HttpGlobals_t *)pHttpGlobals;
  HttpClient_SetReadInfo(pNewClientReq, pNewClientReq->ucReqStr, HTTP_REQSTR_LEN,
                         pNewClientReq->ucReqStr);
  of_strcpy((char *) pNewClientReq->ucInContentType, HTTPD_DEF_CONT_TYPE);
 if(Httpd_Timer_Start(pNewClientReq->pTimer,
                           (HTTPD_TIMER_CBFUN)(pHttpGlobals->pCbkTimers->HttpdReqStrTimeoutCbk),
                           (void*) pNewClientReq, HTTPD_TIMER_SEC,
                           HTTPD_REQSTR_TIMEOUT, 0)!=OF_SUCCESS)
 {
   if(HttpClient_Free(pHttpGlobals,pNewClientReq) != OF_SUCCESS)
   {
        HttpDebug(("client free failed"));       
   }
   HttpDebug("Unable To start the timer \r\n");      
   return OF_FAILURE;
 }   
  iIndex=pClientReq->iReqTableIndex;    
  pHttpGlobals->pHttpReqTable[iIndex] = NULL;
  pHttpGlobals->pHttpReqTable[iIndex] = pNewClientReq;
  pNewClientReq->iReqTableIndex=iIndex; 
  pHttpGlobals->HttpServers[pClientReq->lServId].Stats.ulClientAllocs++;
  pClientReq->pSendHeader = NULL;
  pClientReq->bNeedMoreData = FALSE;
  return OF_SUCCESS;
}/* HttpClient_CreateSubRequest*/

/**************************************************************************
 * Function Name : HttpClient_StartProcessing                             *
 * Description   :                                                        *
 * Input         : pClientReq (I) - pointer to the ClientReq structure.   *
                   pHttpGlobals  - Global pointer                         *
 * Output        : None.                                                  *
 * Return value  : OF_SUCCESS on success and OF_FAILURE, otherwise.         *
 **************************************************************************/

int32_t   HttpClient_StartProcessing(HttpGlobals_t *pHttpGlobals,HttpClientReq_t  *pClientReq)
{
  unsigned char     *pEoh=NULL;  
  int32_t       oldState=0;  
  int32_t        iResult=0;

  if(!pHttpGlobals)
  {
    Httpd_Error("pHttpGlobals is NULL\r\n");	
    return OF_FAILURE;
  }
  if(pClientReq==NULL)
  {
        HttpDebug("pClientRequest is NULL");
        return OF_FAILURE;
  }  
  while (TRUE)
  {
        oldState = pClientReq->eState;
        if(pClientReq->eState==hStateReadReq)
        {
                 pEoh = (unsigned char *) of_strstr((char *) pClientReq->ucReqStr, CRLF_CRLF);                 
                  if (!pEoh)
                  {
                       if(pClientReq->ucRemainBuffer == NULL) /*if NULL then only allocate remain buffer*/
                       {
                       pClientReq->ucRemainBuffer=of_calloc(1,HTTP_REQSTR_LEN+1);
			     if(!pClientReq->ucRemainBuffer)
			     {
			             HttpDebug("pClientRequest->ucReminBuffer is NULL");
       			      return OF_FAILURE;
			     }
                       }
			  if(pClientReq->ucRemainBuffer  !=  NULL)
		  	  {
                       of_strcpy((char *) pClientReq->ucRemainBuffer,(char *) pClientReq->ucReqStr);
			  }
			  pClientReq->bNeedMoreData = TRUE;
                       return OF_SUCCESS; /* Not Yet Received Header */
                   }            
                  *pEoh = '\0'; /* Make it a string to use with ParseRequest */        
                   pClientReq->pEndOfHeader=pEoh;
         }
        if((iResult=HttpStateMachineFunc(pHttpGlobals,pClientReq))!=OF_SUCCESS)
        {
              if(iResult==OF_FAILURE)
              {
	              HttpDebug("state machine failure");
	              return OF_FAILURE;
              }
              else if( iResult== HTTP_DONOT_SEND_RESPONCE)
      	        {
		         HttpDebug("HTTP_DONOT_SEND_RESPONCE");
	                return HTTP_DONOT_SEND_RESPONCE;
	 	 } 
         }  
         if (oldState == pClientReq->eState)
         {            
             break;
         }
  }
  return OF_SUCCESS;
}
/**************************************************************************
 * Function Name : HttpClient_MultiPartProcess                            *
 * Description   : This is the state handler for 'Read MultiPart Data'.   * 
 * Input         : pClientReq (I) - pointer to the ClientReq structure.   * 
 *                 pHttpGlobals  - Global pointer                         *
 * Output        : None.                                                  *
 * Return value  : OF_SUCCESS on success and OF_FAILURE, otherwise.         *
 **************************************************************************/

int32_t HttpClient_MultiPartProcess(HttpGlobals_t  *pHttpGlobals,HttpClientReq_t  *pClientReq )
{
 unsigned char ucExt[HTTP_FILE_EXTENTION_MAX_LEN + 1];
 uint16_t usIsImage = FALSE;

 if(!pHttpGlobals)
 {
    Httpd_Error("pHttpGlobals is NULL\r\n");	
    return OF_FAILURE;
 }
 
/*we need to Reset the Muttex socket fd*/
  if(pClientReq==NULL)
  {
        HttpDebug("pClient Request is NULL");
        return OF_FAILURE;
  }
 pHttpGlobals->MutexSockFd = -1;
 if(pClientReq->eState==hStateReadMPart)
 	{       
      if (Httpd_GetFileExt(pClientReq->ucFileName, ucExt, HTTP_FILE_EXTENTION_MAX_LEN) != OF_SUCCESS)
      {
             of_strcpy((char *) ucExt, "htm");
      }
      if ((!of_strcmp((char *) ucExt, "bin")) ||
          (!of_strcmp((char *) ucExt, "cfg")) ||
          (!of_strcmp((char *) ucExt, "tgz")) ||
#ifdef INTOTO_CMS_SUPPORT
          (!of_strcmp((char *) ucExt, "cer")) ||
#endif  /* INTOTO_CMS_SUPPORT */
          (!of_strcmp((char *) ucExt, "dat")) ||
          (pHttpGlobals->MutexSockFd == pClientReq->SockFd))
         {
       	usIsImage = TRUE;
          }
       if (usIsImage == TRUE)
       {
	         if (HttpClient_UploadBinFile(pHttpGlobals,pClientReq) != OF_SUCCESS)
	        {
			#ifdef HTTPD_DEBUG
	         	 Httpd_Print("\n\rHttpClient_Process(): HttpClient_UploadBinFile");
	          	 Httpd_Print(" fail for %s file\n\r", pClientReq->ucFileName);
			#endif
	          	  if(HttpClient_Delete(pHttpGlobals,pClientReq)!=OF_SUCCESS)
			   {
			        HttpDebug("Client Delete is returned failure");			        
			   }
	          	   return OF_FAILURE;
	        }
       }
       else
       { 	        
             /* Reset the mutex socket fd.*/             
	        pHttpGlobals->MutexSockFd = -1;
	        if (HttpClient_ReadMPart_ReadEvent(pClientReq) != OF_SUCCESS)
	        {
		           if(HttpClient_Delete(pHttpGlobals,pClientReq)!=OF_SUCCESS)
    			    {
    			           HttpDebug("Client Delete is returned failure");	   			    
	   		    }
 		           return OF_FAILURE;
	        }
         }
 }
    return OF_SUCCESS;
}
/*************************************************************************
 * Function Name : Httpd_CheckUserLogin                                  *
 * Description   : This function is used to check whether the ssl based  *
 *                 connections can be established or not. That is https  *
 *                 channel is enabled or not.                            *
 * Input         : pHttpRequest (I) - pointer to the http request        *
 *                                    structure.                         *
 *                 pUName       (I) - pointer to the user name.          *
 *                 pHttpGlobals  - Global pointer                        *
 * Output        : Boolean value representing the secured server is      *
 *                 enabled or not.                                       *
 * Return value  : TRUE if the secured server is enabled, otherwise FALSE*
 *************************************************************************/

bool Httpd_CheckUserLogin(HttpGlobals_t  *pHttpGlobals,HttpRequest  *pHttpRequest,
                              char      *pUName )
{
  HttpClientReq_t  *pClientReq;
  char       ucMsg[HTTP_LOCAL_BUFFER_LEN];

  /**
   * Validate the passed parameter(s).
   */
   if(!pHttpGlobals)
  {
    Httpd_Error("pHttpGlobals is NULL\r\n");	
    return OF_FAILURE;
  }
  if ((!pHttpRequest) ||
      (!pUName))
  {
    return FALSE;
  }

  /**
   * Get the client request from the passed http request parameter and
   * validate it.
   */
  pClientReq = (HttpClientReq_t *) pHttpRequest->pPvtData;

  if (!pClientReq)
  {
    return FALSE;
  }

  /**
   * Verify whether the secured server is enabled or not.
   */
  if (pHttpGlobals->HttpServers[pClientReq->lServId].Type == hServerNormal)
  {
	of_strcpy(ucMsg,"<table width='900'><tr><td align=center><font face='Verdana, Arial, Helvetica, sans-serif' size='3'><BR>For all the Remote users<BR>we are allowing secure sessions only<BR><a href=\"login.htm\">Login Again</a><BR> please verify the certificates before using https session</td></tr></table>");
    if (HttpSendHtmlFile(pHttpRequest, (unsigned char*)("loginout.htm"),
          (unsigned char *) ucMsg) == OF_FAILURE)
    {
      Httpd_Debug("Failed to send errscrn.htm file.");
    }
    return FALSE;
  }

  return TRUE;
} /* Httpd_CheckUserLogin() */

#ifdef OF_XML_SUPPORT
/*************************************************************************
 * Function Name :Httpd_SendXMLBuff                                            *
 * Description   : This function is used to send XML buffer and Header of given *
 *                 size to the client.                                   *
 * Input         : pHttpRequest (I) - pointer to the http request        *
 *                                    structure.                         *
 *                 pHtmlBuf     (I) - pointer to the html buffer to be   *
 *                                    sent.                              *
 *                 ulLength     (I) - length of the html buffer to be    *
 *                                    sent.                              *
 * Output        : length of the buffer or the value, -1.                *
 * Return value  : OF_SUCCESS if successfully sent, otherwise OF_FAILURE.  *
 *************************************************************************/
int32_t Httpd_SendXMLBuff( HttpRequest  *pHttpRequest,
				                    char      *pHtmlBuf,
				                    uint32_t     ulLength )
{

	HttpClientReq_t  *pClientReq;
        HttpGlobals_t      *pHttpg=NULL;
        int32_t  lCount = 0; 
  	char  *pErrBuf;
	/**
	 * First set the Header Fields
	 */

	pClientReq=(HttpClientReq_t *)pHttpRequest->pPvtData;
	
       pHttpg=(HttpGlobals_t *)pClientReq->pGlobalData;
	if(!pHttpg)
	{
	  Httpd_Error("pHttpg is NULL\r\n");
	  return OF_FAILURE;
	}	

 if (of_strnicmp ((char *) pHtmlBuf, "<html>", 6) == 0)
 {
   of_strcpy((char *) pClientReq->ucOutContentType ,"text/html");
 }
 else if(of_strstr((char *) pClientReq->ucFileName, ".xml"))
	{
          if(!bIsLogoFile)
          {
	    lCount = ulNonLogoFileReq_g/ulLogoFileReq_g;
            if(lCount > 20)
	    {
 	      of_strcpy((char *) pClientReq->ucOutContentType ,"text/html");
    	      pErrBuf=of_calloc(1,of_strlen(LOGO_ERROR_STRING)+1);	
  	      if(!pErrBuf)
              {
                return OF_FAILURE;
              }
              of_strcpy(pErrBuf,LOGO_ERROR_STRING);
 	      ulLength=of_strlen(pErrBuf);
              ulNonLogoFileReq_g = 1;
              ulLogoFileReq_g = 1;
	      if(HttpClient_SetHdrFields(pHttpg,pClientReq ) != OF_SUCCESS)
	      {
  	        of_free(pErrBuf);
	    	return OF_FAILURE;
	      }
              
	      if(Httpd_Send(pHttpg,pHttpRequest,pErrBuf, ulLength )==ulLength)
	      {
  	        of_free(pErrBuf);
	 	return OF_SUCCESS;
	      }
	      else
	      {
  	        of_free(pErrBuf);
		return OF_FAILURE;
	      }
              return OF_SUCCESS;
            }
          }
 
		of_strcpy((char *) pClientReq->ucOutContentType ,"text/xml");
	}
	else
	{
		of_strcpy((char *) pClientReq->ucOutContentType ,"text/html");
	}

	pClientReq->lOutContentLen=ulLength; /*Content length is required for HTTP/1.1 header*/
	if(HttpClient_SetHdrFields(pHttpg,pClientReq ) != OF_SUCCESS)
	{
		return OF_FAILURE;
	}
	/**
	 *  Send Xml Buffer
	 */

	if(Httpd_Send(pHttpg,pHttpRequest,pHtmlBuf, ulLength )==ulLength)
	{
		return OF_SUCCESS;
	}
	else
	{
		return OF_FAILURE;
	}
								
	return OF_SUCCESS;

}
#endif /*OF_XML_SUPPORT*/

/*************************************************************************
 * Function Name : Httpd_Send                                            *
 * Description   : This function is used to send an html buffer of given *
 *                 size to the client.                                   *
 * Input         : pHttpRequest (I) - pointer to the http request        *
 *                                    structure.                         *
 *                 pHtmlBuf     (I) - pointer to the html buffer to be   *
 *                                    sent.                              *
 *                 ulLength     (I) - length of the html buffer to be    *
 *                                    sent.                              *
 *                 pHttpGlobals  - Global pointer                        *
 * Output        : length of the buffer or the value, -1.                *
 * Return value  : OF_SUCCESS if successfully sent, otherwise OF_FAILURE.  *
 *************************************************************************/

int32_t Httpd_Send(HttpGlobals_t *pHttpGlobals,HttpRequest  *pHttpRequest,
                    char      *pHtmlBuf,
                    uint32_t     ulLength )
{
  HttpClientReq_t  *pClientReq = NULL;

  /**
   * Validate the passed parameter(s).
   */
  if(!pHttpGlobals)
  {
    Httpd_Error("pHttpGlobals is NULL\r\n");	
    return OF_FAILURE;
  }
  
  if ((!pHttpRequest) ||
      (!pHtmlBuf))
  {
    return OF_FAILURE;
  }

  /**
   * Get the client request from the passed http request parameter and
   * validate it.
   */
  pClientReq = (HttpClientReq_t *) pHttpRequest->pPvtData;

  if (!pClientReq)
  {
    return OF_FAILURE;
  }

  if (HttpClient_Send(pClientReq, pHtmlBuf, ulLength) != OF_SUCCESS)
  {
    return -1;
  }
  else
  {
    return ulLength;
  }
} /* Httpd_Send() */

/*************************************************************************
 * Function Name : HttpClient_MethStr2Enum                               *
 * Description   : This function is used to get the enumerated value for *
 *                 the passed method name.                               *
 * Input         : pMethName (I) - pointer to the method name string. For*
 *                                 example, GET, POST, etc.              *
 * Output        : Equivalent enumerated value of the method.            *
 * Return value  : Equivalent enumerated value on success, otherwise     *
 *                 hMethodBad enumerated value.                          *
 *************************************************************************/


int32_t HttpClient_MethStr2Enum( unsigned char  *pMethName )
{
  HttpMethodInfo  *pHttpMethodInfo;

  /**
   * Validate the passed parameter(s).
   */
  if (!pMethName)
  {
    /**
     * If the method name is null return enumerated value, hMethodBad.
     */
    return hMethodBad;
  }

  /**
   * Initialize the local variable.
   */
  pHttpMethodInfo = httpMethods_f;

  /**
   * Search for the passed method name in the http method list.
   */
  while (pHttpMethodInfo->hmId != hMethodBad)
  {
    if (!of_strcmp((char *) pMethName, pHttpMethodInfo->hmName))
    {
      break;
    }

    ++pHttpMethodInfo;
  }

  /**
   * Return the searched enumerated value for the passed method name.
   */
  return pHttpMethodInfo->hmId;
} /* HttpClient_MethStr2Enum() */

/*************************************************************************
 * Function Name : HttpClient_VersionStr2Enum                            *
 * Description   : This function is used to get the enumerated value for *
 *                 the passed version name.                              *
 * Input         : pVersionName (I) - pointer to the version name string.*
 *                                    For example, HTTP/1.0, HTTP/1.1,   *
 *                                    etc.                               *
 * Output        : Equivalent enumerated value of the version.           *
 * Return value  : Equivalent enumerated value on success, otherwise     *
 *                 hVersionBad enumerated value.                         *
 *************************************************************************/


int32_t HttpClient_VersionStr2Enum( unsigned char  *pVersionName )
{
  HttpVersionInfo  *pHttpVersionInfo;

  /**
   * Validate the passed parameter(s).
   */
  if (!pVersionName)
  {
    return hVersionBad;
  }

  /**
   * Initialize the local variable.
   */
  pHttpVersionInfo = httpVersions_f;

  /**
   * Search for the passed version name in the http version list.
   */
  while (pHttpVersionInfo->hvId != hVersionBad)
  {
    if (!of_strcmp((char *) pVersionName, pHttpVersionInfo->hvName))
    {
      break;
    }

    ++pHttpVersionInfo;
  }

  /**
   * Return the searched enumerated value for the passed version name.
   */
  return pHttpVersionInfo->hvId;
} /* HttpClient_VersionStr2Enum() */

/*************************************************************************
 * Function Name : HttpClient_Send                                       *
 * Description   : This function is used to send the given buffer through*
 *                 the socket. It sends at the most 512 bytes at a time. *
 * Input         : pReq     (I) - pointer to the version name string. For*
 *                 pHtmlBuf       example, HTTP/1.0, HTTP/1.1, etc.      *
 *                 ulLength       example, HTTP/1.0, HTTP/1.1, etc.      *
 * Output        : Equivalent enumerated value of the version.           *
 * Return value  : Equivalent enumerated value on success, otherwise     *
 *                 hVersionBad enumerated value.                         *
 *************************************************************************/

/***********************************************************************
 * Function Name : HttpClient_Send                                     *
 * Description   : Sends the given buffer through the socket           *
 *                 sends only <= 512 bytes at a time                   *
 * Input         : pReq - Socket Descriptor                            *
 *                 pHtmlBuf - Buffer To Send                           *
 *                 ulLength - Length of the Buffer                     *
 * Output        : None                                                *
 * Return value  : OF_SUCCESS on success                                *
 *                 OF_FAILURE on failure.                               *
 ***********************************************************************/

int32_t HttpClient_Send( HttpClientReq_t  *pClientReq,
                         char          *pHtmlBuf,
                         uint32_t         ulLength )
{
  HttpGlobals_t  *pHttpg=NULL;
 
  pClientReq->ulTotalFileSize+= ulLength;
  pHttpg=(HttpGlobals_t *)pClientReq->pGlobalData;
  if(!pHttpg)
  {
    Httpd_Error("pHttpgis NULL\r\n");	
    return OF_FAILURE;
  }
  return HttpWriteToTransmitBuffer(pHttpg,pClientReq, (unsigned char *) pHtmlBuf, ulLength);
} /* HttpClient_Send() */

/***********************************************************************
 * Function Name : Httpd_GetCookieValue                                *
 * Description   : Read Cookie Value from the Browser
 * Input         : pHttpRequest (I) - Pointer To Http Request structure.*
 *                 pCookieName - Cookie Name, MaxLen - CookieValue Len *
 * Output        : value_p - Requested Cookie Value                     *
 * Return value  : None                                                *
  ***********************************************************************/

int32_t Httpd_GetCookieValue( HttpRequest  *pHttpRequest,
                              unsigned char     *pCookieName,
                              unsigned char     *value_p,
                              uint16_t     usMaxLen )
{
   HttpClientReq_t       *pClientReq = NULL;
   HttpdGetCookieInfo_t  *pGetTemp;
   unsigned char              Name[32]    ="";
   unsigned char              *endP       = NULL;

  /**
   * Validate the passed parameter(s).
   */
  if (!pHttpRequest)
  {
    return OF_FAILURE;
  }

  /**
   * Get the client request from the passed http request parameter and
   * validate it.
   */
  pClientReq = (HttpClientReq_t *) pHttpRequest->pPvtData;

  if (!pClientReq)
  {
    Httpd_Error("Httpd_GetCookieValue:: pClientReq == NULL");
    return OF_FAILURE;
  }

  /**
   * Validate the passed parameter(s).
   */
  if ((!value_p) ||
      (!pCookieName))
  {
    return OF_FAILURE;
  }

  pGetTemp = pClientReq->pGetCookieHead;

  while (pGetTemp != NULL)
  {
    if ((pGetTemp->pCookieName == NULL) ||
        (pGetTemp->value_p == NULL))
    {
      pGetTemp=pGetTemp->pNext;
      continue;
    }
    endP = (unsigned char *) of_strchr((char *) pGetTemp->pCookieName, '=');

    if (endP)
    {
      if ((endP-pGetTemp->pCookieName) > 32)
      {
        return OF_FAILURE;
      }
      of_strncpy((char *) Name, (char *) pGetTemp->pCookieName, endP-pGetTemp->pCookieName);
      Name[endP-pGetTemp->pCookieName] = '\0';
    }
    else
    {
      return OF_FAILURE;
    }

    if (!of_strcmp((char *) Name, (char *) pCookieName))
    {
      endP = (unsigned char *) of_strchr((char *) pGetTemp->value_p, ';');

      if (endP)
      {
        if ((endP-pGetTemp->value_p) > usMaxLen)
        {
          return OF_FAILURE;
        }

        of_strncpy((char *) value_p, (char *) pGetTemp->value_p,  endP-pGetTemp->value_p);
        value_p[endP-pGetTemp->value_p] = '\0';
      }
      else
      {
        if (of_strlen((char *) pGetTemp->value_p) > usMaxLen)
        {
          return OF_FAILURE;
        }
        of_strcpy((char *) value_p, (char *) pGetTemp->value_p);
      }
      return OF_SUCCESS;
    }
    pGetTemp = pGetTemp->pNext;
  }

  /*Httpd_Error("Httpd_GetCookieValue::  Cookie Value Not found\n\r");*/
  return OF_FAILURE;
} /* Httpd_GetCookieValue() */

/***********************************************************************
 * Function Name : Httpd_SendCookieInfo                                *
 * Description   : To Send the New Generic Cookie to the Browser
 * Input         : pHttpRequest - Pointer To HttpRequest               *
 * Output        : None                                                *
 * Return value  : None                                                *
 ***********************************************************************/

int32_t Httpd_SendCookieInfo( HttpRequest  *pHttpRequest )
{
  HttpClientReq_t  *pClientReq = NULL;
  HttpGlobals_t     *pHttpg=NULL;
  /**
   * Validate the passed parameter(s).
   */
  if (!pHttpRequest)
  {
    return OF_FAILURE;
  }

  /**
   * Get the client request from the passed http request parameter and
   * validate it.
   */
  pClientReq = (HttpClientReq_t *) pHttpRequest->pPvtData;

  if (!pClientReq)
  {
    Httpd_Error("Httpd_SetCookieInfo::  pClientReq == NULL");
    return OF_FAILURE;
  }
  pHttpg=(HttpGlobals_t *)pClientReq->pGlobalData;
  if(!pHttpg)
  {
    Httpd_Error("pHttpg is NULL\r\n");
    return OF_FAILURE;
  }
  /* API provided to Application to SetCookieHeader and Send Cookie */
  if (HttpClient_SetHdrFields(pHttpg,pClientReq) != OF_SUCCESS)
  {
    return OF_FAILURE;
  }

  return OF_SUCCESS;
} /* Httpd_SendCookieInfo() */

/***********************************************************************
 * Function Name : Httpd_SetCookieInfo                                 *
 * Description   : Set the New Application cookie
 * Input         : pReq - Pointer To HttpRequest                       *
 * Output        : None                                                *
 * Return value  : None                                                *
  ***********************************************************************/

int32_t Httpd_SetCookieInfo( HttpRequest  *pHttpRequest,
                             unsigned char     *pCookieName,
                             unsigned char     *value_p,
                             unsigned char     *pPath,
                             uint32_t     ulTime )
{
  HttpClientReq_t       *pClientReq = NULL;
  HttpdSetCookieInfo_t  *pSetTemp = NULL;
  HttpdSetCookieInfo_t  *pTemp;
  uint16_t              usLen;
  unsigned char              *pStart = NULL;
  int32_t                 iCookieMaxBufferLen;
  HttpGlobals_t          *pHttpg=NULL;
  /**
   * Validate the passed parameter(s).
   */
  if (!pHttpRequest)
  {
    return OF_FAILURE;
  } 
  
   /**
   * Get the client request from the passed http request parameter and
   * validate it.
   */
  pClientReq = (HttpClientReq_t *) pHttpRequest->pPvtData;

  if (!pClientReq)
  {
    Httpd_Error("Httpd_SetCookieInfo::  HttpClientReq == NULL");
    return OF_FAILURE;
  }

  pHttpg=(HttpGlobals_t *)pClientReq->pGlobalData;
  if(!pHttpg)
  {
    Httpd_Error("pHttpg is NULL\r\n");
    return OF_FAILURE;
  }
  iCookieMaxBufferLen=Httpd_GetMaxCookieBufferLen(pHttpg);/*pHttpg->pHttpGlobalConfig->ulHttpdMaxCookieBufferLen;*/
  if (pHttpg->pFreeSetCookieInfo != NULL)
  {
    pSetTemp  = pHttpg->pFreeSetCookieInfo;
    if(!pSetTemp->CookieBuff)
    {
	    pSetTemp->CookieBuff=of_calloc(1,iCookieMaxBufferLen+1);
	    if (!pSetTemp->CookieBuff)
	    {
	      Httpd_Error("Httpd_SetCookieInfo::  pSetTemp cookiebuff calloc failed");
	      return OF_FAILURE;
	    }
    }
    pHttpg->pFreeSetCookieInfo = pHttpg->pFreeSetCookieInfo->pNext;
    pSetTemp->bHeep    = 1;
  }
  else
  {
    pSetTemp = (HttpdSetCookieInfo_t *)
                                of_calloc(1, sizeof(HttpdSetCookieInfo_t));        
    if (!pSetTemp)
    {
      Httpd_Error("Httpd_SetCookieInfo::  pSetTemp calloc failed");
      return OF_FAILURE;
    }
    if(!pSetTemp->CookieBuff)
    {
	    pSetTemp->CookieBuff=of_calloc(1,iCookieMaxBufferLen+1);
	    if (!pSetTemp->CookieBuff)
	    {
	      of_free(pSetTemp);
	      Httpd_Error("Httpd_SetCookieInfo::  pSetTemp cookiebuff calloc failed");
	      return OF_FAILURE;
	    }
    }
     pSetTemp->bHeep = 0;
  }
  usLen  = of_strlen((char *) pCookieName);
  usLen += of_strlen((char *) value_p);

  if (pPath != NULL)
  {
    usLen += of_strlen((char *) pPath);
  }

  usLen += 40; /* Additional chars */

  /**
   * Prepare CookieBuff ="Set-Cookie: pCookieName=value_p; path=pPath;
   * Max-age=ulTime; CRLF".
   */
  if (usLen <= iCookieMaxBufferLen)
  {
    if (pPath == NULL)
    {
#ifdef INTOTO_CMS_SUPPORT
      usLen = sprintf(pSetTemp->CookieBuff,
                        "Set-Cookie: Name-%s=%s; path=/; Max-age=%ld; %s",
                        pCookieName, value_p, ulTime, CRLF);
#else
      usLen = sprintf((char *) pSetTemp->CookieBuff,
                        "Set-Cookie: Name-%s=%s; path=/; %s",
                        pCookieName, value_p, CRLF);
#endif  /* INTOTO_CMS_SUPPORT */
    }
    else
    {
      /*usLen = sprintf(pSetTemp->CookieBuff,
                       "Set-Cookie: Name-%s=%s; path=%s; Max-age=%ld; %s",
                        pCookieName, value_p, pPath, ulTime, CRLF);*/
      usLen = sprintf((char *) pSetTemp->CookieBuff,
                       "Set-Cookie: Name-%s=%s; path=%s; %s",
                        pCookieName, value_p, pPath, CRLF);
    }
  }
  else
  {
      Httpd_Error("Httpd_SetCookieInfo:: CookieBuff overflow\n");
      if(pSetTemp->bHeep == 0)
      	{
      		  if(pSetTemp->CookieBuff)
      		  {
                  of_free(pSetTemp->CookieBuff);	    
                  pSetTemp->CookieBuff=NULL;
              }
              of_free(pSetTemp);	     
              return OF_FAILURE;
      	}
      else
      	{
             if(pSetTemp->CookieBuff)
      		  {
                  of_free(pSetTemp->CookieBuff);	   
                  pSetTemp->CookieBuff=NULL;
              }
              return OF_FAILURE;
      	}
 }

  pStart = (unsigned char *) of_strstr((char *) pSetTemp->CookieBuff, "Name-");
  pStart += of_strlen("Name-");
  pSetTemp->pCookieName = pStart;

  pStart = (unsigned char *) of_strchr((char *) pSetTemp->CookieBuff, '=');
  if (!pStart)
   {
       Httpd_Error("Httpd_SetCookieInfo:: NULL value assigned\n");
       if(pSetTemp->bHeep == 0)
   	{
        	if(pSetTemp->CookieBuff)
      		  {
                  of_free(pSetTemp->CookieBuff);	    
                  pSetTemp->CookieBuff=NULL;
              }
     	      of_free(pSetTemp);            
              return OF_FAILURE;
   	}
       else
       {
               if(pSetTemp->CookieBuff)
      		   {
                  of_free(pSetTemp->CookieBuff);	     
                  pSetTemp->CookieBuff=NULL;
               }     	       
               return OF_FAILURE;       
       }
   }

  pSetTemp->value_p = pStart+1;

  pStart = (unsigned char *) of_strstr((char *) pSetTemp->CookieBuff, "path=");
  pStart += of_strlen("path=");
  pSetTemp->pPath  = pStart;
  pSetTemp->ulTime = ulTime;
  pSetTemp->pNext  = NULL;

  if (pClientReq->pSetCookieHead == NULL)
  {
    pClientReq->pSetCookieHead = pSetTemp;
  }
  else
  {
    /* Add to end of List */
    pTemp = pClientReq->pSetCookieHead;

    while (pTemp->pNext != NULL)
    {
      pTemp = pTemp->pNext;
    }

    pTemp->pNext = pSetTemp;
  }
  return OF_SUCCESS;
} /* Httpd_SetCookieInfo() */

/***********************************************************************
 * Function Name : HttpRequest_SetPageType                             *
 * Description   : This is part of API. This sets the current page type*
 *                 ex: LOGINPAGEREQUEST                                *
 * Input         : pHttpRequest (I) - Pointer To HttpRequest           *
 *                 ulPageType - see httpgdef.h for page types          *
 * Output        : None                                                *
 * Return value  : None                                                *
  ***********************************************************************/

void HttpRequest_SetPageType( HttpRequest  *pHttpRequest,
                                uint32_t     ulPageType )
{
  HttpClientReq_t  *pClientReq = NULL;

  /**
   * Validate the passed parameter(s).
   */
  if (!pHttpRequest)
  {
    return;
  }

  /**
   * Get the client request from the passed http request parameter and
   * validate it.
   */
  pClientReq = (HttpClientReq_t *) pHttpRequest->pPvtData;

  if (!pClientReq)
  {
    return;
  }

  pClientReq->ulPageType = ulPageType;
} /* HttpRequest_SetPageType() */

/***********************************************************************
 * Function Name : Httpd_x2c                                           *
 * Description   : This converts hex bytes to characters in URL        *
 * Input         : what - start of buffer                              *
 * Output        : None                                                *
 * Return value  : Converted character                                 *
 ***********************************************************************/


char Httpd_x2c( unsigned char  *what )
{
  register char digit;

  digit = ((what[0] >= 'A') ? ((what[0] & 0xdf) - 'A')+10 : (what[0] - '0'));
  digit *= 16;
  digit += (what[1] >= 'A' ? ((what[1] & 0xdf) - 'A')+10 : (what[1] - '0'));
  return(digit);
} /* Httpd_x2c() */

/***********************************************************************
 * Function Name : Httpd_UnEscapeUrl                                   *
 * Description   : This removes escape characters(%XX stuff) from URL  *
 * Input         : url - Url String To Be Converted                    *
 * Output        : None                                                *
 * Return value  : None                                                *
 ***********************************************************************/


void Httpd_UnEscapeUrl( unsigned char  *url )
{
  register int32_t  x, y;

  for (x = 0, y = 0; url[y]; ++x, ++y)
  {
    if ((url[x] = url[y]) == '%')
    {
      url[x] = Httpd_x2c(&url[y+1]);
      y += 2;
    }
  }
  url[x] = '\0';
} /* Httpd_UnEscapeUrl() */

/***********************************************************************
 * Function Name : HttpDef_ClientAccpet                                *
 * Description   : This is the default post-accept handler             *
 *                 Useful for SSL stuff.                               *
 * Input         : pSer  - Current Server Context                      *
 *                 pReq  - Newly Accepted Client Context               *
 * Output        : None                                                *
 * Return value  : OF_SUCCESS on success and OF_FAILURE on error         *
 ***********************************************************************/


int32_t HttpDef_ClientAccpet( HttpServer_t     *pSer,
                              HttpClientReq_t  *pClientReq )
{
  return OF_SUCCESS;
} /* HttpDef_ClientAccpet() */

/***********************************************************************
 * Function Name : HttpDef_ClientRead                                  *
 * Description   : This is the default socket-read handler             *
 *                 Useful for SSL stuff. SSL Can Use SSL_recv          *
 * Input         : pReq  - Current Client Request                      *
 *                 pBuf  - Output Buffer To Which Data To Be Read      *
 *                 ulLen - Number of bytes to read                     *
 * Output        : None                                                *
 * Return value  : Number Of Bytes Read On Success                     *
 *                 Negative Value On Failure                           *
 ***********************************************************************/


int32_t HttpDef_ClientRead( HttpClientReq_t  *pClientReq,
                            unsigned char         *pBuf,
                            uint32_t         ulLen,
                            int32_t          *lBytesRead )
{
  /**
   * Validate the passed parameter(s).
   */
  if (!pClientReq)
  {
    return OF_FAILURE;
  }

 *lBytesRead = cm_socket_recv(pClientReq->SockFd, (char *) pBuf, ulLen, 0);  

 if (*lBytesRead <= 0)
 {
     if(HttpdSocketValidateErrorno() != TRUE)  
     {      
        HttpDebug(("HttpDef_ClientRead(): cm_socket_recv() fail :%d\n\r",errno));      
        return OF_FAILURE;
     } 
 }
 else
 {
    return OF_SUCCESS;
 }
return OF_SUCCESS;
} /* HttpDef_ClientRead() */

/***********************************************************************
 * Function Name : HttpDef_ClientWrite                                 *
 * Description   : This is the default socket-write handler            *
 *                 Useful for SSL stuff. SSL Can Use SSL_send          *
 * Input         : pReq  - Current Client Request                      *
 *                 pBuf  - Buffer To Write to Socket                   *
 *                 ulLen - Number of bytes                             *
 * Output        : None                                                *
 * Return value  : Number Of Bytes Written On Success                  *
 *                 Negative Value On Failure                           *
 ***********************************************************************/


int32_t HttpDef_ClientWrite( HttpClientReq_t  *pClientReq,
                             unsigned char         *pBuf,
                             uint32_t         ulLen )
{
int32_t iRetVal=0;
 
  /**
   * Validate the passed parameter(s).
   */
  if ((!pClientReq) ||
      (!pBuf))
  {
    return OF_FAILURE;
  }
  
 iRetVal = cm_socket_send(pClientReq->SockFd, (char *) pBuf, ulLen, 0);  
 if(iRetVal < 0)
 {
      if(HttpdSocketValidateErrorno() != TRUE)  
     {      
        HttpDebug(("HttpDef_ClientWrite(): cm_socket_send() failed :%d\n\r",errno));      
        return OF_FAILURE;
     } 
 }
return OF_SUCCESS;
} /* HttpDef_ClientWrite() */

/***********************************************************************
 * Function Name : HttpDef_ClientClose                                 *
 * Description   : This is the default socket-close handler            *
 * Input         : pReq  - Current Client Request                      *
 * Output        : None                                                *
 * Return value  : OF_SUCCESS on success                                *
 *                 OF_FAILURE on failure                                *
 ***********************************************************************/


int32_t HttpDef_ClientClose( HttpClientReq_t  *pClientReq )
{
  /**
   * Validate the passed parameter(s).
   */
  if (!pClientReq)
  {
    return OF_FAILURE;
  }

  return cm_socket_close(pClientReq->SockFd);
} /* HttpDef_ClientClose() */

/***********************************************************************
 * Function Name : Httpd_InitServers                                   *
 * Description   : This initializes all server contexts                *
 * Input         : pHttpGlobals  - Global pointer                      *
 * Output        : None                                                *
 * Return value  : OF_SUCCESS on success                                *
 *                 OF_FAILURE on failure                                *
 ***********************************************************************/
int32_t Httpd_InitServers(HttpGlobals_t *pHttpGlobals)
{
  uint32_t  ulIndex = 0;
  
  if(!pHttpGlobals)
  {
    Httpd_Error("pHttpGlobals is NULL\r\n");	
    return OF_FAILURE;
  }
  pHttpGlobals->HttpServers = of_calloc(pHttpGlobals->iMaxServers, sizeof(HttpServer_t));
 if(!pHttpGlobals->HttpServers)
 {
    HttpDebug(("Httpservers calloc failed\n\r"));
    return OF_FAILURE;
 }
  /**
   * Set Defaults To All Servers except port numbers.
   */
  for (ulIndex = 0; ulIndex < pHttpGlobals->iMaxServers; ulIndex++)
  {
    pHttpGlobals->HttpServers[ulIndex].Type           = hServerNormal;
    pHttpGlobals->HttpServers[ulIndex].bEnabled       = FALSE;
    pHttpGlobals->HttpServers[ulIndex].ulMaxSess      = pHttpGlobals->HttpdConfig.ulMaxSessions;
    pHttpGlobals->HttpServers[ulIndex].ulCurSess      = 0;   
    of_memset(&(pHttpGlobals->HttpServers[ulIndex].IpAddr),0,sizeof(pHttpGlobals->HttpServers[ulIndex].IpAddr));
    pHttpGlobals->HttpServers[ulIndex].uiPort         = 0;
    pHttpGlobals->HttpServers[ulIndex].SockFd         = -1;
    pHttpGlobals->HttpServers[ulIndex].ClientAccept   = HttpDef_ClientAccpet;
    pHttpGlobals->HttpServers[ulIndex].ClientRead     = HttpDef_ClientRead;
    pHttpGlobals->HttpServers[ulIndex].ClientWrite    = HttpDef_ClientWrite;
    pHttpGlobals->HttpServers[ulIndex].ClientClose    = HttpDef_ClientClose;
    pHttpGlobals->HttpServers[ulIndex].HtmlFilter     = Httpd_FilterHtmlFile;
    pHttpGlobals->HttpServers[ulIndex].HtmlPermission = Httpd_GetPermission;
    pHttpGlobals->HttpServers[ulIndex].pSpData        = NULL;    
  }

  return OF_SUCCESS;
} /* Httpd_InitServers() */
/***********************************************************************
 * Function Name : HttpClient_Alloc                                    *
 * Description   : This allocates a new client context and initializes *
 *                 it with default values                              *
 * Input         : pHttpGlobals  - Global pointer                      *
 * Output        : None                                                *
 * Return value  : Pointer to New Client Context on success            *
 *                 NULL on failure                                     *
 ***********************************************************************/
HttpClientReq_t *HttpClient_Alloc(HttpGlobals_t  *pHttpGlobals)
{
  HttpClientReq_t  *pNewClientReq;
  int32_t          lIndex, i;
  int32_t  ReqTableSize;
  uint32_t   ulAppDataLen=0;
  uint32_t  ulUrlLength=0;

  if(!pHttpGlobals)
  {
    Httpd_Error("pHttpGlobals is NULL\r\n");	
    return NULL;
  }
  lIndex = -1;
  ReqTableSize=Httpd_GetMaxReqTableSize(pHttpGlobals); /*pHttpGlobals->pHttpGlobalConfig->iReqtablesize;*/
  ulAppDataLen=Httpd_GetAppDataLength(pHttpGlobals);/*pHttpGlobals->pHttpGlobalConfig->ulAppDataLength;*/  
  ulUrlLength=Httpd_GetUrlSize(pHttpGlobals);/*pHttpGlobals->pHttpGlobalConfig->ulUrlLength;*/
  /**
   * Find Free Index First.
   */
  for (i = 0; i < ReqTableSize; i++)    /* REQTABLE_SIZE */
  {
    if (!(pHttpGlobals->pHttpReqTable[i]))
    {
      lIndex = i;
      break;
    }
  }

  if (lIndex < 0)
  {
    return NULL;
  }

  pNewClientReq = (HttpClientReq_t *) of_calloc(1, sizeof(HttpClientReq_t));  

  if (!pNewClientReq)
  {
    return NULL;
  }  

  pHttpGlobals->pHttpReqTable[i]= pNewClientReq;
  pNewClientReq->eMethod        = hMethodBad;
  pNewClientReq->eVersion       = hVersionBad;  
  pNewClientReq->lInContentLen  = -1;
  pNewClientReq->lOutContentLen = -1;
  pNewClientReq->bHost=FALSE;  /*PERSISTENCE AND PIPELINING*/
  pNewClientReq->bIsPipeLined=FALSE;  /*PERSISTENCE AND PIPELINING*/
  pNewClientReq->bIsHalfPacketRecv=FALSE;
  pNewClientReq->iReqTableIndex=lIndex;
  pNewClientReq->ulTotalFileSize=0;
  pNewClientReq->bRefererFound      = FALSE;
  pNewClientReq->bISPgUserLoginReq  = TRUE;
  pNewClientReq->ulConnType     =  0;
  pNewClientReq->ucAppData=of_calloc(1,ulAppDataLen+1);
  if(!pNewClientReq->ucAppData)
  {
     of_free(pNewClientReq);
     HttpDebug(("of_calloc failed for pNewClientReq->ucAppData\r\n"));
     return NULL;
  }
  pNewClientReq->ucUrl=of_calloc(1,ulUrlLength+1);
  if(!pNewClientReq->ucUrl)
  {
     of_free(pNewClientReq->ucAppData);  
     of_free(pNewClientReq); 
     HttpDebug(("of_calloc failed for pNewClientReq->ucUrl \r\n"));
     return NULL;
  }
  pNewClientReq->pGlobalData     = (HttpGlobals_t *)pHttpGlobals;
#ifdef INTOTO_CMS_SUPPORT
  pNewClientReq->bDoNotRemoveRequest = FALSE;
#endif  /* INTOTO_CMS_SUPPORT */
  return pNewClientReq;
} /* HttpClient_Alloc() */

/***********************************************************************
 * Function Name : HttpClient_Free                                     *
 * Description   : This frees the client context. This deletes if any  *
 *                 timer attached to it. It frees multi-part buffer if *
 *                 present.                                            *
 * Input         : pReq - Client Request                               *
 *                 pHttpGlobals  - Global pointer                      *
 * Output        : None                                                *
 * Return value  : OF_SUCCESS on success                                *
 *                 OF_FAILURE on failure                                *
 ***********************************************************************/

/*  */
int32_t HttpClient_Free(HttpGlobals_t  *pHttpGlobals,HttpClientReq_t  *pClientReq)
{
  uint32_t uiIndex,iResIndex;
  HttpdSetCookieInfo_t  *pSetTemp = NULL;
  HttpdGetCookieInfo_t  *pGetTemp = NULL;
  HttpTransmitBuffer_t  *pTemp = NULL;
  HttpTransmitBuffer_t  *pNext = NULL;  
  int16_t     			 iPiplineindex=0;
  int16_t     			 iPipelineSubindex=0;
  int16_t    			 iReqTableIndex=0;

  

  /**
   * Validate the passed parameter(s).
   */
   if(!pHttpGlobals)
  {
    Httpd_Error("pHttpGlobals is NULL\r\n");	
    return OF_FAILURE;
  }
  if (!pClientReq)
  {
    return OF_FAILURE;
  }  
  iReqTableIndex=pClientReq->iReqTableIndex;
  iPiplineindex=pClientReq->iPipeLineTableIndex;
  iPipelineSubindex=pClientReq->iPipeLineSubIndex;  
  if(pClientReq->iPipeLineSubIndex <0 )
  {
    if(iPiplineindex>0)
    {
      for(iResIndex= 0;iResIndex < HTTP_MAX_PIPELINED_REQUESTS; iResIndex++)
      {
        HttpClientReq_t       *pClientrequest=NULL;
        pClientrequest = pHttpGlobals->pHttpClientReqTable[iPiplineindex].pHttpPipeline[iResIndex];
        if(pClientrequest == NULL ||(pClientrequest->SockFd <=0))
        {
          continue;
        }
        else
        {
          pClientrequest->bPersistentConn=TRUE;
          HttpFreeClientReqAndTxBuf(pHttpGlobals,pClientrequest);
          pHttpGlobals->pHttpClientReqTable[iPiplineindex].pHttpPipeline[iResIndex]=NULL;
        }
      }
      pHttpGlobals->pHttpClientReqTable[iPiplineindex].iSockFd = -1;
      of_memset(pHttpGlobals->pHttpClientReqTable[iPiplineindex].pHttpPipeline,0,4*HTTP_MAX_PIPELINED_REQUESTS);  
    }
    pHttpGlobals->pHttpReqTable[iReqTableIndex]=NULL;
  }
  else
  {
    pHttpGlobals->pHttpClientReqTable[iPiplineindex].pHttpPipeline[iPipelineSubindex]=NULL;
  }

/***Added by vamsi starts ****/
/*Free the memory allocated by the callback module using
 * the registered Free Routine
 */
for (uiIndex = 0; uiIndex < IGW_HTTPD_MAX_OPAQUE_VARIABLES; uiIndex++)
{
  if( (pClientReq->pHttpReqOpaqueNode[uiIndex] != NULL) && pHttpGlobals->HttpReqOpaqueStatus[uiIndex].pFreeFun )
  {
    /***Get the freeroutine  from the Status array **/
    pHttpGlobals->HttpReqOpaqueStatus[uiIndex].pFreeFun(pClientReq->pHttpReqOpaqueNode[uiIndex]);
  }
}
/***Added by vamsi ends ****/

  /**
   * Also free the SetCookie and GetCooki Linked list and attach to
   * Free pool.
   */
  while (pClientReq->pSetCookieHead != NULL)
  {
    pSetTemp                   = pClientReq->pSetCookieHead;
    pClientReq->pSetCookieHead = pClientReq->pSetCookieHead->pNext;  
    pSetTemp->pNext = NULL;

    if (pSetTemp->bHeep == 1)
    {
     if(pSetTemp->CookieBuff)
     {
        of_free(pSetTemp->CookieBuff);
        pSetTemp->CookieBuff=NULL;
     }
     of_memset(pSetTemp, 0, sizeof(HttpdSetCookieInfo_t));      
     
     pSetTemp->bHeep = 1;

      if (pHttpGlobals->pFreeSetCookieInfo == NULL)
      {
        pHttpGlobals->pFreeSetCookieInfo = pSetTemp;
      }
      else
      {
        pSetTemp->pNext    = pHttpGlobals->pFreeSetCookieInfo;
        pHttpGlobals->pFreeSetCookieInfo = pSetTemp;
      }
    }
    else
    {
          if(pSetTemp->CookieBuff)
          {
               of_free(pSetTemp->CookieBuff);	   
               pSetTemp->CookieBuff=NULL;
          }
          of_memset(pSetTemp, 0, sizeof(HttpdSetCookieInfo_t));
          of_free(pSetTemp);           
    }
  }

  while (pClientReq->pGetCookieHead != NULL)
  {
    pGetTemp                   = pClientReq->pGetCookieHead;
    pClientReq->pGetCookieHead = pClientReq->pGetCookieHead->pNext; 
     pGetTemp->pNext            = NULL;

    if (pGetTemp->bHeep == 1)
    {
          if(pGetTemp->CookieBuff)
          {
               of_free(pGetTemp->CookieBuff);
               pGetTemp->CookieBuff=NULL;
          } 
          of_memset(pGetTemp, 0, sizeof(HttpdGetCookieInfo_t));
          pGetTemp->bHeep = 1;

      if (pHttpGlobals->pFreeGetCookieInfo == NULL)
      {
        pHttpGlobals->pFreeGetCookieInfo = pGetTemp;
      }
      else
      {
        pGetTemp->pNext    = pHttpGlobals->pFreeGetCookieInfo;
        pHttpGlobals->pFreeGetCookieInfo = pGetTemp;
      }
    }
    else
    {      
             if(pGetTemp->CookieBuff)
             {
                 of_free(pGetTemp->CookieBuff);
                 pGetTemp->CookieBuff=NULL;
             }
             of_memset(pGetTemp, 0, sizeof(HttpdGetCookieInfo_t));
             of_free(pGetTemp);      
      }
  }

  if (pClientReq->pSetCookieHead != NULL)
  {
    pClientReq->pSetCookieHead = NULL;
  }

  if (pClientReq->pGetCookieHead != NULL)
  {
    pClientReq->pGetCookieHead = NULL;
  }

  if (pClientReq->pTimer)
  {
    Httpd_Timer_Delete(pClientReq->pTimer);
  }

  if (pClientReq->pMPart)
  {
    of_free(pClientReq->pMPart);
  }

  /*If some how Tx.Buffer is not empty, then free all Tx nodes.*/
  pTemp = pClientReq->pTransmitBuffer;
  while(pTemp != NULL)
  {
    pNext = pTemp->pNext;
    HttpFreeTxBuf(pTemp);
    pTemp = pNext;
  }
  if(pClientReq->ucCookieVal)
  {
      of_free(pClientReq->ucCookieVal);      
  }
  if(pClientReq->pApplData)
  {
     of_free(pClientReq->pApplData);
  }
  if(pClientReq->ucUrl)
  {
      of_free(pClientReq->ucUrl);
  }
  if(pClientReq->ucAppData)
  {
      of_free(pClientReq->ucAppData);
  }
  if(pClientReq->ucRemainBuffer)
  {
    of_free(pClientReq->ucRemainBuffer);
  }
  of_free(pClientReq);
  pClientReq=NULL;
  return OF_SUCCESS;
} /* HttpClient_Free() */

/***********************************************************************
 * Function Name : HttpClient_Delete                                   *
 * Description   : This detaches client context from server and closes *
 *                 its socket, updates statistics and frees it         *
 * Input         : pReq - Client Request                               *
 *                 pHttpGlobals  - Global pointer                      *
 * Output        : None                                                *
 * Return value  : OF_SUCCESS on success                                *
 *                 OF_FAILURE on failure                                *
 ***********************************************************************/
int32_t HttpClient_Delete(HttpGlobals_t  *pHttpGlobals,HttpClientReq_t  *pClientReq)
{

#ifdef HTTP_REDIRECT_HTTPS
  X509      *pCert = NULL;
  EVP_PKEY  *pPrivKey = NULL;
#endif /* OF_HTTPS_SUPPORT */

  /**
   * Validate the passed parameter(s).
   */
   if(!pHttpGlobals)
  {
    Httpd_Error("pHttpGlobals is NULL\r\n");	
    return OF_FAILURE;
  }
  if (!pClientReq)
   {
        HttpDebug("pClient Request is NULL");
        return OF_FAILURE;
   }

#ifdef HTTP_REDIRECT_HTTPS
  if ((pHttpGlobals->HttpServers[pClientReq->lServId].Type == hServerNormal) &&
      (pHttpGlobals->HttpServers[pClientReq->lServId].bEnabled))
  {
    if (HttpsGetValidCert(&pCert, &pPrivKey) == OF_SUCCESS)
    {
      Httpd_Disable(pHttpGlobals,hServerNormal);
      Httpd_Enable(pHttpGlobals,hServerSSL);
    }
    if (!pHttpGlobals->HttpServers[pClientReq->lServId].bEnabled)
    {
      HttpCookie_Del(pClientReq->ucCookieVal);
    }
  }
#endif

 if((pClientReq->bPersistentConn==TRUE)    	    && 
 	            (pClientReq->bHost==TRUE)		    &&
 	            (pClientReq->eVersion==hVersion11 )  &&
 	            (pClientReq->eMethod!=hMethodPost))
  {
      HttpDebug(("We are not closing the connection immediatly for HTTP/1.1 requets\r\n"));
  }
  else
  {
        if(pHttpGlobals->HttpServers[pClientReq->lServId].ClientClose(pClientReq)!=OF_SUCCESS)
        {
            HttpDebug("Client close failed\r\n");
            return OF_FAILURE;
        }    
	
	 if(pClientReq->iPipeLineTableIndex >0 && pHttpGlobals->pHttpClientReqTable[pClientReq->iPipeLineTableIndex].pKeepAliveInfo &&
	 	pHttpGlobals->pHttpClientReqTable[pClientReq->iPipeLineTableIndex].pKeepAliveInfo->pKeepAliveTimer)	 	
	 {
	    Httpd_Timer_Delete(pHttpGlobals->pHttpClientReqTable[pClientReq->iPipeLineTableIndex].pKeepAliveInfo->pKeepAliveTimer);
 	    of_free(pHttpGlobals->pHttpClientReqTable[pClientReq->iPipeLineTableIndex].pKeepAliveInfo);
	    pHttpGlobals->pHttpClientReqTable[pClientReq->iPipeLineTableIndex].pKeepAliveInfo = NULL;	
	 }
  } 
  if(pHttpGlobals->HttpServers[pClientReq->lServId].ulCurSess >0)
  {
     pHttpGlobals->HttpServers[pClientReq->lServId].ulCurSess--;
  }
  pHttpGlobals->HttpServers[pClientReq->lServId].Stats.ulClientFrees++;  
  if(HttpClient_Free(pHttpGlobals,pClientReq)!=OF_SUCCESS)
  {
    HttpDebug("Client free failed\r\n");
    return OF_FAILURE;
  }   
  return OF_SUCCESS;
} /* HttpClient_Delete() */

/***********************************************************************
 * Function Name : HttpClient_ReqStrTimeOut                            *
 * Description   : This timer-handler for timeout of reading complete  *
 *                 request string. This updates the poll table and     *
 *                 deletes the client request                          *
 * Input         : pVal - Client Request                               *
 * Output        : None                                                *
 * Return value  : None                                                *
 ***********************************************************************/

void HttpClient_ReqStrTimeOut( void  *pVal )
{
  HttpClientReq_t  *pClientReq;
  uint32_t         uiIndex;
  int32_t            FdCount;
  HttpGlobals_t  *pHttpg=NULL;
  
  pClientReq = (HttpClientReq_t *) pVal;

  if (!pClientReq)
  {
    return;
  }

 pHttpg=pHTTPG;
 if(!pHttpg)
 {
    Httpd_Error("pHttpg is NULL\r\n");
    return ;
 }
 FdCount= Httpd_GetMaxFds(pHttpg); /*pHttpg->pHttpGlobalConfig->iMaxfds;*/
  /**
   * Reset This Reference If Present In Poll Table.
   */
  for (uiIndex = pHttpg->iMaxServers; uiIndex < FdCount; uiIndex++)    /* HTTP_MAX_FDS */
  {
    if (pHttpg->HttpPollTable[uiIndex].user_data_p == pVal)
    {
      pHttpg->HttpPollTable[uiIndex].returned_events = 0;
      pHttpg->HttpPollTable[uiIndex].user_data_p = NULL;
      break; /* It it is present, there will be only one instance */
    }
  }
  pHttpg->HttpServers[pClientReq->lServId].Stats.ulReqStrTimeOuts++;  
  if(HttpClient_Delete(pHttpg,pClientReq)!=OF_SUCCESS)
   {
        HttpDebug("Client Delete is returned failure");
        return;
   }
  return;
} /* HttpClient_ReqStrTimeOut() */

/***********************************************************************
 * Function Name : HttpClient_SetReadInfo                              *
 * Description   : This sets the current read-info of the client       *
 *                 ulMaxLen bytes are read to pBuf from now on using   *
 *                 polling (parallel)                                *
 * Input         : pClientReq - Client Request                               *
 *                 pBuf - Buffer to which data to be read              *
 *                 ulMaxLen - Maximum number of bytes to read          *
 *                 pCur - initial write pointer                        *
 * Output        : None                                                *
 * Return value  : None                                                *
 ***********************************************************************/


void HttpClient_SetReadInfo( HttpClientReq_t  *pClientReq,
                               unsigned char         *pBuf,
                               uint32_t         ulMaxLen,
                               unsigned char         *pCur )
{
  /**
   * Validate the passed parameter(s).
   */
  if (!pClientReq)
  {
    return;
  }

  pClientReq->ReadInfo.pBuf     = pBuf;
  pClientReq->ReadInfo.ulMaxLen = ulMaxLen;
  pClientReq->ReadInfo.pCur     = pCur;
} /* HttpClient_SetReadInfo() */

/***********************************************************************
 * Function Name : Httpd_AcceptClient                                  *
 * Description   : This function is called when there is incoming      *
 *                 client request on any server. This accepts the      *
 *                 client and allocate new client context and sets     *
 *                 timeout for reading request                         *
 * Input         : ulIndex - Server Index                              *
 * Input         : ip_type_ul - Ipv4 or Ipv6 Index                       *
 * Input           pHttpGlobals  - Global pointer                      *
 * Output        : None                                                *
 * Return value  : OF_SUCCESS on success                                *
 *                 OF_FAILURE on failure                                *
 ***********************************************************************/


int32_t Httpd_AcceptClient(HttpGlobals_t  *pHttpGlobals,uint32_t  ulIndex ,uint32_t ulIptype)
{
  cm_sock_handle    iAccFd=0;
  //struct cm_ip_addr      peerIp={};
  ipaddr           peerIp;
  uint16_t         peerPort=0;
  HttpClientReq_t  *pClientReq=NULL;

   if(!pHttpGlobals)
  {
    Httpd_Error("pHttpGlobals is NULL\r\n");	
    return OF_FAILURE;
  }

  if ((ulIndex < 0) && (ulIndex > pHttpGlobals->iMaxServers))
  {
    HttpDebug(("Inavlid ulIndex\n\r"));
    return OF_FAILURE;
  }

  if (pHttpGlobals->HttpServers[ulIndex].SockFd < 0)
  {
    HttpDebug(("Sockfd < 0\n\r"));
    return OF_FAILURE;
  }
#ifndef OF_CM_SUPPORT
#ifdef OF_IPV6_SUPPORT
 if(ulIptype == UCM_INET_IPV6)
 {
     peerIp.ip_type_ul=UCM_INET_IPV6;
 }
 else
#endif /*INTOTO_IPV6_SUPPORT*/
 {
     peerIp.ip_type_ul=cm_inet_ipv4;
 }
#endif /*OF_CM_SUPPORT*/

 iAccFd = cm_socket_accept(pHttpGlobals->HttpServers[ulIndex].SockFd, &peerIp,
                           &peerPort);

 if (iAccFd < 0)
  {
     if(HttpdSocketValidateErrorno() == TRUE)  /* resources may be exhausted */    
     {
       HttpDebug(("Buffers not available & errno :%d\n\r",errno));
       of_sleep_ms(10); 
       return OF_FAILURE;
     }
     else /*HttpdSocketValidateErrorno() == FALSE*/
     {
       HttpDebug(("cm_socket_accept failed\n\r"));
       return OF_FAILURE;
     }
  }

#ifndef HTTP_REDIRECT_HTTPS
  if(pHttpGlobals->HttpServers[ulIndex].bEnabled != TRUE)
  {
    cm_socket_close(iAccFd);
    return OF_SUCCESS;
  }
#endif

  pHttpGlobals->HttpServers[ulIndex].Stats.ulAccepts++;

  if (pHttpGlobals->HttpServers[ulIndex].ulCurSess >= pHttpGlobals->HttpServers[ulIndex].ulMaxSess)
  {
    Httpd_Debug("Reached Maximum. Rejecting...");
    pHttpGlobals->HttpServers[ulIndex].Stats.ulRejects++;
    cm_socket_close(iAccFd);
    return OF_FAILURE;
  }

  if ((!pHttpGlobals->HttpServers[ulIndex].bEnabled) &&
      (pHttpGlobals->HttpServers[ulIndex].Type != hServerNormal))
  {
    pHttpGlobals->HttpServers[ulIndex].Stats.ulRejects++;
    cm_socket_close(iAccFd);
    return OF_FAILURE;
  }

  if (cm_socket_make_non_blocking(iAccFd) < 0)
  {
    cm_socket_close(iAccFd);
    return OF_FAILURE;
  }

  pClientReq = HttpClient_Alloc(pHttpGlobals);  
  if (!pClientReq)
  {
#ifdef HTTPD_DEBUG
    Httpd_Debug("Unable To Allocate Client Request");
#endif
    pHttpGlobals->HttpServers[ulIndex].Stats.ulClientAllocFails++;
    cm_socket_close(iAccFd);
    return OF_FAILURE;
  }

  pClientReq->ucCookieVal=of_calloc(1,Httpd_GetMaxCookieValueLen(pHttpGlobals)+1);
  if (!pClientReq->ucCookieVal)
  {
#ifdef HTTPD_DEBUG
    Httpd_Debug("Unable To Allocate Client Request");
#endif
    pHttpGlobals->HttpServers[ulIndex].Stats.ulClientAllocFails++;
    cm_socket_close(iAccFd);
    return OF_FAILURE;
  }
  pClientReq->pTimer = NULL;

  if (Httpd_Timer_Create(&pClientReq->pTimer) !=OF_SUCCESS)
  {
#ifdef HTTPD_DEBUG
    Httpd_Debug("Unable To Timer For ReqStr");
#endif
    cm_socket_close(iAccFd);
    HttpClient_Free(pHttpGlobals,pClientReq);
    return OF_FAILURE;
  }
  

  pClientReq->lServId  = ulIndex;
  pClientReq->PeerIp   = peerIp; 
  pClientReq->PeerPort = peerPort;
  pClientReq->SockFd   = iAccFd;
  pClientReq->eState   = hStateReadReq;
  pClientReq->SrcPort  = pHttpGlobals->HttpServers[ulIndex].uiPort;
  pClientReq->bSendResponce=FALSE;
#ifdef INTOTO_CMS_SUPPORT
/* Don't set index -1, to accept cms https connections 
 *  pClientReq->iPipeLineTableIndex = -1;
 */
#else /* INTOTO_CMS_SUPPORT */
  pClientReq->iPipeLineTableIndex=-1; 
#endif /* INTOTO_CMS_SUPPORT */
  pClientReq->iPipeLineSubIndex=-1; 
  HttpClient_SetReadInfo(pClientReq, pClientReq->ucReqStr, HTTP_REQSTR_LEN,
                         pClientReq->ucReqStr);

  of_strcpy((char *) pClientReq->ucInContentType, HTTPD_DEF_CONT_TYPE);

  if (pHttpGlobals->HttpServers[ulIndex].ClientAccept)
  {
    if (pHttpGlobals->HttpServers[ulIndex].ClientAccept(&pHttpGlobals->HttpServers[ulIndex],
                                          pClientReq) != OF_SUCCESS)
    {
      Httpd_Debug("ClientAccept - Returned Failure");
      cm_socket_close(iAccFd);
      HttpClient_Free(pHttpGlobals,pClientReq);
      return OF_FAILURE;
    }
  }

  pHttpGlobals->HttpServers[ulIndex].Stats.ulClientAllocs++;

  Httpd_Timer_Start(pClientReq->pTimer,
                    (HTTPD_TIMER_CBFUN)(pHttpGlobals->pCbkTimers->HttpdReqStrTimeoutCbk),
                    (void*) pClientReq, HTTPD_TIMER_SEC,
                    HTTPD_REQSTR_TIMEOUT, 0);

  pHttpGlobals->HttpServers[ulIndex].ulCurSess++; 
  pClientReq->bIsAcceptFd=TRUE;
  pClientReq->ucRemainBuffer = NULL;
  pClientReq->pSendHeader = NULL;
  pClientReq->bNeedMoreData = FALSE;
  return OF_SUCCESS;
} /* Httpd_AcceptClient() */

/***********************************************************************
 * Function Name : HttpClient_SendFile                                 *
 * Description   : This function is used to send a file to the client  *
 *                 filename is present in pClientReq as field 'filename*
 * Input         : pClientReq - Client Request                         *
 *                 pHttpGlobals  - Global pointer                      *
 * Output        : None                                                *
 * Return value  : OF_SUCCESS on success                                *
 *                 OF_FAILURE on failure                                *
 ***********************************************************************/


int32_t HttpClient_SendFile(HttpGlobals_t *pHttpGlobals, HttpClientReq_t  *pClientReq )
{
  HttpRequest   DummyReq;
#ifndef INTOTO_CMS_SUPPORT
  int32_t  lCount = 0; 
  unsigned char         ucExt[HTTP_FILE_EXTENTION_MAX_LEN +1];
#endif /* INTOTO_CMS_SUPPORT*/

  if(!pHttpGlobals)
  {
    Httpd_Error("pHttpGlobals is NULL\r\n");	
    return OF_FAILURE;
  }
   
  HttpClient_SetDummyReq(pClientReq, &DummyReq);
#ifndef INTOTO_CMS_SUPPORT
  if(!bIsLogoFile)
  {
     if((!of_strcmp((char *) pClientReq->ucFileName,"logoicon.gif")))
     {
        ulLogoFileReq_g ++;
	of_memcpy(pHttpGlobals->HttpFileBuf.HttpUnzipBuf,LogoBuff_g,sizeof(LogoBuff_g));
	HttpMime_Ext2ContentType(pHttpGlobals,(char *) ucExt, (char *) pClientReq->ucOutContentType);
	if (HttpClient_Send(pClientReq, (char *) pHttpGlobals->HttpFileBuf.HttpUnzipBuf,
	     		   sizeof(LogoBuff_g)) != OF_SUCCESS)
	{
	        Httpd_Error("HttpClient_Send Failed!");
	        HttpCookie_ResetInacTimeout(pHttpGlobals,pClientReq->ucCookieVal);
	        return OF_FAILURE;
	}
	pHttpGlobals->HttpFileBuf.ulFileSize = 0;
	HttpCookie_ResetInacTimeout(pHttpGlobals,pClientReq->ucCookieVal);
        return OF_SUCCESS; 
    }
    else
    { 
     ulNonLogoFileReq_g ++;
    }
  }
 if(!bIsLogoFile)
 {
  lCount = ulNonLogoFileReq_g/ulLogoFileReq_g;
  if(lCount > 20)
  {
    if(of_strstr((char *) pClientReq->ucFileName, "htm"))
    {
     of_strcpy((char *) DummyReq.pAppData,"Logo is Not Used and Please restore the Original HTML files");
      of_strcpy((char *) DummyReq.filename, "errscrn.htm");
      ulNonLogoFileReq_g = 1;
      ulLogoFileReq_g = 1;
    }
  }
 }
#endif /*INTOTO_CMS_SUPPORT */
 return HttpRequest_SendFile(pHttpGlobals,&DummyReq);
} /* HttpClient_SendFile() */
/************************************************************************
    * Function Name : LaunchLogViewer                                      *
    * Description   : Posts The JNLP File to the PC                        *
    * Input         : HTTP request
    * Output        : JNLP file to send to the browser with the source ip  *
    *                 IPaddress properly filled                            *
    * Return value  : OF_SUCCESS on setting all data else OF_FAILURE.        *
    ************************************************************************/
int32_t LaunchLogViewer(HttpGlobals_t *pHttpGlobals,HttpRequest *pReq)
{
  int32_t fd = -1,ret = 0 , iLen=0;
  char *buffer = NULL , *temp= NULL , new_addr[20]={};
  char *buffer2 =  NULL;
  HttpClientReq_t *pClientReq= NULL;
    	
  if(pReq == NULL)
   return OF_FAILURE;
          
  fd  = open(JNLP_FILE,O_RDONLY);

  if(!fd)
  {
    if (HttpSendHtmlFile(pReq, (unsigned char *)("errscrn.htm"),
            (unsigned char *) ("Failed to Read the JNLP File")) == OF_FAILURE)
    {
      return OF_FAILURE;
    }
  }
		
  buffer = (char *)calloc(1,JNLP_SIZE);
  if(!buffer)
  {
    return OF_FAILURE;
  }
  buffer2= (char *)calloc(1,JNLP_SIZE);
  if(!buffer2)
  {
    of_free(buffer);
    return OF_FAILURE;
  }
  ret = read(fd,buffer,JNLP_SIZE);
  if(ret <= 0)
  {
    of_free(buffer);
    of_free(buffer2);
    return OF_FAILURE;
  }
  buffer[ret] = '\0';
   
 
  temp = strstr(buffer,"://255") ;
 /**In JNLP File we make a template for IP Address as 255.255.255.255 which will be replaced       *by the actual reqd IP
  **/
  if(!temp)
  {
    if(buffer)
      of_free(buffer);
    if(buffer2)
      of_free(buffer2);
   return OF_FAILURE;
  }
  pClientReq =(HttpClientReq_t *)pReq->pPvtData;
  of_strncpy(new_addr,(char *)pClientReq->ucArgs,sizeof(new_addr));

  of_strncpy(buffer2,buffer,(of_strlen(buffer) - of_strlen(temp)));
  
  if (pHttpGlobals->HttpServers[pClientReq->lServId].Type != hServerNormal) //For Https transfer
  {
	//Add  http's' to the buffer.
	of_strcat(buffer2,"s://");
	iLen=4;
  }
  else
  {
	of_strcat(buffer2,"://");
	iLen=3;
  }

  of_strncat(buffer2,new_addr,of_strlen(new_addr));
  //of_strncat(buffer2,(temp + of_strlen(addr)),of_strlen((temp + of_strlen(addr))));
  of_strncat(buffer2,(temp + MASK_IP_SIZE + iLen),of_strlen((temp + MASK_IP_SIZE)));
  printf("buffer2 from %s = %s \n",__FUNCTION__,buffer2); 

  pReq->inh_content_type=(unsigned char *) ("application/octet-stream");
  if (Httpd_Send_PlainBuf(pReq, buffer2, of_strlen(buffer2)) != OF_SUCCESS)
  {
    if (HttpSendHtmlFile(pReq, (unsigned char *) "errscrn.htm",
               (unsigned char *)  "Failed to Send the JNLP File") == OF_FAILURE)
    {
      close(fd);
      if(buffer)
      {
        of_free(buffer);
      }
      if(buffer2)
      {
        of_free(buffer2);
      }
      return OF_FAILURE;
    }
  }
 
  close(fd);
  if(buffer)
  {
    of_free(buffer);
  }
  if(buffer2)
  {
    of_free(buffer2);
  }
  return OF_SUCCESS;
} /* LaunchLogViewer */
   
   
/**************************************************************************
    * Function Name : PostJarfile                                            *
    * Description   : Posts The Jarfile to the PC                            *
    * Input         : HTTP request                                           *
    * Output        : HTML file to send to the browser with or without       *
    *                 information based on the operation performed.          *
    * Return value  : OF_SUCCESS on setting all data else OF_FAILURE.          *
**************************************************************************/
   
int32_t PostJarfile( HttpRequest  *pReq )
{
     int32_t   index  = 0;
     int32_t   lFd = -1;
     unsigned char  TempBuf[MBB_TEMPORARY_MSG_LENGTH];
     unsigned char  pAta[MBB_ATA_DIRECTORY_LENGTH];
      /**
      * Initialize the local variables.
      */
     of_memset(TempBuf, 0, MBB_TEMPORARY_MSG_LENGTH);
     of_memset(pAta, 0, MBB_ATA_DIRECTORY_LENGTH);
   
     //of_strcpy(pAta, "/ondirector/html/");
     of_strcpy((char *) pAta, HTTPD_DOC_PATH);
     of_strcat((char *) pAta,(char *) pReq->filename);
     lFd =  open((char *) pAta, O_RDONLY, T_S_IRWXU);
   
     if (lFd < 0)
     {
       perror("Error: ");
       Httpd_Print("PrjFileRead: Open Failed!!! for File %s\n", pAta);
   
       HttpSendHtmlFile(pReq, (unsigned char *) "errscrn.htm",(unsigned char *) "Error in Reading Data");
       return OF_SUCCESS;
     }
    /**
      * Point the read/write pointer to the start of the file.
      */
     lseek(lFd, 0, 0);
     index=0;
     while(1)
     {
           index=read(lFd,TempBuf,(MBB_TEMPORARY_MSG_LENGTH));
           if(index  > 0)
           {
                   if(Httpd_Send_PlainBuf(pReq,(char *) TempBuf,index)== OF_FAILURE)
                   {
                           if (HttpSendHtmlFile(pReq, (unsigned char *) "errscrn.htm",
                                   (unsigned char *)  "Error in Sending the Data") == OF_FAILURE)
                           {
                                  close(lFd);
                                  lFd = -1; 
                                   /*of_free(TempBuf);*/
                                   return OF_FAILURE;
                           }
                   }
           }
           else if(index==0)
	   {
                   close(lFd);
                   lFd = -1; 
                   break;
           }
           else
           {
                   perror("Error :");
           }
   
     }
#if 0  //CID 61
     if(lFd > 0)
     {
       close(lFd);
     }
#endif
     return OF_SUCCESS;
   
} /* PostJarfile */

/***********************************************************************
 * Function Name : HttpRequest_SendFile                                *
 * Description   : This function is used to send a file to the client  *
 *                 filename is present in pReq as field 'filename'     *
 * Input         : pReq - Client Request                               *
 * Output        : None                                                *
 * Return value  : OF_SUCCESS on success                                *
 *                 OF_FAILURE on failure                                *
 ***********************************************************************/

int32_t HttpRequest_SendFile(HttpGlobals_t *pHttpGlobals,HttpRequest  *pHttpRequest )
{
  HttpClientReq_t  *pClientReq = NULL;
  unsigned char         ucExt[HTTP_FILE_EXTENTION_MAX_LEN +1];
  int32_t          iRetVal; 

#ifdef HTTP_REDIRECT_HTTPS
  X509      *pCert    = NULL;
  EVP_PKEY  *pPrivKey = NULL;
#endif

  /**
   * Validate the passed parameter(s).
   */
  if (!pHttpRequest)
  {
    return OF_FAILURE;
  }

  /**
   * Get the client request from the passed http request parameter and
   * validate it.
   */
  pClientReq = (HttpClientReq_t *) pHttpRequest->pPvtData;

  if (!pClientReq)
  {
    return OF_FAILURE;
  }

#ifdef HTTP_REDIRECT_HTTPS
  if (pHttpGlobals->HttpServers[pClientReq->lServId].Type == hServerNormal)
  {
    Httpd_MapHtmlFile(pClientReq);
  }

  if ((pHttpGlobals->HttpServers[pClientReq->lServId].Type == hServerSSL) &&
      (pHttpGlobals->HttpServers[pClientReq->lServId].bEnabled))
  {
    if (HttpsGetValidCert(&pCert, &pPrivKey) != OF_SUCCESS)
    {
      Httpd_Enable(pHttpGlobals,hServerNormal);
      Httpd_Disable(pHttpGlobals,hServerSSL);
    }
    if (!pHttpGlobals->HttpServers[pClientReq->lServId].bEnabled)
    {
      HttpCookie_Del(pClientReq->ucCookieVal);
      return OF_FAILURE;
    }
  }
#endif /*HTTP_REDIRECT_HTTPS*/

  if (Httpd_GetFileExt(pHttpRequest->filename, ucExt, HTTP_FILE_EXTENTION_MAX_LEN) != OF_SUCCESS)
  {
    of_strcpy((char *) ucExt, "htm");
  }

  if ( (!of_strcmp((char *) ucExt, "bin") ) ||
       (!of_strcmp((char *) ucExt, "cfg") ) ||
       (!of_strcmp((char *) ucExt, "tgz") ) ||
       (!of_strcmp((char *) ucExt, "dat") )
     )
  {
    return Httpd_SendBinFile(pHttpRequest);
  }

  
 if( of_strcmp((char *) ucExt,"jar") == 0)
     {
           if(pHttpRequest!=NULL)
           {
                   pClientReq = (HttpClientReq_t *) pHttpRequest->pPvtData;
                   if(pClientReq!=NULL)
                   {
                           if(pClientReq->pTimer!=NULL)
                           {
                                           Httpd_Timer_Stop(pClientReq->pTimer);
                           }
                           return PostJarfile(pHttpRequest);
                   }
           }
     }
     else if (of_strcmp((char *) ucExt,"jnlp") == 0)
     {
           return LaunchLogViewer(pHttpGlobals,pHttpRequest);
     }
  
  iRetVal= Httpd_ReadFile(pHttpGlobals,pHttpRequest->filename);
#ifdef INTOTO_CMS_SUPPORT
  if ( (iRetVal != OF_SUCCESS) && (iRetVal == HTTPD_MORESIZE_FILE))
  {    
     pClientReq->bPersistentConn=FALSE;
     if(HttpSendHtmlFile(pHttpRequest, (unsigned char *) "errscrn.htm",
        (unsigned char *)  "file size is ( > 512*1024 ) more")==OF_FAILURE)
     {
       HttpDebug("HttpSendHtmlFile Failed\n");
       return OF_FAILURE;
     }
     return OF_SUCCESS;
  }
  else if(iRetVal == OF_FAILURE) 
  { 
    pClientReq->bPersistentConn=FALSE;   
    return OF_FAILURE;
  }
#else
  if(iRetVal != OF_SUCCESS) 
  { 
     pHttpGlobals->HttpFileBuf.ulFileSize = 0;   
     return OF_FAILURE;
  }
#endif  /* INTOTO_CMS_SUPPORT */

  HttpMime_Ext2ContentType(pHttpGlobals,(char *) ucExt, (char *) pClientReq->ucOutContentType);

  if (!of_strcmp((char *) pClientReq->ucOutContentType, "text/html")||
            !of_strcmp((char *) pClientReq->ucOutContentType, "text/xml") ) /* test */
  {
    if (Httpd_Send_HyperStringEx(pHttpRequest,
                                 (char *) pHttpGlobals->HttpFileBuf.HttpUnzipBuf, 0) != OF_SUCCESS)
    {
      pHttpGlobals->HttpFileBuf.ulFileSize = 0;
      Httpd_Error("HttpClient_ServeFile :: Unable To Send HtmlFile");
      HttpCookie_ResetInacTimeout(pHttpGlobals,pClientReq->ucCookieVal);
      return OF_FAILURE;
    }
  }
  else
  {
    if (HttpClient_Send(pClientReq, (char *) pHttpGlobals->HttpFileBuf.HttpUnzipBuf,
                        pHttpGlobals->HttpFileBuf.ulFileSize) != OF_SUCCESS)
    {
      Httpd_Error("HttpClient_Send Failed!");
      HttpCookie_ResetInacTimeout(pHttpGlobals,pClientReq->ucCookieVal);
      return OF_FAILURE;
    }
  }
  pHttpGlobals->HttpFileBuf.ulFileSize = 0;
  HttpCookie_ResetInacTimeout(pHttpGlobals,pClientReq->ucCookieVal);
  return OF_SUCCESS;
} /* HttpRequest_SendFile() */

/***********************************************************************
 * Function Name : Httpd_GetFileExt                                    *
 * Description   : This function determines the extension of a file    *
 * Input         : pFileName - Name of the file                        *
 *                 ulMaxExt  - Max. Length of (output) extension.      *
 * Output        : pExt - Extension of the file                        *
 * Return value  : OF_SUCCESS on success                                *
 *                 OF_FAILURE on failure                                *
 ***********************************************************************/

/*  */
int32_t Httpd_GetFileExt( unsigned char  *pFileName,
                          unsigned char  *pExt,
                          uint32_t  ulMaxExt )
{
  unsigned char  *p;

  if ((!pFileName) ||
      (!pExt))
  {
    return OF_FAILURE;
  }

  p = (unsigned char *) of_strchr((char *) pFileName, '.');

  if (!p)
  {
    return OF_FAILURE;
  }

  of_strncpy((char *) pExt, (char *) p+1, ulMaxExt);
  pExt[ulMaxExt] = '\0';

  return OF_SUCCESS;
} /* Httpd_GetFileExt() */

/***********************************************************************
 * Function Name : HttpClient_SetHdrFields                             *
 * Description   : This function is called before sending header to    *
 *                 the client. This sets all header related fields.    *
 * Input         : pClientReq - Client Request                         *
 *                 pHttpGlobals  - Global pointer                      * 
 * Output        : None                                                *
 * Return value  : OF_SUCCESS on success                                *
 *                 OF_FAILURE on failure                                *
 ***********************************************************************/


int32_t HttpClient_SetHdrFields(HttpGlobals_t  *pHttpGlobals,HttpClientReq_t  *pClientReq )
{
  unsigned char  ucExt[HTTP_FILE_EXTENTION_MAX_LEN+1] = {};
  char   RandomString[HTTP_RANDOM_STRING_LEN+1]= {}, CookieValue[HTTP_RANDOM_STRING_LEN+1]= {};
  char   Challenge[HTTP_CHALLENGE_LEN + 1]= {};
  char   String[HTTP_RNDNUM_LEN+1]={};
  char   cPeerIp[HTTP_LOCAL_IP_ADDRESS_LEN+1]={};  
#ifdef INTOTO_UAM_SUPPORT
  ipaddr SrcIp;
  IGWUAMRedirAPIInfo_t RedirInfo = {};
  //char UAMFileName[HTTP_FILENAME_LEN + 1];

  of_memset(&RedirInfo,0,sizeof(IGWUAMRedirAPIInfo_t));
#endif

   if(!pHttpGlobals)
  {
    Httpd_Error("pHttpGlobals is NULL\r\n");	
    return OF_FAILURE;
  }
  /**
   * Validate the passed parameter(s).
   */
  if (!pClientReq)
  {
    return OF_FAILURE;
  }

  if (Httpd_GetFileExt(pClientReq->ucFileName, ucExt, HTTP_FILE_EXTENTION_MAX_LEN) != OF_SUCCESS)
  {
    of_strcpy((char *) ucExt, "htm");
  }

 HttpMime_Ext2ContentType(pHttpGlobals,(char *) ucExt, (char *) pClientReq->ucOutContentType);

  /**
   * Added a new condition for javascript files.
   */
  if (!of_strcmp((char *) pClientReq->ucOutContentType, "image/gif")||
    (!of_strcmp((char *) pClientReq->ucOutContentType, "application/x-java-jnlp-file")) ||
    (!of_strcmp((char *) pClientReq->ucOutContentType, "text/plain")))
  {
       if( ( !bIsLogoFile && !of_strcmp((char *) pClientReq->ucFileName,"logoicon.gif")))
       {
         pClientReq->bExpireByOneDay = 0;
       }
       else
       pClientReq->bExpireByOneDay = 1;
  }
  else if (!of_strcmp((char *) pClientReq->ucOutContentType,
                    "application/x-javascript"))
  {
        pClientReq->bExpireByOneDay = 1;
  }
  else if (!of_strcmp((char *) pClientReq->ucOutContentType,
                    "text/css"))
  {
        pClientReq->bExpireByOneDay = 1;
  }
  else
  {
    pClientReq->bExpireByOneDay = 0;
  }

  if ((pClientReq->ulPageType == LOGINPAGEREQUEST ) && 
  	( !HttpCookie_Find(pClientReq->ucCookieVal,NULL)))
  {
    Httpd_GetTimeStr(String);
#ifndef OF_CM_SUPPORT
    #ifdef OF_IPV6_SUPPORT
    if(pClientReq->PeerIp.ip_type_ul == UCM_INET_IPV6)
    {
 	 if(HttpdNumToPresentation(&(pClientReq->PeerIp),(unsigned char *)cPeerIp)!=OF_SUCCESS)
 	 {
   	     HttpDebug((" HttpdNumToPresentation is Failed\r\n"));
  	     return OF_FAILURE;
 	 }
    }
    else
    #endif /*INTOTO_IPV6_SUPPORT*/
#endif /*OF_CM_SUPPORT*/
    {    
		Httpd_ntoa(htonl(pClientReq->PeerIp), cPeerIp);
    }
   sprintf(RandomString, "%s+%s", cPeerIp, String);	

    Httpd_MD5Encrypt((unsigned char *) RandomString, (unsigned char *)CookieValue);

    Httpd_MD5Encrypt((unsigned char *) String, (unsigned char *) Challenge);    
   /**
    * Added for sslvpn where a raw password compare is done based 
    * on a zero value in the first byte of the Challenge string 
    **/
    if((pClientReq->ulConnType) && (Challenge[0] == 0))
    {
      Challenge[0] = 0x55;
    }	    
    of_strcpy((char *) pClientReq->ucCookieVal, CookieValue);
    if(!pClientReq->ulConnType)
    {      
       if (HttpCookie_Add(pHttpGlobals,pClientReq->ucCookieVal,Http_GetCookieName(pHttpGlobals),(unsigned char *) cPeerIp ,(unsigned char *)Challenge,
 	     (unsigned char *) "/", pClientReq->eVersion,Httpd_GetMinSecs(pHttpGlobals)) != OF_SUCCESS)
       {
         return OF_FAILURE;
       }
    }
#ifdef INTOTO_UAM_SUPPORT
    if ((pClientReq->ulPageType == LOGINPAGEREQUEST) &&
         (!of_strcmp(pClientReq->ucFileName, UAM_GetDefLoginFile())))
    {
#ifndef OF_CM_SUPPORT
#ifdef OF_IPV6_SUPPORT
      if( pClientReq->PeerIp.ip_type_ul == IGW_INET_IPV6 &&
        IGWIsIPv4MappedV6Addr(&pClientReq->PeerIp.IP.IPv6Addr) )
      {
        SrcIp = ntohl(pClientReq->PeerIp.IPv6Addr32[3]);
      }
      else
#endif  /*INTOTO_IPV6_SUPPORT*/
#endif  /*OF_CM_SUPPORT*/
      {
        SrcIp = pClientReq->PeerIp;
      }

      RedirInfo.SrcIp = SrcIp;
      of_strncpy(RedirInfo.cCookie, pClientReq->ucCookieVal, 50);
      IGWUAMAppendRedirInfoAPI(&RedirInfo);
    }
#endif
  }

  return OF_SUCCESS;
} /* HttpClient_SetHdrFields() */

/***********************************************************************
 * Function Name : HttpClient_SetDummyReq                              *
 * Description   : This function sets the fields of dummy request which*
 *                 is used for API.                                    *
 * Input         : pClientReq - Pointer To HttpClientReq_t             *
 *                 pDummy - Pointer To HttpRequest. Used only for API  *
 * Output        : None                                                *
 * Return value  : None                                                *
 ***********************************************************************/


void HttpClient_SetDummyReq( HttpClientReq_t  *pClientReq,
                               HttpRequest      *pDummy )
{
  /**
   * Validate the passed parameter(s).
   */
  if ((!pClientReq) ||
      (!pDummy))
  {
    return;
  }

  pDummy->PeerIp           = pClientReq->PeerIp; 
  pDummy->PeerPort         = pClientReq->PeerPort;
  pDummy->filename         = pClientReq->ucFileName;
  pDummy->pAppData         = pClientReq->ucAppData;
  pDummy->inh_content_type = pClientReq->ucInContentType;
  pDummy->cPostString      = pClientReq->ucPostString;
  pDummy->pCookieVal       = pClientReq->ucCookieVal;
  pDummy->pUserName        = pClientReq->ucUserName;
  pDummy->pPvtData         = pClientReq;
  pDummy->SrcPort          = pClientReq->SrcPort;
  pDummy->pApplData= pClientReq->pApplData;  /*Specific to application GET Params */
  pDummy->pUserAgent= pClientReq->pUserAgent; 
  pDummy->pGlobalData    =pClientReq->pGlobalData;
} /* HttpClient_SetDummyReq() */

/***********************************************************************
 * Function Name : HttpClient_EvaluateAccess                           *
 * Description   : This determines the permission to access particular *
 *                 file to the user                                    *
 * Input         : pClientReq - Client Request                         *
 *                 pHttpGlobals  - Global pointer                      * 
 * Output        : None                                                *
 * Return value  : OF_SUCCESS on success                                *
 *                 OF_FAILURE on failure                                *
 ***********************************************************************/


int32_t HttpClient_EvaluateAccess(HttpGlobals_t  *pHttpGlobals,HttpClientReq_t  *pClientReq )
{
  int32_t      lPriv     = HTTPD_PRIV_ADMIN;
  bool      bLoggedIn = FALSE;
  HttpRequest  DummyReq;
#ifdef INTOTO_UAM_SUPPORT
  //char UAMFileName[HTTP_FILENAME_LEN];
#endif

  /**
   * Validate the passed parameter(s).
   */
  if(!pHttpGlobals)
  {
    Httpd_Error("pHttpGlobals is NULL\r\n");	
    return OF_FAILURE;
  }
  
  if (!pClientReq)
  {
    return OF_FAILURE;
  }

  if (pHttpGlobals->HttpServers[pClientReq->lServId].HtmlPermission)
  {
    if (pHttpGlobals->HttpServers[pClientReq->lServId].HtmlPermission((char *) pClientReq->ucFileName,
                                                        &lPriv) != OF_SUCCESS)
    {
      return OF_FAILURE;
    }
  }

  if ((pClientReq->ulPageType != LOGINPAGEREQUEST) &&
      (pClientReq->ulPageType != ERRORPAGEREQUEST) &&
#ifdef INTOTO_UAM_SUPPORT
      (of_strcmp(pClientReq->ucFileName,UAM_DEF_COMPANY_LOGO_FILE)) &&
      (of_strcmp(pClientReq->ucFileName,UAM_GetDefWifiIntroPage())) &&
      (of_strcmp(pClientReq->ucFileName,UAM_GetWifiProvidersPage())) &&
      (of_strcmp(pClientReq->ucFileName,UAM_GetFreeSurfZonePage())) &&
      (of_strcmp(pClientReq->ucFileName,UAM_GetDefUsrNamePage())) &&
      (of_strcmp(pClientReq->ucFileName,UAM_GetDefLoginFile())) &&
      (of_strcmp(pClientReq->ucFileName,UAM_GetDefErrorFile())) &&
      (of_strcmp(pClientReq->ucFileName,UAM_GetDefLoginRedirFile())) &&
#endif /*INTOTO_UAM_SUPPORT*/
      (strcasecmp((char *) pClientReq->ucFileName, (char *) Httpd_GetLoginFile(pHttpGlobals))) &&             /* HTTP_DEF_LOGIN_FILE */
      (strcasecmp((char *) pClientReq->ucFileName, (char *) Httpd_GetLogoFile(pHttpGlobals)))                 /* HTTP_DEF_LOGO_FILE */
#ifdef IGW_MERCED_SUPPORT
      && (strcasecmp((char *) pClientReq->ucFileName, HTTP_MERCED_LOGO_FILE))
      && (strcasecmp((char *) pClientReq->ucFileName, HTTP_MERCED_IMG1_FILE))
      && (strcasecmp((char *) pClientReq->ucFileName, HTTP_MERCED_IMG2_FILE))
      && (strcasecmp((char *) pClientReq->ucFileName, HTTP_MERCED_STYLE_FILE))
#endif /* IGW_MERCED_SUPPORT */

#ifdef LICENSING
   && (strcasecmp((char *) pClientReq->ucFileName, HTTP_DEF_REG_GIFFILE))
#endif


    )
  {
    if (HttpCookie_GetByVal(pClientReq->ucCookieVal, &bLoggedIn, NULL,
                            NULL) != OF_SUCCESS)
    {
      return OF_FAILURE;
    }

    if (!bLoggedIn)
    {
      return OF_FAILURE;
    }
  }

  if (lPriv == HTTPD_PRIV_USER 
#ifdef INTOTO_UAM_SUPPORT
  || (!of_strcmp(pClientReq->ucFileName,UAM_GetDefLoginFile()))
  || (!of_strcmp(pClientReq->ucFileName,UAM_GetDefLoginRedirFile()))
#endif
     )
  {
    return OF_SUCCESS;
  }

  if (HttpCookie_GetByVal(pClientReq->ucCookieVal, &bLoggedIn, NULL,
                          NULL) != OF_SUCCESS)
  {
    return OF_FAILURE;
  }

  if (lPriv == HTTPD_PRIV_ADMIN)
  {
    HttpClient_SetDummyReq(pClientReq, &DummyReq);

    if ((pClientReq->ulPageType != LOGINPAGEREQUEST)&&
        (HttpUserIsConfigType((char *) pClientReq->ucUserName) != TRUE))
    {
      HttpNotifyRandomAccess(pClientReq->PeerIp, (char *) pClientReq->ucUserName,
                             "is randomly accessing page",
                             (char *) pClientReq->ucFileName);

      /**
       * Send the loginout.htm page with the relevant information.
       */
      if (HttpSendHtmlFile(&DummyReq, Httpd_GetLogInOutFile(pHttpGlobals),
                          (unsigned char *)  "Access Denied") == OF_FAILURE)
      {
        ; /* NULL statment */
      }
      return OF_FAILURE;
    }
  }
  return OF_SUCCESS;
} /* HttpClient_EvaluateAccess() */

/*  */
int32_t HttpClient_UploadBinFile(HttpGlobals_t  *pHttpGlobals,HttpClientReq_t  *pClientReq )
{
  HttpRequest  DummyReq;
  int32_t      lPriv = HTTPD_PRIV_ADMIN;
  uint32_t     ulLen;
  uint32_t     uiBytesWritten = 0;

  /**
   * Validate the passed parameter(s).
   */
   if(!pHttpGlobals)
  {
    Httpd_Error("pHttpGlobals is NULL\r\n");	
    return OF_FAILURE;
  }
   
  if ((!pClientReq) ||
      (!pClientReq->pMPart))
  {
    return OF_FAILURE;
  }

  ulLen = pClientReq->ReadInfo.pCur - pClientReq->ReadInfo.pBuf;

  if (ulLen == 0)
  {
    /**
     * Not an Error...Data Not received in this Request.
     */
    return OF_SUCCESS;
  }

  /**
   * Get the bytes written from the cookie record.
   */
  if (HttpCookie_GetBytesWritten(pClientReq->ucCookieVal,
                                 &uiBytesWritten) != OF_SUCCESS)
  {
    return OF_FAILURE;
  }

  /**
   * First time don't wait for pClientReq->pMPart->ucBuf full, call
   * application with what ever data received to take back-up and create
   * new file.
   */
  if ((uiBytesWritten + ulLen < pClientReq->lInContentLen) &&
      (ulLen < HTTP_MPART_LEN)) /*&&(usBytesWritten != 0))  */
  {
    /**
     * Not an error...wait for MPart Buffer full.
     */
    return OF_SUCCESS;
  }

  if (HttpVerifyConnReq(pClientReq->PeerIp,
                        pHttpGlobals->HttpServers[pClientReq->lServId].uiPort)
                            != OF_SUCCESS)
  {
#ifdef HTTPD_DEBUG
    Httpd_Error("HttpClient_UploadBinFile(): HttpVerifyConnReq() failed\n\r");
#endif
    return OF_FAILURE;
  }

  if (pHttpGlobals->HttpServers[pClientReq->lServId].HtmlPermission)
  {
    if (pHttpGlobals->HttpServers[pClientReq->lServId].HtmlPermission((char *) pClientReq->ucFileName,
                                                        &lPriv) != OF_SUCCESS)
    {
#ifdef HTTPD_DEBUG
      Httpd_Error("HttpClient_UploadBinFile(): HtmlPermission() failed\n\r");
#endif
      return OF_FAILURE;
    }
  }

  if ((pHttpGlobals->MutexSockFd != -1) &&
      (pHttpGlobals->MutexSockFd != pClientReq->SockFd))
  {
#ifdef HTTPD_DEBUG
    Httpd_Error("No permissions to you. Already other client is writing data\n\r");
#endif
    return OF_FAILURE;
  }
  /**
   * Introduce Mutex(lock) here to avoid others to write before completing
   * this.
   */
  if (uiBytesWritten == 0)
  {
    pHttpGlobals->MutexSockFd = pClientReq->SockFd;
  }

  HttpClient_SetDummyReq(pClientReq, &DummyReq);

  if (Httpd_StoreBinFile(&DummyReq, ulLen,
                         pClientReq->lInContentLen) != OF_SUCCESS)
  {
#ifdef HTTPD_DEBUG
    Httpd_Error("Httpd_ImageUpload_Process failed\n\r");
#endif
    return OF_FAILURE;
  }

  uiBytesWritten += ulLen;

  /**
   * Set the bytes written in the cookie record.
   */
  if (HttpCookie_SetBytesWritten(pClientReq->ucCookieVal,
                                 uiBytesWritten) != OF_SUCCESS)
  {
    return OF_FAILURE;
  }

  /* pClientReq->ReadInfo.pCur = pClientReq->ReadInfo.pBuf;*/
  HttpClient_SetReadInfo(pClientReq, pClientReq->pMPart->ucBuf,
                         HTTP_MPART_LEN, pClientReq->pMPart->ucBuf);

  if (uiBytesWritten < pClientReq->lInContentLen)
  {
#ifdef HTTPD_DEBUG
    Httpd_Error("Written success received Bytes..waiting for remaining\n\r");
#endif
    return OF_SUCCESS;
  }
  else
  {
   /**
    * Complete the remaining HttpClient_SendResponse functionality here.
    */
   
   /**
    * We need not change the state to pClientReq->eState = hStateSendResp;
    * The final (complete) buffer is  writing into the cookie record.
    */
    pHttpGlobals->MutexSockFd        = -1;

    /**
     * Set the bytes written in the cookie record.
     */
    if (HttpCookie_SetBytesWritten(pClientReq->ucCookieVal,
                                   0) != OF_SUCCESS)
    {
      return OF_FAILURE;
    }
#ifdef HTTPD_DEBUG
    Httpd_Print("Up Load completed\n\r");
#endif
    return OF_SUCCESS;
  }
} /* HttpClient_UploadBinFile() */

/***********************************************************************
 * Function Name : HttpClient_SendResponse                             *
 * Description   : This function is called to send the entire response *
 *                 to the client when receiving, parsing and processing*
 *                 of request string is done. This sends response      *
 *                 Header and the file requested to the client         *
 * Input         : pClientReq - Client Request                         *
 *                 pHttpGlobals  - Global pointer                      *
 * Output        : None                                                *
 * Return value  : OF_SUCCESS on success                                *
 *                 OF_FAILURE on failure                                *
 ***********************************************************************/

/*  */
int32_t HttpClient_SendResponse(HttpGlobals_t  *pHttpGlobals,HttpClientReq_t  *pClientReq)
{
  char      cNewFileName[HTTP_FILENAME_LEN + 1];
  HttpRequest  *DummyReq;
  bool      bLoggedIn = FALSE;
  int32_t      lPriv = HTTPD_PRIV_ADMIN;
  /* Added for HTTP 2.0 */
  unsigned char     *pTemp = NULL;
  unsigned char     aTagName[HTTP_FILENAME_LEN + 1];
  uint16_t     usLen;
  int32_t       iReturnValue = -1;
#ifdef LICENSING
  uint32_t     RetVal;
#endif
#ifdef INTOTO_UAM_SUPPORT
  IGWUAMIpDbRec_t  UAMIpRec;
  int32_t iRetVal =-1;
  //char  UAMFileName[HTTP_FILENAME_LEN];
#ifdef OF_IPV6_SUPPORT
  unsigned char   pTempIpAddr[HTTP_LOCAL_IP_ADDRESS_LEN];
#endif

  of_memset(&UAMIpRec,0,sizeof(IGWUAMIpDbRec_t));
#endif
  /**
   * Validate the passed parameter(s).
   */
   if(!pHttpGlobals)
  {
    Httpd_Error("pHttpGlobals is NULL\r\n");	
    return OF_FAILURE;
  }
  
  if (!pClientReq)
  {
    return OF_FAILURE;
  }
	
  DummyReq=of_calloc(1,sizeof(HttpRequest));
   if(!DummyReq)
  {
    Httpd_Error("DummyReq is NULL\r\n");	
    return OF_FAILURE;
  }
   
#ifndef OF_CM_SUPPORT
#ifdef INTOTO_UAM_SUPPORT
#ifdef OF_IPV6_SUPPORT
  if(pClientReq->PeerIp.ip_type_ul == IGW_INET_IPV6)
  {
      if(HttpdNumToPresentation(&pClientReq->PeerIp,(unsigned char *)pTempIpAddr)!=OF_SUCCESS)
      {	
           HttpDebug((" HttpdNumToPresentation is Failed\r\n")); 	    
	    HTTPDFREE(DummyReq);	   
           return OF_FAILURE;
      }
      IGWAtoN((char *)pTempIpAddr,&(UAMIpRec.SrcIp));	  
  }
  else
  #endif	/*INTOTO_IPV6_SUPPORT*/
  {
     UAMIpRec.SrcIp = pClientReq->PeerIp.IPv4Addr;
  }
  UAMIpRec.SrcPort = pClientReq->PeerPort;
  iRetVal = IGWUAMGetIpRecAPI(&UAMIpRec);
  if((iRetVal == OF_SUCCESS) )
  {
    pClientReq->ulStatus=OF_FAILURE;
    IGWUAMDelIpRecAPI(&UAMIpRec);
    Httpd_Send_Redirect(pClientReq);
    HTTPDFREE(DummyReq);	
    return OF_SUCCESS; 
  }

#endif /*INTOTO_UAM_SUPPORT*/
#endif /*OF_CM_SUPPORT*/



#ifdef HTTP_REDIRECT_HTTPS
  if ((!pHttpGlobals->HttpServers[pClientReq->lServId].bEnabled) &&
      (pHttpGlobals->HttpServers[pClientReq->lServId].Type == hServerNormal))
  {
    pClientReq->ulStatus=OF_FAILURE;
    Httpd_Send_Redirect(pClientReq);
    pClientReq->ulStatus = OF_FAILURE;
    HTTPDFREE(DummyReq);		
    return OF_SUCCESS;
  }
#endif
  if (HttpVerifyConnReq(pClientReq->PeerIp,
                        pHttpGlobals->HttpServers[pClientReq->lServId].uiPort)
                            != OF_SUCCESS)
  {
    HTTPDFREE(DummyReq);	
    return OF_FAILURE;
  }

//#ifdef INTOTO_CMS_SUPPORT
#if (defined(INTOTO_CMS_SUPPORT) || defined(OF_CM_SUPPORT))

  if (pClientReq->eMethod == hMethodPost           && 
      ((!of_strcmp((char *) pClientReq->ucFileName,"login")) ||
      (!of_strcmp((char *) pClientReq->ucFileName,"logout"))))
  {
         HttpClient_SetDummyReq(pClientReq, DummyReq);   
         HttpDebug((" HttpClient_SendResponse()Return\r\n")); 
         return Httpd_PostString_Process(DummyReq);
  }

#endif  /* INTOTO_CMS_SUPPORT */

  if (pClientReq->eMethod == hMethodPost)
  {
    if (pHttpGlobals->HttpServers[pClientReq->lServId].HtmlPermission)
    {
      if (pHttpGlobals->HttpServers[pClientReq->lServId].HtmlPermission(
                      (char *) pClientReq->ucFileName, &lPriv) != OF_SUCCESS)
      {
         HTTPDFREE(DummyReq);	
         return OF_FAILURE;
      }
    }
    if((!pClientReq->ulConnType) || 
      (pClientReq->ulPageType != LOGINPAGEREQUEST))
    {	    

       if (HttpCookie_GetByVal(pClientReq->ucCookieVal, &bLoggedIn, NULL,
                            pClientReq->ucUserName) != OF_SUCCESS)
       {
           HttpClient_SetDummyReq(pClientReq, DummyReq); 
#ifdef INTOTO_UAM_SUPPORT
           if(pClientReq->bRefererFound)
           { 
              if(HttpSendHtmlFile(DummyReq, UAM_GetDefErrorFile(),NULL) == OF_FAILURE) 
              { 
               HTTPDFREE(DummyReq);	
               return OF_FAILURE;
              }
           }
           else 
#endif /*INTOTO_UAM_SUPPORT*/
           {
              if(HttpSendHtmlFile(DummyReq,  Httpd_GetErrorFile(pHttpGlobals),NULL) == OF_FAILURE) 
              { 
               HTTPDFREE(DummyReq);	
               return OF_FAILURE;
              }
           }  
	     HTTPDFREE(DummyReq);			
            return OF_SUCCESS;
       }
    }    
    if (lPriv == HTTPD_PRIV_ADMIN)
    {
      HttpClient_SetDummyReq(pClientReq, DummyReq);

      if (HttpUserIsConfigType((char *) pClientReq->ucUserName) == FALSE)
      {
        HttpNotifyRandomAccess(pClientReq->PeerIp, (char *) pClientReq->ucUserName,
                               "is randomly accessing page",
                              (char *)  pClientReq->ucFileName);

        /**
         * Send the loginout.htm with the relevant information.
         */
        if (HttpSendHtmlFile(DummyReq,  Httpd_GetLogInOutFile(pHttpGlobals),
                            (unsigned char *)  "Access Denied") == OF_FAILURE)
        {
          ; /* NULL statement. */
        }
        HTTPDFREE(DummyReq);			
        return OF_FAILURE;
      }
    }

    HttpClient_SetDummyReq(pClientReq, DummyReq);   
    return Httpd_PostString_Process(DummyReq);
  }

  /********* Adding New HREF method for http2.0 *********/
  /** Check for new tag name (*.igw) for Get method And pass the
   * Application data after '$' or first '*' to the registered callback
   * function. This is bit different from old HREF mechanism using for 'UP'
   * and 'down' arrows (HREF with Application Data). Now the HREF with
   * Application Data format in HTML page is"tag name(*.igw)$ApplicationData"
   * Here Application has to register its callback function for the given
   * tag name (should be in *.igw format). Then whenever we Get this HREF
   * Here the registered callback will be called with the application data.
   * In application callback, it will do processing and sends the next HTML
   * page in case success or sends error message page in case of failure.
   * same like POST method. This like doing similar to"POST"method for GET
   * with Application data. Still we are supporting Old implementation.
   * Example for New HREF :IAP policy up":"iap.igw$[IGW_IAP_RULE_ID]*up*"
   * in iaphttp.c, Callback should registered for"iap.igw"tag name instead
   * of"IAPMOVE"
   */
  /* Search for new HREF tag. i.e, .igw */

  /**
   * Search for ".igw".
   */
  pTemp = (unsigned char *) of_strstr((char *) pClientReq->ucFileName, ".igw");

  if (pTemp != NULL)   /* Found NEW HREF tag */
  {
    /**
     * Increment the length by 4, which is length of ".igw".
     */
    usLen = pTemp-pClientReq->ucFileName + 4;

    of_strncpy((char *) aTagName, (char *) pClientReq->ucFileName, usLen); /* .igw */
    aTagName[usLen] = '\0';

    HttpClient_SetDummyReq(pClientReq, DummyReq);

    /**
     * Find the Application Data to be passed.
     */
    pTemp = (unsigned char *) of_strchr((char *) pClientReq->ucFileName, '$');

    if (pTemp == NULL)
    {
      pTemp = (unsigned char *) of_strchr((char *) pClientReq->ucFileName, '*');
    }

    if (pTemp == NULL)
    {
      pTemp = (unsigned char *) of_strchr((char *) pClientReq->ucFileName, '?');
    }
    /**
   * No Application Data found for the HREF Tag.
   */
    if (pTemp == NULL)
    {
      Httpd_Warning("HttpClient_SendResponse: No App. Data for Tag:%s\n\r",
                    aTagName);
      if (HttpSendHtmlFile(DummyReq, (unsigned char *) "errscrn.htm",
                          (unsigned char *)  "HREF without ApplData is found...Change HREF")
                              == OF_FAILURE)
      {
         HTTPDFREE(DummyReq);	
        return OF_FAILURE;
      }
      HTTPDFREE(DummyReq);	 
      return OF_SUCCESS;
    }
    else
    {
      pTemp++;
    }

    of_strcpy((char *) pClientReq->ucAppData, (char *) pTemp);

    /**
     * Search for Application Callback registered for this TagName.
     */
     iReturnValue = Httpd_HRefTag_Process(aTagName, DummyReq);
     if(iReturnValue != OF_SUCCESS)
     {
        Httpd_Error("HttpClient_SendResponse(): ");
        Httpd_Error("Httpd_HRefTag_Process() failed\n\r");
        HTTPDFREE(DummyReq);	 
        return OF_FAILURE;
     }
     HTTPDFREE(DummyReq);	 
     return OF_SUCCESS;
  }
  else  /* NEW HREF is not found go for old implementation */
  {
    of_strcpy(cNewFileName, "");

    if (pHttpGlobals->HttpServers[pClientReq->lServId].HtmlFilter)
    {
      pHttpGlobals->HttpServers[pClientReq->lServId].HtmlFilter((char *) pClientReq->ucFileName,
                                                  cNewFileName,
                                                  (char *) pClientReq->ucAppData);

      if (of_strcmp(cNewFileName, ""))
      {
        of_strcpy((char *) pClientReq->ucFileName, cNewFileName);
      }
    }

#ifdef OF_CM_SUPPORT
  	if(of_strcmp((char *) pClientReq->ucUrl, "/ucm") == 0)
	{

		HttpRequest DummyRequest;
		int errCode = 0;

		HttpClient_SetDummyReq(pClientReq, &DummyRequest);

		errCode = HttpdUCMConfigGetHandler(&DummyRequest);

		if(errCode == OF_SUCCESS)
		{
	              HTTPDFREE(DummyReq);	
			return OF_SUCCESS;
		}
	}

#endif /*OF_CM_SUPPORT*/

#ifndef OF_CM_SUPPORT
#ifdef OF_XML_SUPPORT
  	if(of_strstr(pClientReq->ucFileName, ".xml"))
	{

		HttpRequest DummyRequest;
		int errCode = 0;

		HttpClient_SetDummyReq(pClientReq, &DummyRequest);

		errCode = IGWXMLCallRegXmlURL(pClientReq->ucFileName, &DummyRequest);

		if(errCode == OF_SUCCESS)
		{
	              HTTPDFREE(DummyReq);	
			return OF_SUCCESS;
		}
	}

#endif /*OF_XML_SUPPORT*/
#endif /*OF_CM_SUPPORT*/

    if (HttpClient_SetHdrFields(pHttpGlobals,pClientReq) != OF_SUCCESS)
    {
      Httpd_Error("HttpClient_SendResponse(): ");
      Httpd_Error("HttpClient_SetHdrFields() failed\n\r");
      HTTPDFREE(DummyReq);	 
      return OF_FAILURE;
    }
#if 0 /*for UCM*/
    if (HttpClient_EvaluateAccess(pHttpGlobals,pClientReq) != OF_SUCCESS)
    {
      Httpd_Error("HttpClient_SendResponse(): ");
      Httpd_Error("HttpClient_EvaluateAccess() failed\n\r");
      HTTPDFREE(DummyReq);	
      return OF_FAILURE;
    }
#endif /*for UCM*/

#ifdef LICENSING
    if (!bLoggedIn)
    {
      if (!(strcasecmp(pClientReq->ucFileName,  Httpd_GetLoginFile(pHttpGlobals)))) /* HTTP_DEF_LOGIN_FILE */
      {
        /**
         * Check registration, if ser num is invalid generate one.
         */
        RetVal = CheckRegistration(GEN_SER_NUM);

        if ((RetVal != REG_VALID) &&
            (RetVal != PERM_KEY))
        {
          if (RetVal == SERNUM_FAILED)
          {
            /**
             * Unable to read/create serial number.
             */
            Httpd_Error("httpd: Invalid Serial Number\n");
   	     HTTPDFREE(DummyReq);			
            return(OF_FAILURE);
          }
          else
          {
            /* new serial number generated */
            sprintf(pClientReq->ucUrl, "/%s",
                      (char *) HTTP_DEF_REG_FILE);

            of_strcpy(pClientReq->ucFileName,
                     (char *) HTTP_DEF_REG_FILE);
          }
        }
      }
    }
#endif
   
    if (HttpClient_SendFile(pHttpGlobals,pClientReq) != OF_SUCCESS)
    {
      Httpd_Error("HttpClient_SendResponse(): ");
      Httpd_Error("HttpClient_SendFile() failed\n\r");
      HTTPDFREE(DummyReq);	
      return OF_FAILURE;
    } 

#ifdef INTOTO_CMS_SUPPORT
    HttpClient_SetDummyReq(pClientReq, DummyReq);
    if(pClientReq->ulPageType == LOGINPAGEREQUEST)
    {
       /*Manage the session if needed */ 
    } 
#endif  /* INTOTO_CMS_SUPPORT */
  }
  HTTPDFREE(DummyReq);	
  return OF_SUCCESS;
} /* HttpClient_SendResponse() */

/***********************************************************************
 * Function Name : HttpClient_SendHeader                               *
 * Description   : This send the response header to the client         *
 *                 depending on header fields present in context       *
 *                 Before calling this all header fields should be set *
 * Input         : pClientReq - Client Request                         *
 *                 pHttpGlobals  - Global pointer                      *   
 * Output        : None                                                *
 * Return value  : OF_SUCCESS on success                                *
 *                 OF_FAILURE on failure                                *
 ***********************************************************************/

int32_t HttpClient_SendHeader(HttpGlobals_t  *pHttpGlobals,HttpClientReq_t  *pClientReq )
{
  unsigned char                   tmpBuf[512], *pStr;
  int32_t                       len=0;
  HttpdSetCookieInfo_t   *pTemp = NULL;
  int32_t			          Flag=0;
  unsigned char                     ucExt[HTTP_FILE_EXTENTION_MAX_LEN +1];
  /**
   * Validate the passed parameter(s).
   */
  if(!pHttpGlobals)
  {
    Httpd_Error("pHttpGlobals is NULL\r\n");	
    return OF_FAILURE;
  }
  
  if (!pClientReq)
  {
    return OF_FAILURE;
  }

  /* Write the status line */
  len = sprintf((char *) tmpBuf, (char *)( "%s %d %s%s"),
                  httpVersions_f[pClientReq->eVersion].hvName,
                  errInfo_f[pClientReq->ulStatus].code,
                  errInfo_f[pClientReq->ulStatus].msg, CRLF);

  of_strncpy((char *) pClientReq->ucHeaderBuff,(char *) tmpBuf,len);
  
  /**
   * Write the Server header.
   */
  len = sprintf((char *) tmpBuf, (char *)"Server: %s v%s%s", pHttpGlobals->HttpdConfig.pSrvName,
                  pHttpGlobals->HttpdConfig.pSrvVer, CRLF);
   of_strncat((char *) pClientReq->ucHeaderBuff,(char *) tmpBuf,len);
    if (pClientReq->ulPageType == LOGINPAGEREQUEST)
  {
  len = sprintf((char *)tmpBuf, (char *)"Set-Cookie: %s=%s; path=/; %s",Http_GetCookieName(pHttpGlobals),
	                    pClientReq->ucCookieVal, CRLF);
	    of_strncat((char *) pClientReq->ucHeaderBuff,(char *) tmpBuf,len);
    }

  /* Gen-Cookie */
  pTemp = pClientReq->pSetCookieHead;

  while (pTemp != NULL)
  {
    /**
     * We can check pTemp->CookieBuff is having"Set-Cookie"..if so we can
     * copy pTemp->CookieBuff to tempBuf.
     */
    pStr = (unsigned char *) of_strstr((char *) pTemp->CookieBuff, "Set-Cookie");

    if (pStr != NULL) /* (unsigned char *)pTemp->CookieBuff)*/
    {
		len = of_strlen((char *) pTemp->CookieBuff);
		of_strncat((char *) pClientReq->ucHeaderBuff,(char *) pTemp->CookieBuff,len);
    }
    else
    {
      Httpd_Error("HttpClient_SendHeader:: No Generic Cookie\n\r");
    }

    pTemp = pTemp->pNext;
  }

  /**
   * Write the Content-type header.
   */
  if (pClientReq->ucOutContentType[0])
  {
	    len = sprintf((char *) tmpBuf, "Content-type: %s%s",
	                   (char *)  pClientReq->ucOutContentType, CRLF);

	    /* fix for https download problem with internet explorer */

	    if(!strcasecmp((char *) pClientReq->ucOutContentType,"application/download") ||
		 (HttpdTemplateFileAccess(pHttpGlobals,(char *) pClientReq->ucFileName) == OF_SUCCESS))  /*Added for XSL template processing*/
	    {
		    Flag=1;
	    }
	     of_strncat((char *) pClientReq->ucHeaderBuff,(char *) tmpBuf,len);
   }

  /**
   * If we know the content_length, we can fulfill byte range requests.
   */
  if (pClientReq->lOutContentLen >= 0)
  {
         len = sprintf((char *) tmpBuf, (char *) "Content-length: %d%s",
                         pClientReq->lOutContentLen, CRLF);     
	 of_strncat((char *) pClientReq->ucHeaderBuff,(char *) tmpBuf,len);
      
  }
  else if((!pClientReq->bDrop) && (pClientReq->eMethod!=hMethodPost))
   /*&&(of_strcmp(pClientReq->ucFileName,HTTP_HTML_WELCOME_PAGE_NAME)))*/
   /*Refer BugID:34459*/
  {
	    len = sprintf((char *) tmpBuf, (char *) "Content-Length: %u %s",pClientReq->ulTotalFileSize, CRLF);		  
           of_strncat((char *) pClientReq->ucHeaderBuff,(char *) tmpBuf,len);  
  }
 
  if (pClientReq->bExpireByOneDay)
  {
	    struct tm Tm, *pTm = &Tm;
	    time_t tme;

	    if (of_time(&tme) == -1)
	    {
	      Httpd_Error("time function returned error\r\n");
	      return 0;
	    }
	    tme += (60 * 24); /* One Day */

	    /*Httpd_GetGMT(&tme, pTm);*/
	    gmtime_r(&tme, pTm);

	    /* RFC822 Date Format */
	    len = sprintf((char *) tmpBuf,(char *)  "Expires: %s, %.2d %s %d %.2d:%.2d:%.2d GMT%s",
	                    day_snames[pTm->tm_wday], pTm->tm_mday,
	                    month_snames[pTm->tm_mon], pTm->tm_year + 1900,
	                    pTm->tm_hour, pTm->tm_min, pTm->tm_sec, CRLF);
	    of_strncat((char *) pClientReq->ucHeaderBuff,(char *) tmpBuf,len);
  }
  else
  {
	    /**
	     * Send no-cache pragma for all files other than gif files.
	     */

	    if(Flag==0)
	    {
		      len = sprintf((char *) tmpBuf,(char *)  "Pragma: no-cache %s", CRLF);
		      of_strncat((char *) pClientReq->ucHeaderBuff,(char *) tmpBuf,len);
	    }

  }
  
  if (Httpd_GetFileExt(pClientReq->ucFileName, ucExt, HTTP_MAX_EXT_LEN) != OF_SUCCESS)
  {
#ifdef INTOTO_CMS_SUPPORT
     Httpd_Debug("Httpd_GetFileExt CMS URL continue....\r\n "); 
     /* fix for /cms url */
#else
     Httpd_Error("Httpd_GetFileExt returned failure\r\n ");
   //  return OF_FAILURE; //HARISH
#endif  /* INTOTO_CMS_SUPPORT */
  }
  if (!of_strcmp((char *) ucExt, HTTP_APPLET_FILE_EXT) || !of_strcmp((char *) ucExt,"jnlp"))
  {
      struct tm Tm, *pTm = &Tm;
      time_t tme;
  
      if (of_time(&tme) == -1)
      {
         Httpd_Error("time function returned error\r\n");
         return OF_FAILURE;
      }
      tme += (60 * 24); /* One Day */
  
      /*Httpd_GetGMT(&tme, pTm);*/
      gmtime_r(&tme, pTm);

  		 /* RFC822 Date Format */
      len = sprintf((char *) tmpBuf,(char *)  "Last-Modified: %s, %.2d %s %d %.2d:%.2d:%.2d GMT%s",
                      day_snames[pTm->tm_wday], pTm->tm_mday,
                      month_snames[pTm->tm_mon], pTm->tm_year + 1900,
                      pTm->tm_hour, pTm->tm_min, pTm->tm_sec, CRLF);
  
      of_strncat((char *) pClientReq->ucHeaderBuff,(char *) tmpBuf,len);
  }
  
 /**
  *don't close the connection immediately.
  */
#if 0
  if((pClientReq->bDrop) || (pClientReq->eMethod==hMethodPost) || 
     (!of_strcmp(pClientReq->ucFileName, 
     Httpd_GetLoginFile(pHttpGlobals))))
#endif
  if((pClientReq->bDrop) || (pClientReq->eMethod==hMethodPost))
  {
     	len = sprintf((char *) tmpBuf,(char *)  "Connection: Close %s", CRLF);
  }
  else
  {
    	 len = sprintf((char *) tmpBuf, (char *) "Connection: keep-alive %s", CRLF);
  }
  of_strncat((char *) pClientReq->ucHeaderBuff,(char *) tmpBuf,len);
  
  len = sprintf((char *) tmpBuf, "%s", CRLF);
  of_strncat((char *) pClientReq->ucHeaderBuff,(char *) tmpBuf,len);
  return OF_SUCCESS;
 } /* HttpClient_SendHeader() */

/***********************************************************************
 * Function Name : Httpd_ReadFile                                      *
 * Description   : This reads the entire content of the file to the    *
 *                 global buffer.                                      *
 * Input         : pFileName - Name of the file                        *
 *                 pHttpGlobals  - Global pointer                      *
 * Output        : None                                                *
 * Return value  : OF_SUCCESS on success                                *
 *                 OF_FAILURE on failure                                *
 ***********************************************************************/

int32_t Httpd_ReadFile(HttpGlobals_t *pHttpGlobals, unsigned char  *pFileName )
{
  int32_t	lFd, iTemp;
  uint32_t	 lInLen=0;
  //uint32_t	 lOutLen=HTTP_MAX_FILE_SIZE;

  if(!pHttpGlobals)
  {
    Httpd_Error("pHttpGlobals is NULL\r\n");	
    return OF_FAILURE;
  }

  lFd = HtmlFs_OpenFile( pHttpGlobals,(char *) pFileName, UCMFILE_MODE_READ);
  if (lFd < 0)
  {
    HttpDebug("Invalid fd_i  \r\n")
    return OF_FAILURE;
  }
  HtmlFs_SeekFile(pHttpGlobals,lFd, UCMFILE_SEEK_BEG, 0);
  
  while ((iTemp = HtmlFs_ReadFile(pHttpGlobals,lFd, (char *) pHttpGlobals->HttpFileBuf.HttpInputBuf + pHttpGlobals->HttpFileBuf.ulFileSize, HTTP_READ_SIZE)) > 0)
  {
	   Httpd_Heavy_Debug("Method is not deflated. no unzipping is needed\n");
	   pHttpGlobals->HttpFileBuf.ulFileSize += iTemp;
	
	   if (pHttpGlobals->HttpFileBuf.ulFileSize > (HTTP_MAX_FILE_SIZE))
	   {
			 Httpd_Error("File size is greater than 45k. returning error");
			 HtmlFs_CloseFile(pHttpGlobals,lFd);
			 return OF_FAILURE;
	   }
	   if (iTemp != HTTP_READ_SIZE)
	   {
		 break;
	   }
 }
 if (lFd < 0)
 {
       HttpDebug(("invalid fd_i  \r\n"))
	return OF_FAILURE;
 }
 lInLen=pHttpGlobals->HttpFileBuf.ulFileSize; 
  //  iRetVal= T_NOT_DEFLATED;   ///BALAJI
// iRetVal = IGW_Uncompress(pHttpGlobals->HttpFileBuf.HttpInputBuf,lInLen,pHttpGlobals->HttpFileBuf.HttpUnzipBuf,(uint32_t *)&lOutLen);	 

//  if (iRetVal == T_NOT_DEFLATED)
  {
	  of_memcpy(pHttpGlobals->HttpFileBuf.HttpUnzipBuf,pHttpGlobals->HttpFileBuf.HttpInputBuf,lInLen);
	  pHttpGlobals->HttpFileBuf.HttpUnzipBuf[lInLen] = '\0';
  }
/*
  else
  {  
	 pHttpGlobals->HttpFileBuf.HttpUnzipBuf[lOutLen] = '\0';
  } */

  HtmlFs_CloseFile(pHttpGlobals,lFd);
  return OF_SUCCESS;
} /* Httpd_ReadFile() */

#ifdef OF_XML_SUPPORT
/**************************************************************************
 * Function Name : HttpdXmlParseUrl					  *
 * Description   : This is called to parses the URL			  *
 * Input         : pClientReq (I) - Client Request                        *
 *                 pArray       (I) - 		                          *
 * Output        : None                                                   *
 * Return value  : OF_SUCCESS on success                                   *
 *                 OF_FAILURE on failure                                   *
 **************************************************************************/

int32_t HttpdXmlParseUrl(unsigned char *value_p,IGWGetParams_t *pXmlArray)
{

  unsigned char *pUrlValue=NULL; 
  unsigned char *pTemp=NULL; 
  int16_t ii=0;
  int32_t lUrlLength=0;

  if(!value_p)
  {
     HttpDebug("url value is  Null \r\n");	 
     return OF_FAILURE;
  }
  lUrlLength=of_strlen((char *) value_p);          
  pUrlValue=of_malloc(lUrlLength+1); 
  if(!pUrlValue)
  {
     HttpDebug("malloc failed\r\n");	 
     return OF_FAILURE;
  
  }
  of_strncpy((char *) pUrlValue, (char *) value_p, lUrlLength+1);
   
  pTemp=(unsigned char *) HttpStrTok((char *) pUrlValue,"=&");
  if(!pTemp)
  {
     HttpDebug("Null value returned with strtok\r\n");
    if(pUrlValue!=NULL)
    {
         of_free(pUrlValue);
    } 
     return OF_FAILURE;
  }

  while(pTemp)
  { 
    if(ii < HTTP_XML_ASSOCLIST_LEN)
    {
      if(of_strlen((char *) pTemp)<= HTTP_XML_GETPARAMS_LEN)
        of_strcpy((char *) pXmlArray[ii].ucName,(char *) pTemp);	 
      else
      {
        if(pUrlValue!=NULL)
        {
          of_free(pUrlValue);
        }
        return OF_FAILURE;
      }

      pTemp=(unsigned char *) HttpStrTok(NULL,"=&");	 
      if(pTemp)
      {
        if(of_strlen((char *) pTemp)<= HTTP_XML_GETPARAMS_LEN)
          of_strcpy((char *) pXmlArray[ii].ucValue,(char *) pTemp);
        else
        {
          if(pUrlValue!=NULL)
          {
            of_free(pUrlValue);
          }
          return OF_FAILURE;
        }
      }

      pTemp=(unsigned char *) HttpStrTok(NULL,"=&");	 
      ii++;
    }
    else
    {
      if(pUrlValue!=NULL)
      {
        of_free(pUrlValue);
      }
      return OF_FAILURE;
    }
  }
  if(pUrlValue!=NULL)
  {
     of_free(pUrlValue);
  }
#ifdef HTTPD_DEBUG
  if(pXmlArray!=NULL)
  {
     ii=0;
     while(ii!=HTTP_XML_ASSOCLIST_LEN)
     { 
         printf("----NAME---------VALUE-------\r\n");	
         printf("     %s          ",pXmlArray[ii].ucName);
         printf("     %s          \r\n",pXmlArray[ii].ucValue);
         ii++;	
     } 	
  }	
#endif /*HTTPD_DEBUG*/

 return OF_SUCCESS;
}/*HttpdXmlParseUrl*/
#endif /*OF_XML_SUPPORT*/

/**************************************************************************
 * Function Name : HttpClient_SetUrl                                      *
 * Description   : This is called to set the request url in the context   *
 *                 This parses it and removes escape sequences and sets   *
 *                 plain url in the context                               *
 * Input         : pClientReq (I) - Client Request                        *
 *                 pUrl       (I) - Raw Url String                        *
 * Output        : None                                                   *
 * Return value  : OF_SUCCESS on success                                   *
 *                 OF_FAILURE on failure                                   *
 **************************************************************************/


int32_t HttpClient_SetUrl( HttpClientReq_t  *pClientReq,
                           unsigned char         *pUrl )
{
  unsigned char  *pTemp=NULL;
 #ifdef OF_XML_SUPPORT
   IGWGetParams_t *pXmlArray;
#endif /*OF_XML_SUPPORT*/
 uint32_t ulUrlLength=0;
 HttpGlobals_t *pHttpg=NULL; 
 
 
  /**
   * Validate the passed parameter(s).
   */
  if ((!pClientReq) ||
      (!pUrl))
  {
    return OF_FAILURE;
  }

  /* back slash is extra in the following case */
  if (of_strstr((char *) pUrl, ".xml"))
    of_strcpy(aPrevXmlUrlData,(char *) pUrl+1);


  pHttpg=(HttpGlobals_t *)pClientReq->pGlobalData;
  ulUrlLength=Httpd_GetUrlSize(pHttpg);/*pHttpg->pHttpGlobalConfig->ulUrlLength;*/
  if (!(of_strstr((char *) pUrl, ".igw")))
  {
    pTemp = (unsigned char *) of_strchr((char *) pUrl, '?');
    if (pTemp)
    {
      *pTemp++ = '\0';
    }
  }
  of_strncpy((char *) pClientReq->ucUrl, (char *) pUrl, ulUrlLength);
  pClientReq->ucUrl[ulUrlLength] = '\0';

  if (!pTemp)
  {
    of_strcpy((char *) pClientReq->ucArgs, "");
  }
  else
  {
    of_strncpy((char *) pClientReq->ucArgs, (char *) pTemp, HTTP_ARGS_LEN);
    pClientReq->ucUrl[HTTP_ARGS_LEN] = '\0';
  }

  if (!of_strcmp((char *) pClientReq->ucUrl, "/"))
  {  	
    sprintf((char *) pClientReq->ucUrl, "/%s",  Httpd_GetLoginFile(pHttpg));  /* HTTP_DEF_LOGIN_FILE */
    pClientReq->ulPageType = LOGINPAGEREQUEST;
  }

  Httpd_UnEscapeUrl(pClientReq->ucUrl);
  
#ifdef HTTPD_DEBUG
  printf("\n pClientReq->ucUrl = %s \r\n", pClientReq->ucUrl);
#endif

 #ifdef OF_XML_SUPPORT   
  if(pTemp)
  {
    Httpd_UnEscapeUrl(pTemp);
  }
  if (!(of_strstr((char *) pClientReq->ucUrl, ".igw")))
  {
    pXmlArray=of_calloc(HTTP_XML_ASSOCLIST_LEN,sizeof(IGWGetParams_t));
    if(!pXmlArray)
    {
   	HttpDebug("calloc failed for pXmlArray \r\n");	 
   	return OF_FAILURE;
    }
  /*call url parser for XML*/ 
   if(HttpdXmlParseUrl(pTemp,pXmlArray)!=OF_FAILURE)
   {
      pClientReq->pApplData=(void *)pXmlArray;    
   }
   else
   {
     of_free(pXmlArray);
   }
 }
 #endif /*OF_XML_SUPPORT*/
  return OF_SUCCESS;
} /* HttpClient_SetUrl() */

/**************************************************************************
 * Function Name : HttpClient_ParseRequestLine                            *
 * Description   : This parses first line of the request for method, URL  *
 *                 and version.                                           *
 * Input         : pClientReq (I) - Client Request                        *
 *                 pFirstLine (I) - First Line Of The Request             *
 * Output        : None                                                   *
 * Return value  : OF_SUCCESS on success                                   *
 *                 OF_FAILURE on failure                                   *
 **************************************************************************/


int32_t HttpClient_ParseRequestLine( HttpClientReq_t  *pClientReq,
                                     unsigned char         *pFirstLine )
{
  unsigned char  *pCur;
  unsigned char  *pMeth;
  unsigned char  *pUrl;
  unsigned char  *pVer;

  /**
   * Validate the passed parameter(s).
   */
  if ((!pClientReq) ||
      (!pFirstLine))
  {
    return OF_FAILURE;
  }

  if (!of_strcmp((char *) pFirstLine, ""))
  {
    return OF_FAILURE;
  }

  pMeth = pUrl = pVer = NULL;
  pCur = pFirstLine;

  pMeth = pCur;

  /**
   * Move till a spcae/tab.
   */
  while ((*pCur != '\0') &&
         (*pCur != ' ') &&
         (*pCur != '\t'))
  {
    pCur++;
  }

  if (*pCur == 0)
  {
    return OF_FAILURE; /* Incomplete Request */
  }

  *pCur++ = 0;

  /**
   * Ignore Spaces/Tabs.
   */
  while ((*pCur != '\0') &&
         ((*pCur == ' ') ||
          (*pCur == '\t')))
  {
    pCur++;
  }

  if (*pCur == 0)
  {
    return OF_FAILURE; /* Incomplete Request */
  }

  pUrl = pCur;

  /**
   * Move till a spcae/tab.
   */
  while ((*pCur != '\0') &&
         (*pCur != ' ') &&
         (*pCur != '\t'))
  {
    pCur++;
  }

  if (*pCur == 0)
  {
    return OF_FAILURE; /* Incomplete Request */
  }

  *pCur++ = 0;

  /**
   * Ignore Spaces/Tabs.
   */
  while ((*pCur != '\0') &&
         ((*pCur == ' ') ||
          (*pCur == '\t')))
  {
    pCur++;
  }

  if (*pCur == 0)
  {
    return OF_FAILURE; /* Incomplete Request */
  }

  pVer = pCur;

  pClientReq->eMethod  = HttpClient_MethStr2Enum(pMeth);
  //pClientReq->eVersion = HttpClient_VersionStr2Enum(pVer);
 // For launching logviewer issues
  pClientReq->eVersion = hVersion10;

  if ((pClientReq->eMethod == hMethodBad) ||
      (pClientReq->eVersion == hVersionBad))
  {
    return OF_FAILURE;
  }

  if (HttpClient_SetUrl(pClientReq, pUrl) != OF_SUCCESS)
  {
    return OF_FAILURE;
  }

  return OF_SUCCESS;
} /* HttpClient_ParseRequestLine() */

/**************************************************************************
 * Function Name : HttpClient_ParseCookieValue                            *
 * Description   : Gives the cookie value                                 *
 * Input         : PtraFieldVal    (I) - Pointer To Cookie Filed String   *
 * Output        : PtraCookieValue (I) - Output Cookie Value              *
 * Return value  : OF_SUCCESS on success                                   *
 *                 OF_FAILURE on failure                                   *
 **************************************************************************/


int32_t HttpClient_ParseCookieValue( char  *PtraFieldVal,
                                     char  *PtrCookieValue )
{
  int32_t ii, jj;
  int32_t iMaxCookieValueLen; 
  HttpGlobals_t *pHttpg=NULL;

  pHttpg=pHTTPG;
  if(!pHttpg)
  {
     Httpd_Error("pHttpg is NULL\r\n");
     return OF_FAILURE;
  }
  if (of_strlen(PtraFieldVal) <= 0)
  {
    return OF_FAILURE;
  }

  if ((PtraFieldVal = (char *)
                            (of_strstr(PtraFieldVal,(char *) Http_GetCookieName(pHttpg)))) == NULL)
  {
    return OF_FAILURE;
  }
  iMaxCookieValueLen=Httpd_GetMaxCookieValueLen(pHttpg);/*pHttpg->pHttpGlobalConfig->ulHttpdMaxCookieValueLen;*/
  /**
   * Old code:
   * for (ii = 0; PtraFieldVal[ii] != '\0' && ii <= HTTP_COOKIE_VAL_LEN;
   * ii++)
   */
  for (ii = 0; PtraFieldVal[ii] != '\0'; ii++)
  {
    if (PtraFieldVal[ii] == '=')
    {
      for (jj = ii+1;
           (PtraFieldVal[jj] != '\0') &&
           (jj - (ii + 1)) <=iMaxCookieValueLen; jj++)
      {
        if (PtraFieldVal[jj] == ';')
        {
          break;
        }

        PtrCookieValue[jj - (ii + 1)] = PtraFieldVal[jj];
      }
      PtrCookieValue[jj - (ii + 1)] = '\0';

      return OF_SUCCESS;
    }
  }

  return OF_FAILURE;
} /* HttpClient_ParseCookieValue() */

/**************************************************************************
 * Function Name : HttpClient_ParseHeaderLine                             *
 * Description   : This parses a line of request header and sets the      *
 *                 appropriate field in the client context. This is for   *
 *                 each line in the request header.                       *
 * Input         : pClientReq (I) - pointer to the ClientReq structure    *
 *                 pHdrLine   (I) - One Line Of The Request Header        * 
 *                 pHttpGlobals  - Global pointer                         *
 * Output        : None                                                   *
 * Return value  : OF_SUCCESS on success                                   *
 *                 OF_FAILURE on failure                                   *
 **************************************************************************/


int32_t HttpClient_ParseHeaderLine(HttpGlobals_t *pHttpGlobals,HttpClientReq_t  *pClientReq,
                                    unsigned char         *pHdrLine )
{
  unsigned char  *aFieldName;
  unsigned char  *aFieldVal;
  unsigned char  *pTemp;
#ifdef INTOTO_UAM_SUPPORT
  IGWUAMIpDbRec_t  UAMIpRec;
  int32_t iRetVal =-1;
  IGWUAMRedirAPIInfo_t RedirInfo = {};

  of_memset(&UAMIpRec,0,sizeof(IGWUAMIpDbRec_t));
  of_memset(&RedirInfo, 0, sizeof(IGWUAMRedirAPIInfo_t));
#endif
  

  /**
   * Validate the passed parameter(s).
   */

  if(!pHttpGlobals)
  {
    Httpd_Error("pHttpGlobals is NULL\r\n");	
    return OF_FAILURE;
  }
   
  if ((!pClientReq) ||
      (!pHdrLine))
  {
    return OF_FAILURE;
  }

  pTemp = (unsigned char *) of_strchr((char *) pHdrLine, ':');

  if (!pTemp)
  {
    return OF_SUCCESS;
  }

  *pTemp++ = '\0';

  while (isspace(*pTemp))
  {
    ++pTemp;
  }

  aFieldName = pHdrLine;
  aFieldVal  = pTemp;

  if (!strcasecmp((char *) aFieldName, "content-length"))
  {
    sscanf((char *) aFieldVal, "%d", &(pClientReq->lInContentLen));
  }
  else if (!strcasecmp((char *) aFieldName, "content-type"))
  {
    of_strncpy((char *) pClientReq->ucInContentType, (char *) aFieldVal, HTTP_CNTTYPE_LEN);
    pClientReq->ucInContentType[HTTP_CNTTYPE_LEN] = '\0';
  }
  else if (!strcasecmp((char *) aFieldName, "Connection"))
  {
      if(!strcasecmp((char *) aFieldVal,"Keep-Alive") && (pClientReq->eVersion==hVersion11))
      {
                pClientReq->bPersistentConn=TRUE; 
      }
      else
      {
         pClientReq->bPersistentConn=FALSE; 
      }
  }
#ifdef INTOTO_UAM_SUPPORT
  else if (!strcasecmp(aFieldName, "Referer"))
  {
    if(UAM_CheckForUAMRelatedFile(aFieldVal) == OF_SUCCESS)
    {
      pClientReq->bRefererFound = TRUE;
    }
  }
#endif  
  else if (!strcasecmp((char *) aFieldName, "Host"))
  {
	  pClientReq->bHost=TRUE;
          of_memset(pClientReq->ucArgs,0,sizeof(HTTP_ARGS_LEN+1));
          of_strncpy((char *) pClientReq->ucArgs,(char *) aFieldVal,HTTP_ARGS_LEN);
	  pClientReq->ucArgs[HTTP_ARGS_LEN]='\0';

#ifndef OF_CM_SUPPORT
#ifdef INTOTO_UAM_SUPPORT
#ifdef OF_IPV6_SUPPORT
  if( pClientReq->PeerIp.ip_type_ul == IGW_INET_IPV6 &&
      IGWIsIPv4MappedV6Addr(&pClientReq->PeerIp.IP.IPv6Addr) )
  {
    UAMIpRec.SrcIp = ntohl(pClientReq->PeerIp.IPv6Addr32[3]);
  }
  else
#endif	/*INTOTO_IPV6_SUPPORT*/
  {
     UAMIpRec.SrcIp = pClientReq->PeerIp.IPv4Addr;
  }
  UAMIpRec.SrcPort = pClientReq->PeerPort;
  iRetVal = IGWUAMGetIpRecAPI(&UAMIpRec);
  if((iRetVal == OF_SUCCESS) )
  {
    RedirInfo.SrcIp = UAMIpRec.SrcIp;
    of_strcpy(RedirInfo.RedirURL,aFieldVal);
    IGWUAMAddRedirInfoAPI(&RedirInfo);
  }
#endif /*INTOTO_UAM_SUPPORT*/
#endif /*OF_CM_SUPPORT*/

  }
 #ifdef OF_XML_SUPPORT
  else if (!strcasecmp((char *) aFieldName, "User-Agent"))
  {
	  if(HttpClient_FindBrowserNameAndVersion(pClientReq,aFieldVal)!=OF_SUCCESS)
	  {
              HttpDebug("HttpClient_FindBrowserNameAndVersion failed ");
              return OF_FAILURE;
	  }
  }
  #endif  /*OF_XML_SUPPORT*/ 
  else if (!strcasecmp((char *) aFieldName, "cookie"))
  {
    return HttpClient_ProcessCookie(pHttpGlobals,pClientReq, aFieldVal);
  }

  return OF_SUCCESS;
} /* HttpClient_ParseHeaderLine() */

/**************************************************************************
 * Function Name : HttpClient_ProcessCookie                               *
 * Description   : This checks the validity of cookie received from client*
 *                 and takes appropriate action.                          *
 * Input         : pClientReq (I) - pointer to the ClientReq structure.   *
 *                 aFieldVal  (I) - pointer to the field value.           *
 *                 pHttpGlobals  - Global pointer                         * 
 * Output        : None                                                   *
 * Return value  : OF_SUCCESS on success                                   *
 *                 OF_FAILURE on failure                                   *
 **************************************************************************/


int32_t HttpClient_ProcessCookie(HttpGlobals_t *pHttpGlobals,HttpClientReq_t  *pClientReq,
                                  unsigned char         *aFieldVal )
{
  bool               bLoggedIn = FALSE;
  unsigned char              *ucParsedCookieVal;
  unsigned char              *pNextCookie;
  unsigned char              *pCookieHdr;
  unsigned char              *endP;
  unsigned char              *start;
  HttpdGetCookieInfo_t  *pGetTemp = NULL;
  int32_t               iCookieMaxValueLen;
  int32_t               iCookieMaxBufferLen;
#ifdef INTOTO_UAM_SUPPORT
 IGWUAMUserRec_t UAMUser;
 //char UAMFileName[HTTP_FILENAME_LEN];
#ifdef OF_IPV6_SUPPORT
 unsigned char pTempIpAddr[HTTP_LOCAL_IP_ADDRESS_LEN];
#endif

 of_memset(&UAMUser,0,sizeof(IGWUAMUserRec_t));  
#endif

  if(!pHttpGlobals)
  {
    Httpd_Error("pHttpGlobals is NULL\r\n");	
    return OF_FAILURE;
  }

 /**
  * Validate the passed parameter(s).
  */
 if ((!pClientReq) ||
      (!aFieldVal))
 {
    return OF_FAILURE;
 }
 
#ifndef OF_CM_SUPPORT
#ifdef INTOTO_UAM_SUPPORT
#ifdef OF_IPV6_SUPPORT
 if(pClientReq->PeerIp.ip_type_ul == IGW_INET_IPV6)
 {
       if(HttpdNumToPresentation(&pClientReq->PeerIp,(unsigned char *)pTempIpAddr)!=OF_SUCCESS)
      {	
           HttpDebug((" HttpdNumToPresentation is Failed\r\n"));
           return OF_FAILURE;
      }
      IGWAtoN((char *)pTempIpAddr,&(UAMUser.IpAddr));	  
 }
 else
#endif /*INTOTO_IPV6_SUPPORT*/
 {
   UAMUser.IpAddr= pClientReq->PeerIp.IPv4Addr;
 }
#endif /*INTOTO_UAM_SUPPORT*/
#endif /*OF_CM_SUPPORT*/
iCookieMaxValueLen=Httpd_GetMaxCookieValueLen(pHttpGlobals);/*pHttpGlobals->pHttpGlobalConfig->ulHttpdMaxCookieValueLen;*/
iCookieMaxBufferLen=Httpd_GetMaxCookieBufferLen(pHttpGlobals);/*pHttpGlobals->pHttpGlobalConfig->ulHttpdMaxCookieBufferLen;*/
 pCookieHdr = aFieldVal;
 ucParsedCookieVal=of_calloc(1,iCookieMaxValueLen+1);
 if(! ucParsedCookieVal)
 {
    Httpd_Print("Memory is not allocated for ucParsedCookieVal::T_Calloc Failed\r\n");
    return OF_FAILURE;
 }
  if (HttpClient_ParseCookieValue((char *) aFieldVal,
                                  (char *) ucParsedCookieVal) == OF_SUCCESS)
  {
    if (HttpCookie_GetByVal(ucParsedCookieVal, &bLoggedIn, NULL,
                            pClientReq->ucUserName) == OF_SUCCESS)
    {
      if ((pClientReq->ulPageType == LOGINPAGEREQUEST) &&
          (!bLoggedIn))
      {
        HttpCookie_Del(ucParsedCookieVal);
        sprintf((char *) pClientReq->ucUrl, "/%s",
                   Httpd_GetLoginFile(pHttpGlobals)); 
       Http_ProcessRegModuleRequest(pHttpGlobals,pClientReq);       
       pClientReq->ulPageType = LOGINPAGEREQUEST;
       pClientReq->eMethod    = hMethodGet;
      }
      else
      {
        if ((pClientReq->ulPageType == LOGINPAGEREQUEST) &&
            (bLoggedIn))
        {
          if (HttpUserIsConfigType((char *) pClientReq->ucUserName) == TRUE)
          {
            sprintf((char *) pClientReq->ucUrl, "/%s",
                       Httpd_GetLogoutFile(pHttpGlobals));  
            pClientReq->ulPageType = SENDMAXAGEZERO;
          }
          else
          {
            sprintf((char *) pClientReq->ucUrl, "/%s",
                       Httpd_GetUlogoutFile(pHttpGlobals));  
            pClientReq->ulPageType = SENDMAXAGEZERO;
          }
        }
      }
    }
    else
    {
      if (HttpdCheckUrlFileAccess(pHttpGlobals,(char *) pClientReq->ucUrl)!=OF_SUCCESS)
       {
#ifdef INTOTO_UAM_SUPPORT
        if(pClientReq->bRefererFound)
        {
          sprintf((char *) pClientReq->ucUrl, "/%s", UAM_GetDefErrorFile());   
          pClientReq->ulPageType = LOGINPAGEREQUEST;
        }
        else
#endif /*INTOTO_UAM_SUPPORT*/
        {
          sprintf((char *) pClientReq->ucUrl, "/%s",  Httpd_GetErrorFile(pHttpGlobals));   
          pClientReq->ulPageType = ERRORPAGEREQUEST;
        }
       }
      else
      {
	       #ifdef INTOTO_UAM_SUPPORT
	        if(iRetVal==OF_SUCCESS)
          	 {
   			 if(IGWUAMIsUserInLogOffDbAPI(&UAMUser) == OF_SUCCESS)
   		        {
   		   	          if(UAMUser.bIsIdleTmOut == TRUE)
   		   		  {
	   		   	   sprintf((char *) pClientReq->ucUrl, "/%s", UAM_GetDefIdleTmoutFile());
	   		   		    pClientReq->ulPageType = LOGINPAGEREQUEST;
   		   		  }
   
   		  		  if(UAMUser.bIsSessTmOut == TRUE)
   		  		  {
	   		  		    sprintf(pClientReq->ucUrl, "/%s",UAM_GetDefSessTmoutFile());
	   		  		    pClientReq->ulPageType = LOGINPAGEREQUEST;
   		  		  }
   			 }
	        }
	        else
	        #endif
		 {
	 	          Http_ProcessRegModuleRequest(pHttpGlobals,pClientReq);
	 	          pClientReq->ulPageType = LOGINPAGEREQUEST;
	        }
	        if(!pClientReq->ulConnType) 
		{
	          pClientReq->eMethod = hMethodGet;
		}
	 }
     }
    of_strncpy((char *)pClientReq->ucCookieVal,(char *) ucParsedCookieVal,
              iCookieMaxValueLen);

    pClientReq->ucCookieVal[iCookieMaxValueLen] = '\0';
  }
else
{
	    #ifdef INTOTO_UAM_SUPPORT
	     if(iRetVal==OF_SUCCESS)
	     {
		    if(IGWUAMIsUserInLogOffDbAPI(&UAMUser) == OF_SUCCESS)
		    {
		  	      if(UAMUser.bIsIdleTmOut == TRUE)
		  	      {
			  		sprintf(pClientReq->ucUrl, "/%s", UAM_GetDefIdleTmoutFile());
			  		pClientReq->ulPageType = LOGINPAGEREQUEST;
		  	      }

		 	      if(UAMUser.bIsSessTmOut == TRUE)
		 	      {
			 		sprintf(pClientReq->ucUrl, "/%s", UAM_GetDefSessTmoutFile());
			 		pClientReq->ulPageType = LOGINPAGEREQUEST;
		 	      }
		     }
	     }
	     else
	     #endif
	     {
		       sprintf((char *)pClientReq->ucUrl, "/%s",
                     Httpd_GetLoginFile(pHttpGlobals));  /* HTTP_DEF_LOGIN_FILE */
		       Http_ProcessRegModuleRequest(pHttpGlobals,pClientReq);
		       pClientReq->ulPageType = LOGINPAGEREQUEST;
	      }
  }

  /* Gen-Cookie */
  pNextCookie = (unsigned char *) of_strstr((char *)pCookieHdr, "Name-");

  while (pNextCookie != NULL)
  {
    if (pHttpGlobals->pFreeGetCookieInfo != NULL)
    {
       pGetTemp           = pHttpGlobals->pFreeGetCookieInfo;
       if(!pGetTemp->CookieBuff)
      	{
	       pGetTemp->CookieBuff=of_calloc(1,iCookieMaxBufferLen+1);
	       if (!pGetTemp->CookieBuff)
	       {
                 if(ucParsedCookieVal) 
                 { 
                	 of_free(ucParsedCookieVal); 
                 } 
	          return OF_FAILURE;
	       }
      	}
       pHttpGlobals->pFreeGetCookieInfo = pHttpGlobals->pFreeGetCookieInfo->pNext;
       pGetTemp->pNext    = NULL;
       pGetTemp->bHeep    = 1;
    }
    else
    {
      pGetTemp =  (HttpdGetCookieInfo_t *)
                        of_calloc(1, sizeof(HttpdGetCookieInfo_t));        
       if (!pGetTemp)
      {
         if(ucParsedCookieVal) 
         { 
             of_free(ucParsedCookieVal); 
         } 
          return OF_FAILURE;
      }
      if(!pGetTemp->CookieBuff)
      	{
	      pGetTemp->CookieBuff=of_calloc(1,iCookieMaxBufferLen+1);    
	      if (!pGetTemp->CookieBuff)
	      {
      	         if(ucParsedCookieVal) 
    	         { 
    	              of_free(ucParsedCookieVal); 
    	         } 
	         of_free(pGetTemp);			   
	          return OF_FAILURE;
	      }
      	}
      pGetTemp->bHeep = 0;
    }

    start = pNextCookie;

    /**
     * Don't change to pNextCookie+1 -> pNextCookie..we are looking for
     * next cookie.
     */
    endP = (unsigned char *)of_strstr((char *)pNextCookie+1, "Name-");

    pNextCookie = endP;

    /**
     * Cookie Buff = Complete cookie.
     */
    if (endP)
    {
      if ((endP-start) > iCookieMaxBufferLen)
      {
             if(pGetTemp->bHeep == 0)
           	{
                     if(pGetTemp->CookieBuff)
                      {
               			  of_free(pGetTemp->CookieBuff);
               			  pGetTemp->CookieBuff=NULL;
                      }
                      of_free(pGetTemp);                
		        if(ucParsedCookieVal) 
                      {  
                      	 of_free(ucParsedCookieVal); 
                      } 
                      return OF_FAILURE;
           	}
             else
           	{
                        if(pGetTemp->CookieBuff)
      			    {
           				of_free(pGetTemp->CookieBuff);
   					 pGetTemp->CookieBuff=NULL;
      			    } 
                          if(ucParsedCookieVal) 
                          { 
                                  of_free(ucParsedCookieVal); 
                          } 
	                   return OF_FAILURE;
           	}
      }

      of_strncpy((char *) pGetTemp->CookieBuff, (char *) start, endP-start);
      pGetTemp->CookieBuff[endP-start] = '\0';
    }
    else
    {
      if (of_strlen((char *) start) > iCookieMaxBufferLen)
      {
            if(pGetTemp->bHeep == 0)
            {
           	       if(pGetTemp->CookieBuff)
                     {
                       of_free(pGetTemp->CookieBuff);
                        pGetTemp->CookieBuff=NULL;
                     }
                     of_free(pGetTemp);                  
                     if(ucParsedCookieVal) 
                     { 
                             of_free(ucParsedCookieVal); 
                     } 
                     return OF_FAILURE;
            }
           else
           {
                   if(pGetTemp->CookieBuff)
                   {
                        of_free(pGetTemp->CookieBuff);
                         pGetTemp->CookieBuff=NULL;
                   }
                   if(ucParsedCookieVal) 
                   { 
                          of_free(ucParsedCookieVal); 
                   } 
                   return OF_FAILURE;
           }
      }

      of_strcpy((char *) pGetTemp->CookieBuff, (char *) start);
    }

    /**
     * Cookie Name.
     */
    start = (unsigned char *) of_strstr((char *) pGetTemp->CookieBuff, "Name-");
    start += of_strlen("Name-");
    pGetTemp->pCookieName = start;
    endP = (unsigned char *) of_strchr((char *) start, '=');

    /**
     * Cookie value.
     */
    if (endP)
    {
      start = endP+1;
      pGetTemp->value_p = start;
    }

    //endP = of_strchr(start, ';'); CID 572 

    if (pClientReq->pGetCookieHead == NULL)
    {
      pClientReq->pGetCookieHead = pGetTemp;
    }
    else
    {
      pGetTemp->pNext            = pClientReq->pGetCookieHead;
      pClientReq->pGetCookieHead = pGetTemp;
    }

    /**
     * Cookie Path is optional.
     */
    start = (unsigned char *) of_strstr((char *) pGetTemp->CookieBuff, "path=");

    if (start!=NULL)
    {
      if ((pNextCookie != NULL) &&
          (start>pNextCookie))
      {
        continue;
      }
      start += of_strlen("path=");
      pGetTemp->pPath = start;
    }
  }
#ifdef INTOTO_UAM_SUPPORT
 if(iRetVal==OF_SUCCESS)
 {
         IGWUAMDelUserFromLogOffDbAPI(&UAMUser);
 }
#endif
 if(ucParsedCookieVal) 
 { 
	 of_free(ucParsedCookieVal); 
 } 
 return OF_SUCCESS;
} /* HttpClient_ProcessCookie() */

/**************************************************************************
 * Function Name : HttpClient_ParseRequest                                *
 * Description   : This parses the request string(first line + header)    *
 * Input         : pClientReq (I) - pointer to the ClientReq structure.   *
 *                 pHttpGlobals  - Global pointer                         *
 * Output        : None                                                   *
 * Return value  : OF_SUCCESS on success                                   *
 *                 OF_FAILURE on failure                                   *
 **************************************************************************/


int32_t HttpClient_ParseRequest(HttpGlobals_t *pHttpGlobals,HttpClientReq_t  *pClientReq )
{
  unsigned char  *pPrev, *pCur;

  /**
   * Validate the passed parameter(s).
   */
   if(!pHttpGlobals)
   {
     Httpd_Error("pHttpGlobals is NULL\r\n");	
     return OF_FAILURE;
   }
  
  if (!pClientReq)
  {
    return OF_FAILURE;
  }

  pPrev = pCur = pClientReq->ucReqStr;

  if (!of_strcmp((char *) pCur, ""))
  {
    return OF_FAILURE;
  }

  pCur = (unsigned char *) of_strstr((char *) pCur, CRLF);

  if (pCur) *pCur = '\0';

  if (HttpClient_ParseRequestLine(pClientReq, pPrev) != OF_SUCCESS)
  {
    return OF_FAILURE;
  }

  /**
   * Single line request.
   */
  if (!pCur)
  {
    if (pClientReq->ucUrl[0] == '/')
    {
      of_strncpy((char *) pClientReq->ucFileName, (char *) pClientReq->ucUrl + 1,
                HTTP_FILENAME_LEN);
    }
    else
    {
      of_strcpy((char *) pClientReq->ucFileName, "");
    }
    pClientReq->ucFileName[HTTP_FILENAME_LEN] = '\0';
    return OF_SUCCESS;
  }
  do
  {
    pCur  += of_strlen(CRLF);
    pPrev  = pCur;
    pCur   = (unsigned char *) of_strstr((char *) pCur, CRLF);

    if (pCur)
    {
      *pCur = '\0';
    }

    if (HttpClient_ParseHeaderLine(pHttpGlobals,pClientReq, pPrev) != OF_SUCCESS)
    {
      return OF_FAILURE;
    }
  } while (pCur);
#ifdef INTOTO_UAM_SUPPORT
  if(pClientReq->ulPageType == LOGINPAGEREQUEST)
  {
   if(Http_ProcessRegModuleRequest(pHttpGlobals,pClientReq) != OF_SUCCESS)
   {
    Httpd_Error("Http_ProcessRegModuleRequest is Failed\r\n");
   } 
  }
#endif /*INTOTO_UAM_SUPPORT*/
  if (pClientReq->ucUrl[0] == '/')
  {
    of_strncpy((char *) pClientReq->ucFileName, (char *) pClientReq->ucUrl + 1,
              HTTP_FILENAME_LEN);
  }
  else
  {
    of_strcpy((char *) pClientReq->ucFileName, "");
  }

  pClientReq->ucFileName[HTTP_FILENAME_LEN] = '\0';

  return OF_SUCCESS;
} /* HttpClient_ParseRequest() */

/**************************************************************************
 * Function Name : HttpClient_ReadReq_ReadEvent                           *
 * Description   : This is the state handler for 'ReadReq'. This checks   *
 *                 for the completion of request string. If completed, it *
 *                 changes the current state to appropriate state         *
 *                 depending on type of method.                           *
 * Input         : pClientReq (I) - pointer to the ClientReq structure.   *
 * Output        : None                                                   *
 * Return value  : OF_SUCCESS on success                                   *
 *                 OF_FAILURE on failure                                   *
 **************************************************************************/
int32_t HttpClient_ReadReq_ReadEvent( HttpClientReq_t  *pClientReq )
{
       return OF_FAILURE;
} /* HttpClient_ReadReq_ReadEvent() */

/***********************************************************************
 * Function Name : HttpClient_ReadPStr_ReadEvent                       *
 * Description   : This function is the state handler for              *
 *                 'Read Post String'. This checks for the completion  *
 *                 of post string. If complete post string is read it  *
 *                 changes the state to 'Send Response'                *
 * Input         : pClientReq - Client Request                         *
 *                 pHttpGlobals  - Global pointer                      *
 * Output        : None                                                *
 * Return value  : OF_SUCCESS on success                                *
 *                 OF_FAILURE on failure                                *
 ***********************************************************************/
int32_t HttpClient_ReadPStr_ReadEvent(HttpGlobals_t  *pHttpGlobals,HttpClientReq_t  *pClientReq )
{
  unsigned char  *pEop=NULL;
  uint32_t  ulLen=0;

#ifdef HTTPD_DEBUG
  Httpd_Print(", Post-String");
#endif

  /**
   * Validate the passed parameter(s).
   */
   if(!pHttpGlobals)
  {
    Httpd_Error("pHttpGlobals is NULL\r\n");	
    return OF_FAILURE;
  }
  if (!pClientReq)
   {
        HttpDebug("pClient Request is NULL");
        return OF_FAILURE;
   }

  if (pClientReq->lInContentLen < 0)
  {
#ifdef HTTPD_DEBUG
    Httpd_Print("(EOL-");
#endif
    pEop = (unsigned char *) of_strstr((char *) pClientReq->ucPostString, CRLF);

    if (!pEop)
    {
#ifdef HTTPD_DEBUG
      Httpd_Print("Incomplete)");
#endif
      /**
       * Not yet received full post string.
       */
      pClientReq->bNeedMoreData = TRUE; 
      return OF_SUCCESS;
    }
#ifdef HTTPD_DEBUG
    Httpd_Print("Complete)");
#endif

    *pEop = '\0';
    pClientReq->eState = hStateSendResp;
    return OF_SUCCESS;
  }

  ulLen = pClientReq->lOutContentLen;

#ifdef HTTPD_DEBUG
  Httpd_Print("(%ld/%d)", ulLen, pClientReq->lInContentLen);
#endif

/**  
 *We are Not supporting more than (4k) HTTP_POSTSTR_LEN size
 */
if(ulLen  >= HTTP_POSTSTR_LEN)
{
   of_strcpy((char *) pClientReq->ucFileName,HTTP_ERRORSCREEN_FILE);	   
   of_strcpy((char *) pClientReq->ucAppData,"Post string length exceeded the limit.");
   /*Here The Method is Changed to avoid Post String Process*/
   pClientReq->eMethod=hMethodGet;
   /*Send the Responce immediatly with the error message*/
   pClientReq->eState = hStateSendResp;
   return OF_SUCCESS;
}  
else if (ulLen < pClientReq->lInContentLen)
{
    /**
     * Not yet received full post string.
     */
    pClientReq->bNeedMoreData = TRUE;
   /* COMMENTED THE BELOW LINE FOR BR 39811  */
   /* pClientReq->lOutContentLen = pClientReq->lInContentLen; 
    */
    return OF_SUCCESS;
  }

  /**
   * End with NULL to have correct length.
   */
  pClientReq->ucPostString[pClientReq->lInContentLen] = '\0';
  pClientReq->eState = hStateSendResp;
  pClientReq->lOutContentLen = -1;

  return OF_SUCCESS;
} /* HttpClient_ReadPStr_ReadEvent() */

/**************************************************************************
 * Function Name : HttpClient_GetMPartBoundary                            *
 * Description   : This returns multi-part boundary string pointer.       *
 *                 Boundary comes in the value of request header field    *
 *                 'Content-Type'                                         *
 *      Content-Type=multi-part/form-data; boundary=----------herf34u2    *
 * Input         : pClientReq - pointer to the ClientReq structure.       *
 * Output        : None                                                   *
 * Return value  : Pointer to boundary string (read-only)                 *
 *                 NULL on failure                                        *
 **************************************************************************/


unsigned char* HttpClient_GetMPartBoundary( HttpClientReq_t  *pClientReq )
{
  unsigned char  *pTemp=NULL;

  /**
   * Validate the passed parameter(s).
   */
  if (!pClientReq)
  {
    return NULL;
  }

  pTemp = (unsigned char *) of_strstr((char *) pClientReq->ucInContentType, "boundary=");

  if (!pTemp)
  {
    return pTemp;
  }

  pTemp += of_strlen("boundary=");

  return pTemp;
} /* HttpClient_GetMPartBoundary() */

/**************************************************************************
 * Function Name : Httpd_Enable                                           *
 * Description   : This function is used to enable the server using the   *
 *                 given type.                                            *
 * Input         : Type (I) - type of the server to be disabled.          *  
 *                 pHttpGlobals  - Global pointer                         * 
 * Output        : None.                                                  *
 * Return value  : OF_SUCCESS on success, otherwise OF_FAILURE.             *
 **************************************************************************/
/**/
int32_t Httpd_Enable( HttpGlobals_t *pHttpGlobals,hServerType_e  Type )
{
  uint32_t  ulIndex = 0;
  
  if(!pHttpGlobals)
  {
    Httpd_Error("pHttpGlobals is NULL\r\n");	
    return OF_FAILURE;
  }
  
  for (ulIndex = 0; ulIndex < pHttpGlobals->iMaxServers; ulIndex++)
  {
    if (pHttpGlobals->HttpServers[ulIndex].Type == Type)
    {
      pHttpGlobals->HttpServers[ulIndex].bEnabled = TRUE;
    }
  }
  return OF_SUCCESS;
} /* Httpd_Enable() */

/**************************************************************************
 * Function Name : Httpd_Disable                                          *
 * Description   : This function is used to disable the server using the  *
 *                 given type.                                            *
 * Input         : Type (I) - type of the server to be disabled.          *
 *                 pHttpGlobals  - Global pointer                         *
 * Output        : None.                                                  *
 * Return value  : OF_SUCCESS on success, otherwise OF_FAILURE.             *
 **************************************************************************/
/**/
int32_t Httpd_Disable(HttpGlobals_t *pHttpGlobals,hServerType_e  Type )
{
  uint32_t  ulIndex = 0;
  
   if(!pHttpGlobals)
  {
    Httpd_Error("pHttpGlobals is NULL\r\n");	
    return OF_FAILURE;
  }
 
  for (ulIndex = 0; ulIndex < pHttpGlobals->iMaxServers; ulIndex++)
  {
    if (pHttpGlobals->HttpServers[ulIndex].Type == Type)
    {
      pHttpGlobals->HttpServers[ulIndex].bEnabled = FALSE;
    }
  }
  return OF_SUCCESS;
} /* Httpd_Disable() */

/**************************************************************************
 * Function Name : HttpRequest_GetMPartBoundary                           *
 * Description   : This is the API to be used to get boundary string of   *
 *                 multi-part posting                                     *
 * Input         : pHttpRequest (I) - pointer to the HttpRequest structure*
 * Output        : None.                                                  *
 * Return value  : pointer to boundary string (read-only), NULL otherwise.*
 **************************************************************************/

unsigned char *HttpRequest_GetMPartBoundary( HttpRequest  *pHttpRequest )
{
  return HttpClient_GetMPartBoundary((HttpClientReq_t*)
                                          pHttpRequest->pPvtData);
} /* HttpRequest_GetMPartBoundary() */

/**************************************************************************
 * Function Name : HttpRequest_GetMPartData                               *
 * Description   : This function is used to get the multi-part data.      *
 * Input         : pHttpRequest (I) - pointer to the HttpRequest structure*
 * Output        : None.                                                  *
 * Return value  : pointer to multi-part data (read-only), NULL otherwise.*
 **************************************************************************/

unsigned char *HttpRequest_GetMPartData( HttpRequest  *pHttpRequest )
{
  HttpClientReq_t  *pClientReq = NULL;

  /**
   * Validate the passed parameter(s).
   */
  if (!pHttpRequest)
  {
    return NULL;
  }

  /**
   * Get the client request from the passed http request parameter and
   * validate it.
   */
  pClientReq = (HttpClientReq_t *) pHttpRequest->pPvtData;

  if (!pClientReq)
  {
    return NULL;
  }

  if (!pClientReq->pMPart)
  {
    return NULL;
  }

  return pClientReq->pMPart->ucBuf;
} /* HttpRequest_GetMPartData() */

/**************************************************************************
 * Function Name : HttpRequest_GetMPartDataLen                            *
 * Description   : This function is used to get the multi-part data length*
 * Input         : pClientReq (I) - pointer to the ClientReq structure.   *
 * Output        : None.                                                  *
 * Return value  : OF_SUCCESS on success, otherwise OF_FAILURE.             *
 **************************************************************************/

int32_t HttpRequest_GetMPartDataLen( HttpRequest  *pHttpRequest )
{
  HttpClientReq_t  *pClientReq = NULL;

  /**
   * Validate the passed parameter(s).
   */
  if (!pHttpRequest)
  {
    return OF_FAILURE;
  }

  /**
   * Get the client request from the passed http request parameter and
   * validate it.
   */
  pClientReq = (HttpClientReq_t *) pHttpRequest->pPvtData;

  if (!pClientReq)
  {
    return OF_FAILURE;
  }

  if (!pClientReq->pMPart)
  {
    return OF_FAILURE;
  }

  return pClientReq->lInContentLen;
} /* HttpRequest_GetMPartDataLen() */

/**************************************************************************
 * Function Name : HttpClient_IsMultiPart                                 *
 * Description   : This function tells whether the current posting is of  *
 *                 multi-part or not.                                     *
 * Input         : pClientReq (I) - pointer to the ClientReq structure.   *
 * Output        : None.                                                  *
 * Return value  : TRUE if it is multi-part posting FALSE otherwise.      *
 **************************************************************************/


bool HttpClient_IsMultiPart( HttpClientReq_t  *pClientReq )
{
  /**
   * Validate the passed parameter(s).
   */
  if (!pClientReq)
  {
    return FALSE;
  }

  if (of_strstr((char *) pClientReq->ucInContentType, "multipart/form-data"))
  {
    return TRUE;
  }
  else
  {
    return FALSE;
  }
} /* HttpClient_IsMultiPart() */

/**************************************************************************
 * Function Name : HttpRequest_IsMultiPart                                *
 * Description   : API That tells whether the current posting is of multi-*
 *                 part or not.                                           *
 * Input         : pHttpRequest (I) - pointer to the HttpRequest structure*
 * Output        : None.                                                  *
 * Return value  : TRUE if it is multi-part posting FALSE otherwise.      *
 **************************************************************************/

bool HttpRequest_IsMultiPart( HttpRequest  *pHttpRequest )
{
  return HttpClient_IsMultiPart((HttpClientReq_t *)pHttpRequest->pPvtData);
} /* HttpRequest_IsMultiPart() */

/**************************************************************************
 * Function Name : HttpRequest_SendWelcomePage                            *
 * Description   : This function is used to send the starting page on the *
 *                 successful login. For the https enabled build, if the  *
 *                 certificates are not loaded into the server then the   *
 *                 certificates page will be sent, otherwise welcome page *
 *                 will be sent.                                          *
 * Input         : pHttpRequest (I) - pointer to the HttpRequest structure*
 *                                    used to send the relevant page.     *
 *                 pHttpGlobals  - Global pointer                         *
 * Output        : Welcome page or certificates page will be sent.        *
 * Return value  : OF_SUCCESS on success, otherwise OF_FAILURE.             *
 **************************************************************************/

int32_t HttpRequest_SendWelcomePage(HttpGlobals_t  *pHttpGlobals,HttpRequest  *pHttpRequest )
{
#if defined(HTTPS_ENABLE)
  HttpClientReq_t  *pClientReq = NULL;

  /**
   * Validate the passed parameter(s).
   */
  if(!pHttpGlobals)
  {
    Httpd_Error("pHttpGlobals is NULL\r\n");	
    return OF_FAILURE;
  }

  if (!pHttpRequest)
  {
    return OF_FAILURE;
  }

  /**
   * Get the client request from the passed http request parameter and
   * validate it.
   */
  pClientReq = (HttpClientReq_t *) pHttpRequest->pPvtData;

  if (!pClientReq)
  {
    return OF_FAILURE;
  }

#ifdef HTTP_REDIRECT_HTTPS
  if (pHttpGlobals->HttpServers[pClientReq->lServId].Type == hServerNormal)
  {
    /**
     * Send the certificates page in case of the https enabled build and
     * when certificates are not uploaded to the server.
     */
    if (HttpSendHtmlFile(pHttpRequest, Httpd_OnlyCerts_GetIndexFile(),
                         NULL) == OF_FAILURE)
    {
      return OF_FAILURE;
    }
    return OF_SUCCESS;
  }
#endif /* HTTP_REDIRECT_HTTPS */
#endif

  /**
   * Send the welcome page.
   */
  if (HttpSendHtmlFile(pHttpRequest,  Httpd_GetIndexFile(pHttpGlobals),
                       NULL) == OF_FAILURE)
  {
    return OF_FAILURE;
  }
  return OF_SUCCESS;
} /* HttpRequest_SendWelcomePage() */
/**************************************************************************
 * Function Name : HttpClient_ReadMPart_ReadEvent                         *
 * Description   : This is the state handler for 'Read MultiPart Data'.   *
 *                 This checks for the completion of data. If completed,  *
 *                 it changes the state to 'Send Response'                *
 * Input         : pClientReq (I) - pointer to the ClientReq structure.   *
 * Output        : None.                                                  *
 * Return value  : OF_SUCCESS on success and OF_FAILURE, otherwise.         *
 **************************************************************************/
int32_t HttpClient_ReadMPart_ReadEvent( HttpClientReq_t  *pClientReq )
{
  uint32_t  ulLen;

  /**
   * Validate the passed parameter(s).
   */
  if ((!pClientReq) ||
      (!pClientReq->pMPart))
  {
    return OF_FAILURE;
  }

  ulLen = pClientReq->ReadInfo.pCur - pClientReq->ReadInfo.pBuf;

  if (ulLen < pClientReq->lInContentLen)
  {
    /**
     * Not yet received total content.
     */
    pClientReq->bNeedMoreData = TRUE; 
    return OF_SUCCESS;
  }

  pClientReq->eState = hStateSendResp;
  return OF_SUCCESS;
} /* HttpClient_ReadMPart_ReadEvent() */

/**************************************************************************
 * Function Name : HttpClient_Process                                     *
 * Description   : This function is called whenever there is incoming data*
 *                 from the client. Depending on current state it calls   *
 *                 respective handlers and if there is any change of state*
 *                 it continues processing. If there is no change of state*
 *                 it returns success. Client context deleted when the    *
 *                 state becomes hStateFinished.                          *
 * Input         : pClientReq (I) - pointer to the ClientReq structure. 
 *                 pHttpGlobals  - Global pointer                         * 
 * Output        : None.                                                  *
 * Return value  : OF_SUCCESS on success and OF_FAILURE, otherwise.         *
 **************************************************************************/


int32_t HttpClient_Process(HttpGlobals_t *pHttpGlobals,HttpClientReq_t  *pClientReq )
{
 int32_t       lBytes=0;
 HttpServer_t  *pServer=NULL;  
 unsigned char   *pBuf=NULL; 
 unsigned char   *pPrev=NULL;
 unsigned char   *pCur=NULL;  
 int32_t         iIndex=0;
 int32_t         iResult=0,iRetval;
 unsigned char    *pCrlf=NULL;   
 
 if (pClientReq==NULL)
 {
    HttpDebug("pClientReq is null\n\r");
    return OF_FAILURE;
  }

 pServer = &pHttpGlobals->HttpServers[pClientReq->lServId];

 if (pClientReq->ReadInfo.pCur >=
          (pClientReq->ReadInfo.pBuf + pClientReq->ReadInfo.ulMaxLen))
 {
    if(HttpClient_Delete(pHttpGlobals,pClientReq)!=OF_SUCCESS)
    {
         HttpDebug("Client Delete is returned failure");        
    }
    Httpd_Debug("Request String Is Very Long!");    
    return OF_FAILURE;
 }
 iRetval = pServer->ClientRead(pClientReq, pClientReq->ReadInfo.pCur,
                                ((pClientReq->ReadInfo.pBuf +
                                  pClientReq->ReadInfo.ulMaxLen) -
                                 pClientReq->ReadInfo.pCur), &lBytes);
  if((iRetval != OF_SUCCESS) && (iRetval != HTTPS_SSL_READ_PENDING))
  {
        if(HttpClient_Delete(pHttpGlobals,pClientReq)!=OF_SUCCESS)
        {
           HttpDebug("Client Delete is returned failure");     	    
        }
        Httpd_Debug("Unable To Read From Socket!");
        return OF_FAILURE;
  }
   if (lBytes == 0)
  {  
        HttpDebug("lbytes is null");
        if(iRetval == HTTPS_SSL_READ_PENDING)
          return HTTPS_SSL_READ_PENDING;
        else
        return OF_SUCCESS;
  }
  pClientReq->ReadInfo.pCur  += lBytes;
  pClientReq->lOutContentLen += lBytes; /* ADDED THIS LINE FOR BR 39811 */
  #ifdef HTTPD_DEBUG
   Httpd_Print(", Read(%d)", lBytes);
  #endif     
  pCur=of_calloc(1,HTTP_REQSTR_LEN+1);
  if(pCur==NULL)
  {
       HttpDebug("unable to allocate memory for pCur");
      if(HttpClient_Delete(pHttpGlobals,pClientReq)!=OF_SUCCESS)
      {
        HttpDebug("Client Delete is returned failure");        
      }   
       return OF_FAILURE;
  }
  of_strcpy((char *) pCur,(char *) pClientReq->ucReqStr);  
  pPrev=pCur;   

  /**
    *we are checking whether we are already received half the packet
    *i.e. 1/2A already received ,now we got  1/2A+B     
    **/     
   if((pClientReq->bIsHalfPacketRecv==TRUE) && (pClientReq->ucRemainBuffer))
  {
      of_strcat((char *) pClientReq->ucRemainBuffer,(char *) pPrev);
      of_strcpy((char *) pClientReq->ucReqStr,(char *) pClientReq->ucRemainBuffer);
      pClientReq->bIsHalfPacketRecv=FALSE;
  }
  /*     ***  COMMENTED OUT FOR BR 39811
  else
   {
      of_strcpy(pClientReq->ucReqStr,pPrev);
   }
   */  
   /*start processing first packet*/  
  if((iResult=HttpClient_StartProcessing(pHttpGlobals,pClientReq))!=OF_SUCCESS)
   {
         if( iResult== OF_FAILURE)
	  {
	        of_free(pCur);
	        HttpDebug("HttpClient_StartProcessing failure");                		
               return OF_FAILURE;	      
	  } 
         else if( iResult== HTTP_DONOT_SEND_RESPONCE)
 	  {
 	         of_free(pCur);  
	         HttpDebug("HTTP_DONOT_SEND_RESPONCE");
                return HTTP_DONOT_SEND_RESPONCE;
 	  }               
   }

#if 0 //CID 62 
  if(pClientReq == NULL)
  {
     of_free(pCur);
     Httpd_Error("pClient Req is NULL \r\n"); 
     return OF_FAILURE;
  }
#endif

  if(pClientReq->eMethod == hMethodPost || pClientReq->eVersion == hVersion10)
  {
      /*For All POST method or HTTP/1.0 requests we cannot expect pipelied requests so return from here*/   
       of_free(pCur);
      if(iRetval == HTTPS_SSL_READ_PENDING)
         return HTTPS_SSL_READ_PENDING;
      else
       return OF_SUCCESS;
  }
  pCur = (unsigned char *) of_strstr((char *) pCur, CRLF_CRLF);
  iIndex=pClientReq->iReqTableIndex;
  while(pCur!=NULL)
  {   
  	pCur  += of_strlen(CRLF_CRLF);
     	pBuf=pCur;     	
       pCur  = (unsigned char *) of_strstr((char *) pCur, CRLF_CRLF);         
       if(pCur=='\0')
  	{
      	    pCur=pBuf;/*we moved the pointer to the previous position*/
    		 
    	   /**
       * Search for atleast single header field CRLF(\r\n)
       * we may got the partial request on this connection
       */	 
    	    pCrlf=(unsigned char *) of_strstr((char *) pCur,CRLF);
    	    if(pCrlf == NULL)	
    	    {	     
      	      /*No more requests are  found on our connection*/
    	      pCur=pPrev;	/*We moved the pointer in the beginning*/	     
    	      of_free(pCur);	
              if(iRetval == HTTPS_SSL_READ_PENDING)
                return HTTPS_SSL_READ_PENDING;
              else
                return OF_SUCCESS;
    	    }
    	    else
    	    {
    	      /*We got the partial request*/
    	      pBuf=pCur; /*this line is duplicate assignment*/		      
    	    }	    
  	}
     	if(of_strcmp((char *) pBuf," "))
     	{    
   	      HttpClientReq_t  *pSubClientReq=NULL;   	    
   	      pSubClientReq=pHttpGlobals->pHttpReqTable[iIndex];
             of_strcpy((char *) pSubClientReq->ucReqStr,(char *) pBuf);
             /**
               * Read and process each request found on our connection
	        * until no requests are left or we decide to close.
	        **/	     
             if((iResult=HttpClient_StartProcessing(pHttpGlobals,pSubClientReq))!=OF_SUCCESS)
             {
		    if( iResult== OF_FAILURE)
	   	    {
	   	           HttpDebug("HttpClient_StartProcessing failure");
	                  if(HttpClient_Delete(pHttpGlobals,pClientReq)!=OF_SUCCESS)
		    	    {
		 		  HttpDebug("Client Delete is returned failure");		 	         
		 	    }
			    pCur=pPrev;
		           of_free(pCur);
	                  return OF_FAILURE;		         
		    } 
		    else if( iResult== HTTP_DONOT_SEND_RESPONCE)
		    {		
		           pCur=pPrev;
		           of_free(pCur);
	                  HttpDebug("HTTP_DONOT_SEND_RESPONCE");
		           return HTTP_DONOT_SEND_RESPONCE;
		    }
             }
     	      if(pSubClientReq == NULL)
	      {
	            pCur=pPrev;
  	            of_free(pCur);   
		     Httpd_Error("pSubClientReq is NULL \r\n"); 
		     return OF_FAILURE;
		}
             if(pSubClientReq->ucRemainBuffer)
             	{
             	         pCur=pPrev;
             	         of_free(pCur);
              	  pSubClientReq->bIsHalfPacketRecv=TRUE;
                   if(iRetval == HTTPS_SSL_READ_PENDING)
                     return HTTPS_SSL_READ_PENDING;
                   else
              	     return OF_SUCCESS;
             	}
             iIndex=pSubClientReq->iReqTableIndex;
     	 }   
   } 
  if(pPrev)
  {
     pCur=pPrev;
     of_free(pCur);
  }
  if(iRetval == HTTPS_SSL_READ_PENDING)
    return HTTPS_SSL_READ_PENDING;
  else
    return OF_SUCCESS;
} /* HttpClient_Process() */

 int32_t IGWHttpdGetEphermalPort(
                                           HttpGlobals_t  *pHttpGlobals,
                                           HttpPortInfo_t *PortInfo,
                                           uint32_t       ulIPAddr,
                                           cm_sock_handle  *pSockFd,
                                           uint16_t       *pucPortNo
                                          )
{
  int32_t   lRetVal;
  int32_t   iIndex;  
#ifdef OF_IPV6_SUPPORT
  cm_sock_handle    Ipv6SFd = -1;
  struct cm_ip_addr      Ipv6IpAddr={};
#else
  cm_sock_handle    SFd = -1;
  /*struct cm_ip_addr      Ipv4IpAddr={}; */
  ipaddr      Ipv4IpAddr;
#endif /*INTOTO_IPV6_SUPPORT*/

#ifdef OF_IPV6_SUPPORT
#ifndef OF_CM_SUPPORT
    Ipv6SFd = cm_socket_create(CM_IPPROTO_TCP);
    if(Ipv6SFd < 0)
    {
      HttpDebug(("Unable to create the socket\n\r"));
      return HTTP_ERR_SOCKET_CREATE_FAIL;
    }
    
    /* cm_set_sock_reuse_opt(Ipv6SFd); */
    Ipv6IpAddr.ip_type_ul=UCM_INET_IPV6;
    if (cm_socket_bind (Ipv6SFd,Ipv6IpAddr,INADDR_ANY) == OF_FAILURE)
    {
      HttpDebug(("Unable to bind the socket\n\r"));
      cm_socket_close (Ipv6SFd);
      return HTTP_ERR_BIND_FAIL;
    }
    
    lRetVal = cm_socket_get_sock_name(Ipv6SFd,&ulIPAddr,pucPortNo);
    if( lRetVal == OF_SUCCESS )
    {
      *pSockFd = Ipv6SFd;
      if (cm_socket_listen(Ipv6SFd, pHttpGlobals->HttpdConfig.ulMaxPendReqs) != OF_SUCCESS)
      {
        HttpDebug(("Unable To Listen\r\n"));
        cm_socket_close(Ipv6SFd);
        return HTTP_ERR_LISTEN_FAIL;
      }
    }
    else
    {
      HttpDebug(("Unable to bind the socket\n\r"));
      cm_socket_close (Ipv6SFd);
      return HTTP_ERR_PORT_NOT_FOUND;
    }
#endif /*OF_CM_SUPPORT*/
#else  
    //Ipv4IpAddr.ip_type_ul=cm_inet_ipv4; // for UCM
    Ipv4IpAddr=HTTPD_INADDR_ANY;
    /*SFd = cm_socket_create(CM_IPPROTO_TCP,Ipv4IpAddr.ip_type_ul);*/
    SFd = cm_socket_create(CM_IPPROTO_TCP);
    if(SFd < 0)
    {
      HttpDebug(("Unable to create the socket\n\r"));
      return HTTP_ERR_SOCKET_CREATE_FAIL;
    }
    cm_set_sock_reuse_opt(SFd);
    if (cm_socket_bind (SFd,Ipv4IpAddr, HTTP_LCL_INADDR_ANY) == OF_FAILURE)
    {
      HttpDebug(("Unable to bind the socket\n\r"));
      cm_socket_close (SFd);
      return HTTP_ERR_BIND_FAIL;
    }
    
    lRetVal = cm_socket_get_sock_name(SFd,&ulIPAddr,pucPortNo);
    if( lRetVal == OF_SUCCESS )
    {
      *pSockFd = SFd;
      if (cm_socket_listen(SFd, pHttpGlobals->HttpdConfig.ulMaxPendReqs) != OF_SUCCESS)
      {
        HttpDebug(("Unable To Listen\r\n"));
        cm_socket_close(SFd);
        return HTTP_ERR_LISTEN_FAIL;
      }
    }
    else
    {
      HttpDebug(("Unable to Get the Port \n\r"));
      cm_socket_close (SFd);
      return HTTP_ERR_PORT_NOT_FOUND;
    }
#endif /*INTOTO_IPV6_SUPPORT*/
    PortInfo->usPort = *pucPortNo;
    PortInfo->bDontSave = TRUE;

#ifndef OF_CM_SUPPORT
    lRetVal = HttpAddPortToList(PortInfo);
    if(lRetVal !=OF_SUCCESS)
    {
      HttpDebug(("Ports already added to database. \n\r"));
      return lRetVal;
    }
#endif /*OF_CM_SUPPORT*/
    
    HttpDebug(("port bind success. runing on port %d\n\r",PortInfo->usPort));
    for(iIndex = 0; iIndex < pHttpGlobals->iMaxServers; iIndex++)
    {
      if((pHttpGlobals->HttpServers[iIndex].SockFd <= 0)&&
         (pHttpGlobals->HttpServers[iIndex].uiPort <= 0))
      {
        pHttpGlobals->HttpServers[iIndex].uiPort         = PortInfo->usPort;
        pHttpGlobals->HttpServers[iIndex].bEnabled       = PortInfo->bEnabled;
        pHttpGlobals->HttpServers[iIndex].Type           = PortInfo->eServerType;
#ifdef OF_IPV6_SUPPORT
        pHttpGlobals->HttpServers[iIndex].SockFd         = Ipv6SFd;
#else      	
        pHttpGlobals->HttpServers[iIndex].SockFd         = SFd;
#endif /*INTOTO_IPV6_SUPPORT*/      	
        if (PortInfo->eServerType == hServerNormal)
        {
          /**
           * Initialize the http related things.
           */
          pHttpGlobals->HttpServers[iIndex].ClientAccept   = HttpDef_ClientAccpet;
          pHttpGlobals->HttpServers[iIndex].ClientRead     = HttpDef_ClientRead;
          pHttpGlobals->HttpServers[iIndex].ClientWrite    = HttpDef_ClientWrite;
          pHttpGlobals->HttpServers[iIndex].ClientClose    = HttpDef_ClientClose;
        }
#ifdef HTTPS_ENABLE
        else if (PortInfo->eServerType == hServerSSL)
        {
          /**
           * Initialize the https related things.
           */
          Https_Init(&pHttpGlobals->HttpServers[iIndex]);
        }
#endif /* HTTPS_ENABLE */
        return OF_SUCCESS;
      }
    }/*end of for loop*/
    return lRetVal;
}

/**************************************************************************
 * Function Name : Httpd_Start                                            *
 * Description   : This function is used to accept the requests coming    *
 *                 from the clients on all servers and will run infinite  *
 *                 loop. This pools on all listening (server) fds along   *
 *                 with current client fds. If there is any read-event on *
 *                 listening fds, it calls Httpd_AcceptClient and if it is*
 *                 client fd it calls HttpClient_Process.                 *
 * Input         : pHttpGlobals  - Global pointer.                        *
 * Output        : None.                                                  *
 * Return value  : OF_SUCCESS on success and OF_FAILURE, otherwise.         *
 **************************************************************************/

int32_t Httpd_Start(HttpGlobals_t *pHttpGlobals)
{
  uint32_t              ulFdIndex=0, ulRdPendFd=0;
  uint32_t              uiIndex=0;
  uint32_t              ulPollTime = 0;
  uint32_t              ulMaxFds=0;
  int32_t               lRetVal=0;
  cm_sock_handle         LpBkSFd=0;
  
  HttpPortInfo_t       PortInfo = {};
  int32_t                FdCount=0;
  int32_t                ReqTableSize=0;
  int32_t 		cProcessRetVal=0;  
  int32_t               iResIndex=0;
  int16_t                iLoop=0; 

  uint16_t usClearPort=0;
  uint16_t usSecurePort=0;
  bool bClearEphermal= FALSE;
  bool bSecureEphermal= FALSE;


  if(!pHttpGlobals)
  {
    Httpd_Error("pHttpGlobals is NULL\r\n");	
    return OF_FAILURE;
  }

  ReqTableSize= Httpd_GetMaxReqTableSize(pHttpGlobals);/*pHttpGlobals->pHttpGlobalConfig->iReqtablesize;*/
  FdCount= Httpd_GetMaxFds(pHttpGlobals);/*pHttpGlobals->pHttpGlobalConfig->iMaxfds;*/

#ifdef INTOTO_CMS_SUPPORT
  /*
  * Create loop back socket and bind it on port HTTP_LOOPBACK_PORT_SRV
  */
  LpBkSFd = HttpCreateLoopBackSocket();

CMSDevLpBkSFd = HttpCMSDevCreateLoopBackSocket();
#endif  /* INTOTO_CMS_SUPPORT */

  if (HttpdListenPorts.usClearPort)
  {
    usClearPort = HttpdListenPorts.usClearPort;
    bClearEphermal =  FALSE;
  }
  else
  {
    HttpGetDefClearListenPort(&usClearPort);
    HttpdListenPorts.usClearPort = usClearPort;
  }
  
  if (HttpdListenPorts.usSecurePort)
  {
    usSecurePort = HttpdListenPorts.usSecurePort;
    bSecureEphermal =  FALSE;
  }
  else
  {
#ifdef OF_HTTPS_SUPPORT
    HttpGetDefSecureListenPort (&usSecurePort);
#endif /*OF_HTTPS_SUPPORT*/
    HttpdListenPorts.usSecurePort = usSecurePort;
  }
 #ifndef OF_CM_SUPPORT 
  /**
   * Load the configuration from config file instead of kernel, if data is present.
   */

  /**
   * Load the configuration from kernel if data is present.
   * This case comes into picture when the Httpd process is restarted.
   * Check whether if the configuration in the kernel is valid.
   */
  lRetVal = HttpGetFirstPort(&PortInfo);
  if(lRetVal == OF_SUCCESS)
  {
    int32_t         iIndex = 0;
    HttpPortArgs_t  PortArgs = {};

    HttpDebug(("Some Ports are already configured. getting the info from kernel\n\r"));
    if(HttpCreateSockAndBind(pHttpGlobals,PortInfo) !=OF_SUCCESS)
    {
      HttpDebug("Unable to Bind on Port \n\r");
      return OF_FAILURE;
    }
    PortArgs.usOldPort = PortInfo.usPort;
    for(iIndex = 1; iIndex < pHttpGlobals->iMaxServers; iIndex++)
    {
      if(HttpGetNextPort(&PortArgs) != OF_SUCCESS)
      {
        break;
      }
      if(HttpCreateSockAndBind(pHttpGlobals,PortArgs.PortInfo) !=OF_SUCCESS)
      {
        HttpDebug("Unable to Bind on Port \n\r");
        return OF_FAILURE;
      }
      PortArgs.usOldPort = PortArgs.PortInfo.usPort;
    }
  }
  else
#endif /*OF_CM_SUPPORT*/
  {

    /*Add default Clear Port and Bind on it */
    PortInfo.usPort=usClearPort;
    PortInfo.bEnabled = TRUE;
    PortInfo.eServerType =hServerNormal;
    PortInfo.bDontSave = TRUE;

    if(HttpCreateSockAndBind(pHttpGlobals,PortInfo) == OF_SUCCESS)
    {
 #ifndef OF_CM_SUPPORT 
      lRetVal = HttpAddPortToList(&PortInfo);
      if(lRetVal !=OF_SUCCESS)
      {
        HttpDebug(("Ports already added to database. getting the info from kernel\n\r"));  	
      }
#endif /*OF_CM_SUPPORT*/
      bClearEphermal= FALSE;
    }
    else
    {
      if(bClearEphermal ==  FALSE)
      {
        HttpDebug(("Unable to bind and add the Clear port to kernal database\n\r"));  	
        return OF_FAILURE;
      }
      else
      {
        HttpdListenPorts.iClearSocketFd=-1;
        HttpdListenPorts.usClearPort=0;

        lRetVal=IGWHttpdGetEphermalPort(pHttpGlobals,
                                        &PortInfo,
                                        HTTP_LCL_INADDR_ANY,
                                        &(HttpdListenPorts.iClearSocketFd),
                                        &(HttpdListenPorts.usClearPort));
        if(lRetVal != OF_SUCCESS )
        {
          HttpDebug("Error in getting an ephermal loopback port\r\n");
          return OF_FAILURE;
        }
      }
    }

    of_memset(&PortInfo,0,sizeof(HttpPortInfo_t));

#ifdef OF_HTTPS_SUPPORT
    /*Add default Secure Port and Bind on it */
    PortInfo.usPort=usSecurePort;
    PortInfo.bEnabled = TRUE;
    PortInfo.eServerType =hServerSSL;
    PortInfo.bDontSave = TRUE;
    if(HttpCreateSockAndBind(pHttpGlobals,PortInfo) == OF_SUCCESS)
    {
#ifndef OF_CM_SUPPORT 
      lRetVal = HttpAddPortToList(&PortInfo);
      if(lRetVal !=OF_SUCCESS)
      {
        HttpDebug(("Ports already added to database. getting the info from kernel\n\r"));  	
      }
#endif /*OF_CM_SUPPORT*/
      bSecureEphermal= FALSE;
    }
    else
    {
      if(bSecureEphermal ==  FALSE)
      {
        HttpDebug(("Unable to bind and add the Secure port to kernal database\n\r"));  	
        return OF_FAILURE;
      }
      else
      {
        HttpdListenPorts.iSecureSocketFd=-1;
        HttpdListenPorts.usSecurePort=0;
        lRetVal=IGWHttpdGetEphermalPort(pHttpGlobals,
                                        &PortInfo,
                                        HTTP_LCL_INADDR_ANY,
                                        &(HttpdListenPorts.iSecureSocketFd),
                                        &(HttpdListenPorts.usSecurePort));
        if(lRetVal != OF_SUCCESS )
        {
          HttpDebug("Error in getting an ephermal loopback port\r\n");
          return OF_FAILURE;
        }
      }
    }
#endif /*OF_HTTPS_SUPPORT*/

  } 
  /*
   * Create loop back socket and bind it on port HTTP_LOOPBACK_PORT_SRV
   */
  LpBkSFd = HttpCreateLoopBackSocket();

  /**
   * Accepts the requests coming from the clients and other drivers
   * registered.
   */
  while (TRUE)
  {
    ulFdIndex = ulMaxFds = 0;

    CM_FDPOLL_ZERO(pHttpGlobals->HttpPollTable, FdCount);  /* HTTP_MAX_FDS */

#ifdef INTOTO_CMS_SUPPORT
    if(CMSDevLpBkSFd > 0)
    {
      pHttpGlobals->HttpPollTable[ulFdIndex].fd_i                = CMSDevLpBkSFd;
      pHttpGlobals->HttpPollTable[ulFdIndex].user_data_p          = NULL;
      pHttpGlobals->HttpPollTable[ulFdIndex].interested_events = CM_FDPOLL_READ;
      ulFdIndex++;
    }
#endif  /* INTOTO_CMS_SUPPORT */
    if(LpBkSFd > 0)
    {
      pHttpGlobals->HttpPollTable[ulFdIndex].fd_i                = LpBkSFd;
      pHttpGlobals->HttpPollTable[ulFdIndex].user_data_p          = NULL;
      pHttpGlobals->HttpPollTable[ulFdIndex].interested_events = CM_FDPOLL_READ;
      ulFdIndex++;
    }

    for (uiIndex = 0; uiIndex < pHttpGlobals->iMaxServers; uiIndex++)
    {
      if(pHttpGlobals->HttpServers[uiIndex].SockFd > 0)
      {
        pHttpGlobals->HttpPollTable[ulFdIndex].fd_i                = pHttpGlobals->HttpServers[uiIndex].SockFd;
        pHttpGlobals->HttpPollTable[ulFdIndex].user_data_p          = NULL;
        pHttpGlobals->HttpPollTable[ulFdIndex].interested_events = CM_FDPOLL_READ;
        ulFdIndex++;
      }
    }

    for (uiIndex = 0; uiIndex < HTTP_MAX_REG_FDS; uiIndex++)
    {
      if(pHttpGlobals->HttpExternRegFds[uiIndex].iSockFd > 0)
      {
        pHttpGlobals->HttpPollTable[ulFdIndex].fd_i = pHttpGlobals->HttpExternRegFds[uiIndex].iSockFd;
        pHttpGlobals->HttpPollTable[ulFdIndex].user_data_p = NULL;
        pHttpGlobals->HttpPollTable[ulFdIndex].interested_events = pHttpGlobals->HttpExternRegFds[uiIndex].uiInterstedEvents;
        ulFdIndex++;
      }
    }

    /**
     * Check Whether Any external (Pseudo driver FDs) are registered. If so
     * add them also in the poll table list. THIS IS VALID IN CASE OF LINUX
     * FOR LISTENING KERNEL EVENTS
     */
    if (pHttpGlobals->HttpdExternFdToPoll[0].ulFd)
    {
      pHttpGlobals->HttpPollTable[ulFdIndex].fd_i       = pHttpGlobals->HttpdExternFdToPoll[0].ulFd;
      pHttpGlobals->HttpPollTable[ulFdIndex].user_data_p = NULL;
      pHttpGlobals->HttpPollTable[ulFdIndex].interested_events = CM_FDPOLL_READ;
      ulFdIndex++;
    }

    if (pHttpGlobals->HttpdExternFdToPoll[1].ulFd)
    {
      pHttpGlobals->HttpPollTable[ulFdIndex].fd_i       = pHttpGlobals->HttpdExternFdToPoll[1].ulFd;
      pHttpGlobals->HttpPollTable[ulFdIndex].user_data_p = NULL;
      pHttpGlobals->HttpPollTable[ulFdIndex].interested_events = CM_FDPOLL_READ;
      ulFdIndex++;

    }
#ifdef INTOTO_CMS_SUPPORT

    /**************NEW FEATURE START************************/
    if (pHttpGlobals->HttpdAdditionalFdToPoll[0].ulFd)
    {
      pHttpGlobals->HttpPollTable[ulFdIndex].fd_i       = pHttpGlobals->HttpdAdditionalFdToPoll[0].ulFd;
      pHttpGlobals->HttpPollTable[ulFdIndex].user_data_p = NULL;
      pHttpGlobals->HttpPollTable[ulFdIndex].interested_events = CM_FDPOLL_READ;
      ulFdIndex++;
    }

    if (pHttpGlobals->HttpdAdditionalFdToPoll[1].ulFd)
    {
      pHttpGlobals->HttpPollTable[ulFdIndex].fd_i       = pHttpGlobals->HttpdAdditionalFdToPoll[1].ulFd;
      pHttpGlobals->HttpPollTable[ulFdIndex].user_data_p = NULL;
      pHttpGlobals->HttpPollTable[ulFdIndex].interested_events = CM_FDPOLL_READ;
      ulFdIndex++;
    }
   /**************NEW FEATURE END************************/
#endif  /* INTOTO_CMS_SUPPORT */
    for (iLoop = 0; iLoop < ReqTableSize; iLoop++) 
    {
       if((!pHttpGlobals->pHttpClientReqTable[iLoop].iSockFd ))
      	 {  
      	      continue;   
        }
   	  iResIndex=iLoop;                       
         for (uiIndex = 0; uiIndex < HTTP_MAX_PIPELINED_REQUESTS; uiIndex++) 
         {
	           if (ulFdIndex >= FdCount)   
	           {
	     	       break;
	           }
	           /*HTTP/1.1 Support*/
	          if(!pHttpGlobals->pHttpClientReqTable[iResIndex].pHttpPipeline[uiIndex])
	           {             
	               continue;
	           }
	           if((pHttpGlobals->pHttpClientReqTable[iResIndex].pHttpPipeline[uiIndex]!=NULL )		    && 
	           	(!(pHttpGlobals->pHttpClientReqTable[iResIndex].pHttpPipeline[uiIndex]->SockFd<=0)))
	           {
	                if(((pHttpGlobals->pHttpClientReqTable[iResIndex].pHttpPipeline[uiIndex])->uiTxBufLen >0) &&			  
	     	   			   ((pHttpGlobals->pHttpClientReqTable[iResIndex].pHttpPipeline[uiIndex])->pTransmitBuffer))
	     	      	   {
             	      	       pHttpGlobals->HttpPollTable[ulFdIndex].fd_i       = (pHttpGlobals->pHttpClientReqTable[iResIndex].pHttpPipeline[uiIndex])->SockFd;
             	      	       pHttpGlobals->HttpPollTable[ulFdIndex].user_data_p = (void*) pHttpGlobals->pHttpClientReqTable[iResIndex].pHttpPipeline[uiIndex];	 
             	      	       pHttpGlobals->HttpPollTable[ulFdIndex].interested_events = CM_FDPOLL_WRITE|CM_FDPOLL_EXCEPTION;	     	      	       
             	      	       ulFdIndex++;	             	      	       
            	      	       break;
	     	      	   }
	            }
	         }
    }
    for (uiIndex = 0; uiIndex < ReqTableSize; uiIndex++) 
    {
      if (ulFdIndex >= FdCount)  
      {
	      break;
      }
      
       /*HTTP/1.0  Support*/
       if (!pHttpGlobals->pHttpReqTable[uiIndex])
       {
         continue;
       }
       if((pHttpGlobals->pHttpReqTable[uiIndex]  !=  NULL )		&& 
       	(!(pHttpGlobals->pHttpReqTable[uiIndex])->SockFd <=0))
       { 
         pHttpGlobals->HttpPollTable[ulFdIndex].fd_i = (pHttpGlobals->pHttpReqTable[uiIndex])->SockFd;
	         pHttpGlobals->HttpPollTable[ulFdIndex].user_data_p = (void*) pHttpGlobals->pHttpReqTable[uiIndex];	                
	         if(((pHttpGlobals->pHttpReqTable[uiIndex])->uiTxBufLen > 0 )  && 				
				 (pHttpGlobals->pHttpReqTable[uiIndex])->pTransmitBuffer) 
	         {
			 /*If there is any data to transmit add write event also*/
	                 pHttpGlobals->HttpPollTable[ulFdIndex].interested_events = CM_FDPOLL_WRITE|CM_FDPOLL_EXCEPTION;
	         }		 
		 else  
	         {
	                /*This is to avoid READ and WRITE event on same FD at a time*/
	                 for(iLoop=0;iLoop<ulFdIndex;iLoop++)
	                 {
	                 	   if((pHttpGlobals->HttpPollTable[iLoop].fd_i) == (pHttpGlobals->HttpPollTable[ulFdIndex].fd_i ))
	                 	   {
		                 	    pHttpGlobals->HttpPollTable[ulFdIndex].fd_i =0;
		                 	    pHttpGlobals->HttpPollTable[ulFdIndex].user_data_p =NULL;
	                 	   }
	                 	   else
	                 	   {
	                             pHttpGlobals->HttpPollTable[ulFdIndex].interested_events = CM_FDPOLL_READ;
	                 	   }
	                 }
	         }
	         ulFdIndex++;
       }
    }  
    
    /*----POLL  START------*/
    ulMaxFds = ulFdIndex;
    ulPollTime = Httpd_Timer_FirstNodeVal(pHttpGlobals);
    lRetVal = cm_fd_poll(ulMaxFds, pHttpGlobals->HttpPollTable,HTTP_FIND_MIN(ulPollTime*1000,HTTPD_MIN_POLL_TIMEOUT));
    if(lRetVal < 0)
    {
        /*
         * Some Error or Failure occured. 
         **/
         HttpDebug(("UCMFDPOLL RETURNED FAILURE \r\n"));
         HttpClient_CleanUp(pHttpGlobals); 	 
    }
    Httpd_Timer_Task(pHttpGlobals);
    ulFdIndex = 0;
    /*-----POLL  END------*/

    /* ------ HTTPS(SSL_read): Read Pending event handling ------*/
    /* IGWSSL_read will return SSL_READ_PENDING incase of SSL_read return SSL_ERROR_WANT_READ 
     * or there may be no enough I/P buffer to copy.
     * In this two conditions we should set the CM_FDPOLL_READ bit on that client FD 
     */
    if(ulRdPendFd)
    {
      HttpDebug(("SSL_READ_PENDING event received, set CM_FDPOLL_READ on FD-%d \n\r", ulRdPendFd));
      pHttpGlobals->HttpPollTable[ulRdPendFd].returned_events |= CM_FDPOLL_READ;
      ulRdPendFd = 0;
    }
    
#ifdef INTOTO_CMS_SUPPORT
    if(CMSDevLpBkSFd > 0)
    {
      if (pHttpGlobals->HttpPollTable[ulFdIndex].returned_events & CM_FDPOLL_READ)
      {
         HttpCMSDevProcessLoopBackData(CMSDevLpBkSFd);
      }
      ulFdIndex++;
    }
#endif  /* INTOTO_CMS_SUPPORT */
	
    if(LpBkSFd > 0)
    {
      if(pHttpGlobals->HttpPollTable[ulFdIndex].returned_events & CM_FDPOLL_READ)
      {
        HttpProcessLoopBackData(pHttpGlobals,LpBkSFd);
      }
      ulFdIndex++;
    } 

    for(uiIndex = 0; uiIndex < pHttpGlobals->iMaxServers; uiIndex++)
    {
      if((pHttpGlobals->HttpServers[uiIndex].SockFd > 0) &&
      	 (pHttpGlobals->HttpPollTable[ulFdIndex].fd_i == pHttpGlobals->HttpServers[uiIndex].SockFd))
      {
        if(pHttpGlobals->HttpPollTable[ulFdIndex].returned_events & CM_FDPOLL_READ)
        {		
          #ifdef OF_IPV6_SUPPORT
	      pHttpGlobals->HttpServers[uiIndex].IpAddr.ip_type_ul=UCM_INET_IPV6;
	      #else
  	      pHttpGlobals->HttpServers[uiIndex].IpAddr.ip_type_ul=cm_inet_ipv4;
	      #endif /*INTOTO_IPV6_SUPPORT*/ 	   
          if(Httpd_AcceptClient(pHttpGlobals,uiIndex,pHttpGlobals->HttpServers[uiIndex].IpAddr.ip_type_ul) != OF_SUCCESS)
          {
               HttpDebug(("Httpd_HandleClient Failed"));
          }
        }
        ulFdIndex++;
      }
    }

    for (uiIndex = 0; uiIndex < HTTP_MAX_REG_FDS; uiIndex++)
    {
      if((pHttpGlobals->HttpExternRegFds[uiIndex].iSockFd > 0) &&
      	 (pHttpGlobals->HttpPollTable[ulFdIndex].fd_i == pHttpGlobals->HttpExternRegFds[uiIndex].iSockFd))
      {
        if(pHttpGlobals->HttpPollTable[ulFdIndex].returned_events & CM_FDPOLL_EXCEPTION)
        {
          if(pHttpGlobals->HttpExternRegFds[uiIndex].pFnError != NULL)
          {
            if(pHttpGlobals->HttpExternRegFds[uiIndex].pFnError(pHttpGlobals->HttpPollTable[ulFdIndex].fd_i,NULL)
            	== HTTP_SKIP_OTHER_EVENTS)
            {
              ulFdIndex++;
              continue;
            }
          }
        }

        if(pHttpGlobals->HttpPollTable[ulFdIndex].returned_events & CM_FDPOLL_READ)
        {
          if(pHttpGlobals->HttpExternRegFds[uiIndex].pFnRead != NULL)
          {
            if(pHttpGlobals->HttpExternRegFds[uiIndex].pFnRead(pHttpGlobals->HttpPollTable[ulFdIndex].fd_i,pHttpGlobals->HttpExternRegFds[uiIndex].user_data_p)
            	== HTTP_SKIP_OTHER_EVENTS)
            {
              ulFdIndex++;
              continue;
            }
          }
        }

        if(pHttpGlobals->HttpPollTable[ulFdIndex].returned_events & CM_FDPOLL_WRITE)
        {
          if(pHttpGlobals->HttpExternRegFds[uiIndex].pFnWrite != NULL)
          {
            if(pHttpGlobals->HttpExternRegFds[uiIndex].pFnWrite(pHttpGlobals->HttpPollTable[ulFdIndex].fd_i,NULL)
            	== HTTP_SKIP_OTHER_EVENTS)
            {
              ulFdIndex++;
              continue;
            }
          }
        }
        ulFdIndex++;
      }
    }
    /**
     * Check whether any PGDRV FD is registered for poll.
     */
    if ((pHttpGlobals->HttpdExternFdToPoll[0].ulFd > 0) &&
        (pHttpGlobals->HttpPollTable[ulFdIndex].fd_i == pHttpGlobals->HttpdExternFdToPoll[0].ulFd))
    {
      if (pHttpGlobals->HttpPollTable[ulFdIndex].returned_events & CM_FDPOLL_READ)
      {
        /**
         * Call the Callback Function Registered along with FD
         */
        if (pHttpGlobals->HttpdExternFdToPoll[0].CbkFun)
        {
          pHttpGlobals->HttpdExternFdToPoll[0].CbkFun(pHttpGlobals->HttpPollTable[ulFdIndex].fd_i);
        }
      }

      ulFdIndex++;
    }

    /**
     * Check whether any ISECPDRI FD is registered for poll.
     */
    if ((pHttpGlobals->HttpdExternFdToPoll[1].ulFd) &&
        (pHttpGlobals->HttpPollTable[ulFdIndex].fd_i == pHttpGlobals->HttpdExternFdToPoll[1].ulFd))
    {
      if (pHttpGlobals->HttpPollTable[ulFdIndex].returned_events & CM_FDPOLL_READ)
      {
        /**
         * Call the Callback Function Registered along with FD
         */
        if (pHttpGlobals->HttpdExternFdToPoll[1].CbkFun)
        {
          pHttpGlobals->HttpdExternFdToPoll[1].CbkFun(pHttpGlobals->HttpPollTable[ulFdIndex].fd_i);
        }
      }

      ulFdIndex++;
    }

#ifdef INTOTO_CMS_SUPPORT
    /****************NEW FEATURE START**************************/

    if ((pHttpGlobals->HttpdAdditionalFdToPoll[0].ulFd > 0) &&
        (pHttpGlobals->HttpPollTable[ulFdIndex].fd_i == pHttpGlobals->HttpdAdditionalFdToPoll[0].ulFd))
    {
      if (pHttpGlobals->HttpPollTable[ulFdIndex].returned_events & CM_FDPOLL_READ)
      {
        /**
         * Call the Callback Function Registered along with FD
         */
        if (pHttpGlobals->HttpdAdditionalFdToPoll[0].CbkFun)
        {
          pHttpGlobals->HttpdAdditionalFdToPoll[0].CbkFun
                               (pHttpGlobals->HttpPollTable[ulFdIndex].fd_i,
                                pHttpGlobals->HttpdAdditionalFdToPoll[0].pArg);
        }
      }

      ulFdIndex++;
    }


    if ((pHttpGlobals->HttpdAdditionalFdToPoll[1].ulFd) &&
        (pHttpGlobals->HttpPollTable[ulFdIndex].fd_i == pHttpGlobals->HttpdAdditionalFdToPoll[1].ulFd))
    {
      if (pHttpGlobals->HttpPollTable[ulFdIndex].returned_events & CM_FDPOLL_READ)
      {
        /**
         * Call the Callback Function Registered along with FD
         */
        if (pHttpGlobals->HttpdAdditionalFdToPoll[1].CbkFun)
        {
          pHttpGlobals->HttpdAdditionalFdToPoll[1].CbkFun
                               (pHttpGlobals->HttpPollTable[ulFdIndex].fd_i,
                                pHttpGlobals->HttpdAdditionalFdToPoll[1].pArg);
        }
      }

      ulFdIndex++;
    }

   /****************NEW FEATURE START**************************/
#endif  /* INTOTO_CMS_SUPPORT */

    for (; ulFdIndex < ulMaxFds; ulFdIndex++)
    {
            /*Some error occured. remove the client request and Tx.Buffer*/
            if(pHttpGlobals->HttpPollTable[ulFdIndex].returned_events & CM_FDPOLL_EXCEPTION)
             {           
               for(uiIndex = 0;uiIndex < ReqTableSize; uiIndex++)  
               {
       	        if((pHttpGlobals->HttpPollTable[ulFdIndex].fd_i  == pHttpGlobals->pHttpClientReqTable[uiIndex].iSockFd))
       	         {
                             for(iResIndex= 0;iResIndex < HTTP_MAX_PIPELINED_REQUESTS; iResIndex++)
                           	 {
                           	       HttpClientReq_t       *pClientrequest=NULL; 
       	    		       pClientrequest = pHttpGlobals->pHttpClientReqTable[uiIndex].pHttpPipeline[iResIndex];
       	    		       if(pClientrequest == NULL ||(pClientrequest->SockFd <=0))
       	    		       { 
       	    		            continue;
       	    		        }
       	    		        else
       	    		        {      	
         	    		            pClientrequest->bPersistentConn=TRUE;
       	    		            HttpFreeClientReqAndTxBuf(pHttpGlobals,pClientrequest);                			      	    		             	    		        
       	    		            pHttpGlobals->pHttpClientReqTable[uiIndex].pHttpPipeline[iResIndex]=NULL;
       	    		         } 
                           	  }                    
                               pHttpGlobals->pHttpClientReqTable[uiIndex].iSockFd=-1000;
        	    		       of_memset(pHttpGlobals->pHttpClientReqTable[uiIndex].pHttpPipeline,0,4*HTTP_MAX_PIPELINED_REQUESTS);
				  
                   	            if(pHttpGlobals->pHttpClientReqTable[uiIndex].pKeepAliveInfo &&
                   	 	       pHttpGlobals->pHttpClientReqTable[uiIndex].pKeepAliveInfo->pKeepAliveTimer)	 	
                   	            {
                   	              Httpd_Timer_Delete(pHttpGlobals->pHttpClientReqTable[uiIndex].pKeepAliveInfo->pKeepAliveTimer);
                    	              pHttpGlobals->pHttpClientReqTable[uiIndex].pKeepAliveInfo->iSockFd=-1;
					pHttpGlobals->pHttpClientReqTable[uiIndex].pKeepAliveInfo=NULL;				  
                    	             }		   
       	          }  
                 }
       	   for(uiIndex = 0;uiIndex < ReqTableSize; uiIndex++)  
                 {
                       if(!(pHttpGlobals->pHttpReqTable[uiIndex]))
            	         {
                            continue;
                        }
       	          if( (pHttpGlobals->pHttpReqTable[uiIndex]) &&
       	          	((pHttpGlobals->HttpPollTable[ulFdIndex].fd_i) == (pHttpGlobals->pHttpReqTable[uiIndex])->SockFd))
       	            {      
        				HttpClientReq_t       *pClientrequest=NULL;
               	               pClientrequest = pHttpGlobals->pHttpReqTable[uiIndex];
               	               if(pClientrequest != NULL)
               	            	 {                   	            	     
                   	       	        pClientrequest->bPersistentConn=FALSE;
                   	                      HttpFreeClientReqAndTxBuf(pHttpGlobals,pClientrequest);
                   	                      pHttpGlobals->pHttpReqTable[uiIndex] = NULL;
                   	                      break;
               	                }
       	               }
       		}        
               	
             }

             /*Write event on the FD. send the data to client*/
             if(pHttpGlobals->HttpPollTable[ulFdIndex].returned_events & CM_FDPOLL_WRITE)
             {      
                    HttpClientReq_t *pClientRequest=NULL;
                    pClientRequest=(HttpClientReq_t *) pHttpGlobals->HttpPollTable[ulFdIndex].user_data_p;
           	     if(HttpSendToClient(pHttpGlobals,pClientRequest,cProcessRetVal) != OF_SUCCESS)
             	     {
                          /**
                           * There is some error while sending the response to the client.
                           * so the request is deleted and the read event on this FD should not be
                           * processed.
                           */
                            continue;
                    }	  
             } 
             if(pHttpGlobals->HttpPollTable[ulFdIndex].returned_events & CM_FDPOLL_READ)
             {
                    /**
                     * Some of them might have been reset by timeout function.
                     */
                    if (!pHttpGlobals->HttpPollTable[ulFdIndex].user_data_p)
                    {
                       continue;
                    }     
                    if(pHttpGlobals->HttpPollTable[ulFdIndex].returned_events==(CM_FDPOLL_READ|CM_FDPOLL_EXCEPTION))
                    {  
#if 0 
	                    	  if(HttpClient_Delete(pHttpGlobals,(HttpClientReq_t *) pHttpGlobals->HttpPollTable[ulFdIndex].user_data_p) !=OF_SUCCESS)
	                    	  {
	                    	      Httpd_Error(("HttpClient_Delete is Failed\r\n"));
	                    	  }
	                    	  Httpd_Debug("Read with Exception \r\n");	     	                    	  	    
#endif
	                    	  continue;
                    }
                    cProcessRetVal=HttpClient_Process(pHttpGlobals, (HttpClientReq_t *)
                                        pHttpGlobals->HttpPollTable[ulFdIndex].user_data_p);
                    if(cProcessRetVal == HTTPS_SSL_READ_PENDING)
                    {
                      ulRdPendFd = ulFdIndex;
                      cProcessRetVal = OF_SUCCESS;   
                    }
             }         
  }
  }
  return OF_SUCCESS;
} /* Httpd_Start() */

/**************************************************************************
 * Function Name : Httpd_Init                                             *
 * Description   : This function is used to initialize the data structures*
 *                 used by the http server.                               *
 * Input         : None.                                                  *
 * Output        : None.                                                  *
 * Return value  : OF_SUCCESS on success and OF_FAILURE, otherwise.         *
 **************************************************************************/

int32_t Httpd_Init( void )
{
   bool    isInitDone = FALSE;
  HttpdSetCookieInfo_t  *pSetTemp  = NULL;
  HttpdGetCookieInfo_t  *pGetTemp  = NULL;
  uint16_t            	   ii=0,usCookies=0;
  int32_t               	   iMaxCookieBufferLen=0;
  int16_t                        iLoop=0; 
  HttpGlobals_t                *pHttpg=NULL; 
  /**
   * Verify if the http server is already initialized.
   */
  if (isInitDone)
  {
    return OF_SUCCESS;
  }
  pHttpg=of_calloc(1,sizeof(HttpGlobals_t));
  if(!pHttpg)
  {
     Httpd_Error("Memory Allocation failed for pHttpGlobals\n\r");
     return OF_FAILURE;
  }  
  pHTTPG=pHttpg;  
  /*memset Last accessed url */
  of_memset(aPrevXmlUrlData,0,HTTP_FILENAME_LEN +1);

  
  pHttpg->pCbkTimers=of_calloc(1,sizeof(HttpdCbkTimers_t));
  if(!pHttpg->pCbkTimers)
  {
     Httpd_Error("Memory Allocation failed for pHttpg->pCbkTimers\n\r");
     return OF_FAILURE;
  }
  
  if(HttpdGlobalConfigInit(pHttpg) != OF_SUCCESS)
  {
      Httpd_Error("HttpdGlobalConfigInit() returned failure\n\r");
      return OF_FAILURE;
  }  
  pHttpg->MutexSockFd = -1;  
  pHttpg->pHttpRegModuleHead_g = NULL;  
  pHttpg->HttpRegModuleFreeQMaxSize_g=0;  
  of_strcpy(pHttpg->IGWHttpdCoreVer_g, "HTTPD-CORE-1.1.1.0");
  of_memset(&pHttpg->HttpdConfig,0,sizeof(HttpdConfig_t));
  of_memset(&pHttpg->HttpFileBuf,0,sizeof(HtmlFileBuf_t));   
  pHttpg->HttpServers=NULL;
  pHttpg->iMaxServers = 0;
  pHttpg->HttpPollTable=NULL;
  pHttpg->lHttpFileId=0;
  pHttpg->pFreeSetCookieInfo = NULL;
  pHttpg->pFreeGetCookieInfo = NULL;
  of_memset(pHttpg->HttpExternRegFds,0,HTTP_MAX_REG_FDS);  
  of_memset( &pHttpg->HttpRegModuleFreeQ,0,sizeof(HttpRegModuleRecFreeQ_t)); 
  
  /**
   * This variable maintains the additional Pseudo driver FD and PGDRV to be
   * polled in LINUX case (for getting Kernel Event when HTTP Server port
   * configuration changes.
   */
   of_memset(pHttpg->HttpdExternFdToPoll,0,HTTP_EXTERNFD_POLLTABLE_LEN);
   of_memset(pHttpg->HttpMimeBuckets,0,HTTP_MIMEBUCKET_SIZE);
   
  /**
   * This will store the module ID as given by policy group module during
   * the registration.
   */   
   pHttpg->uiHttpModuleId_g = 0;   

  /* Get max servers */
  pHttpg->iMaxServers = HttpGetMaxServers(-1);
#ifndef OF_CM_SUPPORT
  HttpSetMaxServersInList(&pHttpg->iMaxServers);
#endif /*OF_CM_SUPPORT*/

#ifndef OF_CM_SUPPORT
  /* Initialize the port numbers from kernel */
  if(HttpInitPortList() != OF_SUCCESS)
  {
#ifdef INTOTO_CMS_SUPPORT
    Httpd_Error("HttpInitPortList() Failed Continue !!\n\r");
#else
    Httpd_Error("HttpInitPortList() Failed !!\n\r");
   //UCM return OF_FAILURE;
#endif  /* INTOTO_CMS_SUPPORT */    
  }
#endif  /* OF_CM_SUPPORT */

 pHttpg->gHttpMaxReqTableSize= Httpd_GetMaxReqTableSize(pHttpg); /*pHttpg->pHttpGlobalConfig->iReqtablesize;*/
 pHttpg->pHttpReqTable=(HttpClientReq_t **)of_calloc(pHttpg->gHttpMaxReqTableSize,sizeof(HttpClientReq_t *));
 if(!pHttpg->pHttpReqTable)
 {
   Httpd_Error("Memory Allocation failed for pHttpGlobals->pHttpReqTable\n\r");
   return OF_FAILURE;
 }

 pHttpg->gHttpMaxFds=Httpd_GetMaxFds(pHttpg); /*pHttpg->pHttpGlobalConfig->iMaxfds;*/
 pHttpg->HttpPollTable=of_calloc(pHttpg->gHttpMaxFds, sizeof (struct cm_poll_info));
 if(!pHttpg->HttpPollTable)
 {
   Httpd_Error("Memory Allocation failed for pHttpGlobals->HttpPollTable\n\r");
   return OF_FAILURE;
 }
  /**
   * Get the configuration.
   */
 Httpd_GetConfig(&pHttpg->HttpdConfig);

  /**
   * Verify if the super user name is set, if not set it.
   */
  if (!pHttpg->HttpdConfig.pSUName)
  {
    pHttpg->HttpdConfig.pSUName = (unsigned char *) "root"; // for UCM GetAdminUserName();
  }

  /**
   * Get the maximum number of cookies configured.
   */
  usCookies = Httpd_GetMaxCookies(pHttpg); /*pHttpg->pHttpGlobalConfig->ulMaxCookies;*/
  iMaxCookieBufferLen=Httpd_GetMaxCookieBufferLen(pHttpg); /*pHttpg->pHttpGlobalConfig->ulHttpdMaxCookieBufferLen;*/
  /**
   * Allocate the memory for SetCookies and GetCookies and make them as
   * free linked list pool.
   */
  for (ii = 0; ii < usCookies; ii++)
  {
    pSetTemp = (HttpdSetCookieInfo_t *)
                    of_calloc(1, sizeof(HttpdSetCookieInfo_t));        

    if (!pSetTemp)
    {
      Httpd_Error("Httpd_Init: Unable To Alloc SetCookieInfo List\n");
      return OF_FAILURE;
    }

    pSetTemp->CookieBuff=of_calloc(1,iMaxCookieBufferLen+1);
    if (!pSetTemp->CookieBuff)
    {
      of_free(pSetTemp);
      Httpd_Error("Httpd_Init: Unable To Alloc SetCookieInfo Buffer\n");
      return OF_FAILURE;
    }
    pSetTemp->bHeep = 1;

    if (pHttpg->pFreeSetCookieInfo == NULL)
    {
      pHttpg->pFreeSetCookieInfo        = pSetTemp;
      pHttpg->pFreeSetCookieInfo->pNext = NULL;
    }
    else
    {
      pSetTemp->pNext    = pHttpg->pFreeSetCookieInfo;
      pHttpg->pFreeSetCookieInfo = pSetTemp;
    }
  }

  for (ii = 0; ii < usCookies; ii++)
  {
    pGetTemp = (HttpdGetCookieInfo_t *)
                    of_calloc(1, sizeof(HttpdGetCookieInfo_t));

    if (!pGetTemp)
    {
      Httpd_Error("Httpd_Init: Unable To Alloc GetCookieInfo List\n");
      return OF_FAILURE;
    }

    pGetTemp->CookieBuff=of_calloc(1,iMaxCookieBufferLen+1);
    if (!pGetTemp->CookieBuff)
    {
      Httpd_Error("Httpd_Init: Unable To Alloc GetCookieInfo List\n");
      of_free(pGetTemp);	   
      return OF_FAILURE;
    }
    pGetTemp->bHeep = 1;

    if (pHttpg->pFreeGetCookieInfo == NULL)
    {
        pHttpg->pFreeGetCookieInfo        = pGetTemp;
        pHttpg->pFreeGetCookieInfo->pNext = NULL;
    }
    else
    {
      pGetTemp->pNext    = pHttpg->pFreeGetCookieInfo;
      pHttpg->pFreeGetCookieInfo = pGetTemp;
    }
  }
  HttpRegModuleInit(pHttpg);

  /* Register with Http Module for specific files */
  HttpRegMod_http_Init(pHttpg);

  /**
   * Initialize the MIME buckets with the respective entries using the
   * static mime entries array.
   */
   HttpMime_Init(pHttpg);

  /**
   * Initialize the maximum number of session cookies and the respective
   * data structures.
   */
  if (HttpCookie_Init(pHttpg,50) != OF_SUCCESS)
  {
    Httpd_Error("Unable To Initialize Cookie List\n");
    return OF_FAILURE;
  }

  /**
   * Initialize the flash file system for html files.
   */
  if (HtmlFs_Init(pHttpg) < 0)
  {
    Httpd_Error("Unable To Initialize HTML File System\n");
    return OF_FAILURE;
  }

  /**
   * Initialize the timer related data structures.
   */
  if (Httpd_Timer_Init(pHttpg) < 0)  //FIXME: UCM
  {
    Httpd_Error("Unable To Initialize HTTP Timer Module\n");
#ifndef INTOTO_CMS_SUPPORT
    // return OF_FAILURE; // for UCM
#endif  /* INTOTO_CMS_SUPPORT */
  }

  /**
   * Initialize the type of tags that are supported by the http server.
   */
  if (Httpd_TagList_Init() < 0)
  {
    Httpd_Error("Internal Initialization Failure\n");
#ifndef INTOTO_CMS_SUPPORT
    // return OF_FAILURE; // for UCM
#endif  /* INTOTO_CMS_SUPPORT */
  }

  /**
   * Register the http server related tags, which includes tags for login,
   * logout, errscrn, etc.
   */
  if (Httpd_RegisterLocalTags() < 0)
  {
    Httpd_Error("Unable To Register Callback Functions\n");
#ifndef INTOTO_CMS_SUPPORT
    // return OF_FAILURE; // for UCM
#endif  /* INTOTO_CMS_SUPPORT */
  }

#ifdef INTOTO_CMS_SUPPORT
if (HttpdCMSDevRegisterLocalTags() < 0)
{
  Httpd_Error(" HttpdCMSDevRegisterLocalTags Failed\n");
    
}
#endif  /* INTOTO_CMS_SUPPORT */

#ifdef OF_CM_SUPPORT
if (IGWUcmHttpInit() < 0)
{
  Httpd_Error(" IGWUcmHttpInit Failed\n");
    
}
#endif  /* OF_CM_SUPPORT */

 /**
 *Initialize the http servers based on the options selected.
 */
  if (Httpd_InitServers(pHttpg) != OF_SUCCESS)
  {
    Httpd_Error("Unable To Initialize Servers\n");
#ifndef INTOTO_CMS_SUPPORT
    return OF_FAILURE;
#endif  /* INTOTO_CMS_SUPPORT */
  }

#ifdef OF_HTTPS_SUPPORT
  /**
   * Initialize the https certificate related prerequisites.
   */
  Https_CertsInit();
#endif /* OF_HTTPS_SUPPORT */

#ifndef OF_CM_SUPPORT
if(HttpAuthOrderInit(pHttpg)!=OF_SUCCESS) //FIXME UCM with PAM
{
  Httpd_Error("Unable To Initialize Authentication order\n");
#ifndef INTOTO_CMS_SUPPORT
  // return OF_FAILURE; // for UCM
#endif  /* INTOTO_CMS_SUPPORT */
}
#endif /* OF_CM_SUPPORT*/

 pHttpg->pHttpClientReqTable=(HttpResponceTable_t  *)
 	                               of_calloc(pHttpg->gHttpMaxFds,sizeof(HttpResponceTable_t ));
 if(pHttpg->pHttpClientReqTable==NULL)
 {
      HttpDebug("pHttpGlobals->pHttpClientReqTable calloc failed");
      return OF_FAILURE; 
 }
 for(iLoop=0;iLoop<pHttpg->gHttpMaxFds;iLoop++)
 {
     of_memset(pHttpg->pHttpClientReqTable[iLoop].pHttpPipeline,0,HTTP_MAX_PIPELINED_REQUESTS);
 }
 
 /**
 * Set the initialization flag.
 */
#ifdef INTOTO_CMS_SUPPORT
 IGWConfRecvInit();
#endif  /* INTOTO_CMS_SUPPORT */
  isInitDone = TRUE;
 return OF_SUCCESS;
} /* Httpd_Init() */

/**************************************************************************
 * Function Name : Httpd_Destroy                                          *
 * Description   : This function is used to close all opened sockets by   *
 *                 the http server.                                       *
 * Input         : pHttpGlobals  - Global pointer.                        *
 * Output        : None.                                                  *
 * Return value  : OF_SUCCESS on success and OF_FAILURE, otherwise.         *
 **************************************************************************/

int32_t Httpd_Destroy(HttpGlobals_t *pHttpGlobals)
{
  int32_t   iRetVal;
  uint32_t  uiIndex;
  int32_t    ReqTableSize;

  if(!pHttpGlobals)
  {
    Httpd_Error("pHttpGlobals is NULL\r\n");	
    return OF_FAILURE;
  }
  
  ReqTableSize=Httpd_GetMaxReqTableSize(pHttpGlobals);/*pHttpGlobals->pHttpGlobalConfig->iReqtablesize;*/

  Httpd_Print("Closing All Socket Fds\n");

/**
 * Close all the opened sockets.
 */
  for (uiIndex = 0; uiIndex < ReqTableSize; uiIndex++)   /* REQTABLE_SIZE */
  {
    if (!pHttpGlobals->pHttpReqTable[uiIndex])
    {
      continue;
    }

    cm_socket_close(
        pHttpGlobals->HttpServers[(pHttpGlobals->pHttpReqTable[uiIndex])->lServId].ClientClose(
            pHttpGlobals->pHttpReqTable[uiIndex]));
  }

  /**
   * Close all the operned sockets.
   */
  for (uiIndex = 0; uiIndex < pHttpGlobals->iMaxServers; uiIndex++)
  {
    if (pHttpGlobals->HttpServers[uiIndex].SockFd >= 0)
    {
      iRetVal = cm_socket_close(pHttpGlobals->HttpServers[uiIndex].SockFd);

      if (iRetVal != OF_SUCCESS)
      {
        Httpd_Print("%s -- ERROR In closing Server Socket\r\n",
                    __FUNCTION__);
      }
    }
  }

  return OF_SUCCESS;
} /* Httpd_Destroy() */

/*  */
void HttpRegMod_http_Init( HttpGlobals_t  *pHttpGlobals)
{
  HttpAPIRegModuleInfo_t Module;

  if(!pHttpGlobals)
  {
    Httpd_Error("pHttpGlobals is NULL\r\n");	
    return;
  }
  
  of_memset(&Module, 0, sizeof(Module));
  Module.ucPriority= HTTP_REG_MIN_PRIORITY;
  of_strcpy(Module.ModuleName,"HTTP");
  of_strcpy((char *) Module.Loginfile,"/login.htm");
  of_strcpy((char *) Module.Logoutfile,"/rlogout.htm");

  Http_LoginRegModuleAPI(pHttpGlobals,&Module);
}

/**************************************************************************
 * Function Name : HttpMime_Init                                          *
 * Description   : This function is used to initialize the MIME related   *
 *                 data structures.                                       *
 * Input         : pHttpGlobals  - Global pointer.                        *
 * Output        : Creates a MIME bucket with the extentions and their    *
 *                 respective content types.                              *
 * Return value  : None.                                                  *
 **************************************************************************/

/*  */
void HttpMime_Init( HttpGlobals_t  *pHttpGlobals)
{
  hMimeType_t  *pMimeType, **ppTrackMimeTypes;
  int32_t      iIndex;

  if(!pHttpGlobals)
  {
    Httpd_Error("pHttpGlobals is NULL\r\n");	
    return ;
  }
  
  /**
   * Get the starting entry of the MIME types static array.
   */
  pMimeType = HttpMimeTypes;

  /**
   * Initilize the MIME buckets array with the MIME types linked list
   * based on the alphabetical order.
   */
  for (iIndex = 0; iIndex < 27; ++iIndex)
  {
    pHttpGlobals->HttpMimeBuckets[iIndex] = NULL_MIMETYPE;
  }

  /**
   * Fill the MIME buckets with the respective MIME entries using the MIME
   * types array.
   */
  while (pMimeType->Ext != NULL)
  {
    /**
     * Get the index of the current MIME entry.
     */
    iIndex = MIME_HASH(pMimeType->Ext[0]);

    /**
     * Get the respective MIME bucket using the obtained index.
     */
    ppTrackMimeTypes = &(pHttpGlobals->HttpMimeBuckets[iIndex]);

    /**
     * Navigate to the correct entry location in the obtained MIME bucket
     * using the cronology.
     */
    while (*ppTrackMimeTypes != NULL_MIMETYPE)
    {
      if (of_strcmp((*ppTrackMimeTypes)->Ext, pMimeType->Ext) >= 0)
      {
        break;
      }
      ppTrackMimeTypes = &((*ppTrackMimeTypes)->Chain);
    }

    /**
     * Copy the current entry to the obtained MIME bucket.
     */
    pMimeType->Chain  = *ppTrackMimeTypes;
    *ppTrackMimeTypes = pMimeType;

    /**
     * Go to the next entry.
     */
    ++pMimeType;
  }
} /* HttpMime_Init() */

/**************************************************************************
 * Function Name : HttpMime_Ext2ContentType                               *
 * Description   : This function is used to get the MIME type or content  *
 *                 type for the given extention.                          *
 * Input         : aExtention   (I) - extention of the file for which     *
 *                                    content type is to get.             *
 *                 aContentType (O) - content type for the given extention*
 *                                    to get.                             *
 *                 pHttpGlobals  - Global pointer                         *
 * Output        : Content type or MIME type for the given extention.     *
 * Return value  : OF_SUCCESS on success and OF_FAILURE, otherwise.         *
 **************************************************************************/

/**/
int32_t HttpMime_Ext2ContentType(HttpGlobals_t  *pHttpGlobals,char  *aExtention,
                                  char  *aContentType )
{
  int32_t      iIndex = 0;
  hMimeType_t  *pMimeType = NULL;

  /**
   * Validate the passed parameter(s).
   */

   if(!pHttpGlobals)
  {
    Httpd_Error("pHttpGlobals is NULL\r\n");	
    return OF_FAILURE;
  }
   
  if (aExtention)
  {
    /**
     * Get the index of the extention to be searched in the MIME buckets
     * global variable.
     */
    iIndex = MIME_HASH(aExtention[0]);

    /**
     * Validate the obtained index.
     */
    if ((iIndex < 0) ||
        (iIndex > 26))
    {
      /**
       * Copy the default content type for the given extention and return
       * failure.
       */
      of_strcpy(aContentType, HTTPD_DEF_CONT_TYPE);
      return OF_FAILURE;
    }

    /**
     * Get the starting entry of the obtained index.
     */
    pMimeType = pHttpGlobals->HttpMimeBuckets[iIndex];

    /**
     * Validate the obtained starting Mime entry.
     */
    if (pMimeType != NULL)
    {
      /**
       * Search for the given extention in the MIME buckets linked list.
       */
      while (pMimeType)
      {
        if (strcasecmp(pMimeType->Ext, aExtention) == 0)
        {
          /**
           * For successful search of the extention copy the relevant
           * content type and return success.
           */
          of_strcpy(aContentType, pMimeType->ContentType);
          return OF_SUCCESS;
        }
        pMimeType = pMimeType->Chain;
      }
    }
  }

  /**
   * For the failure case copy the default content type to the parameter
   * and return failure.
   */
  of_strcpy(aContentType, HTTPD_DEF_CONT_TYPE);
  return OF_FAILURE;
} /* HttpMime_Ext2ContentType() */

/**************************************************************************
 * Function Name : HtmlFs_Init                                            *
 * Description   : This function is used to initialize the flash file     *
 *                 system used by the http server for html files.         *
 * Input         : pHttpGlobals  - Global pointer                         *
 * Output        : Valid file descripter to be used to maintain the html  *
 *                 files. A global variable is used for this.             *
 * Return value  : OF_SUCCESS on success and OF_FAILURE, otherwise.         *
 **************************************************************************/

/*  */
int32_t HtmlFs_Init( HttpGlobals_t  *pHttpGlobals)
{
  if(!pHttpGlobals)
  {
    Httpd_Error("pHttpGlobals is NULL\r\n");	
    return OF_FAILURE;
  }
  
  /**
   * Using the http configuration parameters initialize the file system for
   * the html files. This will return a file descripter, which will be used
   * for all the file accessings related to the html files.
   */
  /* TO BE FIXED FOR UCM
  pHttpGlobals->lHttpFileId = UCMFileNewFileSystem(pHttpGlobals->HttpdConfig.ulHtmlFsOffset,
                                     pHttpGlobals->HttpdConfig.ulHtmlFsSize);
  */

  /**
   * Validate the file descripter obtained from the creation of new file
   * system for the html files.
   */
  if (pHttpGlobals->lHttpFileId < 0)
  {
    Httpd_Error("Unable to set html file system!");
    return OF_FAILURE;
  }
  return OF_SUCCESS;
} /* HtmlFs_Init() */

/**************************************************************************
 * Function Name : HtmlFs_OpenFile                                        *
 * Description   : This function is used to open the given file from the  *
 *                 flash in the given mode.                               *
 * Input         : pHttpGlobals  - Global pointer  
 *				   pFileName (I) - name the file to be opened from the    *
 *                                 flash in the given mode.               *
 *                 iMode     (I) - mode to be used opening the file from  *
 *                                 the flash.                             *
 * Output        : None.                                                  *
 * Return value  : OF_SUCCESS on success and OF_FAILURE, otherwise.         *
 **************************************************************************/

int32_t HtmlFs_OpenFile(HttpGlobals_t *pHttpGlobals,char  *pFileName,
                         int32_t  iMode )
{
  unsigned char  ucFileName[HTTP_FILENAME_LEN + 1];
  
  if(!pHttpGlobals)
  {
    Httpd_Error("pHttpGlobals is NULL\r\n");	
    return OF_FAILURE;
  }
  /**
   * Validate the passed parameter(s).
   */
  if (!pFileName)
  {
    return OF_FAILURE;
  }

  /**
   * Validate the passed file name.
   */
  if (of_strstr(pFileName, ".."))
  {
    return OF_FAILURE;
  }

  /**
   * Validate the length of the file name including its path.
   */
  if ((of_strlen((char *) Httpd_GetDocPath(pHttpGlobals)) +
                of_strlen((char *) pFileName)) > HTTP_FILENAME_LEN)
  {
    return OF_FAILURE;
  }

  /**
   * Construct absolute path.
   */
  sprintf((char *) ucFileName, "%s%s", Httpd_GetDocPath(pHttpGlobals), pFileName);

#ifdef HTTPD_DEBUG
  Httpd_Print("Opening File '%s'\n", ucFileName);
#endif

  /**
   * Using the formatted file name, open the file in the given mode.
   */
  return cm_file_open(pHttpGlobals->lHttpFileId, ucFileName, iMode);
} /* HtmlFs_OpenFile() */

/**************************************************************************
 * Function Name : HtmlFs_ReadFile                                        *
 * Description   : This function is used to read the information from the *
 *                 respective file using the given parameters.            *
 * Input         : iHandle (I) - file descripter to be used to read the   *
 *                               information from the flash file.         *
 *                 pBuf    (O) - pointer to the buffer to be used to copy *
 *                               the information read from the flash file.*
 *                 iCount  (I) - number of charecter to read from the     *
 *                               respective flash file.                   * 
 *                 pHttpGlobals  - Global pointer                         *
 * Output        : Information to be read from the respective flash file. *
 * Return value  : OF_SUCCESS on success and OF_FAILURE, otherwise.         *
 **************************************************************************/

int32_t HtmlFs_ReadFile(HttpGlobals_t *pHttpGlobals,int32_t  iHandle,
                         char  *pBuf,
                         int32_t  iCount )
{
  /**
   * Validate the passed parameter(s).
   */
  if(!pHttpGlobals)
  {
    Httpd_Error("pHttpGlobals is NULL\r\n");	
    return OF_FAILURE;
  }
  if (!pBuf)
  {
    return OF_FAILURE;
  }

  /**
   * Read the information from the reespective flash file using the given
   * parameters.
   */
  return cm_file_read(pHttpGlobals->lHttpFileId, iHandle, (unsigned char *) pBuf, iCount);
} /* HtmlFs_ReadFile() */

/**************************************************************************
 * Function Name : HtmlFs_WriteFile                                       *
 * Description   : This function is used to write the information to the  *
 *                 respective file using the given parameters.            *
 * Input         : iHandle (I) - file descripter to be used to write the  *
 *                               information to the flash file.           *
 *                 pBuf    (I) - pointer to the information to write in   *
 *                               the respective flash file.               *
 *                 iCount  (I) - number of charecter to write in the      *
 *                               respective flash file.                   *
 *                 pHttpGlobals  - Global pointer                         *     
 * Output        : None.                                                  *
 * Return value  : OF_SUCCESS on success and OF_FAILURE, otherwise.         *
 **************************************************************************/
/**/
int32_t HtmlFs_WriteFile(HttpGlobals_t *pHttpGlobals, int32_t  iHandle,
                          char  *pBuf,
                          int32_t  iCount )
{
  /**
   * Validate the passed parameter(s).
   */
  if(!pHttpGlobals)
  {
    Httpd_Error("pHttpGlobals is NULL\r\n");	
    return OF_FAILURE;
  }
  if (!pBuf)
  {
    return OF_FAILURE;
  }

  /**
   * Write the information to the respective flash file.
   */
  return cm_file_write(pHttpGlobals->lHttpFileId, iHandle, (unsigned char *) pBuf, iCount);
} /* HtmlFs_WriteFile() */

/**************************************************************************
 * Function Name : HtmlFs_SeekFile                                        *
 * Description   : This function is used to seek the information from the *
 *                 respective file using the given parameters.            *
 * Input         : iHandle (I) - file descripter to be used to seek the   *
 *                               information from the flash file.         *
 *                 iOffset (I) - offset to be used to seek the information*
 *                               from the flash file.                     *
 *                 lWhence (I) - starting place in the flash file from    *
 *                               where information starts.                *
 *                 pHttpGlobals  - Global pointer                         * 
 * Output        : None.                                                  *
 * Return value  : OF_SUCCESS on success and OF_FAILURE, otherwise.         *
 **************************************************************************/

int32_t HtmlFs_SeekFile(HttpGlobals_t *pHttpGlobals, int32_t  iHandle,
                         int32_t  iOffset,
                         int32_t  iWhence )
{
  if(!pHttpGlobals)
  {
    Httpd_Error("pHttpGlobals is NULL\r\n");	
    return OF_FAILURE;
  }
  return  cm_file_seek(pHttpGlobals->lHttpFileId, iHandle, iOffset, iWhence);
} /* HtmlFs_SeekFile() */

/**************************************************************************
 * Function Name : HtmlFs_CloseFile                                       *
 * Description   : This function is used to close the file from the flash.*
 *                 The file name can be obtained from the global variable.*
 * Input         : iHandle (I) - file descripter to be used to close the  *
 *                               flash file.                              *
 *                 pHttpGlobals  - Global pointer                         *  
 * Output        : Closes the respective flash file.                      *
 * Return value  : OF_SUCCESS on success and OF_FAILURE, otherwise.         *
 **************************************************************************/

int32_t HtmlFs_CloseFile(HttpGlobals_t *pHttpGlobals,int32_t  iHandle )
{
  if(!pHttpGlobals)
  {
    Httpd_Error("pHttpGlobals is NULL\r\n");	
    return OF_FAILURE;
  }
  return cm_file_close(pHttpGlobals->lHttpFileId, iHandle);
} /* HtmlFs_CloseFile() */

/**************************************************************************
 * Function Name : IGWHttpdGetCoreVersion                                 *
 * Description   : This function is used to get the core version of the   *
 *                 http server.                                           *
 *                 This function expects a buffer pointer where in it     *
 *                 fills the value present in the global variable,        *
 *                 pHttpGlobals->IGWHttpdCoreVer_g                        *
 * Input         : pBuffer (O) - pointer to the buffer where the current  *
 *                               version of the http server will be copied*
 *                               This field should have a valid address.  *
 *                 pHttpGlobals  - Global pointer                         *     
 * Output        : Core version of http server.                           *
 * Return value  : OF_SUCCESS - if function copies the version             *
 *                 OF_FAILURE - if the pointer passed is NULL              *
 **************************************************************************/

int32_t IGWHttpdGetCoreVersion(HttpGlobals_t *pHttpGlobals,char  *pBuffer )
{
  /**
   * Validate the parameter and copy the current core version of http
   * server into the given parameter.
   */
   if(!pHttpGlobals)
  {
    Httpd_Error("pHttpGlobals is NULL\r\n");	
    return OF_FAILURE;
  }
   
  if (pBuffer)
  {
    sprintf(pBuffer, "%s\r\n", pHttpGlobals->IGWHttpdCoreVer_g);
    return OF_SUCCESS;
  }

  return OF_FAILURE;
} /* IGWHttpdGetCoreVersion() */

/**************************************************************************
 * Function Name : HttpGetSocketIPAddress                                 *
 * Description   : This function is used to get the IP address used in the*
 *                 socket used by the http server.                        *
 * Input         : pHttpRequest (I) - pointer to the HttpRequest structure*
 *                 pIPAddress   (O) - pointer to the IP address used in   *
 *                                    socket used by the http server.     *
 * Output        : Socket IP address.                                     *
 * Return value  : OF_SUCCESS on success and OF_FAILURE, otherwise.         *
 **************************************************************************/

int32_t HttpGetSocketIPAddress( HttpRequest  *pHttpRequest,
                                ipaddr   *pIPAddress )
{
  uint16_t         usLocalPort = 0;
  HttpClientReq_t  *pClientReq   = NULL;

  /**
   * Validate the passed parameter(s).
   */
  if (!pHttpRequest)
  {
    return OF_FAILURE;
  }

  /**
   * Get the client request from the passed http request parameter and
   * validate it.
   */
  pClientReq = (HttpClientReq_t *) pHttpRequest->pPvtData;

  if (!pClientReq)
  {
    return OF_FAILURE;
  }

  /**
   * Get the socket information.
   */   
  return cm_socket_get_sock_name(pClientReq->SockFd, pIPAddress,
                              &usLocalPort);
} /* HttpGetSocketIPAddress() */
/**************************************************************************
 * Function Name : HttpdGetSockIPAddress                                 *
 * Description   : This function is used to get the IPv6 or IPv4address used in the*
 *                 socket used by the http server.                        *
 * Input         : pHttpRequest (I) - pointer to the HttpRequest structure*
 *                 pIPAddress   (O) - pointer to the  IGWIpAddr_t   *
 * Output        : Socket IP address.                                     *
 * Return value  : OF_SUCCESS on success and OF_FAILURE, otherwise.         *
 **************************************************************************/

int32_t HttpdGetSockIPAddress( HttpRequest  *pHttpRequest,
                                ipaddr *pIPAddress )
{
  uint16_t         usLocalPort = 0;
  HttpClientReq_t  *pClientReq   = NULL;

  /**
   * Validate the passed parameter(s).
   */
  if (!pHttpRequest)
  {
    return OF_FAILURE;
  }

  /**
   * Get the client request from the passed http request parameter and
   * validate it.
   */
  pClientReq = (HttpClientReq_t *) pHttpRequest->pPvtData;

  if (!pClientReq)
  {
    return OF_FAILURE;
  }

  /**
   * Get the socket information.
   */   
   
  #ifdef INTOTO_IPV6_SUPPORT
#ifndef OF_CM_SUPPORT
   if(pHttpRequest->PeerIp.ip_type_ul == UCM_INET_IPV6)
   {
       pIPAddress->ip_type_ul=UCM_INET_IPV6;
   } 
   else
   #endif /*INTOTO_IPV6_SUPPORT*/
   {
       pIPAddress->ip_type_ul=cm_inet_ipv4;
   }
#endif /*OF_CM_SUPPORT*/
  return cm_socket_get_sock_name(pClientReq->SockFd, pIPAddress,
                              &usLocalPort);
} /* HttpGetSocketIPAddress() */

/***************************************************************************
 * Function Name  : HttpProcessLoopBackData
 * Description    : This function will process the data recieved on loop
 *                  back socket
 * Input          : SFd - socket Fd
 *                  pHttpGlobals  - Global pointer
 * Output         : None
 * Return value   :
 ***************************************************************************/
 int32_t HttpProcessLoopBackData(HttpGlobals_t *pHttpGlobals,uint32_t SFd)
{
  HttpPortArgs_t     PortArgs = {};
  /*IGWIpAddr_t        PeerIp = {};*/
  ipaddr             PeerIp;
  uint16_t           PeerPort = 0;
  int32_t            iIndex = 0;
  int32_t            iRetVal = -1;
  HttpPortInfo_t    *pPortInfo=NULL;

  if(!pHttpGlobals)
  {
    Httpd_Error("pHttpGlobals is NULL\r\n");	
    return OF_FAILURE;
  } 
	
 /**
  *  Http Server is  using  loop back address ((0x7F000001) 127.0.0.1),So we are  passing  IPv4 address 
  *  to the cm_socket_recv_from().  
  */   
  /*PeerIp.ip_type_ul=IGW_INET_IPV4;*/
  iRetVal = cm_socket_recv_from(SFd, (char*)&PortArgs,sizeof(HttpPortArgs_t),
                                0, &PeerIp, &PeerPort);
  if(iRetVal != sizeof(HttpPortArgs_t))
  {
    HttpDebug(("Failed to recieve data on loop back socket\n\r"));
    return OF_FAILURE;
  }

  HttpDebug(("portnum =%d, %s  %s \n\r",PortArgs.PortInfo.usPort,
            SERVER_TYPE_2STR(PortArgs.PortInfo.eServerType),
            HTTP_ENABLED_2STR(PortArgs.PortInfo.bEnabled)));
  switch(PortArgs.ucCmd)
  {
    case HTTP_CMD_ADD_PORT:
      /* create the socket and bind*/
      iRetVal = HttpCreateSockAndBind(pHttpGlobals,PortArgs.PortInfo);
#ifndef OF_CM_SUPPORT
      iRetVal = HttpAddPortToList(&PortArgs.PortInfo);
#endif /*OF_CM_SUPPORT*/
      break;

    case HTTP_CMD_DEL_PORT:
      iRetVal = DeletePortFromServer(PortArgs.PortInfo.usPort);
      if(iRetVal != OF_SUCCESS)
      {
        if(cm_socket_send_to(SFd, (void*)&iRetVal, sizeof(iRetVal),
                        0, PeerIp,PeerPort) == OF_FAILURE)
        {
           Httpd_Error("cm_socket_send_to returned failure");
	    return OF_FAILURE;	   
        }
        break;
      }
#ifndef OF_CM_SUPPORT
      iRetVal = HttpDelPortFromList(&PortArgs.PortInfo);
#endif /*OF_CM_SUPPORT*/
      if(cm_socket_send_to(SFd, (void*)&iRetVal, sizeof(iRetVal),
                      0, PeerIp,PeerPort) == OF_FAILURE)
      	{
      	   Httpd_Error("cm_socket_send_to returned failure");
	   return OF_FAILURE;
      	}                      
      break;

    case HTTP_CMD_LOAD_DEF_CONFIG:
      HttpGetDefaultPorts(pHttpGlobals,&pPortInfo);/*pHttpGlobals->pHttpGlobalConfig->DefaultPortConfig;*/
      for(iIndex = 0; iIndex < pHttpGlobals->iMaxServers; iIndex++)
      {
        if(pPortInfo[iIndex].usPort != 0)
        {
          HttpAddPort(pPortInfo[iIndex]);
#ifdef HTTP_REDIRECT_HTTPS
          if(pHttpGlobals->HttpServers[iIndex].Type == hServerNormal )
          {
            pHttpGlobals->HttpServers[iIndex].HtmlPermission = Httpd_OnlyCerts_GetPermission;
          }
#endif
        }
      }
      return OF_SUCCESS;


    case HTTP_CMD_CHANGE_CONFIG:
      iRetVal = ChangePortInfo(PortArgs);
      break;
    default:
      /* send inavlid cmd recived */
      HttpDebug(("Invalid command recieved. cmd = %d\n\r",PortArgs.ucCmd));
      iRetVal = HTTP_ERR_INVALID_COMMAND;
      break;
  }

  if(cm_socket_send_to(SFd, (void*)&iRetVal, sizeof(iRetVal),
                  0, PeerIp,PeerPort) == OF_FAILURE)
  {
     Httpd_Error("cm_socket_send_to returned failure");
      return OF_FAILURE;
  }
  return iRetVal;

} /* HttpProcessLoopBackData */


/***************************************************************************
 * Function Name  : HttpCreateSockAndBind
 * Description    :
 *
 * Input          :
 * Output         : None
 * Return value   :
 ***************************************************************************/
int32_t HttpCreateSockAndBind(HttpGlobals_t  *pHttpGlobals,HttpPortInfo_t PortInfo)
{
  int32_t          iIndex;

#ifndef OF_CM_SUPPORT  
#ifdef INTOTO_IPV6_SUPPORT
  IGWSOCKHANDLE    Ipv6SFd = -1;
  IGWIpAddr_t      Ipv6IpAddr={};
  #else
  IGWSOCKHANDLE    SFd = -1;
  IGWIpAddr_t      Ipv4IpAddr={}; 
#endif /*INTOTO_IPV6_SUPPORT*/
#endif /*OF_CM_SUPPORT*/

  cm_sock_handle    SFd = -1;
  ipaddr      Ipv4IpAddr; 

  if(!pHttpGlobals)
  {
    Httpd_Error("pHttpGlobals is NULL\r\n");	
    return OF_FAILURE;
  }
  
  if (!((PortInfo.eServerType == hServerNormal) ||
       (PortInfo.eServerType == hServerSSL)))
  {
    HttpDebug(("Invalid server type given\n\r"));
    return HTTP_ERR_INVALID_SERVER_TYPE;
  }
  if (PortInfo.usPort == 0)
  {
    HttpDebug(("Invalid port number given\n\r"));
    return HTTP_ERR_INVALID_PORT_NUM;
  }
    
#ifdef INTOTO_IPV6_SUPPORT
#ifndef OF_CM_SUPPORT
  Ipv6SFd = cm_socket_create(CM_IPPROTO_TCP, UCM_INET_IPV6);
  if(Ipv6SFd < 0)
  {
    HttpDebug(("Unable to create the socket\n\r"));
    return HTTP_ERR_SOCKET_CREATE_FAIL;
  }

  /* cm_set_sock_reuse_opt(Ipv6SFd); */
  Ipv6IpAddr.ip_type_ul=UCM_INET_IPV6;
  if (cm_socket_bind (Ipv6SFd,Ipv6IpAddr,PortInfo.usPort) == OF_FAILURE)
  {
    HttpDebug(("Unable to bind the socket\n\r"));
    cm_socket_close (Ipv6SFd);
    return HTTP_ERR_BIND_FAIL;
  }

  if (cm_socket_listen(Ipv6SFd, pHttpGlobals->HttpdConfig.ulMaxPendReqs) != OF_SUCCESS)
  {
    HttpDebug(("Unable To Listen\r\n"));
    cm_socket_close(Ipv6SFd);
    return HTTP_ERR_LISTEN_FAIL;
  }
#endif /* OF_CM_SUPPORT */
#else  

  /*Ipv4IpAddr.ip_type_ul=cm_inet_ipv4;*/
  Ipv4IpAddr=HTTPD_INADDR_ANY;
  /*SFd = cm_socket_create(CM_IPPROTO_TCP,Ipv4IpAddr.ip_type_ul);*/
  SFd = cm_socket_create(CM_IPPROTO_TCP);
  if(SFd < 0)
  {
    HttpDebug(("Unable to create the socket\n\r"));
    return HTTP_ERR_SOCKET_CREATE_FAIL;
  }
  cm_set_sock_reuse_opt(SFd);
  if (cm_socket_bind (SFd,Ipv4IpAddr,PortInfo.usPort) == OF_FAILURE)
  {
    HttpDebug(("Unable to bind the socket\n\r"));
    cm_socket_close (SFd);
    return HTTP_ERR_BIND_FAIL;
  }

  if (cm_socket_listen(SFd, pHttpGlobals->HttpdConfig.ulMaxPendReqs) != OF_SUCCESS)
  {
    HttpDebug(("Unable To Listen\r\n"));
    cm_socket_close(SFd);
    return HTTP_ERR_LISTEN_FAIL;
  }
#endif /*INTOTO_IPV6_SUPPORT*/

  HttpDebug(("port bind success. runing on port %d\n\r",PortInfo.usPort));

  for(iIndex = 0; iIndex < pHttpGlobals->iMaxServers; iIndex++)
  {
    if((pHttpGlobals->HttpServers[iIndex].SockFd <= 0)&&
       (pHttpGlobals->HttpServers[iIndex].uiPort <= 0))
    {
      pHttpGlobals->HttpServers[iIndex].uiPort         = PortInfo.usPort;
      pHttpGlobals->HttpServers[iIndex].bEnabled       = PortInfo.bEnabled;
      pHttpGlobals->HttpServers[iIndex].Type           = PortInfo.eServerType;
#ifndef OF_CM_SUPPORT
#ifdef INTOTO_IPV6_SUPPORT
        pHttpGlobals->HttpServers[iIndex].SockFd         = Ipv6SFd;      	
#else      	
        pHttpGlobals->HttpServers[iIndex].SockFd         = SFd;
#endif /*INTOTO_IPV6_SUPPORT*/      	
#endif /*OF_CM_SUPPORT*/      	

#ifdef OF_CM_SUPPORT
        pHttpGlobals->HttpServers[iIndex].SockFd         = SFd;
#endif /*OF_CM_SUPPORT*/      	

        if (PortInfo.eServerType == hServerNormal)
        {
          /**
           * Initialize the http related things.
           */
          pHttpGlobals->HttpServers[iIndex].ClientAccept   = HttpDef_ClientAccpet;
          pHttpGlobals->HttpServers[iIndex].ClientRead     = HttpDef_ClientRead;
          pHttpGlobals->HttpServers[iIndex].ClientWrite    = HttpDef_ClientWrite;
          pHttpGlobals->HttpServers[iIndex].ClientClose    = HttpDef_ClientClose;
        }
/*#ifdef HTTPS_ENABLE*/
       else if (PortInfo.eServerType == hServerSSL)
       {
	        /**
	         * Initialize the https related things.
	         */
	        Https_Init(&pHttpGlobals->HttpServers[iIndex]);
       }
/*#endif*/ /* HTTPS_ENABLE */
        return OF_SUCCESS;
    }
  }/*end of for loop*/

  return OF_SUCCESS;
} /* HttpUpdatePort */

/***************************************************************************
 * Function Name  : DeletePortFromServer
 * Description    :
 *
 * Input          :
 * Output         : None
 * Return value   :
 ***************************************************************************/
int32_t DeletePortFromServer(uint16_t usPort)
{
  /*int32_t          iRetVal = 0;*/
  HttpPortInfo_t   PortInfo = {};
  int32_t          iIndex = 0;
  uint16_t         Count = 0;
  HttpGlobals_t       *pHttpg=NULL;

  PortInfo.usPort = usPort;
#ifndef OF_CM_SUPPORT
  iRetVal = HttpGetPortInfo(&PortInfo);
  if(iRetVal != OF_SUCCESS)
  {
    HttpDebug(("Port not found %d\n\r",PortInfo.usPort));
    return HTTP_ERR_PORT_NOT_FOUND;
  }
#endif /*OF_CM_SUPPORT*/
 pHttpg=pHTTPG;
 if(!pHttpg)
 {
    Httpd_Error("pHttpg is NULL\r\n");
    return OF_FAILURE;
 }
  for(iIndex = 0; iIndex < pHttpg->iMaxServers; iIndex++)
  {
    if(pHttpg->HttpServers[iIndex].bEnabled == TRUE)
    {
      Count++;
      if(Count > 1)
      	break;
    }
  }
  if(Count < 2)
  	return HTTP_ERR_ONE_SERVER_REQUIRED;

  for(iIndex = 0; iIndex < pHttpg->iMaxServers; iIndex++)
  {
    if(pHttpg->HttpServers[iIndex].uiPort == PortInfo.usPort)
    {
      HttpDebug(("closing the port = %d and sfd = %d\n\r",
                pHttpg->HttpServers[iIndex].uiPort,
                pHttpg->HttpServers[iIndex].SockFd));
      if(pHttpg->HttpServers[iIndex].SockFd > 0)
      {
        cm_socket_close(pHttpg->HttpServers[iIndex].SockFd);
      }
      pHttpg->HttpServers[iIndex].uiPort = 0;
      pHttpg->HttpServers[iIndex].SockFd = -1;
      pHttpg->HttpServers[iIndex].bEnabled = FALSE;
    }
  }

  HttpCookie_DelAllByPort(PortInfo.usPort);

  return OF_SUCCESS;
}

/***************************************************************************
 * Function Name  : DeletePortFromServer
 * Description    :
 *
 * Input          :
 * Output         : None
 * Return value   :
 ***************************************************************************/
int32_t ChangePortInfo(HttpPortArgs_t PortArgs)
{
  HttpPortInfo_t      PortInfo = {};
  int32_t             iRetVal = 0;
  /*char             pErrMsg[100] = {};*/
  int32_t             iIndex = 0;
  uint16_t            Count = 0;
  HttpGlobals_t       *pHttpg=NULL;

  PortInfo.usPort = PortArgs.usOldPort;
#ifndef OF_CM_SUPPORT
  iRetVal = HttpGetPortInfo(&PortInfo);
  if(iRetVal != OF_SUCCESS)
  {
    HttpDebug(("Port not configured %d\n\r",PortArgs.usOldPort));
    iRetVal = HTTP_ERR_PORT_NOT_CONFIGURED;
    return iRetVal;
  }
#endif /*OF_CM_SUPPORT*/
  if(PortArgs.PortInfo.usPort != PortArgs.usOldPort)
  {
    HttpDebug(("Changing port is not allowed. old= %d, new = %d\n\r",
              PortArgs.usOldPort, PortArgs.PortInfo.usPort));
    iRetVal = HTTP_ERR_PORT_CHANGE_NOT_ALLOWED;
    return iRetVal;
  }

  if(PortArgs.PortInfo.eServerType != PortInfo.eServerType)
  {
    HttpDebug(("Server type change is not allowd.\n\r"));
    iRetVal = HTTP_ERR_TYPE_CHANGE_NOT_ALLLOWED;
    return iRetVal;
  }

  if(PortArgs.PortInfo.bEnabled != PortInfo.bEnabled)
  {
    /*
   * Atleast one port must be configured.
   */
    pHttpg=pHTTPG;
    if(!pHttpg)
    {
      Httpd_Error("pHttpg is NULL\r\n");
      return OF_FAILURE;
    }
    if(PortArgs.PortInfo.bEnabled == FALSE)
    {
      for(iIndex = 0; iIndex < pHttpg->iMaxServers; iIndex++)
      {
        if(pHttpg->HttpServers[iIndex].bEnabled == TRUE)
        {
          Count++;
          if(Count > 1)
          	break;
        }
      }
      if(Count < 2)
      	return HTTP_ERR_ONE_SERVER_REQUIRED;
    } /* end of if(PortArgs.PortInfo.bEnabled == FALSE) */

    for(iIndex = 0; iIndex < pHttpg->iMaxServers; iIndex++)
    {
      if(pHttpg->HttpServers[iIndex].uiPort == PortArgs.PortInfo.usPort)
      {
        pHttpg->HttpServers[iIndex].bEnabled = PortArgs.PortInfo.bEnabled;
#ifndef OF_CM_SUPPORT
        iRetVal = HttpChangePortInfoInList(&PortArgs);
        if(iRetVal != OF_SUCCESS)
        {
          HttpGetErrorMsg(iRetVal,pErrMsg);
          HttpDebug((pErrMsg));
        }
#endif /*OF_CM_SUPPORT*/

        HttpCookie_DelAllByPort(PortArgs.PortInfo.usPort);
      }
    }/*end of for loop*/
  }/* end of if(PortArgs.PortInfo.bEnabled != PortInfo.bEnabled)*/
  else
  {
    iRetVal = HTTP_ERR_NOTHING_TO_MODIFY;
    return iRetVal;
  }
  return OF_SUCCESS;
}

/**************************************************************************
 * Function Name : HttpCreateLoopBackSocket                               *
 * Description   : This functions will create the loop back socket and    *
 *                 and binds it on HTTP_LOOPBACK_PORT_SRV                 *
 * Input         : None                                                   *
 * Output        : None                                                   *
 * Return value  : Socket FD upon successffully creation of the socket and*
 *                 binding to the HTTP_LOOPBACK_PORT_SRV port or OF_FAILURE*
 **************************************************************************/
 int32_t HttpCreateLoopBackSocket()
{
  int32_t LpBkSFd = 0;

  LpBkSFd = cm_socket_create(CM_IPPROTO_UDP);
  if(LpBkSFd == OF_FAILURE)
  {
    HttpDebug(("Unable to create Loop back socket\n\r"));
    return OF_FAILURE;
  }
  cm_set_sock_reuse_opt(LpBkSFd);
  if (cm_socket_bind (LpBkSFd, HTTP_LOOPBACK_ADDR, HTTP_LOOPBACK_PORT_SRV) == OF_FAILURE)
  {
    HttpDebug(("Unable to bind the socket\n\r"));
    cm_socket_close (LpBkSFd);
    return OF_FAILURE;
  }
  return LpBkSFd;
}

/**************************************************************************
 * Function Name : HttpClientSocketClose                                  *
 * Description   : This function redirects the http server on changed IP  *
 *                 address.                                               *
 * Input         : pHttpRequest (I) - http request structure.             *
 * Output        : None                                                   *
 * Return value  : OF_SUCCESS on success.                                  *
 *                 OF_FAILURE on failure.                                  *
 **************************************************************************/

int32_t HttpClientSocketClose( HttpRequest  *pHttpRequest )
{
  HttpClientReq_t  *pClientReq   = NULL;

  /**
   * Validate the passed parameter(s).
   */
  if (!pHttpRequest)
  {
    return OF_FAILURE;
  }

  /**
   * Get the client request from the passed http request parameter and
   * validate it.
   */
  pClientReq = (HttpClientReq_t *) pHttpRequest->pPvtData;

  if (!pClientReq)
  {
    return OF_FAILURE;
  }

  /**
   * Close the socket.
   */
  return cm_socket_close(pClientReq->SockFd);
} /* HttpClientSocketClose() */

/**************************************************************************
 * Function Name : HttpWriteToTransmitBuffer                              *
 * Description   : This function will check if the last node of the       *
 *                 transmit buffer has any space left and put as much data*
 *                 as possible (pReq->pTransmitBuffer). If not, allocate  *
 *                 memory for the new node, copy the data to it and attach*
 *                 it to transmit buffer linked list. Updates the         *
 *                 uiTxBufLen in the pReq and return the number of bytes  *
 *                 stored in the transmit buffer                          *
 * Input         : pReq - http request structure.                         *
 *                 pBuf - Pointer to the data to be sent                  *
 *                 uiTxBufLen - Len of the data in pBuf                   *
 *                 pHttpGlobals  - Global pointer                         *    
 * Output        : None                                                   *
 * Return value  : Number of bytes written to the Tx buffer up on success *
 *                 OF_FAILURE on failure.                                  *
 **************************************************************************/

int32_t HttpWriteToTransmitBuffer (
                                 HttpGlobals_t *pHttpGlobals, 
                                 HttpClientReq_t  *pClientReq,
                                 unsigned char         *pBuf,
                                 uint32_t         uiTxBufLen)
{
  int32_t                 iNumOfBytes = 0;
  int32_t                 iOffset = 0;
  int32_t                 iBufLen = 0;
  HttpTransmitBuffer_t    *pLastNode;
  HttpTransmitBuffer_t    *pTemp;

  if(!pHttpGlobals)
  {
    Httpd_Error("pHttpGlobals is NULL\r\n");	
    return OF_FAILURE;
  }
  /* Input validations */
  if((pClientReq == NULL) || (pBuf == NULL))
  {
    return OF_FAILURE;
  }

  /* if data len == 0, then nothing to do. */
  if(uiTxBufLen == 0)
  {
    return OF_SUCCESS;
  }

  if(pClientReq->pTransmitBuffer == NULL)
  {
    /*
     * There is no data to be transmitted till now.
     * allocate memory for Transmit buffer
     */
    pClientReq->pTransmitBuffer = HttpAllocTxBuf(pHttpGlobals);
	if(pClientReq->pTransmitBuffer == NULL)
    {
      /**
       * Couldnot allocate memory.
       */
      return OF_FAILURE;
    }
    pLastNode = pClientReq->pTransmitBuffer;
  }
  else
  {
    /**
     * Data is already available in the pClientReq to send.
     * store the last node address in the pLastNode. later append
     * the data to pLastNode
     */
     pLastNode = pClientReq->pTransmitBuffer;
     while(pLastNode->pNext != NULL)
     {
       pLastNode = pLastNode->pNext;
     }

     /**
      * If there is no space left in the last node then allocate new node
      * and attach it to the Tx. buffers.
      */
     if((pClientReq->uiTxBufLen % pHttpGlobals->HttpdConfig.uiMaxDataBuffer) == 0)
     {
       pTemp = HttpAllocTxBuf(pHttpGlobals);
       if(pTemp == NULL)
       {
         return OF_FAILURE;
       }
       pLastNode->pNext = pTemp;
       pLastNode = pTemp;
     }
  }

  /*
   * pClientReq->uiTxBufLen - will represent the total length of data in the
   *                    Tx.Buffer and not the data to be sent.
   * pClientReq->uiOffset - will represent Offset of the data sent in tansmit buffer.
   * Data to be sent will be calculated using (pClientReq->uiTxBufLen - pClientReq->uiOffset)
   * Where to append data? traverse to the last node(pLastNode) then calculate
   *     the offset using the following formula.
   *     (pHttpGlobals->HttpdConfig.uiMaxDataBuffer - pClientReq->uiTxBufLen % pHttpGlobals->HttpdConfig.uiMaxDataBuffer)
   */
  iBufLen = uiTxBufLen;
  while(iBufLen > 0)
  {
    iOffset = pHttpGlobals->HttpdConfig.uiMaxDataBuffer - pClientReq->uiTxBufLen % pHttpGlobals->HttpdConfig.uiMaxDataBuffer;
    iNumOfBytes = HTTP_FIND_MIN(iBufLen, iOffset);
    of_memcpy(&pLastNode->pDataBuffer[pHttpGlobals->HttpdConfig.uiMaxDataBuffer - iOffset],
    	     &pBuf[uiTxBufLen - iBufLen],
    	     iNumOfBytes);
    iBufLen -= iNumOfBytes;
    pClientReq->uiTxBufLen += iNumOfBytes;
    if(iBufLen > 0)
    {
      pTemp = HttpAllocTxBuf(pHttpGlobals);
      if(pTemp == NULL)
      {
        return OF_FAILURE;
      }
      pLastNode->pNext = pTemp;
      pLastNode = pTemp;
    }
  }
  return OF_SUCCESS;
}

/**************************************************************************
 * Function Name : HttpSendToClient                                       *
 * Description   : This function will send HTTP_MAX_TRANSMIT_BUFFER size  *
 *                 of data for every write event on the FD. If the whole  *
 *                 data is transmitted then the client request (pReq) will*
 *                 be deleted. If the whole data in a node of transmit    *
 *                 buffer linked list is sent then that node will be      *
 *                 deleted from the list.                                 *
 * Input         : iSockFd - Data to be sent for this FD from transmit    *
 *                 buffer                                                 *
 *                 pHttpGlobals  - Global pointer                         *
 * Output        : None                                                   *
 * Return value  : OF_SUCCESS on success.                                  *
 *                 OF_FAILURE on failure.                                  *
 **************************************************************************/
 int32_t HttpSendToClient (HttpGlobals_t *pHttpGlobals,HttpClientReq_t *pClientReq,int32_t cProcessRetVal)
{
  int32_t                     iLoopVar = 0;
  HttpTransmitBuffer_t   *pTemp = NULL;
  int32_t                      iBytesToSend = 0;
  int32_t                  	 iTxBytes = 0;
  unsigned char           	*pBuf=NULL;
  int32_t                 	  iIndexVal=0;
  int32_t 		          iSockFd=0;
  bool                       bFlag=FALSE;
  
  if(!pHttpGlobals)
  {
    Httpd_Error("pHttpGlobals is NULL\r\n");	
    return OF_FAILURE;
  }
  
  if(pClientReq == NULL)
  {
        HttpDebug((" pClientReq is NULL\r\n"));        
        return OF_FAILURE;
  }
  if(pClientReq->SockFd  <= 0)
  {
        HttpDebug((" pClientReq->SockFd  is not valid\r\n"));        
        return OF_FAILURE;
  }

/****************** SENDING HEADER START ***************************/
/**
  *  We need to supply Content-Length in HTTP/1.1 header. we are not filling the header
  *  information  in TransmitBuffers. A seperate Buffer will be filled with the header information
  *  and this  will be send to the client. This change is made for content-length field, this change   
  *  will avoid  Data Length related problems for static  and Dynamic pages.
  */
 if(pClientReq->ulStatus!=OF_FAILURE) 
 {
     if(!pClientReq->pSendHeader)
     {
       pClientReq->pSendHeader = (char *)pClientReq->ucHeaderBuff;
       if(HttpClient_SendHeader(pHttpGlobals,pClientReq)==OF_FAILURE)
       {
         HttpDebug((" HttpClient_SendHeader is failed\r\n")); 
         return OF_FAILURE;
       }
     }
     if(pHttpGlobals->HttpServers[pClientReq->lServId].Type == hServerNormal)
     {
  	  iTxBytes = cm_socket_send(pClientReq->SockFd, pClientReq->pSendHeader,
                                   of_strlen(pClientReq->pSendHeader), 0);
          if(iTxBytes != of_strlen((char *) pClientReq->ucHeaderBuff))
          {
           /*partial write*/
           pClientReq->pSendHeader += iTxBytes;
  	   return OF_SUCCESS;           
          } 
  	   /*Unable to send data. What could be wrong?*/
         if(iTxBytes < 0)
    	  {
  	  	  /**
  	  	   * Check if the send() has set the EWOULDBLOCK error code.
  	  	   */
  	  	   if (HttpdSocketValidateErrorno() == TRUE)
  	  	   {
  		  	     /*
  		  	      * TCP buffer is full, try after some time.
  		  	      * NOTE: This case may not araise as http server will try to write
  		  	      *       only when this FD has a write event
  		  	      */
  		  	     return OF_SUCCESS;
  	  	   }
    	  }
     }	  
    #ifdef OF_HTTPS_SUPPORT
     else if(pHttpGlobals->HttpServers[pClientReq->lServId].Type == hServerSSL)
     {
		  iTxBytes = IGWSSL_write((SSL*)pClientReq->pSpData,
							   pClientReq->ucHeaderBuff,
							  of_strlen((char *) pClientReq->ucHeaderBuff));
		  /*Unable to send data. What could be wrong?*/
		  if(iTxBytes < 0)
		  {
			int32_t  iRetVal = 0;
	  
			/*Get the error value*/
			iRetVal = IGWSSL_get_error((SSL*)pClientReq->pSpData, iTxBytes);
			/*is it read or write error, then return success*/
			if ((iRetVal == SSL_ERROR_WANT_READ) ||
				 (iRetVal == SSL_ERROR_WANT_WRITE))
			{
			  return OF_SUCCESS;
			}
		  }
    }
   #endif /*OF_HTTPS_SUPPORT*/
    pClientReq->ulStatus=OF_FAILURE;
  }
  /****************** SENDING HEADER END ***************************/
  
  iIndexVal=pClientReq->iPipeLineTableIndex;
  iLoopVar=pClientReq->iPipeLineSubIndex;  
  iSockFd=pClientReq->SockFd;
  
  if(iLoopVar <0)
 {
    iLoopVar=pClientReq->iReqTableIndex;
    bFlag=TRUE;
 } 
  if(pClientReq->pTransmitBuffer == NULL)
  {
         HttpDebug(" pClientReq->pTransmitBuffer is NULL");
         return OF_FAILURE; 
  }
  if(pClientReq->pTransmitBuffer->pDataBuffer == NULL)
  {
      HttpDebug("pClientReq->pTransmitBuffer->pDataBuffer is NULL");
      return OF_FAILURE;
  }

  pBuf = pClientReq->pTransmitBuffer->pDataBuffer;
  if(pClientReq->uiTxBufLen <= pHttpGlobals->HttpdConfig.uiMaxDataBuffer)
  {
    iBytesToSend = HTTP_FIND_MIN((pClientReq->uiTxBufLen - pClientReq->uiTxBufOffset),
    	                          pHttpGlobals->HttpdConfig.uiMaxTxBuffer);
  }
  else
  {
    iBytesToSend = HTTP_FIND_MIN((pHttpGlobals->HttpdConfig.uiMaxDataBuffer - pClientReq->uiTxBufOffset),
                                  pHttpGlobals->HttpdConfig.uiMaxTxBuffer);
  }
  /**
   * What type of server it is? HTTP ot HTTPS? Depending on type
   * call the relevent function cm_socket_send or IGWSSL_write.
   */
  if(pHttpGlobals->HttpServers[pClientReq->lServId].Type == hServerNormal)
  {
    iTxBytes = cm_socket_send(iSockFd, (char *) &pBuf[pClientReq->uiTxBufOffset],
    	                     iBytesToSend,0);
    /*Unable to send data. What could be wrong?*/
    if(iTxBytes < 0)
    {
      /**
       * Check if the send() has set the EWOULDBLOCK error code.
       */
       if (HttpdSocketValidateErrorno() == TRUE)
       {
         /*
          * TCP buffer is full, try after some time.
          * NOTE: This case may not araise as http server will try to write
          *       only when this FD has a write event
          */
         return OF_SUCCESS;
       }
    }

  }
#ifdef OF_HTTPS_SUPPORT
  else if(pHttpGlobals->HttpServers[pClientReq->lServId].Type == hServerSSL)
  {
    iTxBytes = IGWSSL_write((SSL*)pClientReq->pSpData,
    	                &pBuf[pClientReq->uiTxBufOffset],
    	                iBytesToSend);
    /*Unable to send data. What could be wrong?*/
    if(iTxBytes < 0)
    {
      int32_t  iRetVal = 0;

      /*Get the error value*/
      iRetVal = IGWSSL_get_error((SSL*)pClientReq->pSpData, iTxBytes);
      /*is it read or write error, then return success*/
      if ((iRetVal == SSL_ERROR_WANT_READ) ||
           (iRetVal == SSL_ERROR_WANT_WRITE))
      {
        return OF_SUCCESS;
      }
    }
  }
#endif /*OF_HTTPS_SUPPORT*/
  else
  {
    /*Invalid type. remove the client request*/
   if(HttpFreeClientReqAndTxBuf(pHttpGlobals,pClientReq)!=OF_SUCCESS)
   {
       HttpDebug("HttpFreeClientReqAndTxBuf is returned failure");
       return OF_FAILURE;
   }
   if(bFlag==TRUE)
   {
     pHttpGlobals->pHttpReqTable[iLoopVar]=NULL;
   }
   else
   {
      pHttpGlobals->pHttpClientReqTable[iIndexVal].pHttpPipeline[iLoopVar]= NULL;        
   }
    HttpDebug("No servers configured");
    return OF_FAILURE;
  }

  /*Unable to send data*/
  if(iTxBytes < 0)
  {
     /**
      * there is an error in sending. Remove all the Tx.buffers and
      * remove the client request
      */
      if(HttpFreeClientReqAndTxBuf(pHttpGlobals,pClientReq)!=OF_SUCCESS)
     {
         HttpDebug("HttpFreeClientReqAndTxBuf is returned failure");
         return OF_FAILURE;
     }
     if(bFlag==TRUE)
     {
         pHttpGlobals->pHttpReqTable[iLoopVar]=NULL;
     }
     else
     {
          pHttpGlobals->pHttpClientReqTable[iIndexVal].pHttpPipeline[iLoopVar] = NULL;       
     }
     HttpDebug("Unable to send data ,iTxBytes < 0");
     return OF_FAILURE;
  }

  /**
   * Data has sent succesfully. update the pClientReq->uiTxBufLen
   * and pClientReq->uiTxBufOffset
   * some times it may not send the requested bytes to send.
   * The number of bytes sent will be the return value of cm_socket_send
   * or IGWSSL_write
   */
  pClientReq->uiTxBufOffset += iTxBytes;

  /* Check whether whole data is transmitted??*/
  if(pClientReq->uiTxBufOffset >= pClientReq->uiTxBufLen)
  { 
#ifdef INTOTO_CMS_SUPPORT
     if(cProcessRetVal == HTTP_DONOT_SEND_RESPONCE)  
     {	     
        return OF_SUCCESS;
     }
     else if(pClientReq->bDoNotRemoveRequest)
     {	     
         /*Reset the request flag*/
        pClientReq->bDoNotRemoveRequest = FALSE;
        return OF_SUCCESS;
     }
#endif  /* INTOTO_CMS_SUPPORT */
    /* if(cProcessRetVal == HTTP_DONOT_SEND_RESPONCE)	  
     {	     
     	return OF_SUCCESS;
     }*/
     /*Yes whole data sent*/
     if(HttpFreeClientReqAndTxBuf(pHttpGlobals,pClientReq)!=OF_SUCCESS)
     {
         HttpDebug("HttpFreeClientReqAndTxBuf is returned failure");
         return OF_FAILURE;
     }
     if(bFlag==TRUE)
     {
          pHttpGlobals->pHttpReqTable[iLoopVar]=NULL;
     }
     else
     {
          pHttpGlobals->pHttpClientReqTable[iIndexVal].pHttpPipeline[iLoopVar]= NULL;         
     }
    return OF_SUCCESS;
  }
  else
  /*whole data in first node transmitted*/
  if(pClientReq->uiTxBufOffset >= pHttpGlobals->HttpdConfig.uiMaxDataBuffer)
  {
       pClientReq->uiTxBufOffset = 0;
       pClientReq->uiTxBufLen -= pHttpGlobals->HttpdConfig.uiMaxDataBuffer;
       pTemp = pClientReq->pTransmitBuffer;
       pClientReq->pTransmitBuffer = pTemp->pNext;
       if(HttpFreeTxBuf(pTemp)!=OF_SUCCESS)
       {
           HttpDebug("HttpFreeTxBuf is returned failure");
           return OF_FAILURE;
       }
       return OF_SUCCESS;
  }
  else
  {
       /*partial data in the first node sent*/
       return OF_SUCCESS;
  }
}

/**************************************************************************
 * Function Name : HttpAllocTxBuf                                         *
 * Description   : This function will allocate memory for Tx buffer       *
 * Input         : pHttpGlobals  - Global pointer                         *
 * Output        : None                                                   *
 * Return value  : pointer to the memory allocated on success.            *
 *                 NULL on failure                                        *
 **************************************************************************/

 HttpTransmitBuffer_t* HttpAllocTxBuf(HttpGlobals_t  *pHttpGlobals)
{
  HttpTransmitBuffer_t   *pTemp;

  if(!pHttpGlobals)
  {
    Httpd_Error("pHttpGlobals is NULL\r\n");	
    return NULL;
  }

  pTemp = of_calloc(1,sizeof(HttpTransmitBuffer_t));
  if(pTemp != NULL)
  {
    pTemp->pDataBuffer = of_calloc(1,pHttpGlobals->HttpdConfig.uiMaxDataBuffer);
    if(pTemp->pDataBuffer == NULL)
    {
      of_free(pTemp);
      return NULL;
    }
  }
  return pTemp;
}
/***********************************************************************
 * Function Name : Httpd_ParseBuffer             	              *
 * Description   : This is a wrapper for parsing  the raw buffer      *
 * Input         : raw buffer					      *
 * Output        :clientreq structure												*
 * Return value  : clientreq structure			      *
 ***********************************************************************/

void Httpd_ParseBuffer(unsigned char *pBuf, HttpClientReq_t *pClientReq)

{
  unsigned char             *tmpBuf,*Temp;
  unsigned char             *aFieldName,*aFieldVal;
  HttpGlobals_t           *pHttpg=NULL;

  tmpBuf=pBuf;
  if((!pBuf)||(!pClientReq))
  {
     Httpd_Print("Invalid Parameters");
     return;    
  }
  if (!of_strcmp((char *) pBuf, ""))
  {
     Httpd_Print("no buffer");
     return;
  }
  pHttpg=(HttpGlobals_t *)pClientReq->pGlobalData;
  pBuf = (unsigned char *) of_strstr((char *) pBuf, CRLF);
  if (pBuf) *pBuf = '\0';
  do
  {
  	pBuf  += of_strlen(CRLF);
     	tmpBuf  = pBuf;
     	pBuf   = (unsigned char *) of_strstr((char *) pBuf, CRLF);
     	if (pBuf)
      	{
          *pBuf = '\0';
      }

  Temp = (unsigned char *) of_strchr((char *) tmpBuf, ':');
  *Temp++ = '\0';
  while (isspace(*Temp))
  {
    ++Temp;
  }
  aFieldName=tmpBuf;
  aFieldVal=(unsigned char *) Temp;
 if (!strcasecmp((char *) aFieldName, "content-length"))
  {
          sscanf((char *) aFieldVal, "%d", &(pClientReq->lInContentLen));
  }
 else if (!strcasecmp((char *) aFieldName, "content-type"))
  {
          of_strncpy((char *) pClientReq->ucInContentType, (char *) aFieldVal, HTTP_CNTTYPE_LEN);
          pClientReq->ucInContentType[HTTP_CNTTYPE_LEN] = '\0';
  }
  else if (!strcasecmp((char *) aFieldName, "cookie"))  	
  {
       if(pClientReq->ucCookieVal!=NULL)
      	{
 of_strncpy((char *) pClientReq->ucCookieVal, (char *) aFieldVal, Httpd_GetMaxCookieValueLen(pHttpg)+ 1);
      	}
  }
  else if(!strcasecmp((char *) aFieldName, "Host"))
  {
#ifndef OF_CM_SUPPORT
	#ifdef INTOTO_IPV6_SUPPORT
	if(pClientReq->PeerIp.ip_type_ul == IGW_INET_IPV6)
	{
           of_memcpy(pClientReq->PeerIp.IPv6Addr8,aFieldVal,IGW_IPV6_ADDR_LEN);
	}
	else
	#endif /*INTOTO_IPV6_SUPPORT*/
	{
           pClientReq->PeerIp.IPv4Addr=(uint32_t)aFieldVal;
	}
#endif /*OF_CM_SUPPORT*/
           pClientReq->PeerIp=(uint32_t)aFieldVal;
  }


 } while (pBuf);


} /* Httpd_ParseBuffer() */

/**************************************************************************
 * Function Name : HttpFreeTxBuf                                          *
 * Description   : This function will free the memory for Tx buffer       *
 * Input         : pTxBuf - Buffer to be freed.                           *
 * Output        : None                                                   *
 * Return value  : T_SUCESS if successfull freed the memory               *
 *                 OF_FAILURE on failure                                   *
 **************************************************************************/

/*   */ int32_t HttpFreeTxBuf(HttpTransmitBuffer_t *pTxBuf )
{
  if(pTxBuf == NULL)
    return OF_FAILURE;

  if(pTxBuf->pDataBuffer != NULL)
  {
    of_free(pTxBuf->pDataBuffer);
  }
  of_free(pTxBuf);
  return OF_SUCCESS;
}

/**************************************************************************
 * Function Name : HttpFreeClientReqAndTxBuf                              *
 * Description   : This function will free all the memory in Tx.Buffers   *
 *                 and deletes the pClientRequest.                        *
 * Input         : pClientReq - Pointer to the client request             *
 *                 pHttpGlobals  - Global pointer                         *
 * Output        : None                                                   *
 * Return value  : T_SUCESS if successfull freed the memory               *
 *                 OF_FAILURE on failure                                   *
 **************************************************************************/

int32_t HttpFreeClientReqAndTxBuf(HttpGlobals_t  *pHttpGlobals,HttpClientReq_t *pClientReq)
{
  HttpTransmitBuffer_t  *pTemp;
  HttpTransmitBuffer_t  *pNext;

  if(!pHttpGlobals)
  {
    Httpd_Error("pHttpGlobals is NULL\r\n");	
    return OF_FAILURE;
  }
  
  if(pClientReq == NULL)
    return OF_FAILURE;

  /*Check if there is any data in Tx.Buffer and free them*/
  pTemp = pClientReq->pTransmitBuffer;
  while(pTemp != NULL)
  {
    pNext = pTemp->pNext;
    HttpFreeTxBuf(pTemp);
    pTemp = pNext;
  }
  pClientReq->pTransmitBuffer = NULL;
  if(HttpClient_Delete(pHttpGlobals,pClientReq)!=OF_SUCCESS)
  {
     HttpDebug("Client Delete is returned failure");
     return OF_FAILURE;
  }
  return OF_SUCCESS;
}
int32_t Http_LoginRegModuleAPI(HttpGlobals_t *pHttpGlobals,HttpAPIRegModuleInfo_t *pNewModule_p)
{
	HttpRegModuleRecList_t *pModule=NULL,*pPrev=NULL,*pNewModule=NULL;

  if(!pHttpGlobals)
  {
    Httpd_Error("pHttpGlobals is NULL\r\n");	
    return OF_FAILURE;
  }
  
  if(!pNewModule_p)
  {
    HttpDebug(("Http_LoginRegModuleAPI :: pNewModule_p is NULL \n"));
    return OF_FAILURE;
  }

  pNewModule = HttpRegModuleAlloc(pHttpGlobals);
  if (pNewModule== NULL)
  {
    HttpDebug(("Http_LoginRegModuleAPI : of_calloc failed\n"));
    return OF_FAILURE;
  }

  pNewModule->ulModuleId = pHttpGlobals->ucRegModuleId_g++;
  pNewModule->ucPriority= pNewModule_p->ucPriority;
  of_strcpy(pNewModule->ModuleName,pNewModule_p->ModuleName);
  of_strcpy((char *) pNewModule->Loginfile,(char *) pNewModule_p->Loginfile);
  of_strcpy((char *) pNewModule->Logoutfile,(char *) pNewModule_p->Logoutfile);

 /* Insert the Register module in  to a priority queue */
  for (pPrev = pModule = pHttpGlobals->pHttpRegModuleHead_g; pModule;pModule=pModule->pNextModule)
  {
    if(pModule->ucPriority > pNewModule->ucPriority)
    {
      break;
    }
    pPrev = pModule;
  }
  pNewModule->pNextModule= pModule;
  if (pPrev == pModule) /* First node */
  {
    pHttpGlobals->pHttpRegModuleHead_g = pNewModule;
  }
  else
  {
    pPrev->pNextModule = pNewModule;
  }
  return OF_SUCCESS;
}

int32_t Http_LoginDeRegModuleAPI(HttpGlobals_t *pHttpGlobals,char  *pModuleName)
{
    HttpRegModuleRecList_t *pModule=NULL;

    if(!pHttpGlobals)
    {
      Httpd_Error("pHttpGlobals is NULL\r\n");	
      return OF_FAILURE;
    }
    if(!pModuleName)
    {
	 HttpDebug(("Http_LoginDeRegModuleAPI :: pModuleName is NULL \n"));
         return OF_FAILURE;
    }

    for ( pModule = pHttpGlobals->pHttpRegModuleHead_g; pModule;pModule=pModule->pNextModule)
    {
       if(of_strcmp(pModule->ModuleName, pModuleName))
           break;
     }
     if(!pModule)
     {
	  HttpDebug(("Http_LoginDeRegModuleAPI :: Module not found \n"));
         return OF_FAILURE;
     }
     HttpRegModuleFree(pHttpGlobals,pModule);
    return OF_SUCCESS;
}
int32_t Http_UpdateRegModuleRequest(HttpGlobals_t *pHttpGlobals,HttpRegModuleRequestInfo_t *pModReqInfo)
{
    HttpRegModuleRequest_t *pRequest=NULL,*pNewRequest=NULL,*pTempRequest=NULL;
    HttpRegModuleRecList_t *pModule=NULL;
#ifdef INTOTO_IPV6_SUPPORT
    unsigned char                cPeerIp[IGW_INET6_ADDRSTRLEN];
    ipaddr                  RequestIpAddr; 
    ipaddr   		    tempIpAddr;   
#endif /*INTOTO_IPV6_SUPPORT*/

    if(!pHttpGlobals)
    {
       Httpd_Error("pHttpGlobals is NULL\r\n");	
       return OF_FAILURE;
    }
	
    if(!pModReqInfo)
    {
	 HttpDebug(("Http_UpdateRegModuleRequest :: pModReqInfo is NULL \n"));
         return OF_FAILURE;
     }

     for ( pModule = pHttpGlobals->pHttpRegModuleHead_g; pModule;pModule=pModule->pNextModule)
     {
      if(!(of_strcmp(pModule->ModuleName, pModReqInfo->ModuleName)))
         break;
     }
     if(!pModule)
     {
	 HttpDebug(("Http_UpdateRegModuleRequest :: Module not found \n"));
         return OF_FAILURE;
     }
     for (pRequest = pModule->pNext; pRequest ;pRequest =pRequest ->pNext)
     {
#ifndef OF_CM_SUPPORT
           #ifdef INTOTO_IPV6_SUPPORT
	    if(pRequest->SrcIp.ip_type_ul == IGW_INET_IPV6)
	    {
		    if(HttpdNumToPresentation(&(pRequest->SrcIp),(unsigned char *)cPeerIp)
									!=  OF_SUCCESS)
		    {
			    HttpDebug((" HttpdNumToPresentation is Failed\r\n"));
			    return OF_FAILURE;
		    }
                    IGWAtoN((char *)cPeerIp,&RequestIpAddr);
                    of_memset(cPeerIp,0,IGW_INET6_ADDRSTRLEN);  
		    if(HttpdNumToPresentation(&(pModReqInfo->SrcIp),(unsigned char *)cPeerIp)
									!=  OF_SUCCESS)
		    {
			    HttpDebug((" HttpdNumToPresentation is Failed\r\n"));
			    return OF_FAILURE;
		    }
                    IGWAtoN((char *)cPeerIp,&tempIpAddr);
  	            if(RequestIpAddr == tempIpAddr)
		    {
			    if(pRequest->SrcPort == pModReqInfo->SrcPort) 
			    {
				    return OF_FAILURE;
			    }
		    }
		    else if(!memcmp(pRequest->SrcIp.IPv6Addr8, pModReqInfo->SrcIp.IPv6Addr8,
									  IGW_IPV6_ADDR_LEN))
		    {
				if(pRequest->SrcPort == pModReqInfo->SrcPort) 
				{
				   return OF_FAILURE;
				}
		    }
	    }
	    else
	    #endif /*INTOTO_IPV6_SUPPORT*/
#endif /*OF_CM_SUPPORT*/
	    {
             	 if(pRequest->SrcIp.CMIPv4Addr == pModReqInfo->SrcIp.CMIPv4Addr)
		 {
			   if(pRequest->SrcPort == pModReqInfo->SrcPort)   
			   {
			   	return OF_FAILURE;
			   }
		 }
	    }
      }

	pNewRequest = of_calloc(1,sizeof(HttpRegModuleRequest_t));
      if (pNewRequest== NULL)
      {
        HttpDebug(("Http_UpdateRegModuleRequest : of_calloc failed\n"));
        return OF_FAILURE;
      }

       of_memcpy(&pNewRequest->SrcIp,&pModReqInfo->SrcIp,sizeof(struct cm_ip_addr));
       pNewRequest->SrcPort= pModReqInfo->SrcPort;
       pNewRequest->ReqType= pModReqInfo->ReqType;
       if(!pModule->pNext)
       {
	       pModule->pNext=pNewRequest;
	       return OF_SUCCESS;
       }
       pTempRequest = pModule->pNext;
       while(pTempRequest->pNext)
       {
       pTempRequest = pTempRequest->pNext;
       }
       pTempRequest->pNext = pNewRequest;
       return OF_SUCCESS;
}


int32_t Http_ProcessRegModuleRequest(HttpGlobals_t *pHttpGlobals,HttpClientReq_t  *pClientReq)
{
  HttpRegModuleRequest_t *pRequest=NULL,*pPrevRequest = NULL;
  HttpRegModuleRecList_t *pModule=NULL;
  bool		  	    bStatus=FALSE;
#ifdef INTOTO_IPV6_SUPPORT
  unsigned char                     cPeerIp[IGW_INET6_ADDRSTRLEN];
  ipaddr                       ReqIpAddr;
  ipaddr                       tempIpAddr;
#endif /*INTOTO_IPV6_SUPPORT*/
  if(!pHttpGlobals)
  {
    Httpd_Error("pHttpGlobals is NULL\r\n");	
    return OF_FAILURE;
  }
    for ( pModule = pHttpGlobals->pHttpRegModuleHead_g; pModule;pModule=pModule->pNextModule)
    {
        for (pRequest = pModule->pNext; pRequest ;pRequest =pRequest ->pNext)
        {
#ifndef OF_CM_SUPPORT
             #ifdef INTOTO_IPV6_SUPPORT
	     if(pClientReq->PeerIp.ip_type_ul == IGW_INET_IPV6)
	     {
		    if(HttpdNumToPresentation(&(pClientReq->PeerIp),(unsigned char *)cPeerIp)
									!=  OF_SUCCESS)
		    {
			    HttpDebug((" HttpdNumToPresentation is Failed\r\n"));
			    return OF_FAILURE;
		    }
                    IGWAtoN((char *)cPeerIp,&ReqIpAddr);
                    of_memset(cPeerIp,0,IGW_INET6_ADDRSTRLEN);  
		    if(HttpdNumToPresentation(&(pRequest->SrcIp),(unsigned char *)cPeerIp)
									!=  OF_SUCCESS)
		    {
			    HttpDebug((" HttpdNumToPresentation is Failed\r\n"));
			    return OF_FAILURE;
		    }
                    IGWAtoN((char *)cPeerIp,&tempIpAddr);
  	            if(tempIpAddr == ReqIpAddr)
		    {
			     bStatus=TRUE;
		    }
		    else if(!memcmp(pRequest->SrcIp.IPv6Addr8, pClientReq->PeerIp.IPv6Addr8,
									  IGW_IPV6_ADDR_LEN))
		    {
			     bStatus=TRUE;
		    }
            }
	    else
	    #endif /*INTOTO_IPV6_SUPPORT*/
#endif /*OF_CM_SUPPORT*/
	    {
           if(pRequest->SrcIp.CMIPv4Addr == pClientReq->PeerIp)
		    {
			     bStatus=TRUE;
		    }
	    }
            if(bStatus)
	    {
		/*  if (pRequest->SrcPort == pClientReq->PeerPort)
             	  {*/
		    if(pRequest->ReqType == HTTP_REQ_LOGIN)
             	    {
             	       of_strcpy((char *) pClientReq->ucUrl, (char *) pModule->Loginfile);
		    }
		    if(pRequest->ReqType == HTTP_REQ_IDLE_TIMEOUT)
             	       of_strcpy((char *) pClientReq->ucUrl, (char *) pModule->Logoutfile);
		    if(!pPrevRequest)
			   pModule->pNext = NULL;
		    else
             	      pPrevRequest->pNext =pRequest->pNext;
             	    if(pRequest)
		    { 
             	      of_free(pRequest);  /*freeing the request node */
             	    }
                    return OF_SUCCESS;
                /* }*/
              }
              pPrevRequest = pRequest;
          }
      }
   return OF_FAILURE;
}

/************************************************************************/
/* Function Name : HttpRegModuleInit                                           */
/* Description   : initiates free queue for modules to register with HTTP           */
/* Input         : pHttpGlobals  - Global pointer                      */
/* Output        : null                          */
/* Return value  : OF_SUCCESS or OF_FAILURE.                        */
/************************************************************************/
int32_t HttpRegModuleInit(HttpGlobals_t *pHttpGlobals)
{
    if(!pHttpGlobals)
    {
      Httpd_Error("pHttpGlobals is NULL\r\n");	
      return OF_FAILURE;
    }
    Httpd_GetRegModules(pHttpGlobals,& pHttpGlobals->HttpRegModuleFreeQMaxSize_g);/*pHttpGlobals->pHttpGlobalConfig->ulRegMaxModules;*/
    HttpRegModuleFreeQInit(pHttpGlobals);

   return OF_SUCCESS;
}


int32_t HttpRegModuleFreeQInit(HttpGlobals_t *pHttpGlobals)
{
   HttpRegModuleRecList_t *RegModule = NULL;
   uint32_t  uiIndex = 0;
   
   if(!pHttpGlobals)
   {
     Httpd_Error("pHttpGlobals is NULL\r\n");	
     return OF_FAILURE;
   }
   for(uiIndex = 0;uiIndex < pHttpGlobals->HttpRegModuleFreeQMaxSize_g; uiIndex++)
   {
      RegModule = of_calloc(1, sizeof(HttpRegModuleRecList_t));
      if(NULL == RegModule)
         return OF_FAILURE;

      HttpRegModuleFree(pHttpGlobals,RegModule);
   }
   return OF_SUCCESS;
}

HttpRegModuleRecList_t *HttpRegModuleAlloc(HttpGlobals_t *pHttpGlobals)
{
  HttpRegModuleRecList_t *RegModule = NULL;

  if(!pHttpGlobals)
  {
    Httpd_Error("pHttpGlobals is NULL\r\n");	
    return NULL;
  }
  
  if( pHttpGlobals->HttpRegModuleFreeQ.ulFreeCount == 0)
  {
    RegModule = of_calloc(1, sizeof(HttpRegModuleRecList_t));
    if(RegModule == NULL)
       return NULL;
    RegModule->bHeap = TRUE;
  }
  else
  {
    RegModule = pHttpGlobals->HttpRegModuleFreeQ.pHead;
    pHttpGlobals->HttpRegModuleFreeQ.pHead = RegModule->pNextModule;
    if(pHttpGlobals->HttpRegModuleFreeQ.pHead == NULL)
     pHttpGlobals->HttpRegModuleFreeQ.pTail = NULL;
    pHttpGlobals->HttpRegModuleFreeQ.ulFreeCount--;
    RegModule->pNextModule = NULL;
    of_memset(RegModule,0, sizeof(HttpRegModuleRecList_t));
  }
  return RegModule;
}

int32_t HttpRegModuleFree(HttpGlobals_t *pHttpGlobals,HttpRegModuleRecList_t *RegModule_p)
{
   if(!pHttpGlobals)
   {
     Httpd_Error("pHttpGlobals is NULL\r\n");	
     return OF_FAILURE;
   }
  
   if(!RegModule_p)
      return OF_FAILURE;

   if(RegModule_p->bHeap == TRUE)
   {
     of_free(RegModule_p);
     return OF_SUCCESS;
   }

   RegModule_p->pNextModule = NULL;
   of_memset(RegModule_p,0,sizeof(HttpRegModuleRecList_t));

   if(pHttpGlobals->HttpRegModuleFreeQ.pTail == NULL)
   {
      pHttpGlobals->HttpRegModuleFreeQ.pTail = pHttpGlobals->HttpRegModuleFreeQ.pHead = RegModule_p;
   }
   else
   {
     pHttpGlobals->HttpRegModuleFreeQ.pTail->pNextModule = RegModule_p;
     pHttpGlobals->HttpRegModuleFreeQ.pTail = RegModule_p;
   }
   pHttpGlobals->HttpRegModuleFreeQ.ulFreeCount++;
   return OF_SUCCESS;

}

int32_t  HttpAsyncRegisterFdCbk(HttpGlobals_t *pHttpGlobals,HttpAsyncExternFdReg_t FdCbkFnPtr)
{
  int32_t   iLoopVar = 0;

  if(!pHttpGlobals)
  {
    Httpd_Error("pHttpGlobals is NULL\r\n");	
    return OF_FAILURE;
  }
  
  if(FdCbkFnPtr.iSockFd <= 0)
  {
    return OF_FAILURE;
  }

  if((FdCbkFnPtr.pFnRead == NULL) &&
  	 (FdCbkFnPtr.pFnError == NULL) &&
  	 (FdCbkFnPtr.pFnWrite == NULL))
  {
    return OF_FAILURE;
  }

  if(FdCbkFnPtr.uiInterstedEvents & CM_FDPOLL_READ)
  {
    if(FdCbkFnPtr.pFnRead == NULL)
    {
      return OF_FAILURE;
    }
  }
  else
  {
    FdCbkFnPtr.pFnRead = NULL;
  }

  if(FdCbkFnPtr.uiInterstedEvents & CM_FDPOLL_WRITE)
  {
    if(FdCbkFnPtr.pFnWrite== NULL)
    {
      return OF_FAILURE;
    }
  }
  else
  {
    FdCbkFnPtr.pFnWrite= NULL;
  }

  if(FdCbkFnPtr.uiInterstedEvents & CM_FDPOLL_EXCEPTION)
  {
    if(FdCbkFnPtr.pFnError == NULL)
    {
      return OF_FAILURE;
   	}
  }
  else
  {
    FdCbkFnPtr.pFnError = NULL;
  }

  for(;iLoopVar < HTTP_MAX_REG_FDS; iLoopVar++)
  {
    if(pHttpGlobals->HttpExternRegFds[iLoopVar].iSockFd > 0)
    {
      continue;
    }
    else
    {
      pHttpGlobals->HttpExternRegFds[iLoopVar] = FdCbkFnPtr;
      return OF_SUCCESS;
   	}
  }

  return OF_FAILURE;
}


int32_t  HttpAsyncUnregisterFd(HttpGlobals_t *pHttpGlobals,int32_t iSockFd)
{
  int32_t   iLoopVar = 0;

  if(!pHttpGlobals)
  {
    Httpd_Error("pHttpGlobals is NULL\r\n");	
    return OF_FAILURE;
  }
  
  for(; iLoopVar < HTTP_MAX_REG_FDS; iLoopVar++)
  {
    if(pHttpGlobals->HttpExternRegFds[iLoopVar].iSockFd == iSockFd)
    {
      pHttpGlobals->HttpExternRegFds[iLoopVar].iSockFd = 0;
      pHttpGlobals->HttpExternRegFds[iLoopVar].pFnError = NULL;
      pHttpGlobals->HttpExternRegFds[iLoopVar].pFnRead = NULL;
      pHttpGlobals->HttpExternRegFds[iLoopVar].pFnWrite = NULL;
      pHttpGlobals->HttpExternRegFds[iLoopVar].uiInterstedEvents = 0;
      return OF_SUCCESS;
    }
  }

  return OF_FAILURE;
}

#ifdef INTOTO_CMS_SUPPORT
/***************************************************************************
 * Function Name  : HttpCMSDevProcessLoopBackData
 * Description    : This function will process the data recieved on loop
 *                  back socket
 * Input          : SFd - socket Fd
 * Output         : None
 * Return value   :
 ***************************************************************************/


int32_t HttpCMSDevProcessLoopBackData(uint32_t SFd)
{
 /*Responce from the CLI engine Daemon or CCDMD Daemon*/
  IGWIpAddr_t        PeerIp = {};
  uint16_t           PeerPort = 63050;//HTTP_CCDMD_RECV_LOOPBACK_PORT;
  int32_t            iRetVal = -1;
  HttpRequest 	     *pHttpRequest=NULL;  
  HttpClientReq_t    *pClientReq=NULL;  
  HttpGlobals_t      *pHttpg=NULL;  
  IGWCMSSendBatchInfo_t  RecvBatch;
  unsigned char            *pResouceData=NULL;
  unsigned char            *pResp=NULL;
  unsigned char            *pFinalBuff=NULL;
  int32_t             iLen =0; 

  of_memset(&RecvBatch,0,sizeof(IGWCMSSendBatchInfo_t));  
  /**
  *Http Server is  using  loop back address ((0x7F000001) 127.0.0.1),So we are  passing 
  *IPv4 addressto the cm_socket_recv_from().  
  */  
  PeerIp.ip_type_ul=IGW_INET_IPV4;
  iRetVal = cm_socket_recv_from(SFd,(char *)&RecvBatch ,sizeof(IGWCMSSendBatchInfo_t),
                                0,&PeerIp.IPv4Addr ,&PeerPort);
  
  if(iRetVal != sizeof(IGWCMSSendBatchInfo_t))
  {
    return OF_FAILURE;
  }
  
  pResp = ConfigRecvFrameResponse(&RecvBatch,0); 
 
  pResouceData=of_calloc(1,RecvBatch.iLen);  
  if(RecvBatch.UserData.CMSProtocolHdr.CMSOperationHdr.bType == 0x03)
  {
     iRetVal = CnfgRecvHandleExecuteResponse(&RecvBatch);
     /* no need to check iRetVal*/
  }
  
  
  /*TBD STATUS SIZE HANDLING*/ 
  if((RecvBatch.iLen !=0 ) && 
     ((RecvBatch.UserData.CMSProtocolHdr.CMSOperationHdr.bType != 0x04) ||
     (RecvBatch.UserData.CMSProtocolHdr.CMSOperationHdr.bType != 0x03)))
  {
    iRetVal = cm_socket_recv_from(SFd,(char *)pResouceData ,RecvBatch.iLen,
                                0,&PeerIp.IPv4Addr ,&PeerPort);
    
    if(iRetVal != RecvBatch.iLen)
    {
      return OF_FAILURE;
    }
  }   
  pHttpRequest = RecvBatch.pOpaqueData;
  pHttpg=(HttpGlobals_t *)pHttpRequest->pGlobalData;
  pClientReq  = (HttpClientReq_t *)pHttpRequest->pPvtData;  

  iLen = CMS_START_HEADERS_LEN;

  of_strcpy(pClientReq->ucOutContentType,"application/text");
  pClientReq->lOutContentLen = iLen;
  if((RecvBatch.UserData.CMSProtocolHdr.CMSOperationHdr.bType != 0x04) ||
  	 (RecvBatch.UserData.CMSProtocolHdr.CMSOperationHdr.bType != 0x03)) 
  {
    pClientReq->lOutContentLen = RecvBatch.iLen + iLen;
  }

  pFinalBuff=of_calloc(1,pClientReq->lOutContentLen);

  of_memcpy(pFinalBuff,pResp,iLen);
  pFinalBuff += iLen;
  if((RecvBatch.UserData.CMSProtocolHdr.CMSOperationHdr.bType != 0x04) ||
  	 (RecvBatch.UserData.CMSProtocolHdr.CMSOperationHdr.bType != 0x03))

  {  
    of_memcpy(pFinalBuff,pResouceData,RecvBatch.iLen);
  }
  
  pFinalBuff -= iLen;

  Httpd_Send(pHttpg, 
             (HttpRequest *)RecvBatch.pOpaqueData,
             pFinalBuff,
             pClientReq->lOutContentLen);    

  HTTPDFREE(pFinalBuff);
  HTTPDFREE(pResouceData);
  HTTPDFREE(pResp);

  return OF_SUCCESS;  
}
/**************************************************************************
 * Function Name : HttpCMSDevCreateLoopBackSocket
 * Description   : This functions will create the loop back socket and    *
 *                 and binds it on HTTP_CMSDEV_LOOPBACK_PORT                 *
 * Input         : None                                                   *
 * Output        : None                                                   *
 * Return value  : Socket FD upon successffully creation of the socket and*
 *                 binding to the HTTP_CMSDEV_LOOPBACK_PORT_ port or OF_FAILURE*
 **************************************************************************/

int32_t HttpCMSDevCreateLoopBackSocket(void)
{
  int32_t LpBkSFd = 0;
  LpBkSFd = cm_socket_create(CM_IPPROTO_UDP);
  if(LpBkSFd == OF_FAILURE)
  {
    HttpDebug(("Unable to create Loop back socket\n\r"));
    return OF_FAILURE;
  }
  cm_set_sock_reuse_opt(LpBkSFd);  
  if (cm_socket_bind (LpBkSFd, HTTP_LOOPBACK_ADDR, HTTP_CCDMD_RECV_LOOPBACK_PORT) == OF_FAILURE)
  {
    HttpDebug(("Unable to bind the socket\n\r"));
    cm_socket_close (LpBkSFd);
    return OF_FAILURE;
  }
  return LpBkSFd;
}
#endif  /* INTOTO_CMS_SUPPORT */

#ifdef OF_XML_SUPPORT
/**************************************************************************
 * Function Name : HttpClient_FindBrowserNameAndVersion                               *
 * Description   : This API used to Find browser name and version   *
 * Input         :    pUserAgent                                                 	*
 * Output        :   None                                                 *
 * Return value  : OF_SUCCESS on success or OF_FAILURE on failure		* 
 **************************************************************************/
int32_t HttpClient_FindBrowserNameAndVersion(HttpClientReq_t  *pClientReq,unsigned char *pUserAgent)
{
   unsigned char  *pTempStr=NULL;
   unsigned char  *pString=NULL;  
   unsigned char  *pTemp=NULL;   
   unsigned char  *pEnd=NULL;   
   unsigned char  *pVersion=NULL;  
   int16_t       iCount=0;
   IGWUserAgentValues_t  *pBrowserVals=NULL;
   
  /**
    *    IE		 :User-Agent: Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.0)    
    *    Mozilla	 :User-Agent: Mozilla/5.0 (Windows; U; Windows NT 5.0; en-US; rv:1.6) Gecko/20040113
    *    FireFox	 :User-Agent: Mozilla/5.0 (Windows; U; Windows NT 5.0; en-US; rv:1.7.10) Gecko/20050716 Firefox/1.0.6       
    *    NetScape:User-Agent: Mozilla/5.0 (Windows; U; Windows NT 5.0; en-CA; rv:1.0.1) Gecko/20020823 Netscape/7.0        
    *    Mozilla/5.0 (X11; U; Linux i686; en-US; rv:1.8.1.3) Gecko/20061201 Firefox/2.0.0.3 (Ubuntu-feisty)
    */
    
    if(!pUserAgent)
    {
          HttpDebug(" pUserAgent is NULL");
          return OF_FAILURE;
     }    
     pBrowserVals=of_calloc(1,sizeof( IGWUserAgentValues_t));
     if(!pBrowserVals)
     {
          HttpDebug(" pBrowserVals is NULL");
          return OF_FAILURE;
     }
     pTempStr=of_calloc(1,HTTP_XML_TMP_BUFF_LEN+1);
     if(pTempStr == NULL)
     {
        of_free(pBrowserVals);
        HttpDebug(" pTempStr is NULL");
        return OF_FAILURE;  
     }
     of_strncpy((char *) pTempStr,(char *) pUserAgent,HTTP_XML_TMP_BUFF_LEN);      
     pTempStr[HTTP_XML_TMP_BUFF_LEN]='\0';
      if(of_strstr((char *) pTempStr, HTTP_BROWSER_OPERA) != NULL)
      {
          of_strcpy((char *) pBrowserVals->ucBrowserName,HTTP_BROWSER_UNKNOWN);
     	   of_strcpy((char *) pBrowserVals->ucBrowserVersion,HTTP_BROWSER_UNKNOWN);
	    pClientReq->pUserAgent=( void *)pBrowserVals;
	    if(pTempStr)
	    {
	    	of_free(pTempStr);
	    }
		return OF_SUCCESS;
	 }
   else if((pString=(unsigned char *) of_strstr((char *) pTempStr,HTTP_BROWSER_INTERNET_EXPLORER))!=NULL)
      {     	   
        	    /* Browser is Internet Explorer with x.x version, Now we  need to  find out Version*/      	         	           	    
        	    pTemp=(unsigned char *) strtok((char *) pString,";");        	        	    
  		    if(pTemp == NULL)
  		    {
         		    /* Browser is unknown */  	      
              	    of_strcpy((char *) pBrowserVals->ucBrowserName,HTTP_BROWSER_UNKNOWN);        	          	        		    		    
                         of_strcpy((char *) pBrowserVals->ucBrowserVersion,HTTP_BROWSER_UNKNOWN);
            		    pClientReq->pUserAgent=(void *)pBrowserVals;
            		    if(pTempStr)
            		    {
            		    	of_free(pTempStr);
            		    }
            		    HttpDebug("Currently we are not supporting these type of Browser"); 
                         return OF_SUCCESS;
  		    }
		    while ((*pTemp != '\0') && (*pTemp!=' '))
		    {
 		        pTemp++;
		    }		    
		     pTemp++;
     		    of_strcpy((char *) pBrowserVals->ucBrowserName,HTTP_BROWSER_INTERNET_EXPLORER);        	          	        		    		    
     		    of_strcpy((char *) pBrowserVals->ucBrowserVersion,(char *) pTemp);
     		    pClientReq->pUserAgent=( void *)pBrowserVals;
     		    if(pTempStr)
     		    {
     		    	of_free(pTempStr);
     		    }
        	    return OF_SUCCESS;
      }
      else if((pString=(unsigned char *) of_strstr((char *) pTempStr,HTTP_BROWSER_FIREFOX) )!=NULL)
      {
          	    /* Browser is Firefox with x.x version, Now we need to find Version*/
          	    pTemp=pString;
		    while ((*pTemp != '/'))
		    {
 		        pTemp++;
		    }	
		    pTemp++;
    		    of_strcpy((char *) pBrowserVals->ucBrowserName,HTTP_BROWSER_FIREFOX);        	          	        		    		    
                    pEnd=(unsigned char *) of_strchr((char *) pTemp,' ');
                    if(pEnd)
                    {
     		      of_strncpy((char *) pBrowserVals->ucBrowserVersion,(char *) pTemp,(pEnd-pTemp));
                      pBrowserVals->ucBrowserVersion[pEnd-pTemp]='\0';
                    }
                    else
           of_strncpy((char *) pBrowserVals->ucBrowserVersion,(char *) pTemp,HTTP_BROWSER_VERSION_LEN);
    		    
     		    pClientReq->pUserAgent=(void *)pBrowserVals;
 		    if(pTempStr)
     		    {
     		    	of_free(pTempStr);
     		    }
	     	    return OF_SUCCESS;
      }          
      else if((pString=(unsigned char *) of_strstr((char *) pTempStr,HTTP_BROWSER_NETSCAPE))!=NULL)
      {     	   
        	    /* Browser is Netscape with x.x version, Now we need to find Version*/
        	    pTemp=pString;
 		    while ((*pTemp != '/'))
		    {
 		        pTemp++;
		    }	
 		    pTemp++;
 		    of_strcpy((char *) pBrowserVals->ucBrowserName,HTTP_BROWSER_NETSCAPE);        	          	        		    		    
       	    of_strcpy((char *) pBrowserVals->ucBrowserVersion,(char *) pTemp);
     		    pClientReq->pUserAgent=(void *)pBrowserVals;
     		    if(pTempStr)
     		    {
     		    	of_free(pTempStr);
     		    }
        	    return OF_SUCCESS;
      } 
      else if((pString=(unsigned char *) of_strstr((char *) pTempStr,HTTP_BROWSER_MOZILLA))!=NULL)
      	{
                 /**
                  * Mozilla	 :User-Agent: Mozilla/5.0 (Windows; U; Windows NT 5.0; en-US; rv:1.6) Gecko/20040113
                  * we need to confirm the browser name and version.
                  */    	    
                  iCount=0;
                  pTemp=(unsigned char *) of_strstr((char *) pString,"rv:");                   
	           if(pTemp != NULL)
	           {
                  while(*pTemp!=':')
                  {
                    pTemp++;
                  }
                  pTemp++;
                  pString=pTemp;
                  pString=(unsigned char *) of_strstr((char *) pString,"Gecko");
			 if(pString == NULL)
			 {
			    iCount=0;  /*Unknown Browser*/
			 }			 
    else
    {
                  while(*pString!='\0')
                  {
                      if(*pString=='/') 
                      {
                      	iCount++;
                      }
                      pString++;
                  }
    }
                  pVersion=(unsigned  char *) strtok((char *) pTemp,")");
			 if(pVersion == NULL)
			 {
                            iCount=0;  /*Unknown Browser*/
			 }
	            }
                  if(iCount==1)
                  {
         	           of_strcpy((char *) pBrowserVals->ucBrowserName,HTTP_BROWSER_MOZILLA);        	          	        		    		    
        		    of_strcpy((char *) pBrowserVals->ucBrowserVersion,(char *) pVersion);  
                  }
                   else  /*Unknown Browser*/
                  {
                         of_strcpy((char *) pBrowserVals->ucBrowserName,HTTP_BROWSER_UNKNOWN);        	          	        		    		    
                         of_strcpy((char *) pBrowserVals->ucBrowserVersion,HTTP_BROWSER_UNKNOWN);
                  }
     		    pClientReq->pUserAgent=(void *)pBrowserVals;
     		    if(pTempStr)
     		    {
     		    	of_free(pTempStr);
     		    }
       	    return OF_SUCCESS;
      	}
      else
      	{
       	     /* Browser is unknown */  	      
       	    of_strcpy((char *) pBrowserVals->ucBrowserName,HTTP_BROWSER_UNKNOWN);        	          	        		    		    
                  of_strcpy((char *) pBrowserVals->ucBrowserVersion,HTTP_BROWSER_UNKNOWN);
     		    pClientReq->pUserAgent=(void *)pBrowserVals;
     		    if(pTempStr)
     		    {
     		    	of_free(pTempStr);
     		    }
     		    HttpDebug("Currently we are not supporting these type of Browser"); 
                  return OF_SUCCESS;
       }      	
      
   /*control  will never hit here*/
   return OF_SUCCESS;
}/*HttpClient_FindBrowserNameAndVersion*/
#endif /*OF_XML_SUPPORT*/

#ifdef INTOTO_IPV6_SUPPORT
/**************************************************************************
 * Function Name : HttpdNumToPresentation                                             *
 * Description   : This frunction is used to convert the network order IP *
 *                 address into the string format.                        *
 * Input         : IPv6Addr (I) - IP address in network order.           *
 *                 cOutStr(O) - pointer to character buffer where the  *
 *                                 IP address will be stored.             *
 * Output        : Network ordered IP address in string format.           *
 * Return value  : String in string format.                               *
 **************************************************************************/
int32_t	HttpdNumToPresentation(IGWIpAddr_t  *IPv6Addr ,unsigned char *cOutStr)
{
  unsigned char *pTempStr=NULL;
  unsigned char cIPv6SrcIPAddress[HTTP_LOCAL_IP_ADDRESS_LEN]={};

  if(cOutStr==NULL)
  {
          HttpDebug(("cOutStr is NULL\r\n"));
          return OF_FAILURE;
  }  
  if(IGWIPv6NumToPresentation(&(IPv6Addr->IP.IPv6Addr),(unsigned char *)cIPv6SrcIPAddress,sizeof(cIPv6SrcIPAddress))!=OF_SUCCESS)
  {	 
          HttpDebug(("IGWIPv6NumToPresentation Failed\r\n"));
          return OF_FAILURE; 	
   }	
   if(of_strstr((unsigned char *)cIPv6SrcIPAddress,HTTP_IPV6_ADDR_PREFIX)!=NULL)	
   {
	   pTempStr=strtok((unsigned char *)cIPv6SrcIPAddress,HTTP_IPV6_ADDR_PREFIX);
	   if(pTempStr!=NULL)	
   	   {					
   	         of_strncpy(cIPv6SrcIPAddress,pTempStr,of_strlen(pTempStr)+1);
   	   }
   }  
   of_strcpy(cOutStr,cIPv6SrcIPAddress);
   return OF_SUCCESS;
}
#endif /*INTOTO_IPV6_SUPPORT*/

/**************************************************************************
 * Function Name : HttpClient_CleanUp                                     *
 * Description   : This will clenup ClientReqs from Global structures     *					  
 * Input         : pHttpGlobals  - Global pointer                         *
 * Output        : Cleans the Global Data Structures 			          *
 * Return value  : None                               			          *
 **************************************************************************/

void HttpClient_CleanUp(HttpGlobals_t *pHttpGlobals)
{ 
   int32_t  uiIndex=0;
   int32_t  iResIndex=0;
   int32_t    ReqTableSize=0;
   
   if(!pHttpGlobals)
   {
     Httpd_Error("pHttpGlobals is NULL\r\n");	
     return ;
   }
   ReqTableSize=Httpd_GetMaxReqTableSize(pHttpGlobals);/*pHttpGlobals->pHttpGlobalConfig->iReqtablesize;*/  		 	
   for(uiIndex = 0;uiIndex < ReqTableSize; uiIndex++)  
   {
            for(iResIndex= 0;iResIndex < HTTP_MAX_PIPELINED_REQUESTS; iResIndex++)
            {
                  HttpClientReq_t		 *pClientrequest=NULL; 
                  pClientrequest = pHttpGlobals->pHttpClientReqTable[uiIndex].pHttpPipeline[iResIndex];
                  if(pClientrequest == NULL ||(pClientrequest->SockFd <=0))
                  { 
                      continue;
                   }
                  else
                  {					
                           pClientrequest->bPersistentConn=TRUE;
                           HttpFreeClientReqAndTxBuf(pHttpGlobals,pClientrequest);																					
                  }                           				   
             }  
             pHttpGlobals->pHttpClientReqTable[uiIndex].iSockFd=-1;
             of_memset(pHttpGlobals->pHttpClientReqTable[uiIndex].pHttpPipeline,0,4*HTTP_MAX_PIPELINED_REQUESTS);
			 
	      if(pHttpGlobals->pHttpClientReqTable[uiIndex].pKeepAliveInfo &&
	        pHttpGlobals->pHttpClientReqTable[uiIndex].pKeepAliveInfo->pKeepAliveTimer)	 	
	      {
	         Httpd_Timer_Delete(pHttpGlobals->pHttpClientReqTable[uiIndex].pKeepAliveInfo->pKeepAliveTimer);
 	         of_free(pHttpGlobals->pHttpClientReqTable[uiIndex].pKeepAliveInfo);
		  pHttpGlobals->pHttpClientReqTable[uiIndex].pKeepAliveInfo = NULL; 
	      }	 
     }
     for(uiIndex = 0;uiIndex < ReqTableSize; uiIndex++)  
     {
            HttpClientReq_t   *pClientrequest=NULL; 
            if(!(pHttpGlobals->pHttpReqTable[uiIndex]))
            {
                     continue;
            }            
            pClientrequest = pHttpGlobals->pHttpReqTable[uiIndex];
            if(pClientrequest != NULL)
            {											 
                      pClientrequest->bPersistentConn=FALSE;
                      HttpFreeClientReqAndTxBuf(pHttpGlobals,pClientrequest);
                      pHttpGlobals->pHttpReqTable[uiIndex] = NULL;                      
            }    
   }
   return;
}/*HttpdCleanUp*/

/**************************************************************************
 * Function Name : HttpdGlobalConfigInit                                  *
 * Description   : This function is used to initialize the Global         * 
 *                 config related                                         *
 *                 data structures.                                       *
 * Input         : pHttpGlobals  - Global pointer.                        *
 * Output        : Creates a Global configuration                         *
 * Return value  :OF_SUCCESS on success OF_FAILURE on failure               *
 **************************************************************************/
 int32_t HttpdGlobalConfigInit(HttpGlobals_t *pHttpGlobals)
 {
     if(!pHttpGlobals)
    {
      Httpd_Error("pHttpGlobals is NULL\r\n");	
      return OF_FAILURE;
    } 
    pHttpGlobals->pHttpGlobalConfig=(HttpdGlobalConfig_t  *)of_calloc(1,sizeof(HttpdGlobalConfig_t));
    if(pHttpGlobals->pHttpGlobalConfig == NULL)
    {
       HttpDebug(("of_calloc is failed for pHttpGlobals->pHttpGlobalConfig \r\n"));
	return OF_FAILURE;
    }
    HttpdUpdateGlobalConfig(pHttpGlobals);
    return OF_SUCCESS;
 }
/**************************************************************************
 * Function Name : HttpdSocketValidateErrorno                             *
 * Description   : This is the API for knowing the socket error numbers   *
 * Input         : None    					                              *
 * Output        : None 										          *
 * Return value  : TRUE or FALSE                               	          *
 **************************************************************************/
bool HttpdSocketValidateErrorno(void)
{
  switch(errno)
  {
      case EWOULDBLOCK:
	  	Httpd_Error("SocketError::errorno EWOULDBLOCK \r\n");
		break;
      case EINTR:
	  	Httpd_Error("SocketError::errorno EINTR\r\n");
		break;
      case EINPROGRESS:  /* we can remove if we are not using CONNECT */
	  	Httpd_Error("SocketError::errorno EINPROGRESS\r\n");
		break;
      case ENOBUFS: 
	  	Httpd_Error("SocketError::errorno ENOBUFS\r\n");
	  	return TRUE;
     default:
               HttpDebug(("unknown error no received = %d\n\r",errno));   
               return FALSE;
  }  
  return FALSE;
}/*HttpdSocketValidateErrorno*/

#ifdef HTTPD_DEBUG
void pr(HttpGlobals_t *pHttpGlobals);
void pr(HttpGlobals_t *pHttpGlobals)
{
  int i;
  int32_t    ReqTableSize;

  ReqTableSize=Httpd_GetMaxReqTableSize(pHttpGlobals);/*pHttpGlobals->pHttpGlobalConfig->iReqtablesize;*/ 

  for(i = 0;i<pHttpGlobals->iMaxServers;i++)
  {
    if(pHttpGlobals->HttpServers[i].uiPort > 0)
    printf("port = %d, sfd = %d, enable = %d, type = %d\n\r",
           pHttpGlobals->HttpServers[i].uiPort, pHttpGlobals->HttpServers[i].SockFd,
           pHttpGlobals->HttpServers[i].bEnabled,pHttpGlobals->HttpServers[i].Type);
  }
  for(i = 0;i<5;i++)
  {
    printf("polltable: fd = %d interested=%ld returned=%ld\n\r",
    	     pHttpGlobals->HttpPollTable[i].fd_i,
    	     pHttpGlobals->HttpPollTable[i].interested_events,
    	     pHttpGlobals->HttpPollTable[i].returned_events);
  }
  for(i=0;i<ReqTableSize;i++)   /* REQTABLE_SIZE */
  {
    if(pHttpGlobals->pHttpReqTable[i] != NULL)
    {
      HttpTransmitBuffer_t *pTx;
      int32_t   iCount = 0;
      pTx = (pHttpGlobals->pHttpReqTable[i])->pTransmitBuffer;
      while(pTx != NULL)
      {
        pTx=pTx->pNext;
        iCount++;
      }

      printf("txlen=%ld, offset=%ld nodes=%d\n\r",
      	pHttpGlobals->pHttpReqTable[i]->uiTxBufLen,
      	(pHttpGlobals->pHttpReqTable[i])->uiTxBufOffset,iCount);
    }
  }
  return;
}
#endif /*HTTPD_DEBUG*/
#endif /* OF_HTTPD_SUPPORT */

