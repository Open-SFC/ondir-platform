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
/*  File        : httpgif.h                                               */
/*                                                                        */
/*  Description : This file contains function prototypes which are used by*/
/*                all the modules of the igateway.                        */
/*                                                                        */
/*  Version      Date      Author      Change Description                 */
/*  -------    --------    ------    ----------------------               */
/*    1.0      00.00.00    kmdhar    Initial implementation               */
/*    1.1      05.10.04    DRAM      Modified during the code review.     */
/**************************************************************************/

#ifndef _HTTPGIF_H
#define _HTTPGIF_H

/********************************************************
 * * * *  F u n c t i o n    P r o t o t y p e s  * * * *
 ********************************************************/

/**
 * Functions implemented in httpd.c file.
 */

/**
 * This function is used to initialize the data structures used by the http
 * server.
 */
int32_t Httpd_Init( void );

/**
 * This function is used to get the IP address used in the socket used by
 * the http server.
 */
int32_t HttpGetSocketIPAddress( HttpRequest  *pHttpRequest,
                                ipaddr    *pIPAddress );

/**
 * This function is used to get the IP address used in the socket used by
 * the http server.
 */

int32_t HttpdGetSockIPAddress( HttpRequest  *pHttpRequest,
                                ipaddr *pIPAddress );

/**
 * This function is used to
 */
void Httpd_Check4Certificate( void );

/**
 * This function is used to
 */
int32_t HtpsHttp_RegLocalTags( void );

/**
 * This function is used to
 */
int32_t Httpd_SendCookieInfo( HttpRequest  *pHttpRequest );

/**
 * This function is used to
 */
int32_t Httpd_SetCookieInfo( HttpRequest  *pHttpRequest,
                             unsigned char     *pCookieName,
                             unsigned char     *value_p,
                             unsigned char     *pPath,
                             uint32_t     ulTime );

/**
 * This function is used to
 */
int32_t Httpd_GetCookieValue( HttpRequest  *pHttpRequest,
                              unsigned char     *pCookieName,
                              unsigned char     *value_p,
                              uint16_t     usMaxLen );

/**
 * This function is used to
 */
int32_t HttpVerifyConnReq( ipaddr    peerIp,
                           uint16_t  servPort );

/**
 * This function is used to register the local tags, which are specific to
 * http server.
 */
int32_t Httpd_RegisterLocalTags( void );

/**
 * This function is used to
 */
void  HttpRequest_SetPageType( HttpRequest  *pHttpRequest,
                                 uint32_t     ulPageType );

/**
 * This function is used to
 */
char *HttpGetAppData( HttpRequest  *pHttpRequest );

/**
 * This function is used to
 */
int32_t HttpRetriveIpAddrFromValArray( Httpd_Value_t  *ValArray,
                                       uint32_t       *pIpAddr,
                                       int32_t        iIndex );

/**
 * R e g i s t r a t i o n    S t u f f
 */

/**
 * This function is used to register a general tag.
 */
int32_t Httpd_Register_GenTag( const char     *pTagName,
                               void            *pCbFunct,
                               int32_t           VarCount,
                               Httpd_Variable_t  *pVarArray );

/**
 * This function is used to register a table tag.
 */
int32_t Httpd_Register_TableTag( const char     *pTagName,
                                 void            *pCbFunct,
                                 int32_t           VarCount,
                                 Httpd_Variable_t  *pVarArray );

/**
 * This function is used to register an action tag.
 */
int32_t Httpd_Register_ActionTag( const char     *pTagName,
                                  void            *pCbFunct,
                                  int32_t            VarCount,
                                  Httpd_Variable_t  *pVarArray );

/**
 * This function is used to register a file store tag.
 */
int32_t Httpd_Register_FileStoreTag( const char     *pTagName,
                                     void            *pCbFunct,
                                     int32_t            VarCount,
                                     Httpd_Variable_t  *pVarArray );

/**
 * This function is used to register a file send tag.
 */
int32_t Httpd_Register_FileSendTag( const char     *pTagName,
                                    void            *pCbFunct,
                                    int32_t            VarCount,
                                    Httpd_Variable_t  *pVarArray );

/**
 * This function is used to any type of tag.
 */
int32_t Httpd_Register_Tag( int32_t           TagType,
                            HttpTagArgs_t     TagArgs);

/**
 * This function is used to any type of tag with the parse engine.
 */
int32_t Httpd_Register_TagEx( Httpd_RegEntry_t  *pThis );

/**
 * This function is used to register a tag array.
 */
int32_t Httpd_Register_TagArray( Httpd_RegEntry_t  *pAry );

/**
 * This function is used to de register a given tag.
 */
int32_t Httpd_DeRegister_Tag( int32_t        Value,
                              const char  *pTagName );
/**
* This function is used to register a if condition support tag.
*/
int32_t Httpd_Register_IfTag( const char    *pTagName,
		              void            *pCbFunct,
			      int32_t           VarCount,
			      Httpd_Variable_t  *pVarArray );
/**
 * HTTPD - Support    A P I
 */

/**
 * This function is used to
 */
int32_t Httpd_Value_CopyStr( Httpd_Value_t  *pVal,
                             char        *pStr );

/**
 * This function is used to
 */
int32_t Httpd_Value_CopyInt( Httpd_Value_t  *pVal,
                             int32_t        uimyVal );

/**
 * This function is used to
 */
int32_t Httpd_Value_CopyUInt( Httpd_Value_t  *pVal,
                              uint32_t       uimyVal );

/**
 * This function is used to
 */
int32_t Httpd_Value_AppendStr( Httpd_Value_t  *pVal,
                               char        *pThisStr );

/**
 * This function is used to
 */
int32_t Httpd_Value_StrCmp( Httpd_Value_t  *pVal,
                            char        *pThisStr );

/**
 * This function is used to
 */
int32_t Httpd_Value_StrCaseCmp( Httpd_Value_t  *pVal,
                                char        *pThisStr );

/**
 * This function is used to
 */
int32_t Httpd_Send_PlainString( HttpRequest  *pHttpRequest,
                                char      *pString );

/**
 * This function is used to
 */
int32_t Httpd_Send_PlainBuf( HttpRequest  *pHttpRequest,
                             char      *pHtmlBuf,
                             uint32_t     uiLength );

/**
 * This function is used to
 */
int32_t Httpd_Send_DynamicString( HttpRequest       *pHttpRequest,
                                  char           *pHtmlBuf,
                                  uint32_t          VarCount,
                                  Httpd_Variable_t  *pVarAry,
                                  Httpd_Value_t     *pValAry );

/**
 * This function is used to
 */
int32_t Httpd_Send_DynamicBuf( HttpRequest       *pHttpRequest,
                               char           *pHtmlBuf,
                               uint32_t          uiLength,
                               uint32_t          VarCount,
                               Httpd_Variable_t  *pVarAry,
                               Httpd_Value_t     *pValAry );

/**
 * This function is used to
 */
int32_t Httpd_Send_HyperString( HttpRequest  *pHttpRequest,
                                char      *pHtmlBuf );

/**
 * This function is used to
 */
int32_t Httpd_Send_HyperStringEx( HttpRequest  *pHttpRequest,
                                  char      *pHtmlBuf,
                                  bool      bReadOnly );

/**
 * This function is used to
 */
int32_t HTTPGetFile( HttpRequest  *pHttpRequest,
                     char      **pFileBuf );

/**
 * This function is used to
 */
int16_t HttpSendHtmlFile( HttpRequest  *pHttpRequest,
                          unsigned char     *pFileName,
                          unsigned char     *pPostValue );


/**
 * Tunable Configuration API.
 * The Should Be Implemented For Porting.
 */

/**
 * This function is used to
 */ 
void Httpd_GetConfig( HttpdConfig_t  *pConf );

/**
 * This function is used to
 */
int32_t Httpd_PostString_Compile( char                 *pPostString,
                                  Httpd_Tag_Var_values_t  *pVarAry,
                                  uint32_t                *pArrLen );

/**
 * This function is used to
 */
int32_t Httpd_FillGetValArray( HttpRequest             *pReq,
                               Httpd_Variable_t        *pVarArray,
                               Httpd_Tag_Var_values_t  *value_ps,
                               uint32_t                ulValCount,
                               Httpd_Value_t           *pValArray );

/**
 * This function is used to
 */
int32_t Httpd_HandleSystemCookies( HttpRequest  *pHttpReq,
                                   uint32_t     uiTemp );

/**
 * This function is used to know whether the user has admin privilages
 */
bool HttpUserHasAdminPriv(char* pUserName);
bool HttpUserIsConfigType(char* pUserName);

int32_t IGWHttpSetOpaqueData( HttpRequest  *pHttpReq,
                              uint32_t     uiIndex,
                              void       *pData );

void *IGWHttpGetOpaqueData( HttpRequest  *pHttpReq,
                              uint32_t     uiIndex );

int32_t SetOpaqueData( HttpRequest  *pHttpReq,
                       uint32_t     uiIndex );

int32_t HttpInitPortList(void);
int32_t HttpDeInitPortList(int32_t,bool);
int32_t HttpAddPortToList(HttpPortInfo_t *PortInfo);
int32_t HttpDelPortFromList(HttpPortInfo_t *PortInfo);
int32_t HttpGetFirstPort (HttpPortInfo_t *pPortInfo);
int32_t HttpGetNextPort (HttpPortArgs_t *pPort);
int32_t HttpGetMaxConfiguredServers(hServerType_e *eServerType);
int32_t HttpGetPortInfo(HttpPortInfo_t *PortInfo);


int32_t HttpAddPort(HttpPortInfo_t PortInfo);
int32_t HttpDeletePort (uint16_t usPortNum);
int32_t HttpChangeConfiguration (uint16_t usOldPortNum,
                                 HttpPortInfo_t PortInfo);


int32_t HttpEnablePort (uint16_t uiPortNum);
int32_t HttpDisablePort (uint16_t uiPortNum);

int32_t HttpGetMaxServers(hServerType_e eServerType);

int32_t HttpGetErrorMsg (uint32_t uiErrIndex, char *pMsg);
int32_t HttpErrorMsgInit(void);

void HttpInstallLeaves(void);
 int32_t LdsvHttpdLoad(unsigned char *pFileName,uint32_t ulRecId);
 int32_t LdSvHTTPDInit(void);


int32_t HttpSetMaxServersInList(int32_t *pMaxServers);
int32_t HttpChangePortInfoInList(HttpPortArgs_t *pPortArgs);
int32_t ChangeHttpPortTagsInit( void );

int32_t HttpDummyPrint(char *a,...);

/**
 * Redirect the Http Server on Changed IP address.
 */
int32_t HttpRedirectServer( HttpRequest  *pHttpRequest,
                            ipaddr LANIPAddr );

int32_t HttpClientSocketClose( HttpRequest  *pHttpRequest );

/**
 * Functions implemented in the httpkuki.c file
 */

/**
 * This function is used to get the cookie information.
 */
int32_t HttpCookie_GetByVal( unsigned char  *pCookieVal,
                             bool   *pLoggedIn,
                             unsigned char  *pChallenge,
                             unsigned char  *pUserName);

/**
 * This function is used to get the SNet identification for the given cookie.
 */
int32_t HttpCookie_GetSNetId( unsigned char  *pCookieVal,
                              uint32_t  *puiSNetId );

/**
 * This function is used to set the information related to the user after
 * successful login.
 */
int32_t HttpCookie_LoggedIn( unsigned char      *pCookieVal,
                             unsigned char      *pUserName,
                             uint32_t      ulNewTime );

/**
 * This function is used to set the information related to the port after
 * successful login.
 */
int32_t HttpCookie_AddPortInfo( unsigned char  *pCookieVal,
                                uint16_t  usPort );

/**
 * This function is used to set the information related to the port after
 * successful login.
 */
int32_t HttpCookie_AddSNetId( unsigned char  *pCookieVal,
                              uint32_t  uiSNetId );

/**
 * This function is used to delete the cookie entry for the given cookie
 * value.
 */
int32_t HttpCookie_Del( unsigned char  *pCookieVal );

/**
 * This function is used to delete the cookie entry for the given user
 * name.
 */
int32_t HttpCookie_DelByName( unsigned char  *name_p );

/**
 * This function is used to delete the cookie by SNet name.
 */
int32_t HttpSNetCookie_Del( HttpRequest  *pHttpRequest );
int32_t ChangePortInfo(HttpPortArgs_t PortArgs);
int32_t DeletePortFromServer(uint16_t usPort);
int32_t HttpLoadDefaultPortConfig(void);


/**
 * This function is used to verify whether the given user is looged in or
 * not. 
 */
bool HttpCookie_IsUserLoggedIn( unsigned char  *pUserName ,unsigned char *PeerIp,unsigned char *ucCookieValue);

void Httpd_RegEvents (void);
void Httpd_DeRegEvents (void);
void HttpEventCbFunc(uint32_t uiEventId,uint32_t uiApplicationType,
    uint32_t uiTotalSize, void *pOpaque, void *pData, void *pCbArg);


int32_t HttpCookie_DelByIpAddress( unsigned char  *Peer);

unsigned char *Httpd_GetSrcIpAddress(unsigned char  *pUserName);


#ifdef INTOTO_XML_SUPPORT
int32_t Httpd_SendXMLBuff( HttpRequest  *pHttpRequest,
				                  char      *pHtmlBuf,
				                  uint32_t     ulLength );
#endif /*INTOTO_XML_SUPPORT*/

int32_t Httpd_GetFileExt( unsigned char  *pFileName,
                          unsigned char  *pExt,
                          uint32_t  ulMaxExt );


int32_t HttpFreeTxBuf(HttpTransmitBuffer_t *pTxBuf);
/**
 * This is the state handler for Read MultiPart Data. This checks for the
 * completion of data. If completed, it changes the state to Send Response.
 */
int32_t HttpClient_ReadMPart_ReadEvent( HttpClientReq_t  *pClientReq );
/**
 * This is the state handler for 'ReadReq'. This checks for the completion
 * of request string. If completed, it changes the current state to
 * appropriate state depending on type of method.
 */
int32_t HttpClient_ReadReq_ReadEvent( HttpClientReq_t  *pClientReq );

/**
 * This function is used to find the cookie entry for the given cookie
 * value.
 */
HttpCookie_t *HttpCookie_Find( unsigned char  *pCookieVal,
                               uint32_t  *pIndex );

/**
 * This function is used to find the cookie entry for the given user name.
 */
HttpCookie_t *HttpCookie_FindByName( unsigned char  *name_p,
                                     uint32_t  *pIndex );


int32_t HttpLdapRespCallBack(HttpRequest *pHttpReq);
/*This function is used for URL parsing */
int32_t HttpdXmlParseUrl(unsigned char *value_p,IGWGetParams_t *pXmlArray);
int32_t HttpClient_FindBrowserNameAndVersion(HttpClientReq_t  *pClientReq,unsigned char *pUserAgent);
void  HttpKernelInit(void);
int32_t	HttpdNumToPresentation(struct cm_ip_addr *IPv6Addr, unsigned char *cOutPutStr);

bool HttpdSocketValidateErrorno(void);

/**
 * This function is used
 */
void  HttpClient_ReqStrTimeOut( void  *pVal );

void  HttpClient_StartResponse( void  *pVal);


void HttpClient_KeepAliveTimeOut( void  *pVal );

int32_t  HttpdCbkTimerInit(HttpClientReq_t *pClientReq);

#ifdef INTOTO_CMS_SUPPORT
int32_t  HttpdCMSDevRegisterLocalTags(void) ;
#endif /* INTOTO_CMS_SUPPORT */

#ifdef OF_CM_SUPPORT
int32_t  IGWUcmHttpInit(void) ;

int32_t Httpd_SendXMLBuff( HttpRequest  *pHttpRequest,
				                  char      *pHtmlBuf,
				                  uint32_t     ulLength );
#endif /* OF_CM_SUPPORT */

int32_t HttpGetDefClearListenPort(uint16_t *pusClearPort);

#ifdef OF_HTTPS_SUPPORT
int32_t HttpGetDefSecureListenPort(uint16_t *pusSecurePort);
#endif /*OF_HTTPS_SUPPORT*/

#endif /*  _HTTPGIF_H_ */
