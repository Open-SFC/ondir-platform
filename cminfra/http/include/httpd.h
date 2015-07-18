
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
/*  File        : httpd.h                                                 */
/*                                                                        */
/*  Description : This file contains the local definitions and function   */
/*                prototypes that are used in the http server module.     */
/*                                                                        */
/*  Version      Date      Author      Change Description                 */
/*  -------    --------    ------    ----------------------               */
/*    1.0      00.00.00    kmdhar    Initial implementation               */
/*    1.1      05.05.04    DRAM      Modified during the code review.     */
/**************************************************************************/

#ifndef __HTTPD_H_
#define __HTTPD_H_


#ifdef INTOTO_CMS_SUPPORT
#include "httpwrp.h"
#include "htpcnfrc.h"
#endif /* INTOTO_CMS_SUPPORT */

/****************************************************
 * * * *  M a c r o    D e f i n i t i o n s  * * * *
 ****************************************************/

#ifdef IGW_MERCED_SUPPORT
    #define  HTTP_MERCED_LOGO_FILE        "ilogo.gif"
    #define  HTTP_MERCED_IMG1_FILE        "ibgtop.gif"
    #define  HTTP_MERCED_IMG2_FILE        "ispacer.gif"
    #define  HTTP_MERCED_STYLE_FILE       "style.css"
#endif /* IGW_MERCED_SUPPORT */

#define HTTP_LCL_INADDR_ANY        0x0L

#ifdef  LICENSING
#define  HTTP_DEF_REG_FILE            "register.htm"
#define  HTTP_DEF_REG_GIFFILE         "reglogo.gif"
#endif


#ifdef INTOTO_UAM_SUPPORT
#define UAM_DEF_COMPANY_LOGO_FILE    "company.gif"  
#endif /*INTOTO_UAM_SUPPORT*/
/**
 * Implementation Specific Declarations.
 */
#define  HTTPD_DEF_TIME_LINGER  (  600 )
#define  HTTPD_DEF_CONT_TYPE    "text/html"
#define  HTTPD_REQSTR_TIMEOUT   (120*10) /* (10) To allow JAR file download for java Web Start */
#define  HTTPD_RESPONCE_TIME    (10)/*in Milli Seconds only */

#define  HTTPS_SSL_READ_PENDING	     2    /* Used to send SSL_READ_PENDING event to HTTP Server */
#define  HTTP_IP_LEN            (   16 )
#define  HTTP_SIGN_LEN          (    1 )
#define  HTTP_RNDNUM_LEN        (   50 )

#define  HTTP_PATH_LEN          (   16 )

#define  HTTPD_ZIPMETH_DEFLATED ( 8 )

#define  CRLF                   "\r\n"
#define  CRLF_CRLF              "\r\n\r\n"

#define HTTP_NUMBER_OF_MONTHS         (12)
#define HTTP_MONTH_STRING_LEN         (4)

#define HTTP_NUMBER_OF_DAYS           (7)
#define HTTP_DAY_STRING_LEN           (4)
#define HTTP_ERRORSCREEN_FILE  "errscrn.htm"
#define HTTP_FIND_MIN(a,b) (((a) < (b)) ? (a) : (b))

/*****************************STATEMACHINE EVENTS******************************/

#define HTTP_STATE_EVENT_BASE 			9000

#define HTTP_GET_METHOD_EVENT 			9001
#define HTTP_HEAD_METHOD_EVENT			9002	
#define HTTP_POST_METHOD_EVENT			9003
#define HTTP_BAD_METHOD_EVENT			9004
#define HTTP_TIMEOUT_EVENT				9005
#define HTTP_CLOSE_EVENT					9006

#define HTTP_MAX_STATES					  6	
#define HTTP_MAX_EVENTS					  6	
#define HTTP_PIPELINED_REQUEST                       100 
#define HTTP_FILE_EXTENTION_MAX_LEN            (16+1)
#define HTTP_RESPONCE_TIME_MAX_VAL              (1)
#define HTTPD_MIN_POLL_TIMEOUT			   100   /*Timeout in  millseconds*/

#define HTTP_SEND_TIMEOUT                  		   (60)  /* In seconds, 1 minutes */
#define HTTP_SEND_RETRY_COUNT       			   (10)
#define HTTP_LOCAL_BUFFER_LEN         		   (512)
#define HTTP_LOCAL_IP_ADDRESS_LEN     		   (CM_INET6_ADDRSTRLEN)
#define HTTP_MAX_PIPELINE_TABLE_LEN     	   (32)
#define HTTP_MAX_EXT_LEN					   (16)
#define HTTP_RANDOM_STRING_LEN			   (50)	
#define HTTP_MAX_KEEPALIVE_TIMEOUT                (60)      /* in seconds*/
#define HTTP_HTML_WELCOME_PAGE_NAME           "welcome.htm" 
#define HTTP_APPLET_FILE_EXT                              "jar"

#define HTTPDFREE(pReq)   \
                             if(pReq)  \
                             {   \
                                   of_free(pReq);  \
                             }
#ifdef INTOTO_CMS_SUPPORT
#define HTTPD_MORESIZE_FILE (-2)

#define ADMIN_PRIV                  0x01
#define USER_PRIV                   0x02
#endif  /* INTOTO_CMS_SUPPORT */
/****************************************************
 * * * *  T y p e    D e c l a r a t i o n s  * * * *
 ****************************************************/
/**
 * The type HtError is an Enumeration of all the standard HTTP status codes
 * as well as other codes defined by us which will be helpful in handling
 * our internal stuff and errors
 */
typedef enum _HtError_e
{
  HTERR_OK = 0,                       /* 200 */
  HTERR_CREATED,                      /* 201 */
  HTERR_ACCEPTED,                     /* 202 */
  HTERR_PARTIAL,                      /* 203 */
  HTERR_NO_RESPONSE,                  /* 204 */
  HTERR_MOVED,                        /* 301 */
  HTERR_REDIRECTED,                   /* 302 */
  HTERR_METHOD,                       /* 303 */
  HTERR_NOT_MODIFIED,                 /* 304 */
  HTERR_BAD_REQUEST,                  /* 400 */
  HTERR_UNAUTHORIZED,                 /* 401 */
  HTERR_PAYMENT_REQUIRED,             /* 402 */
  HTERR_FORBIDDEN,                    /* 403 */
  HTERR_NOT_FOUND,                    /* 404 */
  HTERR_INTERNAL,                     /* 500 */
  HTERR_NOT_IMPLEMENTED,              /* 501 */
  HTERR_BAD_GATEWAY,                  /* 502 */
  HTERR_SERVICE_UNAVAILABE,           /* 503 */
  HTERR_HTTP_CODES_END,      /* Put all non-HTTP status codes after this */
  HTERR_TIME_OUT,
  HTERR_INTERRUPTED,
  HTERR_CON_INTR,
  HTERR_BAD_REPLY,                    /* HTTP */
  HTERR_MAX_REDIRECT,
  HTERR_SYSTEM,
  HTERR_NO_MEM,
  HTERR_ELEMENTS                      /* This MUST be the last element */
} HtError_e;

/********************************************************
 * * * *  F u n c t i o n    P r o t o t y p e s  * * * *
 ********************************************************/

/**
 * Functions implemented in httpd.c file.
 */

/**
 * This function is used to
 */
int32_t  Httpd_InitInflateMemeory( void );

/**
 * This function is used to
 */
int32_t Httpd_GetZipMethod( unsigned char  *pinputBuffer,
                            uint32_t  *pinputBufferIndex,
                            uint32_t  *pinputBufferSize,
                            int32_t   inputFd );

/**
 * This function is used to
 */
int32_t Httpd_Unzip( unsigned char  *pinputBuffer,
                     uint32_t  *pinputBufferIndex,
                     uint32_t  *pinputBufferSize,
                     int32_t   inputFd,
                     int32_t   uioutFd );

/**
 * This function is used to
 */
int32_t HttpRequest_SendFile(HttpGlobals_t *pHttpGlobals,HttpRequest  *pReq );

/**
 * API That tells whether the current posting is of multi-part or not.
 */
bool HttpRequest_IsMultiPart( HttpRequest  *pReq );

/**
 * This function is used to get the multi-part data.
 */
unsigned char *HttpRequest_GetMPartData( HttpRequest  *pReq );

/**
 * This is the API to be used to get boundary string of ulti-part posting.
 */
unsigned char *HttpRequest_GetMPartBoundary( HttpRequest  *pReq );

/**
 * This function is used to get the multi-part data length.
 */
int32_t HttpRequest_GetMPartDataLen( HttpRequest  *pReq );
/**
 * This function is used to
 */
int32_t Https_Init( HttpServer_t  *pSer );

/**
 * This function is used to
 */
int32_t Httpd_OnlyCerts_GetPermission( char  *pFileName,
                                       int32_t  *pPermission );

/**
 * This function is used to
 */
char* Httpd_OnlyCerts_GetIndexFile( void );

/**
 * This function is used to
 */
void Httpd_MapHtmlFile( HttpClientReq_t  *pReq );

/**
 * This function is used to
 */
int32_t Httpd_Send_Redirect( HttpClientReq_t  *pReq );

/**
 * This function is used to initialize the ceritificate related prerequisites.
 */
void Https_CertsInit( void );

/**
 * Functions implemented in httpkuki.c file.
 */

/**
 * This function is used to initialize the session cookies data structures.
 */
int32_t HttpCookie_Init( HttpGlobals_t  *pHttpGlobals,uint32_t  ulMax );

/**
 * This function is used to add a cookie with the given information.
 */
int32_t HttpCookie_Add(HttpGlobals_t *pHttpGlobals,
			   unsigned char  *value_p,
                        unsigned char  *pUserName,
                        unsigned char  *PeerIp,
                        unsigned char  *pChallenge,
                        unsigned char  *pPath,
                        uint32_t  ulVersion,
                        uint32_t  ulMaxAge );

/**
 * This function is used to get the bytes written.
 */
int32_t HttpCookie_GetBytesWritten( unsigned char  *pCookieVal,
                                    uint32_t  *puiBytesWritten );

/**
 * This function is used to get the hash entry.
 */
int32_t HttpCookie_GetHashEntry( unsigned char  *pCookieVal,
                                 void    **ppBinFileEntry );

/**
 * This function is used to set the bytes written.
 */
int32_t HttpCookie_SetBytesWritten( unsigned char  *pCookieVal,
                                    uint32_t  uiBytesWritten );

/**
 * This function is used to set the hash entry.
 */
int32_t HttpCookie_SetHashEntry( unsigned char  *pCookieVal,
                                 void    **ppBinFileEntry );

/**
 * This function is used to delete all the cookies for the given port
 * number.
 */
void HttpCookie_DelAllByPort( uint16_t  usPort );

int32_t HttpClient_Send( HttpClientReq_t  *pClientReq,
                         char          *pHtmlBuf,
                         uint32_t         ulLength );

#ifdef INTOTO_CMS_SUPPORT
extern
int32_t IGWSockCreate (unsigned char ucProtocol , uint32_t ulIptype );
extern
int32_t IGWSockBind (int32_t iSockFd, IGWIpAddr_t IpAddr, uint16_t usLocalPort);
extern
int32_t IGWGetSocketName (int32_t iStreamSock, IGWIpAddr_t *IpAddr, uint16_t *pusPort);
extern
int32_t IGWSockAccept (int32_t iSockFd, IGWIpAddr_t *PeerIp, uint16_t *pusPeerPort);
extern
void  T_gmtime(const time_t *pTime , struct tm *pTm);
int32_t  IGWConfRecvInit(void);
unsigned char *ConfigRecvFrameResponse(IGWCMSSendBatchInfo_t  *RecvBatch,
                                  uint16_t sCommand); 
int32_t Httpd_Init( void );
int32_t ConfigReceiverParseRequest(HttpRequest *pHttpRequest,
                                              char  *pPostStr);
int32_t Httpd_SendXMLBuff( HttpRequest  *pHttpRequest,
				                    char      *pHtmlBuf,
				                    uint32_t     ulLength );
int32_t HttpsLdSvInit( void );
#endif  /* INTOTO_CMS_SUPPORT */
#endif /* __HTTPD_H_ */

