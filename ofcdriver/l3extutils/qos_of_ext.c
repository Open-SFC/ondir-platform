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

/*File: qos_of_extn.c
 *Author: Vikas Choudhary <vikas.choudhary@freescale.com>
 * Description:
 * This file contails NB APIs of experimenter L3 extension instructions 
 */
#include "controller.h"
#include "of_utilities.h"
#include "cntlr_tls.h"
#include "cntrl_queue.h"
#include "cntlr_transprt.h"
#include "cntlr_event.h"
#include "dprmldef.h"
#include "of_utilities.h"
//#include "of_l3ext_utils.h"
#include "fsl_ext.h"
#include "oflog.h"
#include "qos_common.h"
#include "qos_nf_api.h"
int32_t fslx_action_qos_obj(struct of_msg *msg,
                                       void *in_param,uint8_t ofq_base);


/***************************************************************************
 * Function Name : fslx_action_qos_obj
 * Description   :This action will get executed to implement(add/delete) qos configuration 
 * Parameter     : None 
 * Return value  : None
***************************************************************************/
int32_t fslx_action_qos_obj(struct of_msg *msg,
                                      void *in_param,uint8_t ofq_base)
{
struct nf_qos_cfg_inargs *in = (struct nf_qos_cfg_inargs *)in_param;
struct of_msg_descriptor *msg_desc;
  struct fslx_qos_obj *qos_obj;
  uint8_t                  ii;
  uint16_t                 total_len = 0;
    
  OF_LOG_MSG(OF_LOG_QOS, OF_LOG_INFO, "ENTRY");

  msg_desc = &msg->desc;
  if((msg_desc->ptr3+msg_desc->length3+(FSLX_ACTION_QOS_OBJ_LEN)) >
      (msg->desc.start_of_data + msg_desc->buffer_len))
  {   
    OF_LOG_MSG(OF_LOG_QOS, OF_LOG_ERROR,
               "Length of buffer is not suffeceient to add data");
    return OFU_NO_ROOM_IN_BUFFER;
  }
 total_len += FSLX_ACTION_QOS_OBJ_LEN;   
  qos_obj = 
     (struct fslx_qos_obj *)(msg_desc->ptr3 + msg_desc->length3);
  memset(qos_obj,0,sizeof(struct fslx_qos_obj));
  qos_obj->type = htons(OFPAT_EXPERIMENTER);
  qos_obj->vendor = htonl(FSLX_VENDOR_ID);
  qos_obj->subtype = htons(FSLXAT_QOS_CONFIG_OBJ);
  /*TODO:copy all parameters from in to qos_obj*/
  qos_obj->object_type = htonl(in->object_type);
  qos_obj->object_id = htonl(in->object_id);
  qos_obj->parent_id = htonl(in->parent_id);
  qos_obj->if_id = htonl(in->if_id);
  qos_obj->oper = in->oper;
  qos_obj->ofq_base = ofq_base;
  
  if(qos_obj->oper == QOS_CFG_ADD)
  {
   qos_obj->rate = htonl(in->rate);
   qos_obj->ceil = htonl(in->ceil);
   
   qos_obj->class_weight = htonl(in->class_weight);
   qos_obj->overhead = htons(in->overhead);
   qos_obj->queues = htons(in->queues);
   qos_obj->mpu = htons(in->mpu);
 for(ii=0 ; ii < 8 ;ii++)
  {
   qos_obj->cr_map[ii] = in->cr_map[ii];
   qos_obj->er_map[ii] = in->er_map[ii];
   qos_obj->weight[ii] = in->weight[ii];
OF_LOG_MSG(OF_LOG_QOS, OF_LOG_INFO, "weight%d %d",ii,qos_obj->weight[ii]);
   }  
OF_LOG_MSG(OF_LOG_QOS, OF_LOG_INFO, "rate %d ceil %d wt %d cr_mp %d %d %d %d %d %d %d er_map %d %d %d %d %d %d %d",qos_obj->rate,qos_obj->ceil,qos_obj->class_weight,qos_obj->cr_map[0],qos_obj->cr_map[1],qos_obj->cr_map[2],qos_obj->cr_map[3],qos_obj->cr_map[4],qos_obj->cr_map[5],qos_obj->cr_map[6],qos_obj->cr_map[7],qos_obj->er_map[0],qos_obj->er_map[1],qos_obj->er_map[2],qos_obj->er_map[3],qos_obj->er_map[4],qos_obj->er_map[5],qos_obj->er_map[6],qos_obj->er_map[7]);
  }
msg_desc->length3 = total_len;       
qos_obj->len = htons(total_len);   
OF_LOG_MSG(OF_LOG_QOS, OF_LOG_INFO, "EXIT, total len = %u",total_len);
  return OFU_ACTION_PUSH_SUCCESS;
}



