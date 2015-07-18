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

/** \ingroup Flow_Description
 *  \struct of_flow_mod_info
 *  \brief flow description details\n\n
 *  <b>Description </b>\n 
 *  Data structure to hold flow mod all fields are converted into host byte order\n
 */

#define OFU_FLOW_MATCH_FIELD_LEN  46
#define OFU_FLOW_INSTRUCTION_LEN  16
#define OFU_FLOW_ACTION_LEN  10


#define OFU_FLOW_COOKIE_START  0x12345678
#define OFU_FLOW_COOKIE_MASK   0xFFFFFFFF 

/*ERROR MACROS */

#define OF_FLOWMOD_ERROR_INVALID_MISSENTRY 11
#define OF_FLOWMOD_GOTOTABLE_INVALID 22


struct ofi_flow_match_fields
{
  of_list_node_t   next_match_field;
  uint16_t      field_id;
  uint8_t       field_type;
  uint8_t       field_len;

  uint8_t       ui8_data;
  uint16_t       ui16_data;
  uint32_t      ui32_data;
  uint64_t      ui64_data;
  uint8_t       ui8_data_array[6];
  ipv6_addr_t   ipv6_addr;

  //void          match_field_val[OFU_FLOW_MATCH_FIELD_LEN];
  uint8_t       is_mask_set;
  uint8_t       mask[OFU_FLOW_MATCH_FIELD_LEN];
  uint8_t       mask_len;
};
#define OF_FLOW_MATCH_FILED_LISTNODE_OFFSET offsetof(struct ofi_flow_match_fields, next_match_field)

struct ofi_flow_action
{
  of_list_node_t  next_action;
  uint16_t      action_id;
  uint8_t       action_type;
  uint8_t       action_len;
  
  uint32_t	port_no;
  uint16_t	max_len;
  uint8_t       ttl;
  uint16_t      ether_type;
  uint32_t	queue_id;
  uint32_t	group_id;
  int8_t       setfield_type;
  uint8_t       ui8_data;
  uint16_t       ui16_data;
  uint32_t      ui32_data;
  uint64_t      ui64_data;
  uint8_t       ui8_data_array[6];
  ipv6_addr_t   ipv6_addr;

};
#define OF_FLOW_ACTION_LISTNODE_OFFSET offsetof(struct ofi_flow_action, next_action)

struct ofi_flow_instruction
{ 
 of_list_node_t   next_instruction;
 uint16_t      inst_id;
 uint8_t       instruction_type;
 uint8_t       instruction_len;

 uint8_t       ui8_data;
 uint16_t      ui16_data;
 uint32_t      ui32_data;
 uint64_t      ui64_data;
 uint8_t       ui8_data_array[6];

 uint8_t       is_mask_set;
 uint8_t       mask[OFU_FLOW_INSTRUCTION_LEN];
 uint8_t       mask_len;

 of_list_t        action_list;
};

#define OF_FLOW_INSTRUCTION_LISTNODE_OFFSET offsetof(struct ofi_flow_instruction, next_instruction)
struct ofi_flow_mod_info
{
   /** Flow Id*/
   of_list_node_t   next_flow;
   uint32_t  flow_id;
   uint8_t   table_id;
   uint16_t  priority;
   uint8_t   command;
   uint16_t  idle_timeout;
   uint16_t  hard_timeout;
   uint64_t  cookie;
   uint64_t  cookie_mask;
   uint8_t   flow_flags;
   uint32_t  out_port;
   uint32_t  out_group;
   of_list_t    match_field_list; 
   of_list_t    instruction_list; 
   of_list_t    action_list; 
};
#define OF_FLOW_MOD_LISTNODE_OFFSET offsetof(struct ofi_flow_mod_info, next_flow)

int32_t of_flow_set_instruction_data(struct ofi_flow_instruction *inst_info,
                                    char *data, uint32_t data_len);
int32_t of_flow_set_action_setfield(struct of_msg *msg, struct ofi_flow_action *action);

int32_t of_datapath_add_static_flow(uint64_t datapath_handle,
    struct ofi_flow_mod_info *flow_entry_p);

int32_t of_datapath_update_flow(uint64_t datapath_handle, struct ofi_flow_mod_info *flow_info_p);

int32_t of_datapath_remove_static_flow(uint64_t datapath_handle, uint32_t flow_id);

int32_t of_flow_mod_frame_and_send_message(uint64_t datapath_handle, uint32_t flow_id,
    uint8_t  table_id, uint16_t priority,
    uint8_t command);

int32_t of_get_flow(uint64_t datapath_handle,  uint32_t flow_id,
    struct ofi_flow_mod_info **flow_entry_pp);

int32_t of_flow_add_match_field_to_list(uint64_t datapath_handle, 
    struct ofi_flow_match_fields *match_info,
    uint32_t flow_id);

int32_t of_flow_remove_match_field_from_list(uint64_t datapath_handle, uint32_t flow_id, uint8_t match_field_id);

int32_t of_flow_set_match_field_type(
    struct ofi_flow_match_fields *match_info,
    char *match_type, uint32_t match_len);

int32_t of_flow_set_match_field_data(struct ofi_flow_match_fields *match_info,
    char *data, uint32_t data_len);

int32_t of_flow_set_match_field_mask(struct ofi_flow_match_fields *match_info,
    char *field_mask, uint32_t mask_len);

int32_t of_get_flow_match_field(uint64_t datapath_handle,  uint32_t flow_id, uint32_t field_id,
    struct ofi_flow_match_fields **match_entry_pp);

int32_t of_flow_add_instruction_to_list(uint64_t datapath_handle, 
    struct ofi_flow_instruction *instruction_info,
    uint32_t flow_id);

int32_t of_flow_update_inst(uint64_t datapath_handle,  uint32_t flow_id,
    struct ofi_flow_instruction *inst_entry_p);

int32_t of_flow_remove_instruction_from_list(uint64_t datapath_handle,
    uint32_t flow_id, uint8_t instruction_id);

int32_t of_flow_set_instruction_type(
    struct ofi_flow_instruction *instruction_info,
    char *instruction_type, uint32_t instruction_len);

int32_t of_get_flow_inst(uint64_t datapath_handle,  uint32_t flow_id, uint32_t inst_id,
    struct ofi_flow_instruction **inst_entry_pp);

int32_t of_flow_add_action_to_list(uint64_t datapath_handle, 
    struct ofi_flow_action *action_info,
    uint32_t flow_id);

int32_t of_flow_update_actionset(uint64_t datapath_handle,
    uint32_t flow_id,
    struct ofi_flow_action *action_entry_p);

int32_t of_flow_remove_action_from_list(uint64_t datapath_handle,
    uint32_t flow_id, uint8_t action_id);

int32_t of_flow_set_action_type(
    struct ofi_flow_action *action_info,
    char *action_type, uint32_t action_len);

int32_t of_flow_set_action_data(struct ofi_flow_action *action_info,
    char *data, uint32_t data_len);

int32_t of_get_flow_action(uint64_t datapath_handle,  uint32_t flow_id, uint32_t action_id,
    struct ofi_flow_action **action_entry_pp);

int32_t of_flow_instruction_add_action_to_list(uint64_t datapath_handle, 
    struct ofi_flow_action *action_info,
    uint32_t flow_id, uint16_t instruction_id);

int32_t of_flow_update_actionlist(uint64_t datapath_handle,
    uint32_t flow_id, uint32_t inst_id,
    struct ofi_flow_action *action_entry_p);

int32_t of_flow_instruction_remove_action_from_list(uint64_t datapath_handle, 
    struct ofi_flow_action *action_info,
    uint32_t flow_id, uint8_t instruction_id);

int32_t of_flow_set_action_setfield_type(struct ofi_flow_action *action_info,
    char *setfield_type, uint32_t setfield_len);

int32_t of_flow_set_action_setfieldtype_value(struct ofi_flow_action *action_info,
    char *data, uint32_t data_len);

int32_t of_get_flow_action_from_actionlist(uint64_t datapath_handle,  uint32_t flow_id,
    uint32_t inst_id, uint32_t action_id,
    struct ofi_flow_action **action_entry_pp);

int32_t of_flow_set_instruction_data_mask(struct ofi_flow_instruction *inst_info,
    char *data_mask, uint32_t mask_len);

