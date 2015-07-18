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
/*  File        : httpdcfg.c                                              */
/*                                                                        */
/*  Description : This file contains global configuration for http server */
/*                related to lxpc system.                                 */
/*                                                                        */
/*  Version      Date      Author      Change Description                 */
/*  -------    --------    ------    ----------------------               */
/*    1.0      00.00.00    kmdhar    Initial implementation               */
/*    1.1      05.20.04     dram     Modified during the code review.     */
/*    1.2      22.02.06    Harishp   Modified during MultipleInstances	  */	
/*                                   and Multithreading	                  */
/**************************************************************************/

#ifdef OF_HTTPD_SUPPORT

/***********************************************************
 * * * *  I n c l u d e    H e a d e r    f i l e s  * * * *
 ***********************************************************/

#include "cmincld.h"
#include "httpwrp.h"
#include "httpgif.h"
#include "basic_tunable.h"
#include "httpdadv.h"
#include "httpgdef.h"
#include "httpcfg.h"
#include "htpdglb.h"
#include "lxoswrp.h"

/************************************************************
 * * * *  V a r i a b l e    D e c l a r a t i o n s  * * * *
 ************************************************************/

#ifdef OF_HTTPS_SUPPORT

/**
 * Path related to the https certificates.
 */
unsigned char  *HTTPS_CERT_PATH = (unsigned char *) "/ucm/certs/htpscert";

/**
 * Path related to the backup of https certificates.
 */
unsigned char  *HTTPS_CERT_BACKUP_PATH = (unsigned char *) "/ucm/certs/htpscert.bak";

#endif /* OF_HTTPS_SUPPORT */

HttpPortInfo_t DefaultPortConfig[HTTP_MAX_SERVERS] = {
                                /*{PortNum, Type of server, Enable status}*/
                                {80, hServerNormal, TRUE}
#ifdef HTTPS_ENABLE
#ifdef OF_SSLVPN_PROD_SUPPORT
                                ,{8443, hServerSSL, TRUE}
#else
                                ,{443, hServerSSL, TRUE}
#endif 
#endif
                               };


/**
 * Html files allowed for a normal user.
 */
/*TR_PRIVATE*/ unsigned char  *userHtmlPages[] =
{
  (unsigned char  *)"login.htm",
#ifdef OF_UCM_SUPPORT
  (unsigned char  *) "ucm",
#endif /* OF_UCM_SUPPORT */
#ifdef OF_CMS_SUPPORT
 (unsigned char  *)  "cms",
#endif /* OF_CMS_SUPPORT */
(unsigned char  *)   "loginout.htm",
(unsigned char  *)   "errlogin.htm",
(unsigned char  *)   "rlogout.htm",
(unsigned char  *)   "login.igw",
(unsigned char  *)   "logout.igw",
(unsigned char  *)   "pswdchng.igw",
(unsigned char  *)   "ulogout.htm",
(unsigned char  *)   "pswdchng.htm",
(unsigned char  *)   "spacer.gif",
(unsigned char  *)   "logviewer.jar",
(unsigned char  *)   "lv.jnlp",
(unsigned char  *)  "charsets.jar",
(unsigned char  *)   "cryptix32.jar",
(unsigned char  *)   "jbcl.jar",
(unsigned char  *)   "jcommon-0_9_5.jar",
(unsigned char  *)   "jfreechart-0_9_20.jar",
(unsigned char  *)   "jcommon-1_0_12.jar",
(unsigned char  *)   "jfreechart-1_0_8.jar",
(unsigned char  *)   "commons-collections-3_1.jar",
(unsigned char  *)   "commons-logging-1_0_4.jar",
(unsigned char  *)   "itext-2_0_4.jar",
(unsigned char  *)   "jta-spec1_0_1.jar",
(unsigned char  *)   "quartz-all-1_6_0.jar",
(unsigned char  *)   "TableLayout.jar",
(unsigned char  *)   "jfreereport-0_8_4_10-all_new.jar",
(unsigned char  *)   "myactivation_new.jar", 
(unsigned char  *)   "myzipmail_new.jar",  
(unsigned char  *)   "mysql-connector-java-3_0_8-stable-bin.jar",
(unsigned char  *)   "ojdbc14.jar",
(unsigned char  *)   "patbinfree153.jar",
(unsigned char  *)   "postgresql-8-2.jar", 
#ifdef OF_UAM_SUPPORT
  "erruam.htm",
  "wifilognew.htm",
  "wifilog.htm",  
  "uamlogin.igw",
  "wifilog.igw",
  "wifilout.htm",
  "wifilout.htm",
  "wifidefa.htm",
  "wifiidtm.htm",
  "wifitmot.htm",
  "wifiintro.htm",
  "wififorg.htm",
  "wifiusr.htm",
  "wifilink.htm",
  "company.gif",  
#endif
#ifdef IGW_MERCED_SUPPORT
  "ilogo.gif",
  "ibgtop.gif",
  "ispacer.gif",
  "style.css",
#endif /* IGW_MERCED_SUPPORT */
 (unsigned char  *)  ""
};

/**
  *  The folloging part of the code should be added to the system direcotry, which 
  *  helps other products like SSLVPN to go and change their login page dependencies.
  */
/*TR_PRIVATE*/ unsigned char  *LoginHtmlPages[] =
{
(unsigned char  *)  "login.xsl",
#ifdef OF_UCM_SUPPORT
(unsigned char  *)  "ucm",
#endif /* OF_UCM_SUPPORT */
#ifdef OF_CMS_SUPPORT
(unsigned char  *)  "cms",
#endif /* OF_CMS_SUPPORT */
(unsigned char  *)  "common.css",
(unsigned char  *)  "generic_page.css",
(unsigned char  *)  "shim.gif",
(unsigned char  *)  "header.xml",
(unsigned char  *)  "header.xsl",
(unsigned char  *)  "logo_bg.gif",
(unsigned char  *)  "logo.jpg",
(unsigned char  *)  "logo_new2.jpg",
(unsigned char  *)  "header.css",
(unsigned char  *)  "logviewer.jar",
(unsigned char  *)  "lv.jnlp",
(unsigned char  *)  "charsets.jar",
(unsigned char  *)  "cryptix32.jar",
(unsigned char  *)  "jbcl.jar",
(unsigned char  *)  "jcommon-0_9_5.jar",
(unsigned char  *)  "jfreechart-0_9_20.jar",
(unsigned char  *)  "jcommon-1_0_12.jar",
(unsigned char  *)  "jfreechart-1_0_8.jar",
(unsigned char  *)  "commons-collections-3_1.jar",
(unsigned char  *)  "commons-logging-1_0_4.jar",
(unsigned char  *)  "itext-2_0_4.jar",
(unsigned char  *)  "jta-spec1_0_1.jar",
(unsigned char  *)  "quartz-all-1_6_0.jar",
(unsigned char  *)  "TableLayout.jar",
(unsigned char  *)  "jfreereport-0_8_4_10-all_new.jar",
(unsigned char  *)   "myactivation_new.jar", 
(unsigned char  *)   "myzipmail_new.jar",  
(unsigned char  *)  "mysql-connector-java-3_0_8-stable-bin.jar",
(unsigned char  *)  "ojdbc14.jar",
(unsigned char  *)  "patbinfree153.jar",
(unsigned char  *)  "postgresql-8-2.jar", 
(unsigned char  *)  "logoicon.gif",
(unsigned char  *)  ""
};

/*TR_PRIVATE*/ unsigned char  *UrlFilesList[] =
{
(unsigned char  *)     "/login.htm",
#ifdef OF_UCM_SUPPORT
(unsigned char  *)     "/ucm",
#endif /* OF_UCM_SUPPORT */
#ifdef OF_CMS_SUPPORT
(unsigned char  *)     "/cms",
#endif /* OF_CMS_SUPPORT */
#ifdef OF_UAM_SUPPORT
    "/wifilog.htm",
    "/wifilognew.htm",
    "/erruam.htm",
#endif
   
   (unsigned char  *)  "logviewer.jar",
   (unsigned char  *)  "lv.jnlp",
   (unsigned char  *)  "charsets.jar",
   (unsigned char  *)  "cryptix32.jar",
   (unsigned char  *)  "jbcl.jar",
   (unsigned char  *)  "jcommon-0_9_5.jar",
   (unsigned char  *)  "jfreechart-0_9_20.jar",
   (unsigned char  *)  "jcommon-1_0_12.jar",
   (unsigned char  *)  "jfreechart-1_0_8.jar",
   (unsigned char  *)  "commons-collections-3_1.jar",
   (unsigned char  *)  "commons-logging-1_0_4.jar",
   (unsigned char  *)  "itext-2_0_4.jar",
   (unsigned char  *)  "jta-spec1_0_1.jar",
   (unsigned char  *)  "quartz-all-1_6_0.jar",
   (unsigned char  *)  "TableLayout.jar",
   (unsigned char  *)  "jfreereport-0_8_4_10-all_new.jar",
   (unsigned char  *)  "myactivation_new.jar",
   (unsigned char  *)  "jfreereport-0_8_4_10-all_new.jar",
   (unsigned char  *)  "myactivation_new.jar",
   (unsigned char  *)  "jfreereport-0_8_4_10-all_new.jar",
   (unsigned char  *)  "myactivation_new.jar",
   (unsigned char  *)  "myzipmail_new.jar",  
   (unsigned char  *)  "mysql-connector-java-3_0_8-stable-bin.jar",
   (unsigned char  *)  "ojdbc14.jar",
    (unsigned char  *) "patbinfree153.jar",
   (unsigned char  *)  "postgresql-8-2.jar", 
   (unsigned char  *)  "/failure_en.xsl",    
   (unsigned char  *)  ""
};

/*TR_PRIVATE*/ unsigned char  *SSLTemplateList[] =
{
 (unsigned char  *)  "ssltmplt.xsl",
 (unsigned char  *)  "template.xsl",
 (unsigned char  *)  "frm_meta.xsl",
 (unsigned char  *)  "newlogo.gif",
   (unsigned char  *)  ""
};

/********************************************************
 * * * *  F u n c t i o n    P r o t o t y p e s  * * * *
 ********************************************************/
int32_t HttpGetDefClearListenPort(uint16_t *pusClearPort)
{
  if(pusClearPort==NULL)
    return OF_FAILURE;
  
  *pusClearPort=HTTPD_DEF_PORT;

  return OF_SUCCESS;
}

#ifdef OF_HTTPS_SUPPORT
int32_t HttpGetDefSecureListenPort(uint16_t *pusSecurePort)
{
  if(pusSecurePort==NULL)
    return OF_FAILURE;
  
  *pusSecurePort=HTTPS_DEF_PORT;

  return OF_SUCCESS;
}
#endif /*OF_HTTPS_SUPPORT*/


/**************************************************************************
 * Function Name : Httpd_GetConfig                                        *
 * Description   : This function is used to intitialize the http server   *
 *                 configuration.                                         *
 * Input         : pConf (O) - pointer to the http configuration structure*
 * Output        : None.                                                  *
 * Return value  : None.                                                  *
 **************************************************************************/

void Httpd_GetConfig( HttpdConfig_t  *pConf )
{
  /**
   * Set the http server configurtion parameters to the default
   * configuration values.
   */
  pConf->ulPort         = DefaultPortConfig[0].usPort;
  pConf->ulPriority     = HTTPD_DEF_PRIO;
  pConf->ulMaxSessions  = HTTPD_DEF_MAX_SESSIONS;
  pConf->ulMaxPendReqs  = HTTPD_DEF_MAX_PENDREQS;
  pConf->pSrvName       = (unsigned char  *) HTTPD_SERVER_NAME;
  pConf->pSrvVer        = (unsigned char  *) HTTPD_SERVER_VER;
  // pConf->pSUName        = HTTPD_ADMIN_NAME; //for UCM
  pConf->pSUName        = (unsigned char  *) "root"; //for UCM need to get from PAM
  pConf->ulHtmlFsOffset = HTMLFS_OFFSET;
  pConf->ulHtmlFsSize   = HTMLFS_SIZE;
  pConf->uiMaxDataBuffer = HTTP_MAX_DATA_BUFFER;
  pConf->uiMaxTxBuffer   = HTTP_MAX_TRANSMIT_BUFFER;
} /* Httpd_GetConfig() */
/**************************************************************************
 * Description   : This function is used to get cookiename configured *
 *                 for the http server.                                   *
 * Input         : None.                                                  *
 * Output        : cooke name.                                            *
 * Return value  : Configured cookiename.                                 *
 **************************************************************************/

unsigned char *Http_GetCookieName(HttpGlobals_t  *pHttpGlobals)
{
  
   return pHttpGlobals->pHttpGlobalConfig->ucCookiename;
} /* Httpd_GetCookieName() */
/**************************************************************************
* Description   : This function is used to get max fds *
*                 for the http server.                                   *
* Input         : None.                                                  *
* Output        : max fds.                                            *
* Return value  : maximum fds .                                 *
**************************************************************************/

int32_t Httpd_GetMaxFds( HttpGlobals_t  *pHttpGlobals )
{
	  
	   return  pHttpGlobals->pHttpGlobalConfig->iMaxfds	;
} /* Httpd_GetMaxFds() */
/**************************************************************************
* Description   : This function is used to getrequest table size *
*             for the http server.                                   *
* Input         : None.                                                  *
* Output        : request table size.
*Return value  :request table size . 
***************************************************************************/

int32_t Httpd_GetMaxReqTableSize( HttpGlobals_t  *pHttpGlobals )
{
	
	return pHttpGlobals->pHttpGlobalConfig->iReqtablesize	;
} /* Httpd_GetReqTableSize() */

/**************************************************************************
 *  * Description   : This function is used to get cookiemaxsize *
 *                 for the http server.                                   *
 * Input         : None.                                                  *
* Output        : max cookie  size.
*return value  :max cookie size. 
***************************************************************************/

uint32_t Httpd_GetMaxCookieSize( HttpGlobals_t  *pHttpGlobals )
{
	  
	  return pHttpGlobals->pHttpGlobalConfig->ulCookieMaxSize;
} /* Httpd_GetGetMaxCookieSize() */

/**************************************************************************
* Description   : This function is used to get max timer lists *
*                 for the http server.                                   *
* Input         : None.                                                  *
* Output        : max timer  lists.                                            *
* Return value  :max timer lists.                                 *
**************************************************************************/

int32_t Httpd_GetMaxTimerLists( HttpGlobals_t  *pHttpGlobals )
{
    
    return pHttpGlobals->pHttpGlobalConfig->iMaxTimerLists;
} /* Httpd_GetMaxTimerLists() */


/**************************************************************************
 * Function Name : Httpd_GetLoginFile                                     *
 * Description   : This function is used to get the login page configured *
 *                 for the http server.                                   *
 * Input         : None.                                                  *
 * Output        : login page.                                            *
 * Return value  : Configured login page.                                 *
 **************************************************************************/

unsigned char *Httpd_GetLoginFile( HttpGlobals_t  *pHttpGlobals )
{
	 
	 return pHttpGlobals->pHttpGlobalConfig->ucLoginFile;
} /* Httpd_GetLoginFile() */    
#ifdef OF_XML_SUPPORT
/**************************************************************************
 * Function Name : Httpd_GetXmlLoginFile   					                                *
 * Description   : This function is used to get the login page configured				    *
 *                      for the http server.                                                                              *
 * Input         : None.                                                                                                    *
 * Output        : login page.                                                                                           *
 * Return value  : Configured login page.                                                                        *
 **************************************************************************/

unsigned char *Httpd_GetXmlLoginFile( HttpGlobals_t  *pHttpGlobals )
{
	    
	    return pHttpGlobals->pHttpGlobalConfig->ucXMLLogInFile;
} /* Httpd_GetXmlLoginFile() */             
#endif /*OF_XML_SUPPORT*/

/**************************************************************************
 * Function Name : Httpd_GetXSLLogOutFile   					                                  *
 * Description   : This function is used to get the logout page configured				      *
 *                      for the http server.                                                                                *
 * Input         : None.                                                                                                      *
 * Output        : logout page.                                                                                              *
 * Return value  : Configured logout page.                                                                          *
 **************************************************************************/

unsigned char *Httpd_GetXSLLogOutFile( HttpGlobals_t  *pHttpGlobals )
{
	    
	    return pHttpGlobals->pHttpGlobalConfig->ucXSLLogOutFile;	    
} /* Httpd_GetXSLLogOutFile() */      

/**************************************************************************
 * Function Name : Httpd_GetLogoutFile                                     *
 * Description   : This function is used to get the logout page configured *
 *                 for the http server.                                   *
 * Input         : None.                                                  *
 * Output        : logout page.                                            *
 * Return value  : Configured logout page.                                 *
 **************************************************************************/

unsigned char *Httpd_GetLogoutFile( HttpGlobals_t  *pHttpGlobals )
{
    
    return pHttpGlobals->pHttpGlobalConfig->ucLogoutFile;
} /* Httpd_GetLogoutFile() */
/**************************************************************************
 * Function Name : Httpd_GetErrorFile                                     *
 * Description   : This function is used to get the error page configured *
 *                 for the http server.                                   *
 * Input         : None.                                                  *
 * Output        : error page.                                            *
 * Return value  : Configured error page.                                 *
 **************************************************************************/

unsigned char *Httpd_GetErrorFile( HttpGlobals_t  *pHttpGlobals )
{
	   
	    return pHttpGlobals->pHttpGlobalConfig->ucErrorFile;
} /* Httpd_GetErrorFile() */
/**************************************************************************
 * Function Name : Httpd_GetUlogoutFile                                     *
 * Description   : This function is used to get the ulogout page configured *
 *                 for the http server.                                   *
 * Input         : None.                                                  *
 * Output        : ulogout page.                                            *
 * Return value  : Configured ulogout page.                                 *
 **************************************************************************/

unsigned char *Httpd_GetUlogoutFile( HttpGlobals_t  *pHttpGlobals )
{
	    
	    return pHttpGlobals->pHttpGlobalConfig->ucUlogoutFile;
} /* Httpd_GetUlogoutFile() */
/**************************************************************************
 * Function Name : Httpd_GetLogoFile                                     *
 * Description   : This function is used to get the Logo page configured *
 *                 for the http server.                                   *
 * Input         : None.                                                  *
 * Output        : Logo page.                                            *
 * Return value  : Configured Logo page.                                 *
 **************************************************************************/

unsigned char *Httpd_GetLogoFile( HttpGlobals_t  *pHttpGlobals  )
{
	 
	 return pHttpGlobals->pHttpGlobalConfig->ucLogoFile;
} /* Httpd_GetLogoFile() */


/**************************************************************************
 * Function Name : Httpd_GetIndexFile                                     *
 * Description   : This function is used to get the index page configured *
 *                 for the http server.                                   *
 * Input         : None.                                                  *
 * Output        : Index page.                                            *
 * Return value  : Configured index page.                                 *
 **************************************************************************/

unsigned char *Httpd_GetIndexFile( HttpGlobals_t  *pHttpGlobals  )
{
  
  return pHttpGlobals->pHttpGlobalConfig->ucIndexFile;
} /* Httpd_GetIndexFile() */

/**************************************************************************
 * Function Name : Httpd_GetDocPath                                       *
 * Description   : This function is used to get the document path         *
 *                 configured for the http server.                        *
 * Input         : None.                                                  *
 * Output        : Document path.                                         *
 * Return value  : Configured document path.                              *
 **************************************************************************/

unsigned char *Httpd_GetDocPath( HttpGlobals_t  *pHttpGlobals  )
{
  
  return pHttpGlobals->pHttpGlobalConfig->ucDocPath;
} /* Httpd_GetDocPath() */

/**************************************************************************
 * Function Name : Httpd_GetPermission                                    *
 * Description   : This function is used to get the privilege of the html *
 *                 file passed.                                           *
 * Input         : None.                                                  *
 * Output        : Privilege of the given html file.                      *
 * Return value  : An integer telling whether the given file has admin    *
 *                 privilege or user privilege.                           *
 **************************************************************************/

int32_t Httpd_GetPermission( char  *pFileName,
                             int32_t  *pPermission )
{
  int32_t  ii = 0;
  int32_t  iSplHtmlPrefix = 0;
  HttpGlobals_t *pHttpg=NULL;

  pHttpg=pHTTPG;
  if(!pHttpg)
  {
      Httpd_Error("pHttpg is NULL\r\n");
      return OF_FAILURE;
  }
  /**
   * Verify the given html file in the user html pages array.
   */
  while (of_strcmp((char *) pHttpg->pHttpGlobalConfig->ucUserHtmlPages[ii], ""))
  {
    /**
     * Check if the given html matches with the current file at the index.
     */
    if (of_strcmp((char *) pHttpg->pHttpGlobalConfig->ucUserHtmlPages[ii], pFileName) == 0)
    {
	      /**
	        * Set the permission to user privilege.
	        */
	      *pPermission = HTTPD_PRIV_USER;
	       return OF_SUCCESS;
    }      
    ii++;
  }   

    iSplHtmlPrefix=of_strlen(HTTP_HTML_PREFIX_VAL);
   if(!of_strncmp(pFileName,HTTP_HTML_PREFIX_VAL,iSplHtmlPrefix))
   {
	      	  /**
	          * Set the permission to user privilege.
	          */
	          *pPermission = HTTPD_PRIV_USER;
	           return OF_SUCCESS;
    }
        
  /**
   * Set the permission to admin privilege.
   */
  *pPermission = HTTPD_PRIV_ADMIN;

  return OF_SUCCESS;
} /* Httpd_GetPermission() */

/**************************************************************************
 * Function Name : Httpd_GetMaxCookies                                    *
 * Description   : This function is used to get the maximum number of     *
 *                 application cookies configured.                        *
 * Input         : None.                                                  *
 * Output        : Maximum application cookies configured.                *
 * Return value  : Integer value representing the maximum application     *
 *                 cookies configured.                                    *
 **************************************************************************/

int32_t Httpd_GetMaxCookies( HttpGlobals_t  *pHttpGlobals  )
{
  return    pHttpGlobals->pHttpGlobalConfig->ulMaxCookies;
} /* Httpd_GetMaxCookies() */

/**************************************************************************
 * Function Name : Httpd_GetMaxRegModules                                *
 * Description   : This function is used to get the maximum number of     *
 *                 modules to be registered.                        *
 * Input         : None.                                                  *
 * Output        : Maximum number of modules to be registered.                *
 * Return value  : Integer value representing the maximum modules to be registered.        *                           *
 **************************************************************************/

int32_t Httpd_GetRegModules(HttpGlobals_t  *pHttpGlobals ,uint32_t *ulMaxModules)
{
   if(!ulMaxModules)
   	return OF_FAILURE;
     *ulMaxModules = pHttpGlobals->pHttpGlobalConfig->ulRegMaxModules;
  return OF_SUCCESS;
} /* Httpd_GetRegModules() */


/**************************************************************************
 * Function Name : HttpVerifyConnReq                                      *
 * Description   : This function is used to verify whether the connection *
 *                 request should be allowed or not. For the ordinary     *
 *                 http server build, there are no restrictions and hence *
 *                 it allows, otherwise, verifies whether the connection  *
 *                 request has come from port number 80 or 443. For port  *
 *                 80, only internal network is allowed to browse and     *
 *                 with this certificates can be uploaded. For the port   *
 *                 443, there are no restrictions.                        *
 * Input         : peerIPAddress   (I) - peer IP address.                 *
 *                 usServerPort    (I) - server port number.              *
 * Output        : Status of the connection request.                      *
 * Return value  : OF_SUCCESS, if the valid request, otherwise OF_FAILURE.  *
 **************************************************************************/

int32_t HttpVerifyConnReq( ipaddr  peerIPAddress,
                           uint16_t  usServerPort )
{
    return OF_SUCCESS; //HARISH
#ifndef OF_UCM_SUPPORT
#if defined (OF_FW_SUPPORT) && defined (HTTPS_ENABLE)
  uint32_t        ulDirection;
  HttpPortInfo_t  PortInfo = {};
  IGWSubscriberInfo_t	SNetInfo={};
  #ifdef OF_IPV6_SUPPORT
  IGWIPv6SubscriberInfo_t	SNetIPv6Info={};  
 #endif  /*OF_IPV6_SUPPORT*/
  int32_t   iRetValue;

  PortInfo.usPort = usServerPort;
  if(HttpGetPortInfo(&PortInfo) != OF_SUCCESS)
  {
    HttpDebug(("Port number not configured\n\r"));
    return OF_FAILURE;
  }

  /**
   * Verify the server port number.
   */
  if (PortInfo.eServerType == hServerNormal)
  {
       #ifdef OF_IPV6_SUPPORT
   	if(peerIPAddress.ulIpType  == IGW_INET_IPV6)
       {     
            if(IGWIsIPv4MappedV6Addr(&peerIPAddress.IP.IPv6Addr))
            {
                T_memset(&SNetInfo, 0, sizeof(IGWSubscriberInfo_t));
                SNetInfo.input.ulSrcIpAddr= ntohl(peerIPAddress.IPv6Addr32[3]);   
                SNetInfo.input.ulDirType=IGW_SOURCE_SUBSCRIBER_INFO;                   
                iRetValue=IGWGetSubscriberInfoOnIP(&SNetInfo);
                if(iRetValue!=OF_SUCCESS)
                {
                   HttpDebug("subscriber info not found on IP\r\n");
                   return OF_FAILURE;
                }

                ulDirection  = SNetInfo.output.ulNetType;                
                if (ulDirection != IGW_EXT)
                {
                   return OF_SUCCESS;
                }
                else
                {
                   return OF_FAILURE;
                }
            }
          /**
	    * Allow http connections(port 80) only from internal network.
	    */
	    T_memset(&SNetIPv6Info,0,sizeof(IGWIPv6SubscriberInfo_t));
	    SNetIPv6Info.input.ulDirType=IGW_SOURCE_SUBSCRIBER_INFO;
	    of_memcpy(&(SNetIPv6Info.input.ulSrcIpAddr), &peerIPAddress.IPv6Addr8, sizeof(IGWIpAddr_t));	    
	    iRetValue = IGWIPv6GetSubscriberInfoOnIP(&SNetIPv6Info);    
	     if(iRetValue==OF_FAILURE)
	    {
	        HttpDebug(("Invalid Snet direction\n\r"));
	        return OF_FAILURE;
	    }
	    ulDirection=SNetIPv6Info.output.ulNetType;
  	}
      else	
      #endif  /*OF_IPV6_SUPPORT*/
      {
          /**
	     * Allow http connections(port 80) only from internal network.
	     */
	    T_memset(&SNetInfo,0,sizeof(IGWSubscriberInfo_t));
	    SNetInfo.input.ulDirType=IGW_SOURCE_SUBSCRIBER_INFO;
	    SNetInfo.input.ulSrcIpAddr=peerIPAddress.IPv4Addr;
	    iRetValue = IGWGetSubscriberInfoOnIP(&SNetInfo);    
	    if(iRetValue==OF_FAILURE)
	    {
	        HttpDebug(("Invalid Snet direction\n\r"));
	        return OF_FAILURE;
	    }
	    ulDirection=SNetInfo.output.ulNetType;
	}   
    if (ulDirection != IGW_EXT)
    {
      return OF_SUCCESS;
    }
    else
    {
      return OF_FAILURE;
    }
  }
  else if (PortInfo.eServerType == hServerSSL)
  {
   /**
    * By default, allow https connections from all networks.
    */
    return OF_SUCCESS;
  }
  return OF_FAILURE;
#else
  /**
   * For ordinary http server build allow connections from all networks.
   */
  return OF_SUCCESS;
#endif /* OF_FW_SUPPORT && HTTPS_ENABLE */
#endif  /*OF_UCM_SUPPORT*/
} /* HttpVerifyConnReq() */

#ifdef OF_HTTPS_SUPPORT

/**************************************************************************
 * Function Name : HttpsGetFileNames                                      *
 * Description   : This function is used to get the certificate file and  *
 *                 backup certificate file names.                         *
 * Input         : pHCertPath       (O) - reference to store the          *
 *                                        certificate file name.          *
 *                 pHCertBackupPath (O) - reference to store the          *
 *                                        backup certificate file name.   *
 * Output        : File names of the certificate and backup certificates. *
 * Return value  : File names of the certificate and backup certificates. *
 **************************************************************************/

void HttpsGetFileNames( HttpGlobals_t *pHttpGlobals ,unsigned char  **pHCertPath,
                          unsigned char  **pHCertBackupPath )
{
  *pHCertPath       =  pHttpGlobals->pHttpGlobalConfig->ucHCertPath;
  *pHCertBackupPath = pHttpGlobals->pHttpGlobalConfig->ucHCertBackupPath;
} /* HttpsGetFileNames() */

/**************************************************************************
 * Function Name : Https_GetDefPort                                       *
 * Description   : This function is used to get the default port number of*
 *                 http server with security.                             *
 * Input         : None.                                                  *
 * Output        : Https port number.                                     *
 * Return value  : Integer representing the https port number.            *
 **************************************************************************/

int32_t Https_GetDefPort( HttpGlobals_t  *pHttpGlobals  )
{
  return    pHttpGlobals->pHttpGlobalConfig->iHttpsDefaultPort;
} /* Https_GetDefPort() */

#endif /* OF_HTTPS_SUPPORT */

/**************************************************************************
 * Function Name : Httpd_HandleSystemCookies                              *
 * Description   : This function is used to handle the system cookies.    *
 * Input         : pHttpRequest (I) - pointer to the http request         *
 *                                    structure.                          *
 * Input         : ulType       (I) - type of the request, whether the    *
 *                                    login request or logout. For login  *
 *                                    type the value is 1 and for logout  *
 *                                    its 0.                              *
 * Output        : None.                                                  *
 * Return value  : OF_SUCCESS, if successful, otherwise OF_FAILURE.         *
 **************************************************************************/

int32_t Httpd_HandleSystemCookies( HttpRequest  *pHttpRequest,
                                   uint32_t     ulType )
{
  /**
   * Set the Wizard == no.
   */
  if (Httpd_SetCookieInfo(pHttpRequest, (unsigned char *) "wizard",(unsigned char *)  "no",
                          NULL, 10000) != OF_SUCCESS)
  {
    Httpd_Debug("\r\nSetting  of wizard cookie in login.igw failed");
  }

  /**
   * For logout request, send the wizard cookie.
   */
  if (ulType == 0)
  {
    if (Httpd_SendCookieInfo(pHttpRequest) != OF_SUCCESS)
    {
      Httpd_Debug("\r\nSending  of wizard cookie in logout.igw failed");
    }
  }

  return OF_SUCCESS;
} /* Httpd_HandleSystemCookies() */


/***************************************************************************
 * Function Name  : HttpGetDefaultPorts
 * Description    : 
 * Input          : 
 * Output         : None
 * Return value   : 
 ***************************************************************************/
int32_t  HttpGetDefaultPorts(HttpGlobals_t  *pHttpGlobals ,HttpPortInfo_t **pPortInfo)
{
  if(pPortInfo == NULL)
  {
    return OF_FAILURE;
  }
  *pPortInfo = pHttpGlobals->pHttpGlobalConfig->DefaultPortConfig;
  return OF_SUCCESS;
}

/***************************************************************************
 * Function Name  : Httpd_isPartOfLoginPage
 * Description    : 
 * Input          : 
 * Output         :  success or failure
 * Return value   : 
 ***************************************************************************/
int32_t Httpd_IsPartOfLoginPage(HttpGlobals_t *pHttpGlobals,char  *pFileName)
{
  int32_t  ii = 0;

  if(!pHttpGlobals)
  {
      Httpd_Error("pHttpGlobals is NULL\r\n");
      return OF_FAILURE;
  }
  /**
   * Verify the given html file in the user html pages array.
   */
  while (of_strcmp((char *) pHttpGlobals->pHttpGlobalConfig->ucLoginHtmlPages[ii], ""))
  {
    char *pTmp=NULL;
    /**
     * Check if the given html matches with the current file at the index.
     */
    pTmp = of_strstr(pFileName,(char *) pHttpGlobals->pHttpGlobalConfig->ucLoginHtmlPages[ii]);
    if (pTmp)
    {
      return OF_SUCCESS;
    }
    ii++;
  }
  return OF_FAILURE;
} /* Httpd_isPartOfLoginPage() */ 

  /************************************************************************** 
    * Function Name : Httpd_GetLogInOutFile                                                                         * 
    * Description   : This function is used to get the loginout page configured                                    * 
    *                      for the http server.                                                                                * 
    * Input         : None.                                                                                                      * 
    * Output        : loginout page.                                                                                              * 
    * Return value  : Configured loginout page.                                                                          * 
    **************************************************************************/ 
    
   unsigned char *Httpd_GetLogInOutFile( HttpGlobals_t  *pHttpGlobals  ) 
   { 
         
          return pHttpGlobals->pHttpGlobalConfig->ucLogInOutFile; 
   } /* Httpd_GeLogInOutFile() */ 
  
   /**************************************************************************
 * Function Name : Httpd_GetMinSecs                                    *
 * Description   : This function is used to get the minimum number of     *
 *                 seconds that configured.                        *
 * Input         : None.                                                  *
 * Output        : Minimum seconds that configured.                *
 * Return value  : Integer value representing the minimum       *
 *                 seconds that  configured.                                    *
 **************************************************************************/

int32_t Httpd_GetMinSecs( HttpGlobals_t  *pHttpGlobals  )
{
    return pHttpGlobals->pHttpGlobalConfig->iHttpdMinSecs;
} /* Httpd_GetMinSecs() */
   
 /**************************************************************************
 * Function Name : Httpd_GetMaxSecs                                    *
 * Description   : This function is used to get the maximum number of     *
 *                 seconds value that configured.                        *
 * Input         : None.                                                  *
 * Output        : maximum seconds that configured.                *
 * Return value  : Integer value representing the maximum     *
 *                 seconds value.                                    *
 **************************************************************************/

int32_t Httpd_GetMaxSecs( HttpGlobals_t  *pHttpGlobals  )
{
    return pHttpGlobals->pHttpGlobalConfig->iHttpdMaxSecs;
} /* Httpd_GetMaxSecs() */
 
 /**************************************************************************
 * Function Name : Httpd_GetMaxCookieBufferLen                                    *
 * Description   : This function is used to get the maximum buffer length  that     *
 *                 application cookies configured.                        *
 * Input         : None.                                                  *
 * Output        : Maximum application cookie buffer length.                *
 * Return value  : Integer value representing the maximum application     *
 *                 cookies bufferlength that  configured.                                    *
 **************************************************************************/

int32_t Httpd_GetMaxCookieBufferLen( HttpGlobals_t  *pHttpGlobals  )
{
  return pHttpGlobals->pHttpGlobalConfig->ulHttpdMaxCookieBufferLen;
} /* Httpd_GetMaxCookieBufferLen() */
 
 /**************************************************************************
 * Function Name :Httpd_GetMaxCookieValueLen                                    *
 * Description   : This function is used to get the maximum number of cookie value length    *
 *                 .                        *
 * Input         : None.                                                  *
 * Output        : Maximum  cookies value length that  configured.                *
 * Return value  : Integer value representing the maximum application     *
 *                 cookies value length that  configured.                                    *
 **************************************************************************/

int32_t Httpd_GetMaxCookieValueLen( HttpGlobals_t  *pHttpGlobals )
{
  return pHttpGlobals->pHttpGlobalConfig->ulHttpdMaxCookieValueLen;
} /* Httpd_GetMaxCookieValueLen() */
 
/***************************************************************************
 * Function Name  : HttpdCheckUrlFileAccess
 * Description    : 
 * Input          : FileName
 * Output         : success or failure
 * Return value   : OF_SUCCESS on success (or) OF_FAILURE on failure
 ***************************************************************************/
int32_t HttpdCheckUrlFileAccess(HttpGlobals_t *pHttpGlobals,char  *pFileName)
{
   int32_t  ii = 0;
   
   if(!pHttpGlobals)
   {
      Httpd_Error("pHttpGlobals is NULL\r\n");
      return OF_FAILURE;
   }
   /**
    * Verify the given html file ,gif file or xsl file  in the  array.
    */  
   while (of_strcmp((char *) pHttpGlobals->pHttpGlobalConfig->ucUrlFilesList[ii], ""))
   {
     /**
      * Check if the given html, gif file or xsl file  matches with the current file at the index.
      */
     if (of_strcasecmp((char *) pHttpGlobals->pHttpGlobalConfig->ucUrlFilesList[ii], pFileName) == 0)
     {
       	    return OF_SUCCESS;
     }
     ii++;
   }
   return OF_FAILURE;
} /*HttpdCheckUrlFileAccess() */
/***************************************************************************
 * Function Name  : HttpdTemplateFileAccess
 * Description    : 
 * Input          : FileName
 * Output         : success or failure
 * Return value   : OF_SUCCESS on success (or) OF_FAILURE on failure
 ***************************************************************************/
int32_t HttpdTemplateFileAccess(HttpGlobals_t *pHttpGlobals, char  *pFileName)
{
   int32_t  ii = 0;
   
   if(!pHttpGlobals)
   {
      Httpd_Error("pHttpGlobals is NULL\r\n");
      return OF_FAILURE;
   }
   /**
    * Verify the given Template file in the  array.
    */  
   while (of_strcmp((char *) pHttpGlobals->pHttpGlobalConfig->ucSSLTemplateList[ii], ""))
   {
       /**
        * Check if the given Template file matches with the current file at the index.
        */
       if (of_strcasecmp((char *) pHttpGlobals->pHttpGlobalConfig->ucSSLTemplateList[ii], pFileName) == 0)
       {
            return OF_SUCCESS;
       }
       ii++;
   }
   return OF_FAILURE;
} /*HttpdTemplateFileAccess() */
/**************************************************************************
* Description   : This function is used to get URL size *
*             for the http server.                                   *
* Input         : None.                                                  *
* Output        : URL size.
*Return value  :URL size . 
***************************************************************************/
uint32_t  Httpd_GetUrlSize( HttpGlobals_t  *pHttpGlobals)
{
     
     return  pHttpGlobals->pHttpGlobalConfig->ulUrlLength	;
}/*Httpd_GetUrlSize(void)*/

/**************************************************************************
* Description   : This function is used to get Appdata length *
*             for the http server.                                   *
* Input         : None.                                                  *
* Output        : URL size.
*Return value  :URL size . 
***************************************************************************/
uint32_t  Httpd_GetAppDataLength( HttpGlobals_t  *pHttpGlobals)
{
    
     return pHttpGlobals->pHttpGlobalConfig->ulAppDataLength;
}/*Httpd_GetAppDataLength(void)*/

 /**************************************************************************
 * Function Name : HttpdUpdateGlobalConfig                                        *
 * Description   : This function is used to update the http server Global   *
 *                 configuration.                                         *
 * Input         : pGlobalConf (O) - pointer to the http Global configuration structure*
 * Output        : None.                                                  *
 * Return value  : None.                                                  *
 **************************************************************************/
void HttpdUpdateGlobalConfig( HttpGlobals_t  *pHttpGlobals)
{
   if(!pHttpGlobals)
   {
      Httpd_Error("pHttpGlobals is NULL\r\n");
      return;
   }
  /**
   * Set the http server Global configurtion parameters to the default
   * configuration values.
   */
   pHttpGlobals->pHttpGlobalConfig->ulMaxCookies			= HTTPD_MAX_COOKIES;
   pHttpGlobals->pHttpGlobalConfig->iMaxfds				= HTTP_MAX_FDS;
   pHttpGlobals->pHttpGlobalConfig->iReqtablesize			= REQTABLE_SIZE; 
   pHttpGlobals->pHttpGlobalConfig->iMaxTimerLists			= MAX_TIMER_LISTS;
   pHttpGlobals->pHttpGlobalConfig->iHttpdMinSecs			= HTTP_MIN_SECS;
   pHttpGlobals->pHttpGlobalConfig->iHttpdMaxSecs			= HTTP_MAX_SECS;
   pHttpGlobals->pHttpGlobalConfig->iHttpMaxServers			= HTTP_MAX_SERVERS;
#ifdef OF_HTTPS_SUPPORT
   pHttpGlobals->pHttpGlobalConfig->iHttpsDefaultPort	 		= HTTPS_DEF_PORT;
#endif /*OF_HTTPS_SUPPORT*/
   pHttpGlobals->pHttpGlobalConfig->ulCookieMaxSize			= HTTPCOOKIE_MAX;
   pHttpGlobals->pHttpGlobalConfig->ulHttpdMaxCookieBufferLen= HTTP_COOKIE_BUFF_LEN;
   pHttpGlobals->pHttpGlobalConfig->ulHttpdMaxCookieValueLen = HTTP_COOKIE_VAL_LEN;
   pHttpGlobals->pHttpGlobalConfig->ulRegMaxModules		= HTTPD_MAX_REG_MODULES;
   pHttpGlobals->pHttpGlobalConfig->ulUrlLength			= HTTP_URL_LEN;
   pHttpGlobals->pHttpGlobalConfig->ulAppDataLength		= HTTP_APPDATA_LEN; 
   of_strcpy((char *) pHttpGlobals->pHttpGlobalConfig->ucLoginFile,HTTP_DEF_LOGIN_FILE);
   of_strcpy((char *) pHttpGlobals->pHttpGlobalConfig->ucLogoutFile,HTTP_DEF_LOGOUT_FILE);
   of_strcpy((char *) pHttpGlobals->pHttpGlobalConfig->ucErrorFile,HTTP_DEF_ERR_FILE);
   of_strcpy((char *) pHttpGlobals->pHttpGlobalConfig->ucUlogoutFile,HTTP_DEF_ULOGOUT_FILE);
   of_strcpy((char *) pHttpGlobals->pHttpGlobalConfig->ucLogoFile,HTTP_DEF_LOGO_FILE);
   of_strcpy((char *) pHttpGlobals->pHttpGlobalConfig->ucXMLLogInFile, HTTP_DEF_XML_LOGIN_FILE);
   of_strcpy((char *) pHttpGlobals->pHttpGlobalConfig->ucXSLLogOutFile, HTTP_DEF_XSL_LOGOUT_FILE); 
   of_strcpy((char *) pHttpGlobals->pHttpGlobalConfig->ucLogInOutFile,RANDOM_ACCESS_ERR_PAGE); 
   of_strcpy((char *) pHttpGlobals->pHttpGlobalConfig->ucCookiename,HTTP_DEF_COOKIE_NAME);
   of_strcpy((char *) pHttpGlobals->pHttpGlobalConfig->ucIndexFile,HTTPD_INDEX_FILE);
   of_strcpy((char *) pHttpGlobals->pHttpGlobalConfig->ucDocPath, HTTPD_DOC_PATH);

#ifdef OF_HTTPS_SUPPORT
   pHttpGlobals->pHttpGlobalConfig->ucHCertPath 			= (unsigned char *)HTTPS_CERT_PATH;
   pHttpGlobals->pHttpGlobalConfig->ucHCertBackupPath 		= (unsigned char *)HTTPS_CERT_BACKUP_PATH;   
#endif /*OF_HTTPS_SUPPORT*/
   pHttpGlobals->pHttpGlobalConfig->ucUserHtmlPages		= (unsigned char **)userHtmlPages;
   pHttpGlobals->pHttpGlobalConfig->ucLoginHtmlPages		= (unsigned char **)LoginHtmlPages;
   pHttpGlobals->pHttpGlobalConfig->ucUrlFilesList			= (unsigned char **)UrlFilesList;
   pHttpGlobals->pHttpGlobalConfig->ucSSLTemplateList		= (unsigned char **)SSLTemplateList; 
   pHttpGlobals->pHttpGlobalConfig->DefaultPortConfig=(HttpPortInfo_t *)of_calloc(1,HTTP_MAX_SERVERS*sizeof(HttpPortInfo_t));
   of_memcpy(pHttpGlobals->pHttpGlobalConfig->DefaultPortConfig,&DefaultPortConfig,HTTP_MAX_SERVERS*sizeof(HttpPortInfo_t));
   
   pHttpGlobals->pCbkTimers->HttpdKeepAliveTimeoutCbk   =     HttpClient_KeepAliveTimeOut;
   pHttpGlobals->pCbkTimers->HttpdReqStrTimeoutCbk        =    HttpClient_ReqStrTimeOut;
   pHttpGlobals->pCbkTimers->HttpdResponseTimeoutCbk    =    HttpClient_StartResponse;
   return;
} /* HttpdUpdateGlobalConfig() */
#endif /* OF_HTTPD_SUPPORT */

