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

#include "controller.h"
#include "of_utilities.h"
//#include "of_multipart.h"
#include "of_msgapi.h"
#include "cntlr_tls.h"
//#include "cntrl_queue.h"
#include "cntlr_transprt.h"
#include "cntlr_event.h"
#include "dprmldef.h"
#include "fsl_ext.h"
extern void cntrl_ucm_send_multipart_response(uint32_t multipart_type,void *data_p, uint8_t last);
void of_queue_config_reply_handler(uint64_t  controller_handle,
                                                uint64_t  domain_handle,
                                                uint64_t  datapath_handle,
                                                 struct of_msg *msg,
                                                void     *cbk_arg1,
                                                void     *cbk_arg2,
                                                uint8_t  status,
                                                struct ofl_queue_config *queue_config);
int32_t
of_queue_send_get_queue_config_request(uint64_t datapath_handle,uint32_t port);
int32_t of_queue_frame_response(char *msg,struct ofl_queue_config *queue_config_info, uint32_t msg_length);
void of_queue_free_queue_entry(struct rcu_head *node);
int32_t of_queue_frame_response(char *msg,struct ofl_queue_config *queue_config_info, uint32_t msg_length)
{
  struct ofl_queue *queue_entry_p;

  OF_LOG_MSG(OF_LOG_MOD, OF_LOG_DEBUG,"frame response msg_length %d", msg_length);

  OF_LIST_INIT(queue_config_info->queue_list, of_queue_free_queue_entry);

  while (msg_length > 0)
  {
    queue_entry_p = (struct ofl_queue *)calloc(1,sizeof(struct ofl_queue ));
  if (queue_entry_p == NULL )
  {
    OF_LOG_MSG(OF_LOG_MOD, OF_LOG_ERROR,"alloc failed");
    return OF_FAILURE;
  }
    struct ofp_packet_queue *q = (struct ofp_packet_queue *)(msg + sizeof(struct ofp_queue_get_config_reply));
    struct ofp_queue_prop_experimenter *prop = (struct ofp_queue_prop_experimenter *)((char *)q + sizeof(struct ofp_packet_queue));
    queue_entry_p->ofq_id = ntohl(q->queue_id);
    queue_entry_p->cq_id = ntohl(*((uint32_t *)(prop->data)));
    OF_APPEND_NODE_TO_LIST(queue_config_info->queue_list, queue_entry_p,OF_QUEUE_LISTNODE_OFFSET);
	if(OF_LIST_COUNT(queue_config_info->queue_list) >= 45)
    {
		OF_LOG_MSG(OF_LOG_MOD, OF_LOG_DEBUG,"WORK AROUND FOR AVOIDING CRASH DUE TO MEMORY CORRUPTION ");
		break;
    }
    msg += sizeof(struct ofp_packet_queue);
    msg += prop->prop_header.len;/*length of property including data and header*/
    msg_length -= sizeof(struct ofp_packet_queue);
    msg_length -= prop->prop_header.len;
  }
  return OF_SUCCESS;
}

void of_queue_free_queue_entry(struct rcu_head *node)
{
  struct ofl_queue  *queue;

  if(node)
  {
    queue = (struct ofl_queue *)(((char *)node) - OF_QUEUE_LISTNODE_OFFSET);
    if(queue)
    {
      free(queue); 
      OF_LOG_MSG(OF_LOG_MOD, OF_LOG_DEBUG,
                 "Deleted succesfully queue list node");
    }
    else
    {
      OF_LOG_MSG(OF_LOG_MOD, OF_LOG_DEBUG,
                 "Failed to free queue list node"); 
    }
  } 
  return;
}   

	int32_t
of_queue_send_get_queue_config_request(uint64_t datapath_handle,uint32_t port)
{
	struct of_msg  *msg;
	struct ofp_header *msg_hdr;
	struct dprm_datapath_entry   *datapath;
	cntlr_conn_node_saferef_t conn_safe_ref;
	cntlr_conn_table_t        *conn_table;
        void     *cbk_arg1=NULL;
        void     *cbk_arg2=NULL;
	int32_t         retval= CNTLR_SUCCESS;
	int32_t         status= OF_SUCCESS;
    struct ofp_queue_get_config_request *pconfig_request=NULL;

	OF_LOG_MSG(OF_LOG_MOD, OF_LOG_INFO,"%s:%d entered port %X\r\n",__FUNCTION__,__LINE__,port);
	do   
	{
		msg = ofu_alloc_message(OFPT_QUEUE_GET_CONFIG_REQUEST,
				sizeof(struct ofp_queue_get_config_request));
		if(msg == NULL)
		{    
			OF_LOG_MSG(OF_LOG_MOD, OF_LOG_ERROR,"%s:%d OF message alloc error \r\n",__FUNCTION__,__LINE__);
			status = OF_FAILURE;
			break;
		}    
		msg_hdr = (struct ofp_header*)(msg->desc.start_of_data);
		msg_hdr->version  = OFP_VERSION;
		msg_hdr->type     = OFPT_QUEUE_GET_CONFIG_REQUEST;
		msg_hdr->xid      = 0;  
		msg->desc.data_len = sizeof(struct ofp_queue_get_config_request);
		msg_hdr->length    = htons(msg->desc.data_len);
        pconfig_request = (struct ofp_queue_get_config_request *)(msg->desc.start_of_data);
        pconfig_request->port = port; 
		CNTLR_RCU_READ_LOCK_TAKE();
		retval = get_datapath_byhandle(datapath_handle, &datapath);
		if(retval  != DPRM_SUCCESS)
		{
			OF_LOG_MSG(OF_LOG_MOD, OF_LOG_ERROR,"%s:%d Error in getting datapath info, Err=%d \r\n",
					__FUNCTION__,__LINE__,retval);
			status = DPRM_INVALID_DATAPATH_HANDLE;
			break;
		}

		CNTLR_GET_MAIN_CHANNEL_SAFE_REF(datapath,conn_safe_ref,conn_table);
		if ( conn_table == NULL )
		{
			OF_LOG_MSG(OF_LOG_MOD, OF_LOG_ERROR,"conn table NULL");
			status = OF_FAILURE;
			break;
		}
		retval = cntlr_send_msg_to_dp(msg,datapath,conn_table,&conn_safe_ref,
				of_queue_config_reply_handler,cbk_arg1,cbk_arg2);
		if(retval  == OF_FAILURE)
		{
			OF_LOG_MSG(OF_LOG_MOD, OF_LOG_ERROR,"%s:%d Error in sending message to datapath \r\n",__FUNCTION__,__LINE__);
			status = CNTLR_SEND_MSG_TO_DP_ERROR;
			break;
		}
	}
	while(0);
	CNTLR_RCU_READ_LOCK_RELEASE();
	if(status != OF_SUCCESS)
	{
		if(msg != NULL)
			msg->desc.free_cbk(msg);
	}
	return status;
}

void of_queue_config_reply_handler(uint64_t  controller_handle,
                                                uint64_t  domain_handle,
                                                uint64_t  datapath_handle,
                                                 struct of_msg *msg,
                                                void     *cbk_arg1,
                                                void     *cbk_arg2,
                                                uint8_t  _status,
                                                struct ofl_queue_config *queue_config)
{
	OF_LOG_MSG(OF_LOG_MOD, OF_LOG_INFO,"%s:%d entered \r\n",__FUNCTION__,__LINE__);
  cntrl_ucm_send_multipart_response(OFPT_QUEUE_GET_CONFIG_REPLY,(void *)queue_config,TRUE);

}


    
