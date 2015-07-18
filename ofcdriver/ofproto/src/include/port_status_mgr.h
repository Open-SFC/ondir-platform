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


#define PSM_SUCCESS 0
#define PSM_FAILURE -1

#define SYSTEM_PORT_NOT_READY  0
#define SYSTEM_PORT_READY      1

#define PORT_STATUS_UP         2
#define PORT_STATUS_DOWN       3
#define PORT_STATUS_UPDATE     4
#define PORT_STATUS_CREATED    5

int32_t psm_module_init();
int32_t psm_module_uninit();

void  psm_get_port_status(uint64_t logical_switch_handle,
                          char* port_name_p,uint64_t* dp_port_handle_p,
                          uint8_t* port_status_p, uint32_t* port_id_p);




