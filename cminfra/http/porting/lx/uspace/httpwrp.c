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
/*  File        : httpwrp.c                                               */
/*                                                                        */
/*  Description : This file contains wrapper functions required by http   */
/*                server. These wrappers are for linux flavor.            */
/*                                                                        */
/*  Version      Date      Author      Change Description                 */
/*  -------    --------    ------    ----------------------               */
/*    1.0      00.00.00    kmdhar    Initial implementation               */
/*    1.1      05.18.04     dram     Modified during the code review.     */
/**************************************************************************/

#ifdef OF_HTTPD_SUPPORT

/***********************************************************
 * * * *  I n c l u d e    H e a d e r    f i l e s  * * * *
 ***********************************************************/

#include "cmincld.h"
#include "httpdinc.h"
#include "httpgif.h"
#include "httpwrp.h"
#include "htpdglb.h"
#include "httpdtmr.h"
#include "httpd.h"
#include "errno.h"
#include "lxoswrp.h"
#include <netinet/in.h>

/****************************************************
 * * * *  M a c r o    D e f i n i t i o n s  * * * *
 ****************************************************/

#define  CookieSemName                "COOKIE"
#define  HTTP_IP_ADDRESS              (CM_INET6_ADDRSTRLEN)
#define  HTTP_MSG_LENGTH              (120)
#define HTTP_LOCAL_BUFFER_LEN         (512)

/************************************************************
 * * * *  V a r i a b l e    D e c l a r a t i o n s  * * * *
 ************************************************************/

/****************************************************************
 * * * *  F u n c t i o n    I m p l e m e n t a t i o n  * * * *
 ****************************************************************/

/**************************************************************************
 * Function Name : HttpServerTask                                         *
 * Description   : This function is used to create a new http server and  *
 *                 starts it.                                             *
 * Input         : None                                                   *
 * Output        : None                                                   *
 * Return value  : OF_SUCCESS on success, otherwise OF_FAILURE.             *
 **************************************************************************/

int32_t HttpServerTask( void )
{
  /**
   * Start the http server.
   */
  if (Httpd_Start(pHTTPG) != OF_SUCCESS)
  {
    Httpd_Print("Unable to start http server\n");
    return OF_FAILURE;
  }
  return OF_SUCCESS;
} /* HttpServerTask() */

/**************************************************************************
 * Function Name : CreateHttpServer                                       *
 * Description   : This function is used to intitialize the http server   *
 *                 and creates http server task.                          *
 * Input         : None.                                                  *
 * Output        : None.                                                  *
 * Return value  : OF_SUCCESS on success, otherwise OF_FAILURE.             *
 **************************************************************************/

int32_t CreateHttpServer( void )
{
  int32_t  iResult;
  HttpGlobals_t  *pHttpg=NULL;
  /**
   * Initialize the http server resources.
   */
  if (Httpd_Init() != OF_SUCCESS)
  {
    Httpd_Error("Unable To Initialize HTTP Server\n");
    return OF_FAILURE;
  }

  pHttpg=pHTTPG;
  if(!pHttpg)
  {
    Httpd_Error("pHttpg is NULL\r\n");	
    return OF_FAILURE;
  }
  /**
   *
   */
  iResult = HttpServerTask();

  if (iResult != OF_SUCCESS)
  {
    Httpd_Print("Error in creating httpd task errno = %x\n", errno);
    return OF_FAILURE;
  }

  return OF_SUCCESS;
} /* CreateHttpServer() */

/**************************************************************************
 * Function Name : Httpd_GetTimeStr                                       *
 * Description   : This function is used to give the time in string format*
 * Input         : pTimeString (O) - pointer to string in which time in   *
 *                                   string format will be copied.        *
 * Output        : Time in string format.                                 *
 * Return value  : OF_SUCCESS on sucess.                                   *
 **************************************************************************/

int32_t Httpd_GetTimeStr( char  *pTimeString )
{
  time_t  tm;

  time(&tm);

  sprintf(pTimeString, "%ld", tm);

  return OF_SUCCESS;
} /* Httpd_GetTimeStr() */

/**************************************************************************
 * Function Name : Httpdget_ticks_per_sec                                       *
 * Description   : This function is used to get timer interrupt frequency (ticks per sec)*
 * Input         : pTicksPerSec (O) - pointer to integer in whichno of ticks for  Second will be copied.        *
 * Output        : no of ticks for  Second                            *
 * Return value  : Return value of get_ticks_per_sec                               *
 **************************************************************************/


int32_t Httpdget_ticks_per_sec(uint32_t *pTicksPerSec)
{
  int32_t iRetVal=0;
  iRetVal=get_ticks_per_sec(pTicksPerSec);
  return iRetVal;
}
/**************************************************************************
 * Function Name : Httpd_ntoa                                             *
 * Description   : This frunction is used to convert the network order IP *
 *                 address into the string format.                        *
 * Input         : IPAddress (I) - IP address in network order.           *
 *                 pIPAddress(O) - pointer to character buffer where the  *
 *                                 IP address will be stored.             *
 * Output        : Network ordered IP address in string format.           *
 * Return value  : String in string format.                               *
 **************************************************************************/

char *Httpd_ntoa( ipaddr   IPAddress,
                     char  *pIPAddress )
{
  struct in_addr  thisAddr;
  char         *pThisAddr;

  thisAddr.s_addr = IPAddress;

  pThisAddr = inet_ntoa(thisAddr);

  if (!pThisAddr)
  {
    return NULL;
  }

  of_strcpy(pIPAddress, pThisAddr);

  return pIPAddress;
} /* Httpd_ntoa() */

/**************************************************************************
 * Function Name : Httpd_aton                                             *
 * Description   : This function is used to convert an IP address from    *
 *                 string format to integer format.                       *
 * Input         : pIPAddress (I) - pointer to the character buffer with  *
 *                                  the IP address.                       *
 * Output        : IP address in interger format.                         *
 * Return value  : Changed IP address from string format to integer format*
 *                 on success, otherwise 0xffffffff.                      *
 **************************************************************************/

ipaddr Httpd_aton( char  *pIPAddress )
{
  struct in_addr  thisAddr;

  /**
   * Assert on the given IP address.
   */
  HTTPD_ASSERT(!pIPAddress);

  /**
   *
   */
  if (inet_aton(pIPAddress,&thisAddr) < 0)
  {
    return 0xffffffff;
  }
  else
  {
    /**
     * The network address is converted to host byte order since firewall
     * expects the same.
     */
    return (ntohl(thisAddr.s_addr));
  }
} /* Httpd_aton() */

/**************************************************************************
 * Function Name : Httpd_Cookie_SemCreate                                 *
 * Description   : This function is called once as part of http server    *
 *                 initialization.                                        *
 * Input         : None.                                                  *
 * Output        : None.                                                  *
 * Return value  : OF_SUCCESS.                                             *
 **************************************************************************/

int32_t Httpd_Cookie_SemCreate( void )
{
  return OF_SUCCESS;
} /* Httpd_Cookie_SemCreate() */

/**************************************************************************
 * Function Name : Httpd_Cookie_SemGet                                    *
 * Description   : This function is used to aquire the Cookie Semaphore.  *
 * Input         : None.                                                  *
 * Output        : None.                                                  *
 * Return value  : OF_SUCCESS.                                             *
 **************************************************************************/

int32_t Httpd_Cookie_SemGet( void )
{
  return OF_SUCCESS;
} /* Httpd_Cookie_SemGet() */

/**************************************************************************
 * Function Name : Httpd_Cookie_SemRelease                                *
 * Description   : This function is used to release the cookie semaphore. *
 * Input         : None.                                                  *
 * Output        : None.                                                  *
 * Return value  : OF_SUCCESS.                                             *
 **************************************************************************/

int32_t Httpd_Cookie_SemRelease( void )
{
return OF_SUCCESS;
} /* Httpd_Cookie_SemRelease() */

/**************************************************************************
 * Function Name : HttpNotifyMessage                                      *
 * Description   : This function is used to log a system log entry.       *
 * Input         : IPAddress (I) - IP address.                            *
 *                 pUser     (I) - pointer to the user name.              *
 *                 pMsg1     (I) - pointer to the first string of the     *
 *                                 message.                               *
 *                 pMsg2     (I) - pointer to the second string of the    *
 *                                 message.                               *
 * Output        : Message will be logged to the system log.              *
 * Return value  : None.                                                  *
 **************************************************************************/

void HttpNotifyMessage( ipaddr   IPAddress,
                          char  *pUser,
                          char  *pMsg1,
                          char  *pMsg2 )
{
  char  cMsgBuf[HTTP_MSG_LENGTH];
  char  cRemoteIp[HTTP_IP_ADDRESS];	
  
  /**
   * Get the remote ip address in string format.
   */
#ifndef OF_CM_SUPPORT
   #ifdef INTOTO_IPV6_SUPPORT
   if(IPAddress.ip_type_ul==IGW_INET_IPV6)
   {
	  if(HttpdNumToPresentation(&IPAddress,(unsigned char *)cRemoteIp)!=OF_SUCCESS)
	  {
            HttpDebug((" HttpdNumToPresentation is Failed\r\n"));
            return;
	  }	  
   }
   else
   #endif  /*INTOTO_IPV6_SUPPORT*/
#endif  /*OF_CM_SUPPORT*/
   {
         Httpd_ntoa(htonl(IPAddress), cRemoteIp); 
   }
   
   /**
    * Format the message to log.
    */
   snprintf(cMsgBuf, HTTP_MSG_LENGTH, " : User '%s' at %s, %s %s", pUser,
   cRemoteIp, pMsg1, pMsg2);

  /**
   * Log the formatted message.
   */
#ifdef MESGCLNT_SUPPORT
  IBsendSysLogMsg(NULL, 0, 0, cMsgBuf, 0, ISEC_UN_AUTH_ACCESS, LOGINFO,
                  HTTPD_SYSMSGID_USR_SEND_INFO_5);
#endif /* MESGCLNT_SUPPORT  */

} /* HttpNotifyMessage() */

#ifndef OF_CM_SUPPORT

/**************************************************************************
 * Function Name : HttpGetPktDirection                                    *
 * Description   : This is the wrapper function to the function,          *
 *                 IGWGetSubscriberInfoOnIP.                              *
 * Input         : pktIPAddress (I) - IP address to be used send packets. *
 *                 pIface       (I) - pointer to the interface.           *
 *                 pOpaque      (I) - pointer to the opaque.              *
 *                 uiType       (I) - Type of the packet.                 *
 * Output        : Status of the IGWGetSubscriberInfoOnIP function call.  *
 * Return value  : OF_SUCCESS on success, otherwise OF_FAILURE.             *
 **************************************************************************/

int32_t HttpGetPktDirection( struct cm_ip_addr     *pktIPAddress,
                              void    *pIface,
                              void    *pOpaque,
                              uint32_t  uiType )
{
  IGWSubscriberInfo_t	 SNetInfo={};
  int32_t iRetValue=0;
  #ifdef INTOTO_IPV6_SUPPORT
  unsigned char *pTempStr=NULL;
  unsigned char cIPv6SrcIPAddress[HTTP_LOCAL_IP_ADDRESS_LEN]={};  
  IGWIPv6SubscriberInfo_t	 SNetIpv6Info={};
  #endif  /*INTOTO_IPV6_SUPPORT*/

  if(!pktIPAddress)
  {
         HttpDebug("Invalid packet  IP address \r\n");
         return OF_FAILURE;
  } 
  #ifdef INTOTO_IPV6_SUPPORT
  if(pktIPAddress->ip_type_ul==IGW_INET_IPV6)
  {
         /*********TEMPORARY FIX START********BR#19646 VSG mode - Unable to access HTML Pages */        
         if(IGWIPv6NumToPresentation(&(pktIPAddress->IP.IPv6Addr),(unsigned char *)cIPv6SrcIPAddress,sizeof(cIPv6SrcIPAddress))!=OF_SUCCESS)
         {	 
                 HttpDebug(("IGWIPv6NumToPresentation Failed\r\n"));
                 return OF_FAILURE; 	
          }
 	   /*Checking for IPv4 mapped IPv6 Address e.g ::ffff:10.16.5.52*/
          if(of_strstr((unsigned char *)cIPv6SrcIPAddress,HTTP_IPV6_ADDR_PREFIX)!=NULL)	
          {
       	   pTempStr=strtok((unsigned char *)cIPv6SrcIPAddress,HTTP_IPV6_ADDR_PREFIX);
       	   if(pTempStr!=NULL)	
          	   {					
          	         of_strncpy(cIPv6SrcIPAddress,pTempStr,of_strlen(pTempStr)+1);
          	   }
		   else
		   {
                       HttpDebug(("pTempStr is NULL\r\n"));
                       return OF_FAILURE; 
		   }
       	   IGWAtoN((unsigned char *)cIPv6SrcIPAddress, &pktIPAddress->IPv4Addr);	
                 of_memset(&SNetInfo, 0, sizeof(IGWSubscriberInfo_t));
                 SNetInfo.input.ulDirType=uiType;                
                 if(SNetInfo.input.ulDirType==IGW_SOURCE_SUBSCRIBER_INFO)
                 { 
                      SNetInfo.input.ulSrcIpAddr=pktIPAddress->IPv4Addr;
                 }
                 else if (SNetInfo.input.ulDirType== IGW_DEST_SUBSCRIBER_INFO)
                 {
                      SNetInfo.input.ulDstIpAddr=pktIPAddress->IPv4Addr; 
                 }
                 else
                 {
                     HttpDebug("Invalid subscriber info \r\n");
                     return OF_FAILURE;
                 }
                 iRetValue=IGWGetSubscriberInfoOnIP(&SNetInfo);
                 if(iRetValue!=OF_SUCCESS)
                 {
                     HttpDebug("subscriber info not found on IP\r\n");
                     return OF_FAILURE;
                 }
                 if(pOpaque)
                 {
                    of_memcpy(pOpaque,
		                    (void *)&SNetInfo.output.ulSubscriberNetId,
			                sizeof(uint32_t));
                 }
                 return  (int32_t)SNetInfo.output.ulNetType;      	
          }           
	  /*********TEMPORARY FIX END*********/     
	  of_memset(&SNetIpv6Info, 0, sizeof(IGWIPv6SubscriberInfo_t));
  	  SNetIpv6Info.input.ulDirType=uiType;
	    
	  if(SNetIpv6Info.input.ulDirType==IGW_SOURCE_SUBSCRIBER_INFO)
	  { 
	          of_memcpy(&SNetIpv6Info.input.ulSrcIpAddr,pktIPAddress->IPv6Addr8,IGW_IPV6_ADDR_LEN);
	   }
	   else if (SNetIpv6Info.input.ulDirType== IGW_DEST_SUBSCRIBER_INFO)
	   {
	          of_memcpy(&SNetIpv6Info.input.ulDstIpAddr,pktIPAddress->IPv6Addr8,IGW_IPV6_ADDR_LEN);        	
	   }
	   else
	   {
	         HttpDebug("Invalid subsciber info \r\n");
	         return OF_FAILURE;
	   }	   
	   iRetValue=IGWIPv6GetSubscriberInfoOnIP(&SNetIpv6Info);
	   if(iRetValue!=OF_SUCCESS)
	    {
	            HttpDebug("susciber info not found on IP\r\n");
	            return OF_FAILURE;
	    }
        if(pOpaque)
        {
          of_memcpy(pOpaque,
		 	       (void *)&SNetIpv6Info.output.ulSubscriberNetId,
		  	       sizeof(uint32_t));
        }
	    return  (int32_t)SNetIpv6Info.output.ulNetType;    
  }
  else
#endif /*INTOTO_IPV6_SUPPORT*/
  {	  
	  of_memset(&SNetInfo, 0, sizeof(IGWSubscriberInfo_t));
	  SNetInfo.input.ulDirType=uiType;
		
	  if(SNetInfo.input.ulDirType==IGW_SOURCE_SUBSCRIBER_INFO)
	  { 
			  SNetInfo.input.ulSrcIpAddr=pktIPAddress->IPv4Addr;
	   }
	   else if (SNetInfo.input.ulDirType== IGW_DEST_SUBSCRIBER_INFO)
	   {
			  SNetInfo.input.ulDstIpAddr=pktIPAddress->IPv4Addr; 		
	   }
	   else
	   {
			 HttpDebug("Invalid subscriber info \r\n");
			 return OF_FAILURE;
	   }
	   
	   iRetValue=IGWGetSubscriberInfoOnIP(&SNetInfo);
	   if(iRetValue!=OF_SUCCESS)
          {
       			HttpDebug("subscriber info not found on IP\r\n");
       			return OF_FAILURE;
          }
       if(pOpaque)
       {
         of_memcpy(pOpaque,
		 	      (void *)&SNetInfo.output.ulSubscriberNetId,
		 	      sizeof(uint32_t));
       }
	   return	(int32_t)SNetInfo.output.ulNetType;	  
  }
} /* HttpGetPktDirection() */

#endif /*OF_CM_SUPPORT*/

/**************************************************************************
 * Function Name : HttpRedirectServer                                     *
 * Description   : This function redirects the http server on changed IP  *
 *                 address.                                               *
 * Input         : pHttpRequest (I) - http request structure.             *
 *                 LANIPAddr    (I) - IP address to be used to send the   *
 *                                    reply.                              *
 * Output        : None                                                   *
 * Return value  : OF_SUCCESS on success.                                  *
 *                 OF_FAILURE on failure.                                  *
 **************************************************************************/

int32_t HttpRedirectServer( HttpRequest  *pHttpRequest,
                            ipaddr       LANIPAddr )
{
  HttpClientReq_t  *pClientReq   = NULL;
  char          tmpBuf[HTTP_LOCAL_BUFFER_LEN];
  char          cMyAddr[HTTP_LOCAL_IP_ADDRESS_LEN];  
  int32_t          len;  
  HttpGlobals_t *pHttpg=NULL;

  /**
   * Validate the passed parameter(s).
   */
  if (!pHttpRequest)
  {
    return OF_FAILURE;
  }

  pHttpg=(HttpGlobals_t *)pHttpRequest->pGlobalData;
  if(!pHttpg)
  {
    Httpd_Error("pHttpg is NULL\r\n");
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

  cm_inet_ntoal(LANIPAddr, cMyAddr);

	  if (pHttpg->HttpServers[pClientReq->lServId].Type == hServerSSL)
	  {
		    len = sprintf(tmpBuf,
	    	            "<html><head><title>Redirector page</title><meta http-equiv=\"Refresh\" content=\"2; URL=https://%s:%d/start.htm\" target=\"_top\"></head><body>It will automatically redirected to the new IP address, otherwise click on the adjucent link <a href=\"https://%s:%d/start.htm\" target=\"_top\"><strong>Click</strong></a></body></html>",
	    	            cMyAddr, pClientReq->SrcPort, cMyAddr, pClientReq->SrcPort);
	  }
	  else
	  {
		    len = sprintf(tmpBuf,
	    	            "<html><head><title>Redirector page</title><meta http-equiv=\"Refresh\" content=\"2; URL=http://%s:%d/start.htm\" target=\"_top\"></head><body>It will automatically redirected to the new IP address, otherwise click on the adjucent link <a href=\"http://%s:%d/start.htm\" target=\"_top\"><strong>Click</strong></a></body></html>",
	    	            cMyAddr, pClientReq->SrcPort, cMyAddr, pClientReq->SrcPort);
	  }

  if(cm_socket_send(pClientReq->SockFd,tmpBuf,len,0) < 0)
  {
     Httpd_Error("cm_socket_send returned Failure\r\n");
  }
  cm_socket_close(pClientReq->SockFd);

  return OF_SUCCESS;
} /* HttpRedirectServer() */
/**************************************************************************
 * Function Name : HttpdTaskVarAdd                                        *
 * Description   : This function is called once as part of http server    *
 *                 initialization.                                        *
 * Input         : None.                                                  *
 * Output        : None.                                                  *
 * Return value  : OF_SUCCESS on success, otherwise OF_FAILURE.             *
 **************************************************************************/

int32_t HttpdTaskVarAdd(HttpGlobals_t  *pHttpGlobals)
{  
   return OF_FAILURE;
} /* HttpdTaskVarAdd() */
#endif /* OF_HTTPD_SUPPORT */
