
/*
 *
 * Copyright  2012, 2013  Freescale Semiconductor
 *
 *
 * Licensed under the Apache License, version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *     Unless required by applicable law or agreed to in writing, software
 *     distributed under the License is distributed on an "AS IS" BASIS,
 *     WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *     See the License for the specific language governing permissions and
 *     limitations under the License.
*/

#define RRM_SUCCESS  0
#define RRM_FAILURE -1

#define RRM_ERROR_INCOMPLETE_RESOURCE_RELATIONSHIPS            -2
#define RRM_ERROR_NWPORT_NAME_NULL                             -3
#define RRM_ERROR_INVALID_VN_NAME                              -3
#define RRM_ERROR_INVALID_VM_NAME                              -4
#define RRM_ERROR_INVALID_VN_HANDLE                            -5
#define RRM_ERROR_INVALID_VM_HANDLE                            -6 
#define RRM_ERROR_INVALID_SWITCH_NAME                          -7
#define RRM_ERROR_INVALID_SWITCH_INFO                          -8
#define RRM_ERROR_FAILED_TO_ADD_CRM_COMPUTE_NODE               -9 
#define RRM_ERROR_CRM_CN_NODE_OR_CRM_LOGICAL_SWITCH_NOT_FOUND  -10
#define RRM_ERROR_INVALID_CRM_PORT_HANDLE                      -11

int32_t rrm_check_crm_nwport_resource_relationships(char* switch_name_p,
                                                    char* br_name_p,
                                                    uint64_t* switch_handle_p,
                                                    uint64_t* logical_switch_handle_p,
                                                    uint32_t* switch_ip);


int32_t rrm_check_crm_vmport_resource_relationships(char* switch_name_p,
                                                    char* br_name_p,
                                                    char* vn_name_p, 
                                                    char* vm_name_p,
                                                    uint64_t* switch_handle_p,
                                                    uint64_t* logical_switch_handle_p,
                                                    uint64_t* vn_handle_p,
                                                    uint64_t* vm_handle_p,
                                                    uint32_t* switch_ip_p,
                                                    uint8_t*  nw_type_p);


int32_t rrm_setup_crm_vmport_resource_relationships(char* port_name_p,
                                                    uint64_t crm_port_handle,
                                                    uint8_t  port_type,  
                                                    char* vm_name_p,
                                                    char* vn_name_p,
                                                    char* switch_name_p,
                                                    char* br_name_p);


int32_t rrm_delete_crm_vmport_resource_relationships(char*     port_name,
                                                     uint64_t  port_handle,
                                                     uint8_t   port_type,
                                                     char*     switch_name,
                                                     char*     vm_name,
                                                     char*     vn_name,
                                                     uint64_t  vn_handle, 
                                                     uint64_t  logical_switch_handle);

int32_t rrm_get_vm_logicalswitch_side_ports(char*      vm_name,
                                            uint64_t   vm_handle,
                                            struct     crm_virtual_machine* crm_vm_node_p,
                                            uint8_t*   no_of_ports_p,
                                            uint64_t*  port1_saferef_p,
                                            uint64_t*  port2_saferef_p);

int32_t rrm_delete_vmport_resource_relationships(char* port_name, uint64_t port_handle,uint8_t port_type,
                                                 char* switch_name, char* vm_name, char* vn_name,
                                                 uint64_t  vn_handle,
                                                 uint64_t  logical_switch_handle);
int32_t rrm_check_crmport_resource_relationships(char* switch_name_p,char* br_name_p,
                                                 char* vn_name_p, char* vm_name_p,
                                                 uint64_t* switch_handle_p,uint64_t* logical_switch_handle_p,
                                                 uint64_t* vn_handle_p, uint64_t* vm_handle_p,
                                                 uint32_t* switch_ip_p,uint8_t* nw_type_p);

