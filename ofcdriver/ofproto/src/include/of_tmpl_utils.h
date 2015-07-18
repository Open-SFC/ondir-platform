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
* File:of_tmpl_utils.h
* Description:
*       This header file contains template APIs.
* Authors :Varun<B36461@freescale.com>
* Modifier:
* History:
*  Initial implementation.
******************************************************************************/


inline uint32_t ofu_template_start_pushing_instructions(uint8_t *inst);

inline void
ofu_template_start_pushing_apply_action_instruction(uint8_t *inst,
                                                   uint16_t *length);
inline int32_t
ofu_template_push_go_to_table_instruction(uint8_t *inst, 
                                          uint8_t table_id, 
                                          uint16_t *length_p);

inline void
ofu_template_end_pushing_apply_action_instruction(uint8_t *inst,uint16_t len);


inline void
ofu_template_end_pushing_instructions(uint8_t *inst);

inline void
ofu_template_start_pushing_actions(uint8_t *inst);

inline void
ofu_template_end_pushing_actions(uint8_t *inst);


inline int32_t
ofu_template_push_output_action(uint8_t   *inst,
                                uint32_t  port_no,
                                uint16_t  max_len,
                                uint16_t *length_p);


inline int32_t
ofu_template_push_set_ipv4_src_addr_in_set_field_action(uint8_t   *inst,
                                                ipv4_addr_t   *ipv4_addr,
                                                        uint16_t *length_p);

inline int32_t
ofu_template_push_set_ipv4_dst_addr_in_set_field_action(uint8_t   *inst,
                                               ipv4_addr_t   *ipv4_addr,
                                                     uint16_t *length_p);



inline int32_t
ofu_template_push_set_udp_src_port_in_set_field_action(uint8_t   *inst,
                                                       uint16_t  *port_number,
                                                       uint16_t  *length_p);


inline int32_t
ofu_template_push_set_udp_dst_port_in_set_field_action(uint8_t   *inst,
                                                       uint16_t  *port_number,
                                                       uint16_t  *length_p);


inline int32_t
ofu_template_push_set_tcp_src_port_in_set_field_action(uint8_t   *inst,
                                                       uint16_t  *port_number,
                                                       uint16_t  *length_p);


inline int32_t
ofu_template_push_set_tcp_dst_port_in_set_field_action(uint8_t   *inst,
                                                       uint16_t  *port_number,
                                                       uint16_t  *length_p);


inline int32_t
fslx_template_instruction_start_pushing_apply_actions_during_add_mod(
                                               uint8_t *inst,
                                               uint16_t *length_p);



inline
int32_t fslx_template_action_attach_bind_object(uint8_t *inst,
                                               uint32_t bind_obj_id,
                                               uint8_t  obj_type,
                                               uint16_t *length_p);


inline int32_t
fslx_template_instruction_end_pushing_apply_actions_during_add_mod(
                                                uint8_t *inst,uint16_t len);



inline
int32_t fslx_template_action_execute_bind_obj_actions(uint8_t *inst,
                                                    uint32_t bind_obj_id,
                                                    uint16_t *length_p);

inline int32_t
fslx_template_instruction_start_pushing_apply_actions_during_del(uint8_t *inst,
                                                           uint16_t *length_p);



inline
int32_t fslx_template_action_detach_bind_object(uint8_t *inst,
                                                uint32_t bind_obj_id,
                                                uint16_t *length_p);


inline
int32_t fslx_template_instruction_end_pushing_apply_actions_during_del(uint8_t *inst,uint16_t len);




inline int32_t
fslx_template_start_pushing_write_table_action_instruction(uint8_t *inst,
                                                           uint8_t table_id,
                                                           uint16_t *length_p);

inline int32_t
fslx_template_end_pushing_table_action_instruction(uint8_t *inst,uint16_t len);



inline int32_t
nicira_template_action_write_register(uint8_t   *inst,
                                      uint32_t index,
                                      uint64_t data,
                                      uint8_t offset,
                                      uint8_t n_bits,
                                      uint16_t *length_p);
























