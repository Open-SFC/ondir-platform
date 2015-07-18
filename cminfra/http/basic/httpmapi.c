
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
/*  File        : htpmapi.c                                               */
/*                                                                        */
/*  Description : This file contains the APIs that are used to configure  */
/*                (run time) multiple ports.                              */
/*                                                                        */
/*  Version      Date          Author      Change Description             */
/*  -------    --------        ------     ---------------------           */
/*   1.0      23-07-2004     K.RamaRao    Initial Implimentation          */
/**************************************************************************/

#ifdef OF_HTTPD_SUPPORT

#include "cmincld.h"
#include "httpwrp.h"
#include "httpgdef.h"
#include "httpgif.h"
#include "htpercod.h"

 int32_t HttpSetLoopBack(int32_t *iSFd);

extern uint32_t igwHTTPDLogConfigMsg(uint32_t uiMsgId_p,uint32_t uiMesFamily,int32_t iPort);

/***************************************************************************
 * Function Name  : HttpAddPort
 * Description    : This function is used to add a new port. 
 *                  
 * Input          : 
 * Output         : 
 * Return value   : 
 ***************************************************************************/
int32_t HttpAddPort (HttpPortInfo_t PortInfo)
{
  int32_t          SFd = -1;
  int32_t          iRetVal=0;
  HttpPortArgs_t   PortArgs = {};
  struct cm_poll_info    PollFds[2] = {};
  ipaddr           PeerIp;
  uint16_t         PeerPort = 0;
  uint32_t         uiLpBkRecvData[2];
  int32_t          iMaxConfigServersForType = 0;
  int32_t          iMaxServersForType = 0;
#ifndef OF_CM_SUPPORT
  iMaxConfigServersForType = HttpGetMaxConfiguredServers(&PortInfo.eServerType);
#endif /*OF_CM_SUPPORT*/
  iMaxServersForType = HttpGetMaxServers(PortInfo.eServerType);
#ifdef HTTPD_DEBUG
  HttpDebug("max servers = %d max config servers = %d\n\r",
  	        iMaxServersForType, iMaxConfigServersForType);
#endif /*HTTPD_DEBUG*/

  if(iMaxConfigServersForType >= iMaxServersForType)
  {
#ifdef HTTPD_DEBUG
    HttpDebug("Max Servers configurd for %s. max allowed is %d\n\r",
    	       SERVER_TYPE_2STR(PortInfo.eServerType),iMaxServersForType);
#endif /*HTTPD_DEBUG*/
    return HTTP_ERR_MAX_SERVERS_REACHED;
  }

  PortArgs.PortInfo.usPort = PortInfo.usPort;
#ifndef OF_CM_SUPPORT
  iRetVal = HttpGetPortInfo(&PortArgs.PortInfo);
  if(iRetVal == OF_SUCCESS)
  {
    HttpDebug(("PortAlready configured. Port = %d\n\r",PortInfo.usPort));
    return HTTP_ERR_PORT_IN_USE;
  }
#endif /*OF_CM_SUPPORT*/

  PortArgs.ucCmd = (unsigned char)HTTP_CMD_ADD_PORT;
  PortArgs.PortInfo = PortInfo;

  iRetVal = HttpSetLoopBack(&SFd);
  if(iRetVal != OF_SUCCESS)
  {
    HttpDebug(("Unable to set the loop back socket\n\r"));
    return iRetVal;
  }
  HttpDebug(("Loop back socket setup success\n\r"));

  PollFds[0].fd_i = SFd;
  PollFds[0].interested_events = CM_FDPOLL_READ;

  iRetVal = cm_socket_send_to(SFd,(char*)&PortArgs,sizeof(HttpPortArgs_t),
  	                        0, HTTP_LOOPBACK_ADDR, HTTP_LOOPBACK_PORT_SRV);
  if(iRetVal < 0)
  {
    HttpDebug(("Unable to send data\n\r"));
    cm_socket_close(SFd);
    return HTTP_ERR_LPBK_DATA_SEND_FAIL;
  }
  HttpDebug(("data sent on loop back to add port\n\r"));
  iRetVal=cm_fd_poll(1, PollFds, 1000); 
  if(iRetVal < 0)
  {
#ifdef HTTPD_DEBUG
    HttpDebug("FdPoll Returned Error :%d",iRetVal);
#endif /*HTTPD_DEBUG*/
    return HTTP_ERR_NOTHING_TO_MODIFY;
  }
  
  /**
   *  Http Server is  using  loop back address ((0x7F000001) 127.0.0.1),So we are  passing  IPv4 address 
   *  to the cm_socket_recv_from().  
   */    
   /*PeerIp.ip_type_ul=IGW_INET_IPV4;*/
  if(PollFds[0].returned_events & CM_FDPOLL_READ)
  {
    iRetVal = cm_socket_recv_from(SFd, (char*)uiLpBkRecvData,sizeof(uiLpBkRecvData),
  	                            0, &PeerIp, &PeerPort);
    if(iRetVal < 0)
    {
      HttpDebug(("Unable to recieve data on loop back socket\n\r"));
      cm_socket_close(SFd);
      return HTTP_ERR_LPBK_RECV_FAIL;
    }
  }
  else
  {
    HttpDebug(("No response on loop back socket\n\r"));
    cm_socket_close(SFd);
    return HTTP_ERR_LPBK_RECV_FAIL; 
  }

  cm_socket_close(SFd);
#ifdef MESGCLNT_SUPPORT
/*  IGWSprintf(Message, "Http Port number %d has been added successfully",
             PortInfo.usPort);
  IBsendSysLogMsg(NULL, 0, 0, Message, 0, ISEC_CONFIG_CHANGES, LOGINFO,
                  HTTPD_SYSMSGID_HTTP_PORT_ADD_9);*/
  igwHTTPDLogConfigMsg(HTTPD_SYSMSGID_HTTP_PORT_ADD_9,
                     IGW_LOG_FAMILY_CONFIG,PortInfo.usPort) ;
#endif
  return uiLpBkRecvData[0];
}

/***************************************************************************
 * Function Name  : HttpDeletePort
 * Description    : This function is used to delete the configured port. 
 *                  
 * Input          : 
 * Output         : None
 * Return value   : 
 ***************************************************************************/
int32_t HttpDeletePort (uint16_t usPortNum)
{
  int32_t          SFd = -1;
  int32_t          iRetVal=0;
  HttpPortArgs_t   PortArgs = {};
  struct cm_poll_info    PollFds[1] = {};
  ipaddr           PeerIp;
  uint16_t         PeerPort = 0;
  uint32_t         uiLpBkRecvData[2];

  PortArgs.ucCmd = (unsigned char)HTTP_CMD_DEL_PORT;
  PortArgs.PortInfo.usPort = usPortNum;
#ifndef OF_CM_SUPPORT
  iRetVal = HttpGetPortInfo(&PortArgs.PortInfo);
  if(iRetVal != OF_SUCCESS)
  {
    HttpDebug(("Port not found %d\n\r",usPortNum));
    return HTTP_ERR_PORT_NOT_FOUND;
  }
#endif /*OF_CM_SUPPORT*/

  iRetVal = HttpSetLoopBack(&SFd);
  if(iRetVal != OF_SUCCESS)
  {
    HttpDebug(("Unable to set the loop back socket\n\r"));
    return iRetVal;
  }

  PollFds[0].fd_i = SFd;
  PollFds[0].interested_events = CM_FDPOLL_READ;

  iRetVal = cm_socket_send_to(SFd,(char*)&PortArgs,sizeof(HttpPortArgs_t),
  	                        0, HTTP_LOOPBACK_ADDR, HTTP_LOOPBACK_PORT_SRV);
  if(iRetVal < 0)
  {
    HttpDebug(("Unable to send data\n\r"));
    cm_socket_close(SFd);
    return HTTP_ERR_LPBK_DATA_SEND_FAIL;
  }

  iRetVal=cm_fd_poll(1, PollFds, 1000);
   if(iRetVal < 0)
  {
#ifdef HTTPD_DEBUG
    HttpDebug("FdPoll Returned Error :%d",iRetVal);
#endif /*HTTPD_DEBUG*/
    return HTTP_ERR_NOTHING_TO_MODIFY;
  }
  
  /**
   *  Http Server is  using  loop back address ((0x7F000001) 127.0.0.1),So we are  passing  IPv4 address 
   *  to the cm_socket_recv_from().  
   */    
  /*PeerIp.ip_type_ul=IGW_INET_IPV4;*/
  if(PollFds[0].returned_events & CM_FDPOLL_READ)
  {
    iRetVal = cm_socket_recv_from(SFd, (char*)uiLpBkRecvData,sizeof(uiLpBkRecvData),
  	                            0, &PeerIp, &PeerPort);
    if(iRetVal < 0)
    {
      HttpDebug(("Unable to recieve data on loop back socket\n\r"));
      cm_socket_close(SFd);
      return HTTP_ERR_LPBK_RECV_FAIL;
    }
  }
  else
  {
    HttpDebug(("No response on loop back socket\n\r"));
    cm_socket_close(SFd);
    return HTTP_ERR_LPBK_RECV_FAIL; 
  }
  
#ifdef MESGCLNT_SUPPORT
/*  IGWSprintf(Message, "Http Port, %d has been deleted",usPortNum);
  IBsendSysLogMsg(NULL, 0, 0, Message, 0, ISEC_CONFIG_CHANGES, LOGINFO,
                  HTTPD_SYSMSGID_HTTP_PORT_DELETE_10);*/
  igwHTTPDLogConfigMsg(HTTPD_SYSMSGID_HTTP_PORT_DELETE_10,
                     IGW_LOG_FAMILY_CONFIG,usPortNum) ;
#endif
   cm_socket_close(SFd);
  return uiLpBkRecvData[0];
}

/***************************************************************************
 * Function Name  : HttpChangeServerPort
 * Description    : This API will change the port number of the http server.
 *                  Old port number must be already configured and it can 
 *                  be in enabled or disabled state.
 * Input          : 
 * Output         : 
 * Return value   : 
 ***************************************************************************/
int32_t HttpChangeConfiguration (uint16_t usOldPortNum,
                                 HttpPortInfo_t PortInfo)
{
  int32_t          SFd = -1;
  int32_t          iRetVal=0;
  HttpPortArgs_t   PortArgs = {};
  struct cm_poll_info    PollFds[1] = {};
  ipaddr           PeerIp;
  uint16_t         PeerPort = 0;
  uint32_t         uiLpBkRecvData[2]={};

  if((usOldPortNum == 0) || (PortInfo.usPort == 0))
  {
    HttpDebug(("Inavlid arguments\n\r"));
    return HTTP_ERR_INVALID_ARGUMENT;
  }

  PortArgs.usOldPort = usOldPortNum;
  PortArgs.PortInfo = PortInfo;
  PortArgs.ucCmd = HTTP_CMD_CHANGE_CONFIG;

  iRetVal = HttpSetLoopBack(&SFd);
  if(iRetVal != OF_SUCCESS)
  {
    HttpDebug(("Unable to set the loop back socket\n\r"));
    return iRetVal;
  }

  PollFds[0].fd_i = SFd;
  PollFds[0].interested_events = CM_FDPOLL_READ;

  iRetVal = cm_socket_send_to(SFd,(char*)&PortArgs,sizeof(HttpPortArgs_t),
  	                        0, HTTP_LOOPBACK_ADDR, HTTP_LOOPBACK_PORT_SRV);
  if(iRetVal < 0)
  {
    HttpDebug(("Unable to send data\n\r"));
    cm_socket_close(SFd);
    return HTTP_ERR_LPBK_DATA_SEND_FAIL;
  }

  iRetVal=cm_fd_poll(1, PollFds, 1000);  
  if(iRetVal < 0)
  {
#ifdef HTTPD_DEBUG
    HttpDebug("FdPoll Returned Error :%d",iRetVal);
#endif /*HTTPD_DEBUG*/
    return HTTP_ERR_NOTHING_TO_MODIFY;
  }
    /**
   *  Http Server is  using  loop back address ((0x7F000001) 127.0.0.1),So we are  passing  IPv4 address 
   *  to the cm_socket_recv_from().  
   */    
  /*PeerIp.ip_type_ul=IGW_INET_IPV4;*/
   
  if(PollFds[0].returned_events & CM_FDPOLL_READ)
  {
    iRetVal = cm_socket_recv_from(SFd, (char*)uiLpBkRecvData,sizeof(uiLpBkRecvData),
  	                            0, &PeerIp, &PeerPort);
    if(iRetVal < 0)
    {
      HttpDebug(("Unable to recieve data on loop back socket\n\r"));
      cm_socket_close(SFd);
      return HTTP_ERR_LPBK_RECV_FAIL;
    }
  }
  
  cm_socket_close(SFd);
  #ifdef MESGCLNT_SUPPORT
  igwHTTPDLogConfigMsg(HTTPD_SYSMSGID_HTTP_PORT_STATUS,
                     IGW_LOG_FAMILY_CONFIG,usOldPortNum) ;
  #endif
  return uiLpBkRecvData[0];
}


/***************************************************************************
 * Function Name  : HttpSetLoopBack
 * Description    : This function is used to setup a loop back socket and 
 *                  binds to HTTP_LOOPBACK_PORT
 * Input          : None
 * Output         : iSFd - socket fd
 * Return value   : OF_SUCCESS if socket is created and successfully bind to
 *                  HTTP_LOOPBACK_PORT else appropriate error code.
 ***************************************************************************/
 int32_t HttpSetLoopBack(int32_t *iSFd)
{
  int32_t          SFd = 0;

  SFd = cm_socket_create(CM_IPPROTO_UDP);
  if(SFd < 0)
  {
    HttpDebug(("Unable to open socket\n\r"));
    return HTTP_ERR_SOCKET_CREATE_FAIL;
  }

  cm_set_sock_reuse_opt(SFd);
  if (cm_socket_bind (SFd, HTTP_LOOPBACK_ADDR, HTTP_LOOPBACK_PORT) == OF_FAILURE)
  {
    HttpDebug(("Unable to bind the socket for loop back\n\r"));
    cm_socket_close (SFd);
    return HTTP_ERR_BIND_FAIL;
  }

  *iSFd = SFd;
  return OF_SUCCESS;

}

/***************************************************************************
 * Function Name  : HttpLoadDefaultPortConfig
 * Description    : 
 * Input          : 
 * Output         : 
 * Return value   : 
 ***************************************************************************/
int32_t HttpLoadDefaultPortConfig()
{
  int32_t          SFd = -1;
  int32_t          iRetVal;
  HttpPortArgs_t   PortArgs = {};

  HttpDebug(("loading default configuration\n\r"));
  SFd = cm_socket_create(CM_IPPROTO_UDP);
  if(SFd < 0)
  {
    HttpDebug(("Unable to open socket\n\r"));
    return HTTP_ERR_SOCKET_CREATE_FAIL;
  }
  cm_set_sock_reuse_opt(SFd);

  PortArgs.ucCmd = HTTP_CMD_LOAD_DEF_CONFIG;
  iRetVal = cm_socket_send_to(SFd,(char*)&PortArgs,sizeof(HttpPortArgs_t),
  	                        0, HTTP_LOOPBACK_ADDR, HTTP_LOOPBACK_PORT_SRV);
  do
  {
    if(iRetVal < 0)
    {
      HttpDebug(("Unable to send data. Retransmitting\n\r"));
      iRetVal = cm_socket_send_to(SFd,(char*)&PortArgs,sizeof(HttpPortArgs_t),
      	                        0, HTTP_LOOPBACK_ADDR, HTTP_LOOPBACK_PORT_SRV);
      if(iRetVal < 0)
      {
        HttpDebug(("unable to retransmit data\n\r"));
        iRetVal = HTTP_ERR_LPBK_DATA_SEND_FAIL;
        break;
      }
      iRetVal = OF_SUCCESS;
    }
    else
      iRetVal = OF_SUCCESS;
  }while(0);
  cm_socket_close(SFd);
  return iRetVal;
}

#endif
