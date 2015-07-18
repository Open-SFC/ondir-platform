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
/*  File        : httpgdef.h                                              */
/*                                                                        */
/*  Description : This file contains global definitions which are used by */
/*                all the modules of the igateway.                        */
/*                                                                        */
/*  Version      Date      Author      Change Description                 */
/*  -------    --------    ------    ----------------------               */
/*    1.0      00.00.00    kmdhar    Initial implementation               */
/*    1.1      05.14.04    DRAM      Modified during the code review.     */
/**************************************************************************/

#ifndef __HTTPGDEF_H
#define __HTTPGDEF_H


/****************************************************
 * * * *  M a c r o    D e f i n i t i o n s  * * * *
 ****************************************************/

#define bool int
typedef uint32_t ipaddr;

#define  HTTP_LOOPBACK_ADDR      (0x7F000001)  /* 127.0.0.1 */
#define LOOPBACK_PORT_NUM_START          63000
#define HTTP_LOOPBACK_PORT_SRV   (LOOPBACK_PORT_NUM_START + 41)
#define HTTP_LOOPBACK_PORT       (LOOPBACK_PORT_NUM_START + 42)
#define  HTTP_MAX_REG_FDS             5
#define  HTTP_SKIP_OTHER_EVENTS       2
#define  HTTP_DONOT_REMOVE_REQUEST    2
#define  HTTP_DONOT_SEND_RESPONCE     3 

#define  HTTP_SERVER_NORMAL           1
#define  HTTP_SERVER_SSL              2
#define  HTTP_SERVER_ALL              4

#define  HTTP_APPL_TYPE_CLI           1
#define  HTTP_APPL_TYPE_HTTP          2
#define  HTTP_APPL_TYPE_LDSV          4

#define  HTTP_CMD_ADD_PORT            0x01
#define  HTTP_CMD_DEL_PORT            0x02
#define  HTTP_CMD_CHANGE_CONFIG       0x04
#define  HTTP_CMD_ENABLE_PORT         0x08
#define  HTTP_CMD_DISABLE_PORT        0x10
#define  HTTP_CMD_LOAD_DEF_CONFIG     0x11

#define  HTTP_UNAME_LEN               (64)
#define  HTTP_PASSWD_LEN              (32+1)
#define  HTTP_SNETNAME_LEN            (12+1)
#define  HTTP_AUTHORDER_LEN            (12+1)
#define  HTTP_CHALLENGE_LEN           (80)
#define  HTTP_MAX_CCOKIE_TIME_SECS    (10000)

#define  HTTP_SYSTEM_LOGOUT_COOKIE    (0)
#define  HTTP_SYSTEM_LOGIN_COOKIE     (1)

#define  HTTP_CBKFN_PRIV_ADMIN         (0)
#define  HTTP_CBKFN_PRIV_NORMAL        (1)

#define FALSE 0
#define TRUE  1

/**
 * Registration Types.
 */
#define  HTTPD_TAG_TYPE_NONE          ( -1 )
#define  HTTPD_TAG_TYPE_GENERAL       (  0 )
#define  HTTPD_TAG_TYPE_TABLE         (  1 )
#define  HTTPD_TAG_TYPE_ACTION        (  2 )

#define  HTTPD_TAG_TYPE_ACTION_NEW    (  5 ) /* __SAM */
#define  HTTPD_TAG_TYPE_IF            ( 6  )
/**
 * This is added for uploading "images or config" files (.bin only). This
 * is also similar like file post method but file size is big. Simple file
 * post (certificates) mechanism is not changed and its tag name should
 * with ".igw". In addition to old functionality this new functionality is
 * added to upload images; these should register their tag name with ".bin".
 * In application callback one more parameter is added for these new tag
 * types.
 * HTTP2.0 */
#define  HTTPD_TAG_TYPE_FILE_STORE    (  3 )
#define  HTTPD_TAG_TYPE_FILE_SEND     (  4 )

#define  HTTPD_MAX_VARS               ( 120 )
#define  HTTPD_VARIABLE_MAX_LEN       (  64 )

/**
 * usCbTypes to Create, write(append) and close indications of the
 * up-loading Bin file (either image or config)
 */
#define  HTTPD_FILE_CREATE            (0)
#define  HTTPD_FILE_DATA              (1)
#define  HTTPD_FILE_CLOSE             (2)
#define  HTTPD_FILE_VALIDATE          (3)

/**
 * HTTPD VERSION.
 */
#define  IGW_HTTPD_VERSION_LEN        (64)

/***Added by vamsi starts *****/
#define  IGW_HTTPD_MAX_OPAQUE_VARIABLES    8
/*only 'IGW_HTTPD_MAX_OPAQUE_VARIABLES' number of Http call back
 * modules can register for the opaque data.
 */    
/*#ifndef HTTPS_ENABLE
#define HTTPS_ENABLE
#endif */
#define HTTP_MODULENAME_LEN	16

#define  HTTP_MOD_FILENAME_LEN   64 
#define HTTP_EVENT_BASE     	  0                  /* HTTP Event Commands*/
 #define HTTP_EVENT_MOD_REQUEST       HTTP_EVENT_BASE + 1 
 #define  HTTP_EVENT_LOAD_CERTS         HTTP_EVENT_BASE + 2
#define  HTTP_EVENT_SAVE_CERTS          HTTP_EVENT_BASE + 3

 #define HTTP_EVENT_END				   		  255
/* Need to change the existing Http event commands */
 #define HTTP_REG_MAX_PRIORITY   0     /* 0 is the highest priority */
 #define HTTP_UAM_REG_PRIORITY   1 
 #define HTTP_REG_MIN_PRIORITY   100   /* least priority */
 
#define HTTP_REQ_LOGIN     	 	  1      
#define HTTP_REQ_LOGOUT     	  2     
#define HTTP_REQ_IDLE_TIMEOUT     	  3     
#define HTTP_REQ_SESSION_TIMEOUT   	  4     
  
#define  HTTP_POSTSTR_LEN       ( 10*1024 )  /*Increased the length from 4K to 10K*/
#define  HTTP_REQSTR_LEN        ( 640+HTTP_POSTSTR_LEN )
#define  HTTP_FILENAME_LEN      ( 512 )
#define  HTTP_ARGS_LEN          ( 80 ) /* Not Used Currently */
#define  HTTP_CNTTYPE_LEN       ( 128 )
#define  HTTP_PATH_LEN         			 (   16 )
#define  HTTP_MULTIPART_LEN             (13*1024)
typedef void (* FREE_FUN_PTR)(void  *ptr);  
typedef struct HttpReqOpaqueNode_s
{
  bool    bAvailable;
  FREE_FUN_PTR   pFreeFun; 
}HttpReqOpaqueNode_t;

extern HttpReqOpaqueNode_t HttpReqOpaqueStatus[IGW_HTTPD_MAX_OPAQUE_VARIABLES]; 
extern uint32_t  uiOpaqueIndex;
/***Added by vamsi ends *****/

#define HTTP_AUTH_LOCAL            	"Local"   
#define HTTP_AUTH_RADIUS           "Radius"   
#define HTTP_AUTH_LDAP            	"Ldap"   
#define HTTP_AUTH_LIST_LEN 	         (3)
#define HTTP_XML_GETPARAMS_LEN 	         (95+1)
#define HTTP_XML_ASSOCLIST_LEN 	         (5)
/*XML/XSL stuff*/
#define HTTP_BROWSER_INTERNET_EXPLORER   "MSIE"
#define HTTP_BROWSER_MOZILLA                       "Mozilla"
#define HTTP_BROWSER_NETSCAPE                     "Netscape"
#define HTTP_BROWSER_FIREFOX				"Firefox"
#define HTTP_BROWSER_OPERA      			"Opera"
#define HTTP_BROWSER_UNKNOWN                     "UNKNOWN"
#define HTTP_BROWSER_NAME_LEN		   		(20+1)
#define HTTP_BROWSER_VERSION_LEN		    		(10+1)
#define HTTP_XML_TMP_BUFF_LEN                             (255+1) 
#define HTTP_HEADER_BUFF_LEN					(1024+1)
#define HTTP_IPV6_ADDR_PREFIX					"::ffff:"
#define HTTP_EXTERNFD_POLLTABLE_LEN   			    2
#ifdef INTOTO_CMS_SUPPORT
#define HTTP_ADDITIONALFD_POLLTABLE_LEN   		    3
#endif /* INTOTO_CMS_SUPPORT */
#define HTTP_MIMEBUCKET_SIZE                			    27
#define  HTTP_MAX_FILE_SIZE    					(512 *1024)
#define  HTTP_READ_SIZE         					(5 *1024)
#define HTTP_HTML_TMP_BUFF_LEN                          (100)   
#define HTTP_HTML_FILE_INBUFF_LEN                      (1024)
/****************************************************
 * * * *  T y p e    D e c l a r a t i o n s  * * * *
 ****************************************************/

/**
 *
 */
typedef enum __HttpPageType_s
{
  LOGINPAGEREQUEST = 1,
  ERRORPAGEREQUEST = 2,
  SENDMAXAGEZERO   = 3,
  INVALIDPOSTDATA  = 4
} HttPageType_e;

/**
 *
 */
typedef struct AuthPriority_s
{
  unsigned char  ucPasswd[HTTP_PASSWD_LEN];
  unsigned char  ucSnetName[HTTP_SNETNAME_LEN];
  unsigned char  ucChallenge[HTTP_CHALLENGE_LEN];
}AuthPriority_t;

typedef struct  __HttpRequest
{
  //struct cm_ip_addr    PeerIp;
  ipaddr    PeerIp;
  uint16_t  PeerPort;
  unsigned char  *pAppData;
  unsigned char  *filename;
  unsigned char  *inh_content_type;
  unsigned char  *cPostString;
  unsigned char  *pCookieVal;
  unsigned char  *pUserName;
  
  int8_t   iResult;
  unsigned char  *ucFilterId;
  AuthPriority_t *Auth;  
  /**
   * Should not be manipulated by any external module.
   */
  void    *pPvtData;
  uint16_t  SrcPort;
  void                *pApplData; /*Specific to application GET Params */
  void                 *pUserAgent;  
  void                 *pGlobalData; /*Added the pGlobalData for ECOS*/
#ifdef INTOTO_CMS_SUPPORT
  uint32_t                ulSnetID;
#endif /* INTOTO_CMS_SUPPORT */
} HttpRequest;

/**
 * So called DATA-TAG in HTML File, or  Post-Name in Post String.
 */
typedef struct  Httpd_Variable_s
{
  const char  *name_p;
  uint32_t       Size; /* Size Of Variable. i.e. Length of Value */
} Httpd_Variable_t;

typedef struct  Httpd_Value_s
{
  uint32_t  Size;
  char   *value_p;
} Httpd_Value_t;

/* __SAM : start */

/**
 *
 */
typedef struct  Httpd_Tag_Var_values_s
{
  const unsigned char  *name_p;
  unsigned char        *value_p;
  uint32_t        ulLen:16;

#define  HTTP_DATA_TYPE_CHAR          (1)
#define  HTTP_DATA_TYPE_STRING        (2)
#define  HTTP_DATA_TYPE_BYTE          (3)
#define  HTTP_DATA_TYPE_SHORT         (4)
#define  HTTP_DATA_TYPE_INT           (5)

#define  HTTP_HTML_TYPE_STRING        (1)
#define  HTTP_HTML_TYPE_OPTION        (2)
#define  HTTP_HTML_TYPE_RADIO         (3)
#define  HTTP_HTML_TYPE_CHKBOX        (4)
#define  HTTP_HTML_TYPE_DISPLAY       (5)

  uint32_t        ulDataType:8;
  uint32_t        ulHtmlType:8;
  char         *pVarSpecificOption;

} Httpd_Tag_Var_values_t;
/* __SAM : end */

/**
 * Now there is no need to give IGW_, IGW_START_, IGW_TABSTART_ in
 * Tag Name.
 */
typedef struct  Httpd_RegEntry_s
{
  int32_t           Type;       /* See Above for registration types.  */
  char           *pTagName;
  void            *pCbFunct;  /* Callback function pointer.         */
  int32_t           VarCount;
  Httpd_Variable_t  *pVarArray; /* Array of Variables. i.e. data tags. */
  unsigned char          ucPrivilage;
} Httpd_RegEntry_t;

/**
 * Callback function typedef for general tag.
 */
typedef int32_t (*HTTPD_CBFUN_GENERAL)( HttpRequest    *pHttpReq,
                                        Httpd_Value_t  *ValAry,
                                        uint32_t       nCount,
                                        void         *pAppData );

/**
 * Callback function typedef for table tag.
 */
typedef int32_t (*HTTPD_CBFUN_TABLE)( HttpRequest       *pHttpReq,
                                      char           *pLineBuf,
                                      uint32_t          uiLength,
                                      uint32_t          VarCount,
                                      Httpd_Variable_t  *VarAry,
                                      Httpd_Value_t     *ValAry );

/**
 * Callback function typedef for action tag.
 */
typedef int32_t (*HTTPD_CBFUN_ACTION)( HttpRequest    *pHttpReq,
                                       Httpd_Value_t  *ValAry,
                                       uint32_t       nCount );

/**
 * Callback function typedef for file store tag.
 *HTTP2.0 */
typedef int32_t (*HTTPD_CBFUN_FILE_STORE)( HttpRequest    *pHttpReq,
                                           Httpd_Value_t  *ValAry,
                                           uint32_t       nCount,
                                           uint16_t       usCbType );

/**
 * Callback function typedef for file send tag.
 */
typedef int32_t (*HTTPD_CBFUN_FILE_SEND)( HttpRequest  *pHttpReq );

/**
 * Callback function typedef for new action tag.
 */
typedef int32_t (*HTTPD_CBFUN_ACTION_NEW)( HttpRequest  *pHttpReq,
                                           unsigned char     *pPostStr );
/**
 * Callback function typedef for If condition tag.
 */
typedef int32_t (*HTTPD_CBFUN_IF)( HttpRequest  *pHttpReq, unsigned char *TagArg,
				Httpd_Value_t  *pValAry, uint32_t  VarCount ,void  *pAppData);

/**
 * Tunable Configuration For Http Server.
 */

/**
 *
 */
typedef struct  HttpdConfig_s
{
  uint32_t  ulPort;
  uint32_t  ulPriority;
  uint32_t  ulMaxSessions;
  uint32_t  ulMaxPendReqs;
  unsigned char  *pSrvName;
  unsigned char  *pSrvVer;
  unsigned char  *pSUName;
  uint32_t  ulHtmlFsOffset;
  uint32_t  ulHtmlFsSize;
  uint32_t  uiMaxDataBuffer;
  uint32_t  uiMaxTxBuffer;
}HttpdConfig_t;

/* HTTP Listening Ports.
  By default, HTTP server try to listen on 80(clear)/443(secure) ports.
  [Note: These ports are system tunable].
  Incase of binary packaging, if installation script finds that some other
  service is already running on those ports, administrator is prompted to enter
  his desired ports or he can left to system to chose an ephermal port.
*/

typedef struct HttpdListenPorts_s
{
  int32_t   iClearSocketFd;
  int32_t   iSecureSocketFd;
  uint16_t  usClearPort;
  uint16_t  usSecurePort;
} HttpdListenPorts_t;

extern HttpdListenPorts_t HttpdListenPorts;

/**
 * Callback function typedef to get the events.
 */
typedef int32_t (*HTTPD_EVENT_CBKFUN_PTR) (uint32_t);

/**
 * Global FD list maintaining external/additional FDs to be polled
 * This is used to poll the Pseudo Driver FD in case of LINUX to
 * get the Kernel event when HTTP Server Listen port configuration
 * is changed by user. For other OS it is not needed.
 */
typedef struct  HttpdExternFd_s
{
  uint32_t                ulFd;   /* Fd to be polled. */
  HTTPD_EVENT_CBKFUN_PTR  CbkFun; /* Callback Function to be invoked. */
} HttpdExternFd_t;
#ifdef INTOTO_CMS_SUPPORT
/**
 * Callback function typedef to get the events.
 */
typedef int32_t (*HTTPD_ADDITINAL_EVENT_CBKFUN_PTR) (int32_t,void*);

typedef struct  HttpdAdditionalFd_s
{
  uint32_t                ulFd;   /* Fd to be polled. */
  HTTPD_ADDITINAL_EVENT_CBKFUN_PTR  CbkFun; /* Callback Function to be invoked. */
  void   *pArg;
} HttpdAdditionalFd_t;
#endif /* INTOTO_CMS_SUPPORT */
typedef enum __hServerType_e
{
  hServerNormal,
  hServerSSL
} hServerType_e;

typedef  struct HttpPortInfo_s
{
  uint16_t         usPort;
  hServerType_e    eServerType;
  bool          bEnabled;
  bool          bDontSave;
}HttpPortInfo_t;

typedef struct HttpPortArgs_s
{
  unsigned char             ucCmd;
  uint16_t             usOldPort;
  HttpPortInfo_t       PortInfo;
}HttpPortArgs_t;

typedef struct HttpTagArgs_s
{
  char           *pTagName;
  void            *pCbFunct;
  int32_t            VarCount;
  Httpd_Variable_t  *pVarArray;
  unsigned char           ucPrivilage;
}HttpTagArgs_t;

typedef struct HttpAPIRegModuleInfo_s
{
  char  ModuleName[HTTP_MODULENAME_LEN];/*Name of the module*/
  unsigned char   ucPriority;   /* Priority for Module        */
  unsigned char  Loginfile[HTTP_MOD_FILENAME_LEN]; /*login page */
  unsigned char  Logoutfile[HTTP_MOD_FILENAME_LEN]; /*logout page */
}HttpAPIRegModuleInfo_t;

typedef struct HttpRegModuleRequest_s
{
  struct cm_ip_addr       SrcIp;    /* source ip address of the request */
  uint16_t     SrcPort; /* source port of the request */
  unsigned char     ReqType;
  struct HttpRegModuleRequest_s  *pNext;
}HttpRegModuleRequest_t;

typedef struct HttpRegModuleRecList_s
{
  uint32_t ulModuleId; /*Unique module ID */
  char  ModuleName[HTTP_MODULENAME_LEN]; /*Name of the module*/
  unsigned char   ucPriority;   /* Priority for Module        */
  unsigned char  Loginfile[HTTP_MOD_FILENAME_LEN];      /*login page */
  unsigned char  Logoutfile[HTTP_MOD_FILENAME_LEN];  /*logout page */
  bool    bHeap;
  struct HttpRegModuleRecList_s    *pNextModule;
  HttpRegModuleRequest_t  *pNext;
}HttpRegModuleRecList_t;

typedef struct HttpRegModuleRequestInfo_s
{
  char      ModuleName[HTTP_MODULENAME_LEN]; /*Name of the module*/
  struct cm_ip_addr       SrcIp;   /* source ip address of the request */
  uint16_t     SrcPort; /* source port  of the request */
  unsigned char     ReqType;
}HttpRegModuleRequestInfo_t;

typedef struct HttpRegModuleRecFreeQ_s
{
   HttpRegModuleRecList_t * pHead;
   HttpRegModuleRecList_t * pTail;
   uint32_t ulFreeCount;
}HttpRegModuleRecFreeQ_t;

typedef struct HttpEvent_s
{
  uint32_t                 uiEventType;  
  uint32_t                 uiApplicationType;  		
  uint32_t		   uiTotalSize;
  void		   *pOpaque;
  HttpRegModuleRequestInfo_t  ModuleRecInfo;
}HttpEvent_t;

typedef int32_t (*HTTP_ASYNC_REG_CBK) (int32_t,void *);

typedef struct
{
  int32_t               iSockFd;
  uint32_t              uiInterstedEvents;
  HTTP_ASYNC_REG_CBK    pFnError;
  HTTP_ASYNC_REG_CBK    pFnRead;
  HTTP_ASYNC_REG_CBK    pFnWrite;
  void		*user_data_p;
}HttpAsyncExternFdReg_t;

typedef  void (*HTTPD_TIMER_CBFUN)(void *);

typedef struct Httpd_Timer_s
{
  HTTPD_TIMER_CBFUN     pRoutine;
  void *              pArg;
  int32_t               Count;
  uint32_t              Value;
  uint32_t              Type;
  bool               bDynamic;
  uint32_t              uidebugInfo;
  uint32_t              iActive;
  struct Httpd_Timer_s  *pNext;
  struct Httpd_Timer_s  *pPrev;
} Httpd_Timer_t; 

typedef struct HttpdCbkTimers_s  
{
   HTTPD_TIMER_CBFUN HttpdKeepAliveTimeoutCbk;
   HTTPD_TIMER_CBFUN HttpdReqStrTimeoutCbk;   
   HTTPD_TIMER_CBFUN HttpdResponseTimeoutCbk;    
}HttpdCbkTimers_t;

/**
 * Enumerated values representing the names of http methods.
 */
typedef enum __hMethod_e
{
  hMethodGet  =  9000,
  hMethodHead,
  hMethodPost,
  hMethodBad,
  hEventTimeOut,
  hEventClose
} hMethod_e;

/**
 * Enumerated values representing the names of http versions.
 */
typedef enum __hVersion_e
{
  hVersion09 = 0,
  hVersion10,
  hVersion11,
  hVersionBad
} hVersion_e;


typedef struct HttpdSetCookieInfo_s
{
  /**
   * To know allocated at beginning or Dynamic allocation.
   */
  uint16_t                     bHeep;
  uint32_t                     ulTime;
  unsigned char                     *pCookieName;
  unsigned char                     *value_p;
  unsigned char                     *pPath;
  /**
   * To hold pCookieName, pPath, value_p.  It is expected that all three
   * strings don't exceed 128 characters.
   */
  unsigned char                     *CookieBuff;
  struct HttpdSetCookieInfo_s  *pNext;
} HttpdSetCookieInfo_t;

typedef struct HttpdGetCookieInfo_s
{
  /**
   * To know allocated at beginning or Dynamic allocation.
   */
  uint16_t                     bHeep;
  unsigned char                     *pCookieName;
  unsigned char                     *value_p;
  unsigned char                     *pPath;
  /**
   * To hold pCookieName, pPath, value_p.  It is expected that all three
   * strings don't exceed 128 characters.
   */
  unsigned char                     *CookieBuff;
  struct HttpdGetCookieInfo_s  *pNext;
} HttpdGetCookieInfo_t;

typedef struct HttpCookieStats_s
{
  uint32_t  ulAdditions;
  uint32_t  ulDeletions;
  uint32_t  ulTimeOuts;
  uint32_t  ulLogIns;
} HttpCookieStats_t;



typedef struct HttpCookie_s
{
  struct HttpCookie_s *pNext;
  unsigned char       ucUserName[HTTP_UNAME_LEN + 1];
  unsigned char       *ucValue;
  unsigned char       ucChallenge[HTTP_CHALLENGE_LEN + 1];
  uint32_t       ulVersion;
  unsigned char       ucPath[HTTP_PATH_LEN + 1];
  uint32_t       ulMaxAge;
  bool        bLoggedIn;
  Httpd_Timer_t  *pTimer;
  unsigned char        ucIp[40];
  uint16_t       usPort;
  uint32_t       uiSNetId;
  uint32_t       uiBytesWritten;
  void         *pBinFileEntry;
  uint32_t       ulLoginTime;
} HttpCookie_t;


typedef enum __hState_e
{
  hStateReadReq   = 0,   /* Waiting For Completion Of Entire Request */
  hStateReadPStr  = 1,   /* Reading Post String */
  hStateReadMPart = 2,  /* Reading Multi-Part Data */
  hStateSendResp  = 3,  /* Writing Response */
  hStatePipeline  = 4,     /* Forming request pipe */
  hStateFinished  = 5     /* Completed Sending Response */
} hState_e;

typedef struct HttpReadInfo_s
{
  unsigned char  *pBuf;
  unsigned char  *pCur;
  uint32_t  ulMaxLen;
} HttpReadInfo_t;

typedef struct HttpMPartInfo_s
{
  unsigned char  ucBuf[HTTP_MULTIPART_LEN+1];
} HttpMPartInfo_t;
typedef struct HttpTransmitBuffer_s
{
  unsigned char                      *pDataBuffer;
  struct  HttpTransmitBuffer_s  *pNext;
}HttpTransmitBuffer_t;

/**
 * Http method information structure that maintains the name of the method
 * and its respective enumerated value.
 */
typedef struct _HttpMethodInfo
{
  hMethod_e  hmId;
  char    *hmName;
  int16_t    lhmValue;
} HttpMethodInfo;

/**
 * Http version information structure that maintains the name of the version
 * and its respective enumerated value.
 */
typedef struct _HttpVersionInfo
{
  hVersion_e  hvId;
  char     *hvName;
} HttpVersionInfo;
typedef struct _HtErrorMsgInfo
{
  int32_t  code;
  char  *msg;
} HtErrorMsgInfo;
/**
 * MIME Related Stuff.
 */
typedef struct hMimeType_s
{
  char                 *ContentType;
  char                 *Ext;
  struct hMimeType_s      *Chain;
}hMimeType_t;

typedef struct HttpClientReq_s
{
 
  //struct cm_ip_addr        PeerIp;
  ipaddr               PeerIp;
  uint16_t             PeerPort;
  int32_t		SockFd;   /*IGWSOCKHANDLE*/
  /*HTTP 1.1 START*/
  int32_t                lServId; /* Index Of Server */
  bool		bPersistentConn;
  bool		bSendResponce;
  bool		bHost;
  bool             bDrop;  
  bool		bIsPipeLined;
  int32_t              iReqTableIndex;
  int32_t              iPipeLineTableIndex;
  int32_t              iPipeLineSubIndex;
  unsigned char          *pEndOfHeader;
  bool              bIsAcceptFd;
  unsigned char           * ucRemainBuffer;
  bool               bIsHalfPacketRecv;
  uint32_t              ulTotalFileSize;
  /*HTTP 1.1. END*/
  unsigned char           *ucCookieVal;
  unsigned char              ucUserName[HTTP_UNAME_LEN + 1];
  uint32_t               ulPageType;
  uint32_t               ulStatus;
  hState_e                eState;   /* State Of The Connection */
  hMethod_e             eMethod;
  hVersion_e             eVersion;

  unsigned char              ucReqStr[HTTP_REQSTR_LEN+1];
  unsigned char              ucPostString[HTTP_POSTSTR_LEN+1];
  HttpMPartInfo_t       *pMPart;
  HttpReadInfo_t        ReadInfo;

  unsigned char              *ucUrl;
  unsigned char              ucFileName[HTTP_FILENAME_LEN+1];
  unsigned char              ucArgs[HTTP_ARGS_LEN+1];

  unsigned char              ucInContentType[HTTP_CNTTYPE_LEN+1];
  unsigned char              ucOutContentType[HTTP_CNTTYPE_LEN+1];
  int32_t                  lInContentLen;
  int32_t                  lOutContentLen;
  bool                 bExpireByOneDay;
  unsigned char               *ucAppData;
  Httpd_Timer_t         *pTimer;   /* Timeout For Reading Entire Request */  
  void                  *pSpData;  /* Specific Data. SSL Can Use This */
  HttpdSetCookieInfo_t  *pSetCookieHead;   /* Linked List for Set cookie */
  HttpdGetCookieInfo_t  *pGetCookieHead;   /*    ,, */

  /**
   * Using a Set API external modules will set the opaque data in
   * the following array and the index of the array is known before
   * the execution of the SET API through the Regestration API.
   */
  void                *pHttpReqOpaqueNode[IGW_HTTPD_MAX_OPAQUE_VARIABLES];
  uint16_t              SrcPort;
  /*
   * The following 3 fields will be used for Concurrency.
   * uiTxBufLen - Length of the total transmit buffer to be sent
   * uiTxBufOffset - Offset of the data sent in tansmit buffer
   * pTransmitBuffer - Response to be sent will be stored in chunks 
   *                   of size HTTP_MAX_DATA_BUFFER as a linked list 
   *                   and will be attached to it.
   */
  uint32_t               uiTxBufLen;
  uint32_t               uiTxBufOffset;
  HttpTransmitBuffer_t  *pTransmitBuffer;
  void                *pApplData; /*Specific to application GET Params */
  void                *pUserAgent;  
  unsigned char		  ucHeaderBuff[HTTP_HEADER_BUFF_LEN];
  uint32_t              ulConnType;
  void                 *pGlobalData; /*Added the pGlobalData for ECOS*/  
  char               *pSendHeader;
  bool                bNeedMoreData; 
  bool                bISPgUserLoginReq;
  bool                bRefererFound;
#ifdef INTOTO_CMS_SUPPORT
  bool                bDoNotRemoveRequest;   
#endif /* INTOTO_CMS_SUPPORT */
} HttpClientReq_t;

typedef struct HttpAuthOrder_s
{
 unsigned char  pAuthName[HTTP_AUTHORDER_LEN ];
 }HttpAuthOrder_t;
 
typedef struct IGWGetParams_s
{
 unsigned char ucName[HTTP_XML_GETPARAMS_LEN];
 unsigned char ucValue[HTTP_XML_GETPARAMS_LEN];
}IGWGetParams_t;
/*This is specific to XML/XSL stuff to find out the browser name and version*/
typedef struct IGWUserAgentValues_s
{
   unsigned char ucBrowserName[HTTP_BROWSER_NAME_LEN];
   unsigned char ucBrowserVersion[HTTP_BROWSER_VERSION_LEN]; 
}IGWUserAgentValues_t; 
/**
 * HTTP Protocol Related Declarations.
 */

struct HttpServer_s;

typedef struct HttpServStats_s
{
  uint32_t  ulAccepts;
  uint32_t  ulRejects;
  uint32_t  ulClientAllocFails;
  uint32_t  ulClientAllocs;
  uint32_t  ulClientFrees;
  uint32_t  ulReqStrTimeOuts;
  uint32_t  ulMaxKeepAlives;      /*Maximum Requests that can be allowed on a sigle connection*/ 
} HttpServStats_t;

/**
 *
 */
typedef int32_t (*cbkClientAccept)( struct HttpServer_s  *,
                                    HttpClientReq_t      * );

/**
 *
 */
typedef int32_t (*cbkClientRead)( HttpClientReq_t  *,
                                  unsigned char         *pBuf,
                                  uint32_t         ulLen,
                                  int32_t          *pBytesRead );

/**
 *
 */
typedef int32_t (*cbkClientWrite)( HttpClientReq_t  *,
                                   unsigned char         *pBuf,
                                   uint32_t         ulLen );

/**
 *
 */
typedef int32_t (*cbkClientClose)( HttpClientReq_t  * );

/**
 *
 */
typedef int32_t (*cbkHtmlFilter)( char  *pOrgName,
                                  char  *pNewName,
                                  char  *pAppData );

/**
 *
 */
typedef int32_t (*cbkHtmlAccess)( char  *pFileName,
                                  int32_t  *pPermission );

typedef struct HttpServer_s
{
  hServerType_e    Type;
  bool          bEnabled;
  uint32_t         ulMaxSess;
  uint32_t         ulCurSess;
  struct cm_ip_addr      IpAddr;
  uint16_t         uiPort;
  int32_t           SockFd;/*IGWSOCKHANDLE*/
  cbkClientAccept  ClientAccept;
  cbkClientRead    ClientRead;
  cbkClientWrite   ClientWrite;
  cbkClientClose   ClientClose;
  cbkHtmlFilter    HtmlFilter;
  cbkHtmlAccess    HtmlPermission;
  void           *pSpData; /* Specific Data. SSL Can Use This */
  HttpServStats_t  Stats;
} HttpServer_t;

typedef struct HtmlFileBuf_s
{
  int32_t   ulFileSize;
  int32_t   ulCurrOffset;
  unsigned char  HttpUnzipBuf[HTTP_MAX_FILE_SIZE + HTTP_READ_SIZE];
  unsigned char  HttpInputBuf[HTTP_MAX_FILE_SIZE + HTTP_READ_SIZE];
  unsigned char  TmpBuf1[HTTP_HTML_TMP_BUFF_LEN];
  unsigned char  HttpInBuf[HTTP_HTML_FILE_INBUFF_LEN];
  unsigned char  TmpBuf2[HTTP_HTML_TMP_BUFF_LEN];
} HtmlFileBuf_t;

typedef struct HttpKeepAliveFd_s
{
    int32_t    iSockFd;/*IGWSOCKHANDLE*/
    Httpd_Timer_t   *pKeepAliveTimer;   
}HttpKeepAliveFd_t;
typedef HtmlFileBuf_t HttpFileBuf_t;

#endif /*  __HTTPGDEF_H */

