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
/*  File        : htpdglb.h                                               */
/*                                                                        */
/*  Description : This file contains global definitions which are used by */
/*                all the modules of the igateway.                        */
/*                                                                        */
/*  Version      Date      Author      Change Description                 */
/*  -------    --------    ------    ----------------------               */
/*    1.1      22.02.06    Harishp       Initial implementation.          */
/**************************************************************************/

#ifndef __HTPDGLB_H
#define __HTPDGLB_H

#include "cmincld.h"
#include "cmdefs.h"
#include "httpgdef.h"
#include "htmlpars.h"

/****************************************************
 * * * *  M a c r o    D e f i n i t i o n s  * * * *
 ****************************************************/
#define	HTTPD_FILENAME_LEN        			(32+1)
#define   HTTPD_COOKIENAME_LEN   			(48+1)  
#define   HTTPD_DOCPATH_LEN         			(64+1) 
#define   HTTP_MAX_PIPELINED_REQUESTS  	  (10)

#define pHTTPG  pHttpGlobals

int32_t  FLASH_HTTPS_CERTS1_ADDR;
int32_t  FLASH_HTTPS_CERTS2_ADDR;
#define FLASH_HTTPS_CERTS2_LEN                        0x8000
#define FLASH_HTTPS_CERTS1_LEN                        0x8000

#define HTTPS_ID CM_HTTP_MGMT_ENGINE_ID
/*Need to move TRACE_SEVERE,  TRACE_INFO and TRACE_LOG to trace module later*/
#define TRACE_SEVERE 4
#define TRACE_LOG 5
#define TRACE_INFO 2

/****************************************************
 * * * *  T y p e    D e c l a r a t i o n s  * * * *
 ****************************************************/
  /*HTTP/1.1 Persistent and Pipelining*/
 typedef struct HttpResponceTable_s
 {
     cm_sock_handle 	 iSockFd;
     bool   		 bStartResponce;
     HttpKeepAliveFd_t    *pKeepAliveInfo;	 
     HttpClientReq_t  	*pHttpPipeline[ HTTP_MAX_PIPELINED_REQUESTS ];
 }HttpResponceTable_t;
  
typedef struct   HttpdGlobalConfig_s
{
 int32_t  ulMaxCookies;
 int32_t   iMaxfds;
 int32_t   iReqtablesize; 
 int32_t   iMaxTimerLists;
 int32_t   iHttpdMinSecs;
 int32_t   iHttpdMaxSecs;
 int32_t   iHttpMaxServers;
 int32_t   iHttpsDefaultPort;
 uint32_t  ulCookieMaxSize;
 uint32_t ulHttpdMaxCookieBufferLen;
 uint32_t  ulHttpdMaxCookieValueLen;
 uint32_t  ulRegMaxModules; 
 unsigned char  ucLoginFile[HTTPD_FILENAME_LEN];
 unsigned char  ucLogoutFile[HTTPD_FILENAME_LEN];
 unsigned char  ucErrorFile[HTTPD_FILENAME_LEN];
 unsigned char  ucUlogoutFile[HTTPD_FILENAME_LEN];
 unsigned char  ucLogoFile[HTTPD_FILENAME_LEN];
 unsigned char  ucXSLLogOutFile[HTTPD_FILENAME_LEN]; 
 unsigned char  ucXMLLogInFile[HTTPD_FILENAME_LEN]; 
 unsigned char  ucLogInOutFile[HTTPD_FILENAME_LEN]; 
 unsigned char  ucCookiename[HTTPD_COOKIENAME_LEN];
 unsigned char  ucIndexFile[HTTPD_FILENAME_LEN];
 unsigned char  ucDocPath[HTTPD_DOCPATH_LEN];
 unsigned char  *ucHCertPath;
 unsigned char  *ucHCertBackupPath;   
 unsigned char  **ucUserHtmlPages;
 unsigned char  **ucLoginHtmlPages;
 unsigned char  **ucUrlFilesList;
 unsigned char  **ucSSLTemplateList; 
 HttpPortInfo_t *DefaultPortConfig;   
 uint32_t       ulUrlLength;
 uint32_t      ulAppDataLength;
}HttpdGlobalConfig_t;

typedef struct HttpGlobals_s
{
 int32_t                         gHttpMaxFds;
 int32_t                          gHttpCookieMax;
 int32_t                          gHttpTimerLists;
 int32_t                          gHttpMaxReqTableSize;
 HttpClientReq_t               **pHttpReqTable;
 HttpRegModuleRecList_t    *pHttpRegModuleHead_g ;
 unsigned char                      ucRegModuleId_g;
 uint32_t                       HttpRegModuleFreeQMaxSize_g;
 char                        IGWHttpdCoreVer_g[IGW_HTTPD_VERSION_LEN+1];
 HttpdConfig_t                  HttpdConfig;
 HtmlFileBuf_t                   HttpFileBuf;
 cm_sock_handle           MutexSockFd ;
 HttpServer_t                *HttpServers;
 int32_t                         iMaxServers;
 struct cm_poll_info                 *HttpPollTable;
 int32_t                         lHttpFileId;
 HttpdSetCookieInfo_t      *pFreeSetCookieInfo ;
 HttpdGetCookieInfo_t      *pFreeGetCookieInfo ;
 HttpAsyncExternFdReg_t   HttpExternRegFds[HTTP_MAX_REG_FDS];
 HttpdExternFd_t               HttpdExternFdToPoll[HTTP_EXTERNFD_POLLTABLE_LEN]; 
 uint32_t                        uiHttpModuleId_g ;
 HttpRegModuleRecFreeQ_t HttpRegModuleFreeQ;  
 hMimeType_t                    *HttpMimeBuckets[HTTP_MIMEBUCKET_SIZE]; 
 Httpd_Timer_t  	           **pHttpdTmrLst;  
 time_t         			     HttpdLastUpdate;
 HttpCookie_t                    **pHttpCookieList;
 HttpCookieStats_t              HttpCookieStats;
 Httpd_TagType_t 		   *pHttpdTagList;
HttpReqOpaqueNode_t 	     HttpReqOpaqueStatus[IGW_HTTPD_MAX_OPAQUE_VARIABLES];
HttpdGlobalConfig_t            *pHttpGlobalConfig;
HttpResponceTable_t          *pHttpClientReqTable;
 HttpAuthOrder_t                *pAuthOrder;
 t_sem_id_t                        CookieSemId;
uint32_t                           uiOpaqueIndex;
HttpdCbkTimers_t                *pCbkTimers;
#ifdef INTOTO_CMS_SUPPORT
HttpdAdditionalFd_t   HttpdAdditionalFdToPoll[HTTP_ADDITIONALFD_POLLTABLE_LEN]; /*for cms*/
#endif /* INTOTO_CMS_SUPPORT */
} HttpGlobals_t;
HttpGlobals_t  *pHttpGlobals;
  
typedef int32_t (*HTTP_STATE_FUNC_PTR)  (HttpGlobals_t  *pHttpGlobal,HttpClientReq_t *pClientReq);

typedef struct HttpHandleEvent_s
{
  uint32_t ulEvent;
  HTTP_STATE_FUNC_PTR  FuncPtr;  
}HttpHandleEvent_t;

/**
 * This function is used to send an HTML buffer of given size to the
 * client.
 */
int32_t Httpd_Send(HttpGlobals_t *pHttpGlobals,HttpRequest  *pHttpRequest,
                    char      *pHtmlBuf,
                    uint32_t     ulLength );

/*This is written for SSL-VPN */

int32_t Httpd_IsPartOfLoginPage(HttpGlobals_t *pHttpGlobals,char  *pFileName);

int32_t HttpdCheckUrlFileAccess(HttpGlobals_t *pHttpGlobals, char  *pFileName);

int32_t HttpdTemplateFileAccess(HttpGlobals_t *pHttpGlobals,char  *pFileName);

void HttpdUpdateGlobalConfig( HttpGlobals_t  *pHttpGlobals );

/**
 * This function is used to accept the requests coming from the clients on
 * all servers and will run infinite loop. This pools on all listening
 * (server) fds along with current client fds. If there is any read-event
 * on listening fds, it calls Httpd_AcceptClient and if it is client fd it
 * calls HttpClient_Process.
 */
int32_t Httpd_Start(HttpGlobals_t  *pHttpGlobals);

int32_t HttpdGlobalConfigInit(HttpGlobals_t  *pHttpGlobals);

int32_t  HttpAsyncUnregisterFd(HttpGlobals_t  *pHttpGlobals,int32_t iSockFd);

int32_t  HttpAsyncRegisterFdCbk(HttpGlobals_t  *pHttpGlobals,HttpAsyncExternFdReg_t FdCbkFnPtr);

int32_t Http_LoginRegModuleAPI(HttpGlobals_t  *pHttpGlobals,HttpAPIRegModuleInfo_t *pModule);

int32_t Http_LoginDeRegModuleAPI(HttpGlobals_t  *pHttpGlobals,char  *pModuleName);

int32_t Http_UpdateRegModuleRequest(HttpGlobals_t  *pHttpGlobals,HttpRegModuleRequestInfo_t *pModRequest);

int32_t HttpRegModuleInit(HttpGlobals_t  *pHttpGlobals);

int32_t HttpRegModuleFreeQInit(HttpGlobals_t  *pHttpGlobals);

HttpRegModuleRecList_t *HttpRegModuleAlloc(HttpGlobals_t  *pHttpGlobals);

int32_t HttpCreateSockAndBind(HttpGlobals_t  *pHttpGlobals,HttpPortInfo_t PortInfo);

int32_t HttpReqOpaqueInit(HttpGlobals_t  *pHttpGlobals);

int32_t IGWHttpRegisterforOpaqueIndex(HttpGlobals_t  *pHttpGlobals,uint32_t      *pIndex,
                                       FREE_FUN_PTR  pFreeFunPtr );

int32_t IGWHttpDeRegisteOpaqueIndex(HttpGlobals_t  *pHttpGlobals,uint32_t  uiIndex );

/**
 * This function is used to get the core version of the http server. This
 * function expects a buffer pointer where in it fills the value present in
 * the global variable, IGWHttpdCoreVer_g.
 */
int32_t IGWHttpdGetCoreVersion(HttpGlobals_t  *pHttpGlobals,char  *pBuffer );

/**
 * This function is used to close all opened sockets by the http server.
 */
int32_t Httpd_Destroy(HttpGlobals_t  *pHttpGlobals);
/**
 * This function is used to set the inactivity timeout value for the given
 * config admin user.
 */
int32_t SetUserInactivityTimeOut(HttpGlobals_t  *pHttpGlobals,unsigned char  *pCookieVal,
                                  uint32_t  uiAdminTimeOut );

/**
 * This function is used to reset the inactivity timeout value for the
 * given cookie value.
 */
void HttpCookie_ResetInacTimeout(HttpGlobals_t  *pHttpGlobals,unsigned char  *pCookieVal );

int32_t HttpRegModuleFree(HttpGlobals_t  *pHttpGlobals,HttpRegModuleRecList_t *pRegModule_p);

void Httpd_TimerUpdate(HttpGlobals_t  *pHttpGlobals);

int32_t HttpdTaskVarAdd( HttpGlobals_t  *pHttpGlobals);

/**
 * This function is used to initialize the MIME related data structures.
 */
void HttpMime_Init(HttpGlobals_t  *pHttpGlobals);

void HttpRegMod_http_Init(HttpGlobals_t  *pHttpGlobals);

/**
 * This function is used to initialize the flash file system used by the
 * http server for html files.
 */
int32_t HtmlFs_Init(HttpGlobals_t  *pHttpGlobals);

int32_t Httpd_InitServers(HttpGlobals_t  *pHttpGlobals);

int32_t HttpAuthOrderInit(HttpGlobals_t  *pHttpGlobals);

int32_t HttpClient_SendResponse(HttpGlobals_t  *pHttpGlobals,HttpClientReq_t  *pClientReq);

int32_t HttpClient_Delete(HttpGlobals_t  *pHttpGlobals,HttpClientReq_t  *pClientReq);

int32_t HttpClient_Free(HttpGlobals_t  *pHttpGlobals,HttpClientReq_t  *pClientReq);

/**
 * This function is the state handler for 'Read Post String'. This checks
 * for the completion of post string. If complete post string is read it
 * changes the state to 'Send Response'.
 */
int32_t HttpClient_ReadPStr_ReadEvent(HttpGlobals_t  *pHttpGlobals,HttpClientReq_t  *pClientReq);

/**
 * This function is used to get the MIME type or the content type for the
 * given extention.
 */
int32_t HttpMime_Ext2ContentType(HttpGlobals_t  *pHttpGlobals,char  *aExtention,
                                  char  *aContentType );

/**
 * This function is used to check whether the SSL based connections can be
 * established or not. That is https channel is enabled or not.
 */
bool Httpd_CheckUserLogin(HttpGlobals_t  *pHttpGlobals,HttpRequest  *pHttpRequest,
                              char      *pUName );

 int32_t HttpFreeClientReqAndTxBuf(HttpGlobals_t  *pHttpGlobals,HttpClientReq_t *pClientReq);    
 
int32_t HttpClient_UploadBinFile(HttpGlobals_t  *pHttpGlobals,HttpClientReq_t  *pClientReq );

HttpClientReq_t *HttpClient_Alloc(HttpGlobals_t  *pHttpGlobals);
/**
 * This function is used to send the starting page on the successful login.
 * For the https enabled build, if the certificates are not loaded into the
 * server then the certificates page will be sent, otherwise welcome page
 * will be sent.
 */
int32_t HttpRequest_SendWelcomePage(HttpGlobals_t *pHttpGlobals,HttpRequest  *pHttpRequest );


unsigned char *Httpd_GetLogInOutFile( HttpGlobals_t *pHttpGlobals ) ;

int32_t Httpd_GetMinSecs( HttpGlobals_t *pHttpGlobals );

int32_t Httpd_GetMaxSecs( HttpGlobals_t *pHttpGlobals );

int32_t Httpd_GetMaxCookieBufferLen( HttpGlobals_t *pHttpGlobals );

int32_t Httpd_GetMaxCookieValueLen( HttpGlobals_t *pHttpGlobals );

uint32_t  Httpd_GetUrlSize(HttpGlobals_t *pHttpGlobals);

uint32_t  Httpd_GetAppDataLength(HttpGlobals_t *pHttpGlobals);

int32_t Httpd_GetMaxFds( HttpGlobals_t *pHttpGlobals );

int32_t Httpd_GetMaxReqTableSize( HttpGlobals_t *pHttpGlobals );

uint32_t Httpd_GetMaxCookieSize( HttpGlobals_t *pHttpGlobals );

int32_t Httpd_GetMaxTimerLists( HttpGlobals_t *pHttpGlobals );

unsigned char *Httpd_GetLoginFile( HttpGlobals_t *pHttpGlobals );
#ifdef INTOTO_XML_SUPPORT
unsigned char *Httpd_GetXmlLoginFile( HttpGlobals_t *pHttpGlobals );
#endif /*INTOTO_XML_SUPPORT*/
unsigned char *Httpd_GetLogoutFile( HttpGlobals_t *pHttpGlobals );

unsigned char *Httpd_GetErrorFile( HttpGlobals_t *pHttpGlobals );

unsigned char *Httpd_GetUlogoutFile( HttpGlobals_t *pHttpGlobals );

unsigned char *Httpd_GetLogoFile( HttpGlobals_t *pHttpGlobals );


unsigned char *Httpd_GetXSLLogOutFile( HttpGlobals_t *pHttpGlobals );
 
/**
 * This function is used to
 */
unsigned char *Httpd_GetIndexFile( HttpGlobals_t *pHttpGlobals );

unsigned char *Http_GetCookieName( HttpGlobals_t *pHttpGlobals );

/**
 * This function is used to
 */
unsigned char *Httpd_GetDocPath( HttpGlobals_t *pHttpGlobals );

int32_t Httpd_GetRegModules( HttpGlobals_t *pHttpGlobals,uint32_t *ulMaxModules);

void HttpsGetFileNames( HttpGlobals_t *pHttpGlobals ,unsigned char  **pHCertPath,
                          unsigned char  **pHCertBackupPath );

/**
 * This function is used to
 */
int32_t Https_GetDefPort( HttpGlobals_t *pHttpGlobals );

int32_t  HttpGetDefaultPorts(HttpGlobals_t  *pHttpGlobals ,HttpPortInfo_t **pPortInfo);

int32_t Httpd_GetMaxCookies( HttpGlobals_t  *pHttpGlobals  );

uint32_t  Httpd_GetInBuitLogoStatus( HttpGlobals_t  *pHttpGlobals);

/*  INTOTO_CMS_SUPPORT */
int32_t HttpdManageSession(HttpGlobals_t  *pHttpGlobals,
                                     HttpRequest  *pHttpReq,
                                     unsigned char      *pCookieVal,
                                     bool  bIsEtlCreateRecCall);
int32_t HttpdGetRequiredResource(HttpGlobals_t *pHttpGlobals,
                                               unsigned char *pCookieValue,
                                               void *pOpaqueArg);
int32_t HttpdEmbedInfoToCookieValue(HttpGlobals_t *pHttpGlobals,
                                                    unsigned char *pCookieValue);

int32_t HttpdModOtherInfraInit(void);

int32_t HttpdModOtherInfraDeInit(void);

int32_t Httpd_Init( void );

int32_t HTTPSCertsInit( void );

int32_t HttpsLdSvInit( void );
/* INTOTO_CMS_SUPPORT */
#endif /*  __HTPDGLB_H */

