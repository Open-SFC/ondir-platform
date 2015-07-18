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

/******************************************************************************
* Copyright 2013-2014, Freescale Semiconductor, Inc. All rights reserved.
******************************************************************************/
/*
* File:of_tmpl_utils.c
* Description:
*       This file contains template instructions/actions utils APIs.
* Authors :Varun<B36461@freescale.com>
* Modifier:
* History:
*  Initial implementation.
******************************************************************************/
#include "controller.h"
#include "cntrlappcmn.h"
#include "cntlr_transprt.h"
#include "fsl_ext.h"
#include "nicira_ext.h"

#include "of_tmpl_utils.h"

#define OF_SET_FIELD_ACTION_PADDING_LEN(oxm_len) \
        (((oxm_len) + 7)/8*8 - (oxm_len))

/*########################### Instruction routines ###########################*/
inline uint32_t ofu_template_start_pushing_instructions(uint8_t *inst)
{
  cntlr_assert(inst != NULL);
  return OFU_INSTRUCTION_PUSH_SUCCESS;
}


inline void
ofu_template_start_pushing_apply_action_instruction(uint8_t *inst, 
                                                    uint16_t *length)
{
  struct ofp_instruction_actions *action_instruction=NULL;

  action_instruction = (struct ofp_instruction_actions*)(inst);
  action_instruction->type = htons(OFPIT_APPLY_ACTIONS);
  action_instruction->len  = htons(OFU_APPLY_ACTION_INSTRUCTION_LEN);
  *length = OFU_APPLY_ACTION_INSTRUCTION_LEN;
}


inline int32_t
ofu_template_push_go_to_table_instruction(uint8_t *inst, 
                                          uint8_t table_id, 
                                          uint16_t *length_p)
{
  struct ofp_instruction_goto_table *gototable_instrctn=NULL;

  gototable_instrctn = (struct ofp_instruction_goto_table*)(inst);

  gototable_instrctn->type      = htons(OFPIT_GOTO_TABLE);
  gototable_instrctn->len       = htons(OFU_GOTO_TABLE_INSTRUCTION_LEN);
  gototable_instrctn->table_id  = table_id;

  *length_p = OFU_GOTO_TABLE_INSTRUCTION_LEN;
  return OFU_INSTRUCTION_PUSH_SUCCESS;
  
}

inline void
ofu_template_end_pushing_apply_action_instruction(uint8_t *inst, uint16_t inst_len)
{
  ((struct ofp_instruction_actions *)(inst))->len = htons(inst_len);
   return;
}

inline void
ofu_template_end_pushing_instructions(uint8_t *inst)
{
  cntlr_assert(inst != NULL);
  return;
}
/*########################### Action routines ###############################*/
inline void
ofu_template_start_pushing_actions(uint8_t *inst)
{
  cntlr_assert(inst != NULL);
  return;
}

inline void
ofu_template_end_pushing_actions(uint8_t *inst)
{
  cntlr_assert(inst != NULL);
  return;
}

inline int32_t
ofu_template_push_output_action(uint8_t   *inst,
                                uint32_t  port_no,
                                uint16_t  max_len,
                                uint16_t *length_p)
{
  struct ofp_action_output *output_action=NULL;

  cntlr_assert(inst != NULL);
  output_action = (struct ofp_action_output *)inst;
  output_action->type    = htons(OFPAT_OUTPUT);
  output_action->len     = htons(sizeof(struct ofp_action_output));
  output_action->port    = htonl(port_no);
  output_action->max_len = htons(max_len);
  *length_p = sizeof(struct ofp_action_output);
  return OFU_ACTION_PUSH_SUCCESS;
}

inline int32_t
ofu_template_push_set_ipv4_src_addr_in_set_field_action(uint8_t   *inst,
					        ipv4_addr_t   *ipv4_addr,
						        uint16_t *length_p)
{   
  struct ofp_action_set_field *set_field=NULL;
  uint16_t padding_bytes = OF_SET_FIELD_ACTION_PADDING_LEN(4);

  cntlr_assert(inst != NULL);

  set_field = (struct ofp_action_set_field *)inst;
  set_field->type = htons(OFPAT_SET_FIELD); 
  set_field->len  = htons(OFU_SET_FIELD_WITH_IPV4_SRC_FIELD_ACTION_LEN + padding_bytes);
  *(uint32_t*)set_field->field   = htonl(OXM_OF_IPV4_SRC);
  *(uint32_t*)(set_field->field + OF_OXM_HDR_LEN) = htonl(*ipv4_addr);
  memset((set_field->field + OFU_SET_FIELD_WITH_IPV4_SRC_FIELD_ACTION_LEN),0,padding_bytes);
  *length_p = OFU_SET_FIELD_WITH_IPV4_SRC_FIELD_ACTION_LEN + padding_bytes;
  return OFU_ACTION_PUSH_SUCCESS;
}


inline int32_t
ofu_template_push_set_ipv4_dst_addr_in_set_field_action(uint8_t   *inst,
					       ipv4_addr_t   *ipv4_addr,
			     			     uint16_t *length_p)
{   
  struct ofp_action_set_field *set_field=NULL;
  uint16_t padding_bytes = OF_SET_FIELD_ACTION_PADDING_LEN(4);

  cntlr_assert(inst != NULL);

  set_field = (struct ofp_action_set_field *)inst;
  set_field->type = htons(OFPAT_SET_FIELD); 
  set_field->len  = htons(OFU_SET_FIELD_WITH_IPV4_DST_FIELD_ACTION_LEN + padding_bytes);
  *(uint32_t*)set_field->field   = htonl(OXM_OF_IPV4_DST);
  *(uint32_t*)(set_field->field + OF_OXM_HDR_LEN) = htonl(*ipv4_addr);
  memset( (set_field->field + OFU_SET_FIELD_WITH_IPV4_DST_FIELD_ACTION_LEN),0,padding_bytes);
  *length_p = OFU_SET_FIELD_WITH_IPV4_DST_FIELD_ACTION_LEN + padding_bytes;
  return OFU_ACTION_PUSH_SUCCESS;
}

inline int32_t 
ofu_template_push_set_udp_src_port_in_set_field_action(uint8_t   *inst,
                                                       uint16_t  *port_number,
  			     			       uint16_t  *length_p)
{     
  struct ofp_action_set_field *set_field;
  uint16_t padding_bytes = OF_SET_FIELD_ACTION_PADDING_LEN(2);

  set_field = (struct ofp_action_set_field *)(inst);
  set_field->type = htons(OFPAT_SET_FIELD);
  set_field->len  = htons(OFU_SET_FIELD_WITH_UDP_SRC_PORT_FIELD_ACTION_LEN
                          + padding_bytes);

  *(uint32_t*)set_field->field   = htonl(OXM_OF_UDP_SRC);

  *(uint16_t*)(set_field->field + OF_OXM_HDR_LEN) = htons(*port_number);

  memset((set_field->field + OFU_SET_FIELD_WITH_UDP_SRC_PORT_FIELD_ACTION_LEN),
         0,
         padding_bytes);
  *length_p = OFU_SET_FIELD_WITH_UDP_SRC_PORT_FIELD_ACTION_LEN + padding_bytes;
  return OFU_ACTION_PUSH_SUCCESS; 
}


inline int32_t 
ofu_template_push_set_udp_dst_port_in_set_field_action(uint8_t   *inst,
                                                       uint16_t  *port_number,
  			     			       uint16_t  *length_p)
{     
  struct ofp_action_set_field *set_field;
  uint16_t padding_bytes = OF_SET_FIELD_ACTION_PADDING_LEN(2);

  set_field = (struct ofp_action_set_field *)(inst);
  set_field->type = htons(OFPAT_SET_FIELD);
  set_field->len  = htons(OFU_SET_FIELD_WITH_UDP_DST_PORT_FIELD_ACTION_LEN
                          + padding_bytes);

  *(uint32_t*)set_field->field   = htonl(OXM_OF_UDP_DST);
  *(uint16_t*)(set_field->field + OF_OXM_HDR_LEN) = htons(*port_number);
  memset((set_field->field + OFU_SET_FIELD_WITH_UDP_DST_PORT_FIELD_ACTION_LEN),
         0, padding_bytes);
  *length_p = OFU_SET_FIELD_WITH_UDP_DST_PORT_FIELD_ACTION_LEN + padding_bytes;
  return OFU_ACTION_PUSH_SUCCESS; 
}

inline int32_t 
ofu_template_push_set_tcp_src_port_in_set_field_action(uint8_t   *inst,
                                                       uint16_t  *port_number,
  			     			       uint16_t  *length_p)
{     
  struct ofp_action_set_field *set_field;
  uint16_t padding_bytes = OF_SET_FIELD_ACTION_PADDING_LEN(2);

  set_field = (struct ofp_action_set_field *)(inst);
  set_field->type = htons(OFPAT_SET_FIELD);
  set_field->len  = htons(OFU_SET_FIELD_WITH_TCP_SRC_PORT_FIELD_ACTION_LEN
                          + padding_bytes);

  *(uint32_t*)set_field->field   = htonl(OXM_OF_TCP_SRC);

  *(uint16_t*)(set_field->field + OF_OXM_HDR_LEN) = htons(*port_number);

  memset((set_field->field + OFU_SET_FIELD_WITH_TCP_SRC_PORT_FIELD_ACTION_LEN),
         0,
         padding_bytes);
  *length_p = OFU_SET_FIELD_WITH_TCP_SRC_PORT_FIELD_ACTION_LEN + padding_bytes;
  return OFU_ACTION_PUSH_SUCCESS; 
}

inline int32_t 
ofu_template_push_set_tcp_dst_port_in_set_field_action(uint8_t   *inst,
                                                       uint16_t  *port_number,
  			     			       uint16_t  *length_p)
{     
  struct ofp_action_set_field *set_field;
  uint16_t padding_bytes = OF_SET_FIELD_ACTION_PADDING_LEN(2);

  set_field = (struct ofp_action_set_field *)(inst);
  set_field->type = htons(OFPAT_SET_FIELD);
  set_field->len  = htons(OFU_SET_FIELD_WITH_TCP_DST_PORT_FIELD_ACTION_LEN
                          + padding_bytes);

  *(uint32_t*)set_field->field   = htonl(OXM_OF_TCP_DST);
  *(uint16_t*)(set_field->field + OF_OXM_HDR_LEN) = htons(*port_number);
  memset((set_field->field + OFU_SET_FIELD_WITH_TCP_DST_PORT_FIELD_ACTION_LEN),
         0, padding_bytes);
  *length_p = OFU_SET_FIELD_WITH_TCP_DST_PORT_FIELD_ACTION_LEN + padding_bytes;
  return OFU_ACTION_PUSH_SUCCESS; 
}


/* Experimenter instructions */

inline int32_t
fslx_template_instruction_start_pushing_apply_actions_during_add_mod(
                                               uint8_t *inst,
					       uint16_t *length_p)
{
  struct fslx_instrctn_operation_actns *operation=NULL;

  operation = (struct fslx_instrctn_operation_actns *)(inst);
  operation->type = htons(OFPIT_EXPERIMENTER);
  operation->len  = htons(FSLXIT_APPLY_ACTION_ON_CREATE_OR_DELETE_INSTRUCTION_LEN);
  operation->vendor = htonl(FSLX_VENDOR_ID);
  operation->subtype = htons(FSLXIT_APPLY_ACTIONS_ON_CREATE);
  *length_p =  FSLXIT_APPLY_ACTION_ON_CREATE_OR_DELETE_INSTRUCTION_LEN;
  return OFU_INSTRUCTION_PUSH_SUCCESS;
}

inline
int32_t fslx_template_action_attach_bind_object(uint8_t *inst, 
                                               uint32_t bind_obj_id,
                                               uint8_t obj_type,
					       uint16_t *length_p)
{     
  struct fslx_action_attach_bind_obj *bind_obj=NULL;     
  bind_obj = 
     (struct fslx_action_attach_bind_obj *)(inst);        
  bind_obj->type = htons(OFPAT_EXPERIMENTER);
  bind_obj->len  = htons(FSLX_ACTION_ATTACH_BIND_OBJ_ID_LEN);
  bind_obj->vendor  = htonl(FSLX_VENDOR_ID);
  bind_obj->subtype = htons(FSLXAT_ATTACH_BIND_OBJ);
  bind_obj->bind_obj_id = htonl(bind_obj_id);
  bind_obj->obj_type = obj_type;
  bind_obj->hit_count = 0;
  bind_obj->flag = 0;
  memset(bind_obj->pad, 0, 4);
  *length_p = (FSLX_ACTION_ATTACH_BIND_OBJ_ID_LEN);
  return OFU_ACTION_PUSH_SUCCESS;
}

inline int32_t
fslx_template_instruction_end_pushing_apply_actions_during_add_mod(
                                                uint8_t *inst,
						uint16_t tot_len)
{     
    ((struct ofp_instruction_actions *)(inst))->len = htons(tot_len);
     return OFU_INSTRUCTION_PUSH_SUCCESS;
}       


inline
int32_t fslx_template_action_execute_bind_obj_actions(uint8_t *inst,
                                                    uint32_t bind_obj_id,
					            uint16_t *length_p)
{     
  struct fslx_action_execute_bind_obj_apply_actns *bind_action=NULL;  
  bind_action =
     (struct fslx_action_execute_bind_obj_apply_actns *)(inst);
  bind_action->type = htons(OFPAT_EXPERIMENTER);
  bind_action->len = htons(FSLX_ACTION_EXECUTE_BIND_OBJ_APPLY_ACTNS_LEN);
  bind_action->vendor = htonl(FSLX_VENDOR_ID);
  bind_action->subtype = htons(FSLXAT_BIND_OBJECT_APPLY_ACTIONS);
  bind_action->bind_obj_id = htonl(bind_obj_id);
  bind_action->obj_type = htons(FSLX_BIND_OBJ_DYNAMIC);
  //memset(&bind_action->pad, 0 , sizeof(bind_action->pad));
  *length_p = (FSLX_ACTION_EXECUTE_BIND_OBJ_APPLY_ACTNS_LEN);
  return OFU_ACTION_PUSH_SUCCESS;
}

inline int32_t
fslx_template_instruction_start_pushing_apply_actions_during_del(uint8_t *inst,
  							   uint16_t *length_p)
{
  struct fslx_instrctn_operation_actns *operation=NULL;
  cntlr_assert(inst != NULL);
  operation = (struct fslx_instrctn_operation_actns *)(inst);
  operation->type = htons(OFPIT_EXPERIMENTER);
  operation->len  = htons(FSLXIT_APPLY_ACTION_ON_CREATE_OR_DELETE_INSTRUCTION_LEN);
  operation->vendor = htonl(FSLX_VENDOR_ID);
  operation->subtype = htons(FSLXIT_APPLY_ACTIONS_ON_DELETE);
  *length_p = FSLXIT_APPLY_ACTION_ON_CREATE_OR_DELETE_INSTRUCTION_LEN;
   return OFU_INSTRUCTION_PUSH_SUCCESS;
}

inline
int32_t fslx_template_action_detach_bind_object(uint8_t *inst, 
                                                uint32_t bind_obj_id,
					        uint16_t *length_p)
{
  struct fslx_action_detach_bind_obj *bind_obj=NULL;  
  bind_obj = 
     (struct fslx_action_detach_bind_obj *)(inst);      
  bind_obj->type = htons(OFPAT_EXPERIMENTER);
  bind_obj->len = htons(FSLX_ACTION_DETACH_BIND_OBJ_ID_LEN);
  bind_obj->vendor = htonl(FSLX_VENDOR_ID);
  bind_obj->subtype = htons(FSLXAT_DETACH_BIND_OBJ);
  bind_obj->bind_obj_id = htonl(bind_obj_id);
  memset(&bind_obj->pad, 0 , sizeof(bind_obj->pad));
  *length_p = (FSLX_ACTION_DETACH_BIND_OBJ_ID_LEN);
  OF_LOG_MSG(OF_LOG_IPSEC, OF_LOG_INFO, "EXIT");
  return OFU_ACTION_PUSH_SUCCESS;
}

inline 
int32_t fslx_template_instruction_end_pushing_apply_actions_during_del(uint8_t *inst, uint16_t inst_len)
{     
 ((struct ofp_instruction_actions *)(inst))->len = htons(inst_len);   
  return OFU_INSTRUCTION_PUSH_SUCCESS;
}

inline int32_t
fslx_template_start_pushing_write_table_action_instruction(uint8_t *inst, 
                                                           uint8_t table_id,
							   uint16_t *length_p)
{
  struct fslxit_table_actions *operation=NULL;

  cntlr_assert(inst != NULL); 
  operation = (struct fslxit_table_actions *)(inst);
  operation->type = htons(OFPIT_EXPERIMENTER);
  operation->len  = htons(FSLXIT_WRITE_TABLE_ACTIONS_LEN);
  operation->vendor = htonl(FSLX_VENDOR_ID);
  operation->subtype = htons(FSLXIT_WRITE_TABLE_ACTIONS);
  operation->table_id= table_id;
  *length_p = FSLXIT_WRITE_TABLE_ACTIONS_LEN;
  return OFU_INSTRUCTION_PUSH_SUCCESS;
}

inline int32_t
fslx_template_end_pushing_table_action_instruction(uint8_t *inst, uint16_t inst_len)
{      
 ((struct ofp_instruction_actions *)(inst))->len = htons(inst_len);   
  return OFU_INSTRUCTION_PUSH_SUCCESS;
}                      


inline int32_t
nicira_template_action_write_register(uint8_t   *inst, 
                                      uint32_t index, 
                                      uint64_t data, 
                                      uint8_t offset, 
                                      uint8_t n_bits,
                                      uint16_t *length_p)
{
   uint64_t data_n_bits ;
   struct nx_action_reg_load *reg_load=NULL;

   /* Reject if a bit numbered 'n_bits' or higher is set to 1 in data. */
   if(n_bits > 64)
   {
     OF_LOG_MSG(OF_LOG_EXT_NICIRA, OF_LOG_WARN, "n_bits value greater than 64");
     return OFPERR_OFPBAC_BAD_ARGUMENT;
   }
   data_n_bits = data >> n_bits;
   if (data_n_bits) 
   {
     OF_LOG_MSG(OF_LOG_EXT_NICIRA, OF_LOG_WARN, "data_n_bits is non zero");
     return OFPERR_OFPBAC_BAD_ARGUMENT;
   }
   reg_load =
	   (struct nx_action_reg_load *)(inst);
   reg_load->type = htons(OFPAT_EXPERIMENTER);// OFPAT_VENDOR); 
   reg_load->len = htons(NX_ACTION_LOAD_REG_LEN);
   reg_load->vendor = htonl(NX_VENDOR_ID);
   reg_load->subtype = htons(NXAST_REG_LOAD);
   reg_load->ofs_nbits = htons((offset << 6) | (n_bits -1));
   reg_load->dst = htonl(index);
   reg_load->value = htonll(data);
   *length_p = (NX_ACTION_LOAD_REG_LEN);
   return OFU_SET_FIELD_SUCCESS;
}


 

































