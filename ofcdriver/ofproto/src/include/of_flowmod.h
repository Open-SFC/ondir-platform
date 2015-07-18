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

#ifndef __OF_FLOW_MOD_H
#define __OF_FLOW_MOD_H


enum flow_commands
{
     ADD_FLOW=1,
     MODIFY_FLOW=2,
     DEL_FLOW=3
};

struct flow_trans{
    of_list_node_t     *next_flow;
    uint64_t datapath_handle;
    uint32_t trans_id;
    uint8_t  table_id;
    uint16_t  priority;
    uint32_t flow_id;
    uint32_t command_id;
    uint32_t sub_command_id;
};
#define OF_FLOW_TRANS_LISTNODE_OFFSET  offsetof(struct flow_trans, next_flow)

#define OF_FLOW_GENERATE_TRANS_ID(id)\
{\
    id++;\
    OF_LOG_MSG(OF_LOG_MOD, OF_LOG_DEBUG,"trans Id %d",id);\
}

#endif

uint64_t atox_64(char *cPtr);
uint32_t atox_32(char *cPtr);
int32_t of_flow_match_atox_mac_addr(char *data, unsigned char *mac_addr);
